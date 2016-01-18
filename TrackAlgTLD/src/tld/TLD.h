#ifndef TLD_H_
#define TLD_H_

#include <opencv/cv.h>

#include "MedianFlowTracker.h"
#include "DetectorCascade.h"

typedef void (*CbFunc)(void*, std::string&, float*, uint32_t, int32_t, int32_t, int32_t);

namespace tld {

class TLD {
	void storeCurrentData();
	void fuseHypotheses();
	void learn();
	void initialLearning();
public:
	bool trackerEnabled;
	bool detectorEnabled;
	bool learningEnabled;
	bool alternating;

	MedianFlowTracker *medianFlowTracker;
	DetectorCascade *detectorCascade;
	NNClassifier *nnClassifier;
	bool valid;
	bool wasValid;
	cv::Mat prevImg;
	cv::Mat currImg;
	cv::Rect *prevBB;
	cv::Rect *currBB;
	cv::Rect *finalBB;
	float currConf;
	bool learning;	//This is just for display purposes

	bool bCallback;

	TLD();
	virtual ~TLD();
	void release();
	void selectObject(const cv::Mat &img, cv::Rect *bb);
	void processImage(const cv::Mat &img);
	void writeToFile(const char *path);
	void readFromFile(const char *path);
	void setCallbackFunc(std::string& objId, CbFunc Callback, void* userData);

private:
	std::string m_objId;
	CbFunc m_cbObjLeaved;
	void* m_userData;
};

} /* namespace tld */

typedef struct detectInfo {
	tld::TLD *tld;
	cv::Mat grey_frame;
} detectInfo;

#endif /* TLD_H_ */
