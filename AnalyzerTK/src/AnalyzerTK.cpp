//============================================================================
// Name        : AnalyzerTK.cpp
// Author      : Rayan
// Version     :
// Copyright   : Your copyright notice
// Description : AnalyzerTK in C++, Ansi-style
//============================================================================

#include <iostream>
#include <getopt.h>
#include <CCFramework/Transfer/FeatureTransfer.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#include <TrackingParam.h>

#include "TrackAlgTLD.h"
#include "TrackingGenerate.h"
#include "adaboostDetect.h"

map<string, CreateClass> ClassFactory::m_classMap;
IMPLEMENT_RUNTIME(CTrackAlgTLD)

using namespace std;
using namespace ccFramework::transfer;
using namespace boost::uuids;

#define ANALYZER_TRACK_WINDOW "AnalyzerTK"

int32_t g_CameraId = -1;

void recvData(int32_t fromCameraId, uuid objId, void* feature,
		uint32_t sizeofFeature, void* desc, uint32_t sizeofDesc, void* userData) {
	CTrackAlg* pTKAlg = (CTrackAlg*)userData;

	string strObjId = boost::lexical_cast<string>(objId);
	TldFeatureDescription* tldFeature = new TldFeatureDescription;
	memcpy(tldFeature, desc, sizeof(TldFeatureDescription));

	pTKAlg->setObjectFeatures(fromCameraId, strObjId, feature, tldFeature);
	delete tldFeature;
}

void sendData(void* userData, string& objId, void* feature, uint32_t featureSizeInByte,
		void* pDesc, uint32_t descSizeInByte) {
	if (!userData)
		return;

	FeatureTransfer* transfer = (FeatureTransfer*)userData;
	uuid obj_uuid = boost::lexical_cast<uuid>(objId);
	transfer->sendFeatureToAll(g_CameraId, obj_uuid, feature,
			featureSizeInByte, pDesc, descSizeInByte);
}

void* startTrackingThread(void* lpParam) {
	if (!lpParam)
		pthread_exit(0);

	trackingData* pTracking = (trackingData*) lpParam;
	pTracking->pAlg->startTracking(pTracking->objId, pTracking->img);

	pthread_exit(0);
}

