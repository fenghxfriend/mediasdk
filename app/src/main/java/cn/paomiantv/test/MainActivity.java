package cn.paomiantv.test;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import org.json.JSONException;
import org.json.JSONObject;

import cn.paomiantv.mediasdk.PMNativePlayer;
import cn.paomiantv.mediasdk.PMPlayer;
import cn.paomiantv.mediasdk.common.PMBitmapUtil;
import cn.paomiantv.mediasdk.common.PMError;
import cn.paomiantv.mediasdk.common.PMMessage;
import cn.paomiantv.mediasdk.common.PMPositionUtil;
import cn.paomiantv.mediasdk.module.PMAnimation;
import cn.paomiantv.mediasdk.module.PMMultiTrack;
import cn.paomiantv.mediasdk.module.PMTrack;
import cn.paomiantv.mediasdk.module.PMEffect;
import cn.paomiantv.mediasdk.PMEngine;
import cn.paomiantv.mediasdk.module.PMStoryboard;
import cn.paomiantv.mediasdk.common.PMConfig;

import static java.lang.Thread.sleep;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final int PROGRESS = 1;
    private static final int COMPLETE = 2;
    private static final int STOPPED = 3;
    private static final int FAILED = 4;
    private static final String TAG = MainActivity.class.getSimpleName();


    private final String foreground = Environment.getExternalStorageDirectory() + "/targetForeground.png";
    private final String m1 = Environment.getExternalStorageDirectory() + "/material1.mp4";
    private final String m2 = Environment.getExternalStorageDirectory() + "/material2.mp4";
    private final String user = Environment.getExternalStorageDirectory() + "/example.mp4";
    private final String e1 = Environment.getExternalStorageDirectory() + "/effect.webp";
    private final String bgm = Environment.getExternalStorageDirectory() + "/music.mp3";
    private final String background = Environment.getExternalStorageDirectory() + "/back.png";

    private final String sample = Environment.getExternalStorageDirectory() + "/sample.mp4";


    //private final String v1 = Environment.getExternalStorageDirectory() + "/VID_20180306_185247.mp4";
    private final String v1 = Environment.getExternalStorageDirectory() + "/1.mp4";

    private final String v2 = Environment.getExternalStorageDirectory() + "/2.mp4";
    private final String syncPath = Environment.getExternalStorageDirectory() + "/1.mp4";

    private final String maptone1 = Environment.getExternalStorageDirectory() + "/filter/filter_moyuwei.png";

    private final String mask = Environment.getExternalStorageDirectory() + "/mask.png";

    private final String v1080p = Environment.getExternalStorageDirectory() + "/VID_20180306_185247.mp4";


    private final String mask1 = Environment.getExternalStorageDirectory() + "/2-1.png";
    private final String mask2 = Environment.getExternalStorageDirectory() + "/2-2.png";
    private final String maskSource1 = Environment.getExternalStorageDirectory() + "/2-1.mp4";
    private final String maskSource2 = Environment.getExternalStorageDirectory() + "/2-2.mp4";

    private TextView mtvPreogress;

    private TextView mtvTrackIDs;
    private EditText metTrackID;
    private EditText metVolume;
    private Button mbtnSave;
    private Button mbtnStart;
    private Button mbtnStop;
    private Button mbtnPause;
    private Button mbtnResume;
    private Button mbtnSeekTo;
    private Button mbtnVolume;
    private Button mbtnClipFilter;
    private Button mbtnClipFilterNull;
    private Button mbtnSeek;
    private Button mbtnNormal;
    private Button mbtnCartoon;
    private Button mbtnMask;
    private Button mbtnSingle;
    private ImageView mivThumb;

    private int clip = 0;
    //    private PMGLSurfaceView mglvPlayer;
    PMStoryboard mStoryboard;
    private PMPlayer mPlayer;
    private int progress = 0;

    private Thread thread;
    private Button mbtnRemoveT;
    private long count = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PMEngine.getInstance().init(this);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        mtvPreogress = (TextView) findViewById(R.id.tv_progress);
        mtvTrackIDs = (TextView) findViewById(R.id.tv_track_id);
        metTrackID = (EditText) findViewById(R.id.et_track_id);
        metVolume = (EditText) findViewById(R.id.et_volume);
        mbtnSave = (Button) findViewById(R.id.btn_save);
        mbtnStart = (Button) findViewById(R.id.btn_preview_start);
        mbtnStop = (Button) findViewById(R.id.btn_preview_stop);
        mbtnPause = (Button) findViewById(R.id.btn_preview_pause);
        mbtnResume = (Button) findViewById(R.id.btn_preview_resume);
        mbtnSeekTo = (Button) findViewById(R.id.btn_preview_seekto);
        mbtnVolume = (Button) findViewById(R.id.btn_volume);
        mbtnClipFilter = (Button) findViewById(R.id.btn_change_filter);
        mbtnClipFilterNull = (Button) findViewById(R.id.btn_change_filter_null);
        mbtnSeek = (Button) findViewById(R.id.btn_seek);
        mivThumb = (ImageView) findViewById(R.id.thumb);
        mbtnNormal = (Button) findViewById(R.id.btn_init_normal);
        mbtnCartoon = (Button) findViewById(R.id.btn_init_cartoon);
        mbtnMask = (Button) findViewById(R.id.btn_init_mask);
        mbtnSingle = (Button) findViewById(R.id.btn_init_single);
        mbtnRemoveT = (Button) findViewById(R.id.btn_release);


        mPlayer = (PMPlayer) findViewById(R.id.id_player);
        mbtnSave.setOnClickListener(this);
        mbtnStart.setOnClickListener(this);
        mbtnStop.setOnClickListener(this);
        mbtnPause.setOnClickListener(this);
        mbtnResume.setOnClickListener(this);
        mbtnSeekTo.setOnClickListener(this);
        mbtnVolume.setOnClickListener(this);
        mbtnClipFilter.setOnClickListener(this);
        mbtnClipFilterNull.setOnClickListener(this);
        mbtnSeek.setOnClickListener(this);
        mbtnNormal.setOnClickListener(this);
        mbtnCartoon.setOnClickListener(this);
        mbtnMask.setOnClickListener(this);
        mbtnSingle.setOnClickListener(this);
        mbtnRemoveT.setOnClickListener(this);
        verifyStoragePermissions(this);
        verifyAudioSettingPermissions(this);

        //initSingleStoryboard();
    }

    private void initSingleStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }
        mStoryboard = new PMStoryboard();
        PMTrack track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        if (track.setDataSource(syncPath)) {
            mStoryboard.addTrack(track);
        } else {
            track.destory();
        }

        Bitmap map = PMBitmapUtil.decodeFile(this.maptone1, 1);
        Bitmap mask = PMBitmapUtil.decodeFile(this.mask, 1);

        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(syncPath);

        PMEffect effect = null;

        effect = new PMEffect(null, PMConfig.EffectType.EM_EFFECT_BLUR, 0, -1);
        track.addEffect(effect);

        effect = new PMEffect(mask, PMConfig.EffectType.EM_EFFECT_MASK, 0, -1);
        track.addEffect(effect);

        effect = new PMEffect(map, PMConfig.EffectType.EM_EFFECT_TRANSFORM_COLOR, 0, -1);
        track.addEffect(effect);

        effect = new PMEffect(mask, PMConfig.EffectType.EM_EFFECT_MASK, 0, -1);
        track.addEffect(effect);




        mStoryboard.addTrack(track);
        mask.recycle();
        map.recycle();
    }

    private void initCartoonStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }
        mStoryboard = new PMStoryboard();
        PMTrack track = null;
        PMAnimation animation = null;

        // background
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        Bitmap background = PMBitmapUtil.decodeFile(this.background, 1);
        track.setDataSource(background);
        background.recycle();
        mStoryboard.addTrack(track);

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

        track.addAnimation(animation);
        animation = new PMAnimation();
        animation.setStart(0);
        animation.setDuration(1000);
        animation.setStartScaleX(0.85f);
        animation.setEndScaleX(0.85f);
        animation.setStartScaleY(0.288889f);
        animation.setEndScaleY(0.288889f);
        animation.setStartX(-0.85f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);
        animation.setStartAlpha(0.0f);
        animation.setEndAlpha(1.0f);
        track.addAnimation(animation);

        Bitmap mask = PMBitmapUtil.decodeFile(this.mask, 1);
        PMEffect effect = new PMEffect(mask, PMConfig.EffectType.EM_EFFECT_MASK, 0, -1);
        mask.recycle();
        track.addEffect(effect);

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
        Bitmap foreground = PMBitmapUtil.decodeFile(this.foreground, 1);
        track.setDataSource(foreground);
        foreground.recycle();
        mStoryboard.addTrack(track);


        // webp
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSourceWebP(e1);
        track.setShowLastFrame(true);
        mStoryboard.addTrack(track);

