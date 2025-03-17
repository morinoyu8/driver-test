#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>

// デバイス名
#define DEVICE_NAME "sensor_led"

// GPIO の番号
#define GPIO_NUM_LED 4
#define GPIO_NUM_SENSOR_VDD 17
#define GPIO_NUM_SENSOR_OUT 22

#define SENSOR_MOVE 0

static irqreturn_t sensor_irq_handler(int irq, void *dev_id) {
    // センサの出力を読み取る
    int value = gpio_get_value(GPIO_NUM_SENSOR_OUT);
    printk("sensor_led: sensor_out=%d\n", value);

    // LED を点灯・消灯
    if (value == SENSOR_MOVE)
        gpio_set_value(GPIO_NUM_LED, 1);
    else
        gpio_set_value(GPIO_NUM_LED, 0);

    return IRQ_HANDLED;
}

// ロード時に呼ばれる
static int device_init(void) {
    printk("sensor_led: device_init\n");

    // LED 用の GPIO4 を出力に設定
    // 初期値は 0 (LED は消灯)
    gpio_direction_output(GPIO_NUM_LED, 0);

    // センサ用の GPIO17 を出力に設定
    // 初期値は 1 (センサの電源を ON)
    gpio_direction_output(GPIO_NUM_SENSOR_VDD, 1);

    // センサ用の GPIO22 を入力に設定
    gpio_direction_input(GPIO_NUM_SENSOR_OUT);

    // センサ用の割り込み番号を取得
    int irq = gpio_to_irq(GPIO_NUM_SENSOR_OUT);
    printk("sensor_led: irq=%d\n", irq);

    // センサー用の割り込みハンドラを登録
    int ret = request_irq(irq, (void *)sensor_irq_handler, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    if (ret < 0) {
        printk(KERN_ERR "sensor_led: request_irq failed\n");
        return 1;
    }

    return 0;
}

// アンロード時に呼ばれる
static void device_exit(void) {
    printk("sensor_led: device_exit\n");

    // センサ用の割り込みハンドラを解放
    int irq = gpio_to_irq(GPIO_NUM_SENSOR_OUT);
    if (irq >= 0)
        free_irq(irq, NULL);
    
    // センサの電源を OFF
    gpio_set_value(GPIO_NUM_SENSOR_VDD, 0);

    // LED を消灯
    gpio_set_value(GPIO_NUM_LED, 0);
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");