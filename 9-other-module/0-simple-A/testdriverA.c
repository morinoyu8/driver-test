#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// デバイス名
#define DEVICE_NAME "testdeviceA"

void testdeviceA_func(void)
{
    printk("testdeviceA_func\n");
}
// カーネルのシンボルテーブルに登録する
// 他のカーネルモジュールから呼べるようにする
EXPORT_SYMBOL(testdeviceA_func);


static int testdeviceA_init(void)
{
    printk("testdeviceA_init\n");
    testdeviceA_func();
    return 0;
}

static void testdeviceA_exit(void)
{
    printk("testdeviceA_exit\n");
}


module_init(testdeviceA_init);
module_exit(testdeviceA_exit);
MODULE_LICENSE("GPL");