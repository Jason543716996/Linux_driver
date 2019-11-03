#include <linux/module.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h> 
#include <linux/wait.h>
#include <linux/sched.h>
#define KEY_CNT   4

static int  major;
static struct cdev  key_cdev;   //内核中用cdev描述一个字符设备
static struct class *cls = NULL;
unsigned char key_val;


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;
static struct fasync_struct *button_async;

typedef struct 
{
	int gpio;
	int irq;
	int key_val;
	char name[20];
}int_demo_data_t;

static int key_open(struct inode *inode, struct file *file)
{       
        printk("key_open\n");
        return 0;
}

static ssize_t key_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{       
        
        int minor = iminor(file->f_inode);
        wait_event_interruptible(button_waitq, ev_press);
	
        copy_to_user(user_buf, &key_val,1);
        ev_press = 0;
        
        return 1;
}

static unsigned key_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}
static int key_fasync (int fd, struct file *filp, int on)
{
	printk("driver: fifth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}

static struct file_operations key_fops = {
        .owner = THIS_MODULE,
        .open  = key_open,
        .read  = key_read,
		.poll  = key_poll, 
		.fasync = key_fasync,
};

static irqreturn_t int_demo_isr(int irq, void *dev_id)
{
	unsigned int pinval;
	int_demo_data_t *data = dev_id;
//	printk("irq = %d",irq);
//	printk("%s enter, %s: gpio:%d, irq: %d\n", __func__, data->name, data->gpio, data->irq);
	pinval = gpio_get_value(data->gpio);
	
	if (pinval)
	{
		/* 松开 */
		key_val = 0x80|data->key_val;
	}
	else
	{
      	/* 按下 */
      		key_val = data->key_val;
	}	

	ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int int_demo_probe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;
	int irq_gpio = -1;
	int irq = -1;
	int ret = 0;
	int i = 0;
	dev_t devid;
	int_demo_data_t *data = NULL;
	printk("%s enter.\n", __func__);

	if(alloc_chrdev_region(&devid, 0, KEY_CNT, "key") < 0)/* (major,0~1) 对应 hello_fops, (major, 2~255)都不对应hello_fops */
        {
                printk("%s ERROR\n",__func__);
        }
        major = MAJOR(devid);
	cdev_init(&key_cdev, &key_fops);        //绑定文件操作函数
        cdev_add(&key_cdev, devid, KEY_CNT);    //注册到内核

        cls = class_create(THIS_MODULE, "key"); //创建key类,向类中添加设备,mdev会帮我们创建设备节点
        device_create(cls, NULL, devid, NULL, "keys");

	if (!dev->of_node) {
		dev_err(dev, "no platform data.\n");
		goto err1;
	}
	data = devm_kmalloc(dev, sizeof(*data)*4, GFP_KERNEL);
	if (!data) {
		dev_err(dev, "no memory.\n");
		goto err0;  
	}
	for (i = 3; i >= 0; i--) {
		sprintf(data[i].name, "tiny4412,int_gpio%d", i+1);
		irq_gpio = of_get_named_gpio(dev->of_node, data[i].name, 0);//通过名字获取gpio
		if (irq_gpio < 0) {
			dev_err(dev, "Looking up %s property in node %s failed %d\n",
					data[i].name, dev->of_node->full_name, irq_gpio);
			goto err1;
		}
		data[i].gpio = irq_gpio;
		data[i].key_val = i+1;
		irq = gpio_to_irq(irq_gpio);    //将gpio转换成对应的中断号
		if (irq < 0) {
			dev_err(dev,
					"Unable to get irq number for GPIO %d, error %d\n",
					irq_gpio, irq);
			goto err1;
		}
		data[i].irq = irq;
		printk("%s: gpio: %d ---> irq (%d)\n", __func__, irq_gpio, irq);
		//注册中断
		ret = devm_request_any_context_irq(dev, irq, int_demo_isr, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, data[i].name, data+i);
		if (ret < 0) {
			dev_err(dev, "Unable to claim irq %d; error %d\n",
					irq, ret);
			goto err1;
		}
	}
	return 0;
err1:
	devm_kfree(dev, data);
        unregister_chrdev_region(MKDEV(major, 0),KEY_CNT);
        return 0;

err0:
	return -EINVAL;
}

static int int_demo_remove(struct platform_device *pdev) 
{
	printk("enter %s\n",__func__);
        device_destroy(cls, MKDEV(major, 0));
        class_destroy(cls);
        cdev_del(&key_cdev);
        unregister_chrdev_region(MKDEV(major, 0), KEY_CNT);

	return 0;
}

static const struct of_device_id int_demo_dt_ids[] = { 
	{ .compatible = "tiny4412,interrupt_demo", },  
	{},
};


MODULE_DEVICE_TABLE(of, int_demo_dt_ids);
static struct platform_driver int_demo_driver = {  
	.driver        = {      
		.name      = "interrupt_demo",      
		.of_match_table    = of_match_ptr(int_demo_dt_ids),  
	},  
	.probe         = int_demo_probe,  
	.remove        = int_demo_remove,

};



static int __init int_demo_init(void)
{  int ret;
	ret = platform_driver_register(&int_demo_driver);  
	if (ret)      
		printk(KERN_ERR "int demo: probe failed: %d\n", ret);
	return ret;
}
module_init(int_demo_init);

static void __exit int_demo_exit(void)
{
	platform_driver_unregister(&int_demo_driver);
}
module_exit(int_demo_exit);



MODULE_LICENSE("GPL");

