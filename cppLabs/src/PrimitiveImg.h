#pragma once
#include "Img.h"
class PrimitiveImg : public Img<long long**> {
protected:
  const int rows, columns;
  long long** img;
public:
  PrimitiveImg(int rows, int columns) : rows{ rows }, columns{ columns } {
    this->img = new long long* [rows];
    for(size_t i = 0; i < rows; i++) {
      this->img[i] = new long long[columns];
    }
  }

  void fill_img(int* numbers) override {
    for(size_t row = 0; row < this->rows; row++) {
      for(size_t column = 0; column < this->columns; column++) {
        this->img[row][column] = numbers[row * this->columns + column];
      }
    }
  }

  long long** get_img() {
    return this->img;
  }

  ~PrimitiveImg() {
    for(size_t i = 0; i < this->rows; i++) {
      delete[] this->img[i];
    }

    delete[] this->img;
  }
};

