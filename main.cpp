#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <locale.h>
#include "Windows.h"
#include <fcntl.h>
#include <io.h>

using namespace std;

const int THRESHOLD_PRINT_VALUE = 1000000;

class FileChecker
{
private:
    wstring             filename_;
    wifstream           file_to_check_;
    vector<wchar_t>     punctuation_symbols_;
    vector<wchar_t>     part_of_word_punctuation_symbols_;
    vector<wchar_t>     digits_symbols_;
    vector<wchar_t>     ua_letters_symbols_;
    vector<wchar_t>     en_letters_symbols_;
    vector<wstring>     filecontent_;
    map<wchar_t, int>   letters_number_table_;
    map<wchar_t, int>   table_of_the_number_of_en_letters_;
    map<wchar_t, int>   table_of_the_number_of_ua_letters_;
    map<wchar_t, int>   table_of_the_number_of_digits_;
    map<wchar_t, int>   table_of_the_number_of_punctuation_symbols_;
    map<int, int>       word_sizes_number_table_;
    int                 number_of_lines_;
    int                 number_of_symbols_;
    //std::ctype<wchar_t> ua_facet;

    // methods
    void setFilename(const wstring& s);
    bool openFileByName();
    void getFilecontent();
    bool isSymbolEnLetter(const wchar_t& c);
    bool isSymbolUaLetter(const wchar_t& c);
    bool isSymbolDigit(const wchar_t& c);
    bool isSymbolPunctuationSymbol(const wchar_t& c);
    void parseFilecontent(int languageType);

public:
    FileChecker(/* args */);
    ~FileChecker();

    // methods
    bool startCheck(const wstring& filename, int languageType);
    void printResults(int tempNumOfSym, int languageType);
};

FileChecker::FileChecker(/* args */)
{
    //ua_loc.global(locale("ru_RU.UTF-8"));
    //en_loc.global(locale("en-US"));

    punctuation_symbols_ = {',', '.', '!', '?', ':', ';', '"', '/', '(', ')', '-'};
    part_of_word_punctuation_symbols_ = {'`', '\''};
    digits_symbols_ = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    ua_letters_symbols_ = {L'а', L'А', L'б', L'Б', L'в', L'В', L'г', L'Г', L'ґ', L'Ґ', L'д', L'Д', L'е', L'Е', L'є', L'Є',
                            L'ж', L'Ж', L'з', L'З', L'и', L'И', L'і', L'І', L'ї', L'Ї', L'й', L'Й', L'к', L'К', L'л', L'Л',
                            L'м', L'М', L'н', L'Н', L'о', L'О', L'п', L'П', L'р', L'Р', L'с', L'С', L'т', L'Т', L'у', L'У',
                            L'ф', L'Ф', L'х', L'Х', L'ц', L'Ц', L'ч', L'Ч', L'ш', L'Ш', L'щ', L'Щ', L'ь', L'Ь', L'ю', L'Ю',
                            L'я', L'Я'};
    en_letters_symbols_ = {'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', 'g', 'G', 'h', 'H', 'i', 'I', 'j',
                            'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N', 'o', 'O', 'p', 'P', 'q', 'Q', 'r', 'R', 's', 'S',
                            't', 'T', 'u', 'U', 'v', 'V', 'w', 'W', 'x', 'X', 'y', 'Y', 'z', 'Z'};

    for (const auto& le : en_letters_symbols_)
    {
        table_of_the_number_of_en_letters_[towupper(le)] = 0;
    }

    for (const auto& le : ua_letters_symbols_)
    {
        table_of_the_number_of_ua_letters_[towupper(le)] = 0;
    }
}

FileChecker::~FileChecker()
{
}

void FileChecker::setFilename(const wstring& s)
{
    filename_ = s;
}

bool FileChecker::openFileByName()
{
    if (L"" != filename_)
    {
        file_to_check_.open(filename_.c_str());
        if (file_to_check_.is_open())
        {
            // File opened - fine
            file_to_check_.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
            return true;
        }
    }
    // Fail to open file with name filename_
    return false;
}

