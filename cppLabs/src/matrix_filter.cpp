#include "matrix_filter.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <chrono>
#include <thread>
#include "help.h"
#include <algorithm>
#define NO_RUNS 5

std::vector<std::vector<long long>> load_img(std::string file, int rows, int cols) {
  std::ifstream fin(file);
  std::vector<std::vector<long long>> img;
  for(int row = 0; row < rows; row++) {
    img.push_back(std::vector<long long>());
    for(int col = 0; col < cols; col++) {
      std::string s;
      std::getline(fin, s, ',');
      img[row].push_back(atoi(s.c_str()));
    }
  }

  fin.close();
  return img;
}

void padding(std::vector<std::vector<long long>>& img, int M, int N, int padding_row, int padding_col) {
  // Padding UP & DOWN
  // 1 2 3        1 1 2 3 3
  // 4 5 6   ->   4 4 5 6 6
  // 7 8 9        7 7 8 9 9
  for(int i = 0; i < padding_row; i++) {
    std::vector<long long> padding_up(N), padding_down(N);
    for(int j = 0; j < N; j++) {
      padding_up[j] = img[i * 2][j];
      padding_down[j] = img[img.size() - i * 2 - 1][j];
    }

    img.insert(img.begin(), padding_up);
    img.insert(img.end(), padding_down);
  }

  // Padding LEFT & RIGHT
  //                  0 1 2 3 0
  // 1 1 2 3 3        1 1 2 3 3
  // 4 4 5 6 6   ->   1 1 2 3 3
  // 7 7 8 9 9        7 7 8 9 9
  //                  0 7 8 9 0
  for(int i = 0; i < padding_col; i++) {
    std::vector<long long> padding_left(M), padding_right(M);
    for(int j = padding_row; j < M + padding_row; j++) {
      padding_left[j - padding_row] = img[j][i * 2];
      padding_right[j - padding_row] = img[j][img[j].size() - i * 2 - 1];
    }

    for(int j = 0; j < M + padding_row * 2; j++) {
      if(j < padding_row || M + padding_row <= j) {
        img[j].insert(img[j].begin(), 0);
        img[j].insert(img[j].end(), 0);
      }
      else {
        img[j].insert(img[j].begin(), padding_left[j - padding_row]);
        img[j].insert(img[j].end(), padding_right[j - padding_row]);
      }
    }
  }

  // Corner UP LEFT
  /*
  *
  * 4|3|
  * 2|1|
  *    |1|2|
  *    |3|4|
  */
  for(int row = 0; row < padding_row; row++) {
    for(int col = 0; col < padding_col; col++) {
      img[row][col] = img[padding_row * 2 - row - 1][padding_col * 2 - col - 1];
    }
  }

  // Corner UP RIGHT
  /*
  *
  *        |4|3|
  *        |2|1|
  *    |1|2|
  *    |3|4|
  */
  for(int row = 0; row < padding_row; row++) {
    for(int col = N + 2 * padding_col - 1, i = 0; N + padding_col - 1 < col; col--, i += 2) {
      img[row][col] = img[padding_row * 2 - row - 1][col - padding_col - 1 + i];
    }
  }

  // Corner DONW LEFT
  // etc...
  for(int row = M + 2 * padding_row - 1, i = 0; M + padding_row - 1 < row; row--, i += 2) {
    for(int col = 0; col < padding_col; col++) {
      img[row][col] = img[row - padding_row - 1 + i][padding_col * 2 - col - 1];
    }
  }

  // Corner DOWN RIGHT
  // etc...
  for(int row = M + 2 * padding_row - 1, i = 0; M + padding_row - 1 < row; row--, i += 2) {
    for(int col = N + 2 * padding_col - 1, j = 0; N + padding_col - 1 < col; col--, j += 2) {
      img[row][col] = img[row - padding_row - 1 + i][col - padding_col - 1 + j];
    }
  }
}

long long multiply(std::vector<std::vector<long long>>& img, std::vector<std::vector<long long>>& filter, int starting_row, int starting_col) {
  std::vector<std::vector<long long>> result;
  for(int row = starting_row; row < starting_row + filter.size(); row++) {
    result.push_back(std::vector<long long>());
    for(int col = starting_col; col < starting_col + filter[0].size(); col++) {
      result[row - starting_row].push_back(0.0);
      for(int k = 0; k < filter.size(); k++) {
        result[row - starting_row][col - starting_col] += img[row][k] * filter[k][col - starting_col];
      }
    }
  }

  return result[filter.size() / 2][filter[0].size() / 2];
}

std::vector<std::vector<long long>> apply_filter_seq(std::vector<std::vector<long long>>& img, std::vector<std::vector<long long>>& filter, int M, int N, int padding_row, int padding_col) {
  std::vector<std::vector<long long>> result;
  for(int row = padding_row; row < M + padding_row; row++) {
    result.push_back(std::vector<long long>());
    for(int col = padding_col; col < N + padding_col; col++) {
      result[row - padding_row].push_back(multiply(img, filter, row - padding_row, col - padding_col));
    }
  }

  return result;
}

