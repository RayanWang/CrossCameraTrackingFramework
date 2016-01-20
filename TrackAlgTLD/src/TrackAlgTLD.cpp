/*
 * TrackAlgTLD.cpp
 *
 *  Created on: Apr 24, 2015
 *      Author: Rayan
 */


#include "TrackAlgTLD.h"
#include "main/Main.h"
#include "TldFeature.h"

#include "tld/TLDUtil.h"

using namespace std;
using namespace cv;

CTrackAlgTLD::CTrackAlgTLD() :
		m_OverlapThreshold(0.1),
		m_IntersectionThreshold(0.3),
		m_MergeThreshold(0.5),
		m_KeepTrackingTime(600) {
	m_errCode = no_error;
	m_mapTrack.clear();
	m_mapRecvFeatures.clear();
}

CTrackAlgTLD::~CTrackAlgTLD() {
	release();
}

bool CTrackAlgTLD::init(void* pParam) {
	if (!pParam) {
		m_errCode = invalid_arg;
		return false;
	}

	TldParam* param = (TldParam*)(pParam);
	if (param->initTargetNum != param->vInitObjIdList.size()) {
		m_errCode = obj_count_not_match;
		return false;
	}

	m_OverlapThreshold = param->overlapThreshold;
	m_IntersectionThreshold = param->intersectionThreshold;
	m_MergeThreshold = param->mergeThreshold;
	m_KeepTrackingTime = param->keepTrackingTime;

	for (uint32_t i = 0; i < param->initTargetNum; ++i) {
		Main *pMain = new Main();
		pMain->setTrivialObjId(m_TrivialObjId++);

		m_mapTrack.insert(pair<string, Main*>(param->vInitObjIdList[i], pMain));
	}

	return true;
}

void CTrackAlgTLD::release(void) {
	map<string, Main*>::iterator it;
	for (it = m_mapTrack.begin(); it != m_mapTrack.end(); it++)
		delete it->second;
	m_mapTrack.clear();

	map<string, TldFeature*>::iterator it_feature;
	for (it_feature = m_mapRecvFeatures.begin(); it_feature != m_mapRecvFeatures.end(); it_feature++)
		delete it_feature->second;
	m_mapRecvFeatures.clear();
}

bool CTrackAlgTLD::prepareTracking(string& objId, IplImage* img, CvRect* pTargetBox) {
	if (m_mapTrack.find(objId) == m_mapTrack.end()) {
		m_errCode = obj_not_found;
		return false;
	}

	m_mapTrack[objId]->prepareWork(img, pTargetBox);

	return true;
}

bool CTrackAlgTLD::startTracking(string& objId, IplImage* img) {
	if (m_mapTrack.find(objId) == m_mapTrack.end()) {
		m_errCode = obj_not_found;
		return false;
	}

	if (!m_mapTrack[objId]->doWork(img)) {
		m_errCode = tracking_error;
		return false;
	}

	return true;
}

void CTrackAlgTLD::endTracking(string& objId) {
	if (m_mapTrack.find(objId) == m_mapTrack.end()) {
		m_errCode = obj_not_found;
		return;
	}

	m_mapTrack[objId]->endWork();
}

bool CTrackAlgTLD::addTracking(string& objId) {
	Main *pMain = new Main();
	pMain->setTrivialObjId(m_TrivialObjId++);

	pair<map<string, Main*>::iterator, bool> insertResult;
	insertResult = m_mapTrack.insert(pair<string, Main*>(objId, pMain));
	if (!insertResult.second)
		m_errCode = already_exist_error;

	return insertResult.second;
}

