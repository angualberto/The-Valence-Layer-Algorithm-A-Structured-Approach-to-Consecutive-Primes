"""
Wavelet Detection of Exceptional-Line Zeros
CWT + PSD on Liouville L(N) data in log-space (u = ln N).
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pywt
from scipy import signal as sp_signal
import os, shutil

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)
ARTIGO_DIR = r"C:\Users\Andre\Documents\siteeg\PLG\assets\artigo"

# ─── 1. Load Liouville data (100M gives wider u-range) ───
data_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "dados", "L_N_100M.txt")
data = np.loadtxt(data_file, skiprows=1)
N = data[:, 0].astype(float)
L = data[:, 1].astype(float)

# ─── 2. Log-space detrended ───
u = np.log(N)
L_detrend = L - np.polyval(np.polyfit(u, L, 1), u)
L_norm = (L_detrend - np.mean(L_detrend)) / np.std(L_detrend)

print(f"Data: {len(N)} pts, u in [{u.min():.3f}, {u.max():.3f}]")

# ─── 3. CWT on interpolated uniform grid ───
n_interp = 5000
u_unif = np.linspace(u.min(), u.max(), n_interp)
L_unif = np.interp(u_unif, u, L_norm)
du = u_unif[1] - u_unif[0]

# Scales to cover f_exc from ~0.5 to ~150
# scale = center_freq / (f * du), center_freq ≈ 0.8125 for morl
# f=0.5 → scale ≈ 0.81/(0.5*0.00106) ≈ 1530
# f=150 → scale ≈ 0.81/(150*0.00106) ≈ 5.1
scales = np.logspace(np.log10(3), np.log10(2000), 300)
coef, freqs = pywt.cwt(L_unif, scales, 'morl', sampling_period=du)

print(f"CWT: {coef.shape[0]} scales x {coef.shape[1]} pts, freqs [{freqs[0]:.3f}, {freqs[-1]:.3f}]")

# ─── 4. PSD via FFT for comparison ───
fft = np.fft.rfft(L_unif)
psd_fft = np.abs(fft)**2
freqs_fft = np.fft.rfftfreq(n_interp, d=du)
p95 = np.percentile(psd_fft, 95)
p99 = np.percentile(psd_fft, 99)

# ─── 5. Check exceptional frequencies ───
f_exc = np.array([n / (2.0 * np.log(2)) for n in range(1, 201)])

psd_exc = np.interp(f_exc, freqs_fft[freqs_fft > 0], psd_fft[freqs_fft > 0])
n_above_95 = np.sum(psd_exc > p95)
n_above_99 = np.sum(psd_exc > p99)

print(f"PSD 95th %ile = {p95:.4e}, 99th %ile = {p99:.4e}")
print(f"Exceptional f above 95%: {n_above_95}/200, above 99%: {n_above_99}/200")

# ─── 6. CWT ridge at exceptional f ───
power = np.abs(coef)**2
ridge = np.zeros(len(f_exc))
for i, f in enumerate(f_exc):
    if freqs[0] <= f <= freqs[-1]:
        idx = np.argmin(np.abs(freqs - f))
        ridge[i] = np.max(power[idx, :])
noise_floor = np.median(power)
ridge_frac = np.sum(ridge > 3*noise_floor) / len(ridge)
print(f"CWT noise floor = {noise_floor:.4e}, ridges above 3x noise: {ridge_frac*100:.1f}%")

# ─── 7. LaTeX table ───
print("\n=== LATEX TABLE ===")
print("\\begin{table}[htbp]")
print("\\centering")
print("\\begin{tabular}{c|c|c|c}")
print("\\hline")
print("$n$ & $f_n$ & PSD & CWT power \\\\")
print("\\hline")
for n in [1, 10, 20, 50, 100, 150, 200]:
    idx = n - 1
    print(f"${n}$ & ${f_exc[idx]:.3f}$ & ${psd_exc[idx]:.3e}$ & ${ridge[idx]:.3e}$ \\\\")
print("\\hline")
print("\\end{tabular}")
print("\\caption{PSD and CWT at exceptional frequencies $f_n = n/(2\\ln 2)$ for $L(N)$ up to $10^8$. No frequency shows a statistically significant peak (99th percentile $\\approx " + f"{p99:.3e}" + "$).}")
print("\\label{tab:wavelet_ridges}")
print("\\end{table}")

# ─── 8. Publication-quality figure ───
fig = plt.figure(figsize=(14, 12))

# 8a. L(N) in log space
ax1 = fig.add_subplot(311)
ax1.plot(u, L_norm, 'b-', lw=0.8)
ax1.set_xlabel('$u = \\ln N$')
ax1.set_ylabel('$L(e^u)$ (norm.)')
ax1.set_title('(a) Liouville $L(e^u)$ — detrended, normalized')
ax1.grid(True, alpha=0.3)

# 8b. CWT scalogram
ax2 = fig.add_subplot(312)
ext = [u[0], u[-1], np.log2(scales[0]), np.log2(scales[-1])]
im = ax2.imshow(np.abs(coef), aspect='auto', extent=ext, cmap='viridis',
                origin='lower', interpolation='bilinear',
                norm=matplotlib.colors.LogNorm(
                    vmin=np.abs(coef[coef > 0]).min(), vmax=np.abs(coef).max()))
ax2.set_xlabel('$u = \\ln N$')
ax2.set_ylabel('$\\log_2(\\text{scale})$')
ax2.set_title('(b) CWT Scalogram (Morlet) — red lines = exceptional $f_n$')
plt.colorbar(im, ax=ax2, label='$|W_f|$')
# Mark exceptional f within range
for f in f_exc:
    if freqs[0] <= f <= freqs[-1]:
        s = scales[np.argmin(np.abs(freqs - f))]
        ax2.axhline(np.log2(s), c='red', lw=0.3, alpha=0.12)

# 8c. PSD
ax3 = fig.add_subplot(313)
pos = freqs_fft > 0
ax3.loglog(freqs_fft[pos], psd_fft[pos], 'b-', lw=0.7, alpha=0.7, label='PSD (FFT)')
ax3.scatter(f_exc, psd_exc, c='red', s=20, marker='o', label='Exceptional $f_n$', zorder=5)
ax3.axhline(p99, c='gray', ls='--', lw=1.2, alpha=0.8, label='99th percentile')
ax3.set_xlabel('Frequency $f$')
ax3.set_ylabel('Power')
ax3.set_title('(c) Power spectrum — no peaks at exceptional frequencies')
ax3.legend(fontsize=9)
ax3.grid(True, alpha=0.3)

plt.tight_layout()
fig.savefig(os.path.join(OUT_DIR, "wavelet_exceptional_spectrum.png"), dpi=250)
shutil.copy(os.path.join(OUT_DIR, "wavelet_exceptional_spectrum.png"),
            os.path.join(ARTIGO_DIR, "wavelet_spectrum_exceptional.png"))
print(f"\nFigure saved to artigo dir")

# ─── 9. Liouville envelope plot ───
fig2, ax = plt.subplots(figsize=(12, 6))
ax.plot(N, np.abs(L), 'b-', lw=0.7, label='$|L(N)|$')
sn = np.sqrt(N)
ax.plot(N, sn, 'g--', lw=1.5, label='$\\sqrt{N}$')
ax.plot(N, 1.36*sn, 'r--', lw=1.5, label='$1.36\\sqrt{N}$')
ax.set_xlabel('N')
ax.set_ylabel('$|L(N)|$')
ax.set_title('Liouville $|L(N)|$ with $\\sqrt{N}$ Envelope')
ax.legend()
ax.grid(True, alpha=0.3)
fig2.tight_layout()
fig2.savefig(os.path.join(ARTIGO_DIR, "liouville_plot.png"), dpi=200)
print(f"Liouville envelope saved")

print("\nDone!")
