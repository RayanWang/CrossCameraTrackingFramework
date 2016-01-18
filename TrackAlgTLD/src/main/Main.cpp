#include "Main.h"

#include "../tld/TLDUtil.h"

using namespace tld;
using namespace cv;

Main::Main()
{
	tld = new tld::TLD();
	showOutput = 1;
	printResults = NULL;
	saveDir = ".";
	threshold = 0.5;
	showForeground = 0;

	showTrajectory = false;
	trajectoryLength = 0;

	selectManually = 0;

	initialBB = NULL;
	showNotConfident = true;

	reinit = 0;

	loadModel = false;

	exportModelAfterRun = false;
	modelExportFile = "model";
	seed = 0;

	m_breuseFrameOnce = false;
	m_bskipProcessingOnce = false;
	m_presultsFile = NULL;

	modelPath = NULL;

	prevBB = NULL;
	m_nDecideForRemoving = 0;
	m_nTimeOfStopFixed = 0;
	m_nTimeOfStopMoved = 0;

	m_bChecked = false;

	m_TrivialObjId = 0;
}

Main::~Main()
{
	if(tld) {
		delete tld;
		tld = NULL;
	}
	if(prevBB) {
		delete prevBB;
		prevBB = NULL;
	}
}

void Main::prepareWork(IplImage *img, CvRect *pTargetBox) {
	Mat grey(img->height, img->width, CV_8UC1);
	grey.copyTo(m_grey);
	cvtColor(cvarrToMat(img), m_grey, CV_BGR2GRAY);

	tld->detectorCascade->imgWidth = m_grey.cols;
	tld->detectorCascade->imgHeight = m_grey.rows;
	tld->detectorCascade->imgWidthStep = m_grey.step;

	if (showTrajectory) {
		m_trajectory.init(trajectoryLength);
	}

	if (pTargetBox) {
		if (initialBB == NULL) {
			initialBB = new int[4];
		}

		if (selectManually) {

			CvRect box;

			/*if (getBBFromUser(img, box, gui) == PROGRAM_EXIT) {
			 return;
			 }*/

			initialBB[0] = pTargetBox->x;
			initialBB[1] = pTargetBox->y;
			initialBB[2] = pTargetBox->width;
			initialBB[3] = pTargetBox->height;
		} else {
			initialBB[0] = pTargetBox->x;
			initialBB[1] = pTargetBox->y;
			initialBB[2] = pTargetBox->width;
			initialBB[3] = pTargetBox->height;
		}
	}

	if (printResults != NULL) {
		m_presultsFile = fopen(printResults, "w");
		if (!m_presultsFile) {
			fprintf(stderr, "Error: Unable to create results-file \"%s\"\n",
					printResults);
			exit(-1);
		}
	}

	if (loadModel && modelPath != NULL) {
		tld->readFromFile(modelPath);
		m_breuseFrameOnce = true;
	} else if (initialBB != NULL) {
		Rect bb = tldArrayToRect(initialBB);

		printf("Starting at %d %d %d %d\n", bb.x, bb.y, bb.width, bb.height);

		tld->selectObject(m_grey, &bb);
		m_bskipProcessingOnce = true;
		m_breuseFrameOnce = true;
	}
}

