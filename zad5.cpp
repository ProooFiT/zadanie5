/**
 * @file pi_approximation.cpp
 * @brief Program do przybliżania wartości liczby PI za pomocą całkowania numerycznego i wielowątkowości.
 *
 * Program wykorzystuje metodę punktu środkowego do obliczenia całki funkcji
 * f(x) = 4 / (1 + x^2) na przedziale [0, 1]. Obliczenia są równolegle przetwarzane
 * przy użyciu standardowej biblioteki wątków w C++ (POSIX).
 */

#include <iostream>
#include <thread>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>

/**
 * @brief Mutex zapewniający bezpieczny dostęp do wspólnego wyniku.
 *
 * Używany do synchronizacji sekcji krytycznych w programie. Zapobiega jednoczesnemu
 * modyfikowaniu zmiennej globalnej `total_result` przez wiele wątków.
 */
std::mutex result_mutex;

/**
 * @brief Oblicza częściową wartość całki dla danego zakresu i liczby kroków.
 * 
 * Funkcja wykorzystuje metodę punktu środkowego do obliczenia wartości całki
 * funkcji f(x) = 4 / (1 + x^2) na określonym podzakresie.
 *
 * @param start Początek zakresu obliczeń.
 * @param end Koniec zakresu obliczeń.
 * @param steps Liczba podziałów w zakresie.
 * @return Wynik obliczonej całki dla danego zakresu.
 */
double calculate_partial_integral(double start, double end, unsigned long long steps) {
    double sum = 0.0;
    double step_size = (end - start) / steps; ///< Długość pojedynczego kroku
    for (unsigned long long i = 0; i < steps; ++i) {
        double x = start + i * step_size + step_size / 2.0; ///< Punkt środkowy
        sum += 4.0 / (1.0 + x * x); ///< Obliczenie wartości funkcji w punkcie
    }
    return sum * step_size; ///< Zwrot wyniku częściowego całkowania
}

/**
 * @brief Główna funkcja programu.
 *
 * Funkcja główna przyjmuje od użytkownika liczbę podziałów całki oraz liczbę wątków.
 * Następnie dzieli zakres obliczeń między wątki, uruchamia je równolegle
 * i synchronizuje wyniki. Na końcu wypisuje przybliżoną wartość liczby PI
 * oraz czas wykonania obliczeń.
 *
 * @return Kod zakończenia programu (0, jeśli program wykona się poprawnie).
 */
int main() {
    unsigned long long num_steps; ///< Liczba podziałów przedziału całki
    int num_threads; ///< Liczba wątków

    // Pobranie danych od użytkownika
    std::cout << "Podaj liczbe podzialow (np. 1000000000): ";
    std::cin >> num_steps;
    std::cout << "Podaj liczbe watkow: ";
    std::cin >> num_threads;

    // Walidacja danych wejściowych
    if (num_threads <= 0 || num_steps <= 0) {
        std::cerr << "Liczba wątków i podziałów musi być dodatnia!" << std::endl;
        return 1;
    }

    double total_result = 0.0; ///< Zmienna przechowująca wynik końcowy
    std::vector<std::thread> threads; ///< Wektor do przechowywania wątków
    unsigned long long steps_per_thread = num_steps / num_threads; ///< Liczba kroków przypadających na wątek
    double range_per_thread = 1.0 / num_threads; ///< Długość zakresu przypadającego na wątek

    // Rozpoczęcie pomiaru czasu
    auto start_time = std::chrono::high_resolution_clock::now();

    /**
     * @brief Funkcja wykonywana przez każdy wątek.
     *
     * Oblicza częściowy wynik całki dla przypisanego zakresu, a następnie
     * dodaje go do zmiennej globalnej `total_result` w sposób bezpieczny
     * przy użyciu mutexa.
     *
     * @param start Początek zakresu dla danego wątku.
     * @param end Koniec zakresu dla danego wątku.
     * @param steps Liczba kroków dla danego wątku.
     */
    auto thread_worker = [&](double start, double end, unsigned long long steps) {
        double partial_result = calculate_partial_integral(start, end, steps);
        std::lock_guard<std::mutex> lock(result_mutex); ///< Blokada mutexa
        total_result += partial_result; ///< Dodanie wyniku częściowego do wyniku końcowego
    };

    // Uruchomienie wątków
    for (int i = 0; i < num_threads; ++i) {
        double start = i * range_per_thread;
        double end = (i + 1) * range_per_thread;
        threads.emplace_back(thread_worker, start, end, steps_per_thread);
    }

    // Oczekiwanie na zakończenie wątków
    for (auto& t : threads) {
        t.join();
    }

    // Zatrzymanie pomiaru czasu
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    // Wyświetlenie wyników
    std::cout << "Przyblizona wartosc liczby PI: " << total_result << std::endl;
    std::cout << "Czas obliczen: " << elapsed_time.count() << " sekund" << std::endl;

    return 0;
}
