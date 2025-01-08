#pragma once


template <typename T>
constexpr T Clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo
         : (hi < v) ? hi
                    : v;
}


template <typename T>
constexpr T Max(const T& a, const T& b)
{
    return (a < b) ? b : a;
}


template <typename T>
constexpr T Min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}
