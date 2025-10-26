#ifndef UTILS_H
#define UTILS_H

/**
 * 將 x 限制在 [0.0, 1.0] 範圍內
 */
inline double clamp(double x, double min = 0.0, double max = 1.0) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline double normalize(double x, double min, double max) {
    return (x - min) / (max - min);
}

#endif  // UTILS_H
