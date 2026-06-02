#!/usr/bin/env python3
"""
plot.py - Static Visualization: Analisis Speedup Parallel Sorting
Membuat grafik comparative Merge Sort vs Bitonic Sort dari results/results.csv
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np
import sys
import os

# ─────────────────────────────────────────────────────────────
# Konstanta & Style
# ─────────────────────────────────────────────────────────────
CSV_PATH   = "results/results.csv"
OUTPUT_DIR = "results"

PALETTE = {
    "merge_seq":   "#2196F3",  # Biru
    "merge_par":   "#1565C0",  # Biru gelap
    "bitonic_seq": "#FF5722",  # Oranye
    "bitonic_par": "#BF360C",  # Oranye gelap
}

plt.rcParams.update({
    "font.family":     "DejaVu Sans",
    "font.size":       10,
    "axes.titlesize":  11,
    "axes.labelsize":  10,
    "legend.fontsize": 9,
    "figure.dpi":      150,
})

# ─────────────────────────────────────────────────────────────
# Load Data
# ─────────────────────────────────────────────────────────────
def load_data(path=CSV_PATH):
    if not os.path.exists(path):
        print(f"[ERROR] File tidak ditemukan: {path}")
        print("[INFO] Jalankan benchmark terlebih dahulu: benchmark.exe")
        sys.exit(1)

    df = pd.read_csv(path)
    df.columns = df.columns.str.strip()
    print(f"[INFO] Loaded {len(df)} rows dari {path}")
    return df

# ─────────────────────────────────────────────────────────────
# Grafik 1: Execution Time Comparison
# ─────────────────────────────────────────────────────────────
def plot_execution_time(df, axes):
    dataset_types = df["dataset_type"].unique()
    n_types = len(dataset_types)

    for idx, ds_type in enumerate(dataset_types):
        ax = axes[idx // 2][idx % 2]
        sub = df[df["dataset_type"] == ds_type]

        # Merge Sort Par: gunakan cutoff=1024
        merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                        (sub["cutoff_threshold"] == 1024)].copy()
        merge_par = merge_par.groupby("thread_count")["execution_time"].mean().reset_index()

        # Bitonic Sort Par: gunakan schedule=static
        bitonic_par = sub[(sub["algorithm"] == "bitonic_sort_par") &
                          (sub["schedule_type"] == "static")].copy()
        bitonic_par = bitonic_par.groupby("thread_count")["execution_time"].mean().reset_index()

        # Sequential baselines
        merge_seq_time = sub[sub["algorithm"] == "merge_sort_seq"]["execution_time"].mean()
        bitonic_seq_time = sub[sub["algorithm"] == "bitonic_sort_seq"]["execution_time"].mean()

        threads = sorted(merge_par["thread_count"].unique())

        x = np.arange(len(threads))
        width = 0.35

        merge_times   = [merge_par[merge_par["thread_count"]==t]["execution_time"].values[0]
                         if t in merge_par["thread_count"].values else 0 for t in threads]
        bitonic_times = [bitonic_par[bitonic_par["thread_count"]==t]["execution_time"].values[0]
                         if t in bitonic_par["thread_count"].values else 0 for t in threads]

        bars1 = ax.bar(x - width/2, merge_times, width, label="Merge Sort Par",
                       color=PALETTE["merge_par"], alpha=0.85, edgecolor="white")
        bars2 = ax.bar(x + width/2, bitonic_times, width, label="Bitonic Sort Par",
                       color=PALETTE["bitonic_par"], alpha=0.85, edgecolor="white")

        # Baseline lines
        ax.axhline(y=merge_seq_time,   color=PALETTE["merge_seq"],   linestyle="--",
                   linewidth=1.5, label=f"Merge Seq ({merge_seq_time:.0f}ms)", alpha=0.8)
        ax.axhline(y=bitonic_seq_time, color=PALETTE["bitonic_seq"], linestyle=":",
                   linewidth=1.5, label=f"Bitonic Seq ({bitonic_seq_time:.0f}ms)", alpha=0.8)

        ax.set_title(f"Dataset: {ds_type.replace('_', ' ').title()}")
        ax.set_xlabel("Jumlah Thread")
        ax.set_ylabel("Execution Time (ms)")
        ax.set_xticks(x)
        ax.set_xticklabels([str(t) for t in threads])
        ax.legend(fontsize=8, loc="upper right")
        ax.grid(axis="y", alpha=0.3)
        ax.set_ylim(bottom=0)

# ─────────────────────────────────────────────────────────────
# Grafik 2: Speedup Comparison
# ─────────────────────────────────────────────────────────────
def plot_speedup(df, axes):
    dataset_types = df["dataset_type"].unique()
    threads = sorted(df["thread_count"].unique())
    threads = [t for t in threads if t > 1]  # hanya > 1

    for idx, ds_type in enumerate(dataset_types):
        ax = axes[idx // 2][idx % 2]
        sub = df[df["dataset_type"] == ds_type]

        # Merge Sort Par speedup
        merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                        (sub["cutoff_threshold"] == 1024)]
        merge_speedup = merge_par.groupby("thread_count")["speedup"].mean()

        # Bitonic Sort Par speedup
        bitonic_par = sub[(sub["algorithm"] == "bitonic_sort_par") &
                          (sub["schedule_type"] == "static")]
        bitonic_speedup = bitonic_par.groupby("thread_count")["speedup"].mean()

        all_threads = sorted(set(list(merge_speedup.index) + list(bitonic_speedup.index)))

        ax.plot(all_threads,
                [merge_speedup.get(t, np.nan) for t in all_threads],
                "o-", color=PALETTE["merge_par"], linewidth=2,
                markersize=7, label="Merge Sort Par", markerfacecolor="white",
                markeredgewidth=2)
        ax.plot(all_threads,
                [bitonic_speedup.get(t, np.nan) for t in all_threads],
                "s-", color=PALETTE["bitonic_par"], linewidth=2,
                markersize=7, label="Bitonic Sort Par", markerfacecolor="white",
                markeredgewidth=2)

        # Ideal speedup (linear)
        ax.plot(all_threads, all_threads, "--", color="gray",
                linewidth=1.5, alpha=0.6, label="Ideal (Linear)")

        # Kurva Amdahl's Law (f=20.2% serial fraction dari data empiris)
        f = 0.202
        p_range = np.linspace(1, max(all_threads), 100)
        amdahl  = [1 / (f + (1 - f) / p) for p in p_range]
        ax.plot(p_range, amdahl, "r:", linewidth=2,
                alpha=0.8, label=f"Amdahl (f=20.2%)")

        ax.set_title(f"Dataset: {ds_type.replace('_', ' ').title()}")
        ax.set_xlabel("Jumlah Thread")
        ax.set_ylabel("Speedup")
        ax.legend(fontsize=8)
        ax.grid(alpha=0.3)
        ax.set_ylim(bottom=0)
        ax.set_xticks(all_threads)

# ─────────────────────────────────────────────────────────────
# Grafik 3: Efficiency Comparison
# ─────────────────────────────────────────────────────────────
def plot_efficiency(df, axes):
    dataset_types = df["dataset_type"].unique()

    for idx, ds_type in enumerate(dataset_types):
        ax = axes[idx // 2][idx % 2]
        sub = df[df["dataset_type"] == ds_type]

        merge_par = sub[(sub["algorithm"] == "merge_sort_par") &
                        (sub["cutoff_threshold"] == 1024)]
        merge_eff = merge_par.groupby("thread_count")["efficiency"].mean()

        bitonic_par = sub[(sub["algorithm"] == "bitonic_sort_par") &
                          (sub["schedule_type"] == "static")]
        bitonic_eff = bitonic_par.groupby("thread_count")["efficiency"].mean()

        all_threads = sorted(set(list(merge_eff.index) + list(bitonic_eff.index)))

        ax.plot(all_threads,
                [merge_eff.get(t, np.nan) for t in all_threads],
                "o-", color=PALETTE["merge_par"], linewidth=2,
                markersize=7, label="Merge Sort Par", markerfacecolor="white",
                markeredgewidth=2)
        ax.plot(all_threads,
                [bitonic_eff.get(t, np.nan) for t in all_threads],
                "s-", color=PALETTE["bitonic_par"], linewidth=2,
                markersize=7, label="Bitonic Sort Par", markerfacecolor="white",
                markeredgewidth=2)

        ax.axhline(y=1.0, color="gray", linestyle="--",
                   linewidth=1.5, alpha=0.6, label="Ideal Efficiency (1.0)")

        ax.set_title(f"Dataset: {ds_type.replace('_', ' ').title()}")
        ax.set_xlabel("Jumlah Thread")
        ax.set_ylabel("Efficiency (Speedup / Thread)")
        ax.legend(fontsize=8)
        ax.grid(alpha=0.3)
        ax.set_ylim(0, 1.3)
        ax.set_xticks(all_threads)

# ─────────────────────────────────────────────────────────────
# Grafik 4: Dataset Distribution Comparison
# ─────────────────────────────────────────────────────────────
def plot_dataset_comparison(df, ax):
    """Bandingkan execution time per dataset type untuk setiap algoritma"""
    dataset_types = df["dataset_type"].unique()

    # Hanya data sequential untuk perbandingan murni dataset effect
    merge_seq = df[df["algorithm"] == "merge_sort_seq"].groupby("dataset_type")["execution_time"].mean()
    bitonic_seq = df[df["algorithm"] == "bitonic_sort_seq"].groupby("dataset_type")["execution_time"].mean()

    # Data parallel terbaik (thread=8)
    merge_par_best = df[(df["algorithm"] == "merge_sort_par") &
                        (df["thread_count"] == 8) &
                        (df["cutoff_threshold"] == 1024)].groupby("dataset_type")["execution_time"].mean()
    bitonic_par_best = df[(df["algorithm"] == "bitonic_sort_par") &
                          (df["thread_count"] == 8) &
                          (df["schedule_type"] == "static")].groupby("dataset_type")["execution_time"].mean()

    x = np.arange(len(dataset_types))
    width = 0.2
    labels = [d.replace("_", "\n").title() for d in dataset_types]

    ax.bar(x - 1.5*width, [merge_seq.get(d, 0) for d in dataset_types],
           width, label="Merge Seq", color=PALETTE["merge_seq"], alpha=0.85)
    ax.bar(x - 0.5*width, [merge_par_best.get(d, 0) for d in dataset_types],
           width, label="Merge Par (T=8)", color=PALETTE["merge_par"], alpha=0.85)
    ax.bar(x + 0.5*width, [bitonic_seq.get(d, 0) for d in dataset_types],
           width, label="Bitonic Seq", color=PALETTE["bitonic_seq"], alpha=0.85)
    ax.bar(x + 1.5*width, [bitonic_par_best.get(d, 0) for d in dataset_types],
           width, label="Bitonic Par (T=8)", color=PALETTE["bitonic_par"], alpha=0.85)

    ax.set_title("Perbandingan Execution Time per Tipe Dataset")
    ax.set_xlabel("Tipe Dataset")
    ax.set_ylabel("Execution Time (ms)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, fontsize=9)
    ax.legend(fontsize=8, loc="upper right")
    ax.grid(axis="y", alpha=0.3)
    ax.set_ylim(bottom=0)


# ─────────────────────────────────────────────────────────────
# Grafik 5: Waktu vs Karakteristik Dataset (pengganti waktu vs ukuran)
# Menunjukkan bagaimana karakteristik data mempengaruhi waktu eksekusi
# ─────────────────────────────────────────────────────────────
def plot_time_vs_dataset_char(df, ax):
    """
    Grafik Waktu Eksekusi vs Karakteristik Dataset
    Menggantikan grafik waktu vs ukuran masalah karena semua dataset
    menggunakan ukuran yang sama (1 juta elemen).
    Menunjukkan pengaruh distribusi data terhadap performa algoritma.
    """
    # Urutan dari paling ringan ke paling berat berdasarkan waktu sequential (results.csv):
    # reverse=96.8ms → nearly_sorted=101.1ms → duplicates=124.8ms → random=154.4ms
    dataset_order  = ["reverse", "nearly_sorted", "duplicates", "random"]
    dataset_labels = ["Reverse\n96.8ms\n(paling ringan)", 
                      "Nearly Sorted\n101.1ms\n(ringan)", 
                      "Duplicates\n124.8ms\n(sedang)",
                      "Random\n154.4ms\n(paling berat)"]
    threads = [1, 2, 4, 8]
    colors  = ["#1565C0", "#1976D2", "#42A5F5", "#90CAF9"]

    x     = np.arange(len(dataset_order))
    width = 0.2

    for i, t in enumerate(threads):
        if t == 1:
            # T=1 ambil dari merge_sort_par thread=1
            times = []
            for ds in dataset_order:
                val = df[(df["algorithm"]=="merge_sort_par") &
                         (df["dataset_type"]==ds) &
                         (df["thread_count"]==1) &
                         (df["cutoff_threshold"]==1024)]["execution_time"].mean()
                times.append(val if not np.isnan(val) else 0)
        else:
            times = []
            for ds in dataset_order:
                val = df[(df["algorithm"]=="merge_sort_par") &
                         (df["dataset_type"]==ds) &
                         (df["thread_count"]==t) &
                         (df["cutoff_threshold"]==1024)]["execution_time"].mean()
                times.append(val if not np.isnan(val) else 0)

        offset = (i - 1.5) * width
        ax.bar(x + offset, times, width, label=f"T={t} thread",
               color=colors[i], alpha=0.85, edgecolor="white")

    # Tambah sequential baseline
    seq_times = []
    for ds in dataset_order:
        val = df[(df["algorithm"]=="merge_sort_seq") &
                 (df["dataset_type"]==ds)]["execution_time"].mean()
        seq_times.append(val if not np.isnan(val) else 0)

    ax.plot(x, seq_times, "ro-", linewidth=2, markersize=8,
            label="Sequential baseline", zorder=5)

    ax.set_xlabel("Dataset (diurutkan dari waktu sequential terkecil ke terbesar — sumber: results.csv)", fontsize=10)
    ax.set_ylabel("Execution Time (ms)", fontsize=11)
    ax.set_title("Grafik Waktu Eksekusi vs Karakteristik Dataset\n"
                 "Merge Sort Paralel — N=1.000.000 elemen | Diurutkan berdasarkan T_seq", 
                 fontsize=12, fontweight="bold")
    ax.set_xticks(x)
    ax.set_xticklabels(dataset_labels, fontsize=9)
    ax.legend(fontsize=9, loc="upper right")
    ax.grid(axis="y", alpha=0.3)
    ax.set_ylim(bottom=0)

    # Anotasi penurunan waktu
    ax.annotate("Semakin banyak thread\nsemakin pendek batang",
                xy=(3, 48), xytext=(2.0, 100),
                arrowprops=dict(arrowstyle="->", color="navy"),
                fontsize=9, color="navy")
    ax.annotate("Urutan berdasarkan\nwaktu sequential\ndari results.csv",
                xy=(0, 30), xytext=(0.3, 110),
                arrowprops=dict(arrowstyle="->", color="gray"),
                fontsize=8, color="gray")

# ─────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────
def main():
    df = load_data()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # ── Figure 1: Execution Time ─────────────────────────────
    fig1, axes1 = plt.subplots(2, 2, figsize=(14, 10))
    fig1.suptitle(
        "Execution Time Comparison: Merge Sort vs Bitonic Sort (OpenMP)",
        fontsize=13, fontweight="bold", y=1.01
    )
    plot_execution_time(df, axes1)
    plt.tight_layout()
    fig1.savefig(f"{OUTPUT_DIR}/plot_execution_time.png", bbox_inches="tight")
    print(f"[SAVED] {OUTPUT_DIR}/plot_execution_time.png")

    # ── Figure 2: Speedup ────────────────────────────────────
    fig2, axes2 = plt.subplots(2, 2, figsize=(14, 10))
    fig2.suptitle(
        "Speedup Analysis: Merge Sort vs Bitonic Sort (OpenMP)",
        fontsize=13, fontweight="bold", y=1.01
    )
    plot_speedup(df, axes2)
    plt.tight_layout()
    fig2.savefig(f"{OUTPUT_DIR}/plot_speedup.png", bbox_inches="tight")
    print(f"[SAVED] {OUTPUT_DIR}/plot_speedup.png")

    # ── Figure 3: Efficiency ─────────────────────────────────
    fig3, axes3 = plt.subplots(2, 2, figsize=(14, 10))
    fig3.suptitle(
        "Parallel Efficiency: Merge Sort vs Bitonic Sort (OpenMP)",
        fontsize=13, fontweight="bold", y=1.01
    )
    plot_efficiency(df, axes3)
    plt.tight_layout()
    fig3.savefig(f"{OUTPUT_DIR}/plot_efficiency.png", bbox_inches="tight")
    print(f"[SAVED] {OUTPUT_DIR}/plot_efficiency.png")

    # ── Figure 4: Dataset Distribution ──────────────────────
    fig4, ax4 = plt.subplots(figsize=(12, 6))
    plot_dataset_comparison(df, ax4)
    plt.tight_layout()
    fig4.savefig(f"{OUTPUT_DIR}/plot_dataset_comparison.png", bbox_inches="tight")
    print(f"[SAVED] {OUTPUT_DIR}/plot_dataset_comparison.png")

    # ── Figure 5: Waktu vs Karakteristik Dataset ────────────
    fig5, ax5 = plt.subplots(figsize=(12, 6))
    plot_time_vs_dataset_char(df, ax5)
    plt.tight_layout()
    fig5.savefig(f"{OUTPUT_DIR}/plot_time_vs_dataset.png", bbox_inches="tight")
    print(f"[SAVED] {OUTPUT_DIR}/plot_time_vs_dataset.png")

    print("\n[DONE] Semua grafik tersimpan di folder results/")
    plt.show()

if __name__ == "__main__":
    main()

