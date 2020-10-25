#include "vector_sum.h"
#include <chrono>

void sequential_run(std::vector<int>& v1, std::vector<int>& v2, std::vector<int>& v3, const std::function<int(int, int)>& f) {
  for(int i = 0; i < v1.size(); i++) {
    //v3[i] = f(v1[i], v2[i]);
    v3[i] = std::sqrt(v1[i] * v1[i] * v1[i] + v2[i] * v2[i] * v2[i]);
  }
}

void add(std::vector<int>& v1, std::vector<int>& v2, std::vector<int>& v3, int p, int index, const std::function<int(int, int)>& f) {
  for(int i = index; i < v1.size(); i += p) {
    //v3[i] = f(v1[i], v2[i]);
    v3[i] = std::sqrt(v1[i] * v1[i] * v1[i] + v2[i] * v2[i] * v2[i]);
  }
}

void parallel_run(std::vector<int>& v1, std::vector<int>& v2, std::vector<int>& v3, int p, const std::function<int(int, int)>& f) {
  std::vector<std::thread> threads;
  for(int i = 0; i < p; i++) {
    threads.push_back(std::thread(add, std::ref(v1), std::ref(v2), std::ref(v3), p, i, std::ref(f)));
  }

  for(int i = 0; i < p; i++) {
    threads[i].join();
  }
}

void print_vector(std::vector<int>& v) {
  for(int i = 0; i < v.size(); i++) {
    std::cout << v[i] << " ";
  }

  std::cout << "\n";
}

int test() {
  int n = 1 << 20, p = 4;
  std::vector<int> v1(n), v2(n), v3(n), v4(n);
  for(int i = 0; i < n; i++) {
    v1[i] = 2;
    v2[i] = 3;
  }

  auto f = [=](int x, int y) { return (int)std::sqrt(x * x * x + y * y * y); };
  auto t1 = std::chrono::steady_clock().now();
  sequential_run(v1, v2, v3, f);
  auto t2 = std::chrono::steady_clock().now();
  parallel_run(v1, v2, v4, p, f);
  auto t3 = std::chrono::steady_clock().now();
  auto diff1 = t2 - t1;
  auto diff2 = t3 - t2;

  if(v3 == v4) {
    std::cout << "Corect" << "\n";
  }

  std::cout << "sequential   " << std::chrono::duration<double, std::milli>(diff1).count() << " ms" << "\n";
  std::cout << "parallel     " << std::chrono::duration<double, std::milli>(diff2).count() << " ms" << "\n";
  //print_vector(v3);
  return 0;
}

