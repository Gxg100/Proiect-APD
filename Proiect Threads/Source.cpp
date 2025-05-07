#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <cctype>
#include <chrono>
#include <algorithm>

using namespace std;

mutex mtx;

string normalizeWord(const string& word) {
    string result;
    for (size_t i = 0; i < word.length(); ++i) {
        if (isalpha(static_cast<unsigned char>(word[i]))) {
            result += tolower(static_cast<unsigned char>(word[i]));
        }
    }
    return result;
}

void countWords(const vector<string>& lines, unordered_map<string, int>& localMap) {
    for (size_t i = 0; i < lines.size(); ++i) {
        istringstream iss(lines[i]);
        string word;
        while (iss >> word) {
            string norm = normalizeWord(word);
            if (!norm.empty()) {
                localMap[norm]++;
            }
        }
    }
}

int main() {
    
    const string inputFilename = "C:\\Users\\stanc\\source\\repos\\Proiect APD_MPI\\x64\\Debug\\Text1.txt";
 

    const string outputFilename = "word_freq_threads.txt";
    const int numThreads = static_cast<int>(thread::hardware_concurrency());

    ifstream file(inputFilename);
    if (!file) {
        cerr << "Eroare la deschiderea fisierului!" << endl;
        return 1;
    }

    
    vector<string> allLines;
    string line;
    while (getline(file, line)) {
        allLines.push_back(line);
    }
    file.close();

    
    auto start = chrono::high_resolution_clock::now();

    vector<thread> threads;
    vector<unordered_map<string, int>> localMaps(numThreads);
    size_t totalLines = allLines.size();
    size_t chunkSize = (totalLines + numThreads - 1) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        size_t startIdx = i * chunkSize;
        size_t endIdx = min(startIdx + chunkSize, totalLines);

        vector<string> chunk(allLines.begin() + startIdx, allLines.begin() + endIdx);

        threads.emplace_back([&, i, chunk]() {
            countWords(chunk, localMaps[i]);
            });
    }

    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    
    unordered_map<string, int> globalMap;
    for (size_t i = 0; i < localMaps.size(); ++i) {
        unordered_map<string, int>& local = localMaps[i];
        for (unordered_map<string, int>::iterator it = local.begin(); it != local.end(); ++it) {
            globalMap[it->first] += it->second;
        }
    }

    
    vector<pair<string, int>> sortedWords(globalMap.begin(), globalMap.end());
    sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
        });

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    
    ofstream out(outputFilename);
    if (!out) {
        cerr << "Eroare la scrierea fi?ierului de iesire!" << endl;
        return 1;
    }

    for (size_t i = 0; i < sortedWords.size(); ++i) {
        out << sortedWords[i].first << ": " << sortedWords[i].second << '\n';
    }

    out << "\nTimp total de executie: " << elapsed.count() << " secunde\n";
    out.close();

    cout << "Rezultatele au fost salvate in " << outputFilename << endl;
    return 0;
}
