#include "kernel.h"

using namespace std;
using namespace cv;
using namespace cv::gpu;

struct refInfo {
	int numWindows;
	bool isActive;
	int fgListSize;
	int truePositiveSize;
	int falsePositiveSize;
};

struct proData {
	int window[TLD_WINDOW_SIZE];

	// varianceFilter
	int varianceWindowOffsets[TLD_WINDOW_OFFSET_SIZE];

	// ensembleClassifier
	int ensembleWindowOffsets[TLD_WINDOW_OFFSET_SIZE];
	int featureVector[TLD_NUM_TREES];
	unsigned char subImgData[TLD_PATCH_SIZE * TLD_PATCH_SIZE];
	size_t step;
};

__global__ void detection(refInfo* info, proData *data, float *falsePositives,
		float *truePositives, int *ii, long long *ii_squared,
		int *featureOffsets, float *ensemblePosteriors, CvRect *fgList,
		unsigned char *imgData, float *posteriors, float *variances,
		int *indexes) {

	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if (i < (*info).numWindows) {
		if ((*info).isActive) {
			bool isInside = false;
			for (int j = 0; j < (*info).fgListSize; ++j) {
				int bgBox[4];
				bgBox[0] = fgList[j].x;
				bgBox[1] = fgList[j].y;
				bgBox[2] = fgList[j].width;
				bgBox[3] = fgList[j].height;

				//TODO: This is inefficient and should be replaced by a quadtree
				if (data[i].window[0] > bgBox[0] &&
					 data[i].window[1] > bgBox[1] &&
					 data[i].window[0] + data[i].window[2] < bgBox[0] + bgBox[2] &&
					 data[i].window[1] + data[i].window[3] < bgBox[1] + bgBox[3])
					isInside = true;
			}

			if (!isInside) {
				posteriors[i] = 0;
				indexes[i] = -1;
				return;
			}
		}

		// executing varianceFilter
		float mX = (ii[data[i].varianceWindowOffsets[3]] -
							ii[data[i].varianceWindowOffsets[2]] -
							ii[data[i].varianceWindowOffsets[1]] +
							ii[data[i].varianceWindowOffsets[0]]) /
							(float) data[i].varianceWindowOffsets[5]; //Sum of Area divided by area
		float mX2 = (ii_squared[data[i].varianceWindowOffsets[3]] -
							  ii_squared[data[i].varianceWindowOffsets[2]] -
							  ii_squared[data[i].varianceWindowOffsets[1]] +
							  ii_squared[data[i].varianceWindowOffsets[0]]) /
							  (float) data[i].varianceWindowOffsets[5];
		float bboxvar = mX2 - mX * mX;
		variances[i] = bboxvar;
		if (bboxvar < 0) {
			posteriors[i] = 0;
			indexes[i] = -1;
			return;
		}

		// executing ensembleClassifier
		float conf = 0.0;
		for(int n = 0; n < TLD_NUM_TREES; ++n) {
			int index = 0;
			int *off = featureOffsets + data[i].ensembleWindowOffsets[4]
					+ n * 2 * TLD_NUM_FEATURES; //bbox[4] is pointer to features for the current scale

			for (int j = 0; j < TLD_NUM_FEATURES; ++j) {
				index <<= 1;

				int fp0 = imgData[data[i].ensembleWindowOffsets[0] + off[0]];
				int fp1 = imgData[data[i].ensembleWindowOffsets[0] + off[1]];

				if (fp0 > fp1) {
					index |= 1;
				}

				off += 2;
			}

			data[i].featureVector[n] = index;
			int indice = pow(2.0f, 13);
			conf += ensemblePosteriors[n * indice + data[i].featureVector[n]];
		}
		posteriors[i] = conf;

		if (posteriors[i] < 0.5) {
			indexes[i] = -1;
			return;
		}

		// executing nnClassifier
		float mean = 0;
		for (int k = 0; k < TLD_PATCH_SIZE; ++k) {
			for (int l = 0; l < TLD_PATCH_SIZE; ++l) {
				mean += data[i].subImgData[l * data[i].step + k];
			}
		}

		mean /= TLD_PATCH_SIZE * TLD_PATCH_SIZE;

		float values[TLD_PATCH_SIZE * TLD_PATCH_SIZE];
		for (int k = 0; k < TLD_PATCH_SIZE; ++k) {
			for (int l = 0; l < TLD_PATCH_SIZE; ++l) {
				values[l * TLD_PATCH_SIZE + k] = data[i].subImgData[l * data[i].step + k] - mean;
			}
		}

		conf = 0;
		int size = TLD_PATCH_SIZE * TLD_PATCH_SIZE;

		float ccorr_max_p = 0;

		//Compare patch to positive patches
		for (int j = 0; j < info->truePositiveSize; ++j) {
			double corr = 0;
			double norm1 = 0;
			double norm2 = 0;

			for (int k = 0; k < size; ++k) {
				corr += truePositives[k] * values[k];
				norm1 += truePositives[k] * truePositives[k];
				norm2 += values[k] * values[k];
			}

			// normalization to <0,1>

			float ccorr = (corr / sqrt(norm1 * norm2) + 1) / 2.0;

			if (ccorr > ccorr_max_p) {
				ccorr_max_p = ccorr;
			}
		}

		float ccorr_max_n = 0;

		//Compare patch to negative patches
		for (int j = 0; j < info->falsePositiveSize; ++j) {
			double corr = 0;
			double norm1 = 0;
			double norm2 = 0;

			for (int k = 0; k < size; ++k) {
				corr += falsePositives[k] * values[k];
				norm1 += falsePositives[k] * falsePositives[k];
				norm2 += values[k] * values[k];
			}

			// normalization to <0,1>

			float ccorr = (corr / sqrt(norm1 * norm2) + 1) / 2.0;

			if (ccorr > ccorr_max_n) {
				ccorr_max_n = ccorr;
			}
		}

		float dN = 1 - ccorr_max_n;
		float dP = 1 - ccorr_max_p;

		conf = dN / (dN + dP);

		if (conf < 0.65) {
			indexes[i] = -1;
			return;
		}
	}
}

