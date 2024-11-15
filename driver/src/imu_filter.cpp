#include <math.h>
#include "imu_filter.h"

// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion01.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/rodrigues_formula.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion02.html
// https://www.sports-sensing.com/brands/labss/motionmeasurement/motion_biomechanics/quaternion03.html

IMU_FILTER::IMU_FILTER() {
	delta_time = 1.0f / 100.0f;
	beta = 1.0f;
	gscale = 1.0f;
	mscale = 1.0f;
	qw = 1.0f;
	qx = 0;
	qy = 0;
	qz = 0;
}

void IMU_FILTER::update(float wx, float wy, float wz, float ax, float ay, float az, float mx, float my, float mz) {
	// 角速度を(rad/s)に変換
	wx *= 9.5873799e-5f * gscale;
	wy *= 9.5873799e-5f * gscale;
	wz *= 9.5873799e-5f * gscale;
	// 回転量(Δq)＝姿勢(q)が角速度(w)で回転するときの時間変化
	float dqw, dqx, dqy, dqz;
	dqw = -0.5f*(      - wx*qx - wy*qy - wz*qz);
	dqx = -0.5f*(wx*qw         + wz*qy - wy*qz);
	dqy = -0.5f*(wy*qw - wz*qx         + wx*qz);
	dqz = -0.5f*(wz*qw + wy*qx - wx*qy        );
	{
		// X軸基準ベクトル
		float xx, xy, xz;
		xx = 2*(qw*qw + qx*qx) - 1;
		xy = 2*(qx*qy - qw*qz);
		xz = 2*(qx*qz + qw*qy);
		// Y軸基準ベクトル
		float yx, yy, yz;
		yx = 2*(qx*qy + qw*qz);
		yy = 2*(qw*qw + qy*qy) - 1;
		yz = 2*(qy*qz - qw*qx);
		// Z軸基準ベクトル
		float zx, zy, zz;
		zx = 2*(qx*qz - qw*qy);
		zy = 2*(qy*qz + qw*qx);
		zz = 2*(qw*qw + qz*qz) - 1;
		// 補正勾配(grad s)＝鉛直方向の勾配(grad g)＋姿勢方位の勾配(grad h)
		float sw = 0, sx = 0, sy = 0, sz = 0;
		if (!(0 == ax && 0 == ay && 0 == az)) {
			// 加速度を正規化
			float r = 1.0f / sqrtf(ax*ax + ay*ay + az*az);
			ax *= r, ay *= r, az *= r;
			// 鉛直方向の変化(Δg)
			float dgx, dgy, dgz;
			dgx = zx - ax;
			dgy = zy - ay;
			dgz = zz - az;
			// 鉛直方向の勾配(grad g)
			sw += 2*(qx*dgy - qy*dgx);
			sx += 2*(qw*dgy + qz*dgx - 2*qx*dgz);
			sy += 2*(qz*dgy - qw*dgx - 2*qy*dgz);
			sz += 2*(qy*dgy + qx*dgx);
		}
		if (!(0 == mx && 0 == my && 0 == mz)) {
			// 方位を正規化
			float r = mscale / sqrtf(mx*mx + my*my + mz*mz);
			mx *= r, my *= r, mz *= r;
			// 姿勢方位(h)＝方位(m)を姿勢(q)で回転させた向き
			float hx, hy, hz, hxy;
			hx = xx*mx + xy*my + xz*mz;
			hy = yx*mx + yy*my + yz*mz;
			hz = zx*mx + zy*my + zz*mz;
			hxy = sqrtf(hx*hx + hy*hy);
			// 姿勢方位の変化(Δh)
			float dhx, dhy, dhz;
			dhx = xx*hxy + zx*hz - mx;
			dhy = xy*hxy + zy*hz - my;
			dhz = xz*hxy + zz*hz - mz;
			// 姿勢方位の勾配(grad h)
			sw += (qx*hz - qz*hxy)*dhy -             qy*hz *dhx +  qy*hxy           *dhz;
			sx += (qw*hz + qy*hxy)*dhy +             qz*hz *dhx + (qz*hxy - 2*qx*hz)*dhz;
			sy += (qz*hz + qx*hxy)*dhy - (2*qy*hxy + qw*hz)*dhx + (qw*hxy - 2*qy*hz)*dhz;
			sz += (qy*hz - qw*hxy)*dhy - (2*qz*hxy - qx*hz)*dhx +  qx*hxy           *dhz;
		}
		float sr = sqrtf(sw*sw + sx*sx + sy*sy + sz*sz);
		// 補正勾配を正規化して補正係数(β)でスケーリング
		if (sr > 0) {
			sr = beta / sr;
			sw *= sr, sx *= sr, sy *= sr, sz *= sr;
			// 回転量に補正勾配を反映
			dqw -= sw;
			dqx -= sx;
			dqy -= sy;
			dqz -= sz;
		}
	}
	// 回転量を積算して姿勢を更新
	qw += dqw * delta_time;
	qx += dqx * delta_time;
	qy += dqy * delta_time;
	qz += dqz * delta_time;
	// 姿勢を正規化
	float r = 1.0f / sqrtf(qw*qw + qx*qx + qy*qy + qz*qz);
	qw *= r, qx *= r, qy *= r, qz *= r;
}

void IMU_FILTER::compute_angles() {
	roll  = -atan2f(qw*qx + qy*qz, qw*qw + qz*qz - 0.5f);
	yaw   = -atan2f(qw*qz + qx*qy, qy*qy + qz*qz - 0.5f);
	pitch = -asinf(2*(qx*qz - qw*qy));
}
