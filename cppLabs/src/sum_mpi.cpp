#include "sum_mpi.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <filesystem>
#include <fstream>
#include "help.h"

#define MASTER 0

short* load_array(std::string path, int actual_size, int full_size) {
  short* arr = new short[full_size];
  std::ifstream fin(path);
  for(int i = 0; i < full_size; i++) {
    if(i < actual_size) {
      std::string buff;
      std::getline(fin, buff, ',');
      arr[i] = atoi(buff.c_str());
    }
    else {
      arr[i] = 0;
    }
  }

  fin.close();
  return arr;
}

void write_result(std::string path, short* result, int result_size) {
  std::ofstream fout(path, std::ios::out);
  for(int i = 0; i < result_size; i++) {
    fout << result[i];
    if(i < result_size - 1) {
      fout << ",";
    }
  }

  fout << std::endl;
  fout.flush();
  fout.close();
}


short do_sum(short* A, short* B, short* C, int start, int end) {
  short carry = 0;
  for(int i = start; i < end; i++) {
    C[i] = A[i] + B[i] + carry;
    carry = C[i] / 10;
    C[i] %= 10;
  }

  return carry;
}

int lab4_main_sequential(int argc, char* argv[]) {
  int A_LEN = atoi(argv[1]);
  int B_LEN = atoi(argv[2]);
  auto dir = std::filesystem::path(argv[4]);

  std::string A_path = create_file_if_not_exists(dir, "A-" + std::string(argv[1]) + ".csv", A_LEN, 0, 9);
  std::string B_path = create_file_if_not_exists(dir, "B-" + std::string(argv[2]) + ".csv", B_LEN, 0, 9);
  std::string C_path = dir.append("sequential_A-" + std::string(argv[1]) + "_B-" + std::string(argv[2]) + ".csv").string();

  // Prep
  int max_size = std::max(A_LEN, B_LEN);
  short
    * A = load_array(A_path, A_LEN, max_size),
    * B = load_array(B_path, B_LEN, max_size),
    * C = new short[max_size + 1];

  // Work
  auto t1 = std::chrono::steady_clock().now();
  short carry = do_sum(A, B, C, 0, max_size);
  C[max_size] = carry;
  auto t2 = std::chrono::steady_clock().now();
  write_result(C_path, C, max_size + 1);
  std::cout << "Implementation,Sequential" << std::endl;
  std::cout << "P,-" << std::endl;
  std::cout << "Time," << std::chrono::duration<double, std::milli>(t2 - t1).count() << std::endl;

  // Finish
  delete[] A;
  delete[] B;
  delete[] C;
  return 0;
}


/*
  9 9 9 | 9 9 9 | 9 9 9
  1 0 0 | 0 0 0 | 0 0 0
  ----------------------
  0 0 0, c = 1
  9 9 9, c = 0
  9 9 9, c = 0
*/

int lab4_main_mpi_scatter_gather(int argc, char* argv[]) {
  int A_LEN = atoi(argv[1]);
  int B_LEN = atoi(argv[2]);

  // MPI Prep
  double t1, t2;
  int world_size, world_rank, batch_size;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Prep
  int max_size = ((std::max(A_LEN, B_LEN) + world_size - 1) / world_size) * world_size;
  short
    * A = new short[max_size],
    * B = new short[max_size],
    * C = new short[max_size + 1];

  batch_size = max_size / world_size;

  // Work
  if(world_rank == MASTER) {
    std::string A_path = std::filesystem::path(argv[4]).append("A-" + std::string(argv[1]) + ".csv").string();
    A = load_array(A_path, A_LEN, max_size);
    std::string B_path = std::filesystem::path(argv[4]).append("B-" + std::string(argv[2]) + ".csv").string();
    B = load_array(B_path, B_LEN, max_size);
    t1 = MPI_Wtime();
  }

  short* A_p = new short[batch_size], * B_p = new short[batch_size], * C_p = new short[batch_size];
  MPI_Scatter(A, batch_size, MPI_SHORT, A_p, batch_size, MPI_SHORT, MASTER, MPI_COMM_WORLD);
  MPI_Scatter(B, batch_size, MPI_SHORT, B_p, batch_size, MPI_SHORT, MASTER, MPI_COMM_WORLD);
  if(world_rank != MASTER) {
    short recv_carry = 0;
    MPI_Recv(&recv_carry, 1, MPI_SHORT, world_rank - 1, world_rank, MPI_COMM_WORLD, &status);
    A_p[0] += recv_carry;
  }

  short carry = do_sum(A_p, B_p, C_p, 0, batch_size);
  if(world_rank == world_size - 1) {
    MPI_Send(&carry, 1, MPI_SHORT, MASTER, MASTER, MPI_COMM_WORLD);
  }
  else {
    MPI_Send(&carry, 1, MPI_SHORT, world_rank + 1, world_rank + 1, MPI_COMM_WORLD);
  }

  MPI_Gather(C_p, batch_size, MPI_SHORT, C, batch_size, MPI_SHORT, MASTER, MPI_COMM_WORLD);
  if(world_rank == MASTER) {
    short recv_carry = 0;
    MPI_Recv(&recv_carry, 1, MPI_SHORT, world_size - 1, world_rank, MPI_COMM_WORLD, &status);
    C[max_size] = recv_carry;
    t2 = MPI_Wtime();
    std::string C_path = std::filesystem::path(argv[4]).append("scatterGather_A-" + std::string(argv[1]) + "_B-" + std::string(argv[2]) + ".csv").string();
    write_result(C_path, C, max_size + 1);
    std::cout << "Implementation,Scatter&Gather" << std::endl;
    std::cout << "P," << world_size << std::endl;
    std::cout << "Time," << (t2 - t1) * 1000 << std::endl;
  }

  // Finish
  delete[] A;
  delete[] B;
  delete[] C;
  delete[] A_p;
  delete[] B_p;
  delete[] C_p;
  return MPI_Finalize();
}

