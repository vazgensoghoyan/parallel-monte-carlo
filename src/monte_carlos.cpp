#include "monte_carlos.hpp"

inline double count_hits(size_t start, size_t end, float x_min, float x_max,
                         float y_min, float y_max, float z_min, float z_max,
                         RandomGenerator& rng) {
    double hits = 0.0;
    for (size_t i = start; i < end; ++i) {
        float x = x_min + rng.next_float() * (x_max - x_min);
        float y = y_min + rng.next_float() * (y_max - y_min);
        float z = z_min + rng.next_float() * (z_max - z_min);

        if (hit_test(x, y, z))
            hits += 1.0;
    }
    return hits;
}

inline void get_ranges(float& x_min, float& x_max, float& y_min, 
                       float& y_max, float& z_min, float& z_max) {
    const float* ranges = get_axis_range();

    x_min = ranges[0]; x_max = ranges[1];
    y_min = ranges[2]; y_max = ranges[3];
    z_min = ranges[4]; z_max = ranges[5];
}

inline double box_volume(float x_min, float x_max, float y_min, 
                         float y_max, float z_min, float z_max) {
    return (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
}

double monte_carlo_single(size_t N) {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max);

    RandomGenerator rng;
    double hits = count_hits(0, N, x_min, x_max, y_min, y_max, z_min, z_max, rng);

    return box_volume(x_min, x_max, y_min, y_max, z_min, z_max) * hits / N;
}

double monte_carlo_auto_parallel(size_t N, const Arguments& args) {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max);

    double hits = 0.0;
    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());

    omp_sched_t schedule_kind = omp_sched_static;
    if (args.kind == ScheduleKind::Dynamic)
        schedule_kind = omp_sched_dynamic;

    int chunk = (args.chunk_size > 0 ? args.chunk_size : 1);
    omp_set_num_threads(num_threads);
    omp_set_schedule(schedule_kind, chunk);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        RandomGenerator rng(tid);
        double local_hits = 0.0;

        #pragma omp for schedule(runtime)
        for (size_t i = 0; i < N; ++i) {
            float x = x_min + rng.next_float() * (x_max - x_min);
            float y = y_min + rng.next_float() * (y_max - y_min);
            float z = z_min + rng.next_float() * (z_max - z_min);

            if (hit_test(x, y, z))
                local_hits += 1.0;
        }

        #pragma omp atomic
        hits += local_hits;
    }

    return box_volume(x_min, x_max, y_min, y_max, z_min, z_max) * hits / N;
}

double monte_carlo_manual_parallel(size_t N, const Arguments& args) {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    get_ranges(x_min, x_max, y_min, y_max, z_min, z_max);

    double hits = 0.0;
    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());
    omp_set_num_threads(num_threads);

    int chunk = (args.chunk_size > 0 ? args.chunk_size : 1);

    size_t num_chunks = (N + chunk - 1) / chunk;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        RandomGenerator rng(tid);
        double local_hits = 0.0;

        if (args.kind == ScheduleKind::Dynamic) {

            static std::atomic<size_t> next_chunk{0};

            while (true) {
                size_t chunk_id = next_chunk.fetch_add(1);

                if (chunk_id >= num_chunks) break;

                size_t start = chunk_id * chunk;
                size_t end = std::min(start + chunk, N);

                local_hits += count_hits(start, end, x_min, x_max, y_min, y_max, z_min, z_max, rng);
            }
        } else { // Static and Auto

            for (size_t chunk_id = tid; chunk_id < num_chunks; chunk_id += num_threads) {
                size_t start = chunk_id * chunk;
                size_t end = std::min(start + chunk, N);

                local_hits += count_hits(start, end, x_min, x_max, y_min, y_max, z_min, z_max, rng);
            }
        }

        #pragma omp atomic
        hits += local_hits;
    }

    return box_volume(x_min, x_max, y_min, y_max, z_min, z_max) * hits / N;
}

double calculate_volume(size_t N, const Arguments& args) {
    switch (args.realization) {
        case 1: return monte_carlo_single(N);
        case 2: return monte_carlo_auto_parallel(N, args);
        case 3: return monte_carlo_manual_parallel(N, args);
        default: throw std::invalid_argument("Invalid realization number");
    }
}
