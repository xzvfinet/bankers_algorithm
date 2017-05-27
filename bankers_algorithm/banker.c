#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

int main(int argc, char **argv)
{
	xmlDoc         *document;
	xmlNode        *root, *first_child, *node, *process, *resource;
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
	int **allocMat, **maxMat;

	// process size
	processSize = atoi(node->children[0].content);
	node = node->next;

	// resource size
	resourceSize = atoi(node->children[0].content);
	node = node->next;

	// Memory allocation
	allocMat = (int**)malloc(sizeof(int*) * processSize);
	maxMat = (int**)malloc(sizeof(int*) * processSize);
	for (int i = 0; i < processSize; ++i) {
		allocMat[i] = (int*)malloc(sizeof(int) * resourceSize);
		maxMat[i] = (int*)malloc(sizeof(int) * resourceSize);
	}

	//for (int i = 0; i < processSize; ++i) {
	//	for (int j = 0; j < resourceSize; ++j) {
	//		allocMat[i][j] = 
	//	}
	//}

	int i, j;

	// allocation matrix
	for (i = 0, process = node->children; process; i++, process = process->next) {
		//fprintf(stdout, "\t Process is <%s> (%i)\n", process->name, process->type);
		for (j=0, resource = process->children; resource; j++, resource = resource->next) {
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


	// Return the memory
	for (int i = 0; i < processSize; i++) {
		free(allocMat[i]);
		free(maxMat[i]);
	}
	free(allocMat);
	free(maxMat);

	return 0;
}