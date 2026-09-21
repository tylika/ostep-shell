#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

using namespace std;

// Виводить єдине повідомлення про помилку за специфікацією OSTEP
void PrintOstepError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Глобальний список директорій для пошуку виконуваних файлів
vector<string> gSearchPath = {"/bin"};

// Розбиває введений рядок на окремі аргументи (токени)
vector<string> Tokenize(const string &line) {
    vector<string> tokens;
    istringstream iss(line);
    string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

enum class BuiltinResult {
    kNotBuiltin,
    kHandled
};

// Обробка вбудованих команд (exit, cd, path)
BuiltinResult TryRunBuiltin(const vector<string> &tokens) {
    const string &cmd = tokens[0];

    if (cmd == "exit") {
        // Команда exit не повинна приймати аргументів
        if (tokens.size() != 1) {
            PrintOstepError();
        } else {
            exit(0);
        }
        return BuiltinResult::kHandled;
    }

    if (cmd == "cd") {
        // Команда cd вимагає рівно один аргумент
        if (tokens.size() != 2) {
            PrintOstepError();
        } else if (chdir(tokens[1].c_str()) != 0) {
            // Помилка при зміні директорії
            PrintOstepError();
        }
        return BuiltinResult::kHandled;
    }

    if (cmd == "path") {
        // Перезапис списку шляхів пошуку
        gSearchPath.assign(tokens.begin() + 1, tokens.end());
        return BuiltinResult::kHandled;
    }

    return BuiltinResult::kNotBuiltin;
}

int main(int argc, char *argv[]) {
    bool isBatchMode = false;
    ifstream batchFile;
    istream *inputStream = &cin;

    // Визначення режиму роботи (Interactive або Batch)
    if (argc == 1) {
        isBatchMode = false;
    } else if (argc == 2) {
        isBatchMode = true;
        batchFile.open(argv[1]);
        
        if (!batchFile.is_open()) {
            PrintOstepError();
            exit(1);
        }
        inputStream = &batchFile;
    } else {
        PrintOstepError();
        exit(1);
    }

    string line;

    // Головний цикл оболонки
    while (true) {
        // Виведення запрошення лише в інтерактивному режимі
        if (!isBatchMode) {
            cout << "wish> ";
        }

        // Читання рядка та перевірка на кінець файлу (EOF)
        if (!getline(*inputStream, line)) {
            break;
        }

        // Токенізація рядка
        vector<string> tokens = Tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        // Перевірка та запуск вбудованих команд
        if (TryRunBuiltin(tokens) == BuiltinResult::kHandled) {
            continue;
        }
    }

    return 0;
}