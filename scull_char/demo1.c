#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVNAME "scull"

#define CNT 66

/*
 	1. 一套驱动支持多个设备
		
		必须实现open, 思考为什么? 要做什么?
		
	2. 字符设备驱动的第二种注册方式

		1> 注册申请设备号
			register_chrdev_region();
			alloc_chrdev_region();

			unregister_chrdev_region();

		2> 实例化并注册struct cdev对应的对象，这个对象代表一个设备的字符设备驱动
		cdev_init();
		cdev_add();

	3. 设备文件的自动创建

		class_create();
		device_create();
 */

static struct scull_t {
	char *kbuf;
	size_t length;
	struct cdev cdev;
}sculldev[CNT];

static int major = 0;
static dev_t devnum = 0;

static struct class *class;

static int demo_open (struct inode *inodp, struct file *filp)
{
	filp->private_data = container_of(inodp->i_cdev, struct scull_t, cdev);

	return 0;
}


ssize_t demo_read (struct file *filp, char __user *buf, size_t cnt, loff_t *fpos)
{
	struct scull_t *pdev = filp->private_data;

	size_t min;

	if (cnt == 0) {
		return 0;
	}	

	if (pdev->length == 0) {
		return -EBUSY;
	}

	min = min(cnt, pdev->length);

	if (copy_to_user(buf, pdev->kbuf, min)) {
		return -EIO;
	}
	memmove(pdev->kbuf, pdev->kbuf+min, pdev->length-min);
	pdev->length -= min;

	printk("[kernel]: read %dBytes ok!\n", min);

	return min;
}

ssize_t demo_write (struct file *filp, const char __user *buf, size_t cnt, loff_t *fpos)
{
	struct scull_t *pdev = filp->private_data;
	size_t min;

	if (cnt == 0) {
		return 0;
	}

	if (pdev->length == SZ_512) {
		return -EBUSY;
	}

	min = SZ_512 - pdev->length;
	min = min(cnt, min);

	if (copy_from_user(pdev->kbuf + pdev->length, buf, min)) {
		return -EIO;
	}

	pdev->length += min;

	printk("[kernel]: write %dBytes ok!\n", cnt);

	return min;
}

/*由上层的close调用*/
static int demo_release (struct inode *inodp, struct file *filp)
{
	return 0;	
}

const struct file_operations fops = {
	.owner 		=   	THIS_MODULE,	
	.open		=	demo_open,
	.read  		=   	demo_read,
	.write 		=   	demo_write,
	.release	=	demo_release,
};

static int demo_init(void)
{
	int ret = 0;
	int i;
	struct device *device;

	/*静态或动态申请注册设备号*/
	if (major) {
		devnum = MKDEV(major, 0);
		ret = register_chrdev_region(devnum, CNT, DEVNAME);
	} else {
		ret = alloc_chrdev_region(&devnum, 0, CNT, DEVNAME);
		major = MAJOR(devnum);
	}

	if (ret < 0) {
		goto error0;	
	}


	class = class_create(THIS_MODULE, DEVNAME);
	if (IS_ERR(class)) {
		ret = PTR_ERR(class);
		goto error1;
	}

	for (i = 0; i < CNT; i++) {
		/*初始化每个字符设备驱动对象*/
		cdev_init(&sculldev[i].cdev, &fops);		
		devnum = MKDEV(major, i);
		/*向内核注册字符设备驱动*/
		ret = cdev_add(&sculldev[i].cdev, devnum, 1);
		if (ret < 0) {
			goto error2;
		}
		
		/*设备文件的创建*/
		device = device_create(class, NULL, devnum, NULL, "scull%d", i);
		if (IS_ERR(device)) {
			ret = PTR_ERR(device);
			cdev_del(&sculldev[i].cdev);
			goto error2;
		}

		/*为我们这里的设备分配用于用户读写访问的内存空间*/
		sculldev[i].kbuf = kzalloc(SZ_512, GFP_KERNEL);
		if (NULL == sculldev[i].kbuf) {
			ret = -ENOMEM;
			cdev_del(&sculldev[i].cdev);
			device_destroy(class, devnum);
			goto error2;
		}
		sculldev[i].length = 0;
	}

	return 0;
error2:
	while (i--) {
		kfree(sculldev[i].kbuf);
		devnum = MKDEV(major, i);
		device_destroy(class, devnum);
		cdev_del(&sculldev[i].cdev);
	}
	
	class_destroy(class);
error1:
	devnum = MKDEV(major, 0);
	unregister_chrdev_region(devnum, CNT);
error0:
	return ret;
}

static void demo_exit(void)
{
	int i = ARRAY_SIZE(sculldev);

	while (i--) {
		kfree(sculldev[i].kbuf);
		devnum = MKDEV(major, i);
		device_destroy(class, devnum);
		cdev_del(&sculldev[i].cdev);
	}
	
	class_destroy(class);

	devnum = MKDEV(major, 0);
	unregister_chrdev_region(devnum, CNT);
}

/*此宏用于指定驱动的入口函数，　内核启动或插入模块到内核时被调用*/
module_init(demo_init);
/*此宏用于指定驱动模块输出函数*/
module_exit(demo_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("no name");
MODULE_VERSION("J-15");
MODULE_DESCRIPTION("a simple demo for driver module");
