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
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
文件名 : unblockio.c
作者 : lanyuana
版本 : V1.0
描述 : 阻塞IO访问
其他 : 无
日志 : 2025/12/02 lanyuana创建
***************************************************************/

#define IMX6UIRQ_CNT   1               /*设备号个数*/
#define IMX6UIRQ_NAME  "unblockio"    /*设备名*/

/*定义按键值*/
#define KEY0VALUE 0x01/*按键值*/
#define INVAKEY 0x00/*无效按键值*/
#define KEY_NUM 1 /* 按键数量 */

/* 中断IO描述结构体 */
struct irq_keydesc
{
    int gpio;/* gpio */
    int irqnum;/* 中断号 */
    unsigned char value;/* 按键对应的键值 */
    char name[10];/*名字*/
    irqreturn_t (*handler)(int ,void *);/*中断服务函数*/
};

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
    atomic_t keyvalue;/*有效的按键键值*/
    atomic_t releasekey;/*标记是否完成一次完成按键*/
    struct timer_list timer;/*定义一个定时器*/
    struct irq_keydesc irqkeydesc[KEY_NUM];/*按键描述数组*/
    unsigned char curkeynum;/*当前的按键号*/

    wait_queue_head_t r_wait;/*读等待队列头*/
};

struct key_dev key;/*key设备*/


/*
* @description : 中断服务函数 开启定时器 延时10ms 定时器用于按键消抖
* @param – irq : 中断号
* @param - dev_id : 设备结构
* @return : 中断执行结果
*/
static irqreturn_t key0_handler(int irq,void *dev_id)
{
    struct key_dev *dev = (struct key_dev *)dev_id;

    dev->curkeynum = 0;//设置当前的按键号
    dev->timer.data = (volatile long)dev_id;//把设备结构体作为参数传入timer
    mod_timer(&dev->timer,jiffies + msecs_to_jiffies(10));
    return IRQ_RETVAL(IRQ_HANDLED);

}

/*
* @description : 定时器服务程序 用于按键消抖 
*    定时器到了之后再次读取按键值 若按键还是处于按下状态就表示按键有效
* @param – arg : 设备结构变量
* @return : 中断执行结果
*/
void timer_function(unsigned long arg)
{
    struct irq_keydesc keydesc;
    unsigned char num;
    unsigned char value;
    struct key_dev *dev = (struct key_dev *)arg;

    num = dev->curkeynum;//获取当前按键号
    keydesc = dev->irqkeydesc[num];//启用当前按键号对应的结构体
    value = gpio_get_value(keydesc.gpio);/*读取IO值*/
    if(value == 0)
    {
        //如果按下
        atomic_set(&dev->keyvalue,keydesc.value);//把IO写入到dev的键值
    }
    else/*按键松开*/
    {
        atomic_set(&dev->keyvalue, 0x80 | keydesc.value);//keyvalue = 0x80 | keydesc->value = 0x81
        atomic_set(&dev->releasekey,1);/*标记松开按键*/
    }
    /*唤醒进程*/
    if(atomic_read(&dev->releasekey))
    {
        /*完成一次按键操作*/
        /*wake_up(&dev->r_wait)*/
        wake_up_interruptible(&dev->r_wait);
    }
}

/*按键io初始化*/
static int keyio_init(void)
{
    unsigned char i=0;
    int ret = 0;
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
    for(i=0 ;i<KEY_NUM;i++)
    {
        key.irqkeydesc[i].gpio = of_get_named_gpio(key.nd,"key-gpio",i);
        if(key.irqkeydesc[i].gpio < 0)
        {
            printk("can't get key%d\r\n", i);
            return -EINVAL;        
        }
    }
    /**/
    for(i=0 ;i<KEY_NUM;i++)
    {
        memset(key.irqkeydesc[i].name,0,sizeof(key.irqkeydesc[i].name));
        sprintf(key.irqkeydesc[i].name,"key%d",i);//写入对应的按键名
        /*请求IO*/
        gpio_request(key.irqkeydesc[i].gpio,key.irqkeydesc[i].name);
        gpio_direction_input(key.irqkeydesc[i].gpio);//设置为输入模式
        key.irqkeydesc[i].irqnum = irq_of_parse_and_map(key.nd,i);//设备树中查找中断号
    #if 0
        key.irqkeydesc[i].irqnum = gpio_to_irq(key.irqkeydesc[i].gpio);
    #endif
        printk("key%d:gpio=%d,irqnum=%d\r\n",key.curkeynum,key.irqkeydesc[i].gpio,key.irqkeydesc[i].irqnum);
    }    
    //申请中断
    key.irqkeydesc[0].handler = key0_handler;//中断到了就设置定时器
    key.irqkeydesc[0].value = KEY0VALUE;

    for(i=0;i<KEY_NUM;i++)
    {
        ret = request_irq(
            key.irqkeydesc[i].irqnum,
            key.irqkeydesc[i].handler,
            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,//上升沿或下降沿都触发
            key.irqkeydesc[i].name,
            &key
        );
        if(ret < 0 )
        {
            printk("irq %d request failed!\r\n",key.irqkeydesc[i].irqnum);
            return -EFAULT;
        }
    }

    /*创建定时器*/
    init_timer(&key.timer);
    key.timer.function = timer_function;
    /*初始化等待队列头*/
    init_waitqueue_head(&key.r_wait);
    return 0;
}

