#include "stdatomic_asm.h"
#include <chrono>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <thread>
#include <vector>

// Количество потоков и число итераций для каждого потока
#define NUM_THREADS 2
#define ITERATIONS 1'000'000

#ifdef __riscv
// ======================================================
// Реализация для архитектуры RISC-V
// ======================================================

// Функция для получения счетчика циклов (32-битный)
static inline uint32_t get_cycle_riscv()
{
    uint32_t cycles;
    __asm__ volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}

// Атомарное сложение: используется инструкция amoadd.w (расширение A).
// Функция возвращает старое значение по адресу.
static inline uint32_t atomic_add_riscv(volatile uint32_t* addr, uint32_t val)
{
    uint32_t result;
    __asm__ volatile("amoadd.w %0, %2, (%1)" : "=r"(result) : "r"(addr), "r"(val) : "memory");
    return result;
}

// Атомарное CAS: пример реализации на базе lr.w/sc.w (имитация расширения Zacas).
// Если значение по адресу равно expected, оно заменяется на desired.
static inline uint32_t atomic_cas_riscv(volatile uint32_t* addr, uint32_t expected, uint32_t desired)
{
    uint32_t old, tmp;
    __asm__ volatile(
            "0: lr.w %0, (%2)\n"     // Загружаем значение (load-reserved)
            "   bne %0, %3, 1f\n"    // Если оно не равно expected, выходим
            "   sc.w %1, %4, (%2)\n" // Пытаемся записать desired (store-conditional)
            "   bnez %1, 0b\n"       // Если не удалось, повторяем
            "1:"
            : "=&r"(old), "=&r"(tmp)
            : "r"(addr), "r"(expected), "r"(desired)
            : "memory");
    return old;
}

#elif defined(__x86_64)
// ======================================================
// Реализация для архитектуры x86-64
// ======================================================

// Функция для получения счетчика тактов (64-битный)
static inline uint64_t get_tsc_x86()
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

// Атомарное сложение: используется инструкция lock xaddl.
// Функция возвращает старое значение по адресу.
static inline uint32_t atomic_add_x86(volatile uint32_t* addr, uint32_t val)
{
    uint32_t old = val;
    __asm__ volatile("lock xaddl %0, %1" : "+r"(old), "+m"(*addr) : : "memory");
    return old;
}

// Атомарное CAS: используется инструкция lock cmpxchgl.
// Если значение по адресу равно expected, оно заменяется на desired.
static inline uint32_t atomic_cas_x86(volatile uint32_t* addr, uint32_t expected, uint32_t desired)
{
    uint32_t old;
    __asm__ volatile("lock cmpxchgl %2, %1" : "=a"(old), "+m"(*addr) : "r"(desired), "0"(expected) : "memory");
    return old;
}

#else
#error "Unsupported architecture"
#endif

// Глобальные разделяемые переменные для тестов
volatile uint32_t g_var_add = 0;
volatile uint32_t g_var_cas = 0;

// Функции, выполняемые потоками для атомарного сложения
void thread_func_add()
{
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_fetch_add(&g_var_add, 1);
    }
}

// Функции, выполняемые потоками для атомарного CAS.
// Здесь каждый поток пытается увеличить значение переменной,
// сначала считывая текущее значение, затем заменяя его на value+1,
// если оно не изменилось.
// void thread_func_cas()
// {
//     for (int i = 0; i < ITERATIONS; i++) {
//         uint32_t expected;
//         do {
//             expected = g_var_cas;
// #ifdef __riscv
//         } while (atomic_compare_exchange_strong(&g_var_cas, &expected, expected + 1) != expected);
// #elif defined(__x86_64)
//         } while (atomic_compare_exchange_strong(&g_var_cas, &expected, expected + 1) != true);
// #endif
//     }
// }

void thread_func_cas()
{
    for (int i = 0; i < ITERATIONS; i++) {
        uint32_t expected;
        do {
            expected = g_var_cas;
        } while (atomic_compare_exchange_strong(&g_var_cas, &expected, expected + 1) != true);
    }
}

int main()
{
#ifdef __riscv
    printf("Тестирование атомарных операций для RISC-V с использованием %d потоков и %d итераций\n",
           NUM_THREADS,
           ITERATIONS);
#elif defined(__x86_64)
    printf("Тестирование атомарных операций для x86-64 с использованием %d потоков и %d итераций\n",
           NUM_THREADS,
           ITERATIONS);
#endif

    // Тест атомарного сложения
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_add);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное сложение за " << duration.count() << " секунд\n";

    start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_cas);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    end = std::chrono::high_resolution_clock::now();

    duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное CAS за " << duration.count() << " секунд\n";

    // Вывод итоговых значений
    printf("Итоговые значения: g_var_add = %u, g_var_cas = %u\n", g_var_add, g_var_cas);
    return 0;
}
