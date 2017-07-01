#include <stdio.h>
#include <stdlib.h>

typedef enum { false, true } bool;

// Print releated functions
void printStep(FILE *f, int step);
void printTotalResources(FILE *f);
void printAvailableResources(FILE *f);
void printProcessState(FILE *f, int process);
void printProcessOrder(FILE *f);
void printAllocateAndRelease(FILE *f, int process);
void printAvailableProcesses(FILE* f, int num, int* processes);
void printDeadlock(FILE* f);

// Algorithm related functions
int getAvailableProcesses(int* out);
void allocateAndRelease(int process);
bool isAvailable(int process);

// Variables
int processSize, resourceSize;
int *totalResources, *availResources, *availProcesses, **allocMat, **maxMat;
bool *processed;
int *order;
int i, j;
int p, r;
int orderIndex = 0;

int main(int argc, char **argv)
{
	if (argc == 1){
		fprintf(stderr, "You should provide config filename as program parameter\n");
		fprintf(stderr, "ex) ./banker config.txt\n");
		return 1;
	}
	/************************************************************************/
	/* FILE I/O                                                             */
	/************************************************************************/
	FILE *f, *fout;
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("config file doesn't exist\n");
		return 1;
	}
	fout = fopen("trace.txt", "wt");
	if (fout == NULL) {
		printf("cannot write tract.txt\n");
		return 1;
	}

	/************************************************************************/
	/* DEBUG                                                                */
	/************************************************************************/
	FILE *stream = fout;

	/************************************************************************/
	/* READ SIZE OF PROCESS AND RESOURCE                                    */
	/************************************************************************/
	fscanf(f, "%d %d", &processSize, &resourceSize);


	/************************************************************************/
	/* VARIABLE SETUP                                                       */
	/************************************************************************/
	// Memory allocation
	totalResources = (int*)calloc(resourceSize, sizeof(int));
	availResources = (int*)calloc(resourceSize, sizeof(int));
	availProcesses = (int*)calloc(resourceSize, sizeof(int));
	allocMat = (int**)calloc(processSize, sizeof(int*));
	maxMat = (int**)calloc(processSize, sizeof(int*));
	for (int i = 0; i < processSize; ++i) {
		allocMat[i] = (int*)calloc(resourceSize, sizeof(int));
		maxMat[i] = (int*)calloc(resourceSize, sizeof(int));
	}
	order = (int*)calloc(processSize, sizeof(int));

	/************************************************************************/
	/* READ DATA FROM CONFIG FILE                                           */
	/************************************************************************/
	// total resources
	for (i = 0; i < resourceSize; i++) {
		fscanf(f, "%d", &totalResources[i]);
	}

	// allocation matrix
	for (i = 0; i < processSize; i++) {
		for (j = 0; j < resourceSize; ++j) {
			fscanf(f, "%d", &allocMat[i][j]);
		}
	}

	// maximum matrix
	for (i = 0; i < processSize; ++i) {
		for (j = 0; j < resourceSize; ++j) {
			fscanf(f, "%d", &maxMat[i][j]);
		}
	}

	// Initial validation on resources
	for (r = 0; r < resourceSize; ++r) {
		availResources[r] = totalResources[r];
		for (p = 0; p < processSize; ++p) {
			availResources[r] -= allocMat[p][r];
		}
		if (availResources[r] < 0) {
			fprintf(stdout, "sum of initially allocated resource(R%d) exceeds the total\nneeds %d more R%d\n", r, -availResources[r], r);
			return 1;
		}
	}


	/************************************************************************/
	/* START BANKER'S ALGORITHM                                             */
	/************************************************************************/
	int remains = processSize;
	processed = (bool*)calloc(processSize, sizeof(bool));

	printTotalResources(stream);

	while (remains > 0) {
		printStep(stream, processSize - remains + 1);
		printAvailableResources(stream);
		for (p = 0; p < processSize; ++p) {
			if (!processed[p]) printProcessState(stream, p);
		}

		// Get available processes
		int availNum = getAvailableProcesses(availProcesses);
		printAvailableProcesses(stream, availNum, availProcesses);

		// If there is no available process, just break out.
		if (availNum == 0) break;

		// Allocate resources to the first process of available list
		p = availProcesses[0];
		printAllocateAndRelease(stream, p);
		allocateAndRelease(p);

		// Push to order list
		order[orderIndex++] = p;

		remains--;
	}

	if (remains == 0)
		printProcessOrder(stream);
	else
		printDeadlock(stream);

	/************************************************************************/
	/* END                                                                  */
	/************************************************************************/
	// Release the memory
	fclose(f);
	fclose(fout);
	free(totalResources);
	free(availResources);
	free(availProcesses);
	for (int i = 0; i < processSize; i++) {
		free(allocMat[i]);
		free(maxMat[i]);
	}
	free(allocMat);
	free(maxMat);
	free(order);

	return 0;
}

