#!/usr/bin/env python3
"""
realtime_plot.py - Semi Realtime Visualization menggunakan FuncAnimation
Memantau results/results.csv dan update grafik otomatis setiap interval tertentu.

Jalankan SEBELUM atau SAAT benchmark.exe berjalan untuk melihat update realtime.
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.gridspec as gridspec
import numpy as np
import os
import sys
import time

# ─────────────────────────────────────────────────────────────
# Konfigurasi
# ─────────────────────────────────────────────────────────────
CSV_PATH      = "results/results.csv"
UPDATE_INTERVAL = 3000  # ms, cek update setiap 3 detik

PALETTE = {
    "merge_par":   "#1565C0",
    "bitonic_par": "#BF360C",
    "merge_seq":   "#42A5F5",
    "bitonic_seq": "#FF7043",
    "ideal":       "#9E9E9E",
}

plt.rcParams.update({
    "font.family":     "DejaVu Sans",
    "font.size":       9,
    "axes.titlesize":  10,
    "figure.dpi":      120,
})

# ─────────────────────────────────────────────────────────────
# State
# ─────────────────────────────────────────────────────────────
last_row_count = 0
last_modified  = 0.0

# ─────────────────────────────────────────────────────────────
# Setup Figure
# ─────────────────────────────────────────────────────────────
fig = plt.figure(figsize=(16, 9), facecolor="#F5F5F5")
fig.suptitle(
    "Real-Time Benchmark Monitor: Parallel Merge Sort vs Bitonic Sort (OpenMP)",
    fontsize=12, fontweight="bold", color="#212121"
)

gs = gridspec.GridSpec(2, 3, figure=fig, hspace=0.45, wspace=0.35,
                       left=0.07, right=0.97, top=0.92, bottom=0.1)

ax_time    = fig.add_subplot(gs[0, :2])  # Execution time (lebar)
ax_speedup = fig.add_subplot(gs[0, 2])   # Speedup
ax_eff     = fig.add_subplot(gs[1, 0])   # Efficiency
ax_threads = fig.add_subplot(gs[1, 1])   # Progress per thread
ax_status  = fig.add_subplot(gs[1, 2])   # Status / info text

# ─────────────────────────────────────────────────────────────
# Fungsi Load Data
# ─────────────────────────────────────────────────────────────
def load_csv():
    if not os.path.exists(CSV_PATH):
        return None
    try:
        df = pd.read_csv(CSV_PATH)
        if df.empty:
            return None
        return df
    except Exception:
        return None

# ─────────────────────────────────────────────────────────────
# Update: Execution Time Panel
# ─────────────────────────────────────────────────────────────
def update_time_panel(df, ax):
    ax.cla()
    ax.set_facecolor("#FAFAFA")
    ax.set_title("Execution Time vs Thread Count", fontweight="bold")
    ax.set_xlabel("Jumlah Thread")
    ax.set_ylabel("Execution Time (ms)")

    # Gunakan dataset random sebagai representative
    sub = df[df["dataset_type"] == "random"] if "random" in df["dataset_type"].values else df

    merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                    (sub["cutoff_threshold"] == 1024)].groupby("thread_count")["execution_time"].mean()

    bitonic_par = sub[(sub["algorithm"] == "bitonic_sort_par") &
                      (sub["schedule_type"] == "static")].groupby("thread_count")["execution_time"].mean()

    if not merge_par.empty:
        ax.plot(merge_par.index, merge_par.values,
                "o-", color=PALETTE["merge_par"], linewidth=2.5, markersize=8,
                label="Merge Sort Par", markerfacecolor="white", markeredgewidth=2.5)

    if not bitonic_par.empty:
        ax.plot(bitonic_par.index, bitonic_par.values,
                "s-", color=PALETTE["bitonic_par"], linewidth=2.5, markersize=8,
                label="Bitonic Sort Par", markerfacecolor="white", markeredgewidth=2.5)

    # Sequential baseline
    merge_seq = sub[sub["algorithm"] == "merge_sort_seq"]["execution_time"].mean()
    bitonic_seq = sub[sub["algorithm"] == "bitonic_sort_seq"]["execution_time"].mean()

    if not np.isnan(merge_seq):
        ax.axhline(y=merge_seq, color=PALETTE["merge_seq"], linestyle="--",
                   linewidth=1.5, alpha=0.7, label=f"Merge Seq ({merge_seq:.0f}ms)")
    if not np.isnan(bitonic_seq):
        ax.axhline(y=bitonic_seq, color=PALETTE["bitonic_seq"], linestyle=":",
                   linewidth=1.5, alpha=0.7, label=f"Bitonic Seq ({bitonic_seq:.0f}ms)")

    ax.legend(fontsize=8, loc="upper right")
    ax.grid(alpha=0.3)
    ax.set_ylim(bottom=0)

# ─────────────────────────────────────────────────────────────
# Update: Speedup Panel
# ─────────────────────────────────────────────────────────────
def update_speedup_panel(df, ax):
    ax.cla()
    ax.set_facecolor("#FAFAFA")
    ax.set_title("Speedup (Random Dataset)", fontweight="bold")
    ax.set_xlabel("Jumlah Thread")
    ax.set_ylabel("Speedup")

    sub = df[df["dataset_type"] == "random"] if "random" in df["dataset_type"].values else df

    merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                    (sub["cutoff_threshold"] == 1024)].groupby("thread_count")["speedup"].mean()

    bitonic_par = sub[(sub["algorithm"] == "bitonic_sort_par") &
                      (sub["schedule_type"] == "static")].groupby("thread_count")["speedup"].mean()

    all_t = sorted(set(list(merge_par.index) + list(bitonic_par.index)))

    if all_t:
        ax.plot(all_t, all_t, "--", color=PALETTE["ideal"],
                linewidth=1.5, alpha=0.6, label="Ideal")

    if not merge_par.empty:
        ax.plot(merge_par.index, merge_par.values,
                "o-", color=PALETTE["merge_par"], linewidth=2, markersize=7,
                label="Merge Par", markerfacecolor="white", markeredgewidth=2)

    if not bitonic_par.empty:
        ax.plot(bitonic_par.index, bitonic_par.values,
                "s-", color=PALETTE["bitonic_par"], linewidth=2, markersize=7,
                label="Bitonic Par", markerfacecolor="white", markeredgewidth=2)

    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)
    ax.set_ylim(bottom=0)

# ─────────────────────────────────────────────────────────────
# Update: Efficiency Panel
# ─────────────────────────────────────────────────────────────
def update_efficiency_panel(df, ax):
    ax.cla()
    ax.set_facecolor("#FAFAFA")
    ax.set_title("Efficiency per Dataset Type", fontweight="bold")
    ax.set_xlabel("Jumlah Thread")
    ax.set_ylabel("Efficiency")

    dataset_types = df["dataset_type"].unique()
    colors_merge   = ["#1565C0", "#1976D2", "#1E88E5", "#42A5F5"]
    colors_bitonic = ["#BF360C", "#D84315", "#E64A19", "#FF5722"]

    for i, ds_type in enumerate(dataset_types):
        sub = df[df["dataset_type"] == ds_type]
        merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                        (sub["cutoff_threshold"] == 1024)].groupby("thread_count")["efficiency"].mean()

        if not merge_par.empty:
            ax.plot(merge_par.index, merge_par.values,
                    "o:", color=colors_merge[i % len(colors_merge)], linewidth=1.5,
                    markersize=5, label=f"M-{ds_type[:3]}", alpha=0.8)

    ax.axhline(y=1.0, color="gray", linestyle="--", linewidth=1.5,
               alpha=0.5, label="Ideal")
    ax.legend(fontsize=7, loc="upper right")
    ax.grid(alpha=0.3)
    ax.set_ylim(0, 1.5)

# ─────────────────────────────────────────────────────────────
# Update: Thread Progress Panel
# ─────────────────────────────────────────────────────────────
def update_threads_panel(df, ax):
    ax.cla()
    ax.set_facecolor("#FAFAFA")
    ax.set_title("Data Count per Thread Config", fontweight="bold")
    ax.set_xlabel("Thread Count")
    ax.set_ylabel("Jumlah Benchmark Selesai")

    thread_counts = df.groupby(["algorithm", "thread_count"]).size().reset_index(name="count")

    for algo in df["algorithm"].unique():
        sub = thread_counts[thread_counts["algorithm"] == algo]
        color = PALETTE["merge_par"] if "merge" in algo else PALETTE["bitonic_par"]
        ax.bar(sub["thread_count"] + (0.2 if "merge" in algo else -0.2),
               sub["count"], width=0.35, label=algo.replace("_", " "),
               color=color, alpha=0.75)

    ax.legend(fontsize=7, loc="upper left")
    ax.grid(axis="y", alpha=0.3)

# ─────────────────────────────────────────────────────────────
# Update: Status Panel (teks info)
# ─────────────────────────────────────────────────────────────
def update_status_panel(df, ax):
    ax.cla()
    ax.set_facecolor("#E8EAF6")
    ax.axis("off")
    ax.set_title("Status Benchmark", fontweight="bold")

    total_rows     = len(df)
    algorithms     = df["algorithm"].nunique()
    dataset_types  = df["dataset_type"].nunique()
    threads_done   = df["thread_count"].nunique()
    last_algo      = df.iloc[-1]["algorithm"] if total_rows > 0 else "N/A"
    last_ds        = df.iloc[-1]["dataset_type"] if total_rows > 0 else "N/A"
    last_time      = df.iloc[-1]["execution_time"] if total_rows > 0 else 0
    last_speedup   = df.iloc[-1]["speedup"] if total_rows > 0 else 0
    best_speedup   = df["speedup"].max() if total_rows > 0 else 0
    best_algo      = df.loc[df["speedup"].idxmax(), "algorithm"] if total_rows > 0 else "N/A"

    info = [
        f"Total Rows : {total_rows}",
        f"Algoritma  : {algorithms}",
        f"Dataset    : {dataset_types} tipe",
        f"Thread     : {threads_done} konfigurasi",
        "",
        "── Last Entry ──",
        f"Algo  : {last_algo}",
        f"Data  : {last_ds}",
        f"Time  : {last_time:.2f} ms",
        f"Speedup: {last_speedup:.3f}x",
        "",
        "── Best Speedup ──",
        f"{best_speedup:.3f}x",
        f"({best_algo})",
    ]

    y_pos = 0.97
    for line in info:
        weight = "bold" if "──" in line else "normal"
        color  = "#1A237E" if "──" in line else "#212121"
        ax.text(0.05, y_pos, line, transform=ax.transAxes,
                fontsize=9, verticalalignment="top",
                fontfamily="monospace", fontweight=weight, color=color)
        y_pos -= 0.065

    # Timestamp update
    ax.text(0.05, 0.04, f"Update: {time.strftime('%H:%M:%S')}",
            transform=ax.transAxes, fontsize=8, color="#757575",
            fontfamily="monospace")

# ─────────────────────────────────────────────────────────────
# Animate Function (dipanggil setiap interval)
# ─────────────────────────────────────────────────────────────
def animate(frame):
    global last_row_count, last_modified

    # Cek apakah file berubah
    if not os.path.exists(CSV_PATH):
        ax_status.cla()
        ax_status.set_facecolor("#FFEBEE")
        ax_status.axis("off")
        ax_status.text(0.5, 0.5, f"Menunggu\n{CSV_PATH}...",
                       transform=ax_status.transAxes,
                       ha="center", va="center", fontsize=11, color="#C62828")
        return

    current_mtime = os.path.getmtime(CSV_PATH)
    df = load_csv()

    if df is None or len(df) == 0:
        return

    current_count = len(df)

    # Update semua panel
    update_time_panel(df, ax_time)
    update_speedup_panel(df, ax_speedup)
    update_efficiency_panel(df, ax_eff)
    update_threads_panel(df, ax_threads)
    update_status_panel(df, ax_status)

    if current_count != last_row_count:
        print(f"[UPDATE] {current_count} rows ({current_count - last_row_count} baru)")
        last_row_count = current_count

    last_modified = current_mtime

# ─────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────
def main():
    print("=" * 60)
    print(" Real-Time Benchmark Monitor")
    print(" Memantau:", CSV_PATH)
    print(f" Update interval: {UPDATE_INTERVAL/1000:.1f} detik")
    print(" Tekan Ctrl+C atau tutup window untuk keluar")
    print("=" * 60)

    # Jalankan FuncAnimation
    ani = animation.FuncAnimation(
        fig,
        animate,
        interval=UPDATE_INTERVAL,
        cache_frame_data=False
    )

    plt.show()
    return ani  # Simpan referensi agar GC tidak hapus

if __name__ == "__main__":
    ani = main()
