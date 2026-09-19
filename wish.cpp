#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

using namespace std;

// Виводить єдине повідомлення про помилку за специфікацією OSTEP
void PrintOstepError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
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
        
        // Перевірка на успішне відкриття файлу
        if (!batchFile.is_open()) {
            PrintOstepError();
            exit(1);
        }
        inputStream = &batchFile;
    } else {
        // Помилка: передано більше одного аргументу
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

        // Пропуск порожніх рядків
        if (line.empty()) {
            continue;
        }
    }

    return 0;
}