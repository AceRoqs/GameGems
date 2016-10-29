#ifndef MERSENNE_H
#define MERSENNE_H

class MersenneTwister
{
    int m_ix = 0;

    void Regenerate();

public:
    MersenneTwister();

    uint32_t Rand();
    uint64_t Rand64();
};

#endif

