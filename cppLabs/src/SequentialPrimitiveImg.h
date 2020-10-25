#pragma once
#include "PrimitiveImg.h"
#include <deque>
class SequentialPrimitiveImg : public PrimitiveImg {
private:
  long long multiply(long long** kernel, int kernel_rows, int kernel_columns, int row_index, int column_index) {
    long long sum = 0;
    for(
      int row = row_index - kernel_rows / 2, kernel_row = 0;
      row < this->rows + kernel_rows / 2 && kernel_row < kernel_rows;
      row++, kernel_row++
      ) {
      int img_row = this->get_pure_index(row, this->rows);
      for(
        int column = column_index - kernel_columns / 2, kernel_column = 0;
        column < this->columns + kernel_columns / 2 && kernel_column < kernel_columns;
        column++, kernel_column++
        ) {
        int img_column = this->get_pure_index(column, this->columns);
        sum += this->img[img_row][img_column] * kernel[kernel_row][kernel_column];
      }
    }

    return sum;
  }

public:
  SequentialPrimitiveImg(int rows, int columns) : PrimitiveImg(rows, columns) {}

  void apply_kernel(long long** kernel, int kernel_rows, int kernel_columns) override {
    std::deque<long long*> holder;
    for(int row = 0; row < this->rows; row++) {
      long long* top = new long long[this->columns];
      for(int column = 0; column < this->columns; column++) {
        top[column] = this->multiply(kernel, kernel_rows, kernel_columns, row, column);
      }

      if(holder.size() == kernel_rows) {
        long long* old = this->img[row - kernel_rows];
        this->img[row - kernel_rows] = holder.front();
        holder.pop_front();
        delete[] old;
      }

      holder.push_back(top);
    }

    int rowsLeftIndex = kernel_rows;
    while(!holder.empty()) {
      long long* old = this->img[this->rows - rowsLeftIndex];
      this->img[this->rows - rowsLeftIndex] = holder.front();
      holder.pop_front();
      delete[] old;
      rowsLeftIndex--;
    }
  }
};

