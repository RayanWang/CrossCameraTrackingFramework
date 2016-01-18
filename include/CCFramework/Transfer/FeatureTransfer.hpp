/*
 * FeatureTransfer.hpp
 *
 *  Created on: 2016年1月8日
 *      Author: rayan
 */

#ifndef FEATURETRANSFER_HPP_
#define FEATURETRANSFER_HPP_

#include <sstream>
#include <pthread.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>

#include <CCFramework/Transfer/TransferData.hpp>

using namespace std;
using namespace boost::interprocess;
using namespace boost::uuids;

namespace ccFramework {
namespace transfer {

typedef void (*recvFeature)(int32_t, uuid, void*, uint32_t, void*, uint32_t, void*);

/*
 * To provide the transfer mode for AnalyzerTK, for one single current camera
 */
class FeatureTransfer {
public:
	FeatureTransfer();
	FeatureTransfer(const int32_t cameraId) :
			m_TotCameraNum(0),
			m_MaxNumOfTarget(0),
			m_descSizeInByte(0),
			m_featureSizeInByte(0),
			m_checkListSize(0),
			m_cameraId(cameraId),
			m_transferErrorCode(unknown),
			m_boostErrorCode(no_error),
			m_recvThread(0),
			m_pShmCamera(NULL),
			m_cbRecv(NULL),
			m_userData(NULL) {
	}
	~FeatureTransfer();

	bool initTransfer();
	bool sendFeatureToAll(int32_t fromCamId, uuid& objId, void* feature,
			uint32_t featureSizeInByte, void* pDesc = NULL, uint32_t descSizeInByte = 0);
	bool startRecvFeatures();
	void setRecvCallback(recvFeature callback, void* userData) {
		m_cbRecv = callback;
		m_userData = userData;
	}
	static void* recvFeatures(void* lpParam);
	void stopRecvFeatures();

	error_code_transfer getTransferError() const { return m_transferErrorCode; }
	error_code_t getboostError() const { return m_boostErrorCode; }

private:
#pragma pack(push)
#pragma pack(4)
	struct FeatureHeader {
		FeatureHeader& operator=(const FeatureHeader& header) {
			fromCameraId = header.fromCameraId;
			memcpy(&objId, &header.objId, sizeof(uuid));
			return *this;
		}

		int32_t fromCameraId;
		uuid objId;
	};
#pragma pack(pop)

	struct FeatureData {
		FeatureData() :
				sem_feature(1),
				checkList(NULL) {
		}
		~FeatureData() {
		}

		interprocess_semaphore sem_feature;
		bool* checkList;
	};

	struct DataQueue {
		DataQueue() :
				flag(0),
				capacity(0),
				front(-1),
				rear(-1) {
		}
		~DataQueue() {
		}

		void init(int32_t queueSize, char* queueStartAddr) {
			capacity = queueSize;
			buf = new (queueStartAddr) FeatureHeader[capacity];
		}
		bool enqueue(FeatureHeader& header) {
			if (capacity <= 0)
				return false;

			if ((isFull() && flag == 1) || (rear == capacity - 1 && front == -1)) {
				cout << "Buffer is full" << endl;
				return false;
			}
			rear = (rear + 1) % capacity;
			buf[rear] = header;
			if (front == rear)
				flag = 1;

			return true;
		}
		bool dequeue(FeatureHeader* header) {
			if (!header)
				return false;

			if (isEmpty() && flag == 0) {
				cout << "No data in the buffer" << endl;
				return false;
			}
			front = (front + 1) % capacity;
			*header = buf[front];
			if (front == rear)
				flag = 0;

			return true;
		}
		int32_t size() {
			return (capacity);
		}
		bool isFull() {
			if (capacity)
				return (rear % capacity == front);
			return false;
		}
		bool isEmpty() {
			return front == rear;
		}

		int32_t flag;
		int32_t capacity;
		int32_t front;
		int32_t rear;
		FeatureHeader* buf;
	};

#pragma pack(push)
#pragma pack(4)
	struct shm_Camera {
		shm_Camera(int32_t capacity, char* queueStartAddr) :
				sem_camera(1) {
			dataQ.init(capacity, queueStartAddr);
		}
		~shm_Camera() {
		}

		//Semaphores to protect and synchronize access
		interprocess_semaphore sem_camera;

		DataQueue dataQ;
	};
#pragma pack(pop)

	struct recvPackage {
		shm_Camera* feature;
		FeatureTransfer* transfer;
	};

