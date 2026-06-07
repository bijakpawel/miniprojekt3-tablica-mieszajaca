#pragma once
#include <fstream>
#include <random>
#include <chrono>
#include <iostream>


// Liczba powtorzen calego eksperymentu dla danego rozmiaru. Wyniki sa usredniane.
constexpr int LICZBA_POWTORZEN = 8;

// Liczba operacji wykonywanych w jednej serii pomiarowej (peek, modify, extract).
// Pojedyncze wywolanie jest za szybkie aby zmierzyc je dokladnie zegarem,
// dlatego mierzymy laczny czas K wywolan i dzielimy.
constexpr int LICZBA_OPERACJI = 50;

// Liczba wywolan w pomiarze rozmiar() - operacja jest na tyle szybka,
// ze potrzeba duzej liczby powtorzen.
constexpr int LICZBA_OPERACJI_ROZMIAR = 100000;

// Rozmiary kolejki uzywane w pomiarach.
std::vector<int> domyslneRozmiary = {5000, 8000, 10000, 16000, 20000, 40000, 60000, 100000};



// Generuje plik z n losowymi liczbami calkowitymi z zakresu [-1000, 1000]
void generujLiczby(int n) {
    std::ofstream plik("liczby.txt");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distW(-1000, 1000);
    std::uniform_int_distribution<> distP(0, (n*5) - 1);

    for (int i = 0; i < n; i++) {
        plik << distW(gen) << "\n" << distP(gen) << "\n";
    }

    plik.close();
}

// Szablonowa funkcja do wypelniania kolejki z pliku
// Plausible powinien zawierac pary: wartosc priorytet na kazdym wierszu
template <typename KolejkaT>
void wypelnijKolejke(KolejkaT& kolejka, int n) {
    std::ifstream plik("liczby.txt");
    if (!plik) {
        std::cout << "Nie mozna otworzyc pliku!" << std::endl;
        return;
    }
    for (int i = 0; i < n && !plik.eof(); i++) {
        int wartosc, priorytet;
        plik >> wartosc >> priorytet;
        if (plik) {
            kolejka.insert(wartosc, priorytet);
        }
    }
    plik.close();
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


template <typename TypKolejki>
void zmierzTabele(const std::string& nazwaPliku) {
    std::ofstream plik(nazwaPliku);
    if (!plik) {
        std::cout << "Nie mozna otworzyc pliku do zapisu: " << nazwaPliku << std::endl;
        return;
    }
    plik << "rozmiar,insert,extract,peek,modify,rozmiarOp\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distWart(-1000, 1000);


    for (int n : domyslneRozmiary) {
        double sumaInsert = 0.0, sumaExtract = 0.0, sumaPeek = 0.0;
        double sumaModify = 0.0, sumaRozmiar = 0.0;

        std::uniform_int_distribution<> distPrio(0, (n*5) - 1);


        std::cout << "Pomiar insert i peak dla n=" << n << "..." << std::endl;

        for (int r = 0; r < LICZBA_POWTORZEN; r++) {
            generujLiczby(n);
            
            // Pre-generujemy aby pomiar nie obejmowal kosztu losowania.
            int wart = distWart(gen);
            int prio = distPrio(gen);
            int prioPodmiana = distPrio(gen);

            for (int i = 0; i < LICZBA_OPERACJI; i++) {
                TypKolejki kolejka;
                wypelnijKolejke(kolejka, n);
                sumaPeek += zmierzCzas([&]() {
                    volatile int dummy = 0;
                    dummy += kolejka.peek().wartosc;
                    (void)dummy;
                });
                sumaInsert += zmierzCzas([&]() {
                    volatile int dummy = 0;
                    kolejka.insert(wart, prio);
                    (void)dummy;
                });
            }


            std::cout << "Pomiar rozmiar dla n=" << n << "..." << std::endl;
            // --- Pomiar rozmiar: bardzo szybka operacja, wiele powtorzen ---
            TypKolejki kolejka;
            wypelnijKolejke(kolejka, n);
            sumaRozmiar += zmierzCzas([&]() {
                volatile int dummy = 0;
                for (int i = 0; i < LICZBA_OPERACJI_ROZMIAR; i++) {
                    dummy += kolejka.rozmiar();
                }
                (void)dummy;
            });

            std::cout << "Pomiar modify dla n=" << n << "..." << std::endl;
            // --- Pomiar modify: K modyfikacji na pelnej kolejce ---
            for (int i = 0; i < LICZBA_OPERACJI; i++) {
                TypKolejki kolejka;
                wypelnijKolejke(kolejka, n);
                sumaModify += zmierzCzas([&]() {
                    volatile int dummy = 0;
                    dummy += kolejka.modifyKey(prio, prioPodmiana);
                    (void)dummy;
                });
            }

            std::cout << "Pomiar extract dla n=" << n << "..." << std::endl;
            // --- Pomiar extract: K wyciagniec (rozmiar maleje, ale K << n) ---
            for (int i = 0; i < LICZBA_OPERACJI; i++) {
                TypKolejki kolejka;
                wypelnijKolejke(kolejka, n);
                sumaExtract += zmierzCzas([&]() {
                    volatile int dummy = 0;
                    dummy += kolejka.extractMax().wartosc;
                    (void)dummy;
                });
            }
        }

        double sredniInsert = (sumaInsert / LICZBA_POWTORZEN)/ LICZBA_OPERACJI;
        double sredniExtract = (sumaExtract / LICZBA_POWTORZEN) / LICZBA_OPERACJI;
        double sredniPeek = (sumaPeek / LICZBA_POWTORZEN) / LICZBA_OPERACJI;
        double sredniModify = (sumaModify / LICZBA_POWTORZEN) / LICZBA_OPERACJI;
        double sredniRozmiar = (sumaRozmiar / LICZBA_POWTORZEN) / LICZBA_OPERACJI_ROZMIAR;

        plik << n << ","
             << sredniInsert << ","
             << sredniExtract << ","
             << sredniPeek << ","
             << sredniModify << ","
             << sredniRozmiar << "\n";
        plik.flush();

        std::cout << "  n=" << n
                  << "  insert=" << sredniInsert << " ns"
                  << "  extract=" << sredniExtract << " ns"
                  << "  peek=" << sredniPeek << " ns"
                  << "  modify=" << sredniModify << " ns"
                  << "  rozmiar=" << sredniRozmiar << " ns"
                  << std::endl;
    }

    plik.close();
}