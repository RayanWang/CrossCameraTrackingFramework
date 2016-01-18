#ifndef ADABOOST_H
#define ADABOOST_H

#include <vector>
#include <opencv2/opencv.hpp>
#ifdef _GPU_DETECTION_
#include <opencv2/gpu/gpu.hpp>
#endif

using namespace std;
using namespace cv;
#ifdef _GPU_DETECTION_
using namespace cv::gpu;
#endif

typedef struct ObjectPos {
    float x;
    float y;
    float width;
    int found;
    int neighbours;
} ObjectPos;

/**
 Ali and M. N. Dailey, "Multiple human tracking in high-density crowds"
 Abschnitt 2.2 Detection */
class adaboostDetect {
public:
    adaboostDetect(
    		string strModelPath,
    		int flags,
			int minNeighbours,
			double minSize,
			double maxSize,
			double scaleFactor);
    ~adaboostDetect();

    int detectObject(IplImage* img, vector<CvRect> & regions);

private:
    int m_flags; /* CV_HAAR_DO_CANNY_PRUNING */
    int m_minNeighbours;
    double m_minSize;
    double m_maxSize;
    double m_scaleFactor;
    CvMemStorage* m_pStorage;
#ifdef _GPU_DETECTION_
    CascadeClassifier_GPU m_FaceCascade;
#else
    CascadeClassifier m_FaceCascade;
#endif
};

#endif
