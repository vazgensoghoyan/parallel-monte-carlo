import subprocess
import matplotlib.pyplot as plt

exe_path = "../build/MonteCarloVolume"
input_file = "input.txt"
output_file = "output.txt"

num_runs = 3

chunk_sizes = [1 << i for i in range(10)]  # 1, ..., 512

experiments = [
    {"realization": 2, "kind": "static", "label": "R2 static"},
    {"realization": 2, "kind": "dynamic", "label": "R2 dynamic"},
    {"realization": 3, "kind": "static", "label": "R3 static"},
    {"realization": 3, "kind": "dynamic", "label": "R3 dynamic"}
]

results = {exp["label"]: [] for exp in experiments}

print("=== Experiments: Execution time vs chunk_size ===\n")

for exp in experiments:
    label = exp["label"]
    print(f"--- Experiment: {label} ---")

    for chunk in chunk_sizes:
        print(f"Chunk size: {chunk}")
        times = []
        for run in range(num_runs):
            print(f"  Run {run+1} of {num_runs}...", end=" ", flush=True)
            cmd = [
                exe_path,
                "--input", input_file,
                "--output", output_file,
                "--realization", str(exp["realization"]),
                "--kind", exp["kind"],
                "--chunk_size", str(chunk)
            ]
            result = subprocess.run(cmd, capture_output=True, text=True)
            stdout = result.stdout.strip()
            try:
                time_ms = float(stdout.split(":")[1].split()[0])
            except (IndexError, ValueError):
                print("\nError parsing output:")
                print(stdout)
                continue
            times.append(time_ms)
            print(f"Done, time = {time_ms:.2f} ms")

        avg_time = sum(times) / len(times) if times else None
        results[label].append(avg_time)
        print(f"  Average time: {avg_time:.2f} ms\n")

plt.figure(figsize=(10,6))

for label, avg_times in results.items():
    plt.plot(chunk_sizes, avg_times, marker='o', linestyle='-', label=label)

plt.xscale("log")  # логарифмическая ось для chunk_size
plt.xlabel("Chunk size")
plt.ylabel("Execution time (ms)")
plt.title("Execution time vs Chunk size for Monte Carlo Volume (piriform a=2)")
plt.grid(True, which="both", ls="--")
plt.legend(title="Implementation & Schedule")
plt.tight_layout()
plt.savefig("time_vs_chunk_size.png")
plt.show()

print("\n=== Experiment finished. Graph saved to assets/time_vs_chunk_size.png ===")
