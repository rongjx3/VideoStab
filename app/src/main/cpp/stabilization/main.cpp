#include <iostream>
#include <opencv2/opencv.hpp>
#include "StableProcessor.h"
using namespace cv;
using namespace threads;
using namespace chrono;

Mat frame;

void readw(VideoCapture &vc, StableProcessor &sp) {
    Mat frame;

    int i = 0;
    if (vc.isOpened()) {
        while (true) {
            int index = sp.dequeueInputBuffer();
            vc >> frame;
            sp.enqueueInputBuffer(index, &frame);

            if (frame.empty()) {
                break;
            }
        }
//        sp.dequeueInputBuffer();
//        sp.enqueueInputBuffer(-1, &frame);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Arg format error");
        return -1;
    }
    VideoCapture vc(argv[1]);   //读取视频
    if (!vc.isOpened()) {
        fprintf(stderr, "Fail to open input video file");
        return -2;
    }
    Size videoSize; //视频尺寸
    vc.set(CAP_PROP_FRAME_HEIGHT, videoSize.height);
    vc.set(CAP_PROP_FRAME_WIDTH, videoSize.width);
    videoSize.height = (int) vc.get(CAP_PROP_FRAME_HEIGHT);
    videoSize.width = (int) vc.get(CAP_PROP_FRAME_WIDTH);
    int frameCount = (int) vc.get(CAP_PROP_FRAME_COUNT);
    int fourcc = (int) vc.get(CAP_PROP_FOURCC);
    double fps = vc.get(CAP_PROP_FPS);

    clock_t start = clock();
    StableProcessor sp;
    sp.Init(videoSize);

    thread thread1(readw, ref(vc), ref(sp));

    int i = 0;
    Mat newImage;
    VideoWriter vw(argv[2], fourcc, fps, videoSize);    //输出视频
    if (!vc.isOpened()) {
        fprintf(stderr, "Fail to open output video file");
        return -3;
    }
    while (true) {
        Mat stableVec = Mat::eye(3, 3, CV_64FC1);
        sp.dequeueOutputBuffer(&stableVec, &frame);
        if (stableVec.empty() || frame.empty()) {
            break;
        }
        clock_t sta = clock();
        warpPerspective(frame, newImage, stableVec, videoSize);
        clock_t middle = clock();

        vw << newImage;
        clock_t end = clock();
        double elapsed1 = (double) (middle - sta) / 1000;
        double elapsed2 = (double) (end - middle) / 1000;

        cout << "warp " << elapsed1 << " store " << elapsed2 << endl;


        sp.enqueueOutputBuffer();
        i ++;
    }
    thread1.join();
    vw.release();
    clock_t stop = clock();
    double elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
    cout << "\nTime elapsed: " << elapsed << endl;
    return 0;
}