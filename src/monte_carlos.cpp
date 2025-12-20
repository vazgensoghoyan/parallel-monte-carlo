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
        RandomGenerator rng;  // генератор на поток
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

double monte_carlo_manual_parallel(size_t N) {
    throw std::logic_error("Realization 3 not implemented yet");
}
