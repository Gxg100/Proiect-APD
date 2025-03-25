#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <random>
#include <chrono>

using namespace std;

void generateRandomTextFile(const string& filename, int numWords) {
    ofstream file(filename);
    if (!file) {
        cerr << "Eroare la crearea fisierului!\n";
        return;
    }

    vector<string> wordPool = {
        "cuvant", "test", "exemplu", "program", "cod", "text", "procesare", "date", "frecventa", "map",
        "paralel", "thread", "vector", "informatica", "calcul", "fisier", "memorie", "procesor", "sistem", "stiva",
        "coada", "lista", "structura", "algoritm", "baza", "retea", "optimizare", "binar", "hexadecimal", "compilator",
        "interfata", "biblioteca", "executie", "model", "sincronizare", "cache", "directiva", "segment", "prioritate"
    };
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, wordPool.size() - 1);

    for (int i = 0; i < numWords; ++i) {
        file << wordPool[dist(gen)] << " ";
        if (i % 10 == 9) file << "\n";
    }
    file.close();
}

string normalizeWord(const string& word) {
    string result;
    for (char ch : word) {
        if (isalpha(ch)) {
            result += tolower(ch);
        }
    }
    return result;
}

int main() {
    string inputFilename = "text.txt";
    string outputFilename = "word_freq.txt";
    auto start = chrono::high_resolution_clock::now();

    generateRandomTextFile(inputFilename, 1000);

    ifstream file(inputFilename);
    if (!file) {
        cerr << "Eroare la deschiderea fisierului!\n";
        return 1;
    }

    unordered_map<string, int> wordCount;
    string line, word;

    while (getline(file, line)) {
        istringstream iss(line);
        while (iss >> word) {
            string normalized = normalizeWord(word);
            if (!normalized.empty()) {
                wordCount[normalized]++;
            }
        }
    }
    file.close();


    vector<pair<string, int>> sortedWords(wordCount.begin(), wordCount.end());
    sort(sortedWords.begin(), sortedWords.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    
    ofstream outFile(outputFilename);
    if (!outFile) {
        cerr << "Eroare la crearea fisierului de rezultate!\n";
        return 1;
    }

   
    outFile << "Timpul de executie: " << elapsed.count() << " secunde\n\n";

    
    for (const auto& pair : sortedWords) {
        outFile << pair.first << ": " << pair.second << '\n';
    }
    outFile.close();

    cout << "Rezultatele au fost salvate in " << outputFilename << "\n";

    return 0;
}
