#include <iostream>
#include <mpi.h>
#include <stdlib.h>
using namespace std;

#define MASTER 0
#define MAX 100000
double a[MAX], b[MAX], c[MAX];

int main(int argc, char** argv)
{
	int world_size; // processes number
	int world_rank; // rank of a process
	int batch_size, rest;
	MPI_Status status;

	// Initialize the MPI Environment
	MPI_Init(&argc, &argv);

	// Get the number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the rank of the process
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (world_rank == MASTER) {
		// ID 0 - Vector initialization
		for (int i = 0; i < MAX; ++i) {
			a[i] = 22.123;
			b[i] = 35.987;
		}
		double startseq, endseq;
		startseq = MPI_Wtime();
		for (int i = 0; i < MAX; ++i) {
			c[i] = sqrt(a[i] * a[i] * b[i] * b[i]);
		}
		endseq = MPI_Wtime();
		cout << "Secvential: " << (double)endseq - startseq << "\n";

		double starttime, endtime;
		starttime = MPI_Wtime();

		batch_size = MAX / world_size;
		rest = MAX % world_size;
		int start = batch_size;

		for (int i = 1; i < world_size; ++i) {
			MPI_Send(a + start, batch_size + (rest >= i), MPI_DOUBLE, i, 10, MPI_COMM_WORLD);
			MPI_Send(b + start, batch_size + (rest >= i), MPI_DOUBLE, i, 11, MPI_COMM_WORLD);
			start += batch_size + (rest >= i);
		}

		for (int i = 0; i < batch_size; ++i) {
			c[i] = sqrt(a[i] * a[i] * b[i] * b[i]);
		}

		start = batch_size;
		for (int i = 1; i < world_size; ++i) {
			MPI_Recv(c + start, batch_size + (rest >= i), MPI_DOUBLE, i, 12, MPI_COMM_WORLD, &status);
			start += batch_size + (rest >= i);
		}

		endtime = MPI_Wtime();
		cout << "Paralel: " << (double)endtime - starttime << "\n";
		// Show result
		for (int i = 0; i < MAX; ++i) {
			cout << c[i] << " ";
		}
		cout << "\n";
	}
	else {
		// Addition
		// Process with ID other than 0
		MPI_Recv(a, MAX, MPI_DOUBLE, MASTER, 10, MPI_COMM_WORLD, &status);
		MPI_Recv(b, MAX, MPI_DOUBLE, MASTER, 11, MPI_COMM_WORLD, &status);
		int count;
		MPI_Get_count(&status, MPI_DOUBLE, &count);
		for (int i = 0; i < count; ++i) {
			c[i] = sqrt(a[i] * a[i] * b[i] * b[i]);
		}
		MPI_Send(c, count, MPI_DOUBLE, MASTER, 12, MPI_COMM_WORLD);
	}

	// Finalize the MPI Environment.
	MPI_Finalize();
}