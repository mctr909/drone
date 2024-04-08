String[] mRows;
int mDataPos = 1;

float delta_time = 1.0f / 60.0f;
float beta = 0.5f;
float qw = 1.0f;
float qx = 0.0f;
float qy = 0.0f;
float qz = 0.0f;
float hx = 0.0f;
float hy = 0.0f;
float hz = 0.0f;

void setup() {
	size(800, 600, P3D);
	mRows = loadStrings("data.csv");
}

void draw() {
	update();
	//
	background(0);
	translate(width/2, height/2, -100);
	scale(0.4);
	ambientLight(20, 20, 20);                  // 環境光を当てる
	lightSpecular(255, 255, 255);              // 光の鏡面反射色（ハイライト）を設定
	directionalLight(100, 100, 100, 0, 1, -1); // 指向性ライトを設定
	//
	pushMatrix();
	translate(-width*11/16, height*5/8, 0);
	drawPY(atan2(hz, sqrt(hx*hx + hy*hy)), atan2(hy, hx));
	popMatrix();
	//
	float roll = -atan2(qw*qx + qy*qz, 0.5f - qx*qx - qy*qy);
	float pitch = asin(-2*(qx*qz - qw*qy));
	float yaw = atan2(qw*qz + qx*qy, 0.5f - qy*qy - qz*qz);
	pushMatrix();
	translate(width/2, height*5/8, 0);
	drawRPY(roll, pitch, yaw);
	popMatrix();
}

void update() {
	if (mRows == null || mRows.length < 2) {
		return;
	}
	String line = mRows[mDataPos];
	String[] cols = splitTokens(line, ",");
	mDataPos++;
	if (mDataPos >= mRows.length) {
		mDataPos = 1;
	}
	float wx = float(cols[0]);
	float wy = float(cols[1]);
	float wz = float(cols[2]);
	float ax = float(cols[3]);
	float ay = float(cols[4]);
	float az = float(cols[5]);
	float mx = float(cols[6]);
	float my = float(cols[7]);
	float mz = float(cols[8]);
	// 角速度(-32768～32768)を(-π～π)に変換
	wx *= 9.5873799e-5f;
	wy *= 9.5873799e-5f;
	wz *= 9.5873799e-5f;
	// 回転量(Δq)＝姿勢(q)が角速度(w)で回転するときの時間変化
	float dqw, dqx, dqy, dqz;
	dqw = 0.5f*(         wx*qx + wy*qy + wz*qz);
	dqx = 0.5f*(-wx*qw         - wz*qy + wy*qz);
	dqy = 0.5f*(-wy*qw + wz*qx         - wx*qz);
	dqz = 0.5f*(-wz*qw - wy*qx + wx*qy        );
	if (!(0.0f == ax && 0.0f == ay && 0.0f == az)) {
		// 同じ計算を繰り返さないためにあらかじめ変数に置いておく
		float qwqw = qw*qw;
		float qwqx = qw*qx;
		float qwqy = qw*qy;
		float qwqz = qw*qz;
		float qxqx = qx*qx;
		float qxqy = qx*qy;
		float qxqz = qx*qz;
		float qyqy = qy*qy;
		float qyqz = qy*qz;
		float qzqz = qz*qz;
		// 補正勾配(s)＝重力の向きの勾配(grad g)＋姿勢方位の勾配(grad h) ⊗ 方位の変化量(Δm)
		float sw, sx, sy, sz;
		{
			// 加速度を正規化
			float r = 1.0f / sqrt(ax*ax + ay*ay + az*az);
			ax *= r; ay *= r; az *= r;
			// 重力の向き(g)
			float gx, gy, gz;
			gx =     2*(qxqz - qwqy) - ax;
			gy =     2*(qwqx + qyqz) - ay;
			gz = 1 - 2*(qxqx + qyqy) - az;
			// 重力の向きの勾配(grad g)
			sw = -2*qy*gx + 2*qx*gy;
			sx =  2*qz*gx + 2*qw*gy - 4*qx*gz;
			sy = -2*qw*gx + 2*qz*gy - 4*qy*gz;
			sz =  2*qx*gx + 2*qy*gy;
		}
		{
			// 方位を正規化
			float r = 1.0f / sqrt(mx*mx + my*my + mz*mz);
			mx *= r; my *= r; mz *= r;
			// 姿勢方位(h)＝方位(m)を姿勢(q)で回転させた向き
			float bx, bz;
			hx = 2*(qwqw + qxqx)*mx - mx + 2*(qxqy - qwqz)*my      + 2*(qxqz + qwqy)*mz;
			hy = 2*(qxqy + qwqz)*mx      + 2*(qwqw + qyqy)*my - my + 2*(qyqz - qwqx)*mz;
			hz = 2*(qxqz - qwqy)*mx      + 2*(qyqz + qwqx)*my      + 2*(qwqw + qzqz)*mz - mz;
			bx = sqrt(hx*hx + hy*hy);
			bz = hz;
			hx = mx;
			hy = my;
			hz = mz;
			// 方位の変化量(Δm)
			float dmx, dmy, dmz;
			dmx = (0.5f - qyqy - qzqz)*bx + (       qxqz - qwqy)*bz - mx;
			dmy = (       qxqy - qwqz)*bx + (       qwqx + qyqz)*bz - my;
			dmz = (       qwqy + qxqz)*bx + (0.5f - qxqx - qyqy)*bz - mz;
			// 姿勢方位の勾配(grad h)⊗方位の変化量(Δm)
			sw += (         - qy*bz)*dmx + (-qz*bx + qx*bz)*dmy + (qy*bx          )*dmz;
			sx += (           qz*bz)*dmx + ( qy*bx + qw*bz)*dmy + (qz*bx - 2*qw*bz)*dmz;
			sy += (-2*qy*bx - qw*bz)*dmx + ( qx*bx + qz*bz)*dmy + (qw*bx - 2*qy*bz)*dmz;
			sz += (-2*qz*bx + qx*bz)*dmx + (-qw*bx + qy*bz)*dmy + (qx*bx          )*dmz;
		}
		// 補正勾配を正規化
		float r = 1.0f / sqrt(sw*sw + sx*sx + sy*sy + sz*sz);
		sw *= r; sx *= r; sy *= r; sz *= r;
		// 回転量に補正勾配を反映
		dqw -= beta * sw;
		dqx -= beta * sx;
		dqy -= beta * sy;
		dqz -= beta * sz;
	}
	// 回転量を積算して姿勢を更新
	qw += dqw * delta_time;
	qx += dqx * delta_time;
	qy += dqy * delta_time;
	qz += dqz * delta_time;
	// 姿勢を正規化
	float r = 1.0f / sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
	qw *= r; qx *= r; qy *= r; qz *= r;
}

