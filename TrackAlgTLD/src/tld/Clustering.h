#ifndef CLUSTERING_H_
#define CLUSTERING_H_

#include <vector>

#include <opencv/cv.h>

#include "DetectionResult.h"

namespace tld
{

class Clustering
{
    void calcMeanRect(std::vector<int> * indices);
    void calcDistances(float *distances);
    void cluster(float *distances, int *clusterIndices);
public:
    int *windows;
    int numWindows;

    DetectionResult *detectionResult;

    //Configurable members
    float cutoff;



    Clustering();
    virtual ~Clustering();
    void release();
    void clusterConfidentIndices();
};

} /* namespace tld */
#endif /* CLUSTERING_H_ */
