// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion01.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/rodrigues_formula.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion02.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion03.html
#include <Wire.h>
#include "SparkFunLSM9DS1.h"
#include "gam_filter.h"

#define SAMPLE_RATE   200 // サンプリング周波数
#define SEND_INTERVAL  10 // 送信間隔
#define LSM9DS1_M    0x1C // コンパスのI2Cアドレス
#define LSM9DS1_AG   0x6A // 加速度とジャイロのI2Cアドレス

const unsigned long DELTA_TIME = 1000000 / SAMPLE_RATE;
unsigned long micros_prev;
int send_interval_count = 0;

// 9軸センサのインスタンス
LSM9DS1 imu;
// フィルターのインスタンス
GAM_FILTER filter;

void setup() {
	Serial.begin(115200);
	Wire.setClock(400000);
	Wire.begin();
	if (!imu.begin(LSM9DS1_AG, LSM9DS1_M, Wire)) {
		Serial.println("LSM9DS1に接続できません");
		while (1);
	}
	filter.set_sample_rate(SAMPLE_RATE);
	filter.set_beta(2.0f);
	micros_prev = micros();
}

void loop() {
	if (micros() - micros_prev >= DELTA_TIME) {
		imu.readGyro();
		imu.readAccel();
		imu.readMag();
		filter.update(
			imu.gx, imu.gy, imu.gz,
			imu.ax, imu.ay, imu.az,
			imu.mx, imu.my, imu.mz
		);
		if ((++send_interval_count) >= SEND_INTERVAL) {
			filter.compute_angles();
			Serial.printf("%1.3f,%1.3f,%1.3f,%1.3f,%1.3f,%1.3f\n",
				filter.roll, filter.pitch, filter.yaw,
				filter.hx, filter.hy, filter.hz
			);
			send_interval_count = 0;
		}
		micros_prev = micros();
	}
}
