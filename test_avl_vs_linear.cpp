#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <chrono>
#include <random>
#include <algorithm>

// Gera um dicionario de gaps (valores pares tipicos ate ~500)
std::vector<int> generate_gaps() {
    std::vector<int> gaps;
    for (int i = 2; i <= 500; i += 2) {
        gaps.push_back(i);
    }
    // Adiciona alguns gaps grandes (valencias)
    gaps.push_back(1000);
    gaps.push_back(2000);
    gaps.push_back(4000);
    return gaps;
}

// 1. Busca Linear (atual)
bool test_linear(const std::vector<int>& gaps, int delta_N) {
    for (int g : gaps) {
        if (delta_N % g == 0) {
            return true;
        }
    }
    return false;
}

// 2. Busca com std::set (iteracao, pois precisamos testar divisibilidade)
bool test_set(const std::set<int>& gap_set, int delta_N) {
    for (int g : gap_set) {
        if (delta_N % g == 0) {
            return true;
        }
    }
    return false;
}

// 3. Busca com std::unordered_set (iteracao, mesma logica)
bool test_hash(const std::unordered_set<int>& gap_hash, int delta_N) {
    for (int g : gap_hash) {
        if (delta_N % g == 0) {
            return true;
        }
    }
    return false;
}

int main() {
    // =============================================
    // Preparacao dos dados
    // =============================================
    std::vector<int> gaps = generate_gaps();
    std::set<int> gap_set(gaps.begin(), gaps.end());
    std::unordered_set<int> gap_hash(gaps.begin(), gaps.end());

    // Gerar 1.000.000 valores aleatorios de delta_N
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(100000, 999999999);

    std::vector<int> test_values;
    test_values.reserve(1000000);
    for (int i = 0; i < 1000000; ++i) {
        test_values.push_back(dist(gen));
    }

    // =============================================
    // Teste 1: Busca Linear
    // =============================================
    auto start = std::chrono::high_resolution_clock::now();
    int count_linear = 0;
    for (int v : test_values) {
        if (test_linear(gaps, v)) count_linear++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time_linear = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // =============================================
    // Teste 2: Busca com std::set
    // =============================================
    start = std::chrono::high_resolution_clock::now();
    int count_set = 0;
    for (int v : test_values) {
        if (test_set(gap_set, v)) count_set++;
    }
    end = std::chrono::high_resolution_clock::now();
    auto time_set = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // =============================================
    // Teste 3: Busca com std::unordered_set
    // =============================================
    start = std::chrono::high_resolution_clock::now();
    int count_hash = 0;
    for (int v : test_values) {
        if (test_hash(gap_hash, v)) count_hash++;
    }
    end = std::chrono::high_resolution_clock::now();
    auto time_hash = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // =============================================
    // Teste 4: Busca binaria para um valor EXATO
    // =============================================
    std::vector<int> sorted_gaps = gaps;
    std::sort(sorted_gaps.begin(), sorted_gaps.end());
    start = std::chrono::high_resolution_clock::now();
    int count_binary = 0;
    for (int v : test_values) {
        if (std::binary_search(sorted_gaps.begin(), sorted_gaps.end(), v)) {
            count_binary++;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto time_binary = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // =============================================
    // Resultados
    // =============================================
    std::cout << "================================================\n";
    std::cout << "  BENCHMARK: BUSCA NO DICIONARIO DE GAPS\n";
    std::cout << "================================================\n";
    std::cout << "Tamanho do dicionario: " << gaps.size() << " gaps\n";
    std::cout << "Valores testados: " << test_values.size() << "\n\n";

    std::cout << "[1] Linear (std::vector)    : " << time_linear.count() << " ms  (hits: " << count_linear << ")\n";
    std::cout << "[2] std::set (iteracao)     : " << time_set.count() << " ms  (hits: " << count_set << ")\n";
    std::cout << "[3] std::unordered_set      : " << time_hash.count() << " ms  (hits: " << count_hash << ")\n";
    std::cout << "[4] Binary Search (exato)   : " << time_binary.count() << " ms  (hits: " << count_binary << ")\n\n";

    if (time_linear < time_set && time_linear < time_hash) {
        std::cout << " MELHOR RESULTADO: Busca Linear (std::vector)\n";
        std::cout << "  Motivo: baixo overhead de cache e iteracao contigua.\n";
        std::cout << "  Para tamanhos pequenos (~250), a arvore balanceada e DESNECESSARIA.\n";
    } else {
        std::cout << " std::set ou unordered_set teve desempenho similar ou melhor.\n";
    }

    return 0;
}
