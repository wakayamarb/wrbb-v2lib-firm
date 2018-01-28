#ifndef __IPAddress_h__
#define __IPAddress_h__

//#include <stdint.h>
#include <Printable.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

class IPAddress : public Printable{
  public:
    union {
    uint8_t bytes[4];  // IPv4 address
    uint32_t dword;
    } _address;

    uint8_t* raw_address() { return _address.bytes; };

//    virtual ~printTo(){}
    IPAddress();
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    IPAddress(uint32_t address);
    IPAddress(const uint8_t *address);

    operator uint32_t() const { return _address.dword; };

    uint8_t operator[](int index) const { return _address.bytes[index]; };
    IPAddress& operator=(const uint8_t *address);

    uint8_t& operator[](int index) { return _address.bytes[index]; };
    IPAddress& operator=(uint32_t address);

    virtual size_t printTo(Print& p) const;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  //#ifndef __IPAddress_h__
