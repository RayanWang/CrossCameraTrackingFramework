/*
 * TldFeature.h
 *
 *  Created on: 2016年1月13日
 *      Author: rayan
 */

#ifndef TLDFEATURE_H_
#define TLDFEATURE_H_


#include <vector>

class TldFeature {
public:
	TldFeature() :
		cameraId(1 >> 31) {
		vFalsePositives = new std::vector<float*>();
		vTruePositives = new std::vector<float*>();
	}
	~TldFeature() {
		for (uint32_t i = 0; i < vFalsePositives->size(); ++i)
			if (vFalsePositives->at(i))
				delete vFalsePositives->at(i);
		vFalsePositives->clear();
		delete vFalsePositives;

		for (uint32_t i = 0; i < vTruePositives->size(); ++i)
			if (vTruePositives->at(i))
				delete vTruePositives->at(i);
		vTruePositives->clear();
		delete vTruePositives;
	}

	int32_t cameraId;
	std::vector<float*>* vFalsePositives;
	std::vector<float*>* vTruePositives;
};


#endif /* TLDFEATURE_H_ */
