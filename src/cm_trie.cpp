#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include "common/radix_sort.hpp"
#include "common/utils.hpp"
#include "generator/generator.hpp"

#define GENCLASS GenRandom

#define N_ITER 10
#define MIN_BUFSIZ 8192

#define SAMPLE_STRIP (1<<17)
#define SAMPLE_EQUAL 8

#define CM_DEPTH 3
#define CM_WIDTH 100000

#define RADIX_BIT 8
#define RADIX_MASK (((uint64_t)1 << RADIX_BIT) - 1)
#define INITIAL_SHIFT (sizeof(uint64_t)*8 - RADIX_BIT)

#define NUM_NODE 65536
#define MAXINT 0x7fffffff

uint64_t seeds[] = {0xdeadbeef,
                    0xcafebabe,
                    0x12345678,
                    0x87654321,
                    0x12341234,
                    0x43214321,
                    0x87658765,
                    0x56785678
                    };

struct node_t {
    int begin;
    int end;
    // the 0th child of each node
    // the ith child of node x is (child[x] + i)
    // child[root_node] is 0
    // i.e., the top 256 nodes in the trie are 0-255
    uint16_t child;
    // the next radix to sort
    uint8_t shift;
};


static inline uint64_t hash(uint64_t key, uint64_t seed) {
    uint64_t res = seed;
    uint8_t a0 = (key >> 0) & 0xff;
    uint8_t a1 = (key >> 8) & 0xff;
    uint8_t a2 = (key >> 16) & 0xff;
    uint8_t a3 = (key >> 24) & 0xff;
    uint8_t a4 = (key >> 32) & 0xff;
    uint8_t a5 = (key >> 40) & 0xff;
    uint8_t a6 = (key >> 48) & 0xff;
    uint8_t a7 = (key >> 56) & 0xff;
    res = res * seed + a7;
    res = res * seed + a6;
    res = res * seed + a5;
    res = res * seed + a4;
    res = res * seed + a3;
    res = res * seed + a2;
    res = res * seed + a1;
    res = res * seed + a0;
    return res;
}

static inline void cm_set1(int *cm, uint64_t key) {
    int i;
    for (int i = 0; i < CM_DEPTH; i ++) {
        size_t a = hash(key, seeds[i]) % CM_WIDTH;
        cm[i * CM_WIDTH + a] = 1;
    }
}

static inline int cm_update(int *cm, uint64_t key) {
    int i;
    int res = 1;
    for (int i = 0; i < CM_DEPTH; i ++) {
        size_t a = hash(key, seeds[i]) % CM_WIDTH;
        cm[i * CM_WIDTH + a] += 2;
        res &= cm[i * CM_WIDTH + a];
    }
    return res;
}

static inline int cm_query(int *cm, uint64_t key) {
    int i;
    int res = 0x7fffffff;
    for (int i = 0; i < CM_DEPTH; i ++) {
        size_t a = hash(key, seeds[i]) % CM_WIDTH;
        res = min(res, cm[i * CM_WIDTH + a]);
    }
    return res >> 1;
}

// use simpling to build a trie
static void sample_trie(uint64_t *arr, size_t n, int *nnode_p, struct node_t *nodes, int *cm) {
    size_t i, j;
    const size_t m = (n+SAMPLE_STRIP-1)/SAMPLE_STRIP;
    uint64_t *sp = (uint64_t *)malloc(sizeof (uint64_t) * m);

    for (i = 0; i < m; i ++) {
        sp[i] = arr[i * SAMPLE_STRIP];
    }
    q_sort(sp, m);
    
    // top 256 nodes in the trie are 0-255
    *nnode_p = 256;

    for (i = SAMPLE_EQUAL; i < m; i ++) {
        uint64_t prefix = 0;
        for (j = i-SAMPLE_EQUAL; j < i; j ++) {
            prefix |= (sp[j] ^ sp[i]);
        }
        int x = 0;
        size_t shift;
        uint64_t key = 0;
        
        for (shift = INITIAL_SHIFT; shift > RADIX_BIT; shift -= RADIX_BIT) {
            // only shift > 0 has children

            // add nodes for equal prefix
            if ((prefix >> shift) & RADIX_MASK) {
                break;
            }
            uint64_t k = (sp[i] >> shift) & RADIX_MASK;

            // expand the subtree of x+k
            key = key | (k << shift);
            if (!nodes[x+k].child) {
                cm_set1(cm, key | shift);

                nodes[x+k].child = *nnode_p;
                *nnode_p += (1<<RADIX_BIT);
                if (__builtin_expect(*nnode_p > NUM_NODE, 0)) {
                    fprintf(stderr, "Too many nodes\n");
                    exit(1);
                }
            }
            x = nodes[x+k].child;
        }
    }
    free(sp);
}

static void build_trie(uint64_t *arr, size_t n, int *nnode_p, struct node_t *nodes, int *cm) {
    sample_trie(arr, n, nnode_p, nodes, cm);

    // insert all keys into the CM sketch
    for (int i = 0; i < n; i ++) {
        uint8_t shift;
        uint64_t key = 0;
        for (shift = INITIAL_SHIFT; shift > 0; shift -= RADIX_BIT) {
            key = key | (arr[i] & ((uint64_t)RADIX_MASK << shift));
            if (!cm_update(cm, key | shift)) {
                break;
            }
        }
    }
}

