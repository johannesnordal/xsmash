#ifndef TRIANGULATE_HPP
#define TRIANGULATE_HPP
#include "khash.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

KHASH_MAP_INIT_INT64(vec, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

using HashLocator = khash_t(vec)*;
using Indices = std::vector<std::pair<std::string, int>>;
using Pair = std::pair<uint64_t, uint64_t>;
using Mutual = std::vector<Pair>;

HashLocator read_hash_locator(const char *hash_locator_filename)
{
    int ret;
    khiter_t k;
    uint64_t hash;
    uint64_t index;
    khash_t(vec) *hash_locator = kh_init(vec);
    std::fstream fs(hash_locator_filename, std::ios::in);
    std::string line;
    while (std::getline(fs, line))
    {
        std::stringstream ss(line);
        ss >> hash;
        k = kh_put(vec, hash_locator, hash, &ret);
        kh_value(hash_locator, k) = new std::vector<uint64_t>;
        while (ss >> index)
            kh_value(hash_locator, k)->push_back(index);
    }
    return hash_locator;
}

Indices read_indices(const char *indices_filename)
{
    int index;
    std::string genome;
    std::string atom;
    std::fstream fs(indices_filename, std::ios::in);
    std::string line;
    Indices indices;

    while (std::getline(fs, line))
    {
        std::stringstream ss(line);
        ss >> index >> genome >> atom;
        if (atom == "NULL")
            indices.push_back(std::make_pair(genome, -1));
        else
            indices.push_back(std::make_pair(genome, atoi(atom.c_str())));
    }

    return indices;
}

Mutual find_mutual(HashLocator hash_locator, uint64_t *min_hash,
        size_t n)
{
    int ret;
    khiter_t k;
    khash_t(u64) *mutual = kh_init(u64);
    for (int i = 0; i < n; i++)
    {
        k = kh_get(vec, hash_locator, min_hash[i]);
        if (k != kh_end(hash_locator))
        {
            auto indx = kh_value(hash_locator, k);
            for (auto j : *indx)
            {
                k = kh_get(u64, mutual, j);
                if (k != kh_end(mutual))
                {
                    kh_value(mutual, k) += 1;
                }
                else
                {
                    k = kh_put(u64, mutual, j, &ret);
                    kh_value(mutual, k) = 1;
                }
            }
        }
    }

    Mutual mut;
    for (k = kh_begin(mutual); k != kh_end(mutual); ++k)
    {
        if (kh_exist(mutual, k))
            mut.push_back(std::make_pair(kh_key(mutual, k), kh_value(mutual, k)));
    }
    kh_destroy(u64, mutual);

    auto cmp = [](const Pair& x, const Pair& y) -> const bool {
        return x.second > y.second;
    };

    std::sort(mut.begin(), mut.end(), cmp);

    return mut;
}

struct Results
{
    uint64_t id;
    uint64_t mutual;
    int atom;
    char genome[1024];
};

void get_results(Results res[5], HashLocator hash_locator, Indices& indices,
        uint64_t *min_hash, size_t size)
{
    auto mutual = find_mutual(hash_locator, min_hash, size);

    for (int i = 0; i < 5 && i < mutual.size(); ++i)
    {
        res[i].id = mutual[i].first;
        res[i].mutual = mutual[i].second;
        memset(res[i].genome, 0, sizeof(res[i].genome));
        strcpy(res[i].genome, indices[res[i].id].first.c_str());
        res[i].atom = indices[res[i].id].second;
    }
}
#endif
