/*
 * TrackingParam.h
 *
 *  Created on: 2016年1月14日
 *      Author: rayan
 */

#ifndef TRACKINGPARAM_H_
#define TRACKINGPARAM_H_


enum Retval {
    PROGRAM_EXIT = 0,
    SUCCESS = 1
};

enum ProcessState {
	detection_state = 0,
	tracking_state,
};

class CTrackAlg;

class trackingData {
public:
	trackingData() :
		img(NULL),
		pAlg(NULL),
		trackingThread(0) {}
	~trackingData() {}

	std::string objId;
	IplImage* img;
	CTrackAlg*	pAlg;

	// thread
	pthread_t 	trackingThread;
};


#endif /* TRACKINGPARAM_H_ */
