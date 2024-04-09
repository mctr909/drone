class vec3 {
    public float x;
    public float y;
    public float z;

    public vec3() {
        x = 0;
        y = 0;
        z = 0;
    }
    public vec3(vec3 v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
    }
    public vec3(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public float getAbs() {
        return sqrt(x * x + y * y + z * z);
    }
    public float getYaw() {
        return atan2(y, x);
    }
    public float getPitch() {
        return atan2(-z, sqrt(x * x + y * y));
    }

    public vec3 normalize() {
        float r = sqrt(x * x + y * y + z * z);
        if (r > 0) {
            return new vec3(x / r, y / r, z / r);
        } else {
            return new vec3();
        }
    }
    public vec3 scale(float k) {
        return new vec3(x * k, y * k, z * k);
    }
}
