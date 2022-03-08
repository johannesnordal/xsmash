#include "Sketch.hpp"
#include "khash.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

KHASH_MAP_INIT_INT64(vec, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

struct Clusters
{
    private:

    std::uint64_t *cluster_id;
    std::uint64_t n;

    std::uint64_t find(std::uint64_t x)
    {
        while (x != cluster_id[x])
        {
            cluster_id[x] = cluster_id[cluster_id[x]];
            x = cluster_id[x];
        }
        return x;
    }

    void join(std::uint64_t *rank, std::uint64_t x, std::uint64_t y)
    {
        std::uint64_t p = find(x);
        std::uint64_t q = find(y);

        if (p == q) return;

        if (rank[p] > rank[q])
        {
            cluster_id[q] = p;
        }
        else
        {
            cluster_id[p] = q;
            if (rank[p] == rank[q])
                rank[q]++;
        }
    }

    void find_clusters(const std::vector<std::vector<uint64_t>>& min_hash_list,
            khash_t(vec) *hash_locator, std::uint64_t limit)
    {
        int ret;
        khiter_t k;
        std::uint64_t *cluster_rank = (std::uint64_t*) calloc(n, sizeof(std::uint64_t));
        for (std::uint64_t i = 0; i < min_hash_list.size(); i++)
        {
            khash_t(u64) *mutual = kh_init(u64);
            for (auto hash : min_hash_list[i])
            {
                for (auto j : *kh_value(hash_locator, kh_get(vec, hash_locator, hash)))
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
                    auto j = kh_key(mutual, k);
                    if (kh_value(mutual, k) > limit && find(i) != find(j))
                    {
                        join(cluster_rank, i, j);
                    }
                }
            }
            kh_destroy(u64, mutual);
        }
        free(cluster_rank);
    }

    void create_id_to_members_map()
    {
        int ret;
        khiter_t k;
        for (std::uint64_t x = 0; x < n; x++)
        {
            cluster_id[x] = find(x);
            k = kh_get(vec, ctable, cluster_id[x]);
            if (k == kh_end(ctable))
            {
                k = kh_put(vec, ctable, cluster_id[x], &ret);
                kh_value(ctable, k) = new std::vector<uint64_t>{ x };
            }
            else
            {
                kh_value(ctable, k)->push_back(x);
            }
        }
    }

    public:

    khash_t(vec) *ctable = kh_init(vec);

    Clusters(const std::vector<std::vector<std::uint64_t>>& min_hash_list,
            khash_t(vec) *hash_locator, std::uint64_t limit)
    {
        n = min_hash_list.size();
        cluster_id = (std::uint64_t*) malloc(sizeof(std::uint64_t) * n);
        for (std::uint64_t i = 0; i < n; i++)
            cluster_id[i] = i;
        find_clusters(min_hash_list, hash_locator, limit);
        create_id_to_members_map();
    }

    ~Clusters()
    {
        for (khiter_t k = kh_begin(ctable); k != kh_end(ctable); ++k)
        {
            if (kh_exist(ctable, k))
                delete kh_value(ctable, k);
        }
        kh_destroy(vec, ctable);
        free(cluster_id);
    }

    inline int size() const { return n; }
    inline int id(int x) const { return cluster_id[x]; }

    std::vector<std::uint64_t>& members(int x)
    {
        return *kh_value(ctable, kh_get(vec, ctable, cluster_id[x]));
    }
};

khash_t(vec) *locate_hashes(const std::vector<std::vector<uint64_t>>& min_hash_list)
{
    int ret;
    khiter_t k;
    khash_t(vec) *hash_locator = kh_init(vec);
    for (std::uint64_t i = 0; i < min_hash_list.size(); i++)
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

void print_usage(const char *name)
{
    static const char s[] = "\nUsage: %s [options] <file>\n\n"
        "The input <file> should contain a list of pathnames to sketches. All\n"
        "sketches should have been sketched using the same parameters, i.e.,\n"
        "the same kmer size etc.\n\n"
        "Options:\n"
        "  -d    Path to output directory.\n"
        "  -p    The ratio of mutual hashes two sketches have to share to be\n"
        "        combined into the same cluster [default: ~0.99].\n"
        "  -l    Set exact number for limit for mutual hashes, overriding -p.\n\n";
    printf(s, name);
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(1);
    }

    std::string dirpath = "";
    double ratio = 0.99;
    std::uint64_t limit = 0;

    int opt;
    while ((opt = getopt(argc, argv, "d:p:l:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                dirpath = optarg;
                if (dirpath.back() != '/')
                    dirpath += '/';
                break;
            case 'p':
                ratio = atof(optarg);
                break;
            case 'l':
                limit = atoi(optarg);
                break;
        }
    }

    std::vector<std::string> sketch_filenames;
    {
        std::fstream fs(argv[optind], std::ios::in);
        std::istream_iterator<std::string> start(fs), end;
        sketch_filenames.assign(start, end);
    }

    std::vector<std::string> fastx_filenames;
    fastx_filenames.reserve(sketch_filenames.size());

    std::vector<std::vector<std::uint64_t>> min_hash_list;
    min_hash_list.reserve(sketch_filenames.size());

    {
        Sketch sketch;
        for (auto& filename : sketch_filenames)
        {
            sketch = Sketch(filename.c_str());
            fastx_filenames.push_back(std::move(sketch.fastx_filename));
            min_hash_list.push_back(std::move(sketch.min_hash));
        }

        if (!limit)
            limit = ((double) sketch.s) * ratio;
    }

    auto hash_locator = locate_hashes(min_hash_list);
    Clusters clusters(min_hash_list, hash_locator, limit);

    std::ofstream fs(dirpath + "indices");
    for (std::uint64_t x = 0; x < min_hash_list.size(); x++)
    {
        auto id = clusters.id(x);
        if (clusters.members(id).size() > 1)
            fs << x << ' ' << fastx_filenames[x] << ' ' << id << '\n';
        else
            fs << x << ' ' << fastx_filenames[x] << " NULL\n";
    }
    fs.close();

    uint64_t max_hash = 0;
    fs = std::ofstream(dirpath + "hash_locator");
    for (khiter_t k = kh_begin(hash_locator); k != kh_end(hash_locator); ++k)
    {
        if (kh_exist(hash_locator, k))
        {
            max_hash = std::max(kh_key(hash_locator, k), max_hash);
            fs << kh_key(hash_locator, k) << ' ';
            for (auto i : *kh_value(hash_locator, k))
                fs << i << ' ';
            fs << '\n';
        }
    }
    fs.close();

    fs = std::ofstream(dirpath + "max_hash");
    fs << max_hash << '\n';
    fs.close();

    std::string dirname = dirpath + "atoms";
    mkdir(dirname.c_str(), 0777);
    for (khiter_t k = kh_begin(clusters.ctable); k != kh_end(clusters.ctable); ++k)
    {
        if (kh_exist(clusters.ctable, k))
        {
            if (kh_value(clusters.ctable, k)->size() > 1)
            {
                fs = std::ofstream(dirname + '/' +
                        std::to_string(kh_key(clusters.ctable, k)));
                for (auto x : *kh_value(clusters.ctable, k))
                    fs << fastx_filenames[x] << '\n';
                fs.close();
            }
        }
    }

    for (khiter_t k = kh_begin(hash_locator); k != kh_end(hash_locator); ++k)
    {
        if (kh_exist(hash_locator, k))
            delete kh_value(hash_locator, k);
    }
    kh_destroy(vec, hash_locator);
}
