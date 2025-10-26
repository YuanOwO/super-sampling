# HW2: Image Super-sampling

## 技術說明

1.  讀取一張 $N \times N$ 的低解析度灰階影像
2.  對原圖的每一列使用 Lagrange 插值法擴展為 $N \times M$ 的中間影像
3.  對中間影像的每一行使用 Lagrange 插值法擴展為 $M \times M$ 的高解析度影像
4.  對於計算結果進行 clamp，以確保結果依然在 $[0, 1]$ 的範圍內。本專案可以選擇以下 clamp 時機：
    -   每次插值時計算 clamp
    -   最後再對整張圖片進行 clamp
5.  在插值過程中，為避免圖片產生過擬合現象，因此在過程中只會使用大小為 K 的區塊來進行插值計算，而非整列或整行。本專案實作了以下區塊選擇方法：
    -   區塊取樣 (Block)：直接將影像分割為大小約為 K 的不重疊區塊，對每個區塊分別進行插值計算。
    -   重疊取樣 (Overlap)：同上述方法，但邊界會延伸一格像素，使得相鄰區塊間會有重疊的部分，能夠減少邊界效應。
    -   滑動視窗 (Sliding Window)：使用滑動視窗的方式，對每個像素點周圍大小為 K 的區域進行插值計算，以獲得更平滑的結果。
6.  最後會使用 `display` 程式來顯示輸入與輸出影像，並使用 `convert` 程式將輸出影像轉換為 PNG 格式。

## 使用說明

1.  系統需求：

    -   支援 C++ 17 以上版本的編譯器
    -   freeglut
    -   Python 3.10+、matplotlib (僅用於比較圖片差異)

2.  編譯程式碼：

    ```bash
    make clean // 清理舊的編譯檔案
    make
    ```

    **注意：此功能需安裝 makefile**

3.  執行程式：

    ```bash
    ./super <input_image> <M>
    ```

    其中 `<input_image>` 是輸入的低解析度影像檔案名稱，預設為 `image/input.png`；
    `<M>` 是輸出影像的解析度大小 (輸出影像為 M x M)，預設為原圖的八倍大小。
    輸出影像會存放在 `image/output_<K>.txt`，其中 `<K>` 是區塊大小。

4.  在視窗中顯示影像：

    ```bash
    ./display <img1.txt> <img2.txt> < ... >
    ```

5.  將影像轉換為 PNG 格式：

    ```bash
    ./convert <img1.txt> <img2.txt> < ... >
    ```

6.  比較圖片差異：

    ```bash
    python compare.py <img1.txt> <img2.txt> < ... >
    ```

    **注意：此功能僅限 Windows 平台使用**

    將 img1、img2、... 依序與原解析度圖片 (image2.txt) 比較，並輸出比較結果 MSE、PSNR、SSIM。
    若只有一個輸入參數時支援 glob，可以匹配多個檔案進行比較。
    若無輸入參數，則預設為比較 `image/output_*.txt` 的所有圖片。

## 專案結構

-   `super.cpp`：主程式，負責讀取輸入影像、插值、寫出輸出影像。
-   `compare.exe`：比較輸出影像與原解析度影像的差異的可執行檔。
-   `compare.py`：批次比較輸出影像與原解析度影像的差異。
-   `convert.c`：將輸出影像轉換為 PNG 格式。
-   `display.c`：顯示輸出影像。
-   `interpolation.cpp`：實作插值方法。
-   `makefile`：編譯指令。
-   `image/`：存放輸入與輸出影像的資料夾。
-   `include/`：存放標頭檔的資料夾。
-   `plot/`：存放相關比較圖表的資料夾。
    -   `<method>_<0/1>.json`：記錄不同方法對於不同區塊大小 K 的 MSE、PSNR、SSIM 數值。
        -   `0` 表示每一步驟都進行 clamp。
        -   `1` 表示最後才進行 clamp。
    -   `<method>_<0/1>.png`：上述的 PSNR、SSIM 圖表。
    -   `diff_of_clamp_timing.png`：比較不同 clamp 時機的 SSIM 圖表。
    -   `diff_of_methods.png`：比較不同區塊選擇方法的 SSIM 圖表。
