//
// Created by 张哲华 on 2019/1/9.
//

#ifndef GUASSIANMAT_HOMOEXTRACTOR_H
#define GUASSIANMAT_HOMOEXTRACTOR_H

#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
class HomoExtractor {
public:
    static Mat extractHomo(const Mat& img1, const Mat& img2);
};


#endif //GUASSIANMAT_HOMOEXTRACTOR_H
