
/*打开设备*/
static int chrtest_open(struct inode *inode,struct file *filp)
{
    /*用户实现具体功能*/
    return 0;
}

/*从设备读取*/
static ssize_t chrtest_read(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
{
    /*用户实现具体功能*/
    return 0;
}
/*向设备写数据*/
static ssize_t chrtest_write(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
{
    /*用户实现具体功能*/
    return 0;
}
/*关闭/释放设备*/
static int chrtest_release(struct inode *inode,struct file *filp)
{
    /*用户实现具体功能*/
    return 0;
}

static struct  file_operations test_fops ={
    .owner = THIS_MODULE,
    .open = chrtest_open,
    .read = chrtest_read,
    .write = chrtest_write,
    .release = chrtest_release,
};

/*驱动入口函数*/
static int __init xxx_init(void)
{
    /*入口函数具体内容*/
    int retvalue = 0;
    /*注册字符设备驱动*/
    retvalue = register_chrdev(200,"chrtest",&test_fops);
    if(retvalue <0)
    {
        /*字符设备注册失败*/
    }
    return 0;
}

/*驱动出口函数*/
static void __exit xxx_exit(void)
{
    /*注销字符设备驱动*/
    unregister_chrdev(200,"chrtest");
}

/*将上述函数指定为驱动的入口和出口函数*/
module_init(xxx_init);
module_exit(xxx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LanYuanA");