#ifndef TEST_DRIVER_H_
#define TEST_DRIVER_H_

#include <linux/ioctl.h>

// ioctl 用パラメータ(第3引数) の定義
struct testdevice_values {
    int val1;
    int val2;
};

// ioctl 用コマンド (request, 第2引数) の定義

// このドライバで使用する ioctl 用コマンドのタイプ
// なんでも良いが, 'M' にしておく
#define TESTDEVICE_IOC_TYPE 'M'

// デバドラに値を設定するコマンド
#define TESTDEVICE_SET_VALUES _IOW(TESTDEVICE_IOC_TYPE, 1, struct testdevice_values)

// デバドラから値を取得するコマンド
#define TESTDEVICE_GET_VALUES _IOR(TESTDEVICE_IOC_TYPE, 2, struct testdevice_values)

#endif