#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> readLinesFromFile(const std::string &filename)
{
    std::vector<std::string> lines;
    std::ifstream inFile(filename);
    std::string line;

    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }

    inFile.close();
    return lines;
}

std::vector<std::pair<std::string, std::string>>
readReplaceWords(const std::string &filename)
{
    std::vector<std::pair<std::string, std::string>> replaceWords;
    std::ifstream inFile(filename);
    std::string line;

    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string first, second;
        if (iss >> first >> second) {
            replaceWords.emplace_back(first, second);
        }
    }

    inFile.close();
    return replaceWords;
}

bool containsAnyWord(const std::string &text,
                     const std::vector<std::string> &words)
{
    for (const auto &word : words) {
        if (text.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string processLine(
    const std::string &line, const std::vector<std::string> &rejectWords,
    const std::vector<std::pair<std::string, std::string>> &replaceWords)
{
    if (containsAnyWord(line, rejectWords)) {
        return "";
    }

    std::string result = line;

    // Replace words from replaceWords
    for (const auto &pair : replaceWords) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }

    // Remove Chinese phone numbers
    std::regex phoneRegex(R"(\b1\d{10}\b)");
    result = std::regex_replace(result, phoneRegex, "");

    return result;
}

int main()
{
    std::vector<std::string> inputLines = readLinesFromFile("input.txt");
    std::vector<std::string> rejectWords = readLinesFromFile("reject.txt");
    std::vector<std::string> askLines = readLinesFromFile("ask.txt");
    std::vector<std::pair<std::string, std::string>> replaceWords =
        readReplaceWords("replace.txt");

    std::ofstream outFile("output.txt");

    for (const auto &line : askLines) {
        outFile << line << std::endl;
    }

    for (const auto &line : inputLines) {
        std::string processedLine = processLine(line, rejectWords, 
                                                replaceWords);
        if (!processedLine.empty()) {
            outFile << processedLine << std::endl;
        }
    }

    outFile.close();
    return 0;
}
