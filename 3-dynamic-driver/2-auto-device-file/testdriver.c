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

// デバイス名
#define DRIVER_NAME "test-driver"

// マイナー番号の開始番号と個数
// マイナー番号は 0, 1
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 2;

// メジャー番号 (動的に決定する)
static unsigned int testdriver_major;

// キャラクタデバイスのオブジェクト
static struct cdev testdriver_cdev;

// デバイスドライバのクラスオブジェクト
static struct class *testdriver_class = NULL;


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

    int alloc_ret = 0;
    int cdev_err = 0;
    dev_t dev;

    // 空いているメジャー番号を確保する
    alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DRIVER_NAME);
    if (alloc_ret != 0) {
        printk(KERN_ERR "alloc_chrdev_region = %d\n", alloc_ret);
        return -1;
    }

    printk("allocated  dev: %d, major: %d, minor: %d\n", dev, MAJOR(dev), MINOR(dev));

    // 取得した dev (= メジャー番号 + マイナー番号) からメジャー番号を取得する
    testdriver_major = MAJOR(dev);

    // キャラクタデバイスの初期化
    cdev_init(&testdriver_cdev, &test_fops);
    testdriver_cdev.owner = THIS_MODULE;

    // キャラクタデバイスをカーネルに登録
    cdev_err = cdev_add(&testdriver_cdev, dev, MINOR_NUM);
    if (cdev_err != 0) {
        printk(KERN_ERR "cdev_add = %d\n", cdev_err);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // このデバイスのクラス登録をする (/sys/class/testdriver/ を作る)
    testdriver_class = class_create(THIS_MODULE, "testdriver");
    if (IS_ERR(testdriver_class)) {
        printk(KERN_ERR "class_create\n");
        cdev_del(&testdriver_cdev);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // /sys/class/testdriver/testdriver* を作る
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_create(testdriver_class, NULL, MKDEV(testdriver_major, minor), NULL, "testdriver%d", minor);
    }

    return 0;
}

static void __exit test_exit(void) 
{
    printk("test-driver exit\n");

    dev_t dev = MKDEV(testdriver_major, MINOR_BASE);

    // /sys/class/testdriver/testdriver* を削除する
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_destroy(testdriver_class, MKDEV(testdriver_major, minor));
    }

    // このデバイスのクラス登録を取り除く (/sys/class/testdriver/ を削除する)
    class_destroy(testdriver_class);

    // このデバイスをカーネルから取り除く
    cdev_del(&testdriver_cdev);

    // このドライバで使用していたメジャー番号の登録を取り除く
    unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");