#include <string>

void rand_gen_file(std::string file, int size, long long min, long long max);
bool are_files_equals(std::string file1, std::string file2);
void excel_write_line(std::string file, std::string data);
void excel_write_line(std::string file, std::string data, int line);
void trigger_program(std::string cmd, std::string args, int number_of_runs);
