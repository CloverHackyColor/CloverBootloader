//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#include <iostream>
#include <locale.h>

#include "../../../PosixCompilation/xcode_utf_fixed.h"

#include "../../../rEFIt_UEFI/cpp_unit_test/all_tests.h"
//#include "../../../rEFIt_UEFI/cpp_foundation/XToolsCommon.h"
#include "../../../rEFIt_UEFI/libeg/XImage.h"
//#include "../../../rEFIt_UEFI/Platform/platformdata.h"

//class Boolean
//{
//    bool flag;
//public:
//    explicit Boolean() : flag(false) {}
//    explicit Boolean(const bool other) { flag = other; }
//    explicit Boolean(const Boolean& other) { flag = other.flag; }
////    template <typename T>
////    Boolean(T other) = delete;// { return *this; }
//
//    Boolean& operator= (const Boolean& other) { return *this; }
//    Boolean& operator= (const bool other) { return *this; }
//    template <typename T>
//    Boolean& operator= (const T other) = delete;// { return *this; }
//
//    bool getValue() const {return flag;}
//    void setValue(bool a) {flag = a;}
//};

// The following is by no means a FULL solution!
#include <functional>
#include <iostream>
#include <cassert>

template<typename T>
class Property {
public:
    Property(){}
    operator const T& () const {
        // Call override getter if we have it
        if (getter) return getter();
        return get();
    }
    const T& operator = (const T& other) {
        // Call override setter if we have it
        if (setter) return setter(other);
        return set(other);
    }
    bool operator == (const T& other) const {
        // Static cast makes sure our getter operator is called, so we could use overrides if those are in place
        return static_cast<const T&>(*this) == other;
    }
    // Use this to always get without overrides, useful for use with overriding implementations
    const T& get() const {
        return t;
    }
    // Use this to always set without overrides, useful for use with overriding implementations
    const T& set(const T& other) {
        return t = other;
    }
    // Assign getter and setter to these properties
    std::function<const T&()> getter;
    std::function<const T&(const T&)> setter;
private:
    T t;
};

// Basic usage, no override
struct Test {
    Property<int> prop;
};

// Override getter and setter
struct TestWithOverride {
    TestWithOverride(){
        prop.setter = [&](const int& other){
            std::cout << "Custom setter called" << std::endl;
            return prop.set(other);
        };
        prop.setter = std::bind(&TestWithOverride::setProp,this,std::placeholders::_1);
        prop.getter = std::bind(&TestWithOverride::getProp,this);
    }
    Property<int> prop;
private:
    const int& getProp() const {
        std::cout << "Custom getter called" << std::endl;
        return prop.get();
    }
    const int& setProp(const int& other){
        std::cout << "Custom setter called" << std::endl;
        return prop.set(other);
    }
};

class MyFloat {
public:
  float f;
  MyFloat() { f = 0.0f; }
  MyFloat(float _f) : f(_f) {}
  float get() { return 1; }
};
template<typename T>
class MutableRef : public T {
public:
  const T* t;
  const T& operator = (const T* other) {
    t = other;
    return *t;
  }
  operator T& () {
    return *t;
  }
};


extern "C" int main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

  XString8 s("foo");
//  s.strcat("a");
  char* p = s.forgetDataWithoutFreeing();
  free(p);

  XString8 s2;
//  s2.S8Printf("bar");
  char* q = s2.forgetDataWithoutFreeing();
  free(q);

  MyFloat test = 5.0f;
  
  MutableRef<MyFloat> Background;
  
  Background = &test;
  test = 6;
  float test2 = Background.get(); (void)test2;

    Test t;
    TestWithOverride t1;
    t.prop = 1;
    assert(t.prop == 1);
    t1.prop = 1;
    assert(t1.prop == 1);
    /*
    Expected output:
    1. No aborts on assertions
    2. Text:
    Custom setter called
    Custom getter called
    */









//  xcode_utf_fixed_tests();
  const int i = 2;
  (void)i;
  XBool b;
  b = true;
  b = false;
//  b = XBool(i);
//  b = (char*)NULL;
//  b = (float)1.0;
//  b = i;
  //printf("%d", numeric_limits<int>::min());
  //printf("%d", numeric_limits<int>::min());




	return all_tests() ? 0 : -1 ;
}
