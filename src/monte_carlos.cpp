#include "monte_carlos.hpp"

static inline double count_hits(size_t start, size_t end,
                         double x_min, double x_max, double y_min, 
                         double y_max, double z_min, double z_max,
                         double dx, double dy, double dz,
                         RandomGenerator& rng) {
    double hits = 0.0;

    for (size_t i = start; i < end; ++i) {
        double x = x_min + rng.next_double() * dx;
        double y = y_min + rng.next_double() * dy;
        double z = z_min + rng.next_double() * dz;

        if (hit_test(x, y, z))
            hits += 1.0;
    }

    return hits;
}

static inline void get_ranges(double& x_min, double& x_max,
                              double& y_min, double& y_max,
                              double& z_min, double& z_max,
                              double& dx, double& dy, double& dz) {
    const float* ranges = get_axis_range();
    
    x_min = ranges[0]; x_max = ranges[1];
    y_min = ranges[2]; y_max = ranges[3];
    z_min = ranges[4]; z_max = ranges[5];

    dx = x_max - x_min;
    dy = y_max - y_min;
    dz = z_max - z_min;
}

double monte_carlo_single(size_t N) {
    double x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz);

    RandomGenerator rng;
    double hits = count_hits(0, N, x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz, rng);

    return dx * dy * dz * hits / N;
}

double monte_carlo_auto_parallel(size_t N, const Arguments& args) {
    double x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz);

    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());
    omp_sched_t schedule_kind = (args.kind == ScheduleKind::Dynamic) ? omp_sched_dynamic : omp_sched_static;
    int chunk = (args.chunk_size > 0 ? args.chunk_size : 1);

    omp_set_num_threads(num_threads);
    omp_set_schedule(schedule_kind, chunk);

    std::vector<double> local_hits(num_threads, 0.0);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        RandomGenerator rng(tid);

        double hits_thread = 0.0;

        #pragma omp for schedule(runtime)
        for (size_t i = 0; i < N; ++i) {
            double x = x_min + rng.next_double() * (x_max - x_min);
            double y = y_min + rng.next_double() * (y_max - y_min);
            double z = z_min + rng.next_double() * (z_max - z_min);

            if (hit_test(x, y, z))
                hits_thread += 1.0;
        }
        local_hits[tid] = hits_thread;
    }

    double hits = 0.0;
    for (double h : local_hits) hits += h;

    return dx * dy * dz * hits / N;
}

double monte_carlo_manual_parallel(size_t N, const Arguments& args) {
    double x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz);

    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());
    omp_set_num_threads(num_threads);

    int chunk = (args.chunk_size > 0 ? args.chunk_size : 1);
    size_t num_chunks = (N + chunk - 1) / chunk;

    std::vector<double> local_hits(num_threads, 0.0);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        RandomGenerator rng(tid);

        double hits_thread = 0.0;

        if (args.kind == ScheduleKind::Dynamic) {

            static std::atomic<size_t> next_chunk{0};

            while (true) {
                size_t chunk_id = next_chunk.fetch_add(1);
                if (chunk_id >= num_chunks) break;

                size_t start = chunk_id * chunk;
                size_t end = std::min(start + chunk, N);
                
                hits_thread += count_hits(start, end, x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz, rng);
            }
        } else { // Static or Auto

            for (size_t chunk_id = tid; chunk_id < num_chunks; chunk_id += num_threads) {
                
                size_t start = chunk_id * chunk;
                size_t end = std::min(start + chunk, N);
                
                hits_thread += count_hits(start, end, x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, dz, rng);
            }
        }

        local_hits[tid] = hits_thread;
    }

    double hits = 0.0;
    for (double h : local_hits) hits += h;

    return dx * dy * dz * hits / N;
}

double calculate_volume(size_t N, const Arguments& args) {
    switch (args.realization) {
        case 1: return monte_carlo_single(N);
        case 2: return monte_carlo_auto_parallel(N, args);
        case 3: return monte_carlo_manual_parallel(N, args);
        default: throw std::invalid_argument("Invalid realization number");
    }
}
