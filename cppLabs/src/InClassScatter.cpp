#include <iostream>
#include <mpi.h>
#include <stdlib.h>
using namespace std;

#define MASTER 0
#define MAX 24
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

	int n = MAX - world_size;

	if (world_rank == MASTER) {
		// ID 0 - Vector initialization
		for (int i = 0; i < n; ++i) {
			a[i] = 22.123;
			b[i] = 35.987;
		}

		int rest = n % world_size;
		if (rest) {
			for (int i = 0; i < world_size - rest; ++i) {
				a[n + i] = 0;
				b[n + i] = 0;
			}
			n += (world_size - rest);
		}
	}

	double starttime, endtime;
	starttime = MPI_Wtime();
	
	batch_size = MAX / world_size;
	double* a2 = new double[batch_size];
	double* b2 = new double[batch_size];
	double* c2 = new double[batch_size];
	MPI_Scatter(a, batch_size, MPI_DOUBLE, a2, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	MPI_Scatter(b, batch_size, MPI_DOUBLE, b2, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);		
		
	for (int i = 0; i < batch_size; ++i) {
		c2[i] = a2[i] + b2[i];
	}

	MPI_Gather(c2, batch_size, MPI_DOUBLE, c, batch_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	
	endtime = MPI_Wtime();

	if (world_rank == MASTER) {
		cout << "Paralel: " << (double)endtime - starttime << "\n";
		// Show result
		for (int i = 0; i < MAX - world_size; ++i) {
			cout << c[i] << " ";
		}
		cout << "\n";
	}

	// Finalize the MPI Environment.
	MPI_Finalize();
}