/*
* @description : 打开设备
* @param – inode : 传递给驱动的 inode
* @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
*                 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
static int key_open(struct inode *inode,struct file *filp)
{
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
    unsigned char keyvalue =0;
    unsigned char releasekey = 0;
    struct key_dev *dev = (struct key_dev *)filp->private_data;

    if(filp->f_flags & O_NONBLOCK)//非阻塞访问
    {
        if(atomic_read(&dev->releasekey) == 0)//按键未按下
        {
            return -EAGAIN;
        }
    }
    else
    {
        #if 1
            retvalue = wait_event_interruptible(dev->r_wait,atomic_read(&dev->releasekey));//阻塞访问
            if(retvalue) goto wait_error;
        #endif
    }
    #if 0
    DECLARE_WAITQUEUE(wait, current);/*定义一个等待队列*/
    if(atomic_read(&dev->releasekey) == 0)//没有按键按下
    {
        add_wait_queue(&dev->r_wait,&wait);/*添加到等待队列头*/
        __set_current_state(TASK_INTERRUPTIBLE);/*设置任务状态*/
        schedule(); /*进行一个任务切换*/
        if(signal_pending(current))//判断是否为信号引起的唤醒
        {
            retvalue = -ERESTARTSYS;
            goto wait_error;
        }
    }
    __set_current_state(TASK_RUNNING);/*设置为运行态*/
    remove_wait_queue(&dev->r_wait,&wait);/*移除出等待队列*/
    #endif
    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);

    if(releasekey)//有按键按下
    {
        if(keyvalue & 0x80)//检查最高位是否为1
        {
            keyvalue &= ~0x80;//若按键松开 则清除最高位 得到0x01
            retvalue = copy_to_user(buf,&keyvalue,sizeof(keyvalue));
        }
        else
        {
            goto data_error;
        }

        atomic_set(&dev->releasekey,0);/*按下标志清0*/
    }
    else
    {
        goto data_error;
    }
    return 0;
    
    wait_error:
        return retvalue;
    data_error:
        return -EINVAL;
}

/*
* @description : poll函数 应用程序调用poll会同时调用该函数
* @param - filp : 要打开的设备文件(文件描述符)
* @param - wait : 等待列表(poll_table)
* @return : 设备或者资源状态
*/
unsigned int key_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask =0;
    struct key_dev *dev = (struct key_dev *)filp->private_data;
    poll_wait(filp,&dev->r_wait,wait);

    if(atomic_read(&dev->releasekey))//有按键按下
    {
        mask = POLLIN | POLLRDNORM;//返回pollin
    }
    return mask;
}


/*
*   设备操作函数结构体
*/
static struct  file_operations key_fops ={
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .poll = key_poll,
};

/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init mykey_init(void)
{
    /*注册字符设备驱动*/
    if(key.major)//指定了major
    {
        key.devid = MKDEV(key.major,0);//次设备号选择0
        register_chrdev_region(key.devid,IMX6UIRQ_CNT,IMX6UIRQ_NAME);
    }
    else
    {
        alloc_chrdev_region(&key.devid,0,IMX6UIRQ_CNT,IMX6UIRQ_NAME);//申请设备号
        key.major = MAJOR(key.devid);//获得主设备号
        key.minor = MINOR(key.devid);//获得次设备号
    }
    
    key.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&key.cdev,&key_fops);//初始化cdev

    cdev_add(&key.cdev,key.devid,IMX6UIRQ_CNT);//增加一个cdev

    /*创建类*/
    key.class = class_create(THIS_MODULE,IMX6UIRQ_NAME);
    /*创建设备*/
    key.device = device_create(key.class,NULL,key.devid,NULL,IMX6UIRQ_NAME);

    /*初始化按键*/
    atomic_set(&key.keyvalue,INVAKEY);
    atomic_set(&key.releasekey,0);    
    keyio_init();
    return 0;
}

/*
* @description : 驱动出口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static void __exit mykey_exit(void)
{
    unsigned int i=0;
    /*注销字符设备*/
    for(i=0;i<KEY_NUM;i++)
    {
        free_irq(key.irqkeydesc[i].irqnum,&key);
        gpio_free(key.irqkeydesc[i].gpio);
    }
    cdev_del(&key.cdev);
    unregister_chrdev_region(key.devid,IMX6UIRQ_CNT);
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