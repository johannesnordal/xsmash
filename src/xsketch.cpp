#include "kseq.h"
#include "bifrost/src/KmerHashTable.hpp"
#include "bifrost/src/Kmer.hpp"
#include "bifrost/src/KmerIterator.hpp"

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

inline const size_t CandidateSet::update(const Kmer kmer)
{
    it = candidates.find(kmer);
    return it != candidates.end() ? ++(*it) : candidates.insert(kmer, 1).second;
}

inline void CandidateSet::erase(const Kmer kmer)
{
    candidates.erase(kmer);
}

void print_usage(const char *name)
{
    static char const s[] = "Usage: %s [options] <in.fastq>\n\n"
        "Options:\n"
        "  -k    Size of kmers [default: 21]\n"
        "  -c    Candidate set limit [default: 1]\n";
    printf(s, name);
}

static constexpr uint64_t MAX_HASH = 9999999776999205UL;
static uint32_t k = 21;
static uint32_t c = 1;

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "k:c:")) != -1)
    {
        switch (opt)
        {
            case 'k':
                k = atoi(optarg);
                break;
            case 'c':
                c = atoi(optarg);
                break;
        }
    }

    Kmer::set_k(k);

    gzFile fp = gzopen(argv[optind], "r");
    kseq_t *seq = kseq_init(fp);

    CandidateSet set;
    KmerIterator it_end;
    std::vector<uint64_t> min_hash;

    int seq_len;
    while ((seq_len = kseq_read(seq)) >= 0)
    {
        KmerIterator it(seq->seq.s);

        for (; it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (kmer.hash() < MAX_HASH)
            {
                if (c == set.update(kmer))
                {
                    min_hash.push_back(kmer.hash());
                }
            }
        }
    }

    std::sort(min_hash.begin(), min_hash.end());

    ofstream file;
    file.open(std::string(argv[optind]) + ".xsketch");

    for (auto x : min_hash)
    {
        file << x << "\n";
    }

    file.close();

    kseq_destroy(seq);
    gzclose(fp);

    return 0;
}
