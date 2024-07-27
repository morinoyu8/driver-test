#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/io.h>

// デバイス名
#define DEVICE_NAME "led"

// GPIO の番号
#define GPIO_NUM 4

// ペリフェラルレジスタの物理アドレス
#define REG_ADDR_BASE 0x3F000000
#define REG_ADDR_GPIO_BASE (REG_ADDR_BASE + 0x00200000)
#define REG_ADDR_GPIO_GPFSEL_0 0x0000
#define BIT_OUTPUT_GPIO4_GPIO_GPFSEL_0 12
#define REG_ADDR_GPIO_OUTPUT_SET_0 0x001C
#define REG_ADDR_GPIO_OUTPUT_CLR_0 0x0028
#define REG_ADDR_GPIO_LEVEL_0 0x0034

#define REG(addr) (*((volatile unsigned int *)(addr)))
#define DUMP_REG(addr) printk("%08X\n", REG(addr))

#define ADDR_SIZE 4


// マイナー番号の開始番号と個数
// マイナー番号は 0, 1
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 1;

// メジャー番号 (動的に決定する)
static unsigned int led_major;

// キャラクタデバイスのオブジェクト
static struct cdev led_cdev;

// デバイスドライバのクラスオブジェクト
static struct class *led_class = NULL;


/* open で呼ばれる関数 */
static int led_open(struct inode *inode, struct file *file)
{
    printk("LED_open\n");

    // CPU から見た物理アドレスから仮想アドレスを取得する
    int address = (int)ioremap(REG_ADDR_GPIO_BASE + REG_ADDR_GPIO_GPFSEL_0, ADDR_SIZE);

    // GPIO 4 を出力に設定する
    REG(address) = 1 << BIT_OUTPUT_GPIO4_GPIO_GPFSEL_0;

    iounmap((void*)address);
    return 0;
}

/* clone で呼ばれる関数 */
static int led_close(struct inode *inode, struct file *file)
{
    printk("LED_close\n");
    return 0;
}

/* read で呼ばれる関数 */
static ssize_t led_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("LED_read\n");

    // CPU から見た物理アドレスから仮想アドレスを取得する
    int address = (int)ioremap(REG_ADDR_GPIO_BASE + REG_ADDR_GPIO_LEVEL_0, ADDR_SIZE);

    // GPIO 4 の値を読み取る
    if (REG(address) & (1 << GPIO_NUM))
        put_user('1', &buf[0]);
    else
        put_user('0', &buf[0]);

    iounmap((void*)address);
    return 1;
}

/* write で呼ばれる関数 */
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("LED_write\n");

    int address;

    // ユーザが設定した GPIO への出力値を取得
    char outValue;
    get_user(outValue, &buf[0]);

    // GPIO 4 に出力値を設定する
    if (outValue == '1') {
        address = (int)ioremap(REG_ADDR_GPIO_BASE + REG_ADDR_GPIO_OUTPUT_SET_0, ADDR_SIZE);
    } else if (outValue == '0') {
        address = (int)ioremap(REG_ADDR_GPIO_BASE + REG_ADDR_GPIO_OUTPUT_CLR_0, ADDR_SIZE);
    }
    REG(address) = 1 << GPIO_NUM;

    iounmap((void*)address);
    return 1;
}

/* 各種システムコールに対応するハンドラテーブル */
struct file_operations led_fops = {
    .open = led_open,
    .release = led_close,
    .read = led_read,
    .write = led_write,
};

/* init で呼ばれる関数 */
static int __init led_init(void) 
{
    printk("LED_init\n");

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
    led_major = MAJOR(dev);

    // キャラクタデバイスの初期化
    cdev_init(&led_cdev, &led_fops);
    led_cdev.owner = THIS_MODULE;

    // キャラクタデバイスをカーネルに登録
    cdev_err = cdev_add(&led_cdev, dev, MINOR_NUM);
    if (cdev_err != 0) {
        printk(KERN_ERR "cdev_add = %d\n", cdev_err);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // このデバイスのクラス登録をする (/sys/class/led/ を作る)
    led_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(led_class)) {
        printk(KERN_ERR "class_create\n");
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    // /sys/class/led/led* を作る
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_create(led_class, NULL, MKDEV(led_major, minor), NULL, "%s%d", DEVICE_NAME, minor);
    }

    return 0;
}

static void __exit led_exit(void) 
{
    printk("LED_exit\n");

    dev_t dev = MKDEV(led_major, MINOR_BASE);

    // /sys/class/led/led* を削除する
    for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_destroy(led_class, MKDEV(led_major, minor));
    }

    // このデバイスのクラス登録を取り除く (/sys/class/led/ を削除する)
    class_destroy(led_class);

    // このデバイスをカーネルから取り除く
    cdev_del(&led_cdev);

    // このドライバで使用していたメジャー番号の登録を取り除く
    unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");