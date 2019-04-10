//
// Created by 张哲华 on 2019/1/12.
//

#ifndef GUASSIANMAT_HOMOEXTRACTORKLT_H
#define GUASSIANMAT_HOMOEXTRACTORKLT_H

#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
class HomoExtractorKlt {
private:
    int frameIdx = 0;
    vector<Point2f> lastFeatures;
    Mat lastGray;
public:
    Mat extractHomo(const Mat& img1, const Mat& img2);
};


#endif //GUASSIANMAT_HOMOEXTRACTORKLT_H
