package cn.paomiantv.mediasdk.module;

/**
 * Created by John on 2017/9/14.
 */

public class PMAnimation extends PMModule {

    public PMAnimation() {
        _create();
    }

    public boolean destory() {
        return _destroy();
    }


    public float getStartX() {
        return _get_start_X();
    }

    public void setStartX(float m_start_X) {
        _set_start_X(m_start_X);
    }

    public float getStartY() {
        return _get_start_Y();
    }

    public void setStartY(float m_start_Y) {
        _set_start_Y(m_start_Y);
    }

    public float getStartDegreeZ() {
        return _get_start_degree_Z();
    }

    public void setStartDegreeZ(float m_start_Degree) {
        _set_start_degree_Z(m_start_Degree);
    }

    public float getStartScaleX() {
        return _get_start_scale_X();
    }

    public void setStartScaleX(float m_start_Scale_X) {
        _set_start_scale_X(m_start_Scale_X);
    }

    public float getStartScaleY() {
        return _get_start_scale_Y();
    }

    public void setStartScaleY(float m_start_Scale_Y) {
        _set_start_scale_Y(m_start_Scale_Y);
    }

    public float getEndX() {
        return _get_end_X();
    }

    public void setEndX(float m_end_X) {
        _set_end_X(m_end_X);
    }

    public float getEndY() {
        return _get_end_Y();
    }

    public void setEndY(float m_end_Y) {
        _set_end_Y(m_end_Y);
    }

    public float getEndDegreeZ() {
        return _get_end_degree_Z();
    }

    public void setEndDegreeZ(float m_end_Degree) {
        _set_end_degree_Z(m_end_Degree);
    }

    public float getEndScaleX() {
        return _get_end_scale_X();
    }

    public void setEndScaleX(float m_end_Scale_X) {
        _set_end_scale_X(m_end_Scale_X);
    }

    public float getEndScaleY() {
        return _get_end_scale_Y();
    }

    public void setEndScaleY(float m_end_Scale_Y) {
        _set_end_scale_Y(m_end_Scale_Y);
    }


    public float getCropStartX() {
        return _get_crop_start_X();
    }

    public void setCropStartX(float m_start_X) {
        _set_crop_start_X(m_start_X);
    }

    public float getCropStartY() {
        return _get_crop_start_Y();
    }

    public void setCropStartY(float m_start_Y) {
        _set_crop_start_Y(m_start_Y);
    }

    public float getCropStartDegreeZ() {
        return _get_crop_start_degree_Z();
    }

    public void setCropStartDegreeZ(float m_start_Degree) {
        _set_crop_start_degree_Z(m_start_Degree);
    }

    public float getCropStartScaleX() {
        return _get_crop_start_scale_X();
    }

    public void setCropStartScaleX(float m_start_Scale_X) {
        _set_crop_start_scale_X(m_start_Scale_X);
    }

    public float getCropStartScaleY() {
        return _get_crop_start_scale_Y();
    }

    public void setCropStartScaleY(float m_start_Scale_Y) {
        _set_crop_start_scale_Y(m_start_Scale_Y);
    }

    public float getCropEndX() {
        return _get_crop_end_X();
    }

    public void setCropEndX(float m_end_X) {
        _set_crop_end_X(m_end_X);
    }

    public float getCropEndY() {
        return _get_crop_end_Y();
    }

    public void setCropEndY(float m_end_Y) {
        _set_crop_end_Y(m_end_Y);
    }

    public float getCropEndDegreeZ() {
        return _get_crop_end_degree_Z();
    }

    public void setCropEndDegreeZ(float m_end_Degree) {
        _set_crop_end_degree_Z(m_end_Degree);
    }

    public float getCropEndScaleX() {
        return _get_crop_end_scale_X();
    }

    public void setCropEndScaleX(float m_end_Scale_X) {
        _set_crop_end_scale_X(m_end_Scale_X);
    }

    public float getCropEndScaleY() {
        return _get_crop_end_scale_Y();
    }

    public void setCropEndScaleY(float m_end_Scale_Y) {
        _set_crop_end_scale_Y(m_end_Scale_Y);
    }


    public float getStartAlpha() {
        return _get_start_alpha();
    }

    public void setStartAlpha(float m_start_Alpha) {
        _set_start_alpha(m_start_Alpha);
    }

    public float getEndAlpha() {
        return _get_end_alpha();
    }

    public void setEndAlpha(float m_end_Alpha) {
        _set_end_alpha(m_end_Alpha);
    }

    /*相对filter开始时间的开始时间。
    如：
    clip：start=1000ms，duratin=5000ms，
    filter：start=1000ms，duration=4000ms，
    animation：start = 1000ms，duration = 2000ms

    clip从片源1000ms开始剪切，剪切时长为5000ms，即1000ms到6000ms的片段
    filter是从剪切后的数据的1000ms，持续4000ms加滤镜，相对片源则是对2000ms到6000ms加滤镜
    animation是从加的滤镜的时间1000ms后开始，持续时间是2000ms，相对片源则是3000ms到5000ms存在动画。
    */
    public long getStart() {
        return _get_start();
    }

    public void setStart(long start) {
        _set_start(start);
    }

    public long getDuration() {
        return _get_duration();
    }

    public void setDuration(long duration) {
        _set_duration(duration);
    }

    private native boolean _create();

    private native boolean _destroy();

    private native float _get_start_X();

    private native void _set_start_X(float start_X);

    private native float _get_start_Y();

    private native void _set_start_Y(float start_Y);

    private native float _get_start_degree_Z();

    private native void _set_start_degree_Z(float start_degree);

    private native float _get_start_scale_X();

    private native void _set_start_scale_X(float m_start_Scale_X);

    private native float _get_start_scale_Y();

    private native void _set_start_scale_Y(float m_start_Scale_Y);

    private native float _get_end_X();

    private native void _set_end_X(float end_X);

    private native float _get_end_Y();

    private native void _set_end_Y(float end_Y);

    private native float _get_end_degree_Z();

    private native void _set_end_degree_Z(float end_degree);

    private native float _get_end_scale_X();

    private native void _set_end_scale_X(float end_scale_X);

    private native float _get_end_scale_Y();

    private native void _set_end_scale_Y(float end_scale_Y);


    private native float _get_crop_start_X();

    private native void _set_crop_start_X(float start_X);

    private native float _get_crop_start_Y();

    private native void _set_crop_start_Y(float start_Y);

    private native float _get_crop_start_degree_Z();

    private native void _set_crop_start_degree_Z(float start_degree);

    private native float _get_crop_start_scale_X();

    private native void _set_crop_start_scale_X(float m_start_Scale_X);

    private native float _get_crop_start_scale_Y();

    private native void _set_crop_start_scale_Y(float m_start_Scale_Y);

    private native float _get_crop_end_X();

    private native void _set_crop_end_X(float end_X);

    private native float _get_crop_end_Y();

    private native void _set_crop_end_Y(float end_Y);

    private native float _get_crop_end_degree_Z();

    private native void _set_crop_end_degree_Z(float end_degree);

    private native float _get_crop_end_scale_X();

    private native void _set_crop_end_scale_X(float end_scale_X);

    private native float _get_crop_end_scale_Y();

    private native void _set_crop_end_scale_Y(float end_scale_Y);


    private native float _get_start_alpha();

    private native void _set_start_alpha(float start_alpha);

    private native float _get_end_alpha();

    private native void _set_end_alpha(float end_alpha);

    private native long _get_start();

    private native void _set_start(long start);

    private native long _get_duration();

    private native void _set_duration(long duration);

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _destroy();
    }
}
