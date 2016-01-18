/*
 * ClassFactory.h
 *
 *      Author: rayan
 */

#ifndef CLASSFACTORY_H_
#define CLASSFACTORY_H_

#include <string>
#include <map>

typedef void* (*CreateClass)();

class ClassFactory {
public:
	ClassFactory() {};

	static void* GetClassByName(std::string name) {
		std::map<std::string, CreateClass>::const_iterator iter;
		iter = m_classMap.find(name);
		if(iter == m_classMap.end())
			return NULL;
		else
			return iter->second();	// invoke CreateInstance
	}

	static void RegistClass(std::string name, CreateClass method) {
		m_classMap.insert(std::pair<std::string, CreateClass>(name, method));
	}

private:
	static std::map<std::string, CreateClass> m_classMap;
};

#endif /* CLASSFACTORY_H_ */
