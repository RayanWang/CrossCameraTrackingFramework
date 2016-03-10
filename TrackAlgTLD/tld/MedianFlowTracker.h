#ifndef MEDIANFLOWTRACKER_H_
#define MEDIANFLOWTRACKER_H_

#include <opencv/cv.h>

namespace tld
{

class MedianFlowTracker
{
public:
    cv::Rect *trackerBB;

    MedianFlowTracker();
    virtual ~MedianFlowTracker();
    void cleanPreviousData();
    void track(const cv::Mat &prevImg, const cv::Mat &currImg, cv::Rect *prevBB);
};

} /* namespace tld */
#endif /* MEDIANFLOWTRACKER_H_ */
