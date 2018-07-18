package cn.paomiantv.mediasdk.module;

import cn.paomiantv.mediasdk.common.PMConfig;

/**
 * Created by John on 2017/7/21.
 */

public class PMMultiTrack extends PMModule {
    public PMMultiTrack(PMConfig.TrackType type) {
        _create(type.getIndex());
    }

    public boolean destory() {
        return _destroy();
    }

    public int getId() {
        return _getId();
    }

    public PMConfig.TrackType getType() {
        return PMConfig.TrackType.getType(_getType());
    }


    public void setPlayStart(long startPlayTm) {
        _setPlayStart(startPlayTm);
    }

    public long getPlayStart() {
        return _getPlayStart();
    }

    public void setPlayDuration(long duration) {
        _setPlayDuration(duration);
    }

    public long getPlayDuration() {
        return _getPlayDuration();
    }

    public void setWeight(short weight) {
        _setWeight(weight);
    }

    public short getWeight() {
        return _getWeight();
    }

//    public void setZindex(short Zindex) {
//        _setZIndex(Zindex);
//    }

    public short getZindex() {
        return _getZIndex();
    }


    public PMTrack getTrack(int position) {
        return _getTrack(position);
    }

    public boolean pushTrack(PMTrack t) {
        return _pushTrack(t);
    }

    public PMTrack popTrack(boolean isShouldDestroy) {
        return _popTrack(isShouldDestroy);
    }

    public boolean insertTrack(PMTrack t, int position) {
        return _insertTrack(t, position);
    }

    public PMTrack removeTrack(int position) {
        return _removeTrack(position);
    }

    public int getTrackCount() {
        return _getTrackCount();
    }


    public PMTransition getTransition(int position) {
        return _getTransition(position);
    }

    public boolean addAnimation(PMTransition transition, int position) {
        return _addTransition(transition, position);
    }

    public PMTransition removeTransition(int position) {
        return _removeTransition(position);
    }

    public long getDataDuration() {
        return _getDataDuration();
    }

    private native boolean _create(int type);

    private native boolean _destroy();

    private native int _getId();

    private native int _getType();


    private native void _setPlayStart(long playStart);

    private native long _getPlayStart();

    private native void _setPlayDuration(long playDuration);

    private native long _getPlayDuration();


    private native void _setWeight(short weight);

    private native short _getWeight();

    private native void _setZIndex(short Zindex);

    private native short _getZIndex();


    private native PMTrack _getTrack(int position);

    private native boolean _pushTrack(PMTrack track);

    private native PMTrack _popTrack(boolean isShouldDestroy);

    private native boolean _insertTrack(PMTrack track, int position);

    private native PMTrack _removeTrack(int position);

    private native int _getTrackCount();


    private native boolean _addTransition(PMTransition transition, int position);

    private native PMTransition _getTransition(int index);

    private native PMTransition _removeTransition(int index);


    private native long _getDataDuration();

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _destroy();
    }
}
