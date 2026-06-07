#pragma once
#include <fstream>
#include <random>
#include <chrono>
#include <iostream>


// Liczba powtorzen calego eksperymentu dla danego rozmiaru. Wyniki sa usredniane.
constexpr int LICZBA_POWTORZEN = 64;

// Pojedyncze wywolanie jest za szybkie aby zmierzyc je dokladnie zegarem,
// dlatego mierzymy laczny czas K wywolan i dzielimy.
constexpr int LICZBA_OPERACJI = 100;

// ilosc elementow uzywanych w pomiarach.
std::vector<int> domyslneRozmiary = {5000, 8000, 10000, 16000, 20000, 40000, 60000, 100000};

// Rozmiary tablic - pierwsza liczba pierwsza > N*2 dla każdego N
std::vector<int> rozmiarytablic = {10007, 16007, 20011, 32003, 40009, 80021, 120011, 200003};



// Generuje wektor z n losowymi parami klucz-wartość
std::vector<std::pair<int, int>> generujLiczby(int n) {
    std::vector<std::pair<int, int>> dane;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distKlucz(1, 1000000000);

    for (int i = 0; i < n; i++) {
        dane.push_back({distKlucz(gen), i});
    }

    return dane;
}

// Szablonowa funkcja do wypełniania tabeli z wektora
template <typename TypTabeli>
void wypelnijTabele(TypTabeli& tablica, const std::vector<std::pair<int, int>>& dane) {
    for (const auto& para : dane) {
        tablica.insert(para.first, para.second);
    }
}

class Zegar {
    std::chrono::steady_clock::time_point moment_start;

public:
    void start() {
        moment_start = std::chrono::steady_clock::now();
    }

    std::chrono::nanoseconds::rep stop() {
        auto moment_stop = std::chrono::steady_clock::now();
        auto czas = std::chrono::duration_cast<std::chrono::nanoseconds>(moment_stop - moment_start);
        return czas.count();
    }
};

// Mierzy czas wykonania przekazanej operacji (w nanosekundach)
template <typename Funkcja>
double zmierzCzas(Funkcja operacja) {
    Zegar zegar;
    zegar.start();
    operacja();
    return static_cast<double>(zegar.stop());
}


template <typename TypTabeli>
void zmierzTabele(const std::string& nazwaPliku) {
    std::ofstream plik(nazwaPliku);
    if (!plik) {
        std::cout << "Nie mozna otworzyc pliku do zapisu: " << nazwaPliku << std::endl;
        return;
    }
    plik << "rozmiar,insert,remove\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distWart(1, 1000000000);


    // Pre-generujemy klucze do wstawienia i usunięcia
    std::vector<int> klucze;
    for (int i = 0; i < LICZBA_POWTORZEN; i++) {
        klucze.push_back(distWart(gen));
    }

    for (int k = 0; k < domyslneRozmiary.size(); k++) {
        
        int n = domyslneRozmiary[k];
        int rozmiarTablicy = rozmiarytablic[k];

        double sumaInsert = 0.0, sumaRemove = 0.0;

        std::cout << "Pomiar insert i remove dla n=" << n << "..." << std::endl;

        for (int r = 0; r < LICZBA_POWTORZEN; r++) {
            // Generujemy dane do wektora
            std::vector<std::pair<int, int>> dane = generujLiczby(n);

            TypTabeli tablica(rozmiarTablicy);
            wypelnijTabele(tablica, dane);

            // Pomiar insert
            if constexpr (std::is_same_v<TypTabeli, LinearProbingHashTable>) {
                for (int i = 0; i < LICZBA_OPERACJI; i++) {
                    TypTabeli tablicaUni(rozmiarTablicy);
                    wypelnijTabele(tablicaUni, dane);
                    int klucz = klucze[r];
                    sumaInsert += zmierzCzas([&]() {
                        tablicaUni.insert(klucz, i);
                    });
                }
            } else {
                for (int i = 0; i < LICZBA_OPERACJI; i++) {
                    int klucz = klucze[r];
                    sumaInsert += zmierzCzas([&]() {
                        tablica.insert(klucz, i);
                    });
                    tablica.remove(klucz);
                }
            }
            
            // Pomiar remove
            for (int i = 0; i < LICZBA_OPERACJI; i++) {
                TypTabeli tablicaUni(rozmiarTablicy);
                wypelnijTabele(tablicaUni, dane);
                int klucz = dane[klucze[r] % dane.size()].first;
                sumaRemove += zmierzCzas([&]() {
                    tablicaUni.remove(klucz);
                });
            }
        }

        double sredniInsert = (sumaInsert / LICZBA_POWTORZEN) / LICZBA_OPERACJI;
        double sredniRemove = (sumaRemove / LICZBA_POWTORZEN) / LICZBA_OPERACJI;

        plik << n << ","
             << sredniInsert << ","
             << sredniRemove << "\n";
        plik.flush();

        std::cout << "  n=" << n
                  << "  insert=" << sredniInsert << " ns"
                  << "  remove=" << sredniRemove << " ns"
                  << std::endl;
    }

    plik.close();
}