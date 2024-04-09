import processing.serial.*;
import processing.net.*;

Plot mPlot;
Client mClient;
int port = 10002;
String ipaddress = "192.168.0.5";  // ESP32のアドレス

float mRoll = 0.0f;
float mPitch = 0.0f;
float mYaw = 0.0f;

float mOffsetX = 0.0f;
float mOffsetY = 0.0f;
float mScale = 0.4f;

vec3 mG = new vec3();
vec3 mA = new vec3();
vec3 mM = new vec3();
float mX = 0.0f;
float mY = 0.0f;

void setup() {
    mClient = new Client(this, ipaddress, port);
    println("wifi connected");
    mPlot = new Plot(3, width);
    size(1024, 768, P3D);
}

void mouseDragged() {
  int px = (int)(mouseX - mOffsetX);
  int py = (int)(mouseY - mOffsetY);
  mX = px/mScale;
  mY = py/mScale;
  //
  float nx = px * 2.0f / width;
  float ny = (mOffsetY - mouseY) * 2.0f / height;
  mClient.write(nx + "," + ny + "\n");
}

void mouseReleased() {
  mX = 0.0f;
  mY = 0.0f;
  mClient.write("0.0,0.0\n");
}

void receive() {
    if (0 == mClient.available()) {
        return;
    }
    // 1行読み込み
    String str = mClient.readStringUntil('\n');
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
    mOffsetX = width / 2;
    mOffsetY = height / 2;
    translate(mOffsetX, mOffsetY, -100);
    scale(mScale);
    ambientLight(50, 50, 50);                  // 環境光を当てる
    lightSpecular(255, 255, 255);              // 光の鏡面反射色（ハイライト）を設定
    directionalLight(100, 100, 100, 0, 1, -1); // 指向性ライトを設定
    receive();
    // カーソル位置を中心に円を描く
    specular(0, 255, 0);
    ellipse(0, 0, 50, 50);
    specular(255, 0, 0);
    ellipse(mX, mY, 150, 150);
    if (false) {
      /* Waves */
      pushMatrix();
      translate(0, -height/4, 0);
      mPlot.plot(width*2, 1000,
          mA.x/32768, mA.y/32768, mA.z/32768
      );
      popMatrix();
    }
    if (false) {
      /* Accel & Compass */
      pushMatrix();
      translate(-width*7/8, height*6/8, 0);
      drawAxiz(500, 500, 500);
      drawArrowN(mA, 0, 127, 0);
      drawArrowN(mM, 191, 0, 191);
      popMatrix();
    }
    /* Gyro */
    if (false) {
      pushMatrix();
      translate(width*7/8, height*6/8, 0);
      drawAxiz(500, 500, 500);
      drawArrowN(mG, 0, 127, 0);
      popMatrix();
    }
    if (true) {
      /* Roll & Pitch */
      pushMatrix();
      translate(0, height * 3 / 8, 0);
      drawRP(mRoll, mPitch);
      popMatrix();
    }
}
