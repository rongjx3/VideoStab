//
// Created by root on 19-3-17.
//

#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "stabilization/StableProcessor.h"
#include <android/log.h>
#include <include/libavutil/timestamp.h>
#include "videoWriterFFmpeg.h"
#include <vector>

#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "keymatch", __VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "ProjectName", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , "ProjectName", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "ProjectName", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , "ProjectName", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)

extern "C"
{
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

using namespace cv;
using namespace std;
using namespace threads;
using namespace chrono;


jstring str2jstring(JNIEnv* env,const char* pat)
{
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}


std::string jstring2str(JNIEnv* env, jstring jstr)
{
    char*   rtn   =   NULL;
    jclass   clsstring   =   env->FindClass("java/lang/String");
    jstring   strencode   =   env->NewStringUTF("GB2312");
    jmethodID   mid   =   env->GetMethodID(clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
    jbyteArray   barr=   (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
    jsize   alen   =   env->GetArrayLength(barr);
    jbyte*   ba   =   env->GetByteArrayElements(barr,JNI_FALSE);
    if(alen   >   0)
    {
        rtn   =   (char*)malloc(alen+1);
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    env->ReleaseByteArrayElements(barr,ba,0);
    std::string stemp(rtn);
    free(rtn);
    return   stemp;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_frank_ffmpeg_activity_VideoStabActivity_readtest(
        JNIEnv *env,
        jclass clazz,jstring path) {
    FILE* file = NULL;
    file = fopen("/storage/emulated/0/err.txt","a");
    fwrite("here",100,1,file);
    fclose(file);
    String _path = jstring2str(env,path);

    Mat img;

    img=imread(_path+"/001.jpg");
    imwrite(_path+"/output001.jpg",img);
    return path;
}

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

void readw(SwsContext *img_convert_ctx, int videoindex, AVPacket* packet, AVFormatContext* pFormatCtx, AVCodecContext* pCodecCtx, AVFrame* pAvFrame, AVFrame* pFrameBGR, StableProcessor &sp) {
    LOGE("in readw");
    int i=0;
    int ret;
    int got_picture,inpu=0;

    for (;;)
    {
        Mat mRGB(Size(pCodecCtx->width, pCodecCtx->height), CV_8UC3);
        if (av_read_frame(pFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoindex)
            {
                ret = avcodec_decode_video2(pCodecCtx, pAvFrame, &got_picture, packet);
                //LOGE("ret: %d ;g_p: %d",ret,got_picture);
                if (ret < 0) {
                    LOGE("Decode Error.（解码错误）\n");
                    break;
                }
                if (got_picture)
                {
                    i++;
                    inpu=1;
                    LOGE("in image %d",i);
                    //YUV to RGB
                    sws_scale(img_convert_ctx,
                              (const uint8_t *const *) pAvFrame->data,
                              pAvFrame->linesize,
                              0,
                              pCodecCtx->height,
                              pFrameBGR->data,
                              pFrameBGR->linesize);
                    //Mat mRGB(pCodecCtx->height, pCodecCtx->width, CV_8UC3, out_buffer);//（1）等效于下面
                    //Mat mRGB(Size(pCodecCtx->width,pCodecCtx->height), CV_8UC3);//（2）
                    //mRGB.data = out_buffer;//memcpy(pCvMat.data, out_buffer, size);

                    mRGB.data = (uchar *) pFrameBGR->data[0];//注意不能写为：(uchar*)pFrameBGR->data

                }
            }
            av_free_packet(packet);
        } else {

            break;
        }

        if(inpu && got_picture) {
            int index = sp.dequeueInputBuffer();
            //imwrite (path+"/inputtest/pic"+num+".jpeg",iframe);
            sp.enqueueInputBuffer(index, &mRGB);
            if (mRGB.empty()) {
                LOGE("out readw");
            }
        }
        got_picture=0;


    }
    Mat mRGB;
    mRGB.data=NULL;
    int index = sp.dequeueInputBuffer();
    //imwrite (path+"/inputtest/pic"+num+".jpeg",iframe);
    sp.enqueueInputBuffer(index, &mRGB);
    if (mRGB.empty()) {
        LOGE("readw over<<<<<<<<<<<<<<<<<<<<");
    }
}

void av_free(void *ptr)
{
#if CONFIG_MEMALIGN_HACK
    if (ptr) {
        int v= ((char *)ptr)[-1];
        av_assert0(v>0 && v<=ALIGN);
        free((char *)ptr - v);
    }
#elif HAVE_ALIGNED_MALLOC
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}
static size_t max_alloc_size= INT_MAX;
void *av_malloc(size_t size)
{
    void *ptr = NULL;
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;
    ptr = malloc(size);
    if(!ptr && !size) {
        size = 1;
        ptr= av_malloc(1);
    }
    return ptr;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_frank_ffmpeg_VideoStab_ffmpegstab(
        JNIEnv *env,
        jclass clazz,jstring input,jstring savpath,jstring output) {
    String path=jstring2str(env, input);
    const char *filename = path.c_str();
    String opath=jstring2str(env, output);
    const char *filename2 = opath.c_str();
    String spath=jstring2str(env, savpath);

    //
    AVCodec *pCodec; //解码器指针
    AVCodecContext* pCodecCtx; //ffmpeg解码类的类成员

    AVFrame* pAvFrame; //多媒体帧，保存解码后的数据帧
    AVFormatContext* pFormatCtx; //保存视频流的信息

    clock_t t1=clock();
    av_register_all(); //注册库中所有可用的文件格式和编码器
    Size videoSize;

    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) { //检查文件头部
        LOGE("Can't find the stream!\n");
    }
    if (avformat_find_stream_info(pFormatCtx, NULL)<0) { //查找流信息
        LOGE("Can't find the stream information !\n");
    }

    int videoindex = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) //遍历各个流，找到第一个视频流,并记录该流的编码信息
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            videoSize.width = pFormatCtx->streams[i]->codec->width;
            videoSize.height = pFormatCtx->streams[i]->codec->height;
            break;
        }
    }
    if (videoindex == -1) {
        LOGE("Don't find a video stream !\n");
        return output;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec; //得到一个指向视频流的上下文指针
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id); //到该格式的解码器
    if (pCodec == NULL) {
        LOGE("Cant't find the decoder !\n"); //寻找解码器
        return output;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) { //打开解码器
        LOGE("Can't open the decoder !\n");
        return output;
    }
    pAvFrame = av_frame_alloc(); //分配帧存储空间
    AVFrame* pFrameBGR = av_frame_alloc(); //存储解码后转换的RGB数据

    // 保存BGR，opencv中是按BGR来保存的
    int size = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
    uint8_t *out_buffer = (uint8_t *)av_malloc(size);
    avpicture_fill((AVPicture *)pFrameBGR, out_buffer, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);

    AVPacket* packet = (AVPacket*)malloc(sizeof(AVPacket));
    printf("-----------输出文件信息---------\n");
    av_dump_format(pFormatCtx, 0, filename, 0);
    printf("------------------------------");

    AVStream *stream=pFormatCtx->streams[videoindex];
    int fps=stream->avg_frame_rate.num/stream->avg_frame_rate.den;//每秒多少帧
    LOGE("fps = %d",fps);

    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getContext(pCodecCtx->width,
                                     pCodecCtx->height,
                                     pCodecCtx->pix_fmt,
                                     pCodecCtx->width,
                                     pCodecCtx->height,
                                     AV_PIX_FMT_BGR24, //设置sws_scale转换格式为BGR24，这样转换后可以直接用OpenCV显示图像了
                                     SWS_BICUBIC,
                                     NULL, NULL, NULL);
    //
    LOGE("It's OK!");
    int ret;
    int got_picture;
    int i=0;
    StableProcessor sp;
    sp.Init(videoSize); //初始化，建立处理线程

    thread thread1(readw, ref(img_convert_ctx), ref(videoindex), ref(packet), ref(pFormatCtx), ref(pCodecCtx), ref(pAvFrame), ref(pFrameBGR), ref(sp));

    //waiting part
    float secs=0.5;      //定义浮点型变量secs
    clock_t delay;  //定义clock_t类型的变量，表示延时时间
    delay=secs * CLOCKS_PER_SEC;
    LOGE("wait...");
    clock_t start=clock();
    while(clock()-start < delay);
    LOGE("waited completed!");

    clock_t t2=clock();
    double tl1=(double)(t2-t1)/CLOCKS_PER_SEC/5;
    LOGE("It's here OK!");

    videoWriterFFmpeg videoWriter;
    Mat src;

    string video_path = opath;
    int bitrate = 4500000;
    //int fps = 30;
    int w=videoSize.width;
    int h=videoSize.height;

    if (videoWriter.open((char *)video_path.c_str(), bitrate, fps,w,h) != 0)
        LOGE("open error");
    //int fps = 30;
    i=0;

    LOGE("begin loop>>>");
    while (true) {  //对原视频的帧进行形变与存储
        i++;

        LOGE("in the round %d",i);

        Mat newImage, frame;

        Mat stableVec = Mat::eye(3, 3, CV_64FC1);
        sp.dequeueOutputBuffer(&stableVec, &frame);
        if (stableVec.empty() || frame.empty()) {
            LOGE("no picture, out");
            break;
        }
        warpPerspective(frame, newImage, stableVec, videoSize);
        //imwrite(spath + "/output/img" + num + ".jpg", newImage);
        videoWriter.write(newImage);
        sp.enqueueOutputBuffer();

    }
    thread1.join();

    av_free(out_buffer);
    av_free(pFrameBGR);
    av_free(pAvFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    sws_freeContext(img_convert_ctx);
    clock_t t3=clock();
    double tl2=(double)(t3-t2)/CLOCKS_PER_SEC;

    LOGE("write OK!");
    videoWriter.flush();
    double tt=(double)(t3-t1)/CLOCKS_PER_SEC;

    LOGE("read: %f warp&write: %f total: %f",tl1,tl2,tt);

    LOGE("It's still OK!");

    return output;
}