bool Main::doWork(IplImage *img) {
	double tic = cvGetTickCount();

	if (!m_breuseFrameOnce) {
		if (img == NULL) {
			printf("current image is NULL, assuming end of input.\n");
			return false;
		}
	}
	cvtColor(cvarrToMat(img), m_grey, CV_BGR2GRAY);

	if (!m_bskipProcessingOnce)
		tld->processImage(cvarrToMat(img));
	else
		m_bskipProcessingOnce = false;

	if(tld->currBB) {
		m_nDecideForRemoving = 0;
		if(prevBB) {
			if(prevBB->x == tld->currBB->x && prevBB->y == tld->currBB->y) {
				if(tld->learningEnabled) {
					++m_nTimeOfStopFixed;
					if (30 == m_nTimeOfStopFixed) {
						tld->learningEnabled = false;
						m_nTimeOfStopFixed = 0;
					}
				}
			} /*else {
				++m_nTimeOfStopMoved;
				if(tld->learningEnabled) {
					if (30 == m_nTimeOfStopMoved) {
						tld->learningEnabled = false;
					}
				} else if (45 == m_nTimeOfStopMoved) {
					tld->learningEnabled = true;
					m_nTimeOfStopMoved = 0;
				}
			}*/
		}
		else
			tld->learningEnabled = true;

		if (prevBB) {
			delete prevBB;
			prevBB = NULL;
		}
		prevBB = new Rect();
		memcpy(prevBB, tld->currBB, sizeof(Rect));
	}
	else {
		++m_nDecideForRemoving;
		if(prevBB) {
			delete prevBB;
			prevBB = NULL;
		}
	}

	double toc = (cvGetTickCount() - tic) / cvGetTickFrequency();

	toc = toc / 1000000;

	float fps = 1 / toc;

	bool confident = (tld->currConf >= threshold) ? true : false;

	if (showOutput || saveDir != NULL) {
		char string[128];

		char learningString[10] = "";

		if (tld->learning) {
			strcpy(learningString, "Learning");
		}

		sprintf(string, "Similarity %.2f, object %lld, %s", tld->currConf, m_TrivialObjId, learningString);
		CvScalar red = CV_RGB(255, 0, 0);
		CvScalar green = CV_RGB(0, 255, 0);
		CvScalar black = CV_RGB(0, 0, 0);
		CvScalar white = CV_RGB(255, 255, 255);

		if (tld->currBB != NULL) {
			CvScalar rectangleColor = (confident) ? green : red;
			cvRectangle(img, tld->currBB->tl(), tld->currBB->br(),
					rectangleColor, 1, 8, 0);

			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_PLAIN|CV_FONT_ITALIC, 1, 1, 0, 1);

			char buffer [4];
			sprintf(buffer, "%lld", m_TrivialObjId);
			cvPutText(img, buffer, tld->currBB->tl(), &font, red);

			if (showTrajectory) {
				CvPoint center = cvPoint(
						tld->currBB->x + tld->currBB->width / 2,
						tld->currBB->y + tld->currBB->height / 2);
				cvLine(img, cvPoint(center.x - 2, center.y - 2),
						cvPoint(center.x + 2, center.y + 2), rectangleColor, 2);
				cvLine(img, cvPoint(center.x - 2, center.y + 2),
						cvPoint(center.x + 2, center.y - 2), rectangleColor, 2);
				m_trajectory.addPoint(center, rectangleColor);
			}
		}
		else if (showTrajectory) {
			m_trajectory.addPoint(cvPoint(-1, -1), cvScalar(-1, -1, -1));
		}

		if (showTrajectory) {
			m_trajectory.drawTrajectory(img);
		}

		if(tld->currBB) {
			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 1, 8);
			cvRectangle(img, cvPoint(0, 0), cvPoint(img->width, 20), black, CV_FILLED, 8, 0);
			cvPutText(img, string, cvPoint(12, 12), &font, white);
		}

		if (showForeground) {
			for (size_t i = 0; i < tld->detectorCascade->detectionResult->fgList->size(); i++) {
				Rect r = tld->detectorCascade->detectionResult->fgList->at(i);
				cvRectangle(img, r.tl(), r.br(), white, 1);
			}
		}

		/*if (showOutput) {
			gui->showImage(img);
			char key = gui->getKey();

			if (key == 'q')
				return;

			if (key == 'b') {
				ForegroundDetector *fg =
						tld->detectorCascade->foregroundDetector;

				if (fg->bgImg.empty()) {
					fg->bgImg = m_grey.clone();
				} else {
					fg->bgImg.release();
				}
			}

			if (key == 'c') {
				//clear everything
				tld->release();
			}

			if (key == 'l') {
				tld->learningEnabled = !tld->learningEnabled;
				printf("LearningEnabled: %d\n", tld->learningEnabled);
			}

			if (key == 'a') {
				tld->alternating = !tld->alternating;
				printf("alternating: %d\n", tld->alternating);
			}

			if (key == 'e') {
				tld->writeToFile(modelExportFile);
			}

			if (key == 'i') {
				tld->readFromFile(modelPath);
			}

			if (key == 'r') {
				CvRect box;

				if (getBBFromUser(img, box, gui) == PROGRAM_EXIT) {
					return;
				}

				Rect r = Rect(box);

				tld->selectObject(m_grey, &r);
			}
		}*/
	}

	if (m_breuseFrameOnce) {
		m_breuseFrameOnce = false;
	}
}

void Main::endWork(void) {
	if (exportModelAfterRun) {
		tld->writeToFile(modelExportFile);
	}

	if (m_presultsFile) {
		fclose (m_presultsFile);
	}
}
