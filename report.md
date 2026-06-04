# Miniprojekt 3 - Slownik oparty na tablicy mieszajacej

## 1. Cel projektu

Celem bylo zaimplementowanie kilku wariantow slownika hashujacego dla kluczy calkowitych i porownanie kosztu operacji `insert` oraz `remove`.

## 2. Zaimplementowane warianty

### 2.1. Lancuchowanie

Kazdy bucket przechowuje dynamiczny wektor par klucz-wartosc. To najprostszy wariant, latwy do zrozumienia i odporny na klasyczne kolizje.

### 2.2. Adresowanie otwarte z liniowym probkowaniem

Elementy sa trzymane bezposrednio w tablicy. Wariant ten zwykle dziala bardzo szybko przy malej liczbie kolizji, ale gorzej znosi wzrost wypelnienia.

### 2.3. Buckety AVL

Kazdy bucket jest drzewem AVL. Rozwiazanie jest ciezsze implementacyjnie, ale daje stabilniejszy koszt przeszukiwania koszyka.

## 3. Metodyka badan

Benchmark wykonuje nastepujace kroki:

1. Generuje losowo przemieszane, unikalne klucze calkowite.
2. Mierzy osobno wstawianie wszystkich elementow.
3. Mierzy osobno usuwanie wszystkich elementow.
4. Powtarza pomiar kilka razy i zapisuje srednia do CSV.

Testowane rozmiary danych w programie:

- 512,
- 2048,
- 8192,
- 16384.

## 4. Wyniki

Program zapisuje plik `results/benchmark_summary.csv` oraz surowy plik `results/benchmark_raw.csv`.

Wykresy sa renderowane w `docs/chart_viewer.html` po wskazaniu pliku CSV.

## 5. Wnioski

1. Lancuchowanie daje najprostsza i czytelna implementacje.
2. Adresowanie otwarte jest zwykle najbardziej konkurencyjne przy niskim obciazeniu.
3. Buckety AVL pomagaja utrzymac bardziej przewidywalny koszt przy trudniejszych danych.
4. Do projektu zaliczono trzy warianty, w tym wariant z drzewem AVL, wiec spelniono mocniejszy wariant wymagania.

## 6. Co nalezy zrobic przed oddaniem

1. Zbudowac projekt na komputerze z kompilatorem C++17.
2. Uruchomic benchmark i zapisac pliki CSV.
3. Wczytac CSV do viewer'a i wyeksportowac wykresy, jesli prowadzący wymaga obrazkow w sprawozdaniu.
4. Wkleic do tej sekcji ostateczne, rzeczywiste liczby z pomiarow.
