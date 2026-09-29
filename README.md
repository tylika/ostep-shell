# wish — Unix Shell (OSTEP)

Простий Unix shell на C++: власні команди `exit`, `cd`, `path`, запуск зовнішніх програм, перенаправлення `>`, паралельне виконання `&`.

## Збірка та запуск

    g++ -Wall -Wextra -std=c++17 -o wish wish.cpp
    ./wish

## Тестування

Пройдено 20/20 офіційних тестів курсу (OSTEP, `processes-shell/tests`).

## AI

Під час роботи використовувала Claude (Anthropic):
- пояснення системних викликів (fork, execv, dup2, waitpid);
- побудова коду поетапно;
- допомога з git та середовищем WSL.