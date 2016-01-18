#ifndef ENSEMBLECLASSIFIER_H_
#define ENSEMBLECLASSIFIER_H_

#include <opencv/cv.h>

namespace tld
{

class EnsembleClassifier
{
    const unsigned char *img;

    float calcConfidence(int *featureVector);
    int calcFernFeature(int windowIdx, int treeIdx);
    void calcFeatureVector(int windowIdx, int *featureVector);
    void updatePosteriors(int *featureVector, int positive, int amount);
public:
    bool enabled;

    //Configurable members
    int numTrees;
    int numFeatures;

    int imgWidthStep;
    int numScales;
    cv::Size *scales;

    int *windowOffsets;
    int *featureOffsets;
    float *features;

    int numIndices;

    float *posteriors;
    int *positives;
    int *negatives;

    DetectionResult *detectionResult;

    EnsembleClassifier();
    virtual ~EnsembleClassifier();
    void init();
    void initFeatureLocations();
    void initFeatureOffsets();
    void initPosteriors();
    void release();
    void nextIteration(const cv::Mat &img);
    void classifyWindow(int windowIdx);
    void updatePosterior(int treeIdx, int idx, int positive, int amount);
    void learn(int *boundary, int positive, int *featureVector);
    bool filter(int i);
};

} /* namespace tld */
#endif /* ENSEMBLECLASSIFIER_H_ */
