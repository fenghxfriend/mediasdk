package cn.paomiantv.mediasdk.module;

/**
 * Created by John on 2017/7/28.
 */

public class PMTransition extends PMModule {

    public PMTransition(String src, long start, long duration) {
        _init(src, start, duration);
    }

    public boolean destory() {
        return _uninit();
    }

    public String getSrc() {
        return _getSrc();
    }

    public void setSrc(String src) {
        _setSrc(src);
    }

    public long getStart() {
        return _getStart();
    }

    public void setStart(long startTm) {
        _setStart(startTm);
    }

    public long getDuration() {
        return _getDuration();
    }

    public void setDuration(long duration) {
        _setDuration(duration);
    }

    private native boolean _init(String src, long start, long duration);

    private native boolean _uninit();

    private native String _getSrc();

    private native void _setSrc(String src);

    private native long _getStart();

    private native void _setStart(long start);

    private native long _getDuration();

    private native void _setDuration(long duration);
    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _uninit();
    }
}
