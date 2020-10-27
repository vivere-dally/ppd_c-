#define _CRTDBG_MAP_ALLOC
#include "help.h"
#include "lab2_main.h"
#include "SequentialPrimitiveImg.h"
#include "ParallelPrimitiveImg.h"
#include <crtdbg.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>

void print_matrix(long long** matrix, int rows, int columns) {
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < columns; j++) {
      std::cout << matrix[i][j] << " ";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

bool deep_equals(long long** a, long long** b, int rows, int columns) {
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < columns; j++) {
      if(a[i][j] != b[i][j]) {
        return false;
      }
    }
  }

  return true;
}

void test() {
  long long M = 1000, N = 1000, m = 5, n = 5, p = 4;
  int* kernel_numbers = new int[m * n];
  for(int i = 0; i < m * n; i++) {
    kernel_numbers[i] = i + 1;
  }

  int* img_numbers = new int[M * N];
  for(int i = 0; i < M * N; i++) {
    img_numbers[i] = i + 1;
  }

  SequentialPrimitiveImg kernel(m, n);
  kernel.fill_img(kernel_numbers);

  SequentialPrimitiveImg seq_img(M, N);
  seq_img.fill_img(img_numbers);
  seq_img.apply_kernel(kernel.get_img(), m, n);
  //print_matrix(seq_img.get_img(), M, N);

  ParallelPrimitiveImg par_img(M, N, p);
  par_img.fill_img(img_numbers);
  par_img.apply_kernel(kernel.get_img(), m, n);
  //print_matrix(par_img.get_img(), M, N);

  if(!deep_equals(seq_img.get_img(), par_img.get_img(), M, N)) {
    std::cout << "NOPE" << std::endl;
  }

  delete[] kernel_numbers;
  delete[] img_numbers;
}

std::string create_file_if_not_exists(int size, std::filesystem::path path, std::string file_name) {
  auto _path = std::filesystem::path(path);
  _path.append(file_name);
  if(!std::filesystem::exists(_path)) {
    rand_gen_file(_path.string(), size, INT_MAX / 2, INT_MAX);
  }

  return _path.string();
}

int* load_img(int size, std::string path) {
  int* img = new int[size];
  std::ifstream fin(path);
  for(int i = 0; i < size; i++) {
    std::string buff;
    std::getline(fin, buff, ',');
    img[i] = atoi(buff.c_str());
  }

  fin.close();
  return img;
}

int lab2_main(int argc, char* argv[]) {
  //test();
  if(argc < 7) {
    std::cerr
      << "Need 6 arguments: N, M, n, m, p, path." << std::endl
      << "NxM are the dimensions of the image." << std::endl
      << "nxm are the dimensions of the kernel." << std::endl
      << "p is the number of threads." << std::endl
      << "path is the location where the data files are." << std::endl;
    return 1;
  }

  int N = atoi(argv[1]);
  int M = atoi(argv[2]);
  int n = atoi(argv[3]);
  int m = atoi(argv[4]);
  int p = atoi(argv[5]);
  auto dir = std::filesystem::path(argv[6]);

  // handle img
  std::string img_path = create_file_if_not_exists(N * M, dir, "img_N-" + std::string(argv[1]) + "_M-" + std::string(argv[2]) + "_p-" + std::string(argv[5]) + ".csv");
  int* img_numbers = load_img(N * M, img_path);

  SequentialPrimitiveImg seq_img(M, N);
  seq_img.fill_img(img_numbers);

  ParallelPrimitiveImg par_img(M, N, p);
  par_img.fill_img(img_numbers);

  // handle kernel
  std::string kernel_path = create_file_if_not_exists(n * m, dir, "img_n-" + std::string(argv[3]) + "_m-" + std::string(argv[4]) + "_p-" + std::string(argv[5]) + ".csv");
  int* kernel_numbers = load_img(n * m, kernel_path);

  SequentialPrimitiveImg kernel_img(m, n);
  kernel_img.fill_img(kernel_numbers);

  auto t1 = std::chrono::steady_clock().now();
  seq_img.apply_kernel(kernel_img.get_img(), m, n);
  auto t2 = std::chrono::steady_clock().now();
  par_img.apply_kernel(kernel_img.get_img(), m, n);
  auto t3 = std::chrono::steady_clock().now();
  if(!deep_equals(seq_img.get_img(), par_img.get_img(), M, N)) {
    std::cerr << "Results not equal!" << std::endl;
  }

  auto dur1 = t2 - t1;
  auto dur2 = t3 - t2;
  std::cout << "Threads," << p << std::endl;
  std::cout << "Sequential (ms)," << std::chrono::duration<double, std::milli>(dur1).count() << std::endl;
  std::cout << "Parallel (ms)," << std::chrono::duration<double, std::milli>(dur2).count() << std::endl;
  std::cout << "Specification, C++ 11 pointers" << std::endl;
  std::cout << "MatrixColumns," << N << std::endl;
  std::cout << "MatrixRows," << M << std::endl;
  std::cout << "KernelColumns," << n << std::endl;
  std::cout << "KernelRows," << m << std::endl;

  delete[] img_numbers;
  delete[] kernel_numbers;
  _CrtDumpMemoryLeaks();
  return 0;
}
