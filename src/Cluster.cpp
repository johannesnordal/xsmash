#include "Utils.hpp"
#include "khash.h"
#include "kvec.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

// khash.h doesn't play well with std::vector<T> and needs a pointer
// to a heap allocated std::vector<T> so it doesn't segfault, adding unnecessary
// indirection and memory allocation. kvec.h offers a fast vector
// implemented using macros. It creates an anonymous struct which
// needs to be wrapped into a named struct so it can be
// referred to elsewhere, hence the Kvector wrapper.
struct Kvector
{
    kvec_t(uint64_t) vec;
};

KHASH_MAP_INIT_INT64(kvector, Kvector);
KHASH_MAP_INIT_INT64(u64, uint64_t);

using ClusterHashTable = khash_t(kvector)*;
using HashLocator = khash_t(kvector)*;
using MinHash = std::vector<uint64_t>;
using MinHashList = std::vector<MinHash>;
using SketchFileNames = std::vector<std::string>;

struct Clusters
{
    
    ClusterHashTable ctable = nullptr;
    int *p;
    int *r;
    int size;

    static uint64_t limit;

    Clusters(int n) : size{n}
    {
        p = (int*) malloc(sizeof(int) * size);
        r = (int*) malloc(sizeof(int) * size);
        for (int i = 0; i < n; i++)
        {
            p[i] = i;
            r[i] = 0;
        }
    }

    int find(int x)
    {
        while (x != p[x])
        {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    void join(const int x, const int y)
    {
        int xm = find(x);
        int ym = find(y);

        if (xm = ym) return;

        if (r[xm] > r[ym])
        {
            p[ym] = xm;
        }
        else
        {
            p[xm] = ym;
            if (r[xm] == r[ym])
                r[ym]++;
        }
    }

    void build_table()
    {
        if (ctable != nullptr)
        {
            kh_destroy(kvector, ctable);
            ctable = kh_init(kvector);
        }

        int ret;
        khiter_t k;

        for (int x = 0; x < size; x++)
        {
            const int parent = find(x);

            if (k == kh_end(ctable))
            {
                k = kh_put(kvector, ctable, parent, &ret);
                Kvector v;
                kv_init(v.vec);
                kh_value(ctable, k) = v;
            }

            kv_push(uint64_t, kh_value(ctable, k).vec, x);
        }
    }

    ~Clusters()
    {
        free(p);
        free(r);
        kh_destroy(kvector, ctable);
    }
};

HashLocator locate_hashes(MinHashList& min_hash_list)
{
    int ret;
    khiter_t k;
    HashLocator hash_locator = kh_init(kvector);

    for (int i = 0; i < min_hash_list.size(); i++)
    {
        for (auto hash : min_hash_list[i])
        {
            k = kh_get(kvector, hash_locator, hash);

            if (k == kh_end(hash_locator))
            {
                k = kh_put(kvector, hash_locator, hash, &ret);
                Kvector kv;
                kv_init(kv.vec);
                kh_value(hash_locator, k) = kv;
            }

            kv_push(uint64_t, kh_value(hash_locator, k).vec, i);
        }
    }

    return hash_locator;
}

Clusters find_clusters(MinHashList& min_hash_list,
                       HashLocator& hash_locator,
                       const uint64_t limit)
{
    Clusters clusters(min_hash_list.size());

    int ret;
    khiter_t k;

    for (uint64_t i = 0; i < min_hash_list.size(); i++)
    {
        khash_t(u64) *mutual = kh_init(u64);

        for (auto hash : min_hash_list[i])
        {
            k = kh_get(kvector, hash_locator, hash);
            Kvector indices = kh_value(hash_locator, k);

            for (uint64_t j = 0; j < kv_size(indices.vec); j++)
            {

            }
        }
    }

    return clusters;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        exit(1);
    }

    SketchFileNames filenames;
    {
        std::fstream fs(argv[1], std::ios::in);
        std::istream_iterator<std::string> start(fs), end;
        filenames.assign(start, end);
    }

    MinHashList min_hash_list;
    min_hash_list.reserve(filenames.size());

    for (auto& filename : filenames)
    {
        std::fstream fs(filename, std::ios::in);
        std::istream_iterator<uint64_t> start(fs), end;

        // skip k, c, s.
        ++start;
        ++start;
        ++start;

        min_hash_list.push_back({start, end});
    }

    for (auto& min_hash : min_hash_list)
    {
        for (auto hash : min_hash)
        {
            std::cout << hash << "\n";
        }
        std::cout << "\n";
    }
}
