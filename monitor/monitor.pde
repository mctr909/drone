import processing.serial.*;

Serial mPort;
IMU_FILTER mFilter;
Plot mWave;
String[] mRows;
int mDataPos = 1;
boolean mEnableTest = true;

void setup() {
	if (mEnableTest) {
		mRows = loadStrings("test_data.csv");
		mWave = new Plot(6, width);
	} else {
		String[] ports = Serial.list();
		for (int i = 0; i < ports.length; i++) {
			println(i + ": " + ports[i]);
		}
		mPort = new Serial(this, ports[1], 115200);
		mWave = new Plot(2, width);
	}
	mFilter = new IMU_FILTER();
	size(800, 600, P3D);
}

void draw() {
	background(0);
	translate(width/2, height/2, -100);
	scale(0.4);
	ambientLight(50, 50, 50);                  // 環境光を当てる
	lightSpecular(255, 255, 255);              // 光の鏡面反射色（ハイライト）を設定
	directionalLight(100, 100, 100, 0, 1, -1); // 指向性ライトを設定
	//
	if (mEnableTest) {
		read_test_data();
	} else {
		receive();
	}
	//mFilter.azimuth = 6.28*((float)mouseX/width - 0.5f);
	//mFilter.elevation = -6.28*((float)mouseY/height - 0.5f);
	//
	pushMatrix();
	translate(0, height*6/8, 0);
	drawRPY(mFilter.roll, mFilter.pitch, 0);
	popMatrix();
}

void receive() {
	if (mPort.available() == 0) {
		mWave.plot(width*2, height*2);
		return;
	}
	String str = mPort.readStringUntil('\n');  // 1行読み込み
	if (str == null) {
		mWave.plot(width*2, height*2);
		return;
	}
	String toks[] = split(trim(str), ",");
	if (toks.length != 2) {
		mWave.plot(width*2, height*2);
		return;
	}
	mFilter.roll = float(toks[0]);
	mFilter.pitch = float(toks[1]);
	mWave.plot(width*2, height*2, mFilter.roll/3.14, mFilter.pitch/3.14);
}

void read_test_data() {
	if (null == mRows || mRows.length < 2) {
		return;
	}
	String line = mRows[mDataPos];
	String[] cols = splitTokens(line, ",");
	mDataPos++;
	if (mDataPos >= mRows.length) {
		mDataPos = 1;
	}
	if (9 == cols.length) {
		float wx = float(cols[0]);
		float wy = float(cols[1]);
		float wz = float(cols[2]);
		float ax = float(cols[3]);
		float ay = float(cols[4]);
		float az = float(cols[5]);
		float mx = float(cols[6]);
		float my = float(cols[7]);
		float mz = float(cols[8]);
		mFilter.update(
			wx, wy, wz,
			ax, ay, az,
			mx, my, mz
		);
		wx /= 32768; wy /= 32768; wz /= 32768;
		ax /= 32768; ay /= 32768; az /= 32768;
		mx /= 32768; my /= 32768; mz /= 32768;
		//mWave.plot(width*2, height*2,
		//	wx, wy, wz,
		//	ax, ay, az
		//);
		//
		pushMatrix();
		translate(-width*7/8, height*6/8, 0);
		drawAxiz(500, 500, 500);
		float r = sqrt(ax*ax + ay*ay + az*az);
		ax /= r; ay /= r; az /= r;
		r = sqrt(mx*mx + my*my + mz*mz);
		mx /= r; my /= r; mz /= r;
		drawArrowC(ax, az, ay, 0, 127, 0);
		drawArrowC(mx, mz, my, 223, 0, 223);
		popMatrix();
		//
		pushMatrix();
		translate(width*7/8, height*6/8, 0);
		drawAxiz(500, 500, 500);
		drawArrowV(wx, wz, wy, 2, 0, 127, 0);
		popMatrix();
	}
}
