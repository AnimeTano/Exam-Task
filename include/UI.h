#pragma once


#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <vector>
#include <sstream>


class UI {
public:
    static void clearInputBuffer() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    static void printSection(const std::string& title) {
        std::cout << "  " << title << "\n";
    }
    
    static void printError(const std::string& message) {
        std::cerr << "Error: " << message << "\n";
    }
    
    static void printSuccess(const std::string& message) {
        std::cout << message << "\n";
    }
    
    static void printInfo(const std::string& message) {
        std::cout << "Info: " << message << "\n";
    }
    
    static void waitForEnter() {
        std::cout << "\nPress Enter to continue";
        clearInputBuffer();
        std::cin.get();
    }
    
    template<typename T>
    static T getInput(const std::string& prompt, T min = std::numeric_limits<T>::min(), 
                     T max = std::numeric_limits<T>::max()) {
        T value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                if (value >= min && value <= max) {
                    clearInputBuffer();
                    return value;
                } else {
                    std::cout << "Enter a value between " << min << " and " << max << ".\n";
                }
            } else {
                std::cout << "Invalid input\n";
                clearInputBuffer();
            }
        }
    }
    
    static std::string getStringInput(const std::string& prompt, bool allowEmpty = false) {
        std::string input;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, input);
            
            if (allowEmpty || !input.empty()) {
                return input;
            }
            std::cout << "Input cannot be empty\n";
        }
    }
    
    static double getDoubleInput(const std::string& prompt, double min = 0.0) {
        double value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                if (value >= min) {
                    clearInputBuffer();
                    return value;
                } else {
                    std::cout << "Value must be at least " << min << ".\n";
                }
            } else {
                std::cout << "Invalid input\n";
                clearInputBuffer();
            }
        }
    }
};