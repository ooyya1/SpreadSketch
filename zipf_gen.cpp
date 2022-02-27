//#include "src/ss/detector_ss.hpp"
#include "inputadaptor.hpp"
#include <utility>
#include "datatypes.hpp"
#include "util.h"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include "spreadsketch.hpp"
//#include "lib/evaluation.hpp"

int main(int argc, char* argv[]) {

#ifdef BFSAMPLE
    if (argc != 6) {
        std::cout << "[Usage] " << argv[0] << "\t[Config File]" << "\t [Memory(MB)]" << "\t [sample rate]" << "\t [zipf parameter]"
            << "\t [bf_len]" << std::endl;
        return -1;
    }
#elif defined HH
    if (argc != 5) {
        std::cout << "[Usage] " << argv[0] << "\t[Config File]" << "\t [Memory(MB)]" << "\t [#Buckets](power of 2)" << "\t [zipf parameter]" << std::endl;
        return -1;
    }
#else
    if (argc != 3) {
        std::cout << "[Usage] " << argv[0] << "\t [Memory(0.5/MB)]" << "\t [zipf parameter]" << std::endl;
        return -1;
    }
#endif

    srand ( 1234567 );
    //parameter setting
    // conf_t* conf;
    // conf = Config_Init(argv[1]);
    //Common parameter
    const char* filenames = "iptraces.txt";
    // buffer size
    unsigned long long buf_size = 500000000;
    // Superspreader threshold
    double thresh = 0.0001;
    int lgn = 32;
    double allspace = (atof(argv[2])+1)*0.5;
    allspace = allspace*8*1024*1024;

#ifdef BFSAMPLE
    double sample = atof(argv[3]);
    int len = atoi(argv[4]);
    unsigned bf_len = len*96;
    std::cout << "sample=" << sample << "\t bf_len=" << bf_len << std::endl;
#elif defined HH
    int len = atoi(argv[3]);
    unsigned mask = 0;
    if (len && (!(len & (len-1))) != 0) {
        mask = (unsigned)(len-1);
    } else {
        std::cout << "[Error] len should be power of 2" << std::endl;
        return -1;
    }
    allspace = allspace - len * (64+32);
    std::cout << "len=" << len << "\t mask=" << mask << std::endl;
#endif
    int cmdepth = 4;
    int cmwidth = 14096;
    int b = 79;
    int c = 3;
    int memory = 438;


    //1. generate zipf sequence

    //2. construct data from CAIDA traces
    //2.1 read pcap and sort by frequency
    std::ifstream tracefiles(filenames);
    if (!tracefiles.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return -1;
    }


    for (std::string file; getline(tracefiles, file);) {
        std::set<unsigned long> edge;
        InputAdaptor *adaptor =  new InputAdaptor(file, buf_size);
        //Get ground
        std::map<unsigned long, unsigned> fremap;
        fremap.clear();
        tuple_t t;
        adaptor->Reset();
        unsigned long freq_sum = 0;
        unsigned long edge_sum = 0;
        while(adaptor->GetNext(&t) == 1) {
            unsigned long ip = t.src_ip;
            ip = (ip << 32) | t.dst_ip;
            if (fremap.find(ip) != fremap.end()) fremap[ip]++;
            else fremap[ip] = 1;
            edge.insert(ip);
            freq_sum++;
        }
        edge_sum = edge.size();

        int N = edge_sum;
#ifdef HH
        double z = atof(argv[4]);
#else
        double z = atof(argv[2]);
#endif
        std::cout << "Zipf skew = " << z << std::endl;
        int i = 0;
        double harmonic = 0;
        double *part = (double*)malloc((N+1)*sizeof(double));
        double temp = 0;
        double *freq = (double*)malloc((N+1)*sizeof(double));
        for (i = 1; i <= N; i++) {
            temp = pow(i, z);
            part[i] = temp;
            harmonic += 1/temp;
            //printf("[Test] i=%d, harmonic=%lf\n", i, harmonic);
        }
        double accu = 0;
        for (i = 1; i < N; i++) {
            freq[i] = 1.0/(part[i]*harmonic);
            accu += freq[i];
            if (i == 1000) {
                std::cout << " top 1000 accu=" << accu << std::endl;
            }
        }
        free(part);

        std::cout << "1. Finish construct zipf sequence" << std::endl;




        std::vector<std::pair<unsigned long, unsigned> > groundvec;
        groundvec.clear();
        for (auto it = fremap.begin(); it != fremap.end(); it++) {
            groundvec.push_back(*it);
        }
        std::sort(groundvec.begin(), groundvec.end(), [=](std::pair<unsigned long, unsigned>&a, std::pair<unsigned long, unsigned>&b)
                {
                return a.second > b.second;
                }

                );

        delete adaptor;
        std::cout << "2.1 Finish sort pcap data" << std::endl;
        //2.2. re-set the frequency according to the zipf sequence
        //
        unsigned test = 0;
        for (i = 0; i < N; i++) {
            unsigned value = freq[i+1]*freq_sum;
            value = value > 1 ? value : 1;
            groundvec[i].second = value;
            test += value;
        }
        //freq_sum = test;
        double threshold = edge_sum * thresh;
        std::cout << "test=" << test << "\t total_pkt=" << freq_sum << std::endl;
        free(freq);
        std::cout << "2.2 Finish reset pcap frequency according to the zipf sequence" << std::endl;

        //2.3 construct the skewed pacp trace
        unsigned long *buffer = (unsigned long*)malloc(freq_sum*sizeof(unsigned long));
        std::set<unsigned long> newedge;
        newedge.clear();
        int *shuffl = (int*)malloc(freq_sum*sizeof(int));
        for (i = 0; i < freq_sum; i++) {
            shuffl[i] = i;
        }

        //shufl
        for (i = freq_sum-1; i > 0; i--) {
            int j = rand() % (i+1);
            //std::cout <<"i=" << i << "\t j=" << j << std::endl;
            int temp = shuffl[i];
            shuffl[i] = shuffl[j];
            shuffl[j] = temp;
        }


        int index = 0;
        for (i = 0; i < N; i++) {
            for (int j = 0; j < groundvec[i].second; j++) {
                buffer[shuffl[index]] = groundvec[i].first;
                index++;
            }
        }


        free(shuffl);
        std::cout << "2.3 Finish construct shuffl data" << std::endl;

        //3. calculate ground

        mymap ground;
        for (i = 0; i < freq_sum; i++) {
            unsigned long ip = buffer[i];
            ground[ip>>32].insert(ip&0xffffffff);
            newedge.insert(ip);
        }
        std::cout << "3 Finish construct ground truth" << std::endl;

        //4. update sketch

        //re-initialize the random seed
        srand (259374261);

#ifdef BFSAMPLE
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory, sample, bf_len);
#elif defined HH
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory, len, mask);
#else
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory);
#endif
        std::cout << "4. Finish updating sketch" << std::endl;

        //Evaluation* eva = new Evaluation();
        double throughput = 0;
        double t1 = 0, t2 = 0;
        t1 = now_us();
        for (i = 0; i < freq_sum; i++) {
            unsigned long ip = buffer[i];
            sketch->Update(ip>>32, ip&0xffffffff, 1);
        }
        t2 = now_us();
        free(buffer);
        throughput = freq_sum*1000000.0/(t2-t1);
        double precision = 0, recall = 0, error = 0;
        myvector results;
        results.clear();
        double dtime=0;
        t1 = now_us();
        sketch->Query(threshold, results);
        t2 = now_us();
        dtime = (double)(t2-t1)/1000000;
        int tp = 0, cnt = 0;
        for (auto it = ground.begin(); it != ground.end(); it++) {
            if (it->second.size() > threshold) {
                cnt++;
                int truth = (int) it->second.size();
                for (auto res = results.begin(); res != results.end(); res++) {
                    if (res->first == it->first) {
                        error += abs((int)res->second - truth)*1.0/truth;
                        tp++;
                    }
                }
            }
        }
        std::cout << "[Message] Total " << cnt << " superspreaders, detect " << tp << std::endl;
        precision = tp*1.0/results.size();
        recall = tp*1.0/cnt;
        error = error/tp;

        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Detector"
            << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall"
            << std::setw(20) << std::left << "Error"
            << std::setw(20) << std::left << "Throughput"
            << std::setw(20) << std::left << "DTime" << std::endl;
        std::cout << std::setw(20) << std::left << "Sketch"
            << std::setw(20) << std::left << precision
            << std::setw(20) << std::left << recall
            << std::setw(20) << std::left << error
            << std::setw(20) << std::left << throughput
            << std::setw(20) << std::left << dtime << std::endl;
    }



return 0;
}
