#ifndef LK_H_
#define LK_H_

#include <opencv/cv.h>

class LK {
public:
	LK(void) {
		win_size_lk = 4;
		for(int i = 0; i < 3; ++i) {
			points[i] = 0;
		}
		PYR = 0;
	}
	~LK(void) {
		if(PYR) {
			free(PYR);
			PYR = 0;
		}
	}
	/**
	 * Need before start of trackLK and at the end of the program for cleanup.
	 */
	void initImgs();
	int trackLK(IplImage *imgI, IplImage *imgJ, float ptsI[], int nPtsI,
			float ptsJ[], int nPtsJ, int level, float *fbOut, float *nccOut,
			char *statusOut);

private:
	int win_size_lk;
	CvPoint2D32f *points[3];
	IplImage **PYR;
};

#endif /* LK_H_ */
