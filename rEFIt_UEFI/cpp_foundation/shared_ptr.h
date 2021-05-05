/*
 * sharedptr.h
 *
 *  Created on: Oct 17, 2020
 *      Author: jief
 *  From https://www.geeksforgeeks.org/how-to-implement-user-defined-shared-pointers-in-c/
 */

#ifndef CPP_FOUNDATION_SHARED_PTR_H_
#define CPP_FOUNDATION_SHARED_PTR_H_


// Class representing a reference counter class
class Counter {
public:
  // Constructor
  Counter()
    : m_counter(0){};

  Counter(const Counter&) = delete;
  Counter& operator=(const Counter&) = delete;

  // Destructor
  ~Counter()
  {
  }

  void reset()
  {
    m_counter = 0;
  }

  unsigned int get()
  {
    return m_counter;
  }

  // Overload post/pre increment
  Counter& operator++()
  {
    m_counter++;
    return *this;
  }

//  Counter operator++(int)
//  {
//    Counter tmp = *this;
//    m_counter++;
//    return tmp;
//  }
//
//  // Overload post/pre decrement
  Counter& operator--()
  {
    m_counter--;
    return *this;
  }
//  Counter operator--(int)
//  {
//    Counter tmp = *this;
//    m_counter--;
//    return tmp;
//  }

private:
  unsigned int m_counter{};
};

// Class representing a shared pointer
template <typename T>

class Shared_ptr {
public:
  // Constructor
  explicit Shared_ptr(T* ptr = nullptr)
  {
    m_ptr = ptr;
    m_counter = new Counter;
    if (ptr) {
      ++(*m_counter);
    }
  }

  // Copy constructor
  Shared_ptr(Shared_ptr<T>& sp)
  {
    m_ptr = sp.m_ptr;
    m_counter = sp.m_counter;
    ++(*m_counter);
  }

  // Reference count
  unsigned int use_count()
  {
    return m_counter->get();
  }

  // Get the pointer
  T* get()
  {
    return m_ptr;
  }

  // Overload * operator
  T& operator*() {
    return *m_ptr;
  }

  // Overload -> operator
  T* operator->() {
    return m_ptr;
  }
  // Destructor
  ~Shared_ptr()
  {
    --(*m_counter);
    if (m_counter->get() == 0) {
      delete m_counter;
      delete m_ptr;
    }
  }

//// shared_ptr comparisons:
//template<class T, class U>
//    bool operator==(shared_ptr<T> const& a, shared_ptr<U> const& b) noexcept;
//template<class T, class U>
//    bool operator!=(shared_ptr<T> const& a, shared_ptr<U> const& b) noexcept;
//template <class T>
//    bool operator==(const shared_ptr<T>& x, nullptr_t) noexcept;
//template <class T>
//    bool operator==(nullptr_t, const shared_ptr<T>& y) noexcept;
//template <class T>
//    bool operator!=(const shared_ptr<T>& x, nullptr_t) noexcept;
//template <class T>
//    bool operator!=(nullptr_t, const shared_ptr<T>& y) noexcept;

// shared_ptr bool cast ?

//see /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1/memory

private:
  // Reference counter
  Counter* m_counter;

  // Shared pointer
  T* m_ptr;
};

//int main()
//{
//  // ptr1 pointing to an integer.
//  Shared_ptr<int> ptr1(new int(151));
//  cout << "--- Shared pointers ptr1 ---\n";
//  *ptr1 = 100;
//  cout << " ptr1's value now: " << *ptr1 << endl;
//  cout << ptr1;
//
//  {
//    // ptr2 pointing to same integer
//    // which ptr1 is pointing to
//    // Shared pointer reference counter
//    // should have increased now to 2.
//    Shared_ptr<int> ptr2 = ptr1;
//    cout << "--- Shared pointers ptr1, ptr2 ---\n";
//    cout << ptr1;
//    cout << ptr2;
//
//    {
//      // ptr3 pointing to same integer
//      // which ptr1 and ptr2 are pointing to.
//      // Shared pointer reference counter
//      // should have increased now to 3.
//      Shared_ptr<int> ptr3(ptr2);
//      cout << "--- Shared pointers ptr1, ptr2, ptr3 ---\n";
//      cout << ptr1;
//      cout << ptr2;
//      cout << ptr3;
//    }
//
//    // ptr3 is out of scope.
//    // It would have been destructed.
//    // So shared pointer reference counter
//    // should have decreased now to 2.
//    cout << "--- Shared pointers ptr1, ptr2 ---\n";
//    cout << ptr1;
//    cout << ptr2;
//  }
//
//  // ptr2 is out of scope.
//  // It would have been destructed.
//  // So shared pointer reference counter
//  // should have decreased now to 1.
//  cout << "--- Shared pointers ptr1 ---\n";
//  cout << ptr1;
//
//  return 0;
//}

#endif /* CPP_FOUNDATION_SHARED_PTR_H_ */
