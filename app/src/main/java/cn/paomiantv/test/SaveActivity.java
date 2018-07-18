package cn.paomiantv.test;

import android.graphics.Bitmap;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import cn.paomiantv.mediasdk.PMExporter;
import cn.paomiantv.mediasdk.PMReverser;
import cn.paomiantv.mediasdk.common.PMBitmapUtil;
import cn.paomiantv.mediasdk.common.PMConfig;
import cn.paomiantv.mediasdk.common.PMPositionUtil;
import cn.paomiantv.mediasdk.module.PMAnimation;
import cn.paomiantv.mediasdk.module.PMMultiTrack;
import cn.paomiantv.mediasdk.module.PMTrack;
import cn.paomiantv.mediasdk.module.PMEffect;
import cn.paomiantv.mediasdk.module.PMStoryboard;

public class SaveActivity extends AppCompatActivity {
    private final String background = Environment.getExternalStorageDirectory() + "/targetForeground.png";
    private final String m1 = Environment.getExternalStorageDirectory() + "/material1.mp4";
    private final String m2 = Environment.getExternalStorageDirectory() + "/material2.mp4";
    private final String user = Environment.getExternalStorageDirectory() + "/example.mp4";
    private final String e1 = Environment.getExternalStorageDirectory() + "/effect.webp";
    private final String bgm = Environment.getExternalStorageDirectory() + "/music.mp3";


    private final String v1 = Environment.getExternalStorageDirectory() + "/1.mp4";
    private final String v2 = Environment.getExternalStorageDirectory() + "/2.mp4";

    private final String maptone1 = Environment.getExternalStorageDirectory() + "/filter_moyuwei.png";

    private final String cartoon = Environment.getExternalStorageDirectory() + "/cartoon.mp4";
    private final String normal = Environment.getExternalStorageDirectory() + "/normal.mp4";
    private final String reverse = Environment.getExternalStorageDirectory() + "/reverse.mp4";

    PMStoryboard mStoryboard;
    PMExporter mExporter;
    PMReverser mReverser;

    /*相对filter开始时间的开始时间。
    如：
    clip：start=1000ms，duratin=5000ms，
    filter：start=1000ms，duration=4000ms，
    animation：start = 1000ms，duration = 2000ms

    clip从片源1000ms开始剪切，剪切时长为5000ms，即1000ms到6000ms的片段
    filter是从剪切后的数据的1000ms，持续4000ms加滤镜，相对片源则是对2000ms到6000ms加滤镜
    animation是从加的滤镜的时间1000ms后开始，持续时间是2000ms，相对片源则是3000ms到5000ms存在动画。
    */

    @Override
    protected void onCreate(final Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_save);
        findViewById(R.id.btn_save_cartoon_storyboard).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (mStoryboard != null) {
                            mStoryboard.destory();
                            mStoryboard = null;
                        }
                        initCartoonStoryboard();
                        if (mExporter != null) {
                            mExporter.cancel();
                            mExporter = null;
                        }
                        mExporter = new PMExporter(cartoon);
                        boolean re = mExporter.setVideoEncoderParams(PMConfig.OUTPUT_VIDEO_MIME_TYPE, 480, 540, PMConfig.OUTPUT_VIDEO_BIT_RATE, PMConfig.OUTPUT_VIDEO_FRAME_RATE, PMConfig.OUTPUT_VIDEO_IFRAME_INTERVAL, PMConfig.OUTPUT_VIDEO_PROFILE, PMConfig.OUTPUT_VIDEO_LEVEL);
                        if (!re) {
                            return;
                        }
                        re = mExporter.setAudioEncoderParams(PMConfig.OUTPUT_AUDIO_MIME_TYPE, PMConfig.OUTPUT_AUDIO_SAMPLE_RATE_HZ, PMConfig.OUTPUT_AUDIO_CHANNEL_COUNT, PMConfig.OUTPUT_AUDIO_BIT_RATE, PMConfig.OUTPUT_AUDIO_AAC_PROFILE);
                        if (!re) {
                            return;
                        }
                        mExporter.setDataSource(mStoryboard);
                        mExporter.save("this is cartoon test");


                    }
                }).start();
            }
        });


        findViewById(R.id.btn_cancel_cartoon_storyboard).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mExporter != null) {
                    mExporter.cancel();
                    mExporter = null;
                }
            }
        });
        findViewById(R.id.btn_save_normal_storyboard).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (mStoryboard != null) {
                            mStoryboard.destory();
                            mStoryboard = null;
                        }
                        initNoramlStoryboard();
                        if (mExporter != null) {
                            mExporter.cancel();
                            mExporter = null;
                        }
                        mExporter = new PMExporter(normal);
                        boolean re = mExporter.setVideoEncoderParams(PMConfig.OUTPUT_VIDEO_MIME_TYPE, PMConfig.OUTPUT_VIDEO_HEIGHT, PMConfig.OUTPUT_VIDEO_WIDTH, PMConfig.OUTPUT_VIDEO_BIT_RATE, PMConfig.OUTPUT_VIDEO_FRAME_RATE, PMConfig.OUTPUT_VIDEO_IFRAME_INTERVAL, PMConfig.OUTPUT_VIDEO_PROFILE, PMConfig.OUTPUT_VIDEO_LEVEL);
                        if (!re) {
                            return;
                        }
                        re = mExporter.setAudioEncoderParams(PMConfig.OUTPUT_AUDIO_MIME_TYPE, PMConfig.OUTPUT_AUDIO_SAMPLE_RATE_HZ, PMConfig.OUTPUT_AUDIO_CHANNEL_COUNT, PMConfig.OUTPUT_AUDIO_BIT_RATE, PMConfig.OUTPUT_AUDIO_AAC_PROFILE);
                        if (!re) {
                            return;
                        }
                        mExporter.setDataSource(mStoryboard);
                        mExporter.save("this is normal test");
                    }
                }).start();
            }
        });


        findViewById(R.id.btn_cancel_normal_storyboard).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mExporter != null) {
                    mExporter.cancel();
                    mExporter = null;
                }
            }
        });

        findViewById(R.id.btn_reverse_save).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (mReverser != null) {
                            mReverser.cancel();
                            mReverser = null;
                        }
                        mReverser = new PMReverser(reverse);
                        mReverser.reverse(v1,true,false,"this is reverse test");
                    }
                }).start();
            }
        });


        findViewById(R.id.btn_reverse_cancel).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mReverser != null) {
                    mReverser.cancel();
                    mReverser = null;
                }
            }
        });
    }

    private void initCartoonStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }
        mStoryboard = new PMStoryboard();
        PMTrack track = null;
        PMAnimation animation = null;

        // the 1st material;
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(m1);
        track.setShowLastFrame(true);

        //[0.127083, 0.359259, 0.85, 0.288889]
        float[] position = new float[4];
        position[0] = 0.127083f;
        position[1] = 0.359259f;
        position[2] = 0.127083f + 0.85f;
        position[3] = 0.359259f + 0.288889f;
        position = PMPositionUtil.transform(position);
        animation = new PMAnimation();
        animation.setStartScaleX(0.85f);
        animation.setEndScaleX(0.85f);
        animation.setStartScaleY(0.288889f);
        animation.setEndScaleY(0.288889f);
        animation.setStartX((position[2] + position[0]) / 2.f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);

