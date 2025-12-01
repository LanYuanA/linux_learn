struct timer_list timer;/*定义一个计时器*/

void function(unsigned long arg)
{
    /*
    *   定时器处理代码
    */

    /*
    *   若需要用到周期性运行就要mod_timer重新设置超市值并启用定时器
    */
   mod_timer(&dev->timertest,jiffies + msecs_to_jiffies(2000));
}

/* 初始化函数 */
void init(void)
{
    init_timer(&timer);/*初始化定时器*/

    timer.function = function;
    timer.expires = jiffies + msecs_to_jiffies(2000);/*超时时间2s*/
    time.data = (unsigned long)&dev;/*把设备结构体作为参数传入*/
    add_timer(&timer);/*启用定时器*/
}
void exit(void)
{
    del_timer(&timer);
    //或者
    del_timer_sync(&timer);
}