void drawAxiz(int w, int h, int d) {
	specular(167, 167, 167);
	shininess(1.0);
	// Z軸
	stroke(255,0,0);
	strokeWeight(4);
	line(0, 0, -d*0.5f, 0, 0, d*0.5f);
	// X軸
	stroke(0,255,0);
	strokeWeight(4);
	line(-w*0.5f, 0, 0, w*0.5f, 0, 0);
	// Y軸
	stroke(0, 0, 255);
	strokeWeight(4);
	line(0, -h*0.5f, 0, 0, h*0.5f, 0);
}

void drawModel(int l, int w, int h, int r, int g, int b) {
	final float ax = w*0.5f;
	final float ay = 0;
	final float az = l*0.5f;
	final float bx = 0;
	final float by = 0;
	final float bz = -l*0.5f;
	final float cx = -w*0.5f;
	final float cy = 0;
	final float cz = l*0.5f;
	final float dx = 0;
	final float dy = -h;
	final float dz = l*0.5f;
	//
	specular(r, g, b);
	shininess(2.0);
	stroke(0,0,0);
	strokeWeight(2);
	// bottom
	beginShape();
	vertex(ax, ay, az);
	vertex(bx, by, bz);
	vertex(cx, cy, cz);
	endShape();
	// back
	beginShape();
	vertex(ax, ay, az);
	vertex(cx, cy, cz);
	vertex(dx, dy, dz);
	endShape();
	// left
	beginShape();
	vertex(ax, ay, az);
	vertex(dx, dy, dz);
	vertex(bx, by, bz);
	endShape();
	// right
	beginShape();
	vertex(bx, by, bz);
	vertex(dx, dy, dz);
	vertex(cx, cy, cz);
	endShape();
}

void drawPY(float roll, float yaw) {
	drawAxiz(500, 500, 500);
	float rc = cos(roll);
	float rs = sin(roll);
	float yc = cos(yaw);
	float ys = sin(yaw);
	pushMatrix();
	applyMatrix( yc,  rs*ys, -rc*ys, 0,
	              0,  rc   ,  rs   , 0,
	             ys, -rs*yc,  rc*yc, 0,
	              0,      0,      0, 1);
	drawModel(400, 100, 30, 0, 255, 0);
	popMatrix();
}

void drawRPY(float roll, float pitch, float yaw) {
	drawAxiz(1000, 1000, 1000);
	float rc = cos(roll);
	float rs = sin(roll);
	float pc = cos(pitch);
	float ps = sin(pitch);
	float yc = cos(yaw);
	float ys = sin(yaw);
	pushMatrix();
	applyMatrix( pc*yc, rs   *ys + rc*ps*yc, rs*ps*yc - rc   *ys, 0,
	            -ps   ,            rc*pc   ,            rs*pc   , 0,
	             pc*ys, rc*ps*ys - rs   *yc, rc   *yc + rs*ps*ys, 0,
	                 0,                   0,                   0, 1);
	drawModel(800, 800, 50, 0, 255, 255);
	popMatrix();
}
