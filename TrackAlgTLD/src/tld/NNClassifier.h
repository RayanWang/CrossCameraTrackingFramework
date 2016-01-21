#ifndef NNCLASSIFIER_H_
#define NNCLASSIFIER_H_

#include <vector>

#include <opencv/cv.h>

#include "NormalizedPatch.h"
#include "DetectionResult.h"

#include "../TldFeature.h"

namespace tld {

class NNClassifier {
	float ncc(float *f1, float *f2);
public:
	bool enabled;

	int *windows;
	float thetaFP;
	float thetaTP;
	DetectionResult *detectionResult;
	std::vector<NormalizedPatch>* falsePositives;
	std::vector<NormalizedPatch>* truePositives;

	NNClassifier();
	virtual ~NNClassifier();

	void release();
	float classifyPatch(NormalizedPatch *patch);
	float classifyBB(const cv::Mat &img, cv::Rect *bb);
	float classifyWindow(const cv::Mat &img, int windowIdx);
	float classifyFromOtherView(std::map<std::string, std::shared_ptr<TldFeature> > &mapPositiveSamples,
			const cv::Mat &img,
			cv::Rect *bb,
			int32_t& cameraId,
			std::string& objIdFromOthers);
	void learn(std::vector<NormalizedPatch> patches);
	bool filter(const cv::Mat &img, int windowIdx);
};

} /* namespace tld */
#endif /* NNCLASSIFIER_H_ */
