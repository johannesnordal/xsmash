#include "Utils.hpp"
#include "khash.h"
#include "kvec.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

uint32_t k;
uint32_t c;
uint32_t s;
uint64_t lim = 990;
bool validate = false;

struct UnionFind
{
    int *p;
    int *r;
    int size;

    UnionFind(int n) : size{n}
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

    void merge(const int x, const int y)
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

    ~UnionFind()
    {
        free(p);
        free(r);
    }
};

struct Kvec
{
    kvec_t(uint64_t) kvec_array;
};

KHASH_MAP_INIT_INT64(list, Kvec);

khash_t(list) *make_hash_locator(std::vector<uint64_t*>& sketches)
{
    int ret;
    khiter_t k;
    khash_t(list) *hash_locator = kh_init(list);

    for (int i = 0; i < sketches.size(); i++)
    {
        for (int j = 0; j < s; j++)
        {
            k = kh_get(list, hash_locator, sketches[i][j]);

            if (k == kh_end(hash_locator))
            {
                k = kh_put(list, hash_locator, sketches[i][j], &ret);
                Kvec v;
                kv_init(v.kvec_array);
                kh_value(hash_locator, k) = v;
            }

            kv_push(uint64_t, kh_value(hash_locator, k).kvec_array, i);
        }
    }

    return hash_locator;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        exit(1);
    }

    std::fstream fin(argv[1], std::ios::in);

    std::string fname;
    std::getline(fin, fname);

    std::vector<uint64_t*> sketches;
    {
        Sketch sketch(fname.c_str());
        k = sketch.k;
        c = sketch.c;
        s = sketch.s;

        sketches.push_back(sketch.min_hash);

        if (!validate)
        {
            while (std::getline(fin, fname))
            {
                sketch = Sketch(fname.c_str());
                sketches.push_back(sketch.min_hash);
            }
        }
        else
        {
            while (std::getline(fin, fname))
            {
                sketch = Sketch(fname.c_str());

                if (k != sketch.k)
                {
                    std::cerr << "mismatched kmer size\n";
                    exit(1);
                }

                if (c != sketch.c)
                {
                    std::cerr << "mismatched candiate limit\n";
                    exit(1);
                }

                if (s != sketch.s)
                {
                    std::cerr << "mismatched min hash size\n";
                    exit(1);
                }

                sketches.push_back(sketch.min_hash);
            }
        }
    }

    auto hash_locator = make_hash_locator(sketches);
}
