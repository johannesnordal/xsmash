#include "kseq.h"
#include "bifrost/src/Kmer.hpp"
#include "bifrost/src/KmerIterator.hpp"
#include "bifrost/src/KmerHashTable.hpp"

#include <fstream>
#include <iostream>
#include <vector>

#include <zlib.h>
#include <getopt.h>

KSEQ_INIT(gzFile, gzread);

uint32_t k = 21;
uint32_t c = 1;
uint32_t s = 1000;
uint64_t X = 9999999776999205UL;
bool x = false;
bool t = false;

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

void sink(uint64_t *min_hash)
{
    size_t i = 0;
    size_t j = 1;

    while (j < s)
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

void sketch(const char* fname)
{
    gzFile fp = gzopen(fname, "r");
    kseq_t *seq = kseq_init(fp);

    CandidateSet set;
    KmerIterator it_end;
    uint64_t min_hash[s];

    int i = 0;
    int seq_len;
    while ((seq_len = kseq_read(seq)) >= 0)
    {
        KmerIterator it(seq->seq.s);

        for (; i < s && it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (c == set.update(kmer))
            {
                min_hash[i] = kmer.hash();
                i++;
            }
        }

        if (i == s)
            std::make_heap(min_hash, min_hash + i);

        for (; it != it_end; ++it)
        {
            const Kmer kmer = it->first.rep();

            if (min_hash[0] > kmer.hash())
            {
                if (c == set.update(kmer))
                {
                    min_hash[0] = kmer.hash();
                    sink(min_hash);
                }
            }
        }
    }

    std::sort(min_hash, min_hash + i);

    ofstream file;
    file.open(std::string(fname) + ".sketch");

    file << k << "\n";
    file << c << "\n";
    file << i << "\n";

    for (int j = 0; j < i; j++)
    {
        file << min_hash[j] << "\n";
    }

    file.close();

    kseq_destroy(seq);
    gzclose(fp);
}

void xsketch(const char *fname)
{
    gzFile fp = gzopen(fname, "r");
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

            if (kmer.hash() < X)
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
    file.open(std::string(fname) + ".xsketch");

    file << k << "\n";
    file << c << "\n";
    file << min_hash.size() << "\n";

    for (auto x : min_hash)
    {
        file << x << "\n";
    }

    file.close();

    kseq_destroy(seq);
    gzclose(fp);
}

void print_usage(const char *name)
{
    static char const s[] = "Usage: %s [options] <in.fasta | in.fastq>\n\n"
        "Options:\n"
        "  -k    Size of kmers [default: 21].\n"
        "  -c    Candidate set limit [default: 1].\n"
        "  -s    Size of min hash [default: 1000]. Ignored with -x and -X options.\n"
        "  -x    Include all hashes that have a value lower than 9999999776999205UL.\n"
        "  -X    Include all hashes that have a value lower than X.\n";
    printf(s, name);
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "k:c:s:xt")) != -1)
    {
        switch (opt)
        {
            case 'k':
                k = atoi(optarg);
                break;
            case 'c':
                c = atoi(optarg);
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 't':
                t = true;
            case 'x':
                x = true;
                break;
            case 'X':
                x = true;
                X = atoi(optarg);
                break;
        }
    }

    Kmer::set_k(k);

    if (!x)
    {
        for (; optind < argc; optind++)
            sketch(argv[optind]);
    }
    else
    {
        for (; optind < argc; optind++)
            xsketch(argv[optind]);
    }

    return 0;
}