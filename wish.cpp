#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

// Виводить єдине повідомлення про помилку за специфікацією OSTEP
void PrintOstepError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

vector<string> gSearchPath = {"/bin"};

vector<string> Tokenize(const string &line) {
    vector<string> tokens;
    istringstream iss(line);
    string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

string PadRedirectionOperator(const string &line) {
    string result;
    result.reserve(line.size());
    for (char c : line) {
        if (c == '>') result += " > ";
        else result += c;
    }
    return result;
}

// Розбиває рядок на незалежні підкоманди за символом '&'
vector<string> SplitOnAmpersand(const string &line) {
    vector<string> commands;
    string current;
    for (char c : line) {
        if (c == '&') {
            commands.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    commands.push_back(current);
    return commands;
}

bool SplitOnRedirection(const vector<string> &tokens, vector<string> &commandArgs, string &outputFile) {
    commandArgs.clear();
    outputFile.clear();

    int redirectCount = 0;
    size_t redirectPos = 0;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == ">") {
            redirectCount++;
            redirectPos = i;
        }
    }

    if (redirectCount == 0) {
        commandArgs = tokens;
        return true;
    }

    if (redirectCount > 1 || (tokens.size() - redirectPos - 1) != 1 || redirectPos == 0) {
        return false;
    }

    commandArgs.assign(tokens.begin(), tokens.begin() + redirectPos);
    outputFile = tokens[redirectPos + 1];
    return true;
}

enum class BuiltinResult { kNotBuiltin, kHandled };

BuiltinResult TryRunBuiltin(const vector<string> &tokens) {
    const string &cmd = tokens[0];

    if (cmd == "exit") {
        if (tokens.size() != 1) PrintOstepError();
        else exit(0);
        return BuiltinResult::kHandled;
    }
    if (cmd == "cd") {
        if (tokens.size() != 2) PrintOstepError();
        else if (chdir(tokens[1].c_str()) != 0) PrintOstepError();
        return BuiltinResult::kHandled;
    }
    if (cmd == "path") {
        gSearchPath.assign(tokens.begin() + 1, tokens.end());
        return BuiltinResult::kHandled;
    }
    return BuiltinResult::kNotBuiltin;
}

string FindExecutable(const string &command) {
    for (const string &dir : gSearchPath) {
        string candidate = dir + "/" + command;
        if (access(candidate.c_str(), X_OK) == 0) return candidate;
    }
    return "";
}

// Запускає зовнішню програму без очікування її завершення. Повертає PID процесу.
pid_t LaunchCommand(const vector<string> &args, const string &outputFile) {
    string executablePath = FindExecutable(args[0]);

    if (executablePath.empty()) {
        PrintOstepError();
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        PrintOstepError();
        return -1;
    }

    if (pid == 0) {
        // Логіка дочірнього процесу
        if (!outputFile.empty()) {
            int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd < 0) {
                PrintOstepError();
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
                PrintOstepError();
                exit(1);
            }
            close(fd);
        }

        vector<char *> execArgs;
        execArgs.reserve(args.size() + 1);
        for (const string &arg : args) {
            execArgs.push_back(const_cast<char *>(arg.c_str()));
        }
        execArgs.push_back(nullptr);

        execv(executablePath.c_str(), execArgs.data());
        PrintOstepError();
        exit(1);
    }

    // Повертаємо PID дочірнього процесу для подальшого виклику waitpid()
    return pid;
}

int main(int argc, char *argv[]) {
    bool isBatchMode = false;
    ifstream batchFile;
    istream *inputStream = &cin;

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

    while (true) {
        if (!isBatchMode) cout << "wish> ";
        if (!getline(*inputStream, line)) break;

        // Крок 1: Розбиваємо рядок на незалежні команди для паралельного виконання
        vector<string> commandStrings = SplitOnAmpersand(line);
        vector<pid_t> childPids;

        // Крок 2: Запускаємо всі підкоманди без очікування
        for (const string &commandStr : commandStrings) {
            vector<string> tokens = Tokenize(PadRedirectionOperator(commandStr));
            if (tokens.empty()) continue;

            vector<string> commandArgs;
            string outputFile;
            if (!SplitOnRedirection(tokens, commandArgs, outputFile)) {
                PrintOstepError();
                continue;
            }

            if (TryRunBuiltin(commandArgs) == BuiltinResult::kHandled) {
                continue;
            }

            pid_t pid = LaunchCommand(commandArgs, outputFile);
            if (pid > 0) {
                childPids.push_back(pid);
            }
        }

        // Крок 3: Очікуємо завершення всіх паралельно запущених процесів
        for (pid_t pid : childPids) {
            waitpid(pid, nullptr, 0);
        }
    }

    return 0;
}