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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名 : leddriver.c
作者 : lanyuana
版本 : V1.0
描述 : led 驱动文件。
其他 : 无
日志 : 初版 V1.0 2025/12/10 lanyuana创建
***************************************************************/

#define DTSLED_CNT   1               /*设备号个数*/
#define DTSLED_NAME  "dtsplatled"    /*设备名*/

#define LEDOFF 0/*关灯*/
#define LEDON 1/*开灯*/


/*dtsled设备结构体*/
struct dtsled_dev
{
    dev_t devid;    /*设备号*/
    struct cdev cdev;   /*cdev*/
    struct class *class;    /*类*/
    struct device *device;  /*设备*/
    int major;  /*主设备号*/    
    struct device_node *nd; /*设备节点*/
    int led0;/*led的gpio标号*/
};

struct dtsled_dev dtsled;/*led设备*/

/*
* @description : LED打开/关闭
* @param – inode : 传递给驱动的 inode
* @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
*                 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
void led_switch(u8 sta)
{   //LED是低电平触发
    if(sta == LEDON)
    {
        gpio_set_value(dtsled.led0,0);
    }
    else if(sta == LEDOFF)
    {
        gpio_set_value(dtsled.led0,1);
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
    /*用户实现具体功能*/
    filp->private_data = &dtsled;/*设置私有数据*/
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
    int retvalue=0;
    unsigned char ledstat;

    ledstat = gpio_get_value(dtsled.led0)?LEDOFF : LEDON;//读值
    retvalue = copy_to_user(buf,&ledstat,sizeof(ledstat));//将状态发送给用户空间
    if(retvalue)
    {
        printk("send to user failed!\r\n");
        return -EFAULT;
    }
    //read成功通常返回读取的字节数
    return sizeof(ledstat);
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
    unsigned char databuf[2];
    unsigned char ledtat;
    retvalue = copy_from_user(databuf,buf,cnt);//从传入的buf写入到databuf 个数为cnt
    if(retvalue < 0 )
    {
        printk("kernal recevdata failed!\r\n");
        return -EFAULT;
    }
    ledtat = databuf[0];/*从用户输入的指令中获得控制值 获取状态值*/
    if(ledtat == LEDON)
    {
        led_switch(LEDON);
    }
    else if(ledtat == LEDOFF)
    {
        led_switch(LEDOFF);
    }
    return cnt;
}


/*
* @description : 关闭/释放设备
* @param - filp : 要关闭的设备文件(文件描述符)
* @return : 0 成功;其他 失败
*/
static int led_release(struct inode *inode,struct file *filp)
{
    /*用户实现具体功能*/
    return 0;
}

/*
*   设备操作函数结构体
*/
static struct  file_operations dtsled_fops ={
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/*
* @description : platform驱动的probe函数 当驱动和设备匹配后此函数会执行
* @param dev：   platform设备
* @return : 0 成功;其他 失败
*/
static int led_probe(struct platform_device *dev)
{
    printk("led driver and device matched!\r\n");
    /*设置设备号*/
    /*注册字符设备驱动*/
    if(dtsled.major)//指定了major
    {
        dtsled.devid = MKDEV(dtsled.major,0);//次设备号选择0
        register_chrdev_region(dtsled.devid,DTSLED_CNT,DTSLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&dtsled.devid,0,DTSLED_CNT,DTSLED_NAME);//申请设备号
        dtsled.major = MAJOR(dtsled.devid);//获得主设备号
    }
    
    dtsled.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&dtsled.cdev,&dtsled_fops);//初始化cdev

    cdev_add(&dtsled.cdev,dtsled.devid,DTSLED_CNT);//增加一个cdev

    /*创建类*/
    dtsled.class = class_create(THIS_MODULE,DTSLED_NAME);
    /*创建设备*/
    dtsled.device = device_create(dtsled.class,NULL,dtsled.devid,NULL,DTSLED_NAME);

    /*初始化IO*/
    dtsled.nd = of_find_node_by_path("/gpioled");
    if(dtsled.nd == NULL)
    {
        printk("of_find_node_by_path /gpioled failed!\r\n)");
        return -EINVAL; 
    }
    else
    {
        printk("of_find_node_by_path /gpioled successed!\r\n)");        
    }
    //获取属性内容
    dtsled.led0 = of_get_named_gpio(dtsled.nd,"led-gpio",0);//获取gpio标号
    if(dtsled.led0 < 0)
    {
        printk("gpio not get!\r\n");
        return -EINVAL;
    }
    gpio_request(dtsled.led0,"led0");
    gpio_direction_output(dtsled.led0,1);/*设置为输出 默认高电平*/
    
    return 0;
}

/*
* @description : remove函数 移除platform驱动的时候此函数执行
* @param dev：        platform设备
* @return : 0 成功;其他 失败
*/
static int led_remove(struct platform_device *dev)
{
    gpio_set_value(dtsled.led0,1);//恢复高电平
    cdev_del(&dtsled.cdev);//删除cdev
    unregister_chrdev_region(dtsled.devid,DTSLED_CNT);//删除设备号
    device_destroy(dtsled.class,dtsled.devid);
    class_destroy(dtsled.class);
    return 0;
}

/*匹配列表*/
static const struct of_device_id led_of_match[]=
{
    {.compatible = "atkalpha-gpioled"},
    {/*sentinel*/}    
};

/*platform驱动结构体*/
static struct platform_driver led_driver = {
    .driver = {
        .name = "imx6ul-led-test",/*驱动名 用于与设备匹配*/
        .of_match_table = led_of_match,/*设备树匹配表*/
    },
    .probe = led_probe,
    .remove = led_remove,
};



/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init led_init(void)
{
    return platform_driver_register(&led_driver);
}

/*
* @description : 驱动出口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static void __exit led_exit(void)
{
    platform_driver_unregister(&led_driver);
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