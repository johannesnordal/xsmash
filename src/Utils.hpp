#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <fstream>
#include <iostream>

struct Sketch
{
    uint32_t k;
    uint32_t c;
    uint32_t s;
    uint64_t *min_hash;

    Sketch(const char *fname)
    {
        std::ifstream file(fname, std::ios::in);

        file >> k >> c >> s;

        min_hash = new uint64_t[s];

        for (int i = 0; i < s; i++)
        {
            file >> min_hash[i];
        }
    }
};

#endif
