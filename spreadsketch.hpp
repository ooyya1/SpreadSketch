#ifndef __DETECTORSS_H__
#define __DETECTORSS_H__
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
#include <map>
#include "virtualfilter.hpp"
extern "C"
{
#include "hash.h"
}

#define MAX_COMPONENTS 32
static const double bitsetratio = 0.93105; //(1-1/exp(rhomax)) rhomax = 2.6744

class DetectorSS {

    struct CSS_type {

        /*********** parameters for multiresbitmaps *************/
        //The size of a normal component
        int b;

        //The number of components
        int c;

        //The size of the last component
        int lastb;

        //Used to select the base component
        int setmax;

        //bitmaps
        unsigned char ** counts;

        //The bit offsets of the components
        int* offsets;


        /************  parameters for SSketch **************/
        //Outer sketch depth
        int depth;

        //Outer sketch width
        int  width;

        //key table
        key_tp** skey;

        //level table
        int** level;

        //min table
        int **min;

        unsigned long * hash, *scale, *hardner;

        //# key bits
        int lgn;

        int tdepth;

#ifdef HH
        /**************** parameters for HH ******************/
        //unsigned long long *key;
        tuple_t *key;
        int *indicator;
		int *upperbound;
        int len;
        unsigned mask;
        int *positive;
        int *negative;
        int lambda = 8;
#endif


    };

public:
#ifdef HH
    DetectorSS(int depth, int width, int lgn, int b, int c, int memory, int len, unsigned mask);
#else
    DetectorSS(int depth, int width, int lgn, int b, int c, int memory);
#endif

    ~DetectorSS();

    void Update(key_tp src, key_tp dst, val_tp weight);

    //void Update(tuple_t t, val_tp weight);

    int PointQuery(uint32_t key);

    int PointQueryMerge(uint32_t key);

    int PointQueryMerge1(uint32_t key);

    void Query(val_tp thresh, std::vector<std::pair<key_tp, val_tp> > &results);

    void Reset();

    unsigned char** GetTable();

    void Merge(DetectorSS *detector);

    key_tp** GetKey();

    int** GetLevel();

	//add by xy
	std::map<int, int> basemap;

//private:

    //Sketch data structure
    CSS_type ss_;
private:
    //SS to store the heavy hitter

    //double sample_rate = 0.01;

    //VF * vf = new VF(1048576, this->sample_rate);

    std::vector<std::pair<uint32_t, uint64_t> > heap_;

    int loghash(unsigned long p);

    void Setbit(int n, int bucket,  unsigned char* bmp);

    int Estimate(int bucket, unsigned char* bmp);

    int Estimate1(int bucket, unsigned char* bmp);

    void Copybmp(unsigned char* dst, unsigned char* src, int start, int len);

    void Intersec(unsigned char* dst, unsigned char* src, int start, int len);

    void Mergebmp(unsigned char* bmp1, unsigned char* bmp2, int start, int len);
};



#endif