//        animation.setStartScaleX(0.69583f);
//        animation.setEndScaleX(0.69583f);
//        animation.setStartScaleY(0.2926f);
//        animation.setEndScaleY(0.2926f);
//        animation.setStartX(0.1792f);
//        animation.setEndX(0.1792f);
//        animation.setStartY(0.f);
//        animation.setEndY(0.f);

        track.addAnimation(animation);

        mStoryboard.addTrack(track);

        // the 2nd material
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(m2);
        track.setShowLastFrame(true);

        //[0.03125, 0.681481, 0.85, 0.288889]
        position[0] = 0.03125f;
        position[1] = 0.681481f;
        position[2] = 0.03125f + 0.85f;
        position[3] = 0.681481f + 0.288889f;
        position = PMPositionUtil.transform(position);
        animation = new PMAnimation();
        animation.setStartScaleX(0.85f);
        animation.setEndScaleX(0.85f);
        animation.setStartScaleY(0.288889f);
        animation.setEndScaleY(0.288889f);
        animation.setStartX((position[2] + position[0]) / 2.f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);

        track.addAnimation(animation);
        mStoryboard.addTrack(track);

        // user track
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(user);
        track.setShowLastFrame(true);

        //[0.03125, 0.037037, 0.85, 0.288889]
        position[0] = 0.03125f;
        position[1] = 0.037037f;
        position[2] = 0.03125f + 0.85f;
        position[3] = 0.037037f + 0.288889f;
        position = PMPositionUtil.transform(position);
        animation = new PMAnimation();
        animation.setStartScaleX(0.85f);
        animation.setEndScaleX(0.85f);
        animation.setStartScaleY(0.288889f);
        animation.setEndScaleY(0.288889f);
        animation.setStartX((position[2] + position[0]) / 2.f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);

        track.addAnimation(animation);
        mStoryboard.addTrack(track);

        // foreground
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        Bitmap foreground = PMBitmapUtil.decodeFile(background, 1);
        track.setDataSource(foreground);
        mStoryboard.addTrack(track);


        // webp
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSourceWebP(e1);
        track.setShowLastFrame(true);
        mStoryboard.addTrack(track);
        track.setShowLastFrame(true);


        // bgm
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        track.setDataSource(bgm);
        mStoryboard.addTrack(track);

    }

    private void initNoramlStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }

        mStoryboard = new PMStoryboard();
        PMMultiTrack multiTrack = new PMMultiTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        PMTrack track = null;
        PMEffect effect = null;
        PMAnimation animation = null;

        Bitmap lut = PMBitmapUtil.decodeFile(maptone1, 1);
        effect = new PMEffect(lut, PMConfig.EffectType.EM_EFFECT_TRANSFORM_COLOR, 0, -1);
        lut.recycle();

        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(v1);
//        track.setCutStart(2000);
//        track.setCutDuration(5000);
//        track.addEffect(effect);
        multiTrack.pushTrack(track);

//        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
//        track.setDataSource(v2);
////        track.setCutStart(2000);
////        track.setCutDuration(5000);
////        track.addEffect(effect);
//        multiTrack.pushTrack(track);

        mStoryboard.addMultiTrack(multiTrack);


        multiTrack = new PMMultiTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        track.setDataSource(v1);
//        track.setCutStart(2000);
//        track.setCutDuration(5000);
        multiTrack.pushTrack(track);

//        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
//        track.setDataSource(v2);
////        track.setCutStart(2000);
////        track.setCutDuration(5000);
//        multiTrack.pushTrack(track);

        mStoryboard.addMultiTrack(multiTrack);
    }
}
