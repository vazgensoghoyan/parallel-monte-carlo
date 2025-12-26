| Лабораторная работа №4    | 24Б10                 | Архитектура компьютера |
| ------------------------- | --------------------- | ---------------------- |
| Оптимизация по скорости   | Согоян Вазген Айкович | 2025                   |

## Файл с заданием

[Тут](lab4_optimization_montecarlo.pdf), файл lab4_optimization_montecarlo.pdf!

## Инструментарий

> При наличии переписываем из ТЗ, если требуется, указываем свои данные (компилятор/стандарт и т.д.).

## Что реализовано

> Пишем что было выполнено. Если что-то по ТЗ не работает, то лучше указать это здесь.

# Описание:

## Генератор случайных чисел

Использовал генератор на основе алгоритма PCG. Сначала использовал mt19337, потом XorShift32, но по итогу использую этот. Он хорошо и качественно распределяет значения и работает на большом диапозоне.

Генератор реализован в классе RandomGenerator и предоставляет два метода:
- next() — возвращает случайное 32-битное целое число;
- next_double() — возвращает случайное число в диапазоне [0, 1).

Генератор инициализируется с уникальным seed (в коде передаю туда id потока), который комбинируется с текущим временем для получения различной последовательности при каждом запуске.

## hit.cpp

hit.h не изменен, но скопирован в директорию include.

Для локального тестирования реализованы функции проверки попадания точки в фигуру и получения ограничивающего объёма:
- hit_test(float x, float y, float z) — проверяет, находится ли точка внутри тестовой фигуры piriform с параметром a = 2. Для ускорения вычислений используется функция fmaf (как из первой лабораторной). Функция возвращает true, если точка внутри фигуры, и false в противном случае.
- get_axis_range() — возвращает статический массив axis_range[6], содержащий границы ограничивающего объёма: [x_min, x_max, y_min, y_max, z_min, z_max]. Эти границы используются для генерации случайных точек внутри объёма, полностью содержащего фигуру.

# Парсинг аргументов

Парсинг аргументов командной строки

Для удобного управления параметрами программы реализован класс Arguments. Он обрабатывает все необходимые аргументы командной строки и задаёт значения по умолчанию для необязательных параметров.

Значения по умолчанию выбраны:
- chunk size = 256
- threads = omp_get_max_threads()
- kind = ScheduleKind::Auto (в методах избирается лучший вариант)

Класс проверяет корректность значений и наличие обязательных аргументов (--input, --output, --realization).

Метод read_N() читает число точек N из входного файла и проверяет его корректность. Решил его тоже реализовать здесь. Проверяется N > 0.

## Про реализации метода Монте Карлло

В программе реализованы три варианта алгоритма для вычисления объёма 3D-фигуры методом Монте-Карло:

1. Однопоточный вариант (функция monte_carlo_single)
2. Многопоточный вариант с автоматическим распределением работы (monte_carlo_auto_parallel)
3. Многопоточный вариант с ручным распределением работы (monte_carlo_manual_parallel)

**Реализация распараллеливания**

**1. Автоматическое распределение (monte_carlo_auto_parallel)**

* Используется конструкция OpenMP:

```cpp
#pragma omp parallel
{
    #pragma omp for schedule(runtime)
}
```

* #pragma omp parallel создаёт пул потоков, заданный args.threads
* #pragma omp for schedule(runtime) автоматически распределяет итерации цикла между потоками, используя планирование (static или dynamic) с размером блока chunk_size
* Каждый поток создаёт свой экземпляр генератора случайных чисел (RandomGenerator rng(tid)), чтобы избежать гонки при генерации случайных чисел
* Локальная переменная hits_thread суммирует попадания внутри потока
* Итоговое значение hits аккумулируется с помощью #pragma omp atomic (то есть гонки данных не происходит)

**2. Ручное распределение (monte_carlo_manual_parallel)**

