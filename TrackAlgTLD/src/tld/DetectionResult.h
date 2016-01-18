#ifndef DETECTIONRESULT_H_
#define DETECTIONRESULT_H_

#include <vector>

#include <opencv/cv.h>

namespace tld
{

class DetectionResult
{
public:
    bool containsValidData;
    std::vector<cv::Rect>* fgList;
    float *posteriors;  /* Contains the posteriors for each slding window. Is of size numWindows. Allocated by tldInitClassifier. */
    std::vector<int>* confidentIndices;
    int *featureVectors;
    float *variances;
    int numClusters;
    cv::Rect *detectorBB; //Contains a valid result only if numClusters = 1

    DetectionResult();
    virtual ~DetectionResult();

    void init(int numWindows, int numTrees);

    void reset();
    void release();

};

} /* namespace tld */
#endif /* DETECTIONRESULT_H_ */