int lab4_main_mpi_irecv(int argc, char* argv[]) {
  int A_LEN = atoi(argv[1]);
  int B_LEN = atoi(argv[2]);

  // MPI Prep
  double t1, t2;
  short carry = 0;
  int world_size, world_rank, batch_size;
  MPI_Status status;
  MPI_Request A_req, B_req, carry_req;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Prep
  int max_size = ((std::max(A_LEN, B_LEN) + world_size - 2) / (world_size - 1)) * (world_size - 1);
  short
    * A = new short[max_size],
    * B = new short[max_size],
    * C = new short[max_size + 1];

  batch_size = max_size / (world_size - 1); // MASTER does no work

  // Work
  if(world_rank == MASTER) {
    std::string A_path = std::filesystem::path(argv[4]).append("A-" + std::string(argv[1]) + ".csv").string();
    A = load_array(A_path, A_LEN, max_size);
    std::string B_path = std::filesystem::path(argv[4]).append("B-" + std::string(argv[2]) + ".csv").string();
    B = load_array(B_path, B_LEN, max_size);
    t1 = MPI_Wtime();
    MPI_Request* C_req = new MPI_Request[world_size];
    for(int i = 1, step = 0; i < world_size; i++, step++) {
      MPI_Irecv(C + batch_size * step, batch_size, MPI_SHORT, i, i, MPI_COMM_WORLD, &C_req[i]); // C_tag <- rank
      MPI_Send(A + batch_size * step, batch_size, MPI_SHORT, i, i + 1, MPI_COMM_WORLD); // A_tag <- rank + 1
      MPI_Send(B + batch_size * step, batch_size, MPI_SHORT, i, i + 2, MPI_COMM_WORLD); // B_tag <- rank + 2
    }

    MPI_Irecv(&carry, 1, MPI_SHORT, world_size - 1, world_size - 1, MPI_COMM_WORLD, &carry_req);
    for(int i = 1; i < world_size; i++) {
      MPI_Wait(&C_req[i], &status);
    }

    MPI_Wait(&carry_req, &status);

    C[max_size] = carry;
    t2 = MPI_Wtime();
    std::string C_path = std::filesystem::path(argv[4]).append("irecv_A-" + std::string(argv[1]) + "_B-" + std::string(argv[2]) + ".csv").string();
    write_result(C_path, C, max_size + 1);
    std::cout << "Implementation,IRecv" << std::endl;
    std::cout << "P," << world_size << std::endl;
    std::cout << "Time," << (t2 - t1) * 1000 << std::endl;
  }
  else {
    // Init receive requests
    MPI_Irecv(A, batch_size, MPI_SHORT, MASTER, world_rank + 1, MPI_COMM_WORLD, &A_req);
    MPI_Irecv(B, batch_size, MPI_SHORT, MASTER, world_rank + 2, MPI_COMM_WORLD, &B_req);
    if(world_rank > 1) { // Receive carry
      MPI_Irecv(&carry, 1, MPI_SHORT, world_rank - 1, world_rank, MPI_COMM_WORLD, &carry_req); // Carry_tag <- rank
    }

    // Wait for requests
    MPI_Wait(&A_req, &status);
    MPI_Wait(&B_req, &status);
    if(world_rank > 1) {
      MPI_Wait(&carry_req, &status);
    }

    A[0] += carry;
    carry = do_sum(A, B, C, 0, batch_size);    //C[max_size] = carry;
    MPI_Send(C, batch_size, MPI_SHORT, MASTER, world_rank, MPI_COMM_WORLD); // Send result to master
    if(world_rank == world_size - 1) { // Send last carry to MASTER
      MPI_Send(&carry, 1, MPI_SHORT, MASTER, world_rank, MPI_COMM_WORLD);
    }
    else { // Send carry to the next PROCESS
      MPI_Send(&carry, 1, MPI_SHORT, world_rank + 1, world_rank + 1, MPI_COMM_WORLD);
    }
  }

  // Finish
  delete[] A;
  delete[] B;
  delete[] C;
  return MPI_Finalize();
}

int lab4_main(int argc, char* argv[]) {
  if(argc < 5) {
    std::cerr
      << "Need 3 arguments: A_LEN, B_LEN, which, path." << std::endl
      << "A_LEN - size of first number." << std::endl
      << "B_LEN - size of the second number." << std::endl
      << "which - 1 - seq, 2 - scatter-gather, 3 - irecv" << std::endl
      << "path - location where the data files will be created.";
    return 1;
  }

  switch(atoi(argv[3])) {
  case 1:
    return lab4_main_sequential(argc, argv);
  case 2:
    return lab4_main_mpi_scatter_gather(argc, argv);
  case 3:
    return lab4_main_mpi_irecv(argc, argv);
  default:
    break;
  }

  return 0;
}
