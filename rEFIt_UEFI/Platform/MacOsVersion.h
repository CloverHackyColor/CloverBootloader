#ifndef __MacOsVersion_H__
#define __MacOsVersion_H__

#include "../cpp_foundation/XStringArray.h"
#include "Utils.h"

const XString8 getSuffixForMacOsVersion(int LoaderType);

class AbstractMacOsVersion
{
  public:
    static const int nbMaxElement = 5; // if you modify, update the initializer/
    XString8 lastError = ""_XS8; // to silence warning effc++
  protected:
    int versionsNumber[nbMaxElement] = { -1, -1, -1, -1, -1 };

  public:
  
    AbstractMacOsVersion() {};
    AbstractMacOsVersion(int _versionsNumber[nbMaxElement]) {
      size_t idx;
      for ( idx=0 ; idx < nbMaxElement && _versionsNumber[idx] != -1 ; idx++ ) versionsNumber[idx] = _versionsNumber[idx];
      for ( ; idx < nbMaxElement ; idx++ ) if ( _versionsNumber[idx] != -1 ) panic("MacOsVersion::ctor _versionsNumber[%zu] != -1", idx);
    };

    int nbElement() const { int idx; for ( idx=0 ; idx < nbMaxElement && versionsNumber[idx] != -1 ; idx++ ) {}; return idx; }
    int lastElement() const { int idx; for ( idx=1 ; idx < nbMaxElement && versionsNumber[idx] != -1 ; idx++ ) {}; return versionsNumber[idx-1]; }
    void setEmpty() { lastError.setEmpty(); for ( size_t idx=0 ; idx < nbMaxElement ; idx++ ) versionsNumber[idx] = -1; }
    bool isEmpty() const { return versionsNumber[0] == -1; }
    bool notEmpty() const { return !isEmpty(); }

    template<typename IntegralType, enable_if(is_integral(IntegralType))>
    int elementAt(IntegralType i) const {
      if (i < 0) {
        panic("MacOsVersion::elementAt : i < 0. System halted\n");
      }
      if ( (unsigned_type(IntegralType))i >= nbMaxElement ) {
        panic("MacOsVersion::elementAt : i >= nbMaxElement. System halted\n");
      }
      return versionsNumber[(unsigned_type(IntegralType))i];
    }

    template<typename IntegralType, enable_if(is_integral(IntegralType))>
    XString8 asString(IntegralType i) const {
      if (i <= 0) {
        panic("MacOsVersion::asString : i < 0. System halted\n");
      }
      if ( (unsigned_type(IntegralType))i > nbMaxElement ) {
        panic("MacOsVersion::asString : i > nbMaxElement. System halted\n");
      }
      if ( versionsNumber[0] == -1 ) return NullXString8;
      XString8 returnValue;
      if ( versionsNumber[0] == -2 ) {
        returnValue.S8Printf("x");
      }else{
        returnValue.S8Printf("%d", versionsNumber[0]);
      }
      for ( size_t idx=1 ; idx < (unsigned_type(IntegralType))i && versionsNumber[idx] != -1 ; idx++ ) {
        if ( versionsNumber[idx] == -2 ) {
          returnValue.S8Catf(".x");
        }else{
          returnValue.S8Catf(".%d", versionsNumber[idx]);
        }
      }
      return returnValue;
    }

    XString8 asString() const { return asString(nbMaxElement); }

};

class MacOsVersionPattern : public AbstractMacOsVersion
{
  public:

    MacOsVersionPattern() : AbstractMacOsVersion() {};
    MacOsVersionPattern(int _versionsNumber[nbMaxElement]) : AbstractMacOsVersion(_versionsNumber) {};

    template <class XStringClass, enable_if( is___String(XStringClass) || is___LString(XStringClass) ) >
    MacOsVersionPattern(const XStringClass& versionAsString)
    {
      this->takeValueFrom(versionAsString);
    }
    
