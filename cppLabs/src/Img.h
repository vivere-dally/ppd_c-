#pragma once
#include <math.h>


template <typename T>
class Img {
protected:
  int get_pure_index(int index, int limit) {
    int pure_index = abs(index);
    if(pure_index >= limit) {
      int offset = (1 + pure_index - limit) * 2;
      pure_index -= offset;
    }

    return pure_index;
  }

public:
  virtual void fill_img(int* numbers) = 0;
  virtual void apply_kernel(T kernel, int kernel_rows, int kernel_columns) = 0;

};

