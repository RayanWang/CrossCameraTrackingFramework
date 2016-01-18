/*
 * TrackAlg.h
 *
 *  Created on: Apr 27, 2015
 *      Author: shalo
 */

#ifndef TRACKALG_H_
#define TRACKALG_H_

#include <opencv2/core/types_c.h>

#include "ClassFactory.h"

#define DECLARE_RUNTIME(class_name)	\
		static GenDynamic* class_name##GD

#define IMPLEMENT_RUNTIME(class_name)	\
		GenDynamic* class_name::class_name##GD	\
		= new GenDynamic(#class_name, class_name::CreateInstance);

#define RELEASE_RUNTIME(class_name)	\
		delete class_name::class_name##GD;

class GenDynamic {
public:
	GenDynamic(std::string name, CreateClass method) {
		ClassFactory::RegistClass(name, method);
	}
};

typedef void (*cbTrackCallback)(void*, std::string&, void*, uint32_t, void*, uint32_t);

class CTrackAlg {
public:
	CTrackAlg() : m_TrivialObjId(1 >> 63), m_errCode(1 >> 31), m_cbTrack(NULL), m_userData(NULL) {}
	virtual ~CTrackAlg() {}

	static void* CreateInstance() { return NULL; }

	virtual bool init(void* pParam) = 0;

	virtual void release(void) = 0;

	virtual bool prepareTracking(std::string& objId, IplImage* img, CvRect* pTargetBox = NULL) = 0;

	virtual bool startTracking(std::string& objId, IplImage* img) = 0;

	virtual void endTracking(std::string& objId) = 0;

	virtual bool addTracking(std::string& objId)  = 0;

	virtual bool mergeTracking(IplImage *img, CvRect detectedBox, bool* pbMerged) = 0;

	virtual bool willKeepTracking(std::string& objId, bool* pbKeepTrack) = 0;

	virtual bool setObjectFeatures(int32_t cameraId, std::string& objIdFromOthers, void* feature,
			void* pFeatureDesc = NULL) = 0;

	virtual bool identifyObject(std::string& objId, IplImage *img,
			int32_t& cameraId, std::string& objIdFromOthers) = 0;

	virtual void setObjLeaveCallback(std::string& objId, cbTrackCallback cbTrack, void* userData) = 0;

	virtual bool isAnyObjToIdendify(void) const = 0;

	virtual void getObjIdList(std::vector<std::string>& vObjId) const = 0;

	int32_t getLastError(void) const { return m_errCode; }

protected:
	long long m_TrivialObjId;
	int32_t m_errCode;
	cbTrackCallback m_cbTrack;
	void* m_userData;
};

#endif /* TRACKALG_H_ */
