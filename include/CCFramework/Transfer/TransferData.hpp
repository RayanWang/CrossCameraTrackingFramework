/*
 * TransferData.hpp
 *
 *  Created on: 2016年1月11日
 *      Author: rayan
 */

#ifndef TRANSFERDATA_HPP_
#define TRANSFERDATA_HPP_


namespace ccFramework {
namespace transfer {

#define CrossCameraCenter "CrossCameraCenter"
#define safeDelete(p) if(p) { delete p; p = NULL; }
#define safeArrayDelete(p) if(p) { delete[] p; p = NULL; }

struct TK_CrossCamera {
	uint32_t totCameraNum;
	uint32_t maxNumOfTarget;
};

enum error_code_transfer {
	unknown = 0x100,
	not_found_tkcrosscamera,
	buffer_full_error,
	no_data_error,
	no_enough_capacity,
	invalid_arg,
	get_shm_size_error,

	boost_shm_error,			// boost shared memory error, please further confirm the boost error code

	other_error = 0xffff
};

}	// namespace transfer
}	// namespace ccFramework


#endif /* TRANSFERDATA_HPP_ */
