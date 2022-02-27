#include "sketch_cm.hpp"
#include <cstdlib>


CountMin::CountMin(int depth, int width, int lgn, int thresh) {
    this->thresh_ = thresh;
    ccm_.depth = depth;
    ccm_.width = width;
    ccm_.lgn = lgn;
    ccm_.bias = 0;
    ccm_.counts = new val_tp[ccm_.depth*ccm_.width]();
    ccm_.hash = new unsigned long[ccm_.depth];
    char name[] = "CountMin";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);
    for (int i = 0; i < ccm_.depth; i++) {
        ccm_.hash[i] = GenHashSeed(seed++);
    }
}

CountMin::~CountMin() {
    delete [] ccm_.hash;
    delete [] ccm_.counts;
}

void CountMin::Update(key_tp src, key_tp dst, val_tp val) {
    //Update sketch and get the estimation
    val_tp result = 0;
    for (int i = 0; i < ccm_.depth; i++) {
        unsigned long bucket = MurmurHash64A((unsigned char *)&src, ccm_.lgn/8, ccm_.hash[i]) % ccm_.width;
        int index =  i*ccm_.width+bucket;
        ccm_.counts[index] += val;
        if (i == 0) result = ccm_.counts[index];
        else result = std::min(result, ccm_.counts[index]);
    }
    //Store heavy node in heap, support both value and fraction threshold
    double threshold = thresh_;
    result += ccm_.bias;
    //std::cout << "threshold = " << threshold << "\t result = " << result << std::endl;
    if (result >= threshold) {
        //If heap is empty
        if (heap_.size() == 0) {
            std::pair<key_tp, val_tp> node(src, val);
            heap_.push_back(node);
            std::make_heap(heap_.begin(), heap_.end(), less_comparer_desc);
        } else {
            //Check if node is in the heap
            myvector::iterator it;
            for ( it = heap_.begin(); it != heap_.end(); ++it) {
                if(it->first == src) break;
            }
            //Node is not in the heap, insert it
            if (it == heap_.end()) {
                std::pair<key_tp, val_tp> node(src, val);

                heap_.push_back(node);
                std::push_heap(heap_.begin(), heap_.end(), less_comparer_desc);
            } else {
                it->second = result;
                std::make_heap(heap_.begin(), heap_.end(), less_comparer_desc);
            }
        }

        if (heap_.size() > 1024) {
            std::sort_heap(heap_.begin(), heap_.end(), less_comparer_desc);
            heap_.erase(heap_.end());
            std::make_heap(heap_.begin(), heap_.end(), less_comparer_desc);
        }
    }
}



void CountMin::Query(val_tp thresh, myvector &results) {
    for (auto it = heap_.begin(); it != heap_.end(); it++) {
        std::pair<key_tp, val_tp> node;
        node.first = it->first;
        node.second = it->second;
        if (node.second > thresh) {
            results.push_back(node);
        }
    }
}

val_tp CountMin::PointQuery(key_tp key) {
    val_tp ret=0;
    val_tp degree=0;
    for (int i=0; i<ccm_.depth; i++) {
        unsigned long bucket = MurmurHash64A((unsigned char*)&key, ccm_.lgn/8, ccm_.hash[i]) % ccm_.width;
        if (i==0) {
            ret = ccm_.counts[i*ccm_.width+bucket];
        }
        degree = ccm_.counts[i*ccm_.width+bucket];
        ret = ret > degree ? degree : ret;
    }
    return ret+ccm_.bias;
}

void CountMin::Reset() {
    std::fill(ccm_.counts, ccm_.counts+ccm_.depth*ccm_.width, 0);
    heap_.clear();
}

void CountMin::SetThresh(double thresh) {
    this->thresh_ = thresh;
}

int CountMin::GetHeapSize()  {
    std::cout << "[in cm]  heap size=" << heap_.size() << std::endl;
    return this->heap_.size();
}

void CountMin::SetBias(int bias) {
    ccm_.bias = bias;
}

