#include <base/math.hpp>
#include <algorithm>
#include <numeric>
#include <ctime> // time()

#include "CNoise.hpp"

CPerlin::CPerlin(sf::Uint32 seed) noexcept
: m_Engine(seed)
{
	m_Seed = seed;
    auto mid_range = m_aNumsPerlin.begin() + 256;

    std::iota(m_aNumsPerlin.begin(), mid_range, 0); //Generate sequential numbers in the lower half
#if defined(_MSC_VER) && _MSC_VER <= 1600
    // Visual C++ 2010 or lower haven't std::shuffle implemented.
    // Because not use the URNG this 'workaround' break shared seeds.
    std::srand(m_Seed);
    std::random_shuffle(m_aNumsPerlin.begin(), mid_range);
#else
    std::shuffle(m_aNumsPerlin.begin(), mid_range, m_Engine); //Shuffle the lower half
#endif
    std::copy(m_aNumsPerlin.begin(), mid_range, mid_range); //Copy the lower half to the upper half
    //p now has the numbers 0-255, shuffled, and duplicated
}

int CPerlin::GetURandom(int Min, int Max) noexcept
{
	std::uniform_int_distribution<int> uni(Min,Max);
	return uni(m_Engine);
}

double CPerlin::Noise(double x, double y, double z) const noexcept
{
    //See here for algorithm: http://cs.nyu.edu/~perlin/noise/

    const sf::Int32 X = static_cast<sf::Int32>(std::floor(x)) & 0xFF;
    const sf::Int32 Y = static_cast<sf::Int32>(std::floor(y)) & 0xFF;
    const sf::Int32 Z = static_cast<sf::Int32>(std::floor(z)) & 0xFF;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    const double u = upm::fade(x);
    const double v = upm::fade(y);
    const double w = upm::fade(z);

    const int A = m_aNumsPerlin[X] + Y;
    const int AA = m_aNumsPerlin[A] + Z;
    const int AB = m_aNumsPerlin[A + 1] + Z;
    const int B = m_aNumsPerlin[X + 1] + Y;
    const int BA = m_aNumsPerlin[B] + Z;
    const int BB = m_aNumsPerlin[B + 1] + Z;

    const int PAA = m_aNumsPerlin[AA];
    const int PBA = m_aNumsPerlin[BA];
    const int PAB = m_aNumsPerlin[AB];
    const int PBB = m_aNumsPerlin[BB];
    const int PAA1 = m_aNumsPerlin[AA + 1];
    const int PBA1 = m_aNumsPerlin[BA + 1];
    const int PAB1 = m_aNumsPerlin[AB + 1];
    const int PBB1 = m_aNumsPerlin[BB + 1];

    const double a = upm::lerp(v,
    	upm::lerp(u, upm::grad(PAA, x, y, z), upm::grad(PBA, x-1, y, z)),
		upm::lerp(u, upm::grad(PAB, x, y-1, z), upm::grad(PBB, x-1, y-1, z))
    );

    const double b = upm::lerp(v,
    		upm::lerp(u, upm::grad(PAA1, x, y, z-1), upm::grad(PBA1, x-1, y, z-1)),
		upm::lerp(u, upm::grad(PAB1, x, y-1, z-1), upm::grad(PBB1, x-1, y-1, z-1))
    );

    return upm::lerp(w, a, b);
}

CPerlinOctave::CPerlinOctave(int octaves, sf::Uint32 seed) noexcept
:
    m_Perlin(seed),
    m_Octaves(octaves)
{

}

double CPerlinOctave::Noise(double x, double y, double z) const noexcept
{
    double result = 0.0;
    double amp = 1.0;

    int i = m_Octaves;
    while(i--)
    {
        result += m_Perlin.Noise(x, y, z) * amp;
        x *= 2.0;
        y *= 2.0;
        z *= 2.0;
        amp *= 0.5;
    }

    return result;
}
