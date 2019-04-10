package com.frank.ffmpeg.activity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.widget.ProgressBar;

import com.frank.ffmpeg.FFmpegCmd;
import com.frank.ffmpeg.R;
import com.frank.ffmpeg.VideoStab;
import com.frank.ffmpeg.format.VideoLayout;
import com.frank.ffmpeg.util.FFmpegUtil;
import com.frank.ffmpeg.util.FileUtil;

import java.io.File;

import static com.frank.ffmpeg.VideoStab.*;

public class VideoStabActivity extends AppCompatActivity implements View.OnClickListener{

    private static final String TAG = VideoStabActivity.class.getSimpleName();
    private static final int MSG_BEGIN = 101;
    private static final int MSG_FINISH = 102;

    private static final String PATH = Environment.getExternalStorageDirectory().getPath();
    private static final String srcFile = PATH + File.separator + "input.mp4";
    private ProgressBar progress_video;

    public SurfaceView msv;
    public SurfaceHolder sh;
    public MediaPlayer mmp;

    @SuppressLint("HandlerLeak")
    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case MSG_BEGIN:
                    progress_video.setVisibility(View.VISIBLE);
                    setGone();
                    break;
                case MSG_FINISH:
                    progress_video.setVisibility(View.GONE);
                    setVisible();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        if (getSupportActionBar() != null){
            getSupportActionBar().hide();
        }
        setContentView(R.layout.activity_video_stab);
        msv=(SurfaceView)findViewById(R.id.sv_player);
        mmp=new MediaPlayer();

        intView();
    }

    private void intView() {
        progress_video = (ProgressBar)findViewById(R.id.progress_video);
        msv.setOnClickListener(this);
        findViewById(R.id.btn_video_transform).setOnClickListener(this);
        findViewById(R.id.btn_play_ori_video).setOnClickListener(this);
        findViewById(R.id.btn_play_stab_video).setOnClickListener(this);
    }

    private void setVisible() {
        msv.setVisibility(View.VISIBLE);
        findViewById(R.id.btn_video_transform).setVisibility(View.VISIBLE);
        findViewById(R.id.btn_play_ori_video).setVisibility(View.VISIBLE);
        findViewById(R.id.btn_play_stab_video).setVisibility(View.VISIBLE);
    }

    private void setGone() {
        msv.setVisibility(View.GONE);
        findViewById(R.id.btn_video_transform).setVisibility(View.GONE);
        findViewById(R.id.btn_play_ori_video).setVisibility(View.GONE);
        findViewById(R.id.btn_play_stab_video).setVisibility(View.GONE);
    }

    @Override
    public void onClick(View v) {
        int handleType;
        switch (v.getId()){
            case R.id.btn_video_transform:
                handleType = 0;
                break;
            case R.id.btn_play_ori_video:
                handleType = 8;
                break;
            case R.id.btn_play_stab_video:
                handleType = 9;
                break;
            default:
                handleType = 0;
                break;
        }
        doHandleVideo(handleType);
    }

    /**
     * 调用ffmpeg处理视频
     * @param handleType handleType
     */
    private void doHandleVideo(int handleType){
        String[] commandLine = null;
        String[] commandLine1 = null;
        String[] commandLine2 = null;
        switch (handleType){
            case 0://
                String insrcFile = PATH + File.separator + "in.mp4";
                String outsrcFile = PATH + File.separator + "out.mp4";
                executeStaballCmd(insrcFile,PATH,outsrcFile);
                return;
            case 8://视频解码播放
                String srcFile1 = PATH + File.separator + "in.mp4";
                try {
                    mmp.reset();
                    mmp.setAudioStreamType(AudioManager.STREAM_MUSIC);
                    mmp.setDataSource(srcFile1);
                    sh = msv.getHolder();
                    mmp.setDisplay(sh);
                    mmp.prepare();
                    mmp.start();
                }
                catch (Exception e)
                {
                    Log.i("player error","player error");
                }
                return;
            case 9://视频解码播放
                String srcFile2 = PATH + File.separator + "out.mp4";
                try {
                    mmp.reset();
                    mmp.setAudioStreamType(AudioManager.STREAM_MUSIC);
                    mmp.setDataSource(srcFile2);
                    sh = msv.getHolder();
                    mmp.setDisplay(sh);
                    mmp.prepare();
                    mmp.start();
                }
                catch (Exception e)
                {
                    Log.i("player error","player error");
                }
                return;
            default:
                break;
        }
    }

    private void executeStaballCmd(final String c1, final String path, final String c2){
        if(path == null){
            return;
        }
        VideoStab.execute_all(c1, path, c2, new VideoStab.OnHandleListener() {
            @Override
            public void onBegin() {
                Log.i(TAG, "handle video onBegin...");
                mHandler.obtainMessage(MSG_BEGIN).sendToTarget();
            }

            @Override
            public void onEnd(int result) {
                Log.i(TAG, "handle video onEnd...");

                mHandler.obtainMessage(MSG_FINISH).sendToTarget();
            }
        });
    }
}
