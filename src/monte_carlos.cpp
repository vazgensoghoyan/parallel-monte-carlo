#include "monte_carlos.hpp"

double monte_carlo_single(size_t N) {
    const float* ranges = get_axis_range();
    float x_min = ranges[0], x_max = ranges[1];
    float y_min = ranges[2], y_max = ranges[3];
    float z_min = ranges[4], z_max = ranges[5];

    RandomGenerator rng;
    double hits = 0;

    for (size_t i = 0; i < N; ++i) {
        float x = x_min + rng.next() * (x_max - x_min);
        float y = y_min + rng.next() * (y_max - y_min);
        float z = z_min + rng.next() * (z_max - z_min);

        if (hit_test(x, y, z)) {
            hits += 1.0;
        }
    }

    double volume_box = (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
    return volume_box * hits / N;
}

double monte_carlo_auto_parallel(size_t N, const Arguments& args) {
    const float* ranges = get_axis_range();
    float x_min = ranges[0], x_max = ranges[1];
    float y_min = ranges[2], y_max = ranges[3];
    float z_min = ranges[4], z_max = ranges[5];

    double hits = 0.0;

    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());

    omp_sched_t schedule_kind;
    switch (args.kind) {
        case ScheduleKind::Static:
            schedule_kind = omp_sched_static;
            break;
        case ScheduleKind::Dynamic:
            schedule_kind = omp_sched_dynamic;
            break;
        default: // Auto -> static
            schedule_kind = omp_sched_static;
            break;
    }

    int chunk = (args.chunk_size > 0 ? args.chunk_size : 1); // если не задан, chunk = 1
    
    omp_set_num_threads(num_threads);
    omp_set_schedule(schedule_kind, chunk);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        RandomGenerator rng(tid);  // генератор на поток
        double local_hits = 0.0;

        #pragma omp for schedule(runtime) // используем runtime, т.к. мы уже установили schedule
        for (size_t i = 0; i < N; ++i) {
            float x = x_min + rng.next() * (x_max - x_min);
            float y = y_min + rng.next() * (y_max - y_min);
            float z = z_min + rng.next() * (z_max - z_min);

            if (hit_test(x, y, z))
                local_hits += 1.0;
        }

        #pragma omp atomic
        hits += local_hits;
    }

    double volume_box = (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
    return volume_box * hits / N;
}

double monte_carlo_manual_parallel(size_t N, const Arguments& args) {
    const float* ranges = get_axis_range();
    float x_min = ranges[0], x_max = ranges[1];
    float y_min = ranges[2], y_max = ranges[3];
    float z_min = ranges[4], z_max = ranges[5];

    double hits = 0.0;

    int num_threads = (args.threads > 0 ? args.threads : omp_get_max_threads());
    omp_set_num_threads(num_threads);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        RandomGenerator rng(tid);
        double local_hits = 0.0;

        size_t points_per_thread = N / nthreads;
        size_t start = tid * points_per_thread;
        size_t end = (tid == nthreads - 1) ? N : start + points_per_thread;

        for (size_t i = start; i < end; ++i) {
            float x = x_min + rng.next() * (x_max - x_min);
            float y = y_min + rng.next() * (y_max - y_min);
            float z = z_min + rng.next() * (z_max - z_min);

            if (hit_test(x, y, z))
                local_hits += 1.0;
        }

        #pragma omp atomic
        hits += local_hits;
    }

    double volume_box = (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
    return volume_box * hits / N;
}

double calculate_volume(size_t N, const Arguments& args) {
    double volume = 0.0;

    switch (args.realization) {
        case 1:
            volume = monte_carlo_single(N);
            break;
        case 2:
            volume = monte_carlo_auto_parallel(N, args);
            break;
        case 3:
            volume = monte_carlo_manual_parallel(N, args);
            break;
        default:
            throw std::invalid_argument("Invalid realization number");
    }

    return volume;
}
