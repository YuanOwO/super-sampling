import json
from collections import namedtuple
from glob import glob
from subprocess import PIPE, Popen
from sys import argv, platform

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
from mpl_toolkits.axes_grid1 import host_subplot
from tqdm import tqdm

ComparisonResult = namedtuple("ComparisonResult", ["MSE", "PSNR", "SSIM"])


def compare_images(img1_path, img2_path):
    """比較兩張影像的差異"""
    # Construct the command to run the comparison tool
    command = ["./compare.exe", img1_path, img2_path]

    # Execute the command
    process = Popen(command, stdout=PIPE, stderr=PIPE)
    stdout, stderr = process.communicate()

    outs = stdout.decode().strip().splitlines()

    if len(outs) == 3:  # 錯誤訊息
        raise RuntimeError(outs[2])

    results = ComparisonResult(
        MSE=float(outs[16][9:]),
        PSNR=float(outs[17][9:-3]),
        SSIM=float(outs[18][9:]),
    )

    return results


def show_psnr_ssim_plot(filename: str, keys: list, psnr: list, ssim: list):
    """顯示 PSNR 和 SSIM 的折線圖"""
    plt.clf()
    plt.figure(figsize=(12, 8))

    # host = HostAxes(fig, [0.1, 0.1, 0.8, 0.8])  # 主座標軸
    # fig.add_axes(host)

    # par = ParasiteAxes(host, sharex=host)  # 共享 x 軸
    # fig.add_axes(par)

    host = host_subplot(111)
    par = host.twinx()

    plt.title("PSNR and SSIM vs Block Size")
    host.set_xlabel("Block Size (K)")
    host.set_ylabel("PSNR Value (dB)")
    par.set_ylabel("SSIM Value")

    (p1,) = host.plot(keys, psnr, "o-", label="PSNR")
    (p2,) = par.plot(keys, ssim, "^-", label="SSIM")

    host.legend()

    host.set_xlim(0, 64)
    host.xaxis.set_major_locator(MultipleLocator(8))
    host.xaxis.set_minor_locator(MultipleLocator(2))

    host.set_ylim(0, 40)
    host.yaxis.set_major_locator(MultipleLocator(5))
    host.yaxis.set_minor_locator(MultipleLocator(1.25))

    par.set_ylim(0, 1)
    par.yaxis.set_major_locator(MultipleLocator(0.125))

    host.grid(True)
    # host.grid(True, "minor", linestyle="--")
    # par.grid(True)

    host.tick_params(axis="both", which="both", direction="in", length=6)
    par.tick_params(axis="both", which="both", direction="in", length=6)

    # plt.tight_layout()
    plt.savefig(filename, dpi=300)
    # plt.show()


def show_diff_ssim_plot(
    filename: str,
    keys: list,
    ssim1: list,
    label1: str,
    ssim2: list,
    label2: str,
    ssim3: list = None,
    label3: str = None,
):
    """顯示 SSIM 差異的折線圖"""
    plt.clf()
    plt.figure(figsize=(12, 8))

    # plt.title("SSIM Difference")
    plt.xlabel("Block Size (K)")
    plt.ylabel("SSIM Value")

    plt.plot(keys, ssim1, "o-", label=label1)
    plt.plot(keys, ssim2, "^-", label=label2)
    if ssim3 and label3:
        plt.plot(keys, ssim3, "s-", label=label3)

    plt.xlim(0, 64)
    plt.gca().xaxis.set_major_locator(MultipleLocator(8))
    plt.gca().xaxis.set_minor_locator(MultipleLocator(2))

    plt.ylim(0, 1)
    plt.gca().yaxis.set_major_locator(MultipleLocator(0.2))
    plt.gca().yaxis.set_minor_locator(MultipleLocator(0.05))

    plt.grid(True)
    plt.legend()

    plt.savefig(filename, dpi=300)
    # plt.show()


if __name__ == "__main__":
    if not platform.startswith("win"):
        print("This script is only supported on Windows.")
        exit(1)

    if len(argv) == 2:
        images = glob(argv[1])
    elif len(argv) > 2:
        images = argv[1:]
    else:
        images = []

    if images:  # 有指定影像檔案
        width = max(len(img) for img in images) + 1

        for img in images:
            result = compare_images("image/image2.txt", img)
            print(f"{img+':':<{width}} {result}")
    else:
        # mse, psnr, ssim = {}, {}, {}

        # for k in tqdm(range(1, 65), desc="Comparing images"):
        #     img = f"image/output_{k}.txt"
        #     result = compare_images("image/image2.txt", img)
        #     mse[k] = result.MSE
        #     psnr[k] = result.PSNR
        #     ssim[k] = result.SSIM

        # # filename = "plots/block_1_64-512.json"
        # # filename = "plots/overlap_1_64-512.json"
        # filename = "plots/sliding_1_64-512.json"

        # with open(filename, "w") as f:
        #     results = {
        #         "MSE": mse,
        #         "PSNR": psnr,
        #         "SSIM": ssim,
        #     }
        #     json.dump(results, f, indent=4)

        # with open(filename, "r") as f:
        #     results = json.load(f)
        #     keys = sorted(int(k) for k in results["MSE"].keys())
        #     mse = [results["MSE"][str(k)] for k in keys]
        #     psnr = [results["PSNR"][str(k)] for k in keys]
        #     ssim = [results["SSIM"][str(k)] for k in keys]

        # show_psnr_ssim_plot(filename.replace(".json", ".png"), keys, psnr, ssim)

        keys = list(range(1, 65))

        with open("plots/block_1_64-512.json", "r") as f:
            results = json.load(f)
            ssim1 = [results["SSIM"][str(k)] for k in keys]

        with open("plots/overlap_1_64-512.json", "r") as f:
            results = json.load(f)
            ssim2 = [results["SSIM"][str(k)] for k in keys]

        with open("plots/sliding_1_64-512.json", "r") as f:
            results = json.load(f)
            ssim3 = [results["SSIM"][str(k)] for k in keys]

        show_diff_ssim_plot("ssim2.png", keys, ssim1, "Block", ssim2, "Overlap", ssim3, "Sliding")
