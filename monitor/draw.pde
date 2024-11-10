void drawArrowS(vec3 v, float s, color r, int g, int b) {
    pushMatrix();
    /*** YZ軸回転行列 ***/
    float n = s*v.getAbs();
    float pitch = v.getPitch();
    float yc = n*cos(pitch);
    float ys = n*sin(pitch);
    float yaw = v.getYaw();
    float zc = n*cos(yaw);
    float zs = n*sin(yaw);
    applyMatrix(
        zc,  ys*zs, -yc*zs, 0,
         0,  yc   ,  ys   , 0,
        zs, -ys*zc,  yc*zc, 0,
         0,      0,      0, 1
    );
    /*** モデル描画 ***/
    drawModel(50, 30, 500, r, g, b);
    popMatrix();
}

void drawArrowN(vec3 v, int r, int g, int b) {
    pushMatrix();
    /*** YZ軸回転行列 ***/
    float pitch = v.getPitch();
    float yc = cos(pitch);
    float ys = sin(pitch);
    float yaw = v.getYaw();
    float zc = cos(yaw);
    float zs = sin(yaw);
    applyMatrix(
        zc,  ys*zs, -yc*zs, 0,
         0,  yc   ,  ys   , 0,
        zs, -ys*zc,  yc*zc, 0,
         0,      0,      0, 1
    );
    /*** モデル描画 ***/
    drawModel(50, 30, 500, r, g, b);
    popMatrix();
}

void drawRPY(float roll, float pitch, float yaw) {
    pushMatrix();
    /*** 軸描画 ***/
    drawAxiz(800, 800, 800);
    /*** XYZ軸回転行列 ***/
    float xc = cos(roll);
    float xs = sin(roll);
    float yc = cos(pitch);
    float ys = sin(pitch);
    float zc = cos(yaw);
    float zs = sin(yaw);
    applyMatrix(
         xc*zc, xs*yc*zc + ys*zs, xs*ys*zc - yc*zs, 0,
        -xs   , xc*yc           , xc*ys           , 0,
         xc*zs, xs*yc*zs - ys*zc, xs*ys*zs + yc*zc, 0,
             0,                0,                0, 1
    );
    /*** モデル描画 ***/
    drawModel(800, 50, 800, 0, 255, 255);
    popMatrix();
}

void drawRP(float roll, float pitch) {
    pushMatrix();
    /*** 軸描画 ***/
    drawAxiz(1000, 500, 1000);
    /*** XY軸回転行列 ***/
    float xc = cos(roll);
    float xs = sin(roll);
    float yc = cos(pitch);
    float ys = sin(pitch);
    applyMatrix(
         xc, xs*yc, xs*ys, 0,
        -xs, xc*yc, xc*ys, 0,
          0,   -ys,    yc, 0,
          0,     0,     0, 1
    );
    /*** モデル描画 ***/
    drawModel(1000, 50, 1000, 0, 255, 255);
    popMatrix();
}

void drawY(float yaw, int r, int g, int b) {
    pushMatrix();
    /*** 軸描画 ***/
    drawAxiz(500, 500, 500);
    /*** Z軸回転行列 ***/
    float zc = cos(yaw);
    float zs = sin(yaw);
    applyMatrix(
        zc, 0, -zs, 0,
         0, 1,   0, 0,
        zs, 0,  zc, 0,
         0, 0,   0, 1
    );
    /*** モデル描画 ***/
    drawModel(50, 30, 500, r, g, b);
    popMatrix();
}

void drawAxiz(int w, int h, int d) {
    stroke(167, 167, 167);
    strokeWeight(2);
    /*** XZ平面 ***/
    line(-w*0.5f, 0, -d*0.5f, -w*0.5f, 0,  d*0.5f);
    line( w*0.5f, 0, -d*0.5f,  w*0.5f, 0,  d*0.5f);
    line(-w*0.5f, 0, -d*0.5f,  w*0.5f, 0, -d*0.5f);
    line(-w*0.5f, 0,  d*0.5f,  w*0.5f, 0,  d*0.5f);
    /*** Z軸 ***/
    line(0, 0, -d*0.5f, 0, 0, d*0.5f);
    /*** X軸 ***/
    line(-w*0.5f, 0, 0, w*0.5f, 0, 0);
    /*** Y軸 ***/
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
    specular(r, g, b);
    shininess(2.0);
    stroke(0,0,0);
    strokeWeight(2);
    /*** 底面 ***/
    beginShape();
    vertex(ax, ay, az);
    vertex(bx, by, bz);
    vertex(cx, cy, cz);
    endShape();
    /*** 背面 ***/
    beginShape();
    vertex(ax, ay, az);
    vertex(cx, cy, cz);
    vertex(dx, dy, dz);
    endShape();
    /*** 左上面 ***/
    beginShape();
    vertex(ax, ay, az);
    vertex(dx, dy, dz);
    vertex(bx, by, bz);
    endShape();
    /*** 右上面 ***/
    beginShape();
    vertex(bx, by, bz);
    vertex(dx, dy, dz);
    vertex(cx, cy, cz);
    endShape();
}
