//============================================================================
// Name        : TrackingManager.cpp
// Author      : Rayan
// Version     :
// Copyright   : Your copyright notice
// Description : TrackingManager in C++, Ansi-style
//============================================================================

#include <iostream>
#include <getopt.h>
#include <CCFramework/Transfer/CrossCameraManager.hpp>

using namespace std;
using namespace ccFramework::transfer;

int main(int argc, char **argv) {
	int32_t totCameraNum = 0;
	int32_t maxNumOfTarget = 0;
	int32_t c = -1;
	while ((c = getopt(argc, argv, "t:m:")) != -1) {
		string str(optarg);
		switch (c) {
		case 't':
			totCameraNum = atoi(str.c_str());
			break;
		case 'm':
			maxNumOfTarget = atoi(str.c_str());
			break;
		}
	}

	CrossCameraManager manager;
	manager.initManagerData(totCameraNum, maxNumOfTarget);

	do {
		usleep(1000);
	} while (true);

	return 0;
}