//        // bgm
//        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
//        track.setDataSource(bgm);
//        mStoryboard.addTrack(track);

        String ids = "Audio:";
        int id[] = mStoryboard.getTrackIds(PMConfig.TrackType.EM_TRACK_AUDIO);
        for (int i = 0; i < id.length; i++) {
            if (mStoryboard.isMultiTrack(id[i])) {
                PMMultiTrack mt = mStoryboard.getMultiTrack(id[i]);
                ids += "m{";
                for (int j = 0; j < mt.getTrackCount(); j++) {
                    PMTrack t = mt.getTrack(j);
                    ids += (t.getId() + ",");
                }
                ids += "}, ";
            } else {
                PMTrack t = mStoryboard.getTrack(id[i]);
                ids += (t.getId() + ",");
            }
        }

        mtvTrackIDs.setText(ids);

        ids = "Video:";
        id = mStoryboard.getTrackIds(PMConfig.TrackType.EM_TRACK_VIDEO);
        for (int i = 0; i < id.length; i++) {
            if (mStoryboard.isMultiTrack(id[i])) {
                PMMultiTrack mt = mStoryboard.getMultiTrack(id[i]);
                ids += "m{";
                for (int j = 0; j < mt.getTrackCount(); j++) {
                    PMTrack t = mt.getTrack(j);
                    ids += (t.getId() + ":" + t.getWidth() + "*" + t.getHeight() + ",");
                }
                ids += "}, ";
            } else {
                PMTrack t = mStoryboard.getTrack(id[i]);
                ids += (t.getId() + ":" + t.getWidth() + "*" + t.getHeight() + ",");
            }
        }
        Log.e(TAG, ids);

    }

    private void initMaskStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }
        mStoryboard = new PMStoryboard();
        PMTrack track = null;
        PMAnimation animation = null;

        // background
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        Bitmap background = PMBitmapUtil.decodeFile(this.background, 1);
        track.setDataSource(background);
        background.recycle();
        mStoryboard.addTrack(track);

        // the 1st material;
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(maskSource1);
        track.setShowLastFrame(true);

        //[0.0, 0.0, 1.0, 0.65]
        float[] position = new float[4];
        position[0] = 0.0f;
        position[1] = 0.0f;
        position[2] = 1.0f;
        position[3] = 0.65f;
        position = PMPositionUtil.transform(position);
        animation = new PMAnimation();
        animation.setStartScaleY(0.65f);
        animation.setEndScaleY(0.65f);
        animation.setStartX((position[2] + position[0]) / 2.f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);

        track.addAnimation(animation);

        Bitmap mask1 = PMBitmapUtil.decodeFile(this.mask1, 1);
        PMEffect effect = new PMEffect(mask1, PMConfig.EffectType.EM_EFFECT_MASK, 0, -1);
        mask1.recycle();
        track.addEffect(effect);

        mStoryboard.addTrack(track);

        // the 2nd material
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(maskSource2);
        track.setShowLastFrame(true);

        //[0.0, 0.35, 1.0, 0.65]
        //[0.03125, 0.681481, 0.85, 0.288889]
        position[0] = 0.0f;
        position[1] = 0.35f;
        position[2] = 1.0f;
        position[3] = 0.35f + 0.65f;
        position = PMPositionUtil.transform(position);
        animation = new PMAnimation();
        animation.setStartScaleY(0.65f);
        animation.setEndScaleY(0.65f);
        animation.setStartX((position[2] + position[0]) / 2.f);
        animation.setEndX((position[2] + position[0]) / 2.f);
        animation.setStartY((position[3] + position[1]) / 2.f);
        animation.setEndY((position[3] + position[1]) / 2.f);

        track.addAnimation(animation);

        Bitmap mask2 = PMBitmapUtil.decodeFile(this.mask2, 1);
        effect = new PMEffect(mask2, PMConfig.EffectType.EM_EFFECT_MASK, 0, -1);
        mask2.recycle();
        track.addEffect(effect);

        mStoryboard.addTrack(track);


        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        track.setDataSource(bgm);
        track.setCutStart(2000);
        track.setCutDuration(5000);
        mStoryboard.addTrack(track);

        String ids = "Audio:";
        int id[] = mStoryboard.getTrackIds(PMConfig.TrackType.EM_TRACK_AUDIO);
        for (int i = 0; i < id.length; i++) {
            if (mStoryboard.isMultiTrack(id[i])) {
                PMMultiTrack mt = mStoryboard.getMultiTrack(id[i]);
                ids += "m{";
                for (int j = 0; j < mt.getTrackCount(); j++) {
                    PMTrack t = mt.getTrack(j);
                    ids += (t.getId() + ",");
                }
                ids += "}, ";
            } else {
                PMTrack t = mStoryboard.getTrack(id[i]);
                ids += (t.getId() + ",");
            }
        }

        mtvTrackIDs.setText(ids);

        ids = "Video:";
        id = mStoryboard.getTrackIds(PMConfig.TrackType.EM_TRACK_VIDEO);
        for (int i = 0; i < id.length; i++) {
            if (mStoryboard.isMultiTrack(id[i])) {
                PMMultiTrack mt = mStoryboard.getMultiTrack(id[i]);
                ids += "m{";
                for (int j = 0; j < mt.getTrackCount(); j++) {
                    PMTrack t = mt.getTrack(j);
                    ids += (t.getId() + ":" + t.getWidth() + "*" + t.getHeight() + ",");
                }
                ids += "}, ";
            } else {
                PMTrack t = mStoryboard.getTrack(id[i]);
                ids += (t.getId() + ":" + t.getWidth() + "*" + t.getHeight() + ",");
            }
        }
        Log.e(TAG, ids);

    }

    private void initNoramlStoryboard() {
        if (mStoryboard != null) {
            mStoryboard.destory();
        }

        mStoryboard = new PMStoryboard();
        PMMultiTrack vmultiTrack = new PMMultiTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        PMTrack track = null;
        PMEffect effect = null;
        PMAnimation animation = null;

        Bitmap lut = PMBitmapUtil.decodeFile(maptone1, 1);
        effect = new PMEffect(lut, PMConfig.EffectType.EM_EFFECT_TRANSFORM_COLOR, 0, -1);
        lut.recycle();

        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(v1);
        track.setCutStart(0);
        track.setCutDuration(5000);
//        animation = new PMAnimation();
//        animation.setStartDegreeZ(90);
//        animation.setEndDegreeZ(90);
//        track.addAnimation(animation);
//        track.addEffect(effect);
        vmultiTrack.pushTrack(track);

        track = new PMTrack(PMConfig.TrackType.EM_TRACK_VIDEO);
        track.setDataSource(v2);
        track.setCutStart(2000);
        track.setCutDuration(5000);
//        track.addEffect(effect);
        vmultiTrack.pushTrack(track);

        mStoryboard.addMultiTrack(vmultiTrack);
//
//
        PMMultiTrack amultiTrack = new PMMultiTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        if (!track.setDataSource(v1)) {
            track.destory();
            track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
            track.setDataSourceSilence();
        }
        track.setCutStart(0);
        track.setCutDuration(5000);
        amultiTrack.pushTrack(track);

        track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
        if (!track.setDataSource(v2)) {
            track.destory();
            track = new PMTrack(PMConfig.TrackType.EM_TRACK_AUDIO);
            track.setDataSourceSilence();
        }
        track.setCutStart(2000);
        track.setCutDuration(5000);
        amultiTrack.pushTrack(track);

        mStoryboard.addMultiTrack(amultiTrack);


        String ids = "trackID:";
        int id[] = mStoryboard.getTrackIds(PMConfig.TrackType.EM_TRACK_AUDIO);
        for (int i = 0; i < id.length; i++) {
            if (mStoryboard.isMultiTrack(id[i])) {
                PMMultiTrack mt = mStoryboard.getMultiTrack(id[i]);
                ids += "m{";
                for (int j = 0; j < mt.getTrackCount(); j++) {
                    PMTrack t = mt.getTrack(j);
                    ids += (t.getId() + ",");
                }
                ids += "}, ";
            } else {
                PMTrack t = mStoryboard.getTrack(id[i]);
                ids += (t.getId() + ",");
            }
        }

        mtvTrackIDs.setText(ids);
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_init_single: {
                initSingleStoryboard();
                mPlayer.setDataSource(mStoryboard);
                mPlayer.setMessageHandler(mHandler);
                mPlayer.prepare();
            }
            break;
            case R.id.btn_init_normal: {
                initNoramlStoryboard();
                mPlayer.setDataSource(mStoryboard);
                mPlayer.setMessageHandler(mHandler);
                mPlayer.prepare();
            }
            break;
            case R.id.btn_init_cartoon: {
                initCartoonStoryboard();
                mPlayer.setDataSource(mStoryboard);
                mPlayer.setMessageHandler(mHandler);
                mPlayer.prepare();
            }
            break;
            case R.id.btn_init_mask: {
                initMaskStoryboard();
                mPlayer.setDataSource(mStoryboard);
                mPlayer.setMessageHandler(mHandler);
                mPlayer.prepare();
            }
            break;
            case R.id.btn_release: {
                if (mStoryboard != null) {
                    int[] tids = mStoryboard.getAllTrackIds();
                    int length = tids.length;
                    for (int i = 0; i < length; i++) {

                        if (mStoryboard.isMultiTrack(tids[i])) {
                            PMMultiTrack it = mStoryboard.getMultiTrack(tids[i]);
                            Log.e(TAG, "multi track: id=" + it.getId() + " ,data duration US=" + it.getDataDuration());
                        } else {
                            PMTrack it = mStoryboard.getTrack(tids[i]);
                            StringBuilder builder = new StringBuilder();
                            builder.append((it.getSrc() == null || it.getSrc().length == 0) ? "silence" : it.getSrc());
                            Log.e(TAG, "track: id=" + it.getId() + " ,data duration US=" + it.getDataDuration() + " ,file path" + builder.toString());
                        }
                    }
                }
                mPlayer.release();
            }
            break;
            case R.id.btn_save: {
                Intent i = new Intent();
                i.setClass(this, SaveActivity.class);
                startActivity(i);
            }
            break;
            case R.id.btn_change_filter: {
            }
            break;
            case R.id.btn_change_filter_null: {
            }
            break;
            case R.id.btn_seek: {
                if (mPlayer != null) {
                    mPlayer.seekTo(3 * 1000000);
                }
            }
            break;
            case R.id.btn_preview_start: {
                mPlayer.play();
            }
            break;
            case R.id.btn_preview_stop: {
                mPlayer.stop();
            }
            break;
            case R.id.btn_preview_pause: {
                mPlayer.pause();
            }
            break;
            case R.id.btn_preview_resume: {
                mPlayer.resume();
            }
            break;
            case R.id.btn_preview_seekto: {
                if (mPlayer != null) {
                    if (count == 100) {
                        count = 0;
                    }
                    mPlayer.locPreview(mStoryboard.getDuration() * 1000 * count / 100);
                    count++;
                }
            }
            break;
            case R.id.btn_volume: {
                try {
                    int id = Integer.valueOf(metTrackID.getText().toString());
                    int volume = Integer.valueOf(metVolume.getText().toString());
                    PMTrack t = mStoryboard.getTrack(id);
                    t.setVolume(volume * 1.f / 100.f);
                } catch (Exception e) {
                    e.printStackTrace();
                }

            }
            break;
            default:
                break;
        }
    }


    @Override
    protected void onPause() {
        super.onPause();
        mPlayer.pause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mPlayer.resume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mPlayer != null) {
            mPlayer.release();
        }
        if (mStoryboard != null) {
            mStoryboard.destory();
        }
        PMEngine.getInstance().uninit();
    }


    // Storage Permissions
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};

    /**
     * Checks if the app has permission to write to device storage
     * <p>
     * If the app does not has permission then the user will be prompted to
     * grant permissions
     *
     * @param activity
     */
    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity,
                Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE);
        }
    }

    // Storage Permissions
    private static final int REQUEST_AUDIO_SETTING = 2;
    private static String[] PERMISSIONS_AUDIO_SETTING = {
            Manifest.permission.MODIFY_AUDIO_SETTINGS};

    /**
     * Checks if the app has permission to modify audio setting
     * <p>
     * If the app does not has permission then the user will be prompted to
     * grant permissions
     *
     * @param activity
     */
    public static void verifyAudioSettingPermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(activity,
                Manifest.permission.MODIFY_AUDIO_SETTINGS);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_AUDIO_SETTING,
                    REQUEST_AUDIO_SETTING);
        }
    }

    private Handler mHandler = new Handler(Looper.getMainLooper(), new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            // TODO Auto-generated method stub
            int id = msg.what;
            String message = (String) msg.obj;
            switch (id) {
                case PMMessage.MESSAGE_ID_V_RENDER_PROGRESS: {
                    long ts = 0;
                    try {
                        JSONObject json = new JSONObject(message);
                        ts = json.optLong("timestamp", 0);
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                    mtvPreogress.setText(String.valueOf(Math.round(ts * 100.0 / mStoryboard.getDuration())));
                }
                break;
                case PMMessage.MESSAGE_ID_V_RENDER_COMPLETE: {
                    mPlayer.seekTo(0);
                }
                break;
                default:
                    break;
            }
//            Log.e(TAG, "receive message: id=" + id + ", desc=" + message);
            return false;
        }
    });
}
