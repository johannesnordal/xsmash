#ifndef SKETCH_HPP_
#define SKETCH_HPP_

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

struct Sketch
{
    std::string fastx_filename;
    uint32_t k;
    uint32_t c;
    uint32_t s;
    std::vector<uint64_t> min_hash;

    Sketch() = default;

    Sketch(const char *fname)
    {
        std::fstream fs(fname, std::ios::in);
        fs >> fastx_filename >> k >> c >> s;
        std::istream_iterator<uint64_t> start(fs), end;
        min_hash.assign(start, end);
    }
};

#endif
