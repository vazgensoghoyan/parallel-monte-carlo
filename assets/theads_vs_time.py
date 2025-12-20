import subprocess
import matplotlib.pyplot as plt

exe_path = "../build/MonteCarloVolume"
input_file = "../input.txt"
output_file = "../output.txt"
realization = 2
kind = "static"
chunk_size = 20
num_threads_list = [1 << i for i in range(0, 5)]
num_runs = 3

avg_times = []

print("=== Starting Monte Carlo timing experiment ===")
for threads in num_threads_list:
    print(f"\n--- Running for {threads} thread(s) ---")
    times = []
    for run in range(num_runs):
        print(f"Run {run + 1} of {num_runs}...", end=" ", flush=True)

        cmd = [
            exe_path,
            "--input", input_file,
            "--output", output_file,
            "--realization", str(realization),
            "--threads", str(threads),
            "--kind", kind,
            "--chunk_size", str(chunk_size)
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

    if times:
        avg_time = sum(times) / len(times)
        avg_times.append(avg_time)
        print(f"Average time for {threads} thread(s): {avg_time:.2f} ms")
    else:
        avg_times.append(None)
        print(f"No valid measurements for {threads} thread(s)")

plt.figure(figsize=(8, 6))
plt.plot(num_threads_list, avg_times, marker='o', linestyle='-')
plt.xlabel("Number of threads")
plt.ylabel("Execution time (ms)")
plt.title(f"Execution time vs threads (kind={kind}, chunk_size={chunk_size})")
plt.grid(True)
plt.xticks(num_threads_list)
plt.tight_layout()
plt.savefig("threads_vs_time.png")
plt.show()

print("\n=== Experiment finished. Graph saved to assets/threads_vs_time.png ===")
