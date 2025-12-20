import subprocess
import matplotlib.pyplot as plt

exe_path = "../build/MonteCarloVolume"
input_file = "input.txt"
output_file = "output.txt"
num_threads_list = [1 << i for i in range(5)]  # 1,2,4,8,16
num_runs = 3

experiments = [
    {"realization": 2, "kind": "static", "label": "R2 static"},
    {"realization": 2, "kind": "dynamic", "label": "R2 dynamic"},
    {"realization": 3, "kind": "static", "label": "R3 static"},
    {"realization": 3, "kind": "dynamic", "label": "R3 dynamic"}
]

results = {}

print("=== Starting Monte Carlo timing experiments ===\n")

for exp in experiments:
    results[exp["label"]] = []
    print(f"--- Experiment: {exp['label']} ---")

    for threads in num_threads_list:
        print(f"Threads: {threads}")
        times = []
        for run in range(num_runs):
            print(f"  Run {run+1} of {num_runs}...", end=" ", flush=True)

            cmd = [
                exe_path,
                "--input", input_file,
                "--output", output_file,
                "--realization", str(exp["realization"]),
                "--threads", str(threads),
                "--kind", exp["kind"]
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
        results[exp["label"]].append(avg_time)
        print(f"  Average time: {avg_time:.2f} ms\n")

plt.figure(figsize=(10,6))

for label, avg_times in results.items():
    plt.plot(num_threads_list, avg_times, marker='o', linestyle='-', label=label)

plt.xlabel("Number of threads")
plt.ylabel("Execution time (ms)")
plt.title("Execution time vs Threads for Monte Carlo Volume (piriform a=2)")
plt.grid(True)
plt.xticks(num_threads_list)
plt.legend(title="Implementation & Schedule")
plt.tight_layout()
plt.savefig("threads_vs_time_static_dynamic.png")
plt.show()

print("\n=== Experiment finished. Graph saved to assets/threads_vs_time_static_dynamic.png ===")