void safeCall(cudaError e) {
	if (e != cudaSuccess) {
		printf("CUDA error: %s\n", cudaGetErrorString(e));
		system("pause");
	}
}

void detectionWithCuda(DetectorCascade *detectorCascade, const Mat &img, detectionData &detectCuda) {

	int numWindows = detectorCascade->numWindows;
	int numScales = detectorCascade->numScales;
	int positiveSampleSize = TLD_PATCH_SIZE * TLD_PATCH_SIZE;
	int falsePositivesSize = detectCuda.falsePositives->size();
	int truePositivesSize = detectCuda.truePositives->size();
	int numImgBytes = img.total() * img.elemSize();

	// assignment
	refInfo info;
	info.numWindows = detectorCascade->numWindows;
	info.isActive = detectorCascade->foregroundDetector->isActive();
	info.fgListSize = detectCuda.fgList->size();
	info.falsePositiveSize = falsePositivesSize;
	info.truePositiveSize = truePositivesSize;

	int *index = new int[numWindows];
	proData *data = new proData[numWindows];
	for (int i = 0; i < numWindows; ++i) {
		int *window = &detectorCascade->windows[TLD_WINDOW_SIZE * i];
		for (int j = 0; j < TLD_WINDOW_SIZE; ++j)
			data[i].window[j] = window[j];

		int *windowOffsets = &detectorCascade->windowOffsets[TLD_WINDOW_OFFSET_SIZE * i];
		for (int k = 0; k < TLD_WINDOW_OFFSET_SIZE; ++k) {
			data[i].varianceWindowOffsets[k] = windowOffsets[k];
			data[i].ensembleWindowOffsets[k] = windowOffsets[k];
		}
		for (int l = 0; l < TLD_NUM_TREES; ++l)
			data[i].featureVector[l] = 0;

		int x = data[i].window[0];
		int y = data[i].window[1];
		int w = data[i].window[2];
		int h = data[i].window[3];
		Mat subImage;
		subImage = img(Rect(x, y, w, h)).clone();
		Mat result;
		resize(subImage, result, cvSize(TLD_PATCH_SIZE, TLD_PATCH_SIZE)); //Default is bilinear
		for (int n = 0; n < positiveSampleSize; ++n)
			data[i].subImgData[n] = result.data[n];
		data[i].step = result.step;

		index[i] = i;
	}
	float *falsePositives = new float[falsePositivesSize * positiveSampleSize];
	float *truePositives = new float[truePositivesSize * positiveSampleSize];
	for (int i = 0; i < falsePositivesSize; ++i)
		for (int j = 0; j < positiveSampleSize; ++j)
			falsePositives[i * falsePositivesSize + j] = detectCuda.falsePositives->at(i).values[j];
	for (int i = 0; i < truePositivesSize; ++i)
		for (int j = 0; j < positiveSampleSize; ++j)
			truePositives[i * truePositivesSize + j] = detectCuda.truePositives->at(i).values[j];

	CvRect *fgList = new CvRect[detectCuda.fgList->size()];
	for (size_t i = 0; i < detectCuda.fgList->size(); ++i) {
		fgList[i].x = detectCuda.fgList->at(i).x;
		fgList[i].y = detectCuda.fgList->at(i).y;
		fgList[i].width = detectCuda.fgList->at(i).width;
		fgList[i].height = detectCuda.fgList->at(i).height;
	}

	// declare device variables
	refInfo *dev_info = NULL;
	proData *dev_data = NULL;
	float *dev_falsePositives = NULL;
	float *dev_truePositives = NULL;
	int *dev_ii = NULL;
	long long *dev_ii_squared = NULL;
	int *dev_featureOffsets = NULL;
	float *dev_ensemblePosteriors = NULL;
	CvRect *dev_fgList = NULL;
	unsigned char *dev_imgData = NULL;
	float *dev_posteriors = NULL;
	float *dev_variances = NULL;
	int *dev_indexes = NULL;

	// allocate GPU memory for input data
	safeCall(cudaMalloc((void**) &dev_info, sizeof(refInfo)));
	safeCall(cudaMalloc((void**) &dev_data, sizeof(proData)));
	safeCall(cudaMalloc((void**) &dev_falsePositives, falsePositivesSize * positiveSampleSize * sizeof(float)));
	safeCall(cudaMalloc((void**) &dev_truePositives, truePositivesSize * positiveSampleSize * sizeof(float)));
	safeCall(cudaMalloc((void**) &dev_ii, img.size().width * img.size().height * sizeof(int)));
	safeCall(cudaMalloc((void**) &dev_ii_squared, img.size().width * img.size().height * sizeof(long long)));
	safeCall(cudaMalloc((void**) &dev_featureOffsets, numScales * TLD_NUM_TREES * TLD_NUM_FEATURES * 2 * sizeof(int)));
	safeCall(cudaMalloc((void**) &dev_ensemblePosteriors, TLD_NUM_TREES * TLD_NUM_INDICES * sizeof(float)));
	safeCall(cudaMalloc((void**) &dev_fgList, detectCuda.fgList->size() * sizeof(CvRect)));
	safeCall(cudaMalloc((void**) &dev_imgData, numImgBytes));

	// copy input data to GPU
	safeCall(cudaMemcpy(dev_info, &info, sizeof(refInfo), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_data, data, numWindows * sizeof(proData), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_falsePositives, falsePositives, falsePositivesSize * positiveSampleSize * sizeof(float), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_truePositives, truePositives, truePositivesSize * positiveSampleSize * sizeof(float), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_ii, detectCuda.ii, img.size().width * img.size().height * sizeof(int), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_ii_squared, detectCuda.ii_squared, img.size().width * img.size().height * sizeof(long long), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_featureOffsets, detectCuda.featureOffsets, numScales * TLD_NUM_TREES * TLD_NUM_FEATURES * 2 * sizeof(int), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_ensemblePosteriors, detectCuda.ensemblePosteriors, TLD_NUM_TREES * TLD_NUM_INDICES * sizeof(float), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_fgList, fgList, detectCuda.fgList->size() * sizeof(CvRect), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_imgData, (const unsigned char *)img.data, numImgBytes, cudaMemcpyHostToDevice));

	// allocate GPU memory for output data
	safeCall(cudaMalloc((void**) &dev_posteriors, numWindows * sizeof(float)));
	safeCall(cudaMalloc((void**) &dev_variances, numWindows * sizeof(float)));
	safeCall(cudaMalloc((void**) &dev_indexes, numWindows * sizeof(int)));
	safeCall(cudaMemcpy(dev_indexes, index, numWindows * sizeof(int), cudaMemcpyHostToDevice));

	// invoke cuda parallel computing
	detection<<<1024, 1024>>>(dev_info, dev_data, dev_falsePositives, dev_truePositives,
			dev_ii, dev_ii_squared, dev_featureOffsets, dev_ensemblePosteriors,  dev_fgList, dev_imgData, dev_posteriors,
			dev_variances, dev_indexes);

	float *posteriors = new float[numWindows];
	safeCall(cudaMemcpy(posteriors, dev_posteriors, numWindows * sizeof(float), cudaMemcpyDeviceToHost));
	float *variances = new float[numWindows];
	safeCall(cudaMemcpy(variances, dev_variances, numWindows * sizeof(float), cudaMemcpyDeviceToHost));

	safeCall(cudaMemcpy(index, dev_indexes, numWindows * sizeof(int), cudaMemcpyDeviceToHost));

	for (int i = 0; i < numWindows; ++i) {
		detectCuda.posteriors[i] = posteriors[i];
		detectCuda.variances[i] = variances[i];
		if (index[i] >= 0)
			detectCuda.confidentIndices->push_back(index[i]);
	}

	safeCall(cudaFree(dev_info));
	safeCall(cudaFree(dev_data));
	safeCall(cudaFree(dev_falsePositives));
	safeCall(cudaFree(dev_truePositives));
	safeCall(cudaFree(dev_ii));
	safeCall(cudaFree(dev_ii_squared));
	safeCall(cudaFree(dev_featureOffsets));
	safeCall(cudaFree(dev_ensemblePosteriors));
	safeCall(cudaFree(dev_fgList));
	safeCall(cudaFree(dev_imgData));
	safeCall(cudaFree(dev_posteriors));
	safeCall(cudaFree(dev_variances));
	safeCall(cudaFree(dev_indexes));

	delete posteriors;
	delete variances;
	delete fgList;
	delete truePositives;
	delete falsePositives;
	delete data;
	delete index;

	return;
}

