#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

class PredictiveText {
public:
    void Train(const std::string& text);
    std::string Predict(const std::string& lastWord);
    static int CountWords(const std::string& text);

private:
    std::unordered_map<std::string, std::vector<std::string>> nGrams;
    void AddToModel(const std::string& previous, const std::string& next);
};
