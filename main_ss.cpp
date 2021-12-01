#include "spreadsketch.hpp"
#include "inputadaptor.hpp"
#include <utility>
//#include "datatypes.hpp"
#include "util.h"
#include <fstream>
#include <iomanip>
#include <map>

int tuple_equal1(tuple_t a, tuple_t b) {
    if(a.src_ip == b.src_ip && a.dst_ip == b.dst_ip && a.src_port == b.src_port && a.dst_port == b.dst_port && a.protocol == b.protocol)
        return 1;
    else return 0;
}

int main(int argc, char* argv[]) {

    // Configure parameters
    // pcap traces
    const char* filenames = "iptraces.txt";
    // buffer size
    unsigned long long buf_size = 500000000;
    // Superspreader threshold
    double thresh = 0.0001;
    // SpreadSketch parameters
    int lgn = 32;
    int cmdepth = 4;
    int cmwidth = 14096;
    int b = 79;
    int c = 3;
    int memory = 438;
    int total_mem = cmdepth*cmwidth*(memory+lgn+8)/1024/8;
    std::ifstream tracefiles(filenames);
    if (!tracefiles.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return -1;
    }

#ifdef HH
    //number of filter units in HP-Filter
    int len = 32768;
    unsigned mask = 0;
    if (len && (!(len & (len-1))) != 0) {
        mask = (unsigned)(len-1);
    } else {
        std::cout << "[Error] len should be power of 2" << std::endl;
        return -1;
    }
#endif


    // Result array
    double precision = 0, recall = 0, error = 0, throughput = 0, dtime = 0;
    double avgpre = 0, avgrec = 0, avgerr = 0, avgthr = 0, avgdet = 0;
    int epoch = 0;
	std::map<unsigned long long, int> heavypair;
    for (std::string file; getline(tracefiles, file);) {
        epoch++;
        InputAdaptor* adaptor =  new InputAdaptor(file, buf_size);
        std::cout << "[Dataset]: " << file << std::endl;
        std::cout << "[Message] Finish read data." << std::endl;
        // Get the ground truth
        mymap ground;
        edgeset sum;
        tuple_t t;
        std::map<tuple_t, int> flowsize;
        adaptor->Reset();
        while(adaptor->GetNext(&t) == 1) {
            edge_tp eg;
            eg.src_ip = t.src_ip; eg.dst_ip = t.dst_ip;
            sum.insert(eg);
            ground[t.src_ip].insert(t.dst_ip);
			//add by xy
			unsigned long long edge = t.src_ip;
			edge = (edge << 32) | t.dst_ip;
			heavypair[edge]++;
            flowsize[t]++;
        }
        std::cout << "[Message] Finish Insert hash table" << std::endl;

        // Calculate the threshold value
        double threshold = thresh * sum.size();
        std::cout << "[Message] Total distinct paris: " << sum.size() << "  threshold=" << threshold<< std::endl;

        // Create sketch
#ifdef HH
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory, len, mask);
#else
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory);
#endif

        // Update sketch
        double t1=0, t2=0;
        double datasize = adaptor->GetDataSize();
        t1 = now_us();
        adaptor->Reset();
        while(adaptor->GetNext(&t) == 1) {
            sketch->Update(t, 1);
        }
        t2 = now_us();
        throughput = datasize/(double)(t2-t1)*1000000;

        // Query the result
        t1=0, t2=0;
        myvector results;
        results.clear();
        t1 = now_us();
        sketch->Query(threshold, results);
        t2 = now_us();
        dtime = (double)(t2-t1)/1000000;

        // Calculate accuracy
        int tp = 0, cnt = 0;
        double err = 0;
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
		//add by xy
		std::map<unsigned long long, int> filter_res;
        std::map<tuple_t, int> heavyflow;
		// for(int i = 0; i < len; i++) {
		// 	// uint64_t edge = sketch->getValue4(i);
		// 	// int est = sketch->getValue5(i);
		// 	// if(est > 10000)
		// 	// 	filter_res[edge] = est;
        //     tuple_t t = sketch->getValue4(i);
		// 	int est = sketch->getValue7(i);
		// 	if(est > 1000)
		// 		heavyflow[t] = est;
		// }
		int tp_count = 0, cnt_count = 0;
		// for(auto it = flowsize.begin(); it != flowsize.end(); it++) {
		// 	if(it->second > 1000) {
		// 		cnt_count ++;
		// 		for(auto res = heavyflow.begin(); res != heavyflow.end(); res++) {
		// 			if(tuple_equal1(res->first, it->first)) {
		// 				tp_count++;
        //                 err += abs((double)it->second - res->second) / it->second;
		// 			}
		// 		}
 		// 	}
		// }
		// std::cout << filter_res.size() << " tp: " << tp_count << " cnt: " << cnt_count << std::endl;
		// std::cout << "Heavy pair total count: " << cnt_count << " presicion: " << tp_count*1.0/heavyflow.size() << " recall: " << tp_count*1.0/cnt_count << " error: " << err/tp_count << std::endl;
        std::cout << "[Message] Total " << cnt << " superspreaders, detect " << tp << std::endl;
        precision = tp*1.0/results.size();
        recall = tp*1.0/cnt;
        error = error/tp;

        avgpre += precision; avgrec += recall; avgerr += error; avgthr += throughput; avgdet += dtime;
        delete sketch;
        delete adaptor;

        //Output to standard output
        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Memory(KB)" << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall" << std::setw(20)
            << std::left << "Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
            << std::left << "DTime" << std::endl;
        std::cout << std::setw(20) << std::left << total_mem << std::setw(20) << std::left << precision << std::setw(20)
            << std::left << recall << std::setw(20) << std::left << error << std::setw(20) << std::left << throughput << std::setw(20)
            << std::left << dtime << std::endl;
    }

            //Output to standard output
        std::cout << "---------------------------------  summary ---------------------------------" << std::endl;
        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Memory(KB)" << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall" << std::setw(20)
            << std::left << "Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
            << std::left << "DTime" << std::endl;
        std::cout << std::setw(20) << std::left << total_mem << std::setw(20) << std::left << avgpre/epoch << std::setw(20)
            << std::left << avgrec/epoch << std::setw(20) << std::left << avgerr/epoch << std::setw(20) << std::left << avgthr/epoch << std::setw(20)
            << std::left << avgdet/epoch << std::endl;

}

