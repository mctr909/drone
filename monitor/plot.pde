class Plot {
    private int mPos = 0;
    private int[][] mData;

    public Plot(int plotCount, int dataLength) {
        mData = new int[plotCount][];
        for (int i = 0; i < plotCount; i++) {
            mData[i] = new int[dataLength];
        }
    }

    public void plot(int w, int h, float...v) {
        final int PlotCount = mData.length;
        final int DataLength = mData[0].length;
        final float ScaleX = (float)w / DataLength;
        final float ScaleY = h * 0.5f / PlotCount;
        final int OfsX = w / 2;
        final int OfsY = -h * 5 / 8;
        strokeWeight(4);
        for (int i = 0; i < PlotCount; i++) {
            if (0 != v.length) {
                mData[i][mPos] = (int)(ScaleY * v[i]);
            }
            int ofsP = OfsY + (int)(i * (float)h / PlotCount);
            stroke(127,127,127);
            line( -OfsX, ofsP, OfsX, ofsP);
            switch(i % 3) {
                case 0 : stroke(0,255,0); break;
                case 1 : stroke(255,0,0); break;
                case 2 : stroke(191,0,191); break;
            }
            int xa = -OfsX;
            int ya = ofsP + mData[i][0];
            for (int t = 1; t < DataLength; t++) {
                int xb = (int)(t * ScaleX - OfsX);
                int yb = ofsP + mData[i][t];
                line(xa, ya,xb, yb);
                xa = xb;
                ya = yb;
            }
        }
        if (0 != v.length) {
            mPos++;
            if (mPos >= DataLength) {
                mPos = 0;
            }
        }
    }
}
