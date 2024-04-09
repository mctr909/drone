import processing.serial.*;

Serial mPort;
Plot mPlot;

float mRoll = 0.0f;
float mPitch = 0.0f;
float mYaw = 0.0f;

vec3 mG = new vec3();
vec3 mA = new vec3();
vec3 mM = new vec3();

void setup() {
    String[] ports = Serial.list();
    for (int i = 0; i < ports.length; i++) {
        println(i + ": " + ports[i]);
    }
    mPort = new Serial(this, ports[1], 115200);
    mPlot = new Plot(3, width);
    size(1024, 768, P3D);
}

void receive() {
    if (0 == mPort.available()) {
        return;
    }
    String str = mPort.readStringUntil('\n');  // 1行読み込み
    if (null == str) {
        return;
    }
    String toks[] = split(trim(str), ",");
    if (12 > toks.length) {
        return;
    }
    mRoll = float(toks[0]);
    mPitch = float(toks[1]);
    mYaw = float(toks[2]);
    mG.x = float(toks[3]);
    mG.y = float(toks[4]);
    mG.z = float(toks[5]);
    mA.x = float(toks[6]);
    mA.y = float(toks[7]);
    mA.z = float(toks[8]);
    mM.x = float(toks[9]);
    mM.y = float(toks[10]);
    mM.z = float(toks[11]);
}

void draw() {
    background(0);
    translate(width / 2, height / 2, -100);
    scale(0.4);
    ambientLight(50, 50, 50);                  // 環境光を当てる
    lightSpecular(255, 255, 255);              // 光の鏡面反射色（ハイライト）を設定
    directionalLight(100, 100, 100, 0, 1, -1); // 指向性ライトを設定
    receive();
    /* Waves */
    pushMatrix();
    translate(0, -height/4, 0);
    mPlot.plot(width*2, 1000,
        mA.x/32768, mA.y/32768, mA.z/32768
    );
    popMatrix();
    /* Accel & Compass */
    pushMatrix();
    translate(-width*7/8, height*6/8, 0);
    drawAxiz(500, 500, 500);
    drawArrowN(mA, 0, 127, 0);
    drawArrowN(mM, 191, 0, 191);
    popMatrix();
    /* Gyro */
    pushMatrix();
    translate(width*7/8, height*6/8, 0);
    drawAxiz(500, 500, 500);
    drawArrowN(mG, 0, 127, 0);
    popMatrix();
    /* Roll & Pitch */
    pushMatrix();
    translate(0, height * 6 / 8, 0);
    drawRP(mRoll, mPitch);
    popMatrix();
}
