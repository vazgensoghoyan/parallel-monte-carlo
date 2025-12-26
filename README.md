| Лабораторная работа №4    | 24Б10                 | Архитектура компьютера |
| ------------------------- | --------------------- | ---------------------- |
| Оптимизация по скорости   | Согоян Вазген Айкович | 2025                   |

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

**schedule_vs_time.py**

=== Experiments: fixed threads, varying schedule ===

--- Experiment: R2 ---
Schedule: static
  Run 1 of 3... Done, time = 1427.73 ms
  Run 2 of 3... Done, time = 1603.04 ms
  Run 3 of 3... Done, time = 1779.47 ms
  Average time: 1603.41 ms

Schedule: dynamic
  Run 1 of 3... Done, time = 2104.89 ms
  Run 2 of 3... Done, time = 1915.30 ms
  Run 3 of 3... Done, time = 2046.03 ms
  Average time: 2022.07 ms

--- Experiment: R3 ---
Schedule: static
  Run 1 of 3... Done, time = 2231.08 ms
  Run 2 of 3... Done, time = 1778.44 ms
  Run 3 of 3... Done, time = 2189.23 ms
  Average time: 2066.25 ms

Schedule: dynamic
  Run 1 of 3... Done, time = 2027.90 ms
  Run 2 of 3... Done, time = 1844.11 ms
  Run 3 of 3... Done, time = 1717.50 ms
  Average time: 1863.17 ms


=== Experiment finished. Graph saved to assets/time_vs_schedule.png ===

**theads_vs_time.py**

=== Starting Monte Carlo timing experiments ===

--- Experiment: R2 static ---
Threads: 1
  Run 1 of 3... Done, time = 13939.50 ms
  Run 2 of 3... Done, time = 9040.34 ms
  Run 3 of 3... Done, time = 6836.63 ms
  Average time: 9938.82 ms

Threads: 2
  Run 1 of 3... Done, time = 5404.53 ms
  Run 2 of 3... Done, time = 5733.42 ms
  Run 3 of 3... Done, time = 4249.64 ms
  Average time: 5129.20 ms

Threads: 4
  Run 1 of 3... Done, time = 2806.26 ms
  Run 2 of 3... Done, time = 2909.16 ms
  Run 3 of 3... Done, time = 2987.42 ms
  Average time: 2900.95 ms

Threads: 8
  Run 1 of 3... Done, time = 2502.91 ms
  Run 2 of 3... Done, time = 2914.75 ms
  Run 3 of 3... Done, time = 2426.49 ms
  Average time: 2614.72 ms

Threads: 16
  Run 1 of 3... Done, time = 2093.80 ms
  Run 2 of 3... Done, time = 2104.17 ms
  Run 3 of 3... Done, time = 1914.29 ms
  Average time: 2037.42 ms

--- Experiment: R2 dynamic ---
Threads: 1
  Run 1 of 3... Done, time = 8207.70 ms
  Run 2 of 3... Done, time = 8630.53 ms
  Run 3 of 3... Done, time = 8416.63 ms
  Average time: 8418.29 ms

Threads: 2
  Run 1 of 3... Done, time = 4165.95 ms
  Run 2 of 3... Done, time = 3839.97 ms
  Run 3 of 3... Done, time = 4718.51 ms
  Average time: 4241.48 ms

Threads: 4
  Run 1 of 3... Done, time = 2577.62 ms
  Run 2 of 3... Done, time = 2206.58 ms
  Run 3 of 3... Done, time = 2032.19 ms
  Average time: 2272.13 ms

Threads: 8
  Run 1 of 3... Done, time = 2049.49 ms
  Run 2 of 3... Done, time = 2139.07 ms
  Run 3 of 3... Done, time = 1763.79 ms
  Average time: 1984.12 ms

Threads: 16
  Run 1 of 3... Done, time = 1161.38 ms
  Run 2 of 3... Done, time = 1257.11 ms
  Run 3 of 3... Done, time = 1526.53 ms
  Average time: 1315.01 ms

--- Experiment: R3 static ---
Threads: 1
  Run 1 of 3... Done, time = 6918.58 ms
  Run 2 of 3... Done, time = 7403.98 ms
  Run 3 of 3... Done, time = 6953.90 ms
  Average time: 7092.15 ms

