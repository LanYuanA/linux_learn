# 关于修改设备树并更新的操作

```c
make dtbs//编译bts文件

/*以下指令在uboot里使用*/
tftp 83000000 imx6ull-alientek-emmc.dtb //从网络中下载设备树
fatwrite mmc 1:1 83000000 imx6ull-alientek-emmc.dtb 0x97c7//将文件写入emmc分区 后面参数为文件大小
fatls mmc 1:1//查看文件

// 从eMMC加载内核
fatload mmc 1:1 80800000 zImage
fatload mmc 1:1 83000000 imx6ull-alientek-emmc.dtb
bootz 80800000 - 83000000
```