package com.frank.ffmpeg.activity;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import com.frank.ffmpeg.R;

/**
 * 使用ffmpeg进行音视频处理入口
 * Created by frank on 2018/1/23.
 */
public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    private final static String[] mPermissions = new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };
    private final static int CODE_STORAGE = 999;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();
        checkPermission();
    }

    private void initView() {
        findViewById(R.id.btn_stab).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Intent intent = new Intent();
        switch (v.getId()){
            case R.id.btn_stab://视频倒播
                intent.setClass(MainActivity.this, VideoStabActivity.class);
                break;
            default:
                break;
        }
        startActivity(intent);
    }

    //动态申请权限
    private void checkPermission(){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (checkSelfPermission(mPermissions[0]) != PackageManager.PERMISSION_GRANTED){
                requestPermissions(mPermissions, CODE_STORAGE);
            }
        }
    }

}
