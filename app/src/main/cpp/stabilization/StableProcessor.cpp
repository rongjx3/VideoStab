//
// Created by 张哲华 on 2019-01-13.
//

#include "StableProcessor.h"
#include "GuassFilter.h"
#include "HomoExtractorKlt.h"
#include <android/log.h>
using namespace chrono;

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "ProjectName", __VA_ARGS__)

int StableProcessor::dequeueInputBuffer() { //输入队列出队
    _in_semaphore -> Wait();
    LOGE("in buffer");
    return _buffer_idx;
}

void StableProcessor::enqueueInputBuffer(int buffer_index, const Mat *new_frame) {  //输入队列入对
    assert(buffer_index == _buffer_idx);
    new_frame->copyTo(_frameBuffer[_index_in_buffer(_buffer_idx)]);

    if (_buffer_base_idx != -1 || _buffer_idx != 0) {
        _prc_semaphore->Signal();
    }
    _buffer_idx ++;
}

void StableProcessor::enqueueOutputBuffer() {   //输出队列入队
    _in_semaphore->Signal();
}

void StableProcessor::dequeueOutputBuffer(Mat *const stableVec, Mat *const frame) { //输出队列出队
    _out_semaphore->Wait();
    _frameBuffer[_index_in_buffer(++_buffer_base_idx)].copyTo(*frame);
    //LOGE("it deqob3");
    if (!frame->empty()) {
        //LOGE("it deqob4");
        _transBuffer.front().copyTo(*stableVec);
        //LOGE("it deqob5");
        _transBuffer.pop();
        //LOGE("it deqob6");
    }
    //LOGE("it deqob7");
}

void StableProcessor::Init(Size videoSize) {    //各队列进程初始化
    _frameBuffer.clear();
    _frameBuffer.resize(BUFFER_SIZE);
    _buffer_idx = 0;

    if (_in_semaphore != nullptr) {
        delete _in_semaphore;
        _in_semaphore = nullptr;
    }
    _in_semaphore = new MySemaphore(BUFFER_SIZE);
    if (_out_semaphore != nullptr) {
        delete _out_semaphore;
        _out_semaphore = nullptr;
    }
    _out_semaphore = new MySemaphore(0);
    if (_prc_semaphore != nullptr) {
        delete _prc_semaphore;
        _prc_semaphore = nullptr;
    }
    _prc_semaphore = new MySemaphore(0);

    _thread_worker = thread(&StableProcessor::worker, this);
}

StableProcessor::~StableProcessor() {
    _thread_worker.join();
}

void StableProcessor::worker() {

    GuassFilter guassFilter(30, 50, GuassFilter::delta_T);
    HomoExtractorKlt extractor;
    int i=0;

    while (true) {
        _prc_semaphore->Wait();
        i++;
        LOGE("in worker %d",i);
        //cout << "in worker" << endl;
        Mat lastFrame = _frameBuffer[_index_in_buffer(_buffer_prc_idx ++)];
        Mat frame = _frameBuffer[_index_in_buffer(_buffer_prc_idx)];
        if (!frame.empty())
        {
            Mat H = extractor.extractHomo(lastFrame, frame);    //！提取单应矩阵
            bool isValidH = !H.empty();
            bool readyToPull = guassFilter.push(isValidH ? H : Mat::eye(3, 3, CV_64F));
            if (readyToPull) {
                Mat goodH = guassFilter.pop();
                _transBuffer.push(goodH);
                _out_semaphore->Signal();
            }
        }
        else
        {
            guassFilter.push(Mat());
            while (!guassFilter.empty()) {
                Mat goodH = guassFilter.pop();
                _transBuffer.push(goodH);
                _out_semaphore->Signal();
            }
            _transBuffer.push(Mat());
            _out_semaphore->Signal();
            LOGE("worker out!>>>>>>>>>>>>>>>>>!");
            break;
        }
    }
    //cout << "worker over" << endl;
}

inline int StableProcessor::_index_in_buffer(int idx) {
    return idx % BUFFER_SIZE;
}
