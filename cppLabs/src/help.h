#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

void rand_gen_file(std::string file, int size, long long min, long long max);
bool are_files_equals(std::string file1, std::string file2);
void excel_write_line(std::string file, std::string data, std::chrono::system_clock::duration duration);
void excel_write_line(std::string file, std::string data, int line, std::chrono::system_clock::duration duration);
std::vector<std::pair<std::chrono::system_clock::duration, int>> trigger_program(std::string cmd, std::string args, int number_of_runs);
std::vector<std::pair<std::chrono::system_clock::duration, int>> trigger_program(int (*cmd)(), int number_of_runs);
std::string create_file_if_not_exists(std::filesystem::path path, std::string file_name, int size, int lower_bound, int upper_bound);
