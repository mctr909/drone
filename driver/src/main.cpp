#include <Arduino.h>
#include <Wire.h>

#include "lsm9ds1.h"
#include "imu_filter.h"

#define SAMPLE_RATE   400 // サンプリング周波数
#define SEND_INTERVAL  20 // 送信間隔
#define LSM9DS1_M    0x1C // コンパスのI2Cアドレス
#define LSM9DS1_AG   0x6A // 加速度とジャイロのI2Cアドレス

const unsigned long DELTA_TIME = 1000000 / SAMPLE_RATE;
unsigned long micros_prev;
int send_interval_count = 0;

// 9軸センサのインスタンス
LSM9DS1 imu;
// フィルターのインスタンス
IMU_FILTER filter;

void setup() {
	Serial.begin(115200);
	Wire.setClock(400000);
	Wire.begin();
	if (!imu.begin(LSM9DS1_AG, LSM9DS1_M, Wire)) {
		Serial.println("LSM9DS1に接続できません");
		while (1);
	}
	imu.calibrate_m();
	filter.set_sample_rate(SAMPLE_RATE);
	filter.set_beta(3.0f);
	micros_prev = micros();
}

void loop() {
	if (micros() - micros_prev >= DELTA_TIME) {
		imu.read_g();
		imu.read_a();
		imu.read_m();
		filter.update(
			imu.gx, imu.gy, imu.gz,
			imu.ax, imu.ay, imu.az,
			-imu.my, -imu.mx, imu.mz
		);
		if ((++send_interval_count) >= SEND_INTERVAL) {
			filter.compute_angles();
			Serial.printf("%1.3f,%1.3f\n",
				filter.roll, filter.pitch
			);
			send_interval_count = 0;
		}
		micros_prev = micros();
	}
}
