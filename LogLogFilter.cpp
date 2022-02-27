#include "LogLogFilter.hpp"

#define MAX_HASH_NUM 4
#define PHI 0.77351
#define NUM_BITS 4

int loghash(unsigned long p) {
    int ret = 0;
    while ((p&0x00000001) == 1) {
        p >>= 1;
        ret++;
    }
    return ret;
}

LLF::LLF(int d, int memory, int threshold) {
    this->d = d;
    this->w = memory / NUM_BITS;
    this->threshold = threshold;
    l = new unsigned [w]();
    hash = new unsigned [d]();
    char name[] = "LogLogFilter";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);
    for (int i = 0; i < d; i++) {
        hash[i] = GenHashSeed(seed++);
    }
}

LLF::~LLF() {
    delete [] l;
}

int LLF::update(key_tp src) {
    int value[MAX_HASH_NUM];
    int index[MAX_HASH_NUM];
    int updatesketch = 0;

    int v = 1 << 30;
    for (int i = 0; i < d; i++) {
        index[i] = MurmurHash64A((unsigned char*)(&src), 4, hash[d]) % w;
        value[i] = l[index[i]];
        v = std::min(value[i], v);
    }
    if (v < threshold) {
        f++;
        for (int i = 0; i < d; i++) {
            uint32_t seeds = src & timestamp;
            uint64_t random_num = GenHashSeed(seeds);
            int temp = loghash(random_num);
            uint32_t hash_value = (temp > threshold) ? threshold : temp;
            uint32_t beta = (l[index[i]] > hash_value) ? l[index[i]] : hash_value;
            l[index[i]] = beta;
        }
    }
    else {
        updatesketch = 1;
    }
    timestamp++;
    return updatesketch;
}

int LLF::query1(key_tp src) {
    double frequency = 0.0;
    double sum_value = 0.0;
    for (int i = 0; i < d; i++) {
        int index = MurmurHash64A((unsigned char*)(&src), 4, hash[d]) % w;
        sum_value = sum_value + l[index];
    }
    sum_value = sum_value / d;
    frequency = ((double)(w * d) / (w - d)) * (pow(2, sum_value) / PHI / d - (double)f / w);
    if (frequency > 0) {
        return frequency;
    }
    else {
        return (pow(2, sum_value) / PHI / d);
    }
}

int LLF::query2(key_tp src) {
    double frequency = 0.0;
    frequency = ((double)(w * d) / (w - d)) * (pow(2, threshold) / PHI / d - (double)f / w);
    return frequency;
}

int LLF::probe(key_tp src) {
    int v = 1 << 30;
    for (int i = 0; i < d; i++) {
        int index = MurmurHash64A((unsigned char*)(&src), 4, hash[d]) % w;
        int value = l[index];
        v = std::min(value, v);
    }
    if (v < threshold) {
        return 1;
    }
    else {
        return 0;
    }
}