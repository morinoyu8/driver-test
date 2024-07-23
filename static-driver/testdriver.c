#include <linux/module.h>
#include <linux/init.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "test-driver"
#define DRIVER_MAJOR 63

/* open で呼ばれる関数 */
static int test_open(struct inode *inode, struct file *file)
{
    printk("test_open\n");
    return 0;
}

/* clone で呼ばれる関数 */
static int test_close(struct inode *inode, struct file *file)
{
    printk("test_close\n");
    return 0;
}

/* read で呼ばれる関数 */
static ssize_t test_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("test_read\n");
    buf[0] = 'A';
    return 1;
}

/* write で呼ばれる関数 */
static ssize_t test_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("test_write\n");
    return 1;
}

/* 各種システムコールに対応するハンドラテーブル */
struct file_operations test_fops = {
    .open = test_open,
    .release = test_close,
    .read = test_read,
    .write = test_write,
};

/* init で呼ばれる関数 */
static int __init test_init(void) 
{
    printk("test-driver init\n");
    // カーネルに本ドライバを登録する
    register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &test_fops);
    return 0;
}

static void __exit test_exit(void) 
{
    printk("test-driver exit\n");
    unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");