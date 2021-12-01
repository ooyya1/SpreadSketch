#ifndef DATATYPE_H
#define DATATYPE_H
#include "hash.h"
#include "string.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

typedef uint32_t key_tp;

typedef uint32_t val_tp;

typedef struct edge_t_s {
    uint32_t src_ip;
    uint32_t dst_ip;
} edge_tp;

typedef struct __attribute__ ((__packed__)) Tuple {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
	bool operator <(const Tuple& other) const {
        return std::tie(src_ip,dst_ip,src_port,dst_port,protocol) < std::tie(other.src_ip,other.dst_ip,other.src_port,other.dst_port,other.protocol);
	}
} tuple_t;


/**
 * Object for hash
 */
typedef struct edge_tp_hash {
    /// overloaded operation
    long operator() (const edge_tp &k) const { return MurmurHash64A(&k, 8, 388650253); }
} edge_tp_hash;


/**
 * Object for equality
 */
typedef struct edge_tp_eq {
    /// overloaded operation
    bool operator() (const edge_tp &x, const edge_tp &y) const {
        return memcmp(&x, &y, 8)==0;
    }
} edge_tp_eq;

typedef std::unordered_set<key_tp> myset;

typedef std::unordered_map<key_tp, myset> mymap;

typedef std::unordered_map<key_tp, val_tp> nodemap;

typedef std::vector<std::pair<key_tp, val_tp> > myvector;

typedef std::unordered_set<edge_tp, edge_tp_hash, edge_tp_eq> edgeset;

#endif
