void drawArrowV(float x, float y, float z, float s, int r, int g, int b) {
	float n = s*sqrt(x*x + y*y + z*z);
	// Y軸回転
	float pitch = atan2(-y, sqrt(x*x + z*z));
	float yc = n*cos(pitch);
	float ys = n*sin(pitch);
	// Z軸回転
	float yaw = atan2(z, -x);
	float zc = n*cos(yaw);
	float zs = n*sin(yaw);
	pushMatrix(); // 現在の行列をpush
	// YZ軸回転行列
	applyMatrix( zc,  ys*zs, -yc*zs, 0,
	              0,  yc   ,  ys   , 0,
	             zs, -ys*zc,  yc*zc, 0,
	              0,      0,      0, 1);
	// モデル描画
	drawModel(50, 30, 500, r, g, b);
	popMatrix(); // 元の行列をpop
}

void drawArrowC(float x, float y, float z, int r, int g, int b) {
	// Y軸回転
	float pitch = atan2(-y, sqrt(x*x + z*z));
	float yc = cos(pitch);
	float ys = sin(pitch);
	// Z軸回転
	float yaw = atan2(z, -x);
	float zc = cos(yaw);
	float zs = sin(yaw);
	pushMatrix(); // 現在の行列をpush
	// YZ軸回転行列
	applyMatrix( zc,  ys*zs, -yc*zs, 0,
	              0,  yc   ,  ys   , 0,
	             zs, -ys*zc,  yc*zc, 0,
	              0,      0,      0, 1);
	// モデル描画
	drawModel(50, 30, 500, r, g, b);
	popMatrix(); // 元の行列をpop
}

void drawRPY(float roll, float pitch, float yaw) {
	// 軸描画
	drawAxiz(800, 800, 800);
	// X軸回転
	float xc = cos(roll);
	float xs = sin(roll);
	// Y軸回転
	float yc = cos(pitch);
	float ys = sin(pitch);
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
	stroke(167, 167, 167);
	strokeWeight(3);
	// XZ平面
	line(-w*0.5f, 0, -d*0.5f, -w*0.5f, 0,  d*0.5f);
	line( w*0.5f, 0, -d*0.5f,  w*0.5f, 0,  d*0.5f);
	line(-w*0.5f, 0, -d*0.5f,  w*0.5f, 0, -d*0.5f);
	line(-w*0.5f, 0,  d*0.5f,  w*0.5f, 0,  d*0.5f);
	// Z軸
	line(0, 0, -d*0.5f, 0, 0, d*0.5f);
	// X軸
	line(-w*0.5f, 0, 0, w*0.5f, 0, 0);
	// Y軸
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
