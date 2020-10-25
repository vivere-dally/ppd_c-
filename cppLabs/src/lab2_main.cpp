#define _CRTDBG_MAP_ALLOC
#include "lab2_main.h"
#include "SequentialPrimitiveImg.h"
#include "ParallelPrimitiveImg.h"
#include <crtdbg.h>
#include <iostream>

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

int lab2_main(int argc, char* argv[]) {
  test();
  _CrtDumpMemoryLeaks();
  return 0;
}
