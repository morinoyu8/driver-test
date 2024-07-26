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
#include <linux/slab.h>

// デバイス名
#define DEVICE_NAME "testdevice"

// マイナー番号の開始番号と個数
// マイナー番号は 0, 1
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 2;

// メジャー番号 (動的に決定する)
static unsigned int testdevice_major;

// キャラクタデバイスのオブジェクト
static struct cdev testdevice_cdev;

// デバイスドライバのクラスオブジェクト
static struct class *testdevice_class = NULL;

// read 時に保持する値
// 各ファイルに紐づくデータ
#define NUM_BUFFER 256
struct _testdevice_file_data {
    unsigned char buffer[NUM_BUFFER];
};


/* open で呼ばれる関数 */
static int test_open(struct inode *inode, struct file *file)
{
    printk("test_open\n");

    // 各ファイル固有のデータを格納する領域を確保する
    struct _testdevice_file_data *p = kmalloc(sizeof(struct _testdevice_file_data), GFP_KERNEL);
    if (p == NULL) {
        printk(KERN_ERR "kmalloc\n");
        return -ENOMEM;
    }

    // ファイル固有のデータを初期化する
    strlcat(p->buffer, "dummy", 5);

    // 確保したポインタはユーザ側の fd で保持してもらう
    file->private_data = p;

    return 0;
}

/* clone で呼ばれる関数 */
static int test_close(struct inode *inode, struct file *file)
{
    printk("test_close\n");

    if (file->private_data) {
        // open 時に確保した, 各ファイルの固有のデータ領域を解放する
        kfree(file->private_data);
        file->private_data = NULL;
    }
    return 0;
}

/* read で呼ばれる関数 */
static ssize_t test_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("test_read\n");

    if (count > NUM_BUFFER)
        count = NUM_BUFFER;

    struct _testdevice_file_data *p = file->private_data;
    if (copy_to_user(buf, p->buffer, count) != 0)
        return -EFAULT;
    return 1;
}

/* write で呼ばれる関数 */
static ssize_t test_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("test_write\n");

    struct _testdevice_file_data *p = file->private_data;
    if (copy_from_user(p->buffer, buf, count) != 0)
        return -EFAULT;
    printk("stored_value: %s\n", p->buffer);
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
    printk("testdriver init\n");

    int alloc_ret = 0;
    int cdev_err = 0;
    dev_t dev;

    // 空いているメジャー番号を確保する
    alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
    if (alloc_ret != 0) {
        printk(KERN_ERR "alloc_chrdev_region = %d\n", alloc_ret);
        return -1;
    }

    printk("allocated device major: %d\n", MAJOR(dev));

    // 取得した dev (= メジャー番号 + マイナー番号) からメジャー番号を取得する
    testdevice_major = MAJOR(dev);

    // キャラクタデバイスの初期化
    cdev_init(&testdevice_cdev, &test_fops);
    testdevice_cdev.owner = THIS_MODULE;

    // キャラクタデバイスをカーネルに登録
    cdev_err = cdev_add(&testdevice_cdev, dev, MINOR_NUM);
    if (cdev_err != 0) {
        printk(KERN_ERR "cdev_add = %d\n", cdev_err);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // このデバイスのクラス登録をする (/sys/class/testdevice/ を作る)
    testdevice_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(testdevice_class)) {
        printk(KERN_ERR "class_create\n");
        cdev_del(&testdevice_cdev);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // /sys/class/testdevice/testdevice* を作る
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_create(testdevice_class, NULL, MKDEV(testdevice_major, minor), NULL, "%s%d", DEVICE_NAME, minor);
    }

    return 0;
}

static void __exit test_exit(void) 
{
    printk("testdriver exit\n");

    dev_t dev = MKDEV(testdevice_major, MINOR_BASE);

    // /sys/class/testdevice/testdevice* を削除する
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_destroy(testdevice_class, MKDEV(testdevice_major, minor));
    }

    // このデバイスのクラス登録を取り除く (/sys/class/testdevice/ を削除する)
    class_destroy(testdevice_class);

    // このデバイスをカーネルから取り除く
    cdev_del(&testdevice_cdev);

    // このドライバで使用していたメジャー番号の登録を取り除く
    unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");