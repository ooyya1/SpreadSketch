#include "virtualfilter.hpp"

VF::VF(int n, double p) {
    this->n = n;
    this->p = p;
    if(p < 1/exp(1)) {
        m = n * p * exp(1);
        virtual_m = n;
    }
    else if(p < 1) {
        m = n / log(1/p);
        virtual_m = m;
    }
    else {
        m = 8 * 1024 * 1024;
        virtual_m = m;
    }
    z = m;
    bmp = allocbitmap(m);
    //bmp = new unsigned char [m]();
    fillzero(bmp, m);
    std::cout << "m: " << m << " m': " << virtual_m << " p: " << p << std::endl;
    //std::cout << 1.0 * m / z * virtual_m * p<< std::endl;
    char name[] = "VirtualFilter";
    seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);  
    hash = GenHashSeed(seed);

}

VF::~VF() {
    free(bmp);
}

int VF::checkpass(key_tp src, key_tp dst) {
    int pass = 0;
    edge_tp tmp;
    tmp.src_ip = src; tmp.dst_ip = dst;
    int idx = MurmurHash64A((unsigned char*)(&tmp), 8, hash) % virtual_m;
    if(idx < m) {
        if(!getbit(idx, bmp)) {
            setbit(idx, bmp);
            z--;
            //std::cout << "pass" << std::endl;
            if(idx < 1.0 * m / z * virtual_m * p) {
                pass = 1;
            }
        }
    }
    // if(z <= virtual_m * p) {
    //     bmp = new unsigned char [(m+7)/8]();
    //     z = m;
    //     hash = GenHashSeed(++seed);
    // }
    return pass;
}