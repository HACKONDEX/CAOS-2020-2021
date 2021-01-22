#include <stdio.h>
#include <inttypes.h>
#include <string.h>

const uint64_t all_digits_1 = (uint64_t) (~0);
const uint64_t one = 1;
const uint64_t plus_zero = +0;

typedef enum {
  PlusZero = 0x00,
  MinusZero = 0x01,
  PlusInf = 0xF0,
  MinusInf = 0xF1,
  PlusRegular = 0x10,
  MinusRegular = 0x11,
  PlusDenormal = 0x20,
  MinusDenormal = 0x21,
  SignalingNaN = 0x30,
  QuietNaN = 0x31
} float_class_t;

extern float_class_t calc_classify(uint64_t digits) {
    uint64_t sign = (digits >> 63); // 1 bit
    uint64_t mantis = ((digits << 12) >> 12); // 52 bit
    uint64_t exponent = ((digits << 1) >> 53);
    /// zero check
    if (exponent == 0x0u && mantis == 0x0u) {
        return sign == 0x0u ? PlusZero : MinusZero;
    }
    /// Inf check
    if (exponent == 0x7ffu && mantis == 0x0u) {
        return sign == 0x0u ? PlusInf : MinusInf;
    }

    ///  Qnan and Snan
    if (exponent == 0x7ffu) {
        uint64_t first_bit_of_mantis = (mantis >> 51);
        return first_bit_of_mantis == 0x1u ? QuietNaN : SignalingNaN;
    }
    /// Denormal check
    if (exponent == 0x0u) {
        return sign == 0x0u ? PlusDenormal : MinusDenormal;
    }
    /// Regular sign check
    return sign == 0x0u ? PlusRegular : MinusRegular;
}

extern float_class_t classify(double *value_ptr) {
    uint64_t digits;
    memcpy(&digits, value_ptr, sizeof(digits));
    return calc_classify(digits);
}
