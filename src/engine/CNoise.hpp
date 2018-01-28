#ifndef NOISE_H
#define NOISE_H

#include <random>
#include <array>

class CPerlin final
{
public:
    CPerlin(sf::Uint32 seed=0) noexcept;

    double Noise(double x) const noexcept { return Noise(x, 0, 0); }
    double Noise(double x, double y) const noexcept { return Noise(x, y, 0); }
    double Noise(double x, double y, double z) const noexcept;
    int GetURandom(int Min, int Max) noexcept;
    sf::Uint32 GetSeed() const noexcept { return m_Seed; }

private:
    std::array<int, 512> m_aNumsPerlin;
    // std::mt19937_64
    std::mt19937 m_Engine;
    sf::Uint32 m_Seed;
};


class CPerlinOctave final
{
public:
    CPerlinOctave(int octaves, sf::Uint32 seed=0) noexcept;

    const CPerlin& Perlin() const noexcept { return m_Perlin; }
    double Noise(double x) const noexcept { return Noise(x, 0, 0); }
    double Noise(double x, double y) const noexcept { return Noise(x, y, 0); }
    double Noise(double x, double y, double z) const noexcept;

private:
    CPerlin m_Perlin;
    int m_Octaves;
};

#endif // NOISE_H
