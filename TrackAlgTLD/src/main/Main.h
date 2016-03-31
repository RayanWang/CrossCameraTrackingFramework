#ifndef MAIN_H_
#define MAIN_H_

#include "../tld/TLD.h"
#include "Trajectory.h"

enum Retval
{
    PROGRAM_EXIT = 0,
    SUCCESS = 1
};

using namespace tld;

class Main {
public:
	TLD *tld;
	bool showOutput;
	bool showTrajectory;
	int trajectoryLength;
	const char *printResults;
	const char *saveDir;
	double threshold;
	bool showForeground;
	bool showNotConfident;
	bool selectManually;
	int *initialBB;
	bool reinit;
	bool exportModelAfterRun;
	bool loadModel;
	const char *modelPath;
	const char *modelExportFile;
	int seed;

	Main();
	~Main();

	void prepareWork(IplImage *img, std::string stWinName, CvRect *pTargetBox = NULL);

	bool doWork(IplImage *img);

	void endWork(void);

	int getRemovingCount(void) {
		return m_nDecideForRemoving;
	}

	void resetRemovingCount(void) {
		m_nDecideForRemoving = 0;
	}

	bool isCheckedBefore(void) {
		return m_bChecked;
	}

	void setChecked(bool bChecked) {
		m_bChecked = bChecked;
	}

	void setTrivialObjId(long long trivialObjId) { m_TrivialObjId = trivialObjId; }
	long long getTrivialObjId() { return m_TrivialObjId; }

private:
	bool m_breuseFrameOnce;
	bool m_bskipProcessingOnce;
	Trajectory m_trajectory;
	cv::Mat m_grey;
	FILE *m_presultsFile;

	cv::Rect *prevBB;
	int m_nDecideForRemoving;
	int m_nTimeOfStopFixed;
	int m_nTimeOfStopMoved;
	bool m_bChecked;

	long long m_TrivialObjId;
};

#endif /* MAIN_H_ */