* Используется только #pragma omp parallel для создания потоков, без #pragma omp for.
* Итерации разбиваются на чанки (блоки) размером chunk_size.
* Динамическое распределение (dynamic) реализуется через атомарный счётчик std::atomic<size_t> next_chunk. Потоки берут следующий доступный чанк до тех пор, пока не будут обработаны все точки.
* Статическое распределение (static или auto) реализуется простым циклом, где каждый поток обрабатывает чанки с шагом равным числу потоков.
* Локальные счётчики hits_thread аккумулируются в общий hits через #pragma omp atomic.

## main.cpp


Через класс Arguments обрабатываются аргументы --input, --output, --realization, --threads, --kind и --chunk_size.

Метод read_N() класса Arguments открывает входной файл и считывает число точек N.

С помощью функции calculate_volume(N, args) выбирается нужная реализация и считается объем.

Перед вызовом функции запускается измерение времени через omp_get_wtime(), а после окончания вычислений — фиксируется конечное время. Используется volatile, чтобы избежать оптимизаций компилятора, как нам показывали на паре.

Разница времени преобразуется в миллисекунды для удобного отображения.

Функция write_result() открывает файл output_file и записывает рассчитанный объём в формате %g\n.
Проверяется успешность открытия файла, при ошибке генерируется исключение.

Определяется число потоков:
* 0 для однопоточной реализации
* заданное `args.threads` или значение по умолчанию OpenMP для многопоточной реализации
Вывод в формате:

    ```
    Time (<num_threads> thread(s)): <total_time> ms
    ```

Любые ошибки при парсинге аргументов, чтении файла или вычислениях перехватываются, и выводится сообщение с префиксом [ERROR].
Программа завершает выполнение с кодом 1 при ошибках, иначе возвращает 0.

## Замеры времени

Замеры проводились с число точек 100000000.

**Графики**

[Тут](assets/time_vs_chunk_size.png), график time_vs_chunk_size.png!

[Тут](assets/threads_vs_time_static_dynamic.png), график threads_vs_time_static_dynamic.png!

**theads_vs_time.py**

=== Starting Monte Carlo timing experiments ===

--- Experiment: R2 static ---
Threads: 1
  Run 1 of 3... Done, time = 1695.01 ms
  Run 2 of 3... Done, time = 1701.36 ms
  Run 3 of 3... Done, time = 1699.83 ms
  Average time: 1698.73 ms

Threads: 2
  Run 1 of 3... Done, time = 1057.70 ms
  Run 2 of 3... Done, time = 1070.98 ms
  Run 3 of 3... Done, time = 1050.02 ms
  Average time: 1059.57 ms

Threads: 4
  Run 1 of 3... Done, time = 717.46 ms
  Run 2 of 3... Done, time = 706.25 ms
  Run 3 of 3... Done, time = 718.24 ms
  Average time: 713.98 ms

Threads: 8
  Run 1 of 3... Done, time = 818.67 ms
  Run 2 of 3... Done, time = 805.83 ms
  Run 3 of 3... Done, time = 822.87 ms
  Average time: 815.79 ms

Threads: 16
  Run 1 of 3... Done, time = 806.62 ms
  Run 2 of 3... Done, time = 843.77 ms
  Run 3 of 3... Done, time = 811.76 ms
  Average time: 820.72 ms

--- Experiment: R2 dynamic ---
Threads: 1
  Run 1 of 3... Done, time = 1715.74 ms
  Run 2 of 3... Done, time = 1723.84 ms
  Run 3 of 3... Done, time = 1714.83 ms
  Average time: 1718.14 ms

Threads: 2
  Run 1 of 3... Done, time = 1071.53 ms
  Run 2 of 3... Done, time = 1096.12 ms
  Run 3 of 3... Done, time = 1058.74 ms
  Average time: 1075.46 ms

Threads: 4
  Run 1 of 3... Done, time = 734.91 ms
  Run 2 of 3... Done, time = 712.18 ms
  Run 3 of 3... Done, time = 727.75 ms
  Average time: 724.95 ms

Threads: 8
  Run 1 of 3... Done, time = 807.68 ms
  Run 2 of 3... Done, time = 812.16 ms
  Run 3 of 3... Done, time = 805.54 ms
  Average time: 808.46 ms

