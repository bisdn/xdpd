/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SAFEVECTOR_H
#define SAFEVECTOR_H 1

#include <pthread.h>
#include <vector>

/*
* Very simple Wrap of vector so that it can be thread-safely managed 
*/

namespace xdpd {
namespace gnu_linux {

template <typename T>
class safevector{ 

protected:
	//Internal vector
	std::vector<T> vec;	

	//RWlock
	pthread_rwlock_t rwlock;	

	inline void write_lock(void){pthread_rwlock_wrlock(&rwlock);};
	inline void write_unlock(void){pthread_rwlock_unlock(&rwlock);};
	
public:
	
	//Override constructor&destructor
	safevector(void);

	~safevector(void);
	

	//Read functions
	inline void read_lock(void){pthread_rwlock_rdlock(&rwlock);};
	inline void read_unlock(void){pthread_rwlock_unlock(&rwlock);};
	 
	//Public interface
	inline T& operator[](unsigned int j){	if(j < size())
							return vec[j];
						else
							throw "Not found"; };

	void pop_back(void);
	void push_back(const T& val);
	
	typedef typename std::vector<T>::iterator safe_iterator;
	void erase(int j);
	void erase(safe_iterator itr);
	void erase(const T& elem);
	
	void clear(void);
	bool contains(const T& elem);

	inline size_t size(void){return vec.size();};
	inline bool empty(){return vec.empty();};
	inline safe_iterator begin(void){ return vec.begin();};
	inline safe_iterator end(void){ return vec.end();};


};

template <typename T>
safevector<T>::safevector(){
	
	//Init mutex
	pthread_rwlock_init(&rwlock,NULL);
}

template <typename T>
safevector<T>::~safevector(){
	
	//Destroy mutex
	pthread_rwlock_destroy(&rwlock);
}

/* Wrapping (few) methods */
template <typename T>
void safevector<T>::pop_back(){
	write_lock();
	try{
		vec.pop_back();
	}catch(...){
		write_unlock();
		throw;			
	}
	write_unlock();
}

template <typename T>
void safevector<T>::push_back(const T& val){
	write_lock();
	try{
		vec.push_back(val);
	}catch(...){
		write_unlock();
		throw;			
	}
	write_unlock();
}

template <typename T>
void safevector<T>::erase(safe_iterator itr){
	write_lock();
	try{
		vec.erase(itr);
	}catch(...){
		write_unlock();
		throw;			
	}
	write_unlock();
}

template <typename T>
void safevector<T>::erase(int j){
	write_lock();
	try{
		vec.erase(vec.begin()+j);
	}catch(...){
		write_unlock();
		throw;			
	}
	write_unlock();
}

template <typename T>
void safevector<T>::erase(const T& elem){
	for(unsigned int i=0; i < size(); ++i)
		if( vec[i] == elem )
			erase(i);
}

template <typename T>
void safevector<T>::clear(){

	write_lock();
	try{
		vec.clear();
	}catch(...){
		write_unlock();
		throw;			
	}
	write_unlock();

}



template <typename T>
bool safevector<T>::contains(const T& elem){

	read_lock();
	for(unsigned int i=0;i<vec.size();++i){
		if( vec[i] == elem ){
			read_unlock();
			return true;
		}
	}
	read_unlock();
	return false;
	
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* SAFEVECTOR_H_ */
