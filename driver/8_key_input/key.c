#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
文件名 : key.c
作者 : lanyuana
版本 : V1.0
描述 : Linux按键输入驱动
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

#define KEY_CNT   1               /*设备号个数*/
#define KEY_NAME  "key"    /*设备名*/

/*定义按键值*/
#define KEYVALUE 0xF0/*按键值*/
#define INVAKEY 0x00/*无效按键值*/

/*dtskey设备结构体*/
struct key_dev
{
    dev_t devid;    /*设备号*/
    struct cdev cdev;   /*cdev*/
    struct class *class;    /*类*/
    struct device *device;  /*设备*/
    int major;  /*主设备号*/    
    int minor;  /*次设备号*/
    struct device_node *nd; /*设备节点*/
    int key_gpio;/*key所使用的GPIO编号*/
    atomic_t keyvalue;/*原子变量*/
};

struct key_dev key;/*key设备*/




/*
* @description : 打开设备
* @param – inode : 传递给驱动的 inode
* @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
*                 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
static int key_open(struct inode *inode,struct file *filp)
{
    /*通过判断原子变量的值来检查LED有没有被其他应用使用*/
    if(!atomic_dec_and_test(&key.keyvalue))
    {
        atomic_inc(&key.keyvalue);//如果小于0的话就加1 让原子变量等于0
        return -EBUSY;/*返回忙*/
    }
    /*用户实现具体功能*/
    filp->private_data = &key;/*设置私有数据*/
    return 0;
}

/*
* @description : 从设备读取数据 并保存在buf中
* @param - filp : 要打开的设备文件(文件描述符)
* @param - buf : 返回给用户空间的数据缓冲区
* @param - cnt : 要读取的数据长度
* @param - offt : 相对于文件首地址的偏移
* @return : 读取的字节数，如果为负值，表示读取失败
*/
static ssize_t key_read(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
{
    
    int retvalue=0;
    unsigned char val ;
    struct key_dev *dev = filp->private_data;
    //读取值
    if(gpio_get_value(dev->key_gpio) == 0)/*key0按下*/
    {
        /*等待按键松开*/
        while(!gpio_get_value(dev->key_gpio));//按键松开
        atomic_set(&dev->keyvalue,KEYVALUE);
    }
    else
    {
        atomic_set(&dev->keyvalue,INVAKEY);//无效值 
    }
    val = atomic_read(&dev->keyvalue);
    retvalue = copy_to_user(buf,&val,sizeof(val));
    if(retvalue)
    {
        printk("send to user failed!\r\n");
        return -EFAULT;
    }
    //read成功通常返回读取的字节数
    
    return retvalue;
}

/*
* @description : 向设备写数据
* @param - filp : 设备文件，表示打开的文件描述符
* @param - buf : 要写给设备写入的数据
* @param - cnt : 要写入的数据长度
* @param - offt : 相对于文件首地址的偏移
* @return : 写入的字节数，如果为负值，表示写入失败
*/
static ssize_t key_write(struct file *filp,const char __user *buf,size_t cnt,loff_t *offt)
{
#if 0
    int retvalue =0;
    unsigned char databuf[1];
    unsigned char keytat;
    struct key_dev *dev = filp->private_data;

    retvalue = copy_from_user(databuf,buf,cnt);//从传入的buf写入到databuf 个数为cnt
    if(retvalue)
    {
        printk("kernal recevdata failed!\r\n");
        return -EFAULT;
    }
    keytat = databuf[0];/*从用户输入的指令中获得控制值 获取状态值*/
    if(keytat == LEDON)
    {
        key_switch(dev,LEDON);
    }
    else if(keytat == LEDOFF)
    {
        key_switch(dev,LEDOFF);
    }
    */
#endif
    return 0;
}


/*
* @description : 关闭/释放设备
* @param - filp : 要关闭的设备文件(文件描述符)
* @return : 0 成功;其他 失败
*/
static int key_release(struct inode *inode,struct file *filp)
{
    /*用户实现具体功能*/
    struct key_dev *dev = filp->private_data;
    atomic_inc(&dev->keyvalue);//在关闭设备的时候让原子变量加1
    return 0;
}

/*
*   设备操作函数结构体
*/
static struct  file_operations key_fops ={
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .write = key_write,
    .release = key_release,
};

/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init mykey_init(void)
{
    /*入口函数具体内容*/
    int ret;
    /*初始化原子变量*/
    atomic_set(&key.keyvalue,1);
    /*设置LED所使用的GPIO*/
    /*1.获取设备树中属性数据*/
    key.nd = of_find_node_by_path("/key");
    if(key.nd == NULL)
    {
        printk("key node cant not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("key node has been found!\r\n");
    }
    /*获取设备树中GPIO属性 得到key所使用的key编号*/
    key.key_gpio = of_get_named_gpio(key.nd,"key-gpio",0);
    if(key.key_gpio < 0)
    {
        printk("key.key_gpio cant not found!\r\n");
        return -EINVAL;        
    }
    else
    {
        printk("key_gpio=%d\r\n", key.key_gpio);
    }
    /*请求IO*/
    gpio_request(key.key_gpio,"key0");
    /*设置GPIO为输入*/
    ret = gpio_direction_input(key.key_gpio);
    if(ret < 0)
    {
        printk("gpio_direction_output failed!\r\n");
        return -EINVAL;        
    }

    /*注册字符设备驱动*/
    if(key.major)//指定了major
    {
        key.devid = MKDEV(key.major,0);//次设备号选择0
        register_chrdev_region(key.devid,KEY_CNT,KEY_NAME);
    }
    else
    {
        alloc_chrdev_region(&key.devid,0,KEY_CNT,KEY_NAME);//申请设备号
        key.major = MAJOR(key.devid);//获得主设备号
        key.minor = MINOR(key.devid);//获得次设备号
    }
    
    key.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&key.cdev,&key_fops);//初始化cdev

    cdev_add(&key.cdev,key.devid,KEY_CNT);//增加一个cdev

    /*创建类*/
    key.class = class_create(THIS_MODULE,KEY_NAME);
    /*创建设备*/
    key.device = device_create(key.class,NULL,key.devid,NULL,KEY_NAME);

    return 0;
}

/*
* @description : 驱动出口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static void __exit mykey_exit(void)
{
    /*注销字符设备*/
    gpio_free(key.key_gpio);
    cdev_del(&key.cdev);
    unregister_chrdev_region(key.devid,KEY_CNT);
    device_destroy(key.class,key.devid);
    class_destroy(key.class);


    printk("chrdevbase_exit()\r\n");
}

/*
*将上述函数指定为驱动的入口和出口函数
*/
module_init(mykey_init);
module_exit(mykey_exit);
/*
*LICENSE和作者信息
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LanYuanA");