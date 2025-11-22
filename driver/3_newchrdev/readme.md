# 新字符设备驱动

使用register_chrdev函数注册字符设备的时候 需要我们知道哪个主设备号未使用 并且次设备号浪费 可以用一下函数来申请设备号

> int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)

若给定设备的主设备号和次设备号就使用一下函数

> int register_chrdev_region(dev_t from, unsigned count, const char *name)

from是起始的设备号 count是要申请的数量 name是设备名字

统一使用以下的释放函数

> void unregister_chrdev_region(dev_t from, unsigned count)

```c
int major;  /*主设备号*/
int minor;  /*次设备号*/
dev_t devid;    /*设备号*/

if(major)
{
    devid = MKDEV(major,0);/*大部分驱动次设备号都选择0*/
    register_chrdev_region(devid,1,"test");/*申请设备号*/
}
else    /*没有设置设备号*/
{
    alloc_chrdev_region(&devid,0,1,"test");/*申请设备号*/
    major = MAJOR(devid);/*获得主设备号*/
    minor = MINOR(devid);/*获得次设备号*/
}

/***************/
unregister_chrdev_region(devid,1);
```

如果指定major就使用MKDEV来构成dev_t类型的数据 次设备号都选择0 约定俗成的规定

---

## 新的字符设备注册方法

```c
struct cdev 
{
    struct kobject kobj;
    struct module *owner;
    const struct file_operations *ops;
    struct list_head list;
    dev_t dev;
    unsigned int count;
};
/*ops 是file_operations字符设备文件操作函数；dev就是设备号*/

struct cdev testcdev;

 /* 设备操作函数 */
static struct file_operations test_fops = {
    .owner = THIS_MODULE,
    /* 其他具体的初始项 */
    };

/*void cdev_init(struct cdev *cdev, const struct file_operations *fops)原型*/

testcdev.owner = THIS_MODULE;
cdev_init(&testcdev, &test_fops); /* 初始化 cdev 结构体变量 */

/*int cdev_add(struct cdev *p, dev_t dev, unsigned count)原型 用于增加字符设备*/
cdev_add(&testcdev, devid, 1); /* 添加字符设备 */

/*  卸载驱动一定要使用cdev_del删除对应的字符设备
    原型：void cdev_del(struct cdev *p)
*/
cdev_del(&testcdev); /* 删除 cdev */

/*  分配设备号 加上上述代码就是register_chrdev的功能*/

/* cdev_del和unregister_chrdev_region合起来就是unregister_chrdev函数*/
```

---

## 自动创建设备节点

udev是一个用户程序 用于实现设备文件的创建和删除 可以检测硬件设备的状态

自动创建设备节点是在驱动的入口函数完成的 一般在cdev_add函数后增加相关代码

```c
#define class_create(owner, name)        \
({                                       \
    static struct lock_class_key __key;  \
    __class_create(owner, name, &__key); \
})

struct class *__class_create(struct module *owner, 
                             const char *name,
                             struct lock_class_key *key)
```

展开后内容如下

> struct class *class_create (struct module *owner, const char *name)

owner一般为THIS_MODULE name是类名字 返回值是一个指向结构体class的指针 也就是创建的类

卸载驱动的时候需要删除类 class_destroy 原型如下：

> void class_destroy(struct class *cls)

在创建好类之后需要创建一个设备 使用device_create函数 原型如下下：

```c
/* 
    class:要创建的类下面
    parent:父设备 一般为NULL
    devt:设备号
    drvdata:设备可能使用到的数据 一般为NULL
    fmt: 设备名 如果设置fmt=xxx 就会生成/dev/xxx
    返回值：创建好的设备

*/

struct device *device_create(struct class *class,
                            struct device *parent,
                            dev_t devt,
                            void *drvdata,
                            const char *fmt, ...)
/*删除创建的设备*/
void device_destroy(struct class *class, dev_t devt)            
```

根据上述代码 在驱动入口处创建类和设备 在驱动出口处删除类和设备 参开示例如下：

```c
struct class *class;/*类*/
struct device *device;/*设备*/
dev_t devid;/*设备号*/

/*驱动入口函数*/
static int __init led_init(void)
{
    /*创建类*/
    class = class_create(THIS_MODULE,"led");
    /*创建设备*/
    device = device_create(class,NULL,devid,NULL,"led");
    return 0;
}
/*驱动出口函数*/
static int __exit led_exit(void)
{
    /*删除设备*/
    device_destroy(newchrled.class,newchrled.devid);
    /*删除类*/
    class_destroy(newchrled.class);
}

module_init(led_init);
module_exit(led_exit);
```

每个硬件设备都有属性 在编写驱动的时候可以将这些属性全部写成变量的形式 

```c
struct test_dev
{
    dev_t devid; /* 设备号 */
    struct cdev cdev; /* cdev */
    struct class *class; /* 类 */
    struct device *device; /* 设备 */
    int major; /* 主设备号 */
    int minor; /* 次设备号 */
};

struct test_dev test;
/*open函数*/
static int test_open(struct inode *inode,struct file *filp)
{
    file->private_data = &test;/*设置私有数据*/
    return 0;
}
```

完整实例如下：

```c
/*
* @description : 驱动入口函数
* @param：       无
* @return : 0 成功;其他 失败
*/
static int __init led_init(void)
{
    /*入口函数具体内容*/
    u32 val = 0;
    /*初始化LED*/
    /*1.寄存器地址映射*/
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE,4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE,4);

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
    if(newchrled.major)//指定了major
    {
        newchrled.devid = MKDEV(newchrled.major,0);//次设备号选择0
        register_chrdev_region(newchrled.devid,NEWCHRLED_CNT,NEWCHRLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&newchrled.devid,0,NEWCHRLED_CNT,NEWCHRLED_NAME);//申请设备号
        newchrled.major = MAJOR(newchrled.devid);//获得主设备号
        newchrled.minor = MINOR(newchrled.devid);//获得次设备号
    }
    
    newchrled.cdev.owner = THIS_MODULE;//设置cdev.owner属性
    cdev_init(&newchrled.cdev,&newchrled_fops);//初始化cdev

    cdev_add(&newchrled.cdev,newchrled.devid,NEWCHRLED_CNT);//增加一个cdev

    /*创建类*/
    newchrled.class = class_create(THIS_MODULE,NEWCHRLED_NAME);
    /*创建设备*/
    newchrled.device = device_create(newchrled.class,NULL,newchrled.devid,NULL,NEWCHRLED_NAME);

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
    cdev_del(&newchrled.cdev);
    unregister_chrdev_region(newchrled.devid,NEWCHRLED_CNT);
    device_destroy(newchrled.class,newchrled.devid);
    class_destroy(newchrled.class);


    printk("chrdevbase_exit()\r\n");
}
```

其流程为：

在入口函数里：

- 初始化GPIO
- 注册字符设备
    
    - 申请devid: register_chrdev_region或alloc_chrdev_region
    -  配置cdev:设置cdev.owner=THIS_MODULE 使用cdev_init和cdev_add
    - 创建类class_create
    - 创建设备device_create

在出口函数里：

- 取消映射 iounmap
- 注销字符设备

    - cdev_del(&cdev)
    - unregister_chrdev_region(devid,cnt)
    - device_destroy(class,devid)
    - class_destroy(class)

此外在chrdev_open函数里将传入的filp->private_data设置私有数据