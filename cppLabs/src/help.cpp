#include "help.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <random>

void rand_gen_file(std::string file, int size, long long min, long long max) {
  srand((int)time(0));
  std::ofstream fout(file, std::ofstream::out);
  if(!fout.is_open()) {
    throw std::runtime_error("Couldn't open " + file);
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> uid(min, max);
  for(int i = 0; i < size; i++) {
    fout << uid(gen);
    if(i != size - 1) {
      fout << ",";
    }
  }

  fout << "\n";
  fout.close();
}

bool are_files_equals(std::string file1, std::string file2) {
  bool are_equal = true;
  std::ifstream fin1(file1);
  if(!fin1.is_open()) {
    throw std::runtime_error("Couldn't open " + file1);
  }

  std::ifstream fin2(file2);
  if(!fin2.is_open()) {
    throw std::runtime_error("Couldn't open " + file2);
  }

  std::string line1, line2;
  while(!fin1.eof() && !fin2.eof()) {
    std::getline(fin1, line1, '\n');
    std::getline(fin2, line2, '\n');
    if(line1.compare(line2) != 0) {
      are_equal = false;
      break;
    }
  }

  if(fin1.eof() != fin2.eof()) {
    are_equal = false;
  }

  fin1.close();
  fin2.close();
  return are_equal;
}

void excel_write_line(std::string file, std::string data, std::chrono::system_clock::duration duration) {
  std::ofstream fout(file, std::ios::out | std::ios_base::app);
  if(fout.is_open()) {
    throw std::runtime_error("Couldn't open " + file);
  }

  //auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  //auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  //auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count();

  fout << "Milliseconds:" << milliseconds << "," << data << "\n";
  fout.close();
}

void excel_write_line(std::string file, std::string data, int line, std::chrono::system_clock::duration duration) {
  std::vector<std::string> lines;
  try {
    std::ifstream fin(file);
    std::string s;
    while(!fin.eof()) {
      std::getline(fin, s, '\n');
      lines.push_back(s);
    }
  }
  catch(std::runtime_error error) {}

  auto it = lines.begin();
  for(int i = 0; i < line; i++) {
    ++it;
  }

  //auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  //auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  //auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count();
  lines.insert(it, "Milliseconds:" + milliseconds + ',' + data);

  std::ofstream fout(file);
  if(fout.is_open()) {
    throw std::runtime_error("Couldn't open " + file);
  }

  for(auto s : lines) {
    fout << s << "\n";
  }

  fout.close();
}

std::vector<std::pair<std::chrono::system_clock::duration, int>> trigger_program(std::string cmd, std::string args, int number_of_runs) {
  std::string fullCmd = cmd + ' ' + args;
  std::vector<std::pair<std::chrono::system_clock::duration, int>> duration_and_status;
  for(int i = 0; i < number_of_runs; i++) {
    auto start = std::chrono::system_clock::now();
    int result = system(fullCmd.c_str());
    auto stop = std::chrono::system_clock::now();
    duration_and_status.push_back({ stop - start, result });
  }

  return duration_and_status;
}

std::vector<std::pair<std::chrono::system_clock::duration, int>> trigger_program(int(*cmd)(), int number_of_runs) {
  std::vector<std::pair<std::chrono::system_clock::duration, int>> duration_and_status;
  for(int i = 0; i < number_of_runs; i++) {
    auto start = std::chrono::system_clock::now();
    int result = cmd();
    auto stop = std::chrono::system_clock::now();
    duration_and_status.push_back({ stop - start, result });
  }

  return duration_and_status;
}

std::string create_file_if_not_exists(std::filesystem::path path, std::string file_name, int size, int lower_bound, int upper_bound) {
  auto _path = std::filesystem::path(path);
  _path.append(file_name);
  if(!std::filesystem::exists(_path)) {
    rand_gen_file(_path.string(), size, lower_bound, upper_bound);
  }

  return _path.string();
}
