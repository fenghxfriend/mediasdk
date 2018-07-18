package cn.paomiantv.mediasdk;

import cn.paomiantv.mediasdk.module.PMModule;

/**
 * Created by ASUS on 2018/1/4.
 */

public class PMMuxer extends PMModule {
    public PMMuxer(String dst) {
        _init(dst);
    }

    public boolean setDescription(String dsc) {
        return _setTags(dsc);
    }

    public boolean destory() {
        return _uninit();
    }

    public String getDst() {
        return _getDst();
    }

    public boolean writeH264Frame(byte[] data, int size, int type, long pts, boolean isEOS) {
        return _writeH264Frame(data, size, type, pts, isEOS);
    }

    public boolean writeAACFrame(byte[] data, int size, long pts, boolean isEOS) {
        return _writeAACFrame(data, size, pts, isEOS);
    }

    public int addH264VideoTrack(int width, int height, byte level, byte[] sps, short spsLen, byte[] pps, short ppsLen) {
        return _addH264VideoTrack(width, height, level, sps, spsLen, pps, ppsLen);
    }

    public int addAACAudioTrack(int sampleHZ, byte level, byte[] esds, short esdsLen) {
        return _addAACAudioTrack(sampleHZ, level, esds, esdsLen);
    }

//    public void copyFromSource(String source, int trackType, long starttimeUs, long durationUs) {
//        _copyFromSource(source, trackType, starttimeUs, durationUs);
//    }

    private native boolean _init(String dst);

    private native boolean _setTags(String dsc);

    private native boolean _uninit();

    private native String _getDst();

    private native boolean _writeH264Frame(byte[] data, int size, int type, long pts, boolean isEOS);

    private native boolean _writeAACFrame(byte[] data, int size, long pts, boolean isEOS);

    private native int _addH264VideoTrack(int width, int height, byte level, byte[] sps, short spsLen, byte[] pps, short ppsLen);

    private native int _addAACAudioTrack(int sampleHZ, byte level, byte[] esds, short esdsLen);

//    private native void _copyFromSource(String source, int trackType, long starttimeUs, long durationUs);


}
