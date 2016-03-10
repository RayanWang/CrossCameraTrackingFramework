#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda_device_runtime_api.h>
#include <opencv2/gpu/gpu.hpp>

#include "DetectorCascade.h"

using namespace tld;

void detectionWithCuda(DetectorCascade *detectorCascade, const cv::Mat &img, detectionData &detectCuda);
