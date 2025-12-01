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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名 : dtsled.c
作者 : lanyuana
版本 : V1.0
描述 : led 驱动文件。
其他 : 无
日志 : 初版 V1.0 2025/11/22 lanyuana创建
***************************************************************/

#define DTSLED_CNT   1               /*设备号个数*/
#define DTSLED_NAME  "dtsled"    /*设备名*/

#define LEDOFF 0/*关灯*/
#define LEDON 1/*开灯*/

/*寄存器物理地址*/
/*#define CCM_CCGR1_BASE          (0x020C406C) // 27:26 对应gpio1 clock
#define SW_MUX_GPIO1_IO03_BASE  (0x020E0068)
#define SW_PAD_GPIO1_IO03_BASE  (0x020E02F4)
#define GPIO1_DR_BASE           (0x0209C000)
#define GPIO1_GDIR_BASE         (0x0209C004)*/

/*映射后的寄存器虚拟地址指针*/
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/*dtsled设备结构体*/
struct dtsled_dev
{
    dev_t devid;    /*设备号*/
    struct cdev cdev;   /*cdev*/
    struct class *class;    /*类*/
    struct device *device;  /*设备*/
    int major;  /*主设备号*/    
    int minor;  /*次设备号*/
    struct device_node *nd; /*设备节点*/
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
    u32 val =0;
    if(sta == LEDON)
    {
        val = readl(GPIO1_DR);//读取DR寄存器的值 
        val &=~(1<<3);//将val的第3位 bit3清零 保持其他位不动 &是按位与 都为1时才为1
        writel(val,GPIO1_DR);
    }
    else if(sta == LEDOFF)
    {
        val = readl(GPIO1_DR);//读取DR寄存器的值 
        val |=(1<<3);//将val的第4位 bit3清零 保持其他位不动 |是按位或 都为0时才为0
        writel(val,GPIO1_DR);   
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
    u32 val =0;
    unsigned char status =0;//存放发送给用户的数据
    val = readl(GPIO1_DR);//读DR寄存器的值 看第四位 io_03
    if(val & (1<<3))//高电平是关闭
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
    return 1;
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
    retvalue = copy_from_user(databuf,buf,cnt);//从传入的buf写入到databuf 个数为cnt
    if(retvalue)
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
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init led_init(void)
{
    /*入口函数具体内容*/
    u32 val = 0;
    struct property *proper;
    const char *str;
    u32 regdata[14];
    int ret;
    /*初始化LED*/
    /*1.获取设备树中属性数据*/
    dtsled.nd = of_find_node_by_path("/alphaled");
    if(dtsled.nd == NULL)
    {
        printk("of_find_node_by_path /alphaled failed!\r\n)");
        return -EINVAL; 
    }
    else
    {
        printk("of_find_node_by_path /alphaled successed!\r\n)");        
    }
    //获取属性内容
    proper = of_find_property(dtsled.nd,"compatible",NULL);
    if(proper == NULL)
    {
        printk("of_find_property compatible failed!\r\n");
    }
    else
    {
        printk("of_find_property compatible successed!\r\n");        
    }
    /*获取reg属性内容*/

    ret = of_property_read_string(dtsled.nd,"status",&str);
    if(ret <0)
    {
        printk("of_property_read_string status failed!\r\n");
    }
    else
    {
        printk("status successed!:%s\r\n",str);        
    }
    /*获取reg的值*/
    ret = of_property_read_u32_array(dtsled.nd,"reg",regdata,10);
    if(ret<0)
    {
        printk("of_property_read_u32_array failed");
    }
    else
    {
        u8 i =0;
        printk("reg data:\r\n");
        for(i=0;i<10;i++)
            printk("%#X",regdata[i]);
        
        printk("\r\n");
    }
    /*初始化LED*/
#if 0
/*寄存器地址映射*/
 IMX6U_CCM_CCGR1 = ioremap(regdata[0], regdata[1]);
 SW_MUX_GPIO1_IO03 = ioremap(regdata[2], regdata[3]);
 SW_PAD_GPIO1_IO03 = ioremap(regdata[4], regdata[5]);
 GPIO1_DR = ioremap(regdata[6], regdata[7]);
 GPIO1_GDIR = ioremap(regdata[8], regdata[9]);
#else
    IMX6U_CCM_CCGR1 = of_iomap(dtsled.nd, 0);
    SW_MUX_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
    SW_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
    GPIO1_DR = of_iomap(dtsled.nd, 3);
    GPIO1_GDIR = of_iomap(dtsled.nd, 4);
#endif

    /*2.使能GPIO1时钟 0+26 1+26*/
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3<<26);//清除之前的设置 将26 27位清零
    val |=(3<<26);//将26 27为置1
    writel(val,IMX6U_CCM_CCGR1);

    /*设置GPIO1_IO03的复用功能 复用为GPIO1_IO03 最后设置IO属性*/
    //mux_mode alt5 0101 5 
    writel(5,SW_MUX_GPIO1_IO03);
    //设置PAD IO属性
    /*
        0001 0000 1011 0000
        bit 12 PKE 使能keeper功能
        bit 11 ODE 开漏输出关闭 推挽输出模式
        bit 7-6 10 100Mhz
        bit 5-3 110 驱动能力R0/6 较大的驱动电流
    */
    writel(0x10B0,SW_PAD_GPIO1_IO03);

    /*4.设置GPIO1_IO03输出功能 GDIR方向寄存器*/
    val = readl(GPIO1_GDIR);
    val &= ~(1<<3);//将第4位清0
    val |= (1<<3);//将第4位置1
    writel(val,GPIO1_GDIR);

    /*默认关闭led*/
    val = readl(GPIO1_DR);
    val |= (1<<3);/*将第四位置1*/
    writel(val,GPIO1_DR);

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
        dtsled.minor = MINOR(dtsled.devid);//获得次设备号
    }
    
    dtsled.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&dtsled.cdev,&dtsled_fops);//初始化cdev

    cdev_add(&dtsled.cdev,dtsled.devid,DTSLED_CNT);//增加一个cdev

    /*创建类*/
    dtsled.class = class_create(THIS_MODULE,DTSLED_NAME);
    /*创建设备*/
    dtsled.device = device_create(dtsled.class,NULL,dtsled.devid,NULL,DTSLED_NAME);

    return 0;
}

/*
* @description : 驱动出口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static void __exit led_exit(void)
{
    /*取消映射*/
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
    /*注销字符设备*/
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid,DTSLED_CNT);
    device_destroy(dtsled.class,dtsled.devid);
    class_destroy(dtsled.class);


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