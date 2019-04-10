//
// Created by 张哲华 on 2019-01-13.
//

#ifndef GUASSIANMAT_STABLEPROCESSOR_H
#define GUASSIANMAT_STABLEPROCESSOR_H


#include <opencv2/opencv.hpp>
#include <thread>
#include "MySemaphore.h"
using namespace cv;
using namespace std;
using namespace threads;
class StableProcessor {
private:
    const static int BUFFER_SIZE = 100;
    vector<Mat> _frameBuffer;
    queue<Mat> _transBuffer;
    volatile int _buffer_idx = 0;
    volatile int _buffer_base_idx = -1;
    volatile int _buffer_prc_idx = 0;
    inline int _index_in_buffer(int idx);
    void worker();
    thread _thread_worker;
    MySemaphore* _in_semaphore = nullptr;
    MySemaphore* _prc_semaphore = nullptr;
    MySemaphore* _out_semaphore = nullptr;
public:
    StableProcessor() = default;
    ~StableProcessor();

    void Init(Size videoSize);
    int dequeueInputBuffer();
    void enqueueInputBuffer(int buffer_index, const Mat* new_frame);
    void enqueueOutputBuffer();
    void dequeueOutputBuffer(Mat* const stableVec, Mat* const frame);
};


#endif //GUASSIANMAT_STABLEPROCESSOR_H
