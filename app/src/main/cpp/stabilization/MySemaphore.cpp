//
// Created by 张哲华 on 20/09/2017.
//

#include "MySemaphore.h"
#include <android/log.h>

using namespace threads;
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "ProjectName", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)

void MySemaphore::Wait() {
    //LOGE("waiting 1");
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    //LOGI("Wait() count: %d",count_);
    while(!count_) // Handle spurious wake-ups.
    {
        //LOGE("waiting 3");
        condition_.wait(lock);
    }
    //LOGE("waiting 4");
    --count_;
}

void MySemaphore::Signal() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    ++count_;
    //LOGI("Signal() count: %d",count_);
    condition_.notify_one();
}

bool MySemaphore::TryWait() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    if(count_) {
        --count_;
        return true;
    }
    return false;
}