/*struct result{
	int fern[10];
	int res;
	float conf;
};

struct proData{
	int size, nstructs, structSize, featureSizeSize;
	float var, thr_fern;
};

struct boundingbox{
	int x;
	int y;
	int width; 
	int height;
	int sidx;
};
struct feature{
	uchar x1, y1, x2, y2;
};

__global__ void kernel(float* posteriors, feature* featureData, proData * dt, boundingbox* bbdata, const PtrStepSz<uchar1> img, const PtrStepSz<float1> iisum, PtrStep<float1> iisqsum, result *result)
{
	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if (i < (*dt).size){
		result[i].res = 0;
		int fern[10];
		feature featureTmp;
		float brs = iisum(bbdata[i].y + bbdata[i].height, bbdata[i].x + bbdata[i].width).x;
		float bls = iisum(bbdata[i].y + bbdata[i].height, bbdata[i].x).x;
		float trs = iisum(bbdata[i].y, bbdata[i].x + bbdata[i].width).x;
		float tls = iisum(bbdata[i].y, bbdata[i].x).x;
		float brsq = iisqsum(bbdata[i].y + bbdata[i].height, bbdata[i].x + bbdata[i].width).x;
		float blsq = iisqsum(bbdata[i].y + bbdata[i].height, bbdata[i].x).x;
		float trsq = iisqsum(bbdata[i].y, bbdata[i].x + bbdata[i].width).x;
		float tlsq = iisqsum(bbdata[i].y, bbdata[i].x).x;
		float mean = (brs + tls - trs - bls) / ((float)bbdata[i].height*(float)bbdata[i].width);
		float sqmean = (brsq + tlsq - trsq - blsq) / ((float)bbdata[i].height*(float)bbdata[i].width);
		if (sqmean - mean*mean >= (*dt).var){
			int leaf;
			for (int t = 0; t<(*dt).nstructs; t++){
				leaf = 0;
				for (int f = 0; f<(*dt).structSize; f++){
					featureTmp = featureData[bbdata[i].sidx*(*dt).featureSizeSize + t*(*dt).structSize + f];
					leaf = (leaf << 1) + (img(bbdata[i].y + featureTmp.y1, bbdata[i].x + featureTmp.x1).x > img(bbdata[i].y+featureTmp.y2, bbdata[i].x+featureTmp.x2).x);
				}
				fern[t] = leaf;
			}
			float votes = 0;
			for (int t = 0; t <(*dt).nstructs; t++) {
				votes += posteriors[t*(int)pow(2.0f, (float)(*dt).structSize)+ fern[t]];
			}
			result[i].conf = votes;
			for (int t = 0; t<10; t++){
				result[i].fern[t] = fern[t];
			}
			if (votes>(*dt).nstructs*(*dt).thr_fern){
				result[i].res = 1;
			}
		}
		else{
			result[i].conf = 0.0;
		}
	}
}

void safeCall(cudaError e){
	if (e != cudaSuccess){
		printf("CUDA error: %s\n", cudaGetErrorString(e));
		system("pause");
	}
}

void processWithCuda(TLD *tld, const Mat &img, const FerNNClassifier &classifier){
	GpuMat gpuiisum, gpuiisqsum, gpuimg;
	gpuiisum.upload(tld->iisum);
	gpuiisum.convertTo(gpuiisum, CV_32F);
	gpuiisqsum.upload(tld->iisqsum);
	gpuiisqsum.convertTo(gpuiisqsum, CV_32F);
	gpuimg.upload(img);
	vector<BoundingBox> grid = tld->grid;
	proData dt;
	dt.size = grid.size();
	dt.var = tld->var;
	dt.nstructs = classifier.nstructs;
	dt.structSize = classifier.structSize;
	dt.thr_fern = classifier.thr_fern;
	boundingbox * data = new boundingbox[dt.size];
	for (int i = 0; i < dt.size; i++){
		data[i].x = grid[i].x;
		data[i].y = grid[i].y;
		data[i].width = grid[i].width;
		data[i].height = grid[i].height;
		data[i].sidx = grid[i].sidx;
	}
	int featureSize = classifier.features.size();
	int featureSizeSize = classifier.features[0].size();
	dt.featureSizeSize = featureSizeSize;
	feature * featureData = new feature[featureSize*featureSizeSize];
	for (int i = 0; i < featureSize; i++){
		for (int j = 0; j < featureSizeSize; j++){
			featureData[i*featureSizeSize + j].x1 = classifier.features[i][j].x1;
			featureData[i*featureSizeSize + j].y1 = classifier.features[i][j].y1;
			featureData[i*featureSizeSize + j].x2 = classifier.features[i][j].x2;
			featureData[i*featureSizeSize + j].y2 = classifier.features[i][j].y2;
		}
	}
	boundingbox *dev_bbdata = NULL;
	feature *dev_featureData = NULL;
	proData *dev_dt = NULL;
	float * dev_posteriors = NULL;
	int posteriorsSize = classifier.posteriors.size();
	int posteriorsSizeSize = classifier.posteriors[0].size();
	float *posteriors = new float[posteriorsSize*posteriorsSizeSize];
	for (int i = 0; i < posteriorsSize; i++){
		for (int j = 0; j < posteriorsSizeSize; j++){
			posteriors[i*posteriorsSizeSize + j] = classifier.posteriors[i][j];
		}
	}
	safeCall(cudaMalloc((void**)&dev_posteriors, posteriorsSize*posteriorsSizeSize*sizeof(float)));
	safeCall(cudaMalloc((void**)&dev_featureData, featureSize*featureSizeSize*sizeof(feature)));
	safeCall(cudaMalloc((void**)&dev_bbdata, dt.size*sizeof(boundingbox)));
	safeCall(cudaMalloc((void**)&dev_dt, sizeof(proData)));
	safeCall(cudaMemcpy(dev_posteriors, posteriors, posteriorsSize*posteriorsSizeSize*sizeof(float), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_dt, &dt, sizeof(proData), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_bbdata, data, dt.size*sizeof(boundingbox), cudaMemcpyHostToDevice));
	safeCall(cudaMemcpy(dev_featureData, featureData, featureSize*featureSizeSize*sizeof(feature), cudaMemcpyHostToDevice));

	result *dev_result = NULL;
	result *res = new result[dt.size];
	safeCall(cudaMalloc((void**)&dev_result, dt.size*sizeof(result)));
	kernel << <1024, 1024 >> >(dev_posteriors, dev_featureData, dev_dt, dev_bbdata, gpuimg, gpuiisum, gpuiisqsum, dev_result);
	safeCall(cudaMemcpy(res, dev_result, dt.size * sizeof(result), cudaMemcpyDeviceToHost));

	int output = 0;
	for (int i = 0; i < dt.size; i++){
		if (res[i].res)
			output++;
	}
	printf("%d %d\n", dt.size, output);

	for (int i = 0; i < dt.size; i++){
		tld->tmp.conf[i] = res[i].conf;
		for (int j = 0; j < 10; j++){
			tld->tmp.patt[i][j] = res[i].fern[j];
		}
		if (res[i].res){
			tld->dt.bb.push_back(i);
		}
	}

	safeCall(cudaFree(dev_posteriors));
	safeCall(cudaFree(dev_bbdata));
	safeCall(cudaFree(dev_dt));
	safeCall(cudaFree(dev_featureData));
	safeCall(cudaFree(dev_result));
	
	delete data;
	delete featureData;
	delete res;

	return;
}*/
