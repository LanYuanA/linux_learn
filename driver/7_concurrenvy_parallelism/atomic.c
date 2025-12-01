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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
文件名 : atomic.c
作者 : lanyuana
版本 : V1.0
描述 : 原子操作实验 使用原子变量来实现对设备的互斥访问。
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

#define GPIOLED_CNT   1               /*设备号个数*/
#define GPIOLED_NAME  "gpioled"    /*设备名*/

#define LEDOFF 0/*关灯*/
#define LEDON 1/*开灯*/

/*dtsled设备结构体*/
struct gpioled_dev
{
    dev_t devid;    /*设备号*/
    struct cdev cdev;   /*cdev*/
    struct class *class;    /*类*/
    struct device *device;  /*设备*/
    int major;  /*主设备号*/    
    int minor;  /*次设备号*/
    struct device_node *nd; /*设备节点*/
    int led_gpio;/*led所使用的GPIO编号*/
    atomic_t lock;/*原子变量*/
};

struct gpioled_dev gpioled;/*led设备*/

/*
* @description : LED打开/关闭
* @param – inode : 传递给驱动的 inode
* @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
*                 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
void led_switch(struct gpioled_dev *dev,u8 sta)
{   //LED是低电平触发
    if(sta == LEDON)
    {
        gpio_set_value(dev->led_gpio,0);/*打开led*/
    }
    else if(sta == LEDOFF)
    {
        gpio_set_value(dev->led_gpio,1); /*关闭led*/
    }
}


/*
* @description : 打开设备
* @param – inode : 传递给驱动的 inode
* @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
*                 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
static int led_open(struct inode *inode,struct file *filp)
{
    /*通过判断原子变量的值来检查LED有没有被其他应用使用*/
    if(!atomic_dec_and_test(&gpioled.lock))
    {
        atomic_inc(&gpioled.lock);//如果小于0的话就加1 让原子变量等于0
        return -EBUSY;/*返回忙*/
    }
    /*用户实现具体功能*/
    filp->private_data = &gpioled;/*设置私有数据*/
    return 0;
}

/*
* @description : 从设备读取数据
* @param - filp : 要打开的设备文件(文件描述符)
* @param - buf : 返回给用户空间的数据缓冲区
* @param - cnt : 要读取的数据长度
* @param - offt : 相对于文件首地址的偏移
* @return : 读取的字节数，如果为负值，表示读取失败
*/
static ssize_t led_read(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
{
    /*
    int retvalue=0;
    u32 val =0;
    unsigned char status =0;//存放发送给用户的数据
    struct gpioled_dev *dev = filp->private_data;
    val = gpio_get_value(dev->led_gpio);//读取值
    if(val == 1)//高电平是关闭
    {
        status = LEDOFF;
    }
    else 
    {
        status = LEDON;
    }
    retvalue = copy_to_user(buf,&status,1);
    if(retvalue)
    {
        printk("send to user failed!\r\n");
        return -EFAULT;
    }
    //read成功通常返回读取的字节数
    */
    return 0;
}

/*
* @description : 向设备写数据
* @param - filp : 设备文件，表示打开的文件描述符
* @param - buf : 要写给设备写入的数据
* @param - cnt : 要写入的数据长度
* @param - offt : 相对于文件首地址的偏移
* @return : 写入的字节数，如果为负值，表示写入失败
*/
static ssize_t led_write(struct file *filp,const char __user *buf,size_t cnt,loff_t *offt)
{
    /*用户实现具体功能*/
    int retvalue =0;
    unsigned char databuf[1];
    unsigned char ledtat;
    struct gpioled_dev *dev = filp->private_data;

    retvalue = copy_from_user(databuf,buf,cnt);//从传入的buf写入到databuf 个数为cnt
    if(retvalue)
    {
        printk("kernal recevdata failed!\r\n");
        return -EFAULT;
    }
    ledtat = databuf[0];/*从用户输入的指令中获得控制值 获取状态值*/
    if(ledtat == LEDON)
    {
        led_switch(dev,LEDON);
    }
    else if(ledtat == LEDOFF)
    {
        led_switch(dev,LEDOFF);
    }
    return 0;
}


/*
* @description : 关闭/释放设备
* @param - filp : 要关闭的设备文件(文件描述符)
* @return : 0 成功;其他 失败
*/
static int led_release(struct inode *inode,struct file *filp)
{
    /*用户实现具体功能*/
    struct gpioled_dev *dev = filp->private_data;
    atomic_inc(&dev->lock);//在关闭设备的时候让原子变量加1
    return 0;
}

/*
*   设备操作函数结构体
*/
static struct  file_operations gpioled_fops ={
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init led_init(void)
{
    /*入口函数具体内容*/
    int ret;
    /*初始化原子变量*/
    atomic_set(&gpioled.lock,1);
    /*设置LED所使用的GPIO*/
    /*1.获取设备树中属性数据*/
    gpioled.nd = of_find_node_by_path("/gpioed");
    if(gpioled.nd == NULL)
    {
        printk("gpioled node cant not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("gpioled node has been found!\r\n");
    }
    /*获取设备树中GPIO属性 得到led所使用的led编号*/
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd,"led-gpios",0);
    if(gpioled.led_gpio < 0)
    {
        printk("gpioled.led_gpio cant not found!\r\n");
        return -EINVAL;        
    }
    else
    {
        printk("gpioled.led_gpio has been found!\r\n");
    }
    /*设置GPIO1_IO03为输出 输出高电平 默认关闭LED*/
    ret = gpio_direction_output(gpioled.led_gpio,1);
    if(ret < 0)
    {
        printk("gpio_direction_output failed!\r\n");
        return -EINVAL;        
    }

    /*注册字符设备驱动*/
    if(gpioled.major)//指定了major
    {
        gpioled.devid = MKDEV(gpioled.major,0);//次设备号选择0
        register_chrdev_region(gpioled.devid,GPIOLED_CNT,GPIOLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&gpioled.devid,0,GPIOLED_CNT,GPIOLED_NAME);//申请设备号
        gpioled.major = MAJOR(gpioled.devid);//获得主设备号
        gpioled.minor = MINOR(gpioled.devid);//获得次设备号
    }
    
    gpioled.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&gpioled.cdev,&gpioled_fops);//初始化cdev

    cdev_add(&gpioled.cdev,gpioled.devid,GPIOLED_CNT);//增加一个cdev

    /*创建类*/
    gpioled.class = class_create(THIS_MODULE,GPIOLED_NAME);
    /*创建设备*/
    gpioled.device = device_create(gpioled.class,NULL,gpioled.devid,NULL,GPIOLED_NAME);

    return 0;
}

/*
* @description : 驱动出口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static void __exit led_exit(void)
{
    /*注销字符设备*/
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid,GPIOLED_CNT);
    device_destroy(gpioled.class,gpioled.devid);
    class_destroy(gpioled.class);


    printk("chrdevbase_exit()\r\n");
}

/*
*将上述函数指定为驱动的入口和出口函数
*/
module_init(led_init);
module_exit(led_exit);
/*
*LICENSE和作者信息
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LanYuanA");