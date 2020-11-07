#include "mpi.h"
#include "vector_sum_mpi.h"
#include <iostream>

#define MASTER 0
#define MAX 10

int sem3_main(int argc, char* argv[]) {
  int world_size, world_rank, batch_size;
  MPI_Status status;

  double a[MAX], b[MAX], c[MAX];

  // Initialize MPI env
  MPI_Init(&argc, &argv);

  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if(world_rank == MASTER) {
    for(int i = 0; i < MAX; i++) {
      a[i] = 2;
      b[i] = 3;
    }

    auto t1 = MPI_Wtime();
    for(int i = 0; i < MAX; i++) {
      c[i] = a[i] + b[i];
    }

    auto t2 = MPI_Wtime();
    std::cout << t2 - t1 << std::endl;

    batch_size = MAX / world_size;
    for(int i = 1; i < world_size; i++) {
      int diff = 0;
      if(i == world_size - 1) {
        diff = MAX % world_size;
      }

      MPI_Send(a + batch_size * i, batch_size + diff, MPI_DOUBLE, i, 10, MPI_COMM_WORLD);
      MPI_Send(b + batch_size * i, batch_size + diff, MPI_DOUBLE, i, 11, MPI_COMM_WORLD);
    }

    for(int i = 0; i < batch_size; i++) {
      c[i] = a[i] + b[i];
    }

    for(int i = 1; i < world_size; i++) {
      int diff = 0;
      if(i == world_size - 1) {
        diff = MAX % world_size;
      }

      MPI_Recv(c + batch_size * i, batch_size + diff, MPI_DOUBLE, i, 12, MPI_COMM_WORLD, &status);
    }

    auto t3 = MPI_Wtime();
    std::cout << t3 - t2 << std::endl;
    for(int i = 0; i < MAX; i++) {
      std::cout << c[i] << " ";
    }
  }
  else {
    MPI_Recv(a, MAX, MPI_DOUBLE, MASTER, 10, MPI_COMM_WORLD, &status);
    MPI_Recv(b, MAX, MPI_DOUBLE, MASTER, 11, MPI_COMM_WORLD, &status);
    int count;
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    for(int i = 0; i < count; i++) {
      c[i] = a[i] + b[i];
    }

    MPI_Send(c, count, MPI_DOUBLE, MASTER, 12, MPI_COMM_WORLD);
  }

  // Finalize the MPI env
  return MPI_Finalize();
}

int sem3_main_2(int argc, char* argv[]) {

  int world_size, world_rank, batch_size;
  MPI_Status status;

  double a[MAX], b[MAX], c[MAX];

  // Initialize MPI env
  MPI_Init(&argc, &argv);

  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  batch_size = MAX / world_size;
  if(world_rank == world_size - 1) {
    batch_size += MAX % world_size;
  }

  if(world_rank == MASTER) {
    for(int i = 0; i < batch_size; i++) {
      a[i] = 2;
      b[i] = 3;
    }
  }

  double* a_recv = new double[batch_size];
  MPI_Scatter(a, batch_size, MPI_DOUBLE, a_recv, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
  double* b_recv = new double[batch_size];
  MPI_Scatter(b, batch_size, MPI_DOUBLE, b_recv, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
  double* c_recv = new double[batch_size];
  for(int i = 0; i < batch_size; i++) {
    c_recv[i] = a_recv[i] + b_recv[i];
  }

  MPI_Gather(c_recv, batch_size, MPI_DOUBLE, c, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
  //if(world_rank == MASTER) {
  //  for(int i = 0; i < MAX; i++) {
  //    std::cout << c[i] << " ";
  //  }
  //}

  std::cout << "final";

  delete[] a;
  delete[] b;
  // Finalize the MPI env
  return MPI_Finalize();
}
