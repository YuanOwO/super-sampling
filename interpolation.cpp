
#include "interpolation.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "image.h"
#include "utils.h"

/**
 * 拉格朗日插值法
 * @param ys 取樣點陣列
 * @param xi 要插值的點
 * @return 插值結果
 */
double lagrange(const std::vector<double>& ys, double xi) {
    double ret = 0.0;  // 回傳值
    for (int i = 0; i < ys.size(); i++) {
        double term = ys[i];
        for (int j = 0; j < ys.size(); j++) {
            if (i == j) continue;
            term *= (xi - j) / (i - j);
        }
        ret += term;
    }
    return ret;
}

/**********************************************************************************************************************/

/**
 * 計算區塊取樣範圍
 *
 * @param xi 插值位置
 * @param N 插值範圍大小
 * @param K 區塊大小
 *
 * @return std::pair<int, int> 取樣範圍的左、右界
 */
std::pair<int, int> get_block_range(int x, int N, int K) {
    int left = (x - std::min(x / K, N % K)) / K * K;  // 我也不太清楚
    left += std::min(left / K, (N - left) % K);       // 我在算什麼東西？
    if ((N - left) % K) K++;                          // 但他應該是對的。
    return {left, left + K};                          // 回傳區塊範圍
}

/**
 * 列方向的 super sampling
 *
 * @param src 輸入影像
 * @param dst 輸出影像
 * @param blockSize 區塊大小 (K)
 * @param overlap 是否使用重疊取樣
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> super_row(const Image& src, Image& dst, int blockSize, bool overlap, bool clamped) {
    blockSize = src.width / (src.width / blockSize);  // 調整 blockSize 的大小，使每個區塊儘量均勻

    double scale = (double)src.width / dst.width;  // [0, M) -> [0, N) 的縮放比例
    double mx = 1.0, mn = 0.0;                     // 記錄最大值、最小值

    for (int i = 0; i < dst.height; i++) {
        int last_left = -1;      // 上一次的 left 位置
        std::vector<double> ys;  // 插值的取樣點

        for (int j = 0; j < dst.width; j++) {
            double xi = j * scale;  // 在原始影像中的位置

            auto [left, right] = get_block_range((int)xi, src.width, blockSize);  // 取樣區塊的範圍

            if (overlap) {                       // 使用 overlap 方式
                if (left > 0) left--;            // 向左擴展取樣範圍
                if (right < src.width) right++;  // 向右擴展取樣範圍
            }

            if (left != last_left) {  // 更新取樣點 (如有需要)
                ys.resize(right - left);
                for (int jj = 0, l = left; l < right; jj++, l++) {
                    ys[jj] = src.data[i][l];
                }
                last_left = left;
            }

            double value = lagrange(ys, xi - left);
            if (clamped) value = clamp(value);
            mx = std::max(mx, value), mn = std::min(mn, value);

            dst.data[i][j] = value;
        }
    }

    return {mn, mx};
}

/**********************************************************************************************************************/

/**
 * 計算 sliding window 的取樣範圍
 *
 * @param xi 插值位置
 * @param N 插值範圍大小
 * @param K 區塊大小
 * @return std::pair<int, int> 取樣範圍的左、右界
 */
std::pair<int, int> get_sliding_range(int xi, int N, int K) {
    int left = std::max(0, (int)xi - K / 2),  // 取樣區間的左界
        right = std::min(N, left + K);        // 取樣區間的右界
    left = std::max(0, right - K);            // 調整 left 以確保有 K 個點
    return {left, right};
}

/**
 * 列方向的 super sampling
 * 使用 sliding window 的方式使插入點保持在區塊中央
 *
 * @param src 輸入影像
 * @param dst 輸出影像
 * @param blockSize 區塊大小 (K)
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> sliding_row(const Image& src, Image& dst, int blockSize, bool clamped) {
    double scale = (double)src.width / dst.width;  // [0, M) -> [0, N) 的縮放比例
    double mx = 1.0f, mn = 0.0f;                   // 記錄最大值、最小值

    for (int i = 0; i < dst.height; i++) {
        int last_left = -1;                 // 上一次的 left 位置
        std::vector<double> ys(blockSize);  // 插值的取樣點

        for (int j = 0; j < dst.width; j++) {
            double xi = j * scale;  // 在原始影像中的位置

            auto [left, right] = get_sliding_range((int)xi, src.width, blockSize);  // 取樣區塊的範圍

            if (left != last_left) {  // 更新取樣點 (如有需要)
                for (int jj = 0, l = left; l < right; jj++, l++)
                    ys[jj] = src.data[i][l];
                last_left = left;
            }

            double value = lagrange(ys, xi - left);
            if (clamped) value = clamp(value);
            mx = std::max(mx, value), mn = std::min(mn, value);

            dst.data[i][j] = value;
        }
    }

    return {mn, mx};
}

/**
 * 進行 super sampling
 * 先對行方向進行插值，再對列方向進行插值
 *
 * @param src 輸入影像
 * @param dst 輸出影像
 * @param blockSize 區塊大小 (K)
 * @param method 計算方法
 *      十六位數: 0: 使用區塊取樣 (預設)，1: 使用 overlap 取樣，2: 使用 sliding window
 *      個位數: 0: 每次插值時 clamp，1: 最後再 clamp (預設)，2: 線性正規化
 */
void super_sample(const Image& src, Image& dst, int blockSize, int method) {
    Image mid = zerosImage(dst.width, src.height, NULL);  // 中間影像

    bool overlap = (method / 16 == 1);  // 是否使用 overlap 取樣
    bool clamped = (method % 16 == 0);  // 是否在每次插值時 clamp
    std::pair<double, double> p1, p2;   // 記錄最大值、最小值

    if (method / 16 < 2) {  // 使用一般或 overlap 方法
        // 列方向插值
        p1 = super_row(src, mid, blockSize, overlap, clamped);
        transposeImage(&mid);
        // 行方向插值
        p2 = super_row(mid, dst, blockSize, overlap, clamped);
        transposeImage(&dst);
    } else if (method / 16 == 2) {  // 使用 sliding window 方法
        // 列方向插值
        p1 = sliding_row(src, mid, blockSize, clamped);
        // 行方向插值
        transposeImage(&mid);
        p2 = sliding_row(mid, dst, blockSize, clamped);
        transposeImage(&dst);
    } else {  // 未知的方法
        std::cerr << "Error: Unknown method code " << std::hex << method << std::endl;
        freeImage(mid);
        return;
    }

    double mn1 = p1.first, mx1 = p1.second, mn2 = p2.first, mx2 = p2.second;

    if (method % 16 == CLAMP_AT_END) {  // 最後再 clamp
        for (int i = 0; i < dst.height; i++)
            for (int j = 0; j < dst.width; j++)
                dst.data[i][j] = clamp(dst.data[i][j]);
    } else if (method % 16 == NORMALIZE_AT_END) {  // 正規化到 [0, 1]
        double mx = std::max(mx1, mx2), mn = std::min(mn1, mn2);
        for (int i = 0; i < dst.height; i++) {
            for (int j = 0; j < dst.width; j++) {
                dst.data[i][j] = normalize(dst.data[i][j], mn, mx);
            }
        }
    }

    freeImage(mid);
}
