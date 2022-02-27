#include <iostream>
#include <zlib.h>
#include "../src/kseq.h"

KSEQ_INIT(gzFile, gzread);

int main(int argc, char** argv)
{
    gzFile fp = gzopen(argv[1], "r");
    kseq_t *seq = kseq_init(fp);

    int seq_len;
    while ((seq_len = kseq_read(seq)) >= 0)
    {
        if (seq->name.l)
            std::cout << seq->name.s << "\n";

        if (seq->comment.l)
            std::cout << seq->comment.s << "\n";

        std::cout << "\n";
    }
}
