/*
 * TrackingGenerate.h
 *
 *  Created on: 2015年6月18日
 *      Author: rayan
 */

#ifndef SRC_TRACKINGGENERATE_H_
#define SRC_TRACKINGGENERATE_H_

#include <string>
#include "ClassFactory.h"

using namespace std;

class CTrackAlg;

class TrackingGenerate {
public:
	TrackingGenerate() {}
	virtual ~TrackingGenerate() {}

	CTrackAlg* CreateTrackingObject() {
			string ClassName = "CTrackAlg" + m_strAlgName;
			return (CTrackAlg*)ClassFactory::GetClassByName(ClassName);
	}

	void SetTrackingAlg(string algName) { m_strAlgName = algName; }

private:
	string m_strAlgName;
};

#endif /* SRC_TRACKINGGENERATE_H_ */
