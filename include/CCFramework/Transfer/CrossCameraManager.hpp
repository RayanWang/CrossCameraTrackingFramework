/*
 * CrossCameraManager.hpp
 *
 *  Created on: 2016年1月11日
 *      Author: rayan
 */

#ifndef CROSSCAMERAMANAGER_HPP_
#define CROSSCAMERAMANAGER_HPP_


#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <CCFramework/Transfer/TransferData.hpp>

using namespace boost::interprocess;

namespace ccFramework {
namespace transfer {

class CrossCameraManager {
public:
	CrossCameraManager();
	~CrossCameraManager();

	bool initManagerData(int32_t totCameraNum, int32_t maxNumOfTarget);

	error_code_transfer getTransferError() const { return m_transferErrorCode; }
	error_code_t getboostError() const { return m_boostErrorCode; }

private:
	error_code_transfer m_transferErrorCode;
	error_code_t m_boostErrorCode;
};

inline CrossCameraManager::CrossCameraManager() :
		m_transferErrorCode(unknown),
		m_boostErrorCode(no_error) {
}

inline CrossCameraManager::~CrossCameraManager() {
	shared_memory_object::remove(CrossCameraCenter);
}

inline bool CrossCameraManager::initManagerData(int32_t totCameraNum, int32_t maxNumOfTarget) {
	try {
		shared_memory_object shmCrossCamera(create_only, CrossCameraCenter, read_write);
		shmCrossCamera.truncate(sizeof(TK_CrossCamera));
		mapped_region regionCrossCamera(shmCrossCamera, read_write);
		void* sharedmemAddr = regionCrossCamera.get_address();
		TK_CrossCamera* data = new (sharedmemAddr) TK_CrossCamera;
		data->totCameraNum = totCameraNum;
		data->maxNumOfTarget = maxNumOfTarget;

		return true;
	} catch (interprocess_exception& e) {
		m_transferErrorCode = boost_shm_error;
		m_boostErrorCode = e.get_error_code();
	}

	return false;
}

}
}


#endif /* CROSSCAMERAMANAGER_HPP_ */
