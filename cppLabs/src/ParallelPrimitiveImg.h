#pragma once
#include <deque>
#include <thread>
#include <functional>
#include "PrimitiveImg.h"

class ParallelPrimitiveImg : public PrimitiveImg {
private:
  const int p;

#pragma region RowWise
#pragma region Overlaid Rows
  long long* copy_row(int row_index) {
    long long* new_row = new long long[this->columns];
    for(int column = 0; column < this->columns; column++) {
      new_row[column] = this->img[row_index][column];
    }

    return new_row;
  }

  long long*** get_top_overlaid_rows(int kernel_rows, int batch_size) {
    long long*** overlaid_rows = new long long** [this->p]();
    for(int thread_index = 0; thread_index < this->p; thread_index++) {
      overlaid_rows[thread_index] = new long long* [kernel_rows / 2]();
      for(int row = thread_index * batch_size - 1, steps = kernel_rows / 2 - 1; row >= 0 && steps >= 0; row--, steps--) {
        overlaid_rows[thread_index][steps] = copy_row(row);
      }
    }

    return overlaid_rows;
  }

  long long*** get_bottom_overlaid_rows(int kernel_rows, int batch_size) {
    long long*** overlaid_rows = new long long** [this->p]();
    for(int thread_index = 0; thread_index < this->p; thread_index++) {
      overlaid_rows[thread_index] = new long long* [kernel_rows / 2]();
      for(int kernel_row = 0; kernel_row < kernel_rows / 2; kernel_row++) {
        int img_index = (thread_index + 1) * batch_size + kernel_row;
        if(0 <= img_index && img_index < this->rows) {
          overlaid_rows[thread_index][kernel_row] = copy_row(img_index);
        }
      }
    }

    return overlaid_rows;
  }
#pragma endregion
  long long row_wise_multiplication(long long** kernel, int kernel_rows, int kernel_columns, long long** overlaid_top_rows, long long** overlaid_bottom_rows, int row_index, int column_index, int batch_start_index, int batch_finish_index) {
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
        long long value = 0;
        if(img_row < batch_start_index) {
          value = overlaid_top_rows[kernel_rows / 2 + img_row - batch_start_index][img_column];
        }
        else if(batch_start_index <= img_row && img_row < batch_finish_index) {
          value = this->img[img_row][img_column];
        }
        else {
          value = overlaid_bottom_rows[img_row - batch_finish_index][img_column];
        }

        sum += value * kernel[kernel_row][kernel_column];
      }
    }

    return sum;
  }

  void apply_row_wise(long long** kernel, int kernel_rows, int kernel_columns, long long** overlaid_top_rows, long long** overlaid_bottom_rows, int batch_start_index, int batch_finish_index) {
    std::deque<long long*> holder;
    for(int row = batch_start_index; row < batch_finish_index; row++) {
      long long* top = new long long[this->columns];
      for(int column = 0; column < this->columns; column++) {
        top[column] = this->row_wise_multiplication(kernel, kernel_rows, kernel_columns, overlaid_top_rows, overlaid_bottom_rows, row, column, batch_start_index, batch_finish_index);
      }

      if(holder.size() == kernel_rows) {
        long long* old = this->img[row - kernel_rows];
        this->img[row - kernel_rows] = holder.front();
        holder.pop_front();
        delete[] old;
      }

      holder.push_back(top);
    }

    int rows_left_index = batch_finish_index - holder.size();
    while(!holder.empty()) {
      long long* old = this->img[rows_left_index];
      this->img[rows_left_index] = holder.front();
      holder.pop_front();
      delete[] old;
      rows_left_index++;
    }
  }

#pragma endregion

#pragma region ColumnWise
#pragma region Overlaid Columns
  long long* copy_column(int column_index) {
    long long* new_column = new long long[this->rows];
    for(int row = 0; row < this->rows; row++) {
      new_column[row] = this->img[row][column_index];
    }

    return new_column;
  }

  long long*** get_left_overlaid_columns(int kernel_columns, int batch_size) {
    long long*** overlaid_columns = new long long** [this->p]();
    for(int thread_index = 0; thread_index < this->p; thread_index++) {
      overlaid_columns[thread_index] = new long long* [kernel_columns / 2]();
      for(int column = thread_index * batch_size - 1, steps = kernel_columns / 2 - 1; column >= 0 && steps >= 0; column--, steps--) {
        overlaid_columns[thread_index][steps] = copy_column(column);
      }
    }
  }

  long long*** get_right_overlaid_columns(int kernel_columns, int batch_size) {
    long long*** overlaid_columns = new long long** [this->p]();
    for(int thread_index = 0; thread_index < this->p; thread_index++) {
      overlaid_columns[thread_index] = new long long* [kernel_columns / 2]();
      for(int kernel_column = 0; kernel_column < kernel_columns / 2; kernel_column++) {
        int img_index = (thread_index + 1) * batch_size + kernel_column;
        if(0 <= img_index && img_index < this->columns) {
          overlaid_columns[thread_index][img_index] = copy_column(img_index);
        }
      }
    }
  }
#pragma endregion

  void apply_column_wise(long long** kernel, int kernel_rows, int kernel_columns, long long** overlaid_top_rows, long long** overlaid_bottom_rows, int batch_start_index, int batch_finish_index) {

  }

#pragma endregion


public:
  ParallelPrimitiveImg(int rows, int columns, int p) : PrimitiveImg(rows, columns), p{ p } {}

  void apply_kernel(long long** kernel, int kernel_rows, int kernel_columns) override {
    int batch_size = (this->rows >= this->columns) ? (this->rows / this->p) : (this->columns / this->p);
    std::thread* threads = new std::thread[this->p];
    if(this->rows >= this->columns) {
      // Row Wise
      long long*** overlaid_top_rows = this->get_top_overlaid_rows(kernel_rows, batch_size);
      long long*** overlaid_bottom_rows = this->get_bottom_overlaid_rows(kernel_rows, batch_size);
      for(int thread_index = 0; thread_index < this->p; thread_index++) {
        int batch_start = batch_size * thread_index, batch_finish = batch_size * (thread_index + 1);
        if(thread_index == this->p - 1) {
          batch_finish = this->rows;
        }

        threads[thread_index] = std::thread(
          [this, kernel, kernel_rows, kernel_columns, overlaid_top_rows, overlaid_bottom_rows, batch_start, batch_finish, thread_index] {
          this->apply_row_wise(
            kernel,
            kernel_rows,
            kernel_columns,
            overlaid_top_rows[thread_index],
            overlaid_bottom_rows[thread_index],
            batch_start,
            batch_finish
          );
        });
      }

      for(int i = 0; i < this->p; i++) {
        threads[i].join();
      }

      for(int i = 0; i < this->p; i++) {
        for(int j = 0; j < kernel_rows / 2; j++) {
          if(overlaid_top_rows[i][j] != nullptr) {
            delete[] overlaid_top_rows[i][j];
          }

          if(overlaid_bottom_rows[i][j] != nullptr) {
            delete[] overlaid_bottom_rows[i][j];
          }
        }

        delete[] overlaid_top_rows[i];
        delete[] overlaid_bottom_rows[i];
      }

      delete[] overlaid_top_rows;
      delete[] overlaid_bottom_rows;
    }
    else {
      // Column Wise
    }

    delete[] threads;
  }
};

