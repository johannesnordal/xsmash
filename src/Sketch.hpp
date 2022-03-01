#ifndef SKETCH_HPP_
#define SKETCH_HPP_

#include <fstream>
#include <iostream>

struct Sketch
{
    std::string fastx_filename;
    uint64_t *min_hash;
    uint32_t k;
    uint32_t c;
    uint32_t s;

    Sketch(const char *fname)
    {
        std::ifstream file(fname, std::ios::in);

        file >> fastx_filename >> k >> c >> s;

        min_hash = (uint64_t*) malloc(sizeof(uint64_t) * s);

        for (int i = 0; i < s; i++)
        {
            file >> min_hash[i];
        }
    }

    void destroy()
    {
        free(min_hash);
    }
};

#endif
