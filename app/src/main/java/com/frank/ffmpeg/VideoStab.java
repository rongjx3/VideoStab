package com.frank.ffmpeg;

public class VideoStab {

    public interface OnHandleListener{
        void onBegin();
        void onEnd(int result);
    }

    static{
        System.loadLibrary("native_opencv");
        System.loadLibrary("media-handle");
    }

    public static void execute_all(final String input, final String savpath, final String output,final VideoStab.OnHandleListener onHandleListener){
        new Thread(new Runnable() {
            public void run() {
                if(onHandleListener != null){
                    onHandleListener.onBegin();
                }
                //调用ffmpeg进行处理
                ffmpegstab(input,savpath,output);
                if(onHandleListener != null){
                    onHandleListener.onEnd(1);
                }
            }
        }).start();
    }

    public static native String ffmpegstab(String input,String savpath,String output);
}
