//
// Created by Des Caldnd on 3/27/2024.
//
#include "server.h"
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Creating server..." << std::endl;

    // Создаем сервер
    server s;

    std::cout << "Server created successfully. Test completed." << std::endl;

    // При выходе из main, будет вызван деструктор server,
    // который корректно остановит сервер и дождется завершения потока
    return 0;
}