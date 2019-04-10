//
// Created by 张哲华 on 2019/1/12.
//

#include "HomoExtractorKlt.h"
bool outOfImg(const Point2f &point, const Size &size)
{
    return (point.x <= 0 || point.y <= 0 || point.x >= size.width - 1 || point.y >= size.height - 1 );
}

Mat HomoExtractorKlt::extractHomo(const Mat &img1, const Mat &img2) {
    Mat curGray;
    Mat img2_r;
    int downSampleScale = 2;

    resize(img2, img2_r, Size(img2.cols / downSampleScale, img2.rows / downSampleScale));
    cvtColor(img2_r, curGray, COLOR_BGR2GRAY );

    if (frameIdx == 0) {
        lastFeatures.clear();
        Mat img1_r;
        resize(img1, img1_r, Size(img1.cols / downSampleScale, img1.rows / downSampleScale));
        cvtColor(img1_r, lastGray , COLOR_BGR2GRAY );
        clock_t start = clock();
        goodFeaturesToTrack(lastGray, lastFeatures, 120, 0.1, 20);//检测特征点
        clock_t end = clock();
        cout << "   good features " << (double) (end - start) / 1000;
#ifdef DEBUG
        cout << "======good features======" << endl;
        for (auto i = lastFeatures.begin(); i != lastFeatures.end(); i ++) {
            cout << *i << endl;
        }
#endif
    }
    vector<uchar> status;
    vector<float> err;
    vector<Point2f> curFeatures;
    clock_t start = clock();
    calcOpticalFlowPyrLK( lastGray , curGray , lastFeatures , curFeatures , status , err );//根据已检测到的前一帧特征点在后一帧查找匹配的特征点
    clock_t end = clock();
    cout << "   optical flow " << (double) (end - start) / 1000;
#ifdef DEBUG
    cout << "======last features======" << endl;
    for (auto i = lastFeatures.begin(); i != lastFeatures.end(); i ++) {
        cout << *i << endl;
    }
    cout << "======cur features======" << endl;
    for (auto i = curFeatures.begin(); i != curFeatures.end(); i ++) {
        cout << *i << endl;
    }
#endif
    Mat m_Fundamental;
    vector<uchar> m_RANSACStatus;
    Mat p1(lastFeatures);
    Mat p2(curFeatures);
    m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);
    for (int j = 0 ; j < status.size() ; j++ )
    {
        if (m_RANSACStatus.size() > j && m_RANSACStatus[j] == 0) // 状态为0表示野点(误匹配)
        {
            status[j] = 0;
        }
    }
    int idx = 0;
    vector<Point2f> curFeaturesTmp, lastFeaturesTmp;
    for (auto itC = curFeatures.begin(), itP = lastFeatures.begin(); itC != curFeatures.end(); itC ++, itP ++, idx ++) {
        if (status[idx] == 0 || err[idx] > 20 || outOfImg(*itC, Size(img1.cols, img1.rows))) {

        } else {
            curFeaturesTmp.push_back(*itC * downSampleScale);
            lastFeaturesTmp.push_back(*itP * downSampleScale);
        }
    }
#ifdef DEBUG
    cout << endl << "======picked features======" << endl;
    for (auto i = curFeatures.begin(), j = lastFeatures.begin(); i != curFeatures.end(); i ++, j ++) {
        cout << *j << "   " << *i << endl;
    }
#endif
    Mat H = Mat();
    if (!lastFeaturesTmp.empty() && !curFeaturesTmp.empty() && lastFeaturesTmp.size() > 8) {
        clock_t start = clock();
        H = findHomography(lastFeaturesTmp, curFeaturesTmp, RANSAC);
        clock_t end = clock();
        cout << "   find homo " << (double) (end - start) / 1000 << endl;

    }
    lastFeatures.clear();
    lastFeatures.assign(curFeatures.begin(), curFeatures.end());
    lastGray = curGray;
    frameIdx ++;
    if (frameIdx == 40) {
        frameIdx = 0;
    }

    return H;
}
