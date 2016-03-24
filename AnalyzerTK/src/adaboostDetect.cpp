#include <stdio.h>
#include "adaboostDetect.h"

adaboostDetect::adaboostDetect(string strModelPath, int flags, int minNeighbours,
		double minSize, double maxSize, double scaleFactor) :
		m_flags(0 | CV_HAAR_SCALE_IMAGE),
		m_minNeighbours(minNeighbours),
		m_minSize(minSize),
		m_maxSize(maxSize),
		m_scaleFactor(scaleFactor) {
	m_pStorage = cvCreateMemStorage(0);

    if (!m_FaceCascade.load(strModelPath)) {
		cout << "--(!)Error loading face cascade" << endl;
	}
}

adaboostDetect::~adaboostDetect() {
    cvReleaseMemStorage(&m_pStorage);
}

int adaboostDetect::detectObject(IplImage* img, vector<CvRect> & regions) {
	double t = 0;
	t = (double) cvGetTickCount();

	IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
	IplImage* smallImg = cvCreateImage(
			cvSize(cvRound(img->width / m_scaleFactor),
					cvRound(img->height / m_scaleFactor)), 8, 1);
	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvResize(gray, smallImg, CV_INTER_LINEAR);
	cvEqualizeHist(smallImg, smallImg);
	cvClearMemStorage(m_pStorage);

#ifdef _GPU_DETECTION_
	Rect* faces;
	GpuMat faces_gpu;

	GpuMat gray_gpu(smallImg);

	Size detectsize = Size(m_minSize, m_minSize);
	int detect_num = m_FaceCascade.detectMultiScale(gray_gpu, faces_gpu,
			m_scaleFactor, m_minNeighbours, detectsize);
	Mat obj_host;
	faces_gpu.colRange(0, detect_num).download(obj_host);
	faces = obj_host.ptr<Rect>();

	int nx1, nx2, ny1, ny2;
	for (int i = 0; i < detect_num; ++i) {
		if ((faces[i].width <= m_maxSize) && (faces[i].height <= m_maxSize)) {
			nx1 = cvRound(faces[i].x * m_scaleFactor);
			ny1 = cvRound(faces[i].y * m_scaleFactor);
			nx2 = cvRound((faces[i].x + faces[i].width) * m_scaleFactor);
			ny2 = cvRound((faces[i].y + faces[i].height) * m_scaleFactor);

			CvRect cvRc;
			cvRc = cvRect(nx1, ny1, nx2 - nx1, ny2 - ny1);
			regions.push_back(cvRc);
		}
	}
#else
	Mat mat_small(smallImg, 0);

	int nx1, nx2, ny1, ny2;

	vector<Rect> faces;
	m_FaceCascade.detectMultiScale(mat_small, faces, m_scaleFactor, m_minNeighbours,
			m_flags, Size(m_minSize, m_minSize), Size(m_maxSize, m_maxSize));

	for (unsigned int i = 0; i < faces.size(); ++i) {
		if ((faces[i].width <= m_maxSize) && (faces[i].height <= m_maxSize)) {
			nx1 = cvRound(faces[i].x * m_scaleFactor);
			ny1 = cvRound(faces[i].y * m_scaleFactor);
			nx2 = cvRound((faces[i].x + faces[i].width) * m_scaleFactor);
			ny2 = cvRound((faces[i].y + faces[i].height) * m_scaleFactor);

			CvRect cvRc;
			cvRc = cvRect(nx1, ny1, nx2 - nx1, ny2 - ny1);
			regions.push_back(cvRc);
		}
	}
#endif

	cvReleaseImage(&gray);
	cvReleaseImage(&smallImg);

	t = (double) cvGetTickCount() - t;
	//printf("detection time = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));

	return regions.size();
}
