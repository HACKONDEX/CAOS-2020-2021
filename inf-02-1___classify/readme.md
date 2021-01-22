**Problem inf02-1: generic/ieee754/classify**
============================================

    _Реализуйте функцию с прототипом:_

    typedef enum {
        PlusZero      = 0x00,
        MinusZero     = 0x01,
        PlusInf       = 0xF0,
        MinusInf      = 0xF1,
        PlusRegular   = 0x10,
        MinusRegular  = 0x11,
        PlusDenormal  = 0x20,
        MinusDenormal = 0x21,
        SignalingNaN  = 0x30,
        QuietNaN      = 0x31
    } float_class_t;

    extern float_class_t classify(double \*value_ptr);
      
    которая классифицирует специальные значения вещественного числа, 
    на которое указывает value_ptr, в соответствии со стандартом IEEE 754.

    При решении допускается использовать только побитовые операции, включая сдвиги.

***
