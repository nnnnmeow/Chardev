#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>

static struct cdev mydev_cdev;
static int major;
static struct class *mydev_class;
static dev_t dev;
static char *buffer;
static size_t buffer_used;
static DEFINE_MUTEX(buffer_lock);

#define BUFFER_SIZE 1024
#define MYDEV_IOC_MAGIC 'k'
#define MYDEV_IOC_CLEAR _IO(MYDEV_IOC_MAGIC, 0)

static int mydev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int mydev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t mydev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret;
    mutex_lock(&buffer_lock);
    if (*ppos >= buffer_used) {
        ret = 0;
        goto out;
    }
    if (*ppos + count > buffer_used)
        count = buffer_used - *ppos;
    if (copy_to_user(buf, buffer + *ppos, count) != 0) {
        ret = -EFAULT;
        goto out;
    }

    *ppos += count;
    ret = count;
    out:
        mutex_unlock(&buffer_lock); 
        return ret;
}

static ssize_t mydev_write(struct file *file,const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret;
    mutex_lock(&buffer_lock);
    if (count > BUFFER_SIZE) 
        count = BUFFER_SIZE;
    if (copy_from_user(buffer, buf, count) != 0) {
        ret = -EFAULT;
        goto out;
    }

    buffer_used = count;
    *ppos += count;
    ret = count;
    out:
        mutex_unlock(&buffer_lock);
        return ret;
}

static long mydev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case MYDEV_IOC_CLEAR:
            mutex_lock(&buffer_lock);
            memset(buffer, 0, BUFFER_SIZE);
            buffer_used = 0;
            mutex_unlock(&buffer_lock);
            return 0;
        default:
            return -ENOTTY;
    }
}

static const struct file_operations mydev_fops = { 
    .owner = THIS_MODULE,
    .read = mydev_read,
    .write = mydev_write,
    .open = mydev_open,
    .release = mydev_release,
    .unlocked_ioctl = mydev_ioctl
};

static int __init mydev_init(void) {
    buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (buffer == NULL)
        return -ENOMEM;
    int ret = alloc_chrdev_region(&dev, 0, 1, "mydev");
    if (ret < 0)
        return ret;
    major = MAJOR(dev);
    pr_info("mydev: loaded, major: %d\n", major);
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
