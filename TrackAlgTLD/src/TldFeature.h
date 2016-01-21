/*
 * TldFeature.h
 *
 *  Created on: 2016年1月13日
 *      Author: rayan
 */

#ifndef TLDFEATURE_H_
#define TLDFEATURE_H_


#include <vector>
#include <memory>

class TldFeature {
public:
	TldFeature() :
		cameraId(1 >> 31) {
		vFalsePositives = std::make_shared<std::vector<std::unique_ptr<float[]> > >();
		vTruePositives = std::make_shared<std::vector<std::unique_ptr<float[]> > >();
	}
	~TldFeature() {
		vFalsePositives->clear();
		vTruePositives->clear();
	}

	int32_t cameraId;
	std::shared_ptr<std::vector<std::unique_ptr<float[]> > > vFalsePositives;
	std::shared_ptr<std::vector<std::unique_ptr<float[]> > > vTruePositives;
};


#endif /* TLDFEATURE_H_ */
