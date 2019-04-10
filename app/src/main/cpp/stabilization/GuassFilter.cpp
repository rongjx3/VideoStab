//
// Created by 张哲华 on 2019/1/8.
//

#include "GuassFilter.h"

#define PI (2 * acos(0))
#define guass(k,theta) exp(-(k) * (k) / 2.0 / (theta) / (theta)) / sqrt(2 * PI * (theta))

void GuassFilter::preprocess(Mat& mat) {
//    Mat lastLine = Mat::zeros(1, 3, CV_64F);
//    lastLine.at<double>(0, 2) = 1;
//    vconcat(mat, lastLine, mat);
}

void GuassFilter::postprocess(Mat& mat) {

}

void GuassFilter::null_func(const deque<Mat>& window, vector<Mat>& transWindow) {
    for (int i = 0; i < window.size(); i ++) {
        transWindow[i] = window[i];
    }
}

void GuassFilter::delta_T(const deque<Mat>& window, vector<Mat>& transWindow) {
    int kernel = window.size();
    transWindow.clear();
    transWindow.resize(window.size());
    transWindow[kernel / 2] = Mat::eye(3, 3, CV_64F);
    for (int i = kernel / 2 - 1; i >= 0; i --) {
        transWindow[i] = window[i].inv() * transWindow[i + 1];
    }
    for (int i = kernel / 2 + 1; i < kernel; i ++) {
        transWindow[i] = window[i - 1] * transWindow[i - 1];
    }
}

bool GuassFilter::push(Mat data) {
    if (data.empty()) {
        int i = 0;
        while (_window.size() > 1) {
            inc_filter(i / 2 + 1);
            _window.pop_front();
            i ++;
        }
        _outputBuffer.push(Mat::eye(3, 3, CV_64F));
        return true;
    }
    preprocess(data);
    _window.push_back(data);

    if (_window.size() < _kernel) {
        if (_window.size() <= _kernel / 2) {
            _outputBuffer.push(Mat::eye(3, 3, CV_64F));
        }
        return false;
    } else {
        inc_filter();
        _window.pop_front();
        return true;
    }
}

Mat GuassFilter::pop() {
    if (!_outputBuffer.empty()) {
        Mat ret = _outputBuffer.front().clone();
        _outputBuffer.pop();
        return ret;
    }
    return Mat();
}

void GuassFilter::filter(vector<Mat>& matData, int kernel, double sigma, void (*transform_)(const deque<Mat>& window, vector<Mat>& transWindow)) {
    vector<double> kernelVec;
    deque<Mat> window;
    vector<Mat> transformedWindow;
    transformedWindow.resize(kernel);

    double kernalSum = 0;
    for (int i = 0; i < kernel; i ++) {
        double weight = guass(i - kernel / 2, sigma);
        kernelVec.push_back(weight);
        kernalSum += weight;
        // init window
        window.push_back(matData[i].clone());
    }
    window.pop_back();

    for (int i = kernel / 2; i < matData.size() - (kernel - 1) / 2; i ++) {
        window.push_back(matData[i + (kernel - 1) / 2].clone());
        transform_(window, transformedWindow);
        Mat sumMat = Mat::zeros(3, 3, CV_64F);
        for (int j = 0; j < kernel; j ++) {
            if (transformedWindow.size() == kernel)
                sumMat += kernelVec[j] * transformedWindow[j];
            else
                sumMat += kernelVec[j] * window[j];
        }
        matData[i] = sumMat / kernalSum;
        window.pop_front();
    }
}

GuassFilter::GuassFilter(int kernel, double sigma, void (*transform)(const deque<Mat>& window, vector<Mat>& transWindow))
    : _kernel(kernel), _sigma(sigma), _transform(transform) {
    _kernelSum = 0;
    for (int i = 0; i < kernel; i ++) {
        double weight = guass(i - kernel / 2, sigma);
        _kernelVec.push_back(weight);
        cout << weight << endl;
        _kernelSum += weight;
    }
    _transformedWindow.resize(_kernel);
}

void GuassFilter::inc_filter(int kernelOffset) {
//    cout << "====== window ======" << endl;
//    for (int i = 0; i < _window.size(); i ++) {
//        cout << _window[i] << endl;
//    }
    _transform(_window, _transformedWindow);
//    cout << "====== transformed ======" << endl;
//    for (int i = 0; i < _transformedWindow.size(); i ++) {
//        cout << _transformedWindow[i] << endl;
//    }
//    cout << endl;
    Mat sumMat = Mat::zeros(3, 3, CV_64F);
    for (int j = 0; j < _transformedWindow.size(); j ++) {
        sumMat += _kernelVec[j + kernelOffset] * _transformedWindow[j];
    }
    sumMat /= _kernelSum;
    _outputBuffer.push(sumMat);
}

bool GuassFilter::empty() {
    return _outputBuffer.empty();
}
