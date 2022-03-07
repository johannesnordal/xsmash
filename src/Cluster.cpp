#include "Sketch.hpp"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

using HashLocator = std::unordered_map<std::uint64_t, std::vector<std::uint64_t>>;
using MinHashList = std::vector<std::vector<std::uint64_t>>;

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

    void find_clusters(const MinHashList& min_hash_list,
            const HashLocator& hash_locator, std::uint64_t limit)
    {
        std::uint64_t *cluster_rank = (std::uint64_t*) calloc(n, sizeof(std::uint64_t));
        for (std::uint64_t i = 0; i < min_hash_list.size(); i++)
        {
            std::unordered_map<std::uint64_t, std::uint64_t> mutual;
            for (auto hash : min_hash_list[i])
            {
                for (auto j : (hash_locator.find(hash))->second)
                {
                    auto it = mutual.find(j);
                    if (it != mutual.end())
                        mutual.find(j)->second++;
                    else
                        mutual.insert({j, 1});
                }
            }
            for (auto& entry : mutual)
            {
                if (entry.second > limit && find(i) != find(entry.first))
                    join(cluster_rank, i, entry.first);
            }
        }
        free(cluster_rank);
    }

    void create_id_to_members_map()
    {
        for (std::uint64_t x = 0; x < n; x++)
        {
            cluster_id[x] = find(x);
            auto it = ctable.find(cluster_id[x]);
            if (it == ctable.end())
                ctable.insert({ cluster_id[x], { x }});
            else
                (it->second).push_back(x);
        }
    }

    public:

    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> ctable;

    Clusters(const std::vector<std::vector<std::uint64_t>>& min_hash_list,
            const HashLocator& hash_locator, std::uint64_t limit)
    {
        n = min_hash_list.size();
        cluster_id = (std::uint64_t*) malloc(sizeof(std::uint64_t) * n);
        for (std::uint64_t i = 0; i < n; i++)
            cluster_id[i] = i;
        find_clusters(min_hash_list, hash_locator, limit);
        create_id_to_members_map();
    }

    ~Clusters() { free(cluster_id); }

    inline int size() const { return n; }
    inline int id(int x) const { return cluster_id[x]; }

    std::vector<std::uint64_t>& members(int x)
    {
        return ctable.find(cluster_id[x])->second;
    }
};

HashLocator locate_hashes(const MinHashList& min_hash_list)
{
    HashLocator hash_locator;
    for (std::uint64_t i = 0; i < min_hash_list.size(); i++)
    {
        for (auto hash : min_hash_list[i])
        {
            auto it = hash_locator.find(hash);
            if (it == hash_locator.end())
                hash_locator.insert({ hash, { i } });
            else
                (it->second).push_back(i);
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
            fs << x << " " << fastx_filenames[x] << " " << id << "\n";
        else
            fs << x << " " << fastx_filenames[x] << " NULL\n";
    }
    fs.close();

    fs = std::ofstream(dirpath + "hash_locator");
    for (auto& entry : hash_locator)
    {
        fs << entry.first << ' ';
        for (auto i : entry.second)
            fs << i << ' ';
        fs << '\n';
    }
    fs.close();

    std::string dirname = dirpath + "atoms";
    mkdir(dirname.c_str(), 0777);
    for (auto& entry : clusters.ctable)
    {
        if (entry.second.size() > 1)
        {
            fs = std::ofstream(dirname + "/" + std::to_string(entry.first));
            for (auto x : entry.second)
                fs << fastx_filenames[x] << '\n';
            fs.close();
        }
    }
}
