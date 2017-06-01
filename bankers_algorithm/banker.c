#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

typedef enum { false, true } bool;

void printTotalRes(int resourceSize, int* totalRes);
void printAvailRes(int resourceSize, int* availRes);
void printProcessState(int resourceSize, int process, int** allocMat, int** maxMat);
void printAvailProcess(int processSize, int resourceSize, int* processed, int* availRes, int** allocMat, int** maxMat);

int main(int argc, char **argv)
{
	/************************************************************************/
	/* XML INPUT SETUP                                                      */
	/************************************************************************/
	xmlDoc         *document;
	xmlNode        *root, *node, *process, *resource;
	char           *filename;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename.xml\n", argv[0]);
		return 1;
	}
	xmlKeepBlanksDefault(0);

	filename = argv[1];
	document = xmlReadFile(filename, NULL, 0);
	root = xmlDocGetRootElement(document);
	fprintf(stdout, "Root is <%s> (%i)\n", root->name, root->type);

	node = root->children;

	int processSize, resourceSize;
	// process size
	processSize = atoi(node->children[0].content);
	node = node->next;

	// resource size
	resourceSize = atoi(node->children[0].content);
	node = node->next;


	/************************************************************************/
	/* VARIABLE SETUP                                                       */
	/************************************************************************/
	// Variables
	int *totalRes, *availRes, **allocMat, **maxMat;
	int i, j;
	int p, r;

	// Memory allocation
	totalRes = (int*)calloc(resourceSize, sizeof(int));
	availRes = (int*)calloc(resourceSize, sizeof(int));
	allocMat = (int**)calloc(processSize, sizeof(int*));
	maxMat = (int**)calloc(processSize, sizeof(int*));
	for (int i = 0; i < processSize; ++i) {
		allocMat[i] = (int*)calloc(resourceSize, sizeof(int));
		maxMat[i] = (int*)calloc(resourceSize, sizeof(int));
	}

	/************************************************************************/
	/* READ DATA FROM CONFIG FILE                                           */
	/************************************************************************/
// total resources
	for (i = 0, resource = node->children; resource; i++, resource = resource->next) {
		//fprintf(stdout, "\t Total size of resource <%s> is (%s)\n", resource->name, resource->children[0].content);
		totalRes[i] = atoi(resource->children[0].content);
	}
	node = node->next;

	// allocation matrix
	for (i = 0, process = node->children; process; i++, process = process->next) {
		//fprintf(stdout, "\t Process is <%s> (%i)\n", process->name, process->type);
		for (j = 0, resource = process->children; resource; j++, resource = resource->next) {
			//fprintf(stdout, "\t Resource is <%s> (%s)\n", resource->name, resource->children[0].content);
			allocMat[i][j] = atoi(resource->children[0].content);
		}
	}
	node = node->next;

	// maximum matrix
	for (i = 0, process = node->children; process; i++, process = process->next) {
		//fprintf(stdout, "\t Process is <%s> (%i)\n", process->name, process->type);
		for (j = 0, resource = process->children; resource; j++, resource = resource->next) {
			//fprintf(stdout, "\t Resource is <%s> (%s)\n", resource->name, resource->children[0].content);
			maxMat[i][j] = atoi(resource->children[0].content);
		}
	}
	node = node->next;

	/************************************************************************/
	/* START BANKER'S ALGORITHM                                             */
	/************************************************************************/
	int remains = processSize;
	bool *processed = (bool*)calloc(processSize, sizeof(bool));

	// Initial available resources
	for (r = 0; r < resourceSize; ++r) {
		availRes[r] = totalRes[r];
		for (p = 0; p < processSize; ++p) {
			availRes[r] -= allocMat[p][r];
		}
		if (availRes[r] < 0) {
			fprintf(stderr, "sum of initial resource(R%d) exceeds the total resources\n", r);
		}
	}

	// Allocate resources to processes
	printTotalRes(resourceSize, totalRes);
	while (remains > 0) {
		printf("[step %d]====================================\n", processSize - remains + 1);
		printAvailRes(resourceSize, availRes);
		for (p = 0; p < processSize; ++p)
			if (!processed[p]) printProcessState(resourceSize, p, allocMat, maxMat);
		printAvailProcess(processSize, resourceSize, processed, availRes, allocMat, maxMat);

		for (p = 0; p < processSize; ++p) {
			if (processed[p]) continue;

			bool available = true;
			for (r = 0; r < resourceSize; ++r) {
				int needed = maxMat[p][r] - allocMat[p][r];
				if (needed > availRes[r]) {
					available = false;
					break;
				}
			}
			if (available) {
				processed[p] = true;
				remains--;

				// allocate
				printf("\t (1) P%d is selected to be processed\n", p);
				printf("\t (2) allocate [");
				for (int r = 0; r < resourceSize; ++r) {
					int needed = maxMat[p][r] - allocMat[p][r];
					if (r != 0) printf(",");
					printf("%d", needed);
				}
				printf("] to P%d\n", p);

				// release
				for (r = 0; r < resourceSize; ++r) {
					availRes[r] += allocMat[p][r];
				}
				break;
			}
			else {
				continue;
			}
		}
	}

	/************************************************************************/
	/* END                                                                  */
	/************************************************************************/

	// Return the memory
	free(totalRes);
	free(availRes);
	for (int i = 0; i < processSize; i++) {
		free(allocMat[i]);
		free(maxMat[i]);
	}
	free(allocMat);
	free(maxMat);

	return 0;
}

void printTotalRes(int resourceSize, int* totalRes) {
	printf("Total:[");
	for (int i = 0; i < resourceSize - 1; ++i)
		printf("%d,", totalRes[i]);
	printf("%d]\n", totalRes[resourceSize - 1]);
}

void printAvailRes(int resourceSize, int* availRes) {
	printf("Available:[");
	for (int i = 0; i < resourceSize - 1; ++i)
		printf("%d,", availRes[i]);
	printf("%d]\n", availRes[resourceSize - 1]);
}

void printProcessState(int resourceSize, int process, int** allocMat, int** maxMat) {
	printf("P%d -> A:[", process);
	for (int i = 0; i < resourceSize - 1; ++i) {
		printf("%d,", allocMat[process][i]);
	}
	printf("%d], N:[", allocMat[process][resourceSize - 1]);
	for (int i = 0; i < resourceSize - 1; ++i) {
		printf("%d,", maxMat[process][i]);
	}
	printf("%d]\n", maxMat[process][resourceSize - 1]);
}

void printAvailProcess(int processSize, int resourceSize, int* processed, int* availRes, int** allocMat, int** maxMat) {
	printf("[");
	int cnt = 0;
	for (int p = 0; p < processSize; ++p) {
		if (processed[p]) continue;

		bool available = true;
		for (int r = 0; r < resourceSize; ++r) {
			int needed = maxMat[p][r] - allocMat[p][r];
			if (needed > availRes[r]) {
				available = false;
				break;
			}
		}
		if (available) {
			if (cnt++ != 0)
				printf(",");
			printf("P%d", p);
		}
	}
	printf("] can be processed \n");
}
