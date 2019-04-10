//
// Created by 张哲华 on 2019/1/9.
//

#include "HomoExtractor.h"
#include <opencv2/xfeatures2d.hpp>
using namespace cv::xfeatures2d;
//#define SHOW_TRANSFORMED 1
//#define RANSAC_METHOD 1
//#define SHOW_MATCH 1
void match_features(Mat& query, Mat& train, vector<DMatch>& matches) {
    vector<vector<DMatch>> knn_matches;
    BFMatcher matcher(NORM_L2);

    matcher.knnMatch(query, train, knn_matches, 2);

    //获取满足Ratio Test的最小匹配的距离
    float min_dist = FLT_MAX;
    for (int r = 0; r < knn_matches.size(); ++r) {
        //Ratio Test
        if (knn_matches[r][0].distance > 0.6 * knn_matches[r][1].distance)
            continue;

        float dist = knn_matches[r][0].distance;
        if (dist < min_dist) min_dist = dist;
    }

    matches.clear();
    for (size_t r = 0; r < knn_matches.size(); ++r) {
        //排除不满足Ratio Test的点和匹配距离过大的点
        if (
                knn_matches[r][0].distance > 0.6 * knn_matches[r][1].distance ||
                knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
                )
            continue;

        //保存匹配点
        matches.push_back(knn_matches[r][0]);
    }
}

void drawTransformedMatches(const Mat& inputImg, Mat& outimg, std::vector<Point2f>& obj, std::vector<Point2f>& scene, const Mat& H) {
    std::vector<KeyPoint> scene_t;
    std::vector<KeyPoint> scene_k;
    cv::Mat_<double> src(3/*rows*/,1 /* cols */, CV_64F);


    for (int i = 0; i < obj.size(); i ++) {
        src(0,0)=obj[i].x;
        src(1,0)=obj[i].y;
        src(2,0)=1.0;
        Mat ret = H * src;
        scene_t.push_back(KeyPoint(ret.at<double>(0, 0) / ret.at<double> (0, 2), ret.at<double>(1, 0) / ret.at<double> (0, 2), 1.f));
    }
    for (int i = 0; i < scene.size(); i ++) {
        scene_k.push_back(KeyPoint(scene[i], 1.f));
    }


    drawKeypoints(inputImg, scene_t, outimg, Scalar::all(-1));
    drawKeypoints(outimg, scene_k, outimg, Scalar::all(-1));

    std::vector<Point2f> obj_t;
    KeyPoint::convert(scene_t, obj_t);
    for (int i = 0; i < scene_t.size(); i++) {
        line(outimg, obj_t[i], scene[i], Scalar::all(-1), 3);
    }
}


Mat HomoExtractor::extractHomo(const Mat &img1, const Mat &img2) {
    CV_Assert(!img2.empty() && !img1.empty());
    Ptr<SURF> surf;      //创建方式和2中的不一样
    surf = SURF::create(800);

    BFMatcher matcher;
    Mat descriptor1, descriptor2;
    vector<KeyPoint>key1, key2;
    vector<DMatch> good_matches;
    surf->detectAndCompute(img1, Mat(), key1, descriptor1);
    surf->detectAndCompute(img2, Mat(), key2, descriptor2);

    if (descriptor1.empty() || descriptor2.empty())
        return Mat();
    match_features(descriptor1, descriptor2, good_matches);

#if defined(SHOW_TRANSFORMED) || defined(SHOW_MATCH)
    Mat outimg;
#endif
#ifdef SHOW_MATCH
    drawMatches(img1, key1, img2, key2, good_matches, outimg, Scalar::all(-1), Scalar::all(-1),vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);  //绘制匹配点
#endif

    if (good_matches.size() < 5) {
        return Mat();
    }
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for (size_t i = 0; i < good_matches.size(); i++)
    {
        obj.push_back(key1[good_matches[i].queryIdx].pt);
        scene.push_back(key2[good_matches[i].trainIdx].pt);
    }

    if (obj.empty() || scene.empty())
        return Mat();
#ifdef RANSAC_METHOD
    vector<char> mask_;
    Mat H = findHomography(obj, scene, RANSAC, 2, mask_);
        int maskedCount = 0;
    for (int i = 0; i < mask_.size(); i ++) {
        maskedCount += mask_[i];
        cout << (int) mask_[i] << " " << obj[i] << " " << scene[i] << endl;
    }
    if (maskedCount < 8)
        return Mat();
#else
    Mat H = findHomography(obj, scene, LMEDS);
#endif
    cout << "get H " << endl << H << endl;

#ifdef SHOW_TRANSFORMED
    drawTransformedMatches(img2, outimg, obj, scene, H);
#endif
#if defined(SHOW_TRANSFORMED) || defined(SHOW_MATCH)
    imshow("hello", outimg);
    cvWaitKey(0);
#endif
    return H;
}
