#include "khash.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

KHASH_MAP_INIT_INT64(vec, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

using ClusterMembers = std::vector<uint64_t>*;
using ClusterMembersTable = khash_t(vec)*;
using FastxFilenames = std::vector<std::string>;
using HashLocator = khash_t(vec)*;
using MinHash = std::vector<uint64_t>;
using MinHashList = std::vector<MinHash>;
using SketchFilenames = std::vector<std::string>;

struct DisjointSets
{
    int *p;
    int *r;
    int size;

    DisjointSets(int n) : size{n}
    {
        p = (int*) malloc(sizeof(int) * size);
        r = (int*) malloc(sizeof(int) * size);
        for (int i = 0; i < n; i++)
        {
            p[i] = i;
            r[i] = 0;
        }
    }

    ~DisjointSets()
    {
        // free(p);
        // free(r);
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

    void join(const int x, const int y)
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
};

struct Clusters
{
    private:

    int *m_leader;
    int m_size;

    public:

    ClusterMembersTable ctable = kh_init(vec);

    Clusters(DisjointSets sets) : m_leader{new int[sets.size]}, m_size{sets.size}
    {
        int ret;
        khiter_t k;

        for (int x = 0; x < sets.size; x++)
        {
            m_leader[x] = sets.find(x);

            k = kh_get(vec, ctable, m_leader[x]);

            if (k == kh_end(ctable))
            {
                k = kh_put(vec, ctable, m_leader[x], &ret);
                kh_value(ctable, k) = new std::vector<uint64_t>;
            }

            kh_value(ctable, k)->push_back(x);
        }
    }

    inline int leader(int x) const { return m_leader[x]; }
    inline int size() const { return m_size; }

    ClusterMembers members(int x)
    {
        khiter_t k = kh_get(vec, ctable, m_leader[x]);
        return kh_value(ctable, k);
    }
};

Clusters find_clusters(MinHashList& min_hash_list, HashLocator& hash_locator,
        const uint64_t limit)
{
    DisjointSets sets(min_hash_list.size());

    int ret;
    khiter_t k;

    for (int i = 0; i < min_hash_list.size(); i++)
    {
        khash_t(u64) *mutual = kh_init(u64);

        for (auto hash : min_hash_list[i])
        {
            k = kh_get(vec, hash_locator, hash);

            for (auto j : *kh_value(hash_locator, k))
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

        for (k = kh_begin(mutual); k != kh_end(mutual); ++k)
        {
            if (kh_exist(mutual, k))
            {
                const auto j = kh_key(mutual, k);
                const auto c = kh_value(mutual, k);

                if (c > limit && sets.find(i) != sets.find(j))
                {
                    sets.join(i, j);
                }
            }
        }

        kh_destroy(u64, mutual);
    }

    Clusters clusters(sets);

    return clusters;
}

HashLocator locate_hashes(MinHashList& min_hash_list)
{
    int ret;
    khiter_t k;
    HashLocator hash_locator = kh_init(vec);

    for (int i = 0; i < min_hash_list.size(); i++)
    {
        for (auto hash : min_hash_list[i])
        {
            k = kh_get(vec, hash_locator, hash);

            if (k == kh_end(hash_locator))
            {
                k = kh_put(vec, hash_locator, hash, &ret);
                kh_value(hash_locator, k) = new std::vector<uint64_t>;
            }

            kh_value(hash_locator, k)->push_back(i);
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

    uint64_t limit = 990;

    SketchFilenames sketch_filenames;
    {
        std::fstream fs(argv[1], std::ios::in);
        std::istream_iterator<std::string> start(fs), end;
        sketch_filenames.assign(start, end);
    }

    FastxFilenames fastx_filenames;
    fastx_filenames.reserve(sketch_filenames.size());

    MinHashList min_hash_list;
    min_hash_list.reserve(sketch_filenames.size());

    for (auto& filename : sketch_filenames)
    {
        std::fstream fs(filename, std::ios::in);

        std::string fastx_filename;
        fs >> fastx_filename;
        fastx_filenames.push_back(std::move(fastx_filename));

        std::istream_iterator<uint64_t> start(fs), end;

        // skip k, c, s.
        ++start;
        ++start;
        ++start;

        min_hash_list.push_back({start, end});
    }

    auto hash_locator = locate_hashes(min_hash_list);
    auto clusters = find_clusters(min_hash_list, hash_locator, limit);

    {
        std::ofstream fs("indicies");

        for (int x = 0; x < min_hash_list.size(); x++)
        {
            auto leader = clusters.leader(x);
            if (clusters.members(leader)->size() > 1)
            {
                fs << x << " " << fastx_filenames[x] << " " << leader << "\n";
            }
            else
            {
                fs << x << " " << fastx_filenames[x] << " NULL\n";
            }
        }

        fs.close();
    }

    {
        std::ofstream fs("hash_locator");

        for (khiter_t k = kh_begin(hash_locator); k != kh_end(hash_locator); ++k)
        {
            if (kh_exist(hash_locator, k))
            {
                auto hash = kh_key(hash_locator, k);
                auto indx = kh_value(hash_locator, k);

                fs << hash << " ";
                for (auto i : *indx)
                {
                    fs << i << " ";
                }
                fs << "\n";
            }
        }

        fs.close();
    }

    {
        std::string dirname = "atoms";
        int res = mkdir(dirname.c_str(), 0777);

        for (khiter_t k = kh_begin(clusters.ctable); k != kh_end(clusters.ctable); ++k)
        {
            if (kh_exist(clusters.ctable, k))
            {
                auto leader = kh_key(clusters.ctable, k);
                auto members = kh_value(clusters.ctable, k);

                if (members->size() > 1)
                {
                    std::ofstream fs(dirname + "/" + std::to_string(leader));

                    for (auto x : *members)
                    {
                        fs << fastx_filenames[x] << "\n";
                    }

                    fs.close();
                }
            }
        }
    }
}