void row_split(
  std::vector<std::vector<long long>>& result,
  std::vector<std::vector<long long>>& img,
  std::vector<std::vector<long long>>& filter,
  int M,
  int N,
  int padding_row,
  int padding_col,
  int batch_start,
  int batch_finish) {
  for(int row = padding_row + batch_start; row < M + padding_row && row <= padding_row + batch_finish; row++) {
    for(int col = padding_col; col < N + padding_col; col++) {
      result[row - padding_row][col - padding_col] = multiply(img, filter, row - padding_row, col - padding_col);
    }
  }
}

void col_split(
  std::vector<std::vector<long long>>& result,
  std::vector<std::vector<long long>>& img,
  std::vector<std::vector<long long>>& filter,
  int M,
  int N,
  int padding_row,
  int padding_col,
  int batch_start,
  int batch_finish) {
  for(int row = padding_row; row < M + padding_row; row++) {
    for(int col = padding_col + batch_start; col < N + padding_col && col <= padding_col + batch_finish; col++) {
      result[row - padding_row][col - padding_col] = multiply(img, filter, row - padding_row, col - padding_col);
    }
  }
}

std::vector<std::vector<long long>> apply_filter_par(std::vector<std::vector<long long>>& img, std::vector<std::vector<long long>>& filter, int M, int N, int padding_row, int padding_col, int p) {
  std::vector<std::vector<long long>> result(M, std::vector<long long>(N, 0));
  std::vector<std::thread> threads;

  // Row separation
  if(M >= N) {
    int batch_size_per_thread = M / p + ((M % p == 0) ? 0 : 1);
    for(int thread = 0; thread < p; thread++) {
      threads.push_back(std::thread(
        row_split,
        std::ref(result),
        std::ref(img),
        std::ref(filter),
        M,
        N,
        padding_row,
        padding_col,
        batch_size_per_thread * thread,
        batch_size_per_thread * (thread + 1)
      ));
    }
  }

  // Column Separation
  else {
    int batch_size_per_thread = N / p + ((N % p == 0) ? 0 : 1);
    for(int thread = 0; thread < p; thread++) {
      threads.push_back(std::thread(
        col_split,
        std::ref(result),
        std::ref(img),
        std::ref(filter),
        M,
        N,
        padding_row,
        padding_col,
        batch_size_per_thread * thread,
        batch_size_per_thread * (thread + 1)
      ));
    }
  }

  for(int thread = 0; thread < p; thread++) {
    threads[thread].join();
  }

  return result;
}


int lab1_main(int argc, char* argv[]) {
  if(argc < 7) {
    std::cerr << "Need 6 arguments: N, M, n, m, p, Path." << std::endl;
    return 1;
  }

  // args
  int N = atoi(argv[1]), M = atoi(argv[2]), n = atoi(argv[3]), m = atoi(argv[4]), p = atoi(argv[5]);
  auto dir = std::filesystem::path(argv[6]);

  // handle img
  std::string matrix_file_name = "matrix_N-" + std::string(argv[1]) + "_M-" + std::string(argv[2]) + "_n-" + std::string(argv[3]) + "_m-" + std::string(argv[4]) + "_p-" + std::string(argv[5]) + ".csv";
  std::string matrix_file = std::filesystem::path(dir).append(matrix_file_name).string();
  if(!std::filesystem::exists(matrix_file)) {
    rand_gen_file(matrix_file, N * M, (long long)(INT_MAX / 2), (long long)INT_MAX);
  }

  auto img = load_img(matrix_file, M, N);

  // handle filter
  std::string filter_file_name = "filter_N-" + std::string(argv[1]) + "_M-" + std::string(argv[2]) + "_n-" + std::string(argv[3]) + "_m-" + std::string(argv[4]) + "_p-" + std::string(argv[5]) + ".csv";
  std::string filter_file = std::filesystem::path(dir).append(filter_file_name).string();
  if(!std::filesystem::exists(filter_file)) {
    rand_gen_file(filter_file, n * m, (long long)(INT_MAX / 4), (long long)(INT_MAX / 2));
  }

  auto filter = load_img(filter_file, m, n);

  // add padding on img
  int padding_row = m / 2, padding_col = n / 2;
  padding(img, M, N, padding_row, padding_col);

  auto t1 = std::chrono::steady_clock().now();

  // secv
  auto seq = apply_filter_seq(img, filter, M, N, padding_row, padding_col);
  auto t2 = std::chrono::steady_clock().now();

  // threads
  auto par = apply_filter_par(img, filter, M, N, padding_row, padding_col, p);
  auto t3 = std::chrono::steady_clock().now();

  if(seq != par) {
    std::cerr << "Different result" << std::endl;
    return 1;
  }

  auto diff1 = t2 - t1;
  auto diff2 = t3 - t2;
  std::cout << "Threads," << p << std::endl;
  std::cout << "Sequential (ms)," << std::chrono::duration<double, std::milli>(diff1).count() << std::endl;
  std::cout << "Parallel (ms)," << std::chrono::duration<double, std::milli>(diff2).count() << std::endl;
  std::cout << "Specification,C++ 17 std::vector" << std::endl;

  return 0;
}
