struct class *class;/*类*/
struct device *device;/*设备*/
dev_t devid;/*设备号*/

struct test_dev
{
    dev_t devid; /* 设备号 */
    struct cdev cdev; /* cdev */
    struct class *class; /* 类 */
    struct device *device; /* 设备 */
    int major; /* 主设备号 */
    int minor; /* 次设备号 */
};

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