void FileChecker::getFilecontent()
{
    wstring buff = L"";
    number_of_lines_ = 0;
    number_of_symbols_ = 0;

    while (getline(file_to_check_, buff))
    {
        number_of_symbols_ += buff.size();

        filecontent_.push_back(buff);
        ++number_of_lines_;

        buff = L"";
    }
    file_to_check_.close();
}

bool FileChecker::isSymbolEnLetter(const wchar_t& c)
{
    if (en_letters_symbols_.end() != find(en_letters_symbols_.begin(), en_letters_symbols_.end(), c))
    {
        return true;
    }
    return false;
}

bool FileChecker::isSymbolUaLetter(const wchar_t& c)
{
    if (ua_letters_symbols_.end() != find(ua_letters_symbols_.begin(), ua_letters_symbols_.end(), c))
    {
        return true;
    }
    return false;
}

bool FileChecker::isSymbolDigit(const wchar_t& c)
{
    if (digits_symbols_.end() != find(digits_symbols_.begin(), digits_symbols_.end(), c))
    {
        return true;
    }
    return false;
}

bool FileChecker::isSymbolPunctuationSymbol(const wchar_t& c)
{
    if (punctuation_symbols_.end() != find(punctuation_symbols_.begin(), punctuation_symbols_.end(), c))
    {
        return true;
    }
    return false;
}

void FileChecker::parseFilecontent(int languageType)
{
    int simbolNum = 0;

    for (const wstring& line : filecontent_)
    {
        int wordSize = 0;
        for (const wchar_t& symbol : line)
        {
            if (1 == languageType)
            {
                if (isSymbolEnLetter(symbol))
                {
                    ++table_of_the_number_of_en_letters_[static_cast<wchar_t>(towupper(symbol))];
                    ++wordSize;
                }
            }
            else if (isSymbolUaLetter(symbol))
            {
                ++table_of_the_number_of_ua_letters_[static_cast<wchar_t>(towupper(symbol))];
                ++wordSize;
            }

            if (isdigit(symbol))
            {
                ++table_of_the_number_of_digits_[symbol];
                ++wordSize;
            }
            else if (isSymbolPunctuationSymbol(symbol))
            {
                if (0 < wordSize)
                {
                    ++word_sizes_number_table_[wordSize];
                }
                wordSize = 0;
                ++table_of_the_number_of_punctuation_symbols_[symbol];
            }
            else if (isspace(symbol))
            {
                if (0 < wordSize)
                {
                    ++word_sizes_number_table_[wordSize];
                }
                wordSize = 0;
            }
            else
            {
                // invalid symbol
            }

            ++simbolNum;
            if (0 == (simbolNum % THRESHOLD_PRINT_VALUE))
            {
                double partOfText = static_cast<double>(simbolNum) / number_of_symbols_;
                int percentageOfText = 100 * partOfText;
                cout << endl;
                wcout << "Executed " << percentageOfText << "% of file's text." << endl;
                wcout << "Statistic:" << endl;
                printResults(simbolNum, languageType);
            }
        }
    }

    wcout << endl;
    wcout << "Executed 100% of file's text." << endl;
    wcout << "Statistic:" << endl;
    printResults(number_of_symbols_, languageType);
}

