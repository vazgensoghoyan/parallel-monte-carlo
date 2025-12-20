import subprocess
import matplotlib.pyplot as plt

exe_path = "../build/MonteCarloVolume"
input_file = "input.txt"
output_file = "output.txt"

num_runs = 3

schedules = ["static", "dynamic"]
experiments = [
    {"realization": 2, "label": "R2"},
    {"realization": 3, "label": "R3"}
]

results = {}

print("=== Experiments: fixed threads, varying schedule ===\n")

for exp in experiments:
    label = exp["label"]
    results[label] = []
    print(f"--- Experiment: {label} ---")
    
    for kind in schedules:
        print(f"Schedule: {kind}")
        times = []
        for run in range(num_runs):
            print(f"  Run {run+1} of {num_runs}...", end=" ", flush=True)
            cmd = [
                exe_path,
                "--input", input_file,
                "--output", output_file,
                "--realization", str(exp["realization"]),
                "--kind", kind
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

plt.figure(figsize=(8,6))
x_pos = range(len(schedules))

for label, avg_times in results.items():
    plt.plot(x_pos, avg_times, marker='o', linestyle='-', label=label)

plt.xticks(x_pos, schedules)
plt.xlabel("Schedule (kind)")
plt.ylabel("Execution time (ms)")
plt.title(f"Execution time vs Schedule (threads=default, chunk_size=default)")
plt.grid(True)
plt.legend(title="Implementation")
plt.tight_layout()
plt.savefig("time_vs_schedule.png")
plt.show()

print("\n=== Experiment finished. Graph saved to assets/time_vs_schedule.png ===")
