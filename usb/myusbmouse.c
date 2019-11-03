#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

//usb_interface表示逻辑上的设备，比如录音和播放，就会有两个设备
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("found usbmouse!\n");
	return 0;
}
       
static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	printk("disconnect usbmouse!\n");
}

static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	//{USB_DEVICE(vend, prod)},
	{ }	/* Terminating entry */
}; 
//分配设置
static struct usb_driver usbmouse_as_key_driver = {
        .name           = "usbmouse_as_key",
        .probe          = usbmouse_as_key_probe,
        .disconnect     = usbmouse_as_key_disconnect,
        .id_table       = usbmouse_as_key_id_table,
};



module_usb_driver(usbmouse_as_key_driver);
MODULE_AUTHOR("Jason <jason_linuxc@163.com>");
MODULE_LICENSE("GPL");