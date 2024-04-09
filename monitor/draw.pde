void drawPY(float roll, float yaw) {
	// 軸描画
	drawAxiz(300, 300, 300);
	// Y軸回転
	float yc = cos(roll);
	float ys = sin(roll);
	// Z軸回転
	float zc = cos(yaw);
	float zs = sin(yaw);
	pushMatrix(); // 現在の行列をpush
	// YZ軸回転行列
	applyMatrix( zc,  ys*zs, -yc*zs, 0,
	              0,  yc   ,  ys   , 0,
	             zs, -ys*zc,  yc*zc, 0,
	              0,      0,      0, 1);
	// モデル描画
	drawModel(250, 80, 20, 0, 255, 0);
	popMatrix(); // 元の行列をpop
}

void drawRPY(float roll, float pitch, float yaw) {
	// 軸描画
	drawAxiz(1000, 1000, 1000);
	// X軸回転
	float xc = cos(pitch);
	float xs = sin(pitch);
	// Y軸回転
	float yc = cos(roll);
	float ys = sin(roll);
	// Z軸回転
	float zc = cos(yaw);
	float zs = sin(yaw);
	pushMatrix(); // 現在の行列をpush
	// XYZ軸回転行列
	applyMatrix( xc*zc, xs*yc*zc + ys*zs, xs*ys*zc - yc*zs, 0,
	            -xs   , xc*yc           , xc*ys           , 0,
	             xc*zs, xs*yc*zs - ys*zc, xs*ys*zs + yc*zc, 0,
	                 0,                0,                0, 1);
	// モデル描画
	drawModel(800, 50, 800, 0, 255, 255);
	popMatrix(); // 元の行列をpop
}

void drawAxiz(int w, int h, int d) {
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

void drawModel(int w, int h, int d, int r, int g, int b) {
	float ax = w*0.5f;
	float ay = 0;
	float az = d*0.5f;
	float bx = 0;
	float by = ay;
	float bz = -az;
	float cx = -ax;
	float cy = ay;
	float cz = az;
	float dx = bx;
	float dy = -h;
	float dz = az;
	//
	specular(r, g, b);
	shininess(2.0);
	stroke(0,0,0);
	strokeWeight(2);
	// 底面
	beginShape();
	vertex(ax, ay, az);
	vertex(bx, by, bz);
	vertex(cx, cy, cz);
	endShape();
	// 背面
	beginShape();
	vertex(ax, ay, az);
	vertex(cx, cy, cz);
	vertex(dx, dy, dz);
	endShape();
	// 左上面
	beginShape();
	vertex(ax, ay, az);
	vertex(dx, dy, dz);
	vertex(bx, by, bz);
	endShape();
	// 右上面
	beginShape();
	vertex(bx, by, bz);
	vertex(dx, dy, dz);
	vertex(cx, cy, cz);
	endShape();
}
