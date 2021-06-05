#pragma once

class RandomGenerator
{
public:
    RandomGenerator();
    ~RandomGenerator();

    void SetRandomSeed(unsigned int random_seed);
    unsigned int GetRandomNumber();

private:

    unsigned int m_seed_x;
    unsigned int m_seed_y;
};