Threads: 2
  Run 1 of 3... Done, time = 3392.31 ms
  Run 2 of 3... Done, time = 3801.25 ms
  Run 3 of 3... Done, time = 3745.07 ms
  Average time: 3646.21 ms

Threads: 4
  Run 1 of 3... Done, time = 2392.33 ms
  Run 2 of 3... Done, time = 3113.31 ms
  Run 3 of 3... Done, time = 2719.07 ms
  Average time: 2741.57 ms

Threads: 8
  Run 1 of 3... Done, time = 1921.24 ms
  Run 2 of 3... Done, time = 2143.09 ms
  Run 3 of 3... Done, time = 2383.00 ms
  Average time: 2149.11 ms

Threads: 16
  Run 1 of 3... Done, time = 1685.64 ms
  Run 2 of 3... Done, time = 1944.62 ms
  Run 3 of 3... Done, time = 1551.06 ms
  Average time: 1727.11 ms

--- Experiment: R3 dynamic ---
Threads: 1
  Run 1 of 3... Done, time = 8095.72 ms
  Run 2 of 3... Done, time = 8326.88 ms
  Run 3 of 3... Done, time = 7841.73 ms
  Average time: 8088.11 ms

Threads: 2
  Run 1 of 3... Done, time = 5421.71 ms
  Run 2 of 3... Done, time = 2926.28 ms
  Run 3 of 3... Done, time = 1409.38 ms
  Average time: 3252.46 ms

Threads: 4
  Run 1 of 3... Done, time = 953.89 ms
  Run 2 of 3... Done, time = 835.83 ms
  Run 3 of 3... Done, time = 851.29 ms
  Average time: 880.34 ms

Threads: 8
  Run 1 of 3... Done, time = 972.18 ms
  Run 2 of 3... Done, time = 985.49 ms
  Run 3 of 3... Done, time = 963.84 ms
  Average time: 973.84 ms

Threads: 16
  Run 1 of 3... Done, time = 963.83 ms
  Run 2 of 3... Done, time = 950.38 ms
  Run 3 of 3... Done, time = 980.67 ms
  Average time: 964.96 ms


=== Experiment finished. Graph saved to assets/threads_vs_time_static_dynamic.png ===


**chunk_size_vs_time.py**

=== Experiments: Execution time vs chunk_size ===

--- Experiment: R2 static ---
Chunk size: 1
  Run 1 of 3... Done, time = 662.41 ms
  Run 2 of 3... Done, time = 703.22 ms
  Run 3 of 3... Done, time = 1072.35 ms
  Average time: 812.66 ms

Chunk size: 2
  Run 1 of 3... Done, time = 1354.66 ms
  Run 2 of 3... Done, time = 2532.09 ms
  Run 3 of 3... Done, time = 2703.32 ms
  Average time: 2196.69 ms

Chunk size: 4
  Run 1 of 3... Done, time = 2438.60 ms
  Run 2 of 3... Done, time = 1967.15 ms
  Run 3 of 3... Done, time = 2106.71 ms
  Average time: 2170.82 ms

Chunk size: 8
  Run 1 of 3... Done, time = 2360.76 ms
  Run 2 of 3... Done, time = 2610.06 ms
  Run 3 of 3... Done, time = 2995.15 ms
  Average time: 2655.32 ms

Chunk size: 16
  Run 1 of 3... Done, time = 1866.85 ms
  Run 2 of 3... Done, time = 1373.83 ms
  Run 3 of 3... Done, time = 1257.38 ms
  Average time: 1499.35 ms

Chunk size: 32
  Run 1 of 3... Done, time = 1110.42 ms
  Run 2 of 3... Done, time = 1180.02 ms
  Run 3 of 3... Done, time = 1156.19 ms
  Average time: 1148.88 ms

Chunk size: 64
  Run 1 of 3... Done, time = 1118.59 ms
  Run 2 of 3... Done, time = 1316.07 ms
  Run 3 of 3... Done, time = 1981.00 ms
  Average time: 1471.89 ms

