#ifndef VARIANCEFILTER_H_
#define VARIANCEFILTER_H_

#include <opencv/cv.h>

#include "IntegralImage.h"
#include "DetectionResult.h"

namespace tld
{

class VarianceFilter
{
public:
    IntegralImage<int>* integralImg;
    IntegralImage<long long>* integralImg_squared;

public:
    bool enabled;
    int *windowOffsets;

    DetectionResult *detectionResult;

    float minVar;

    VarianceFilter();
    virtual ~VarianceFilter();

    void release();
    void nextIteration(const cv::Mat &img);
    bool filter(int idx);
    float calcVariance(int *off);
};

} /* namespace tld */
#endif /* VARIANCEFILTER_H_ */
