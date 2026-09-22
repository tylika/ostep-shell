#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

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
        if (tokens.size() != 1) {
            PrintOstepError();
        } else {
            exit(0);
        }
        return BuiltinResult::kHandled;
    }

    if (cmd == "cd") {
        if (tokens.size() != 2) {
            PrintOstepError();
        } else if (chdir(tokens[1].c_str()) != 0) {
            PrintOstepError();
        }
        return BuiltinResult::kHandled;
    }

    if (cmd == "path") {
        gSearchPath.assign(tokens.begin() + 1, tokens.end());
        return BuiltinResult::kHandled;
    }

    return BuiltinResult::kNotBuiltin;
}

// Перевіряє наявність файлу та права на виконання у вказаних директоріях
string FindExecutable(const string &command) {
    for (const string &dir : gSearchPath) {
        string candidate = dir + "/" + command;
        if (access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
    }
    return "";
}

// Запуск зовнішньої програми у дочірньому процесі
void ExecuteCommand(const vector<string> &args) {
    string executablePath = FindExecutable(args[0]);

    if (executablePath.empty()) {
        PrintOstepError();
        return;
    }

    // Створення нового процесу
    pid_t pid = fork();

    if (pid < 0) {
        PrintOstepError();
        return;
    }

    if (pid == 0) {
        // Логіка дочірнього процесу: підготовка аргументів для execv
        vector<char *> execArgs;
        execArgs.reserve(args.size() + 1);

        for (const string &arg : args) {
            execArgs.push_back(const_cast<char *>(arg.c_str()));
        }
        execArgs.push_back(nullptr); // Обов'язковий null-термінатор

        // Заміна образу процесу на нову програму
        execv(executablePath.c_str(), execArgs.data());

        // Якщо execv повернув керування — сталася помилка
        PrintOstepError();
        exit(1);
    } else {
        // Логіка батьківського процесу: очікування завершення дочірнього
        int status;
        waitpid(pid, &status, 0);
    }
}

int main(int argc, char *argv[]) {
    bool isBatchMode = false;
    ifstream batchFile;
    istream *inputStream = &cin;

    // Визначення режиму роботи
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
        if (!isBatchMode) {
            cout << "wish> ";
        }

        if (!getline(*inputStream, line)) {
            break; // Вихід при EOF
        }

        vector<string> tokens = Tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        if (TryRunBuiltin(tokens) == BuiltinResult::kHandled) {
            continue;
        }

        ExecuteCommand(tokens);
    }

    return 0;
}