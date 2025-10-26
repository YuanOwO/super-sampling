#include <cassert>
#include <iostream>

#include "image.h"
#include "interpolation.h"
#include "read.h"
#include "write.h"

using namespace std;

int main(int argc, char** argv) {
    string srcFilename = "image/image1.txt";  // 輸入檔案名稱
    int srcSize = 0, dstSize = 0;             // 輸入、輸出影像大小 (N*N, M*M)

    // 讀取命令列參數
    if (argc > 1) srcFilename = argv[1];    // 自訂輸入檔案
    if (argc > 2) dstSize = atoi(argv[2]);  // 自訂輸出大小

    // 讀取輸入影像
    Image src = readImage(srcFilename.c_str());

    if (!src.data) {  // 讀取失敗
        cerr << "Error: Unable to read image from " << srcFilename << endl;
        freeImage(src);
        return 1;
    }

    if (src.width != src.height) {  // 目前只支援正方形影像
        cerr << "Error: Input image must be square." << endl;
        freeImage(src);
        return 1;
    }

    srcSize = src.width;                  // 輸入影像大小
    if (!dstSize) dstSize = srcSize * 8;  // 預設放大 8 倍

    // 刪除舊的輸出檔案
#if _WIN32 || _WIN64  // Windows
    int ret = system("del /Q image\\output_*");
#else  // Linux 或 macOS
    int ret = system("rm -f image/output_*");
#endif
    assert(ret == 0);  // 命令應該要成功執行

    // 進行 super sampling
    vector<int> k_list = {1, 2, 4, 6, 8, 16, 32};  // 不同的 K 值測試
    string outputs;                                // 輸出檔案名稱列表

    // for (int k : k_list) {
    for (int k = 1; k <= 64; k++) {
        string dstFilename = "image/output_" + to_string(k) + ".txt";
        outputs += " " + dstFilename;
        cout << "Generating `" << dstFilename << "' ..." << endl;

        Image dst = zerosImage(dstSize, dstSize, dstFilename.c_str());  // 輸出影像
        super_sample(src, dst, k, USE_METHOD_SLIDING | CLAMP_AT_END);
        writeImage(dstFilename.c_str(), dst);
        freeImage(dst);
    }

    // 釋放記憶體
    freeImage(src);

    // 顯示輸入、輸出影像
#if _WIN32 || _WIN64  // Windows
    string command = "./display.exe " + srcFilename + outputs;
#else  // Linux 或 macOS
    string command = "./display " + srcFilename + outputs + " & ./convert" + outputs;
#endif
    ret = system(command.c_str());
    assert(ret == 0);  // 命令應該要成功執行

    return 0;
}
