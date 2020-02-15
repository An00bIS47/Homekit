#ifndef __HAPFLASH_HPP__
#define __HAPFLASH_HPP__

#include <pgmspace.h>
#include "Arduino.h"

// Example: FLASH_ARRAY(float, temperatures, 98.1, 98.5, 99.1, 102.1);
#define FLASH_ARRAY(type, name, values...) \
static const type name##_flash[] PROGMEM = { values }; \
_FLASH_ARRAY<type> name(name##_flash, sizeof(name##_flash) / sizeof(type));


#ifndef ARDUINO_CORE_PRINTABLE_SUPPORT
class _Printable
{
public:
    virtual void print(Print &stream) const = 0;
};
#endif

/* _FLASH_ARRAY template class.  Use the FLASH_ARRAY() macro to create these. */
template<class T>
class _FLASH_ARRAY : public _Printable
{
    typedef T _DataType;
    
public:
    _FLASH_ARRAY(const _DataType *arr, size_t count) : _arr(arr), _size(count)
    { }
    
    size_t count() const
    { return _size; }
    
    size_t size() const
    { return _size; }
    
    size_t available()
    { return _size -_lastread; }
    
    void open()
    { _lastread=0; }
    
    void close()
    { _lastread=0; }
    
    size_t read(uint8_t *dst, size_t len)
    {
        size_t i = 0;
        
        for(i=0; i<len; i++) {
            
            if( _lastread >= _size ) {
                break;
            }
            
            dst[i] = (*this)[_lastread];
            _lastread++;
        }
        return i;
    }
    
    const _DataType *access() const
    { return _arr; }
    
    T operator[](int index) const
    {
        uint32_t val = 0;
        if (sizeof(T) == 1)
            val = pgm_read_byte(_arr + index);
            else if (sizeof(T) == 2)
                val = pgm_read_word(_arr + index);
                else if (sizeof(T) == 4)
                    val = pgm_read_dword(_arr + index);
                    return *reinterpret_cast<T *>(&val);
    }
    
    void print(Print &stream) const
    {
        for (size_t i=0; i<_size; ++i)
        {
            stream.print((*this)[i]);
            if (i < _size - 1)
                stream.print(",");
        }
    }
    
private:
    const _DataType *_arr;
    size_t _size;
    size_t _lastread;
};

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING

template<class T>
inline Print &operator <<(Print &stream, T arg)
{ stream.print(arg); return stream; }

#endif

inline Print &operator <<(Print &stream, const _Printable &printable)
{ printable.print(stream); return stream; }

template<class T>
inline Print &operator <<(Print &stream, const _FLASH_ARRAY<T> &printable)
{ printable.print(stream); return stream; }

#endif // def __HAPFLASH_HPP__