    template <class XStringClass, enable_if( is___String(XStringClass) || is___LString(XStringClass) ) >
//    MacOsVersionPattern& operator = ( const XStringClass& versionAsString)
    MacOsVersionPattern& takeValueFrom(const XStringClass& versionAsString)
    {
      setEmpty(); // we call our own setEmpty although we already are empty (this is a ctor). That's because in case of nbMaxElement is increased and there is a missing value in versionsNumber array initializer.
      size_t currentElementIdx = 0;
      int* currentElementPtr = &versionsNumber[currentElementIdx];
      size_t idx;
      for ( idx=0 ; idx < versionAsString.length(); idx++ )
      {
        if ( *currentElementPtr == -2 ) {
          if ( versionAsString[idx] == 'x' || versionAsString[idx] == 'X' ) {
            // ok. we allow multiple following x.
          }else
          if ( versionAsString[idx] == '.' ) {
            currentElementIdx += 1;
            if ( currentElementIdx >= nbMaxElement ) {
              setEmpty();
              lastError.S8Printf("Version number cannot be more than %d numbers.", nbMaxElement);
              return *this;
            }
            currentElementPtr = &versionsNumber[currentElementIdx];
          }else{
            setEmpty();
            lastError.S8Printf("Version number wildcard ('x') must be followed by a dot.");
            return *this;
          }
        }else
        if ( *currentElementPtr == -1 ) {
          if ( versionAsString[idx] == 'x' || versionAsString[idx] == 'X' ) {
            *currentElementPtr = -2;
          }else
          if ( IS_DIGIT(versionAsString[idx]) ) {
            *currentElementPtr = ((int)versionAsString[idx]) - '0'; // safe cast because versionAsString[idx] is a digit.
          }else
          {
            setEmpty();
            lastError.S8Printf("Version number must be numbers separated with one dot.");
            return *this;
          }
        }else{
          if ( versionAsString[idx] == '.' ) {
            currentElementIdx += 1;
            if ( currentElementIdx >= nbMaxElement ) {
              setEmpty();
              lastError.S8Printf("Version number cannot be more than %d numbers.", nbMaxElement);
              return *this;
            }
            currentElementPtr = &versionsNumber[currentElementIdx];
          }else
          if ( IS_DIGIT(versionAsString[idx]) ) {
            *currentElementPtr = *currentElementPtr * 10  + ((int)versionAsString[idx]) - '0'; // safe cast because versionAsString[idx] is a digit.
          }else{
            setEmpty();
            lastError.S8Printf("Version number must be numbers separated with one dot.");
            return *this;
          }
        }
      }
      return *this;
    }
    
    template<typename CharType, enable_if(is_char(CharType))>
    MacOsVersionPattern& operator = ( const CharType* p) { return *this = LString8(p); }

    
    MacOsVersionPattern(const MacOsVersionPattern& other)
    {
      lastError = other.lastError;
      memcpy(versionsNumber, other.versionsNumber, sizeof(versionsNumber));
    }
    MacOsVersionPattern& operator = ( const MacOsVersionPattern& other)
    {
      lastError = other.lastError;
      memcpy(versionsNumber, other.versionsNumber, sizeof(versionsNumber));
      return *this;
    }

};


class MacOsVersion : public AbstractMacOsVersion
{
  public:

    MacOsVersion() : AbstractMacOsVersion() {};
    MacOsVersion(int _versionsNumber[nbMaxElement]) : AbstractMacOsVersion(_versionsNumber) {};

    template <class XStringClass, enable_if( is___String(XStringClass) || is___LString(XStringClass) ) >
    MacOsVersion(const XStringClass& versionAsString)
    {
      this->takeValueFrom(versionAsString);
    }
    