Chunk size: 128
  Run 1 of 3... Done, time = 1966.39 ms
  Run 2 of 3... Done, time = 2034.25 ms
  Run 3 of 3... Done, time = 1862.82 ms
  Average time: 1954.49 ms

Chunk size: 256
  Run 1 of 3... Done, time = 1660.84 ms
  Run 2 of 3... Done, time = 1318.61 ms
  Run 3 of 3... Done, time = 1288.74 ms
  Average time: 1422.73 ms

Chunk size: 512
  Run 1 of 3... Done, time = 1483.71 ms
  Run 2 of 3... Done, time = 1768.39 ms
  Run 3 of 3... Done, time = 2012.80 ms
  Average time: 1754.97 ms

--- Experiment: R2 dynamic ---
Chunk size: 1
  Run 1 of 3... Done, time = 3726.16 ms
  Run 2 of 3... Done, time = 3706.97 ms
  Run 3 of 3... Done, time = 3885.61 ms
  Average time: 3772.91 ms

Chunk size: 2
  Run 1 of 3... Done, time = 2605.78 ms
  Run 2 of 3... Done, time = 2954.75 ms
  Run 3 of 3... Done, time = 2980.30 ms
  Average time: 2846.94 ms

Chunk size: 4
  Run 1 of 3... Done, time = 1409.04 ms
  Run 2 of 3... Done, time = 1466.78 ms
  Run 3 of 3... Done, time = 1461.51 ms
  Average time: 1445.78 ms

Chunk size: 8
  Run 1 of 3... Done, time = 1323.25 ms
  Run 2 of 3... Done, time = 1298.60 ms
  Run 3 of 3... Done, time = 1314.56 ms
  Average time: 1312.14 ms

Chunk size: 16
  Run 1 of 3... Done, time = 1291.07 ms
  Run 2 of 3... Done, time = 1246.91 ms
  Run 3 of 3... Done, time = 1279.44 ms
  Average time: 1272.47 ms

Chunk size: 32
  Run 1 of 3... Done, time = 1265.39 ms
  Run 2 of 3... Done, time = 1228.33 ms
  Run 3 of 3... Done, time = 1197.74 ms
  Average time: 1230.49 ms

Chunk size: 64
  Run 1 of 3... Done, time = 1125.81 ms
  Run 2 of 3... Done, time = 1256.72 ms
  Run 3 of 3... Done, time = 1434.91 ms
  Average time: 1272.48 ms

Chunk size: 128
  Run 1 of 3... Done, time = 1343.89 ms
  Run 2 of 3... Done, time = 1256.30 ms
  Run 3 of 3... Done, time = 1178.54 ms
  Average time: 1259.58 ms

Chunk size: 256
  Run 1 of 3... Done, time = 1183.46 ms
  Run 2 of 3... Done, time = 1314.95 ms
  Run 3 of 3... Done, time = 1435.33 ms
  Average time: 1311.25 ms

Chunk size: 512
  Run 1 of 3... Done, time = 1314.82 ms
  Run 2 of 3... Done, time = 1345.52 ms
  Run 3 of 3... Done, time = 1226.37 ms
  Average time: 1295.57 ms

--- Experiment: R3 static ---
Chunk size: 1
  Run 1 of 3... Done, time = 2066.30 ms
  Run 2 of 3... Done, time = 1867.69 ms
  Run 3 of 3... Done, time = 1932.28 ms
  Average time: 1955.42 ms

Chunk size: 2
  Run 1 of 3... Done, time = 1897.18 ms
  Run 2 of 3... Done, time = 2062.97 ms
  Run 3 of 3... Done, time = 1951.94 ms
  Average time: 1970.70 ms

Chunk size: 4
  Run 1 of 3... Done, time = 1935.87 ms
  Run 2 of 3... Done, time = 2041.50 ms
  Run 3 of 3... Done, time = 2313.96 ms
  Average time: 2097.11 ms

Chunk size: 8
  Run 1 of 3... Done, time = 3163.79 ms
  Run 2 of 3... Done, time = 2708.89 ms
  Run 3 of 3... Done, time = 1196.33 ms
  Average time: 2356.34 ms

