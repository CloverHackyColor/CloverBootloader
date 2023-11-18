/*
 * apd.h
 *
 *  Created on: Nov 11, 2023
 *      Author: jief
 */

#ifndef CPP_FOUNDATION_APD_H_
#define CPP_FOUNDATION_APD_H_


template<typename T>
class apd
{
protected:
  T t = 0;
public:
  using type = T;
  
  apd() : t(NULL) {};
  apd(T _t) : t(_t) {};
  apd(const apd<T>& other) {
    panic("You must NOT copy an apd object. Use shared_ptr instead\n");
  }
  apd& operator=(const apd<T>& other) {
    panic("You must NOT copy an apd object. Use shared_ptr instead\n");
    return *this;
  }

  apd& operator=(T _t) {
    delete t;
    t = _t;
    return *this;
  }

  constexpr bool isNull() const { return t == NULL; }
  constexpr bool notNull() const { return !isNull(); }
  
  remove_ptr(T)& operator * () {
    return *t;
  }
  T operator -> () {
    return t;
  }
  T* operator & ()
  {
    return &t;
  }
  
  T& get() {
    return t;
  }
  const T& get() const {
    return t;
  }

  operator T () {
    return t;
  }
  operator const T () const {
    return t;
  }

  	/* [] */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	remove_ptr(T)&  operator [](IntegralType i) const { return t[i]; }

  
  ~apd() {
    delete t;
  }
};

#endif /* CPP_FOUNDATION_APD_H_ */
