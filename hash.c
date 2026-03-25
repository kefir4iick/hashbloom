#include <stdint.h>
#include <stddef.h>

static uint16_t make_word(uint8_t low, uint8_t high) {
    return (uint16_t)low | ((uint16_t)high << 8);
}

static uint16_t hash(uint16_t k, const uint8_t *data, size_t len) {
    uint16_t Y = 1;

    for (size_t i = 0; i < len; i += 2) {
        uint8_t a1 = data[i];
        uint8_t a2 = (i + 1 < len) ? data[i + 1] : 0;

        uint16_t xi = make_word(a1, a2);
        Y = (uint16_t)(Y * k + xi);
    }

    return Y;
}