Chunk size: 16
  Run 1 of 3... Done, time = 1241.64 ms
  Run 2 of 3... Done, time = 1285.35 ms
  Run 3 of 3... Done, time = 1165.68 ms
  Average time: 1230.89 ms

Chunk size: 32
  Run 1 of 3... Done, time = 1097.17 ms
  Run 2 of 3... Done, time = 1179.40 ms
  Run 3 of 3... Done, time = 1179.81 ms
  Average time: 1152.13 ms

Chunk size: 64
  Run 1 of 3... Done, time = 1287.98 ms
  Run 2 of 3... Done, time = 1133.94 ms
  Run 3 of 3... Done, time = 1082.83 ms
  Average time: 1168.25 ms

Chunk size: 128
  Run 1 of 3... Done, time = 1086.34 ms
  Run 2 of 3... Done, time = 1103.65 ms
  Run 3 of 3... Done, time = 1139.46 ms
  Average time: 1109.82 ms

Chunk size: 256
  Run 1 of 3... Done, time = 1091.08 ms
  Run 2 of 3... Done, time = 1153.97 ms
  Run 3 of 3... Done, time = 1149.62 ms
  Average time: 1131.56 ms

Chunk size: 512
  Run 1 of 3... Done, time = 1177.71 ms
  Run 2 of 3... Done, time = 1100.69 ms
  Run 3 of 3... Done, time = 1202.20 ms
  Average time: 1160.20 ms

--- Experiment: R3 dynamic ---
Chunk size: 1
  Run 1 of 3... Done, time = 5166.13 ms
  Run 2 of 3... Done, time = 5084.74 ms
  Run 3 of 3... Done, time = 6105.05 ms
  Average time: 5451.97 ms

Chunk size: 2
  Run 1 of 3... Done, time = 3568.95 ms
  Run 2 of 3... Done, time = 3921.22 ms
  Run 3 of 3... Done, time = 4638.43 ms
  Average time: 4042.87 ms

Chunk size: 4
  Run 1 of 3... Done, time = 2467.86 ms
  Run 2 of 3... Done, time = 1844.79 ms
  Run 3 of 3... Done, time = 1718.42 ms
  Average time: 2010.36 ms

Chunk size: 8
  Run 1 of 3... Done, time = 1247.09 ms
  Run 2 of 3... Done, time = 1316.22 ms
  Run 3 of 3... Done, time = 1477.65 ms
  Average time: 1346.99 ms

Chunk size: 16
  Run 1 of 3... Done, time = 1341.60 ms
  Run 2 of 3... Done, time = 1319.58 ms
  Run 3 of 3... Done, time = 1207.50 ms
  Average time: 1289.56 ms

Chunk size: 32
  Run 1 of 3... Done, time = 1174.91 ms
  Run 2 of 3... Done, time = 1176.21 ms
  Run 3 of 3... Done, time = 1206.06 ms
  Average time: 1185.73 ms

Chunk size: 64
  Run 1 of 3... Done, time = 1162.67 ms
  Run 2 of 3... Done, time = 1161.20 ms
  Run 3 of 3... Done, time = 1362.33 ms
  Average time: 1228.73 ms

Chunk size: 128
  Run 1 of 3... Done, time = 1177.99 ms
  Run 2 of 3... Done, time = 1246.47 ms
  Run 3 of 3... Done, time = 1288.81 ms
  Average time: 1237.76 ms

Chunk size: 256
  Run 1 of 3... Done, time = 1394.78 ms
  Run 2 of 3... Done, time = 1298.51 ms
  Run 3 of 3... Done, time = 1362.51 ms
  Average time: 1351.93 ms

Chunk size: 512
  Run 1 of 3... Done, time = 1526.31 ms
  Run 2 of 3... Done, time = 1255.79 ms
  Run 3 of 3... Done, time = 1230.63 ms
  Average time: 1337.58 ms


=== Experiment finished. Graph saved to assets/time_vs_chunk_size.png ===

**Процессор**

CPU: 11th Gen Intel® Core™ i5-1135G7 @ 2.40GHz

Количество ядер: 4

Количество потоков: 8 (2 потока на ядро)

# СПАСИБО!