//ngram prediction
#include "PredictiveText.h"
#include <algorithm>
#include <sstream>
#include <iterator>

void PredictiveText::Train(const std::string& text) {
    std::istringstream words(text);
    std::string prevWord, word;
    while (words >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (!prevWord.empty()) {
            AddToModel(prevWord, word);
        }
        prevWord = word;
    }
}

void PredictiveText::AddToModel(const std::string& previous, const std::string& next) {
    nGrams[previous].push_back(next);
}

std::string PredictiveText::Predict(const std::string& lastWord) {
    auto it = nGrams.find(lastWord);
    if (it != nGrams.end() && !it->second.empty()) {
        // Simple prediction: returning the first occurrence
        return it->second.front();
    }
    return "";
}

int PredictiveText::CountWords(const std::string& text) {
    std::istringstream iss(text);
    return std::distance(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>());
}