// use dfs to get the offset of every trie node
static int dfs_getoff(int x, uint64_t key, uint8_t shift, struct node_t *nodes, int *nleaf_p, int *leaves, int *cm, int cur_off) {
    for (uint64_t i = 0; i < (1 << RADIX_BIT); i ++) {
        if (!nodes[x + i].child) {
            int size = cm_query(cm, key | (i << shift) | shift);
            nodes[x + i].begin = cur_off;
            nodes[x + i].end = nodes[x + i].begin;
            nodes[x + i].shift = shift;
            cur_off += size;
            leaves[(*nleaf_p) ++] = x + i;
        }
        else {
            cur_off = dfs_getoff(nodes[x + i].child, key | (i << shift),  shift - RADIX_BIT, nodes, nleaf_p, leaves, cm, cur_off);
        }
    }
    return cur_off;
}

double work(int n) {
    int nnode = 0;
    struct node_t nodes[NUM_NODE];
    memset(nodes, 0, sizeof(nodes));

    uint64_t *arr = (uint64_t *)malloc(n * sizeof(uint64_t));
    int cm[CM_DEPTH * CM_WIDTH] = {0};
    // allocate more memory for cm
    uint64_t *buffer = (uint64_t *)malloc(sizeof (uint64_t) * n * 1.5);
    
    double st_ms, ed_ms;
    GENCLASS gen;
    gen(arr, arr + n);

    build_trie(arr, n, &nnode, nodes, cm);

    // printf("nnode = %d\n", nnode);

    st_ms = gettime_ms();

    /************ two phase sort begin ************/
    int nleaf = 0;
    int leaves[NUM_NODE];

    int max_off = dfs_getoff(0, 0, INITIAL_SHIFT, nodes, &nleaf, leaves, cm, 0);

    // move the number into the correct leaf
    for (int i = 0; i < n; i ++) {
        int x = 0;
        for (uint8_t shift = INITIAL_SHIFT; shift; shift -= RADIX_BIT) {
            const uint8_t radix = (uint8_t)((arr[i] >> shift) & 0xff);
            if (!nodes[x + radix].child) {
                buffer[nodes[x + radix].end ++] = arr[i];
                break;
            }
            else {
                x = nodes[x + radix].child;
            }
        }
    }

    int arr_off = 0;
    // sort each leaf
    for (int i = 0; i < nleaf; i ++) {
        int x = leaves[i];
        int off = nodes[x].begin;
        int size = nodes[x].end - nodes[x].begin;
        memcpy(arr + arr_off, buffer + off, size * sizeof (uint64_t));
        radix_sort(arr + arr_off, buffer + off, size, nodes[x].shift);
        arr_off += size;
    }
    
    /************ two phase sort end ************/
    ed_ms = gettime_ms();

    // verify correctness
    for (int i = 1; i < n; i ++) {
        if (arr[i-1] > arr[i]) {
            printf("Wrong! arr[%d] = %016lx > arr[%d] = %016lx!\n", i-1, arr[i-1], i, arr[i]);
            for (int j = i > 10 ? i - 10 : 0; j < i + 10; j ++) {
                printf("arr[%d] = %016lx\n", j, arr[j]);
            }
            break;
        }
    }
    
    free(buffer);
    free(arr);
    return ed_ms - st_ms;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    int n;
    if (argc < 2) {
        n = 1 << 25;
    }
    else {
        n = atoi(argv[1]);
    }

    int i;
    double total_time = 0;
    for (int iter = 0; iter < N_ITER; iter ++) {
        printf("=");
        fflush(stdout);
        double time_used = work(n);
        total_time += time_used;
    }

    printf("Average Time : %.2lf ms\n", total_time / N_ITER);
    
    std::string filepath(argv[0]);
    int last_slash = filepath.find_last_of('/');
    std::string filename = filepath.substr(last_slash + 1);
    time_t nowtime;
    time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数
    tm* p = localtime(&nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1
    std::string date = std::to_string(p->tm_mon + 1) + std::to_string(p->tm_mday) + "-" + std::to_string(p->tm_hour) + std::to_string(p->tm_min) + std::to_string(p->tm_sec);
    std::string logfile = "../log/" + filename + date +  + ".txt";

    FILE *fout = fopen(logfile.c_str(), "w");
    fprintf(fout, "n = %d\n", n);
    fprintf(fout, "N_ITER = %d\n", N_ITER);
    fprintf(fout, "GENCLASS = %s\n", GENCLASS::name().c_str());
    fprintf(fout, "SAMPLE_STRIP = %d\n", SAMPLE_STRIP);
    fprintf(fout, "SAMPLE_EQUAL = %d\n", SAMPLE_EQUAL);
    fprintf(fout, "Average Time : %.2lf ms\n", total_time / N_ITER);
    fclose(fout);
    return 0;
}
