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
static char *buffer;
static size_t buffer_used;

#define BUFFER_SIZE 1024

static int mydev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int mydev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t mydev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    if (*ppos >= buffer_used)
        return 0;
    if (*ppos + count > buffer_used)
        count = buffer_used - *ppos;
    if (copy_to_user(buf, buffer + *ppos, count) != 0)
        return -EFAULT;

    *ppos += count;
    return count;
}

static ssize_t mydev_write(struct file *file,const char __user *buf, size_t count, loff_t *ppos) {
    if (count > BUFFER_SIZE) 
        count -= BUFFER_SIZE;
    if (copy_from_user(buffer, buf, count) != 0)
        return -EFAULT;

    buffer_used = count;
    *ppos += count;
    return count;
}

static const struct file_operations mydev_fops = { 
    .owner = THIS_MODULE,
    .read = mydev_read,
    .write = mydev_write,
    .open = mydev_open,
    .release = mydev_release    
};

static int __init mydev_init(void) {
    pr_info("mydev: loaded, major: %d\n", major);
    buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (buffer == NULL)
        return -ENOMEM;
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

static void __exit mydev_exit(void) {
    pr_info("mydev: unloaded\n");
    device_destroy(mydev_class, MKDEV(major, 0));
    class_destroy(mydev_class);
    cdev_del(&mydev_cdev);
    unregister_chrdev_region(dev, 1);
    kfree(buffer);
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple character device example");
MODULE_AUTHOR("Maksym");
