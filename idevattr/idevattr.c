#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/device.h>

struct device *idevice = NULL;
unsigned char ivalue = 0;

static const struct class idevice_class = {
	.name = "idevice"
};

static ssize_t misc_show(struct device *dev, struct device_attribute *da, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ivalue);
}

static ssize_t misc_store(struct device *dev, struct device_attribute *da,
							const char *buf, size_t count)
{
	ivalue = simple_strtol(buf, NULL, 10);

	return count;
}

static DEVICE_ATTR(misc, 0664, misc_show, misc_store);

static int __init idevattr_init(void)
{
	int err;

	printk(KERN_INFO "%s: module init\n", __func__);

	err = class_register(&idevice_class);
	if (err) {
		printk(KERN_ERR "%s: unable to create class\n", __func__);
		goto out_class;
	}

	idevice = device_create(&idevice_class, NULL, MKDEV(0, 0), NULL, "idevice");
	if (IS_ERR(idevice)) {
		printk(KERN_ERR "%s: unable to create device\n", __func__);
		err = PTR_ERR(idevice);
		goto out_device;
	}
	
	err = device_create_file(idevice, &dev_attr_misc);
	if (err) {
		printk(KERN_ERR "%s: unable to create device attribue\n", __func__);
		goto out_devattr;
	}

	return 0;

out_devattr:
	device_destroy(&idevice_class, MKDEV(0, 0));
out_device:
	class_unregister(&idevice_class);
out_class:
	return err;
}

static void __exit idevattr_exit(void)
{
	printk(KERN_INFO "%s: module exit\n", __func__);
	
	device_remove_file(idevice, &dev_attr_misc);
	device_destroy(&idevice_class, MKDEV(0, 0));
	class_unregister(&idevice_class);
}

module_init(idevattr_init);
module_exit(idevattr_exit);

MODULE_AUTHOR("Advantech ICVG SW");
MODULE_DESCRIPTION("Advantech Device Attribute testing driver");
MODULE_LICENSE("GPL");
