#ifndef INTERPOLATION_H
#define INTERPOLATION_H
#include <functional>
#include <utility>
#include <vector>

#include "image.h"

double lagrange(const std::vector<double>& y, double xi);

// 使用一般或 overlap 方法

std::pair<int, int> get_block_range(int xi, int N, int K);

std::pair<double, double> super_row(const Image& src, Image& dst, int K, bool overlap = true, bool clamped = true);
std::pair<double, double> super_column(const Image& src, Image& dst, int K, bool overlap = true, bool clamped = true);

// 使用 sliding window

std::pair<int, int> get_sliding_range(int xi, int N, int K);

std::pair<double, double> sliding_row(const Image& src, Image& dst, int K, bool clamped = true);
std::pair<double, double> sliding_column(const Image& src, Image& dst, int K, bool clamped = true);

#define USE_METHOD_BLOCK 0
#define USE_METHOD_OVERLAP 0x10
#define USE_METHOD_SLIDING 0x20

#define CLAMP_EACH_STEP 0
#define CLAMP_AT_END 1
#define NORMALIZE_AT_END 2

void super_sample(const Image& src, Image& dst, int K, int clamping_method = USE_METHOD_SLIDING | CLAMP_AT_END);
#endif  // INTERPOLATION_H
