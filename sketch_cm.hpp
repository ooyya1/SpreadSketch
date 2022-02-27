#ifndef COUNTMIN_H
#define COUNTMIN_H
#include <utility>
#include <algorithm>
#include <cstring>
#include <iostream>
#include "datatypes.hpp"
extern "C"
{
    #include "hash.h"
}

class CountMin {

    struct CCM_type {

        //Sketch depth
        int depth;

        //Sketch width
        int width;

        //Counter table
        val_tp * counts;
        unsigned long * hash;

        //# key bits
        int lgn;

        int bias;
    };

public:
    CountMin(int depth, int width, int lgn, int thresh);

    ~CountMin();

    void Update(key_tp src, key_tp dst, val_tp weight);

    val_tp PointQuery(key_tp src);

    void Query(val_tp thresh, myvector &results);

    void Reset();

    void SetThresh(double thresh);

    int GetHeapSize();

    void SetBias(int bias);

private:
    static bool less_comparer_desc(const std::pair<key_tp, val_tp> &a, const std::pair<key_tp, val_tp> &b)
    {
        return (b.second > a.second);
    }
    //Update threshold
    double thresh_;

    //Sketch data structure
    CCM_type ccm_;

    //Heap to store the heavy hitter
    myvector heap_;
};

#endif
