#ifndef __VIRTUALFILTER_H__
#define __VIRTUALFILTER_H__
#include "bitmap.h"
#include <vector>
#include <utility>
#include "datatypes.hpp"
#include "inputadaptor.hpp"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>
#include <unordered_set>
#include <fstream>
extern "C"
{
#include "hash.h"
}

class VF {
private:
    int n;
    double p;
    int m;
    int virtual_m;
    int z;
    unsigned char * bmp;
    unsigned hash;
    unsigned long seed;
public:
    VF(int n, double p);
    ~VF();
    int checkpass(key_tp src, key_tp dst);
};

#endif