bool CTrackAlgTLD::mergeTracking(IplImage *img, CvRect DetectedBox) {
	if (m_mapTrack.empty()) {
		m_errCode = no_tracking_obj;
		return false;
	}

	if (!img) {
		m_errCode = invalid_arg;
		return false;
	}

	Rect detect;
	detect.x = DetectedBox.x;
	detect.y = DetectedBox.y;
	detect.width = DetectedBox.width;
	detect.height = DetectedBox.height;

	Mat grey;
	cvtColor(cvarrToMat(img), grey, CV_BGR2GRAY);

	float confident = 0;
	float tmp = 0;
	map<string, Main*>::iterator it;
	for (it = m_mapTrack.begin(); it != m_mapTrack.end(); it++) {
		if (it->second->tld->currBB) {
			int bb1[4];
			int bb2[4];
			tldRectToArray<int>(detect, bb1);
			tldRectToArray<int>(*it->second->tld->currBB, bb2);

			if ((tldOverlapRectRect(detect, *it->second->tld->currBB) > m_OverlapThreshold
					&& intersectionRatio(bb1, bb2) > m_IntersectionThreshold)
					|| tldIsInside(bb1, bb2) || tldIsInside(bb2, bb1)) {
				return true;
			}
		}
		/*else if (it->second->tld->currBB == NULL
		 && it->second->tld->finalBB->width != 0
		 && it->second->tld->finalBB->height != 0) {
		 int diffHeight = abs(it->second->tld->finalBB->y - detect.y);
		 int diffWidth = abs(it->second->tld->finalBB->x - detect.x);
		 if (diffHeight > it->second->tld->finalBB->height / 2
		 || diffHeight > detect.height / 2
		 || diffWidth > it->second->tld->finalBB->width / 2
		 || diffWidth > detect.width / 2)
		 return true;
		 }*/

		tmp = it->second->tld->nnClassifier->classifyBB(grey, &detect);
		if (tmp > confident)
			confident = tmp;
	}

	if (confident > m_MergeThreshold)
		return true;

	return false;
}

bool CTrackAlgTLD::willKeepTracking(string& objId, bool* pbKeepTrack) {
	if (m_mapTrack.find(objId) == m_mapTrack.end()) {
		m_errCode = obj_not_found;
		return false;
	}

	if (m_mapTrack[objId]->getRemovingCount() > m_KeepTrackingTime) {
		m_mapTrack[objId]->resetRemovingCount();

		m_mapTrack[objId]->endWork();

		delete m_mapTrack[objId];
		m_mapTrack[objId] = NULL;

		m_mapTrack.erase(objId);
		if (pbKeepTrack)
			*pbKeepTrack = false;
		return true;
	}

	if (pbKeepTrack)
		*pbKeepTrack = true;

	return true;
}

// set the object feature to TLD
bool CTrackAlgTLD::setObjectFeatures(int32_t cameraId, string& objIdFromOthers, void* feature,
		void* pFeatureDesc) {
	if (!feature || !pFeatureDesc) {
		m_errCode = invalid_arg;
		return false;
	}

	float* objFeature = (float*)(feature);
	TldFeatureDescription* desc = (TldFeatureDescription*)(pFeatureDesc);

	int32_t eachElementSize = desc->eachElementSize;	// TLD_PATCH_SIZE * TLD_PATCH_SIZE
	for (int32_t i = 0; i < desc->splitIndex; ++i) {
		float* pTruePositive = new float[eachElementSize];
		for (int32_t j = 0; j < eachElementSize; ++j)
			pTruePositive[j] = objFeature[i * eachElementSize + j];
		if (m_mapRecvFeatures.find(objIdFromOthers) == m_mapRecvFeatures.end()) {
			TldFeature* samples = new TldFeature();
			samples->cameraId = cameraId;
			samples->vTruePositives->push_back(pTruePositive);
			m_mapRecvFeatures.insert(pair<string, TldFeature*>(objIdFromOthers, samples));
		} else
			m_mapRecvFeatures[objIdFromOthers]->vTruePositives->push_back(pTruePositive);
	}
	for (int32_t i = desc->splitIndex; i < desc->totFeatureSize; ++i) {
		float* pFalsePositive = new float[eachElementSize];
		for (int32_t j = 0; j < eachElementSize; ++j)
			pFalsePositive[j] = objFeature[i * eachElementSize + j];
		if (m_mapRecvFeatures.find(objIdFromOthers) == m_mapRecvFeatures.end()) {
			TldFeature* samples = new TldFeature();
			samples->cameraId = cameraId;
			samples->vFalsePositives->push_back(pFalsePositive);
			m_mapRecvFeatures.insert(pair<string, TldFeature*>(objIdFromOthers, samples));
		} else
			m_mapRecvFeatures[objIdFromOthers]->vFalsePositives->push_back(pFalsePositive);
	}

	return true;
}

