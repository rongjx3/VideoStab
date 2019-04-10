//
// Created by 张哲华 on 2019/1/8.
//

#ifndef GUASSIANMAT_GUASSFILTER_H
#define GUASSIANMAT_GUASSFILTER_H

#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class GuassFilter {
private:
    queue<Mat> _outputBuffer;
    static void null_func(const deque<Mat>& window, vector<Mat>& transWindow);
    void inc_filter(int kernelOffset = 0);
    int _kernel;
    double _sigma;
    deque<Mat> _window;
    vector<Mat> _transformedWindow;
    double _kernelSum;
    vector<double> _kernelVec;
    void (*_transform)(const deque<Mat>& window, vector<Mat>& transWindow);


public:
    explicit GuassFilter(int kernel = 5, double sigma = 2.44949, void (*transform_)(const deque<Mat>& window, vector<Mat>& transWindow) = null_func);
    virtual void preprocess(Mat&);
    virtual void postprocess(Mat&);
    static void filter(vector<Mat>& matData, int kernel = 5, double sigma = 2.44949, void (*transform_)(const deque<Mat>& window, vector<Mat>& transWindow) = null_func);
    static void delta_T(const deque<Mat>& window, vector<Mat>& transWindow);
    bool push(Mat data);
    Mat pop();
    bool empty();
};


#endif //GUASSIANMAT_GUASSFILTER_H
