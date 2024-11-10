#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include "lsm9ds1.h"
#include "imu_filter.h"

#define LSM9DS1_M    0x1C // コンパスのI2Cアドレス
#define LSM9DS1_AG   0x6A // 加速度とジャイロのI2Cアドレス
#define SAMPLE_RATE   600 // サンプリング周波数
#define WIFI_INTERVAL  10 // wifi送受信間隔

const unsigned long DELTA_TIME = (int)1e+6 / SAMPLE_RATE;
unsigned long micros_prev;
int wifi_interval_count = 0;

const char *ssid = "auhikari-MzQmYz-g"; // アクセスポイントのSSID
const char *pass = "UGNVmZwQWZzU3";     // アクセスポイントのパスワード
const int port = 10002;                 // ESP32サーバのポート
WiFiServer server(port);
WiFiClient client;
bool connected = false;

// 9軸センサのインスタンス
LSM9DS1 imu;
IMU_FILTER filter;

void setup() {
	Serial.begin(115200);
	Serial2.setRxInvert(true);
	Serial2.begin(100000);
	Wire.setClock(400000);
	Wire.begin();
	if (!imu.begin(LSM9DS1_AG, LSM9DS1_M, Wire)) {
		while (1);
	}
	// アクセスポイントに接続
	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED) {
		// 接続が完了するまで待つ
		delay(100);
		Serial.print(".");
	}
	// ESP32サーバ開始
	server.begin();
	// ESP32のIPアドレスを表示
	Serial.println();
	Serial.println(WiFi.localIP());
	imu.calibrate_m();
	micros_prev = micros();
}

void loop() {
	if (!connected) {
		client = server.available();
		if (!client) {
			delay(100);
			return;
		}
		Serial.println("new client");
		connected = true;
	}
	if (micros() - micros_prev < DELTA_TIME) {
		return;
	}
	imu.read_g();
	imu.read_a();
	imu.read_m();
	filter.update(
		imu.gx, imu.gy, imu.gz,
		imu.ax, imu.ay, imu.az,
		imu.mx, imu.my, imu.mz
	);
	if (++wifi_interval_count >= WIFI_INTERVAL) {
		wifi_interval_count = 0;
		filter.compute_angles();
		connected = client.connected();
		if (connected) {
			client.printf("%f,%f,%f,%d,%d,%d\n",
				filter.roll, filter.pitch, filter.yaw,
				imu.ax, imu.ay, imu.az
			);
			while (client.available()) {
				auto line = client.readStringUntil('\n');
				Serial.println(line);
				auto col = strtok((char*)line.c_str(), " ");
				auto type = col;
				if (0 == strcmp("beta", type)) {
					col = strtok(nullptr, " ");
					if (col != nullptr) {
						filter.set_beta(atoff(col));
					}
				}
				if (0 == strcmp("gscale", type)) {
					col = strtok(nullptr, " ");
					if (col != nullptr) {
						filter.set_gscale(atoff(col));
					}
				}
				if (0 == strcmp("mscale", type)) {
					col = strtok(nullptr, " ");
					if (col != nullptr) {
						filter.set_mscale(atoff(col));
					}
				}
				if (0 == strcmp("p", type)) {
					col = strtok(nullptr, " ");
					if (col != nullptr) {
					}
					col = strtok(nullptr, " ");
					if (col != nullptr) {
					}
				}
			}
		}
	}
	micros_prev = micros();
}
