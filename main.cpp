#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <thread>


using Keywords = std::vector<std::string>;
using Result = std::pair<int, std::string>;
using Results = std::vector<Result>;
using Cache = std::unordered_map<std::string, bool>;


int minimum(int a, int b, int c) {
    if ((a < b) && (a < c)) {
        return a;
    } else if ((b < a) && (b < c)) {
        return b;
    }

    return c;
}

void print_matrix(std::vector<std::vector<int>>& matrix) {

    std::cout << "\nResultant Matrix:\n" << std::endl;
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix.front().size(); j++) {
            std::cout << matrix[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int levenshtein_distance(std::string& s1, std::string& s2) {
    std::vector<std::vector<int>> matrix;

    size_t len1 = s1.length() + 1;
    size_t len2 = s2.length() + 1;

    for (size_t i = 0; i < len1; i++) {
        std::vector<int> tmp;
        for (size_t j = 0; j < len2; j++) {
            tmp.push_back(0);
        }

        matrix.push_back(tmp);
    }

    for (size_t i = 1; i < len1; i++) {
        matrix[i][0] = i;
    }

    for (size_t j = 1; j < len2; j++) {
        matrix[0][j] = j;
    }


    for (size_t j = 1; j < len2; j++) {
        for (size_t i = 1; i < len1; i++) {
            int cost = 1;

            if (s1[i - 1] == s2[j - 1]) {
                cost = 0;
            }

            matrix[i][j] = minimum(
                matrix[i - 1][j] + 1,
                matrix[i][j - 1] + 1,
                matrix[i - 1][j - 1] + cost
            );
        }
    }

    // print_matrix(matrix);

    return matrix[len1 - 1][len2 - 1];
}

int find_most_matched(Results& results) {
    Result tmp;
    tmp.first = (int) std::pow(2, 31) - 1;  // simulate infinity
    tmp.second = "N/A";

    size_t idx = -1;

    for (size_t i = 0; i < results.size(); i++) {
        if (results[i].first < tmp.first) {
            tmp.first = results[i].first;
            tmp.second = results[i].second;

            idx = i;
        }
    }

    return idx;
}

int _abs(int x) {
    return x > 0 ? x : x * -1;
}

void run_analysist(Cache* cache, std::vector<std::string> words, std::string keyword) {
    if ((*cache)[keyword]) {
        return;
    }

    (*cache)[keyword] = true;
    bool found_exact = false;

    Results results;

    auto start = std::chrono::high_resolution_clock::now();
    for (std::string word : words) {
        // skip over if the length were much greater
        int len_diff = _abs(keyword.length() - word.length());
        if (len_diff > 2) {
            continue;
        }

        if (keyword == word) {
            found_exact = true;
            break;
        }

        int diff = levenshtein_distance(keyword, word);

        // somehow?
        if (diff == 0) {
            found_exact = true;
            break;
        }

        results.push_back(Result(diff, word));
    }
    auto dist_time = std::chrono::high_resolution_clock::now();

    if (found_exact) {
        return;
    }

    int matched = find_most_matched(results);
    if (matched != -1 && results[matched].first != 0) {
        std::cout << keyword << " -> " << results[matched].second;
        auto end = std::chrono::high_resolution_clock::now();
        auto dist_consumed = std::chrono::duration_cast<std::chrono::milliseconds>(dist_time - start).count();
        auto time_consumed = std::chrono::duration_cast<std::chrono::milliseconds>(end - dist_time).count();

        std::cout << " | (" << dist_consumed << " & " << time_consumed << " ms)" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file.txt>" << std::endl;
        return 0;
    }

    std::ifstream doc(argv[1]);
    std::vector<std::string> keywords;
    std::string line;
    Cache cache;

    if (!doc.is_open()) {
        std::cout << "Failed to open the file " << '"' << argv[1] << '"' << std::endl;
        return 1;
    }

    while (std::getline(doc, line)) {
        std::string keyword;
        std::istringstream token_stream(line);
        while (std::getline(token_stream, keyword, ' ')) {
            std::stringstream ss;
            // lowercasing and cleaning the keyword
            for (auto& ch : keyword) {
                ch = std::tolower(ch);
                if (ch >= 'a' && ch <= 'z') {
                    ss << ch;
                }
            }

            keyword = ss.str();

            if ((keyword.length() < 2) || (cache.count(keyword) > 0)) {
                continue;
            }

            cache[keyword] = false;
            keywords.push_back(keyword);
        }
    }
    doc.close();

    std::ifstream dict("./words.txt");
    std::vector<std::string> words;
    std::string word;

    while (std::getline(dict, word)) {
        words.push_back(word);
    }
    dict.close();

    std::thread thread_obj(run_analysist, &cache, words, keywords[10]);

    for (size_t i = 0; i < keywords.size();) {
        int thread_size = 10;
        if ((i + thread_size) >= keywords.size()) {
            thread_size = keywords.size() - i;
        }

        std::thread threads[thread_size];

        for (int j = 0; j < thread_size; j++) {
            threads[j] = std::thread(run_analysist, &cache, words, keywords[i+j]);
        }

        for (int j = 0; j < thread_size; j++) {
            threads[j].join();
        }

        i += thread_size;
    }
}
