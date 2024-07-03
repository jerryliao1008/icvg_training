#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/hwmon.h>

struct device *idevice = NULL;

static const struct class ihwmon_class = {
	.name = "ihwmon"
};

static umode_t ihwmon_is_visible(const void *data,
				   enum hwmon_sensor_types type,
				   u32 attr, int channel)
{
	return 0644;
}

static int ihwmon_read(struct device *dev, enum hwmon_sensor_types type,
		      u32 attr, int channel, long *val)
{
	switch (type) {
	case hwmon_temp:
		switch (attr) {
		case hwmon_temp_input:
			*val = ((channel + 1) * 10);
			printk(KERN_INFO "current temp from CH%d, value: %ld\n", channel, *val);
			break;
		case hwmon_temp_max:
			*val = ((channel + 1) * 10 + 100);
			printk(KERN_INFO "max temp from CH%d, value: %ld\n", channel, *val);
			break;
		case hwmon_temp_fault:
			*val = -1;
			printk(KERN_INFO "temp fault from CH%d, value: %ld\n", channel, *val);
			break;
		default:
			return -EINVAL;
		}
		break;
	case hwmon_in:
		/* Leave it to you to practice */
		printk(KERN_INFO "to be implemented\n");
		*val = 0;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int ihwmon_write(struct device *dev, enum hwmon_sensor_types type,
		       u32 attr, int channel, long val)
{
	printk(KERN_INFO "%s\n", __func__);
	
	return 0;
}

static const u32 ihwmon_temp_config[] = {
	HWMON_T_INPUT | HWMON_T_MAX,
	HWMON_T_INPUT | HWMON_T_MAX | HWMON_T_FAULT,
	0
};

static const struct hwmon_channel_info ihwmon_temp = {
	.type = hwmon_temp,
	.config = ihwmon_temp_config,
};

static const u32 ihwmon_in_config[] = {
	HWMON_I_INPUT | HWMON_I_MIN | HWMON_I_MAX,
	HWMON_I_INPUT | HWMON_I_MIN | HWMON_I_MAX,
	HWMON_I_INPUT | HWMON_I_MIN | HWMON_I_MAX,
	0
};

static const struct hwmon_channel_info ihwmon_in = {
	.type = hwmon_in,
	.config = ihwmon_in_config,
};

static const struct hwmon_channel_info * const ihwmon_info[] = {
	&ihwmon_temp,
	&ihwmon_in,
	NULL
};

static const struct hwmon_ops ihwmon_ops = {
	.is_visible = ihwmon_is_visible,
	.read = ihwmon_read,
	.write = ihwmon_write,
};

static const struct hwmon_chip_info ihwmon_chip_info = {
	.ops = &ihwmon_ops,
	.info = ihwmon_info,
};

static int hwmon_init(struct device *dev)
{
	struct device *hwmon_dev;

	hwmon_dev = devm_hwmon_device_register_with_info(dev, "ihwmon", NULL,
							&ihwmon_chip_info, NULL);
	
	return PTR_ERR_OR_ZERO(hwmon_dev);
}

static int __init ihwmon_init(void)
{
	int err;

	printk(KERN_INFO "%s: module init\n", __func__);

	err = class_register(&ihwmon_class);
	if (err) {
		printk(KERN_ERR "%s: unable to create class\n", __func__);
		goto out_class;
	}

	idevice = device_create(&ihwmon_class, NULL, MKDEV(0, 1), NULL, "idevice");
	if (IS_ERR(idevice)) {
		printk(KERN_ERR "%s: unable to create device\n", __func__);
		err = PTR_ERR(idevice);
		goto out_device;
	}

	err = hwmon_init(idevice);
	if (err) {
		printk(KERN_ERR "%s: failed to init hwmon, err: %d\n", __func__, err);
		goto out_hwmon;
	}

	return 0;

out_hwmon:
	device_destroy(&ihwmon_class, MKDEV(0, 1));
out_device:
	class_unregister(&ihwmon_class);
out_class:
	return err;
}

static void __exit ihwmon_exit(void)
{
	printk(KERN_INFO "%s: module exit\n", __func__);
	
	device_destroy(&ihwmon_class, MKDEV(0, 1));
	class_unregister(&ihwmon_class);
}

module_init(ihwmon_init);
module_exit(ihwmon_exit);

MODULE_AUTHOR("Advantech ICVG SW");
MODULE_DESCRIPTION("Advantech Hardware Monitor testing driver");
MODULE_LICENSE("GPL");
