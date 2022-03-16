#include "Sketch.hpp"

#include "../local/include/bifrost/kseq.h"
#include "../local/include/bifrost/Kmer.hpp"
#include "../local/include/bifrost/KmerIterator.hpp"
#include "../local/include/bifrost/KmerHashTable.hpp"

#include <fstream>
#include <iostream>
#include <vector>

#include <zlib.h>
#include <getopt.h>

KSEQ_INIT(gzFile, gzread);

struct CandidateSet {
    const size_t update(const Kmer);
    void erase(const Kmer);

    private:

    KmerHashTable<uint32_t> candidates;
    KmerHashTable<uint32_t>::iterator it;
};

const size_t CandidateSet::update(const Kmer kmer)
{
    it = candidates.find(kmer);
    return it != candidates.end() ? ++(*it) : candidates.insert(kmer, 1).second;
}

void CandidateSet::erase(const Kmer kmer)
{
    candidates.erase(kmer);
}

void Sketch::sink()
{
    size_t i = 0;
    size_t j = 1;

    while (j < min_hash.size())
    {
        const size_t temp = j + 1;
        if (temp < s && min_hash[temp] > min_hash[j])
            j = temp;

        if (min_hash[i] > min_hash[j])
            return;

        const uint64_t hash = min_hash[i];
        min_hash[i] = min_hash[j];
        min_hash[j] = hash;

        i = j;
        j = j << 1;
    }
}

void Sketch::sketch()
{
    gzFile fp = gzopen(fastx_filename.c_str(), "r");
    kseq_t *seq = kseq_init(fp);

    CandidateSet set;
    KmerIterator it_end;
    min_hash.reserve(s);

    int seq_len;
    while ((seq_len = kseq_read(seq)) >= 0)
    {
        KmerIterator it(seq->seq.s);

        for (; min_hash.size() < s && it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (c == set.update(kmer))
            {
                min_hash.push_back(kmer.hash());
            }
        }

        if (min_hash.size() == s)
            std::make_heap(min_hash.begin(), min_hash.end());

        for (; it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (min_hash[0] > kmer.hash())
            {
                if (c == set.update(kmer))
                {
                    min_hash[0] = kmer.hash();
                    sink();
                }
            }
        }
    }

    std::sort(min_hash.begin(), min_hash.end());

    kseq_destroy(seq);
    gzclose(fp);
}

void Sketch::xsketch()
{
    gzFile fp = gzopen(fastx_filename.c_str(), "r");
    kseq_t *seq = kseq_init(fp);

    CandidateSet set;
    KmerIterator it_end;

    int seq_len;
    while ((seq_len = kseq_read(seq)) >= 0)
    {
        KmerIterator it(seq->seq.s);

        for (; it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (kmer.hash() < max_hash)
            {
                if (c == set.update(kmer))
                {
                    min_hash.push_back(kmer.hash());
                }
            }
        }
    }

    std::sort(min_hash.begin(), min_hash.end());

    kseq_destroy(seq);
    gzclose(fp);
}

void print_usage(const char *name)
{
    static char const s[] =
        "\nUsage: %s [options] (<in.fasta> | <in.fastq>) ...\n\n"
        "Options:\n"
        "  -k    Size of kmers [default: 21].\n"
        "  -c    Candidate set limit [default: 1].\n"
        "  -s    Size of min hash [default: 1000]. Ignored with -x and"
            "-X options.\n"
        "  -x    Include all hashes that have a value lower than"
            "9999999776999205UL.\n"
        "  -X    Include all hashes that have a value lower than X.\n\n";
    printf(s, name);
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(1);
    }

    SketchOpt sketch_opt;
    std::string dirpath = "";
    std::string batch_file = "";

    int opt;
    while ((opt = getopt(argc, argv, "d:f:k:c:s:x:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                dirpath = optarg;
                if (dirpath.back() != '/')
                    dirpath += '/';
                break;
            case 'f':
                batch_file = optarg;
                break;
            case 'k':
                sketch_opt.k = atoi(optarg);
                break;
            case 'c':
                sketch_opt.c = atoi(optarg);
                break;
            case 's':
                sketch_opt.s = atoi(optarg);
                break;
            case 'x':
                sketch_opt.max_hash = atol(optarg);
                break;
        }
    }

    Kmer::set_k(sketch_opt.k);

    if (batch_file == "")
    {
        for (; optind < argc; optind++)
            Sketch(sketch_opt, argv[optind]).write(dirpath);
    }
    else
    {
        std::fstream fs(batch_file, std::ios::in);
        std::string filename;
        while (std::getline(fs, filename))
            Sketch(sketch_opt, filename.c_str()).write(dirpath);
        fs.close();
    }

    return 0;
}
