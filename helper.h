#ifndef HELPER_H
#define HELPER_H

template<typename T> inline T min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T> inline T max(T a, T b)
{
    return a > b ? a : b;
}

float max(int a, float b)
{
    return a > b ? (float)a : b;
}

template<typename Tx, typename Ta, typename Tb> inline Tx constrain(Tx x, Ta a, Tb b)
{
    return x < a ? a : x > b ? b : x;
}

template<typename Tx, typename Ta, typename Tb> inline Tx smoothstep(Ta a, Tb b, Tx x)
{
    x = constrain((x - a) / (b - a), 0, 1);
    return x * x * (3 - 2 * x);
}

struct vec2
{
    float x,y;
    vec2(): x(0), y(0) {}
    vec2(float x, float y): x(x), y(y) {}
    float get_distance_to(vec2 v) {return sqrt(pow(v.x-x,2) + pow(v.y-y,2));}
};

Uint32 RGBA(int r, int g, int b, float a)
{
    return r * 0x00000001 + g * 0x00000100 + b * 0x00010000 + (int)(255 * a) * 0x01000000;
}

Uint32 RGB(int r, int g, int b)
{
    return RGBA(r, g, b, 1);
}

float pseudorandom(float x)
{
    return fmod((sin(x) * 12.9898 + 78.233) * 43758.5453, 1.);
}

long random(int max)
{
    return abs(random() % max);
}

#endif
