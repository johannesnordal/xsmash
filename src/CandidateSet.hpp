#ifndef CANDIDATESET_HPP_
#define CANDIDATESET_HPP_

#include "bifrost/src/KmerHashTable.hpp"
#include "bifrost/src/Kmer.hpp"

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

#endif
