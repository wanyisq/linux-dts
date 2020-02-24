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
#include <linux/timer.h>
#include <linux/semaphore.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/io.h>

#define DRIVER_NAME 	"gpio-irq"
#define IMX6UIRQ_CNT	1
#define KEY0VALUE		0x01
#define INVAKEY			0xff
#define KEY_NUM			1


struct irq_keydest
{
	int gpio;
	int irqnum;
	unsigned char value;
	char name[10];
	irqreturn_t (*handler)(int, void *);
};

struct imx6uirq_dev
{
	int major;
	int minor;
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *node;
	atomic_t keyvalue;
	atomic_t releasekey;
	struct timer_list timer;
	struct irq_keydest irq_keydest[KEY_NUM];
	unsigned char curkeynum;
};
struct imx6uirq_dev imx6uirq;


static irqreturn_t key0_handler(int irq, void *dev_id)
{
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)dev_id;

	dev->curkeynum = 0;
	dev->timer.data = (volatile long)dev_id;
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
	return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long arg)
{
	unsigned char value;
	unsigned char num;
	struct irq_keydest *keydest;
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)arg;

	num  = dev->curkeynum;
	keydest = &dev->irq_keydest[num];
	value  = gpio_get_value(keydest->gpio);
	if(value == 0){
		atomic_set(&dev->keyvalue, keydest->value);
	}else{
		atomic_set(&dev->keyvalue, 0x80 | keydest->value);
		atomic_set(&dev->releasekey, 1);
	}
}

static int keyio_init(void)
{
	unsigned char i = 0;
	char name[10];
	int ret = 0;

	imx6uirq.node = of_find_node_by_path("/key");
	if(imx6uirq.node == NULL){
		printk("find node none\n");
		return -EINVAL;
	}else
	{
		printk("fine node\n");
	}
	
	for(i = 0; i < KEY_NUM; i++){
		imx6uirq.irq_keydest[i].gpio = of_get_named_gpio(imx6uirq.node, "key-gpio", i);
		if(imx6uirq.irq_keydest[i].gpio < 0){
			printk("can't get key %d\n", i);
		}
	}

	for(i = 0; i < KEY_NUM; i++){
		memset(imx6uirq.irq_keydest[i].name, 0, sizeof(name));
		sprintf(imx6uirq.irq_keydest[i].name, "KEY%d", i);
		gpio_request(imx6uirq.irq_keydest[i].gpio, name);
		gpio_direction_input(imx6uirq.irq_keydest[i].gpio);
		imx6uirq.irq_keydest[i].irqnum = irq_of_parse_and_map(imx6uirq.node, i);
		printk("key%d: gpio=%d, irqnum=%d\r\n", i, 
								imx6uirq.irq_keydest[i].gpio,
								imx6uirq.irq_keydest[i].irqnum);
	}
	imx6uirq.irq_keydest[0].handler = key0_handler;
	imx6uirq.irq_keydest[0].value = KEY0VALUE;

	for(i = 0; i < KEY_NUM; i++){
		ret = request_irq(imx6uirq.irq_keydest[i].irqnum, 
						  imx6uirq.irq_keydest[i].handler,
						  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
						  imx6uirq.irq_keydest[i].name, &imx6uirq);

		if(ret < 0){
			printk("irq %d request failed!\r\n",
				imx6uirq.irq_keydest[i].irqnum);
			return -EFAULT;
		}
	}

	init_timer(&imx6uirq.timer);
	imx6uirq.timer.function = timer_function;
	return 0;
}
static ssize_t imx6uirq_open(struct inode *node, struct file *filp)
{
	filp->private_data = &imx6uirq;
	return 0;
}

static ssize_t imx6uirq_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int ret = 0;
	unsigned char keyvalue = 0;
	unsigned char releasekey = 0;
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;

	keyvalue = atomic_read(&dev->keyvalue);
	releasekey = atomic_read(&dev->releasekey);

	if(releasekey){
		if(keyvalue & 0x80){
			keyvalue &= 0x80;
			ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
		}else{
			goto data_error;
		}
		atomic_set(&dev->releasekey, 0);
	}else{
		goto data_error;
	}
	return 0;

data_error:
	return -EINVAL;

}


static struct file_operations imx6uirq_fops = {
	.owner = THIS_MODULE,
	.open = imx6uirq_open,
	.read = imx6uirq_read,
};

static int __init imx6uirq_init(void)
{
	if(imx6uirq.major){
		imx6uirq.devid = MKDEV(imx6uirq.major, 0);
		register_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT, DRIVER_NAME);
	}else{
		alloc_chrdev_region(&imx6uirq.devid, 0, IMX6UIRQ_CNT, DRIVER_NAME);
		imx6uirq.major = MAJOR(imx6uirq.devid);
		imx6uirq.minor = MINOR(imx6uirq.devid);
	}
	cdev_init(&imx6uirq.cdev, &imx6uirq_fops);
	cdev_add(&imx6uirq.cdev, imx6uirq.devid, IMX6UIRQ_CNT);
	imx6uirq.class = class_create(THIS_MODULE, DRIVER_NAME);
	if(IS_ERR(imx6uirq.class)){
		return PTR_ERR(imx6uirq.class);
	}
	imx6uirq.device = device_create(imx6uirq.class, NULL, imx6uirq.devid, NULL, DRIVER_NAME);
	if(IS_ERR(imx6uirq.device)){
		return PTR_ERR(imx6uirq.device);
	}

	atomic_set(&imx6uirq.keyvalue, INVAKEY);
	atomic_set(&imx6uirq.releasekey, 0);
	keyio_init();
	return 0;
}

static void __exit imx6uirq_exit(void)
{
	unsigned i = 0;
	del_timer_sync(&imx6uirq.timer);

	for(i = 0; i < KEY_NUM; i++){
		free_irq(imx6uirq.irq_keydest[i].irqnum, &imx6uirq);
	}

	cdev_del(&imx6uirq.cdev);
	unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
	device_destroy(imx6uirq.class, imx6uirq.devid);
	class_destroy(imx6uirq.class);
}

module_init(imx6uirq_init);
module_exit(imx6uirq_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("wanyisq");
MODULE_DESCRIPTION("imx6ull tinterrupt");
