#pragma once


#include <iostream>
#include <string>


#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif


class ConsoleUtils {
    public:
        static void setupConsole() {
    #ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
    #endif
            std::locale::global(std::locale(""));
            std::cout.imbue(std::locale());
            std::cin.imbue(std::locale());
            std::cerr.imbue(std::locale());
        }
        
        static void printLine(const std::string& text) {
            std::cout << text << "\n";
        }
        
        static void printSuccess(const std::string& text) {
            std::cout << "Success: " << text << "\n";
        }
        
        static void printError(const std::string& text) {
            std::cerr << "Error: " << text << "\n";
        }
};