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
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
文件名 : timer.c
作者 : lanyuana
版本 : V1.0
描述 : 原子操作实验 使用原子变量来实现对设备的互斥访问。
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

#define GPIOLED_CNT   1               /*设备号个数*/
#define GPIOLED_NAME  "gpioled"    /*设备名*/

#define CLOSE_CMD (_IO(0XEF, 0x1)) /* 关闭定时器 */
#define OPEN_CMD (_IO(0XEF, 0x2)) /* 打开定时器 */
#define SETPERIOD_CMD (_IO(0XEF, 0x3)) /* 设置定时器周期命令 */

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
    spinlock_t lock;/*自旋锁*/
    struct timer_list timer;/*定时器*/
    int timeperiod;/*定时周期 单位是ms*/
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
    /*用户实现具体功能*/
    filp->private_data = &gpioled;/*设置私有数据*/

    gpioled.timeperiod = 1000;/*默认周期是1s*/
    return 0;
}

/*
* @description : timer_unlocked_ioctl
* @param - filp : 要打开的设备文件(文件描述符)
* @param - cmd : 应用程序发来的命令
* @param - arg : 参数
* @return : 0 成功 ；其他 失败
*/
static long timer_unlocked_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
    struct gpioled_dev *dev = (struct gpioled_dev *)filp->private_data;
    int timerperiod;
    unsigned long flags;

    switch (cmd)
    {
    case CLOSE_CMD:
        /* code */
        del_timer_sync(&dev->timer);
        break;
    case OPEN_CMD:
        spin_lock_irqsave(&dev->lock,flags);/*上锁*/
        timerperiod = dev->timeperiod;
        spin_unlock_irqrestore(&dev->lock,flags);/*解锁*/
        mod_timer(&dev->timer,jiffies + msecs_to_jiffies(timerperiod));/*重新设置定时器*/
    case SETPERIOD_CMD:
        spin_lock_irqsave(&dev->lock,flags);/*上锁*/
        dev->timeperiod = arg;
        spin_unlock_irqrestore(&dev->lock,flags);/*解锁*/
        mod_timer(&dev->timer,jiffies + msecs_to_jiffies(arg));/*重新设置定时器*/        
    default:
        break;
    }
    return 0;
}

/*
*   设备操作函数结构体
*/
static struct  file_operations gpioled_fops ={
    .owner = THIS_MODULE,
    .open = led_open,
    .unlocked_ioctl = timer_unlocked_ioctl,
};

/*定时器回调函数 arg是gpioled_dev的地址*/
void timer_function(unsigned long arg)
{
    struct gpioled_dev *dev = (struct gpioled_dev *)arg;
    static int sta = 1;
    int timerperiod;
    unsigned long flags;

    sta =!sta;/*每次翻转*/
    gpio_set_value(dev->led_gpio,sta);
    /*重启定时器*/
    spin_lock_irqsave(&dev->lock,flags);/*上锁*/
    timerperiod = dev->timeperiod;
    spin_unlock_irqrestore(&dev->lock,flags);/*解锁*/
    mod_timer(&dev->timer,jiffies + msecs_to_jiffies(timerperiod));/*重新设置定时器*/
}

/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init led_init(void)
{
    /*入口函数具体内容*/
    int ret;
    /*初始化自旋锁*/
    spin_lock_init(&gpioled.lock);
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

    /*初始化定时器*/
    init_timer(&gpioled.timer);
    gpioled.timer.function = timer_function;
    gpioled.timer.data = (unsigned long)&gpioled;
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
    gpio_set_value(gpioled.led_gpio, 1); /* 卸载驱动的时候关闭 LED */
    del_timer_sync(&gpioled.timer); /* 删除 timer */
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