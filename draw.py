import json

from .compare import show_diff_plot

keys = list(range(1, 65))

with open("plots/block_1.json", "r") as f:
    results = json.load(f)
    mse1 = [results["MSE"][str(k)] for k in keys]
    psnr1 = [results["PSNR"][str(k)] for k in keys]
    ssim1 = [results["SSIM"][str(k)] for k in keys]

with open("plots/overlap_1.json", "r") as f:
    results = json.load(f)
    mse2 = [results["MSE"][str(k)] for k in keys]
    psnr2 = [results["PSNR"][str(k)] for k in keys]
    ssim2 = [results["SSIM"][str(k)] for k in keys]

with open("plots/sliding_1.json", "r") as f:
    results = json.load(f)
    mse3 = [results["MSE"][str(k)] for k in keys]
    psnr3 = [results["PSNR"][str(k)] for k in keys]
    ssim3 = [results["SSIM"][str(k)] for k in keys]

with open("plots/sliding_0.json", "r") as f:
    results = json.load(f)
    psnr4 = [results["PSNR"][str(k)] for k in keys]
    ssim4 = [results["SSIM"][str(k)] for k in keys]

show_diff_plot("mse.png", keys, "MSE Value", mse1, "Block", mse2, "Overlap", mse3, "Sliding", logscale=True)
show_diff_plot("psnr.png", keys, "PSNR Value (dB)", psnr1, "Block", psnr2, "Overlap", psnr3, "Sliding")
show_diff_plot("ssim.png", keys, "SSIM Value", ssim1, "Block", ssim2, "Overlap", ssim3, "Sliding")

show_diff_plot("clamp-timing.png", keys, "SSIM Value", ssim4, "Clamped each step", ssim3, "Clamp at last step")
show_diff_plot("clamp-timing2.png", keys, "PSNR Value (dB)", psnr4, "Clamped each step", psnr3, "Clamp at last step")

# keys = [1, 2, 3, 4, 8, 16, 32, 64]

# for filename in [
#     "plots/block_1.json",
#     "plots/overlap_1.json",
#     "plots/sliding_1.json",
# ]:
#     print(filename)
#     with open(filename, "r") as f:
#         results = json.load(f)
#         for k in keys:
#             r = ComparisonResult(
#                 MSE=float(results["MSE"][str(k)]),
#                 PSNR=float(results["PSNR"][str(k)]),
#                 SSIM=float(results["SSIM"][str(k)]),
#             )
#             print(f"  K={k}: {r}")

<<<<<<< HEAD
print("Hello main")
=======
print("Hello feature")
>>>>>>> feature
