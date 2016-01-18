#ifndef DETECTORCASCADE_H_
#define DETECTORCASCADE_H_

#include "DetectionResult.h"
#include "ForegroundDetector.h"
#include "VarianceFilter.h"
#include "EnsembleClassifier.h"
#include "Clustering.h"
#include "NNClassifier.h"
#include "NormalizedPatch.h"

namespace tld
{

struct detectionData {
//---------------------------------input-------------------------------------------
	// nnClassifier
	std::vector<NormalizedPatch>* falsePositives;
	std::vector<NormalizedPatch>* truePositives;

	// varianceFilter integral image
	int *ii;
	long long *ii_squared;

	// ensembleClassifier
	int *featureOffsets;
	float *ensemblePosteriors;

	// detectionResult foreground list
	std::vector<cv::Rect>* fgList;
//------------------------------------------------------------------------------------

//-------------------------------output-------------------------------------------
	// detectionResult
	float *posteriors;
	float *variances;
	int *featureVectors;
	std::vector<int>* confidentIndices;
//-------------------------------------------------------------------------------------
};

//Constants
static const int TLD_WINDOW_SIZE = 5;
static const int TLD_WINDOW_OFFSET_SIZE = 6;
static const int TLD_NUM_TREES = 10;
static const int TLD_NUM_FEATURES = 13;
static const int TLD_NUM_INDICES = pow(2.0f, TLD_NUM_FEATURES);

class DetectorCascade
{
	cv::Size *scales;

public:
    //Working data
    int numScales;

    //Configurable members
    int minScale;
    int maxScale;
    bool useShift;
    float shift;
    int minSize;
    int numFeatures;
    int numTrees;

    //Needed for init
    int imgWidth;
    int imgHeight;
    int imgWidthStep;
    int objWidth;
    int objHeight;

    int numWindows;
    int *windows;
    int *windowOffsets;

    //State data
    bool initialised;

    //Components of Detector Cascade
    ForegroundDetector *foregroundDetector;
    VarianceFilter *varianceFilter;
    EnsembleClassifier *ensembleClassifier;
    Clustering *clustering;
    NNClassifier *nnClassifier;

    DetectionResult *detectionResult;

    void propagateMembers();

    DetectorCascade();
    ~DetectorCascade();

    void init();

    void initWindowOffsets();
    void initWindowsAndScales();

    void release();
    void cleanPreviousData();
    void detect(const cv::Mat &img);
};

} /* namespace tld */
#endif /* DETECTORCASCADE_H_ */
