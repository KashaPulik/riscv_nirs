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

// Глобальные разделяемые переменные для тестов
volatile uint32_t g_var_exch = 0;
volatile uint32_t g_var_add = 0;
volatile uint32_t g_var_and = 0xffffffff;
volatile uint32_t g_var_or = 0;
volatile uint32_t g_var_xor = 0;
volatile uint32_t g_var_cas = 0;

// Функции, выполняемые потоками для атомарного сложения

void thread_func_exch()
{
    for (int i = 1; i <= ITERATIONS; i++) {
        atomic_exchange(&g_var_exch, i);
    }
}

void thread_func_add()
{
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_fetch_add(&g_var_add, 1);
    }
}

void thread_func_and()
{
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_fetch_and(&g_var_and, i);
    }
}

void thread_func_or()
{
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_fetch_or(&g_var_or, i);
    }
}

void thread_func_xor()
{
    for (int i = 0; i < ITERATIONS; i++) {
    atomic_fetch_xor(&g_var_xor, i);
    }
}

void thread_func_cas()
{
    for (int i = 0; i < ITERATIONS; i++) {
        uint32_t expected;
        do {
            expected = g_var_cas;
#ifdef __riscv
        } while (atomic_compare_exchange_strong(&g_var_cas, &expected, expected + 1) != expected);
#elif defined(__x86_64)
        } while (atomic_compare_exchange_strong(&g_var_cas, &expected, expected + 1) != true);
#endif
    }
}

void test_exch()
{
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_exch);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарный обмен за " << duration.count() << " секунд\n";
}

void test_add()
{
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
}

void test_and()
{
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_and);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное и за " << duration.count() << " секунд\n";
}

void test_or()
{
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_or);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное или за " << duration.count() << " секунд\n";
}

void test_xor()
{
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_xor);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное искл или за " << duration.count() << " секунд\n";
}

void test_cas()
{
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(thread_func_cas);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = (end - start) / NUM_THREADS / ITERATIONS;

    std::cout << "Атомарное CAS за " << duration.count() << " секунд\n";
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

    test_exch();
    test_add();
    test_and();
    test_or();
    test_xor();
    test_cas();

    // Вывод итоговых значений
    std::cout << "Итоговые значения: ";
    std::cout << "g_var_exch = " << g_var_exch;
    std::cout << ", g_var_add = " << g_var_add;
    std::cout << ", g_var_and = " << g_var_and;
    std::cout << ", g_var_or = " << g_var_or;
    std::cout << ", g_var_xor = " << g_var_xor;
    std::cout << ", g_var_cas = " << g_var_cas;
    std::cout << '\n';
    return 0;
}
