#include <linux/module.h>
#include <linux/init.h>

static int __init test_init(void) 
{
    printk("Hello my module\n");
    return 0;
}

static void __exit test_exit(void) 
{
    printk("Goodbye my module\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");