void printStep(FILE *f, int step) {
	fprintf(f, "[step %d]====================================\n", step);
}

void printTotalResources(FILE *f) {
	fprintf(f, "Total resources:[");
	for (int i = 0; i < resourceSize - 1; ++i)
		fprintf(f, "%d,", totalResources[i]);
	fprintf(f, "%d]\n", totalResources[resourceSize - 1]);
}

void printAvailableResources(FILE *f) {
	fprintf(f, "Available resources:[");
	for (int i = 0; i < resourceSize; ++i) {
		if (i != 0) fprintf(f, ",");
		fprintf(f, "%d", availResources[i]);
	}
	fprintf(f, "]\n");
}

void printProcessState(FILE *f, int process) {
	fprintf(f, "P%d -> A:[", process);
	for (int i = 0; i < resourceSize; ++i) {
		if (i != 0) fprintf(f, ",");
		fprintf(f, "%d", allocMat[process][i]);
	}
	fprintf(f, "], N:[");
	for (int i = 0; i < resourceSize; ++i) {
		if (i != 0) fprintf(f, ",");
		fprintf(f, "%d", maxMat[process][i]);
	}
	fprintf(f, "]\n");
}

void printProcessOrder(FILE *f) {
	fprintf(f, "[FINAL]=====================================\n");
	fprintf(f, "Processing order:");
	for (int i = 0; i < orderIndex; ++i) {
		if (i != 0) fprintf(f, "->");
		fprintf(f, "P%d", order[i]);
	}
	fprintf(f, "\n");
}

void printAllocateAndRelease(FILE *f, int process) {
	fprintf(f, "\t (1) P%d is selected to be processed\n", process);
	fprintf(f, "\t (2) allocate [");
	for (int r = 0; r < resourceSize; ++r) {
		int needed = maxMat[process][r] - allocMat[process][r];
		if (r != 0) fprintf(f, ",");
		fprintf(f, "%d", needed);
	}
	fprintf(f, "] to P%d\n", process);
	fprintf(f, "\t (3) P%d completes its work and returns resources\n", process);

}


void printAvailableProcesses(FILE* f, int num, int* processes) {
	fprintf(f, "[");
	for (int i = 0; i < num; ++i) {
		if (i != 0)
			fprintf(f, ",");
		fprintf(f, "P%d", processes[i]);
	}
	if (num != 0)
		fprintf(f, "] can be processed \n");
	else
		fprintf(f, "] no process can be processed\n");
}

void printDeadlock(FILE* f) {
	fprintf(f, "[FINAL]=====================================\n");
	fprintf(f, "Deadlock situation.\n");
}

void allocateAndRelease(int process) {
	processed[process] = true;
	for (int r = 0; r < resourceSize; ++r) {
		int needed = maxMat[process][r] - allocMat[process][r];
		// alocate
		availResources[r] -= needed;
		// release
		availResources[r] += allocMat[process][r] + needed;
	}
}

int getAvailableProcesses(int* out) {
	int cnt = 0;
	for (int p = 0; p < processSize; ++p) {
		if (processed[p]) continue;

		if (isAvailable(p))
			out[cnt++] = p;
	}
	return cnt;
}

bool isAvailable(int process) {
	for (int r = 0; r < resourceSize; ++r) {
		int needed = maxMat[process][r] - allocMat[process][r];
		if (needed > availResources[r]) {
			return false;
		}
	}
	return true;
}