bool CTrackAlgTLD::identifyObject(string& objId, IplImage *img, CvRect detectedBox, int32_t& cameraId,
		string& objIdFromOthers) {
	if (m_mapTrack.find(objId) != m_mapTrack.end()) {
		if (false == m_mapTrack[objId]->isCheckedBefore()) { // satisfied at the beginning
			Rect detect;
			detect.x = detectedBox.x;
			detect.y = detectedBox.y;
			detect.width = detectedBox.width;
			detect.height = detectedBox.height;

			Mat grey;
			cvtColor(cvarrToMat(img), grey, CV_BGR2GRAY);
			float f = m_mapTrack[objId]->tld->nnClassifier->classifyFromOtherView(m_mapRecvFeatures,
							grey, &detect, cameraId, objIdFromOthers);
			printf("[TrackAlgTLD][identifyObject] Confident: %f, id: %s\n", f, objIdFromOthers.c_str());

			if (f >= m_MergeThreshold) {
				if (m_mapRecvFeatures[objIdFromOthers]) {
					delete m_mapRecvFeatures[objIdFromOthers];
					m_mapRecvFeatures[objIdFromOthers] = NULL;
				}
				m_mapRecvFeatures.erase(objIdFromOthers);

				m_mapTrack[objId]->setChecked(true);
				return true;
			}
		} else
			m_errCode = obj_already_checked;
	} else
		m_errCode = obj_not_found;

	return false;
}

void CTrackAlgTLD::setObjLeaveCallback(string& objId, cbTrackCallback cbTrack, void* userData) {
	m_cbTrack = cbTrack;
	m_userData = userData;
	if (m_mapTrack.find(objId) != m_mapTrack.end())
		m_mapTrack[objId]->tld->setCallbackFunc(objId, CTrackAlgTLD::tldCallbackFunc, this);
}

bool CTrackAlgTLD::isAnyObjToIdendify() const {
	if (m_mapRecvFeatures.size()  > 0)
		return true;

	return false;
}

void CTrackAlgTLD::getObjIdList(vector<string>& vObjId) const {
	map<string, Main*>::iterator it;
	for (it = m_mapTrack.begin(); it != m_mapTrack.end(); it++)
		vObjId.push_back(it->first);
}

float CTrackAlgTLD::intersectionRatio(int *detectBB, int *currBB) {
	int detectArea = detectBB[2] * detectBB[3];
	int currArea = currBB[2] * currBB[3];
	int intersectionWidth = 0;
	int intersectionHeight = 0;

	if (currBB[0] + currBB[2] > detectBB[0]) {
		intersectionWidth = currBB[2] - (detectBB[0] - currBB[0]);
		if (currBB[1] > detectBB[1] && currBB[1] < detectBB[1] + detectBB[3]) {
			intersectionHeight = detectBB[3] - (currBB[1] - detectBB[1]);
		} else if (detectBB[1] > currBB[1]
				&& detectBB[1] + detectBB[3] < currBB[1] + currBB[3]) {
			intersectionHeight = detectBB[3];
		} else if (detectBB[1] < currBB[1] + currBB[3]
				&& detectBB[1] + detectBB[3] > currBB[1] + currBB[3]) {
			intersectionHeight = detectBB[3]
					- ((detectBB[1] + detectBB[3]) - (currBB[1] + currBB[3]));
		}
	} else if (currBB[0] < detectBB[0]
			&& currBB[0] + currBB[2] > detectBB[0] + detectBB[2]) {
		intersectionWidth = detectBB[2];
		if (currBB[1] > detectBB[1] && currBB[1] < detectBB[1] + detectBB[3]) {
			intersectionHeight = detectBB[3] - (currBB[1] - detectBB[1]);
		} else if (detectBB[1] < currBB[1] + currBB[3]
				&& detectBB[1] + detectBB[3] > currBB[1] + currBB[3]) {
			intersectionHeight = detectBB[3]
					- ((detectBB[1] + detectBB[3]) - (currBB[1] + currBB[3]));
		}
	} else if (currBB[0] > detectBB[0]
			&& currBB[0] < detectBB[0] + detectBB[2]) {
		intersectionWidth = detectBB[2] - (currBB[0] - detectBB[0]);
		if (currBB[1] > detectBB[1] && currBB[1] < detectBB[1] + detectBB[3]) {
			intersectionHeight = detectBB[3] - (currBB[1] - detectBB[1]);
		} else if (detectBB[1] > currBB[1]
				&& detectBB[1] + detectBB[3] < currBB[1] + currBB[3]) {
			intersectionHeight = detectBB[3];
		} else if (detectBB[1] < currBB[1] + currBB[3]
				&& detectBB[1] + detectBB[3] > currBB[1] + currBB[3]) {
			intersectionHeight = detectBB[3]
					- ((detectBB[1] + detectBB[3]) - (currBB[1] + currBB[3]));
		}
	}

	if (intersectionWidth != 0 && intersectionHeight != 0) {
		if (detectArea < currArea)
			return (float) ((float) (intersectionWidth * intersectionHeight)
					/ detectArea);
		else
			return (float) ((float) (intersectionWidth * intersectionHeight)
					/ currArea);
	} else
		return 0.0;
}
