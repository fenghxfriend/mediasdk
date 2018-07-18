package cn.paomiantv.mediasdk.module;

import cn.paomiantv.mediasdk.common.PMConfig;

/**
 * Created by John on 2017/7/25.
 */

public class PMStoryboard extends PMModule {

    public PMStoryboard() {
        _create();
    }

    public boolean destory() {
        return _destroy();
    }

    public long getDuration() {
        return _getDuration();
    }

    public boolean addTrack(PMTrack track) {
        return _addTrack(track, (short) 0, false);
    }

    public boolean addMultiTrack(PMMultiTrack track) {
        return _addMultiTrack(track, (short) 0, false);
    }

    public boolean addTrackAtZIdex(PMTrack track, short zIndex) {
        return _addTrack(track, zIndex, true);
    }

    public boolean addMultiTrackAtZIdex(PMMultiTrack track, short zIndex) {
        return _addMultiTrack(track, zIndex, true);
    }

    public PMTrack removeTrack(int id) {
        return _removeTrack(id);
    }

    public PMMultiTrack removeMultiTrack(int id) {
        return _removeMultiTrack(id);
    }

    public boolean isMultiTrack(int id) {
        return _isMultiTrack(id);
    }

    public PMTrack getTrack(int id) {
        return _getTrack(id);
    }

    public PMMultiTrack getMultiTrack(int id) {
        return _getMultiTrack(id);
    }

    public int[] getAllTrackIds() {
        return _getAllTrackIds();
    }

    public int[] getTrackIds(PMConfig.TrackType type) {
        return _getTrackIds(type.getIndex());
    }

    public int getTrackCount(PMConfig.TrackType type) {
        return _getTrackCount(type.getIndex());
    }

    private native boolean _create();

    private native boolean _destroy();

    private native long _getDuration();

    private native boolean _addTrack(PMTrack track, short zIndex, boolean isZIndexEnable);

    private native boolean _addMultiTrack(PMMultiTrack track, short zIndex, boolean isZIndexEnable);

    private native PMTrack _removeTrack(int id);

    private native PMMultiTrack _removeMultiTrack(int id);

    private native int[] _getAllTrackIds();

    private native int[] _getTrackIds(int type);

    private native boolean _isMultiTrack(int id);

    private native PMTrack _getTrack(int id);

    private native PMMultiTrack _getMultiTrack(int id);


    private native int _getTrackCount(int type);

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _destroy();
    }
}