    template <class XStringClass, enable_if( is___String(XStringClass) || is___LString(XStringClass) ) >
//    MacOsVersion& operator = ( const XStringClass& versionAsString)
    MacOsVersion& takeValueFrom(const XStringClass& versionAsString)
    {
      setEmpty(); // we call our own setEmpty although we already are empty (this is a ctor). That's because in case of nbMaxElement is increased and there is a missing value in versionsNumber array initializer.
      size_t currentElementIdx = 0;
      int* currentElementPtr = &versionsNumber[currentElementIdx];
      for ( size_t idx=0 ; idx < versionAsString.length(); idx++ )
      {
        if ( *currentElementPtr == -1 ) {
          if ( !IS_DIGIT(versionAsString[idx]) ) {
            setEmpty();
            lastError.S8Printf("Version number must be numbers separated with one dot.");
            return *this;
          }
          *currentElementPtr = ((int)versionAsString[idx]) - '0'; // safe cast because versionAsString[idx] is a digit.
        }else{
          if ( versionAsString[idx] == '.' ) {
            currentElementIdx += 1;
            if ( currentElementIdx >= nbMaxElement ) {
              setEmpty();
              lastError.S8Printf("Version number cannot be more than %d numbers.", nbMaxElement);
              return *this;
            }
            currentElementPtr = &versionsNumber[currentElementIdx];
          }else
          if ( IS_DIGIT(versionAsString[idx]) ) {
            *currentElementPtr = *currentElementPtr * 10  + ((int)versionAsString[idx]) - '0'; // safe cast because versionAsString[idx] is a digit.
          }else{
            setEmpty();
            lastError.S8Printf("Version number must be numbers separated with one dot.");
            return *this;
          }
        }
      }
      return *this;
    }
    
    template<typename CharType, enable_if(is_char(CharType))>
    MacOsVersion& operator = ( const CharType* p) { return *this = LString8(p); }
    
    MacOsVersion(const MacOsVersion& other)
    {
      lastError = other.lastError;
      memcpy(versionsNumber, other.versionsNumber, sizeof(versionsNumber));
    }
    MacOsVersion& operator = ( const MacOsVersion& other)
    {
      lastError = other.lastError;
      memcpy(versionsNumber, other.versionsNumber, sizeof(versionsNumber));
      return *this;
    }

        
    bool match(const MacOsVersionPattern& pattern) const
    {
//      int nbMax = nbElemen() <= pattern.nbMaxElement ? nbMaxElement : pattern.nbMaxElement;
      int idx;
      for ( idx=0 ; idx < nbMaxElement && versionsNumber[idx] != -1 && pattern.elementAt(idx) != -1 ; idx++ ) {
        if ( pattern.elementAt(idx) == -2 ) continue;
        if ( versionsNumber[idx] == pattern.elementAt(idx) ) continue;
        return false;
      }
      if ( idx >= nbMaxElement ) return true; // the whole pattern was macthed => ok.
      if ( versionsNumber[idx] == -1 ) {
        // self is shorter than pattern
        if ( pattern.elementAt(idx) == -1 ) return true; // pattern and self are the same length, and they matched.
        if ( pattern.nbElement() == idx+1 && pattern.lastElement() == -2 ) return true;
      }else{
        // pattern is short then self
        if ( pattern.lastElement() == -2 ) return true;
      }
      return false;
    }
    
    bool operator ==(const MacOsVersion &other) const
    {
      for ( size_t idx=0 ; idx < nbMaxElement ; idx++ ) {
        if ( versionsNumber[idx] != other.elementAt(idx) ) return false;
      }
      return true;
    };
    bool operator !=(const MacOsVersion &other) const { return ! ( *this == other); }
    
    bool operator <(const MacOsVersion &other) const
    {
      for ( size_t idx=0 ; idx < nbMaxElement ; idx++ ) {
        if ( versionsNumber[idx] < other.elementAt(idx) ) return true;
        if ( versionsNumber[idx] > other.elementAt(idx) ) return false;
      }
      return false; // here, means it's equal
    };
    bool operator >=(const MacOsVersion &other) const { return ! ( *this < other); }
    
    bool operator >(const MacOsVersion &other) const
    {
      for ( size_t idx=0 ; idx < nbMaxElement ; idx++ ) {
        if ( versionsNumber[idx] > other.elementAt(idx) ) return true;
        if ( versionsNumber[idx] < other.elementAt(idx) ) return false;
      }
      return false; // here, means it's equal
    };
    bool operator <=(const MacOsVersion &other) const { return ! ( *this > other); }
};



extern MacOsVersion nullMacOsVersion;

#endif // __MacOsVersion_H__

