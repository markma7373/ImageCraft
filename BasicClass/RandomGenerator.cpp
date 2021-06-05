#include "stdafx.h"
#include "RandomGenerator.h"

RandomGenerator::RandomGenerator()
{
    m_seed_x = 1;
    m_seed_y = 2;
}

RandomGenerator::~RandomGenerator()
{

}

void RandomGenerator::SetRandomSeed(unsigned int random_seed)
{
    m_seed_x = random_seed | 1;
    m_seed_y = random_seed | 2;
}

unsigned int RandomGenerator::GetRandomNumber()
{
    m_seed_x = (18000 * (m_seed_x & 0xffff)) + (m_seed_x >> 16);
    m_seed_y = (30903 * (m_seed_y & 0xffff)) + (m_seed_y >> 16);

    return (m_seed_x << 16) + (m_seed_y & 0xffff);
}

