ImuFilter mFilter;
String[] mRows;
int mDataPos = 1;

void setup() {
	size(800, 600, P3D);
	mFilter = new ImuFilter();
	mRows = loadStrings("test_data.csv");
}

void draw() {
	update();
	//
	background(0);
	translate(width/2, height/2, -100);
	scale(0.4);
	ambientLight(50, 50, 50);                  // 環境光を当てる
	lightSpecular(255, 255, 255);              // 光の鏡面反射色（ハイライト）を設定
	directionalLight(100, 100, 100, 0, 1, -1); // 指向性ライトを設定
	//
	pushMatrix();
	translate(-width*11/16, height*5/8, 0);
	drawPY(0, mFilter.azimuth);
	popMatrix();
	//
	pushMatrix();
	translate(width/2, height*5/8, 0);
	drawRPY(mFilter.roll, mFilter.pitch, 0);
	popMatrix();
}

void update() {
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
		mFilter.update(
			float(cols[0]), float(cols[1]), float(cols[2]),
			float(cols[3]), float(cols[4]), float(cols[5]),
			float(cols[6]), float(cols[7]), float(cols[8])
		);
	}
}
