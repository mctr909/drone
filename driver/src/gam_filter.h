#ifndef __GAM_FILTER_H__
#define __GAM_FILTER_H__

class GAM_FILTER{
private:
	float delta_time;
	float beta;
	float qw;
	float qx;
	float qy;
	float qz;
public:
	float roll;
	float pitch;
	float yaw;
	float hx;
	float hy;
	float hz;
public:
	GAM_FILTER();
	void update(float wx, float wy, float wz, float ax, float ay, float az, float mx, float my, float mz);
	void compute_angles();
	void set_sample_rate(float sample_rate) {
		delta_time = 1.0f / sample_rate;
	}
	void set_beta(float beta) {
		this->beta = beta;
	}
};

#endif