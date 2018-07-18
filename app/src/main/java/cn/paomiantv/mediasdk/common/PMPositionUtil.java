package cn.paomiantv.mediasdk.common;

public class PMPositionUtil {
    public static float[] transform(float[] position) {
        if (position.length % 2 == 0) {
            float[] tmp = new float[position.length];
            for (int i = 0; i < position.length / 2; i++) {
                tmp[i * 2] = 2.f * position[i * 2] - 1.f;
                tmp[i * 2 + 1] = 1.f - (2.f * position[i * 2 + 1]);
            }
            return tmp;
        }
        return position;
    }

}
