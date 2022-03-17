#ifndef TRIANGULATE_HPP
#define TRIANGULATE_HPP
#include "khash.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

KHASH_MAP_INIT_INT64(vec, std::vector<uint64_t>*);

khash_t(vec)* read_hash_locator(const char *hash_locator_filename)
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
#endif
