#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

static struct cdev mydev_cdev;
static int major;
static struct class *mydev_class;
static dev_t dev;

static const struct file_operations mydev_fops = { .owner = THIS_MODULE };

static int __init mydev_init(void) 
{
    int ret = alloc_chrdev_region(&dev, 0, 1, "mydev");
    if (ret < 0)
        return ret;
    major = MAJOR(dev);
    cdev_init(&mydev_cdev, &mydev_fops);
    cdev_add(&mydev_cdev, dev, 1);
    mydev_class = class_create("mydev");
    device_create(mydev_class, NULL, dev, NULL, "mydev");
    return 0;
}

static void __exit mydev_exit(void)
{
    device_destroy(mydev_class, MKDEV(major, 0));
    class_destroy(mydev_class);
    cdev_del(&mydev_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple character device example");
MODULE_AUTHOR("Maksym");
