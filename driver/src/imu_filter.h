#ifndef __IMU_FILTER_H__
#define __IMU_FILTER_H__

class IMU_FILTER {
private:
	float delta_time;
	float beta;
	float gscale;
	float mscale;
	float qw;
	float qx;
	float qy;
	float qz;

public:
	float roll;
	float pitch;
	float yaw;

public:
	IMU_FILTER();
	void update(float wx, float wy, float wz, float ax, float ay, float az, float mx, float my, float mz);
	void compute_angles();
	void set_sample_rate(float sample_rate) {
		delta_time = 1.0f / sample_rate;
	}
	void set_beta(float beta) {
		this->beta = beta;
	}
	void set_gscale(float gscale) {
		this->gscale = gscale;
	}
	void set_mscale(float mscale) {
		this->mscale = mscale;
	}
};

#endif /* __IMU_FILTER_H__ */
