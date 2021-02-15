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
#include <linux/miscdevice.h>

#define MISCBEEP_NAME   "miscbeep"
#define MISCBEEP_MINOR  144
#define BEEPON  1
#define BEEPOFF 0

struct miscbeep_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *node;
    int beep_gpio;
};
struct miscbeep_dev beep_dev;

static int beep_open(struct inode *node, struct file *flip)
{
    flip->private_data = &beep_dev;
    return 0;
}

static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
    unsigned char databuff[1];
    unsigned char beepstate;
    struct  miscbeep_dev *dev = filp->private_data;

    if(dev == NULL){
        printk("beep private data is null\r\n");
    }


    ret = copy_from_user(databuff, buf, cnt);
    if(ret < 0){
        printk("kernel write failed\r\n");
        return 1;
    }

    beepstate = databuff[0];
    if(beepstate == BEEPOFF){
        gpio_set_value(dev->beep_gpio, 1);
    }else if(beepstate == BEEPON){
        gpio_set_value(dev->beep_gpio, 0);
    }
    return 0;
}

static struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
};

static struct miscdevice beep_miscdev = {
    .minor = MISCBEEP_MINOR,
    .name = MISCBEEP_NAME,
    .fops = &beep_fops,
};

//设备树中的节点完全替代了platform_device,两者都是描述设备硬件信息
//参考https://blog.csdn.net/zqixiao_09/article/details/50889458
static int beep_probe(struct platform_device *pdev)
{
    int ret;
    
    printk("beep device and driver matched\r\n");
    
    // struct device_node *node = pdev->dev.of_node;
    // beep_dev.beep_gpio = of_get_named_gpio(node, "beep-gpio", 0);
    beep_dev.node = of_find_node_by_path("/beep");
    //以上两种获得node的方法不同

    if(beep_dev.node == NULL){
        printk("beep node cant find\r\n");
        return -EINVAL;
    }
    beep_dev.beep_gpio = of_get_named_gpio(beep_dev.node, "beep-gpio", 0);
    if(beep_dev.beep_gpio < 0){
        printk("beep gpio cant find\r\n");
        return -EINVAL;
    }
    ret = gpio_direction_output(beep_dev.beep_gpio, 1);
    if(ret < 0){
        printk("beep gpio cant set\r\n");
        return -EINVAL;
    }

    ret = misc_register(&beep_miscdev);
    if(ret < 0){
        printk("beep device cant register\r\n");
        return -EINVAL;
    }
    return 0;
}

static int beep_remove(struct platform_device *dev)
{
    gpio_set_value(beep_dev.beep_gpio, 1);
    misc_deregister(&beep_miscdev);
    return 0;
}

static const struct of_device_id beep_of_match[] = {
    {.compatible = "beep"},
    {}
};

static struct platform_driver beep_driver = {
    .driver = {
        .name = "imx6ull-beep",
        .of_match_table = beep_of_match,
    },
    .probe = beep_probe,
    .remove = beep_remove,
};

static int __init leddriver_init(void)
{
    return platform_driver_register(&beep_driver);
}

static void __exit leddriver_exit(void)
{
    platform_driver_unregister(&beep_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanyisq");
