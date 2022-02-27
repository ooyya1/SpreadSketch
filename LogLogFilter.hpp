#ifndef LLF_H
#define LLF_H

#include "bitmap.h"
#include <iostream>
#include "datatypes.hpp"
#include <algorithm>
#include <cmath>

class LLF {
private:
    int d;
    int w;
    unsigned timestamp = 0;
    unsigned f = 0;
    unsigned * l;
    unsigned * hash;
    int threshold;
public:
    LLF(int d, int memory, int threshold);
    ~LLF();
    int update(key_tp src);
    int query1(key_tp src);
    int query2(key_tp src);
    int probe(key_tp src);
};

#endif