/*
 * TrackAlgTLD.h
 *
 *  Created on: Apr 24, 2015
 *      Author: Rayan
 */

#ifndef TRACKALGTLD_H_
#define TRACKALGTLD_H_


#include <vector>
#include <memory>

#include "TrackAlg.h"

class Main;
class TldFeature;

enum error_TLD {
	no_error = 0,
	obj_count_not_match,
	no_tracking_obj,
	obj_not_found,
	already_exist_error,
	tracking_error,
	invalid_arg,
	obj_already_checked,

	other_error = 0xffff
};

struct TldParam {
	float overlapThreshold;
	float intersectionThreshold;
	float mergeThreshold;

	int32_t keepTrackingTime;

	uint32_t initTargetNum;
	std::vector<std::string> vInitObjIdList;
};

#pragma pack(push)
#pragma pack(4)
struct TldFeatureDescription {
	int32_t splitIndex;
	int32_t totFeatureSize;
	int32_t eachElementSize;
};
#pragma pack(pop)

class CTrackAlgTLD : public CTrackAlg {
public:
	DECLARE_RUNTIME(CTrackAlgTLD);

	CTrackAlgTLD();
	virtual ~CTrackAlgTLD();

	static void* CreateInstance() { return new CTrackAlgTLD; }

	static void tldCallbackFunc(void* userData, std::string& objId, float* feature, uint32_t featureSizeInByte,
			int32_t splitIndex, int32_t totFeatureSize, int32_t eachElementSize) {
		CTrackAlgTLD* trackAlg = (CTrackAlgTLD*)userData;
		TldFeatureDescription desc;
		desc.splitIndex = splitIndex;
		desc.totFeatureSize = totFeatureSize;
		desc.eachElementSize = eachElementSize;

		if (trackAlg->m_cbTrack)
			trackAlg->m_cbTrack(trackAlg->m_userData, objId, (BYTE*)feature, featureSizeInByte,
					(BYTE*)&desc, sizeof(TldFeatureDescription));
	}

	// implement abstract class CTrackAlg
public:
	bool init(void* pParam);

	void release(void);

	bool prepareTracking(std::string& objId, IplImage* img, std::string stWinName, CvRect* pTargetBox = NULL);

	bool startTracking(std::string& objId, IplImage* img);

	void endTracking(std::string& objId);

	bool addTracking(std::string& objId);

	bool mergeTracking(IplImage *img, CvRect detectedBox);

	bool willKeepTracking(std::string& objId, bool* pbKeepTrack);

	bool setObjectFeatures(int32_t cameraId, std::string& objIdFromOthers, uint32_t sizeofFeature, BYTE* feature,
			uint32_t sizeofDesc = 0, BYTE* pFeatureDesc = NULL);

	bool identifyObject(std::string& objId, IplImage *img, CvRect detectedBox, int32_t& cameraId,
			std::string& objIdFromOthers);

	void setObjLeaveCallback(std::string& objId, cbTrackCallback cbTrack, void* userData);

	bool isAnyObjToIdendify(void) const;

	void getObjIdList(std::vector<std::string>& vObjId) const;

private:
	float intersectionRatio(int *detectBB, int *currBB);

private:
	float m_OverlapThreshold;
	float m_IntersectionThreshold;
	float m_MergeThreshold;

	int32_t m_KeepTrackingTime;

	mutable std::map<std::string, std::shared_ptr<Main> > m_mapTrack;
	std::map<std::string, std::shared_ptr<TldFeature> > m_mapRecvFeatures;
};


#endif /* TRACKALGTLD_H_ */