Threads: 16
  Run 1 of 3... Done, time = 817.64 ms
  Run 2 of 3... Done, time = 812.60 ms
  Run 3 of 3... Done, time = 812.58 ms
  Average time: 814.27 ms

--- Experiment: R3 static ---
Threads: 1
  Run 1 of 3... Done, time = 1711.47 ms
  Run 2 of 3... Done, time = 1651.03 ms
  Run 3 of 3... Done, time = 1694.99 ms
  Average time: 1685.83 ms

Threads: 2
  Run 1 of 3... Done, time = 1094.99 ms
  Run 2 of 3... Done, time = 1099.97 ms
  Run 3 of 3... Done, time = 1037.80 ms
  Average time: 1077.59 ms

Threads: 4
  Run 1 of 3... Done, time = 719.12 ms
  Run 2 of 3... Done, time = 695.01 ms
  Run 3 of 3... Done, time = 706.39 ms
  Average time: 706.84 ms

Threads: 8
  Run 1 of 3... Done, time = 818.68 ms
  Run 2 of 3... Done, time = 828.91 ms
  Run 3 of 3... Done, time = 817.84 ms
  Average time: 821.81 ms

Threads: 16
  Run 1 of 3... Done, time = 824.84 ms
  Run 2 of 3... Done, time = 827.66 ms
  Run 3 of 3... Done, time = 822.66 ms
  Average time: 825.05 ms

--- Experiment: R3 dynamic ---
Threads: 1
  Run 1 of 3... Done, time = 1910.32 ms
  Run 2 of 3... Done, time = 2017.06 ms
  Run 3 of 3... Done, time = 2011.08 ms
  Average time: 1979.49 ms

Threads: 2
  Run 1 of 3... Done, time = 1065.82 ms
  Run 2 of 3... Done, time = 1067.79 ms
  Run 3 of 3... Done, time = 1047.74 ms
  Average time: 1060.45 ms

Threads: 4
  Run 1 of 3... Done, time = 708.53 ms
  Run 2 of 3... Done, time = 703.82 ms
  Run 3 of 3... Done, time = 714.90 ms
  Average time: 709.08 ms

Threads: 8
  Run 1 of 3... Done, time = 815.03 ms
  Run 2 of 3... Done, time = 808.47 ms
  Run 3 of 3... Done, time = 815.24 ms
  Average time: 812.91 ms

Threads: 16
  Run 1 of 3... Done, time = 808.25 ms
  Run 2 of 3... Done, time = 811.71 ms
  Run 3 of 3... Done, time = 815.53 ms
  Average time: 811.83 ms


=== Experiment finished. Graph saved to assets/threads_vs_time_static_dynamic.png ===

**chunk_size_vs_time.py**

=== Experiments: Execution time vs chunk_size ===

--- Experiment: R2 static ---
Chunk size: 1
  Run 1 of 3... Done, time = 980.61 ms
  Run 2 of 3... Done, time = 989.78 ms
  Run 3 of 3... Done, time = 985.32 ms
  Average time: 985.24 ms

Chunk size: 2
  Run 1 of 3... Done, time = 930.71 ms
  Run 2 of 3... Done, time = 926.92 ms
  Run 3 of 3... Done, time = 957.37 ms
  Average time: 938.33 ms

Chunk size: 4
  Run 1 of 3... Done, time = 848.71 ms
  Run 2 of 3... Done, time = 845.40 ms
  Run 3 of 3... Done, time = 864.97 ms
  Average time: 853.03 ms

Chunk size: 8
  Run 1 of 3... Done, time = 820.66 ms
  Run 2 of 3... Done, time = 846.06 ms
  Run 3 of 3... Done, time = 831.88 ms
  Average time: 832.86 ms

Chunk size: 16
  Run 1 of 3... Done, time = 821.61 ms
  Run 2 of 3... Done, time = 832.89 ms
  Run 3 of 3... Done, time = 836.04 ms
  Average time: 830.18 ms

Chunk size: 32
  Run 1 of 3... Done, time = 815.86 ms
  Run 2 of 3... Done, time = 810.26 ms
  Run 3 of 3... Done, time = 825.64 ms
  Average time: 817.25 ms