int main(int argc, char **argv) {
	string strDetectionModelPath;
	string strVideoPath;
	TldParam tldParam;
	std::shared_ptr<TrackingGenerate> pTrackGen = make_shared<TrackingGenerate>();

	int c;
	while ((c = getopt(argc, argv, "c:k:l:r:v:o:a:m:")) != -1) {
		string str(optarg);
		switch (c) {
		case 'c':
			g_CameraId = atoi(str.c_str());
			break;
		case 'k':
			tldParam.keepTrackingTime = atoi(str.c_str());
			break;
		case 'l':
			strDetectionModelPath = str;
			break;
		case 'r':
			pTrackGen->SetTrackingAlg(str);
			break;
		case 'v':
			strVideoPath = str;
			break;
		case 'o':
			tldParam.overlapThreshold = strtof(str.c_str(), NULL);
			break;
		case 'a':
			tldParam.intersectionThreshold = strtof(str.c_str(), NULL);
			break;
		case 'm':
			tldParam.mergeThreshold = strtof(str.c_str(), NULL);
			break;
		}
	}

	VideoCapture capture;
	if (!capture.open(strVideoPath)) {
		cout << "--(!) Error opening video capture" << endl;
		return 0;
	}

	FeatureTransfer* transfer = FeatureTransfer::getInstance();
	if (!transfer->initTransfer(g_CameraId)) {
		error_code_transfer error = transfer->getTransferError();
		cout << "--(!) Transfer init error" << error << endl;
		return 0;
	}

	std::shared_ptr<adaboostDetect> detect = make_shared<adaboostDetect>(strDetectionModelPath,
			CV_HAAR_SCALE_IMAGE, 3, 20, 30, 1.1);
	CTrackAlg* pTKAlg = pTrackGen->CreateTrackingObject();

	transfer->setRecvCallback(recvData, pTKAlg);

	int32_t nCurrState = detection_state;
	int32_t nHeads = 0;
	vector<CvRect> vRegions;
	vector<string> vObjIdList;
	vector<trackingData*> vTrackObj;
	Mat frame;
	while (capture.read(frame)) {
		double t1 = 0;
		double t2 = 0;
		t1 = (double) cvGetTickCount();

		vRegions.clear();
		IplImage img = IplImage(frame);
		nHeads = detect->detectObject(&img, vRegions);
		if (detection_state == nCurrState) {
			if (nHeads && pTKAlg) {
				tldParam.initTargetNum = nHeads;
				for (int32_t i = 0; i < nHeads; ++i) {
					uuid objId = random_generator()();
					string strObjId = boost::lexical_cast<string>(objId);
					tldParam.vInitObjIdList.push_back(strObjId);
				}
				pTKAlg->init(&tldParam);
				for (int32_t i = 0; i < nHeads; ++i) {
					if (false == pTKAlg->prepareTracking(tldParam.vInitObjIdList[i], &img, &vRegions[i]))
						goto exit;
					else
						pTKAlg->setObjLeaveCallback(tldParam.vInitObjIdList[i], sendData, transfer);
				}

				nCurrState = tracking_state;
				pTKAlg->getObjIdList(vObjIdList);

				transfer->startRecvFeatures();
			}
		}
		if (tracking_state == nCurrState) {
			//-------------------------- start tracking -----------------------------
			for (uint32_t i = 0; i < vObjIdList.size(); ++i) {
				// make sure whether this target leave out the camera view and never come  back again
				bool bKeepTrack = false;
				if (pTKAlg->willKeepTracking(vObjIdList[i], &bKeepTrack)) {
					if (bKeepTrack) {
						trackingData *pTracking = new trackingData();
						pTracking->objId = vObjIdList[i];
						pTracking->img = &img;
						pTracking->pAlg = pTKAlg;
						pthread_create(&pTracking->trackingThread, NULL,
								startTrackingThread, pTracking);
						vTrackObj.push_back(pTracking);
					} else {
						vObjIdList.clear();
						pTKAlg->getObjIdList(vObjIdList);
					}
				}
			}

			// wait until the tracking of all targets have been finished
			for (uint32_t i = 0; i < vTrackObj.size(); ++i) {
				pthread_join(vTrackObj[i]->trackingThread, NULL);
				delete vTrackObj[i];
			}
			vTrackObj.clear();
			//---------------------------- end tracking -----------------------------

			for (int32_t i = 0; i < nHeads; ++i) {
				bool bObjFromOtherCam = false;
				if (pTKAlg->isAnyObjToIdendify()) {
					int32_t cameraId = 1 >> 31;
					string objId;
					for (uint32_t j = 0; j < vObjIdList.size(); ++j) {
						if (pTKAlg->identifyObject(vObjIdList[j], &img, vRegions[i], cameraId, objId)) {
							bObjFromOtherCam = true;
							printf(
									"[AnalyzerTK][main] The target in camera %d with id %s is from camera %d with id %s\n",
									g_CameraId, vObjIdList[i].c_str(), cameraId,
									objId.c_str());
						}
					}
				}

				if (!bObjFromOtherCam && !pTKAlg->mergeTracking(&img, vRegions[i])) {
						uuid objId = random_generator()();
						string strObjId = boost::lexical_cast<string>(objId);
						pTKAlg->addTracking(strObjId);
						vObjIdList.clear();
						pTKAlg->getObjIdList(vObjIdList);

						if (!pTKAlg->prepareTracking(strObjId, &img, &vRegions[i]))
							goto exit;
						else
							pTKAlg->setObjLeaveCallback(strObjId, sendData, transfer);
				}
			}
		}

		imshow(ANALYZER_TRACK_WINDOW, frame);
		int esc = waitKey(20);
		if ((char)esc == 27)
			break;
	}

exit:
	FeatureTransfer::releaseInstance();
	pTKAlg->release();
	delete pTKAlg;

	RELEASE_RUNTIME(CTrackAlgTLD)

	return 0;
}
