#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <iostream>

int main() {
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < 10000000; i++) { for (int j = 0; j < i; j++) { } }
    
    auto stop = std::chrono::system_clock::now();
    auto duration = stop - start;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % (int) 1e3;
    auto milliseconds = (std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % (int) 1e6) / (int) 1e3;
    auto seconds = (std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % (int) 1e9) / (int) 1e6;
    std::cout << std::chrono::system_clock::to_time_t(start) << "\n" 
              << std::chrono::system_clock::to_time_t(stop) << "\n" 
              << "microseconds " << microseconds << "\n" 
              << "milliseconds " << milliseconds << "\n" 
              << "seconds " << seconds;
}