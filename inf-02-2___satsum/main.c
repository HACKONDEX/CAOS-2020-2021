#include <stdio.h>
#include <inttypes.h>

uint16_t satsum(uint16_t x, uint16_t y) {
    uint16_t sum = x + y;
    if(sum < x || sum < y) {
        return (uint16_t )(~0u);
    }
    return sum;
}