Chunk size: 64
  Run 1 of 3... Done, time = 807.53 ms
  Run 2 of 3... Done, time = 815.79 ms
  Run 3 of 3... Done, time = 813.39 ms
  Average time: 812.24 ms

Chunk size: 128
  Run 1 of 3... Done, time = 807.95 ms
  Run 2 of 3... Done, time = 813.57 ms
  Run 3 of 3... Done, time = 810.47 ms
  Average time: 810.66 ms

Chunk size: 256
  Run 1 of 3... Done, time = 809.30 ms
  Run 2 of 3... Done, time = 833.63 ms
  Run 3 of 3... Done, time = 826.05 ms
  Average time: 822.99 ms

Chunk size: 512
  Run 1 of 3... Done, time = 822.16 ms
  Run 2 of 3... Done, time = 821.25 ms
  Run 3 of 3... Done, time = 810.47 ms
  Average time: 817.96 ms

--- Experiment: R2 dynamic ---
Chunk size: 1
  Run 1 of 3... Done, time = 2040.71 ms
  Run 2 of 3... Done, time = 2020.53 ms
  Run 3 of 3... Done, time = 2038.45 ms
  Average time: 2033.23 ms

Chunk size: 2
  Run 1 of 3... Done, time = 1186.35 ms
  Run 2 of 3... Done, time = 1156.69 ms
  Run 3 of 3... Done, time = 1149.58 ms
  Average time: 1164.21 ms

Chunk size: 4
  Run 1 of 3... Done, time = 942.02 ms
  Run 2 of 3... Done, time = 993.92 ms
  Run 3 of 3... Done, time = 950.71 ms
  Average time: 962.21 ms

Chunk size: 8
  Run 1 of 3... Done, time = 861.98 ms
  Run 2 of 3... Done, time = 862.33 ms
  Run 3 of 3... Done, time = 873.19 ms
  Average time: 865.83 ms

Chunk size: 16
  Run 1 of 3... Done, time = 848.43 ms
  Run 2 of 3... Done, time = 846.27 ms
  Run 3 of 3... Done, time = 846.15 ms
  Average time: 846.95 ms

Chunk size: 32
  Run 1 of 3... Done, time = 827.45 ms
  Run 2 of 3... Done, time = 878.31 ms
  Run 3 of 3... Done, time = 827.02 ms
  Average time: 844.26 ms

Chunk size: 64
  Run 1 of 3... Done, time = 820.36 ms
  Run 2 of 3... Done, time = 817.02 ms
  Run 3 of 3... Done, time = 817.26 ms
  Average time: 818.22 ms

Chunk size: 128
  Run 1 of 3... Done, time = 814.79 ms
  Run 2 of 3... Done, time = 813.98 ms
  Run 3 of 3... Done, time = 817.27 ms
  Average time: 815.35 ms

Chunk size: 256
  Run 1 of 3... Done, time = 825.71 ms
  Run 2 of 3... Done, time = 817.69 ms
  Run 3 of 3... Done, time = 821.78 ms
  Average time: 821.73 ms

Chunk size: 512
  Run 1 of 3... Done, time = 823.07 ms
  Run 2 of 3... Done, time = 811.18 ms
  Run 3 of 3... Done, time = 815.11 ms
  Average time: 816.45 ms

--- Experiment: R3 static ---
Chunk size: 1
  Run 1 of 3... Done, time = 1239.62 ms
  Run 2 of 3... Done, time = 1238.81 ms
  Run 3 of 3... Done, time = 1218.20 ms
  Average time: 1232.21 ms

Chunk size: 2
  Run 1 of 3... Done, time = 1087.85 ms
  Run 2 of 3... Done, time = 1036.54 ms
  Run 3 of 3... Done, time = 1069.66 ms
  Average time: 1064.68 ms

Chunk size: 4
  Run 1 of 3... Done, time = 1032.10 ms
  Run 2 of 3... Done, time = 937.50 ms
  Run 3 of 3... Done, time = 917.72 ms
  Average time: 962.44 ms

Chunk size: 8
  Run 1 of 3... Done, time = 888.38 ms
  Run 2 of 3... Done, time = 910.78 ms
  Run 3 of 3... Done, time = 889.75 ms
  Average time: 896.30 ms