	uint32_t m_TotCameraNum;
	uint32_t m_MaxNumOfTarget;
	uint32_t m_descSizeInByte;
	uint32_t m_featureSizeInByte;
	uint32_t m_checkListSize;
	int32_t m_cameraId;
	error_code_transfer m_transferErrorCode;
	error_code_t m_boostErrorCode;
	pthread_t m_recvThread;
	shm_Camera* m_pShmCamera;
	recvFeature m_cbRecv;
	void* m_userData;

	template<class Q>
	static string ConvertToString(Q value) {
		stringstream ss;
		ss << value;
		return ss.str();
	}
};

inline FeatureTransfer::FeatureTransfer() :
		m_TotCameraNum(0),
		m_MaxNumOfTarget(0),
		m_descSizeInByte(0),
		m_featureSizeInByte(0),
		m_checkListSize(0),
		m_cameraId(1 >> 31),
		m_transferErrorCode(unknown),
		m_boostErrorCode(no_error),
		m_recvThread(0),
		m_pShmCamera(NULL),
		m_cbRecv(NULL),
		m_userData(NULL) {
}

inline FeatureTransfer::~FeatureTransfer() {
	shared_memory_object::remove((const char*) ConvertToString(m_cameraId).c_str());
}

inline bool FeatureTransfer::initTransfer() {
	try {
		shared_memory_object shmCrossCamera(open_only, CrossCameraCenter, read_write);
		mapped_region regionCrossCamera(shmCrossCamera, read_write);
		TK_CrossCamera* data = static_cast<TK_CrossCamera*>(regionCrossCamera.get_address());

		m_TotCameraNum = data->totCameraNum;
		m_MaxNumOfTarget = data->maxNumOfTarget;

		// create the shared memory of current camera
		shared_memory_object shmCamera(create_only,
				(const char*) ConvertToString(m_cameraId).c_str(), read_write);
		shmCamera.truncate(sizeof(shm_Camera) +
					sizeof(FeatureHeader) * (m_TotCameraNum - 1) * m_MaxNumOfTarget);
		mapped_region regionCamera(shmCamera, read_write);
		char* currCameraAddr = static_cast<char*>(regionCamera.get_address());

		int32_t capacity = (m_TotCameraNum - 1) * m_MaxNumOfTarget;
		char* queueStartAddr = currCameraAddr + sizeof(shm_Camera);
		m_pShmCamera = new (currCameraAddr) shm_Camera(capacity, queueStartAddr);

		return true;
	} catch (interprocess_exception& e) {
		if (e.get_error_code() == not_found_error)
			m_transferErrorCode = not_found_tkcrosscamera;
		else {
			m_transferErrorCode = boost_shm_error;
			m_boostErrorCode = e.get_error_code();
		}
	}

	return false;
}

inline bool FeatureTransfer::sendFeatureToAll(int32_t fromCamId, uuid& objId,
		 void* feature, uint32_t featureSizeInByte, void* pDesc, uint32_t descSizeInByte) {
	if (!feature) {
		m_transferErrorCode = invalid_arg;
		return false;
	}
	m_descSizeInByte = descSizeInByte;
	m_featureSizeInByte = featureSizeInByte;
	m_checkListSize = m_TotCameraNum - 1;
	try {
		//---------- create an uniqle shared memory for the feature data ----------
		string strCamId = ConvertToString(fromCamId);
		const string strObjId = boost::lexical_cast<string>(objId);
		string strFeatureShm = strCamId + "_" + strObjId;

		shared_memory_object shmFeature(create_only,
				(const char*) strFeatureShm.c_str(), read_write);
		shmFeature.truncate(sizeof(FeatureData) + m_checkListSize * sizeof(bool) +
				m_descSizeInByte + m_featureSizeInByte);
		mapped_region regionFeature(shmFeature, read_write);
		char* dataAddr = static_cast<char*>(regionFeature.get_address());

		FeatureData* data = new (dataAddr) FeatureData;
		char* checkListAddr = dataAddr + sizeof(FeatureData);
		data->checkList = new (checkListAddr) bool[m_checkListSize];
		char* descAddr = checkListAddr + m_checkListSize * sizeof(bool);
		char* featureAddr = descAddr + m_descSizeInByte;

		data->sem_feature.wait();
		memset(data->checkList, false, m_checkListSize);
		if (pDesc && m_descSizeInByte > 0)
			memcpy(descAddr, pDesc, m_descSizeInByte);
		if (feature && m_featureSizeInByte > 0)
			memcpy(featureAddr, feature, m_featureSizeInByte);
		data->sem_feature.post();
		//---------------------------------------------------------------------------------------------

		for (int32_t i = 0; i < m_TotCameraNum; ++i) {
			if (i == m_cameraId)
				continue;

			shared_memory_object shmCamera(open_only,
					(const char*) ConvertToString(i).c_str(), read_write);
			mapped_region regionCamera(shmCamera, read_write);
			shm_Camera* feature = static_cast<shm_Camera*>(regionCamera.get_address());

			feature->sem_camera.wait();
			FeatureHeader header;
			memset(&header, 0, sizeof(header));
			header.fromCameraId = fromCamId;
			memcpy(&header.objId, &objId, sizeof(uuid));
			if (!feature->dataQ.enqueue(header)) {
				cout << "Failed to send data to camera " << i << endl;
				m_transferErrorCode = buffer_full_error;
				feature->sem_camera.post();
				return false;
			}
			feature->sem_camera.post();
		}

		return true;
	} catch (interprocess_exception& e) {
		m_transferErrorCode = boost_shm_error;
		m_boostErrorCode = e.get_error_code();
	}

	return false;
}

inline bool FeatureTransfer::startRecvFeatures() {
	try {
		shared_memory_object shmCamera(open_only,
				(const char*) ConvertToString(m_cameraId).c_str(), read_write);
		mapped_region regionCamera(shmCamera, read_write);
		shm_Camera* feature = static_cast<shm_Camera*>(regionCamera.get_address());

		recvPackage pack;
		pack.feature = feature;
		pack.transfer = this;

		pthread_create(&m_recvThread, NULL, recvFeatures, &pack);

		return true;
	} catch (interprocess_exception& e) {
		m_transferErrorCode = boost_shm_error;
		m_boostErrorCode = e.get_error_code();
	}

	return false;
}

inline void* FeatureTransfer::recvFeatures(void* lpParam) {
	recvPackage* pack = (recvPackage*) lpParam;
	if (pack && pack->feature) {
		while (true) {
			pthread_testcancel();

			pack->feature->sem_camera.wait();
			FeatureHeader header;
			memset(&header, 0, sizeof(header));
			bool bRet = pack->feature->dataQ.dequeue(&header);
			pack->feature->sem_camera.post();

			if (bRet) {
				string strCamId = ConvertToString(header.fromCameraId);
				const string strObjId = boost::lexical_cast<string>(header.objId);
				string strFeatureShm = strCamId + "_" + strObjId;

				try {
					shared_memory_object shmFeature(open_only,
							(const char*) strFeatureShm.c_str(), read_write);
					mapped_region regionFeature(shmFeature, read_write);

					char* recvDesc = static_cast<char*>(regionFeature.get_address()) +
							sizeof(FeatureData) + pack->transfer->m_checkListSize * sizeof(bool);
					char* recvFeature = recvDesc + pack->transfer->m_descSizeInByte;

					pack->transfer->m_cbRecv(header.fromCameraId, header.objId,
							recvFeature, pack->transfer->m_featureSizeInByte,
							recvDesc, pack->transfer->m_descSizeInByte,
							pack->transfer->m_userData);

					pthread_testcancel();
					FeatureData* data = static_cast<FeatureData*>(regionFeature.get_address());

					data->sem_feature.wait();
					data->checkList[pack->transfer->m_cameraId] = true;
					bool bRemoveShm = true;
					for (int32_t i = 0; i < pack->transfer->m_checkListSize; ++i)
						if (data->checkList[i] == false) {
							bRemoveShm = false;
							break;
						}
					data->sem_feature.post();

					if (bRemoveShm)
						shared_memory_object::remove((const char*) strFeatureShm.c_str());
				} catch (interprocess_exception& e) {
					pack->transfer->m_transferErrorCode = boost_shm_error;
					pack->transfer->m_boostErrorCode = e.get_error_code();
				}
			} else
				pack->transfer->m_transferErrorCode = no_data_error;

			usleep(100000);
		}
	}

	pthread_exit(0);
}

inline void FeatureTransfer::stopRecvFeatures() {
	pthread_cancel(m_recvThread);

	pthread_join(m_recvThread, NULL);
}

} // namespace transfer
} // namespace ccFramework

#endif /* FEATURETRANSFER_HPP_ */
