#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "hash.c"


#define N (1 << 16)
#define BIT_SIZE (N / 64)
#define MAX_LEN 100



typedef struct {
    uint64_t bits[BIT_SIZE];
} BitArray;

static void bit_set(BitArray *b, uint16_t pos) {
    b->bits[pos >> 6] |= (1ULL << (pos & 63));
}

static int bit_get(BitArray *b, uint16_t pos) {
    return (b->bits[pos >> 6] >> (pos & 63)) & 1;
}

static void bit_clear(BitArray *b) {
    memset(b->bits, 0, sizeof(b->bits));
}


typedef struct {
    BitArray table;
    uint16_t *keys;
    size_t s;
} BloomFilter;

static uint16_t rnd16() {
    return (uint16_t)(rand() & 0xFFFF);
}

void bloom_init(BloomFilter *bf, size_t s) {
    bf->s = s;
    bf->keys = (uint16_t*)malloc(s * sizeof(uint16_t));

    for (size_t i = 0; i < s; i++) {
        uint16_t k;
        int ok;

        do {
            k = rnd16();
            ok = 1;

            for (size_t j = 0; j < i; j++) {
                if (bf->keys[j] == k) {
                    ok = 0;
                    break;
                }
            }
        } while (!ok);

        bf->keys[i] = k;
    }

    bit_clear(&bf->table);
}

void bloom_free(BloomFilter *bf) {
    free(bf->keys);
}

void bloom_add(BloomFilter *bf, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < bf->s; i++) {
        uint16_t h = hash(bf->keys[i], data, len);
        bit_set(&bf->table, h);
    }
}

int bloom_check(BloomFilter *bf, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < bf->s; i++) {
        uint16_t h = hash(bf->keys[i], data, len);
        if (!bit_get(&bf->table, h))
            return 0;
    }
    return 1;
}


static void gen_str(uint8_t *buf, size_t *len) {
    *len = rand() % MAX_LEN + 1;
    for (size_t i = 0; i < *len; i++)
        buf[i] = rand() & 0xFF;
}


double experiment(double alpha, size_t s) {
    size_t n = (size_t)(alpha * N);

    BloomFilter bf;
    bloom_init(&bf, s);

    uint8_t buf[MAX_LEN];
    size_t len;

    for (size_t i = 0; i < n; i++) {
        len = rand() % MAX_LEN + 1;
        for (size_t j = 0; j < len; j++)
            buf[j] = rand();

        bloom_add(&bf, buf, len);
    }

    size_t Q = 200000;   
    size_t false_positives = 0;

    for (size_t i = 0; i < Q; i++) {
        len = rand() % MAX_LEN + 1;
        for (size_t j = 0; j < len; j++)
            buf[j] = rand();

        if (bloom_check(&bf, buf, len))
            false_positives++;
    }

    bloom_free(&bf);

    return false_positives / (double)Q;
}


int main() {
    srand(time(NULL));

    double alphas[] = {0.05,0.10,0.15,0.20,0.25,0.30,0.35,0.40,0.45,0.50};

    printf("alpha\ts\tp_err\n");

    for (int i = 0; i < 10; i++) {
        double alpha = alphas[i];

        size_t s0 = (size_t)((1.0 / alpha) * log(2.0));

        size_t s_list[3] = {s0 ? s0 - 1 : 1, s0, s0 + 1};

        for (int j = 0; j < 3; j++) {
            size_t s = s_list[j];
            if (s == 0) continue;

            double avg = 0.0;

            for (int r = 0; r < 100; r++) {
                avg += experiment(alpha, s);
            }

            avg /= 100.0;

            printf("%.2f\t%zu\t%.8f\n", alpha, s, avg);
        }
    }

    return 0;
}