Chunk size: 16
  Run 1 of 3... Done, time = 866.02 ms
  Run 2 of 3... Done, time = 860.19 ms
  Run 3 of 3... Done, time = 868.90 ms
  Average time: 865.04 ms

Chunk size: 32
  Run 1 of 3... Done, time = 833.85 ms
  Run 2 of 3... Done, time = 835.38 ms
  Run 3 of 3... Done, time = 850.75 ms
  Average time: 839.99 ms

Chunk size: 64
  Run 1 of 3... Done, time = 844.26 ms
  Run 2 of 3... Done, time = 833.16 ms
  Run 3 of 3... Done, time = 829.51 ms
  Average time: 835.64 ms

Chunk size: 128
  Run 1 of 3... Done, time = 833.15 ms
  Run 2 of 3... Done, time = 838.93 ms
  Run 3 of 3... Done, time = 824.98 ms
  Average time: 832.35 ms

Chunk size: 256
  Run 1 of 3... Done, time = 830.83 ms
  Run 2 of 3... Done, time = 835.13 ms
  Run 3 of 3... Done, time = 826.26 ms
  Average time: 830.74 ms

Chunk size: 512
  Run 1 of 3... Done, time = 830.15 ms
  Run 2 of 3... Done, time = 816.05 ms
  Run 3 of 3... Done, time = 833.70 ms
  Average time: 826.63 ms

--- Experiment: R3 dynamic ---
Chunk size: 1
  Run 1 of 3... Done, time = 4110.64 ms
  Run 2 of 3... Done, time = 4174.16 ms
  Run 3 of 3... Done, time = 4135.83 ms
  Average time: 4140.21 ms

Chunk size: 2
  Run 1 of 3... Done, time = 2246.02 ms
  Run 2 of 3... Done, time = 2329.18 ms
  Run 3 of 3... Done, time = 2291.77 ms
  Average time: 2288.99 ms

Chunk size: 4
  Run 1 of 3... Done, time = 1284.28 ms
  Run 2 of 3... Done, time = 1405.94 ms
  Run 3 of 3... Done, time = 1276.70 ms
  Average time: 1322.31 ms

Chunk size: 8
  Run 1 of 3... Done, time = 1046.41 ms
  Run 2 of 3... Done, time = 1036.20 ms
  Run 3 of 3... Done, time = 1034.98 ms
  Average time: 1039.20 ms

Chunk size: 16
  Run 1 of 3... Done, time = 990.15 ms
  Run 2 of 3... Done, time = 888.36 ms
  Run 3 of 3... Done, time = 889.78 ms
  Average time: 922.76 ms

Chunk size: 32
  Run 1 of 3... Done, time = 850.13 ms
  Run 2 of 3... Done, time = 867.74 ms
  Run 3 of 3... Done, time = 857.96 ms
  Average time: 858.61 ms

Chunk size: 64
  Run 1 of 3... Done, time = 832.82 ms
  Run 2 of 3... Done, time = 838.74 ms
  Run 3 of 3... Done, time = 837.69 ms
  Average time: 836.42 ms

Chunk size: 128
  Run 1 of 3... Done, time = 884.97 ms
  Run 2 of 3... Done, time = 882.07 ms
  Run 3 of 3... Done, time = 825.31 ms
  Average time: 864.12 ms

Chunk size: 256
  Run 1 of 3... Done, time = 816.53 ms
  Run 2 of 3... Done, time = 815.07 ms
  Run 3 of 3... Done, time = 821.12 ms
  Average time: 817.57 ms

Chunk size: 512
  Run 1 of 3... Done, time = 811.60 ms
  Run 2 of 3... Done, time = 810.27 ms
  Run 3 of 3... Done, time = 816.61 ms
  Average time: 812.83 ms


=== Experiment finished. Graph saved to assets/time_vs_chunk_size.png ===

**Процессор**

CPU: 11th Gen Intel® Core™ i5-1135G7 @ 2.40GHz

Количество ядер: 4

Количество потоков: 8 (2 потока на ядро)

# СПАСИБО!