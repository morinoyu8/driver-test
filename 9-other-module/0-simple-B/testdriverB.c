#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// デバイス名
#define DEVICE_NAME "testdeviceB"

extern void testdeviceA_func(void);

static int testdeviceB_init(void)
{
    printk("testdeviceB_init\n");

    testdeviceA_func();
    return 0;
}

static void testdeviceB_exit(void)
{
    printk("testdeviceB_exit\n");
}


module_init(testdeviceB_init);
module_exit(testdeviceB_exit);
MODULE_LICENSE("GPL");