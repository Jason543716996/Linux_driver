/*************************************************************************
  @Author: Jason
  @Created Time : Sun 20 Oct 2019 08:51:29 PM CST
  @File Name: key.c
  @Description:
 ************************************************************************/
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

#define KEY_CNT   1

static int  major;
static struct cdev  key_cdev;   //内核中用cdev描述一个字符设备
static struct class *cls = NULL;
static struct device *device = NULL;
static int key1,key2,key3,key4;

static ssize_t key_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	unsigned char key_vals[4];

	int minor = iminor(file->f_inode);

	key_vals[0] = gpio_get_value(key1);
	key_vals[1] = gpio_get_value(key2);
	key_vals[2] = gpio_get_value(key3);
	key_vals[3] = gpio_get_value(key4);
	copy_to_user(user_buf, key_vals, sizeof(key_vals));

	return 1;
}
static int key_open(struct inode *inode, struct file *file)
{
	printk("key_open\n");
	return 0;
}

static struct file_operations key_fops = {
	.owner = THIS_MODULE,
	.open  = key_open,
	.read = key_read,
};

static int key_probe(struct platform_device *pdev) {

	struct device *dev = &pdev->dev;
	dev_t devid;
	struct pinctrl *pctrl;
	struct pinctrl_state *pstate;
	pctrl = devm_pinctrl_get(dev);
	if(pctrl == NULL)
	{
		printk("devm_pinctrl_get error\n");
	}
	pstate = pinctrl_lookup_state(pctrl, "key_demo");
	if(pstate == NULL)
	{
		printk("pinctrl_lookup_state error\n");
	}
	pinctrl_select_state(pctrl, pstate);//设置为输出模式 
	printk("enter %s\n",__func__);
	key1 = of_get_named_gpio(dev->of_node, "tiny4412,int_gpio1", 0);;
	key2 = of_get_named_gpio(dev->of_node, "tiny4412,int_gpio2", 0);;
	key3 = of_get_named_gpio(dev->of_node, "tiny4412,int_gpio3", 0);;
	key4 = of_get_named_gpio(dev->of_node, "tiny4412,int_gpio4", 0);;
	if(key1 <= 0)
	{
		printk("%s error\n",__func__);
		return -EINVAL;
	}
	else
	{
		printk("key1 %d\n",key1);
		printk("key2 %d\n",key2);
		printk("key3 %d\n",key3);
		printk("key4 %d\n",key4);
		devm_gpio_request_one(dev, key1, GPIOF_OUT_INIT_HIGH, "LED1");
		devm_gpio_request_one(dev, key2, GPIOF_OUT_INIT_HIGH, "LED2");
		devm_gpio_request_one(dev, key3, GPIOF_OUT_INIT_HIGH, "LED3");
		devm_gpio_request_one(dev, key4, GPIOF_OUT_INIT_HIGH, "LED4");
	}

	if(alloc_chrdev_region(&devid, 0, KEY_CNT, "key") < 0)/* (major,0~1) 对应 hello_fops, (major, 2~255)都不对应hello_fops */
	{
		printk("%s ERROR\n",__func__);
		goto error;
	}
	major = MAJOR(devid);                     

	cdev_init(&key_cdev, &key_fops);        //绑定文件操作函数
	cdev_add(&key_cdev, devid, KEY_CNT);    //注册到内核

	cls = class_create(THIS_MODULE, "key"); //创建key类,向类中添加设备,mdev会帮我们创建设备节点
	device_create(cls, NULL, devid, NULL, "keys"); 
	return 0;
error:
	unregister_chrdev_region(MKDEV(major, 0),KEY_CNT);
	return 0;
}

static int key_remove(struct platform_device *pdev) {

	printk("enter %s\n",__func__);
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);

	cdev_del(&key_cdev);
	unregister_chrdev_region(MKDEV(major, 0), KEY_CNT);

	printk("%s enter.\n", __func__);
	return 0;
}

static const struct of_device_id key_dt_ids[] = {
	{ .compatible = "tiny4412,key_demo", },
	{},
};

MODULE_DEVICE_TABLE(of, key_dt_ids);

static struct platform_driver key_driver = {
	.driver        = {
		.name      = "key_demo",
		.of_match_table    = of_match_ptr(key_dt_ids),
	},
	.probe         = key_probe,
	.remove        = key_remove,
};

static int key_init(void){
	int ret;
	printk("enter %s\n",__func__);
	ret = platform_driver_register(&key_driver);
	if (ret)
		printk(KERN_ERR "key demo: probe faikey: %d\n", ret);

	return ret; 
}

static void key_exit(void)
{
	printk("enter %s\n",__func__);
	platform_driver_unregister(&key_driver);
}

module_init(key_init);
module_exit(key_exit);

MODULE_AUTHOR("Jason <jason_linuxc@163.com>");
MODULE_LICENSE("GPL");

