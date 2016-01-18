/*
 * CallbackProxy.hpp
 *
 *  Created on: 2016年1月10日
 *      Author: rayan
 */

#ifndef CALLBACKPROXY_HPP_
#define CALLBACKPROXY_HPP_


#include <boost/uuid/uuid.hpp>

using namespace boost::uuids;

namespace ccFramework {
namespace transfer {

template <typename Tobject>
class CallbackProxy {
	typedef void (Tobject::*CbFun)(void*, uint32_t, int32_t, int32_t, int32_t);

public:
	void set(Tobject* pInstance, CbFun pFun);

	bool exec(int32_t fromCamId, uuid& objId, uint32_t featureSizeInByte, void* feature);

private:
	CbFun m_pCbFun;
	Tobject* m_pInstance;
};

template <typename Tobject>
inline void CallbackProxy<Tobject>::set(Tobject* pInstance, CbFun pFun) {
	m_pInstance = pInstance;
	m_pCbFun = pFun;
}

template <typename Tobject>
inline bool CallbackProxy<Tobject>::exec(
		void* feature,
		uint32_t featureSizeInByte,
		int32_t splitIndex,
		int32_t totFeatureSize,
		int32_t eachElementSize) {
	(m_pInstance->*m_pCbFun)(feature, featureSizeInByte, splitIndex, totFeatureSize, eachElementSize);

	return true;
}

}	// namespace transfer
}	// namespace ccFramework


#endif /* CALLBACKPROXY_HPP_ */
