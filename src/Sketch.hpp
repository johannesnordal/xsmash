#ifndef SKETCH_HPP_
#define SKETCH_HPP_

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

std::string get_filename_from_path(std::string path)
{
    size_t last_slash_index = path.find_last_of("\\/");
    if (std::string::npos != last_slash_index)
        path.erase(0, last_slash_index + 1);
    return path;
}

struct SketchOpt
{
    uint64_t max_hash = 0;
    uint32_t k = 21;
    uint32_t c = 1;
    uint32_t s = 1000;
};

struct Sketch
{
    std::vector<uint64_t> min_hash;
    std::string fastx_filename;
    uint64_t max_hash;
    uint32_t k;
    uint32_t c;
    uint32_t s;

    Sketch() = default;

    Sketch(SketchOpt& opt, const char *fname) :
        fastx_filename{fname},
        max_hash{opt.max_hash},
        k{opt.k},
        c{opt.c},
        s{opt.s}
    {
        if (max_hash)
            xsketch();
        else
            sketch();
    }

    void write(std::string& dirpath)
    {
        std::string extension = max_hash ? ".xsketch" : ".sketch";
        std::ofstream file;
        file.open(dirpath + get_filename_from_path(fastx_filename) + extension);
        file << fastx_filename << '\n';
        file << k << '\n';
        file << c << '\n';
        file << s << '\n';
        for (auto hash : min_hash)
            file << hash << '\n';
        file.close();
    }

    Sketch(const char *sketch_filename)
    {
        std::fstream fs(sketch_filename, std::ios::in);
        fs >> fastx_filename;
        fs >> k >> c >> s;
        std::istream_iterator<uint64_t> start(fs), end;
        min_hash.assign(start, end);
    }

    void sketch();
    void xsketch();

    private:

    void sink();
};

#endif
