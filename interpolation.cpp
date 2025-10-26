
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
 * @param y 取樣點陣列
 * @param xi 要插值的點
 * @return 插值結果
 */
double lagrange(const std::vector<double>& y, double xi) {
    double ret = 0.0;  // 回傳值
    for (int i = 0; i < y.size(); i++) {
        double term = y[i];
        for (int j = 0; j < y.size(); j++) {
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
 * @param K 區塊大小
 * @param get_range 取樣範圍的計算函數
 * @param overlap 是否使用重疊取樣
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> super_row(const Image& src, Image& dst, int K, bool overlap, bool clamped) {
    K = src.width / (src.width / K);  // 調整 K 的大小，使每個區塊儘量均勻

    double scale = (double)(src.width) / (dst.width);  // [0, M] -> [0, N] 的縮放比例
    double mx = 1.0f, mn = 0.0f;                       // 記錄最大值、最小值

    for (int i = 0; i < dst.height; i++) {
        int last_left = -1;      // 上一次的 left 位置
        std::vector<double> ys;  // 插值的取樣點

        for (int j = 0; j < dst.width; j++) {
            double xi = j * scale;  // 在原始影像中的位置

            auto [left, right] = get_block_range((int)xi, src.width, K);  // 取樣區塊的範圍

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

/**
 * 行方向的 super sampling
 *
 * @param src 輸入影像
 * @param dst 輸出影像
 * @param K 區塊大小
 * @param get_range 取樣範圍的計算函數
 * @param overlap 是否使用重疊取樣
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> super_column(const Image& src, Image& dst, int K, bool overlap, bool clamped) {
    K = src.height / (src.height / K);  // 調整 K 的大小，使每個區塊儘量均勻

    double scale = (double)(src.height) / (dst.height);  // [0, M] -> [0, N] 的縮放比例
    double mx = 1.0f, mn = 0.0f;                         // 記錄最大值、最小值

    for (int j = 0; j < dst.width; j++) {
        int last_top = -1;       // 上一次的 top 位置
        std::vector<double> ys;  // 插值的取樣點

        for (int i = 0; i < dst.height; i++) {
            double xi = i * scale;  // 在原始影像中的位置

            auto [top, bottom] = get_block_range((int)xi, src.height, K);  // 取樣區塊的範圍

            if (overlap) {                          // 使用 overlap 方式
                if (top > 0) top--;                 // 向上擴展取樣範圍
                if (bottom < src.height) bottom++;  // 向下擴展取樣範圍
            }

            if (top != last_top) {  // 更新取樣點 (如有需要)
                ys.resize(bottom - top);
                for (int ii = 0, t = top; t < bottom; ii++, t++) {
                    ys[ii] = src.data[t][j];
                }
                last_top = top;
            }

            double value = lagrange(ys, xi - top);
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
 * @param K 區塊大小
 *
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
 * @param K 區塊大小
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> sliding_row(const Image& src, Image& dst, int K, bool clamped) {
    double scale = (double)(src.width) / (dst.width);  // [0, M] -> [0, N] 的縮放比例
    double mx = 1.0f, mn = 0.0f;                       // 記錄最大值、最小值

    for (int i = 0; i < dst.height; i++) {
        int last_left = -1;         // 上一次的 left 位置
        std::vector<double> ys(K);  // 插值的取樣點

        for (int j = 0; j < dst.width; j++) {
            double xi = j * scale;  // 在原始影像中的位置

            auto [left, right] = get_sliding_range((int)xi, src.width, K);  // 取樣區塊的範圍

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
 * 行方向的 super sampling
 * 使用 sliding window 的方式使插入點保持在區塊中央
 *
 * @param src 輸入影像
 * @param dst 輸出影像
 * @param K 區塊大小
 * @param clamped 是否將結果限制在 [0, 1]
 *
 * @return std::pair<double, double> 計算出的最小值與最大值
 */
std::pair<double, double> sliding_column(const Image& src, Image& dst, int K, bool clamped) {
    double scale = (double)(src.height) / (dst.height);  // [0, M] -> [0, N] 的縮放比例
    double mx = 1.0f, mn = 0.0f;                         // 記錄最大值、最小值

    for (int j = 0; j < dst.width; j++) {
        int last_top = -1;          // 上一次的 top 位置
        std::vector<double> ys(K);  // 插值的取樣點

        for (int i = 0; i < dst.height; i++) {
            double xi = i * scale;  // 在原始影像中的位置

            auto [top, bottom] = get_sliding_range((int)xi, src.height, K);  // 取樣區塊的範圍

            if (top != last_top) {  // 更新取樣點 (如有需要)
                for (int ii = 0, t = top; t < bottom; ii++, t++)
                    ys[ii] = src.data[t][j];
                last_top = top;
            }

            double value = lagrange(ys, xi - top);
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
 * @param K 區塊大小
 * @param method 計算方法
 *      十六位數: 0: 使用區塊取樣 (預設)，1: 使用 overlap 取樣，2: 使用 sliding window
 *      個位數: 0: 每次插值時 clamp，1: 最後再 clamp (預設)，2: 線性正規化
 */
void super_sample(const Image& src, Image& dst, int K, int method) {
    Image mid = zerosImage(dst.width, src.height, NULL);  // 中間影像

    bool clamped = (method % 16 == 0);  // 每次插值時是否 clamp
    bool overlap = (method / 16 == 1);  // 是否使用 overlap 取樣
    double mn1, mx1, mn2, mx2;

    if (method / 16 < 2) {  // 使用一般或 overlap 方法
        auto p1 = super_row(src, mid, K, overlap, clamped);
        auto p2 = super_column(mid, dst, K, overlap, clamped);
        mn1 = p1.first, mx1 = p1.second;
        mn2 = p2.first, mx2 = p2.second;
    } else if (method / 16 == 2) {  // 使用 sliding window 方法
        auto p1 = sliding_row(src, mid, K, clamped);
        auto p2 = sliding_column(mid, dst, K, clamped);
        mn1 = p1.first, mx1 = p1.second;
        mn2 = p2.first, mx2 = p2.second;
    } else {  // 未知的方法
        std::cerr << "Error: Unknown method code " << std::hex << method << std::endl;
        freeImage(mid);
        return;
    }

    if (method % 16 == 1) {  // 最後再 clamp
        for (int i = 0; i < dst.height; i++)
            for (int j = 0; j < dst.width; j++)
                dst.data[i][j] = clamp(dst.data[i][j]);
    } else if (method % 16 == 2) {  // 正規化到 [0, 1]
        double mx = std::max(mx1, mx2), mn = std::min(mn1, mn2);

        for (int i = 0; i < dst.height; i++) {
            for (int j = 0; j < dst.width; j++) {
                dst.data[i][j] = normalize(dst.data[i][j], mn, mx);
            }
        }
    }

    freeImage(mid);
}