void FileChecker::printResults(int tempNumOfSym, int languageType)
{
    wcout << "--------------------------------------------------------------" << endl;

    // Number of symbols
    wcout << "Number of symbols: " << tempNumOfSym << endl;
    wcout << endl;

    // Frequency of letter use
    wcout << "Table of frequency of letter use:" << endl;
    wcout << " _________________ " << endl;
    wcout << "|" << setw(6) << "Letter" << "|" << setw(10) << "Frequency" << "|" << endl;
    wcout << "|" << setw(6) << "______" << "|" << setw(10) << "__________" << "|" << endl;

    //table_of_the_number_of_en_letters_ = {{'C', 20}, {'A', 100}, {'B', 2011}};
    if (1 == languageType)
    {
        for (const auto& le : table_of_the_number_of_en_letters_)
        {
            wcout << "|" << setw(6) << le.first << "|" << setw(10) << le.second << "|" << endl;
            wcout << "|" << setw(6) << "______" << "|" << setw(10) << "__________" << "|" << endl;
        }
    }
    else
    {
        for (const auto& le : table_of_the_number_of_ua_letters_)
        {
            wcout << "|" << setw(6) << le.first << "|" << setw(10) << le.second << "|" << endl;
            wcout << "|" << setw(6) << "______" << "|" << setw(10) << "__________" << "|" << endl;
        }
    }
    wcout << endl;

    // Frequency of digit use
    wcout << "Table of frequency of digit use:" << endl;
    wcout << " _________________ " << endl;
    wcout << "|" << setw(6) << "Digit" << "|" << setw(10) << "Frequency" << "|" << endl;
    wcout << "|" << setw(6) << "______" << "|" << setw(10) << "__________" << "|" << endl;

    //table_of_the_number_of_digits_ = {{'8', 111}, {'2', 2325}, {'3', 1335}};
    for (const auto& le : table_of_the_number_of_digits_)
    {
        wcout << "|" << setw(6) << le.first << "|" << setw(10) << le.second << "|" << endl;
        wcout << "|" << setw(6) << "______" << "|" << setw(10) << "__________" << "|" << endl;
    }
    wcout << endl;

    // Frequency of p-symbol use
    wcout << "Table of frequency of p-symbol use:" << endl;
    wcout << " ___________________ " << endl;
    wcout << "|" << setw(8) << "p-symbol" << "|" << setw(10) << "Frequency" << "|" << endl;
    wcout << "|" << setw(8) << "________" << "|" << setw(10) << "__________" << "|" << endl;

    //table_of_the_number_of_punctuation_symbols_ = {{':', 20}, {';', 331}, {'.', 2020}};
    for (const auto& le : table_of_the_number_of_punctuation_symbols_)
    {
        wcout << "|" << setw(8) << le.first << "|" << setw(10) << le.second << "|" << endl;
        wcout << "|" << setw(8) << "________" << "|" << setw(10) << "__________" << "|" << endl;
    }
    wcout << endl;

    // Frequency of word size use
    wcout << "Table of frequency of word size use:" << endl;
    wcout << " _______________________________ " << endl;
    wcout << "|" << setw(20) << "Word size (letters)" << "|" << setw(10) << "Frequency" << "|" << endl;
    wcout << "|" << setw(20) << "____________________" << "|" << setw(10) << "__________" << "|" << endl;

    //word_sizes_number_table_ = {{3, 20}, {5, 331}, {1, 2020}};
    for (const auto& le : word_sizes_number_table_)
    {
        wcout << "|" << setw(20) << le.first << "|" << setw(10) << le.second << "|" << endl;
        wcout << "|" << setw(20) << "____________________" << "|" << setw(10) << "__________" << "|" << endl;
    }
    wcout << endl;
}

bool FileChecker::startCheck(const wstring& filename, int languageType)
{
    setFilename(filename);
    if (openFileByName())
    {
        getFilecontent();
        parseFilecontent(languageType);
        return true;
    }
    else
    {
        wcout << "Cannot open file, invalid filename: " << filename_ << endl;
        return false;
    }
    
}

int main()
{
    // SetConsoleCP(1251);
    // SetConsoleOutputCP(1251);

    _setmode(_fileno(stdout), _O_U16TEXT);

    // wcout << L"Привіт, як справи?" << endl;
    // wcout << "Hello, how are you?" << endl;

    wstring filename;
    int languageType;
    wcout << "Write filename: ";
    wcin >> filename;
    wcout << "Choose language of text file:" << endl;
    wcout << " 1. English." << endl;
    wcout << " 2. Ukrainian." << endl;
    wcout << "Your choose: ";
    wcin >> languageType;

    if (1 == languageType || 2 == languageType)
    {
        if (2 == languageType)
        {
            setlocale(LC_ALL, "ukr");
        }
        
        FileChecker checker;
        checker.startCheck(filename, languageType);
    }
    else
    {
        wcout << "Invalid language for text file!" << endl;
    }

    return 0;
}