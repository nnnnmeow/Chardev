#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init mydev_init(void) 
{
    pr_info("mydev: hello\n");
    return 0;
}

static void __exit mydev_exit(void)
{
    pr_info("mydev: bye\n");
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple character device example");
MODULE_AUTHOR("Maksym");
