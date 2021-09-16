#include <stdlib.h>
#include <stdio.h>

int i;

typedef struct pageNode {
	struct pageNode* prevPage;
	struct pageNode* nextPage;
	int pageNum;
	pid_t processID;	
	double processSize;
} page;

typedef struct pageTab {
	page* start;
	page* end;
	int size;
	double availableMem;
} pageTable;

typedef struct segmentNode {
	struct segmentNode* prevSegment;
	struct segmentNode* nextSegment;
	int segmentNum;
	pageTable* pt;
} segment;

typedef struct segmentTab {
	segment* start;
	segment* end;
	int size;
	double availableMem;
} segmentTable;


page* createPage() {
	page* pagina = (page*) malloc (sizeof(page));
	pagina -> prevPage = NULL;
	pagina -> nextPage = NULL;
	pagina -> processID = 0;
	pagina -> processSize = 0;
	return pagina;
}

int pageTableIsEmpty(pageTable* tabla) {
	return (tabla -> start == NULL && tabla -> end == NULL);
}

int segmentTableIsEmpty(segmentTable* tabla) {
	return (tabla -> start == NULL && tabla -> end == NULL);
}

pageTable* createPageTable() {
	pageTable* tabla = (pageTable*) malloc(sizeof(pageTable));
	tabla -> start = NULL;
	tabla -> end = NULL;
	page* previous = NULL;
	page* current = NULL;
	tabla -> availableMem = 10;
	for (i = 0; i < 5; i++) {
		tabla -> size = i + 1;
		if (pageTableIsEmpty(tabla)) {
			tabla -> start = createPage();
			tabla -> end = tabla -> start;
			tabla -> start -> pageNum = i;
			previous = tabla -> start;
		}
		else {
			current = createPage();
			tabla -> end = current;
			current -> pageNum = i;
			current -> prevPage = previous;
			previous -> nextPage = current;
			previous = current;
		}
	}
	tabla -> size=5;
	return tabla;
}

segment* createSegment() {
	segment* segmento = (segment*) malloc(sizeof(segment));
	segmento -> prevSegment = NULL;
	segmento -> nextSegment = NULL;
	segmento -> pt = createPageTable();
	return segmento;
}

segmentTable* createSegmentTable() {
	segmentTable* tabla = (segmentTable*) malloc(sizeof(segmentTable));
	segment* segmento = createSegment();
	tabla -> start = segmento;
	tabla -> end = segmento;
	tabla -> availableMem = 10;
	segmento -> segmentNum = 0;
	tabla -> size = 1;
	return tabla;
}

void addSegment(segmentTable* tabla) {
	segment* segmento = createSegment();
	segment* previous;
	previous = tabla -> end;
	previous -> nextSegment = segmento;
	tabla -> end = segmento;
	segmento -> prevSegment = previous;
	segmento -> segmentNum = segmento -> prevSegment -> segmentNum + 1;
	tabla -> size += 1;
	tabla -> availableMem += 10;
}

void showMemory(segmentTable* tabla) {
	segment* currentSegment;
	page* currentPage;
	for(currentSegment = tabla -> start; currentSegment != NULL; currentSegment = currentSegment -> nextSegment) {
		printf(":::::Seg %d:::::\n", currentSegment->segmentNum);
		for (currentPage = currentSegment -> pt -> start; currentPage != NULL; currentPage = currentPage -> nextPage) {
			if (currentPage -> processID == 0) {
				printf("Pag %d: --- \n", currentPage -> pageNum);	
			}
			else {
				printf("Pag %d: Process %d \n", currentPage -> pageNum, currentPage -> processID);
			}	
		}	
	}
}

int availablePrimaryMemory(segmentTable* tabla, double processSize) {
	if((tabla -> availableMem) >= processSize) {
		return 1;
	}
	else{
		if((tabla -> size) < 5) {
			addSegment(tabla);
			return availablePrimaryMemory(tabla, processSize);
		}
		else {
			return 0;
		}
	}
}

int availableSecondaryMemory(segmentTable* tabla, double processSize) {
	if ((tabla->availableMem) >= processSize) {
		return 1;
	}
	else {
		if((tabla -> size) < 3) {
			addSegment(tabla);
			return availableSecondaryMemory(tabla, processSize);
		}
		else {
			return 0;
		}
	}
}

int createProcess(segmentTable* tabla, double processSize, int processID) {
	double size = processSize;
	segment* currentSegment = tabla -> start;
	page* currentPage;
	pageTable* currentPageTable;
	while (size > 0) {
		currentPageTable = currentSegment -> pt;
		currentPage = currentPageTable -> start;
		while (currentPage != NULL) {
			if (currentPage -> processID == 0 && size > 0) {
				currentPage -> processID = processID;
				currentPage -> processSize = processSize;
				size -= 2;
				currentPageTable -> availableMem -= 2;
				tabla -> availableMem -= 2;
				currentPage = currentPage -> nextPage;
			}
			else {
				currentPage = currentPage -> nextPage;
			}
		}
		currentSegment = currentSegment -> nextSegment;
	}
	return 1;
}

int closeProcess(segmentTable* tabla, double processSize, int processID) {
	double size = processSize;
	segment* currentSegment = tabla -> start;
	page* currentPage;
	pageTable* currentPageTable;
	while (size > 0) {
		currentPageTable = currentSegment -> pt;
		currentPage = currentPageTable -> start;
		while (currentPage != NULL) {
			if (currentPage -> processID == processID && size > 0) {
				currentPage -> processID = 0;
				currentPage -> processSize = 0;
				size -= 2;
				currentPageTable -> availableMem += 2;
				tabla -> availableMem += 2;
				currentPage = currentPage -> nextPage;
			}
			else {
				currentPage = currentPage -> nextPage;
			}
		}
		currentSegment = currentSegment -> nextSegment;
	}
	return 1;
}

double searchPID(segmentTable* tabla, pid_t pid){
	segment* currentSegment = tabla -> start;
	page* currentPage;
	while (currentSegment != NULL) {
		currentPage=currentSegment->pt->start;
		while (currentPage != NULL) {
			if (currentPage -> processID == pid) {
				return currentPage -> processSize;
			}
			else {
				currentPage = currentPage -> nextPage;
			}
		}
		currentSegment = currentSegment -> nextSegment;
	}
	return 0;
}

int swappingToSecondaryMem(segmentTable* tablaOr, segmentTable* tablaDest, pid_t pid, double processSize) {
	double pSize = searchPID(tablaOr, pid);
	if (pSize) {
		if (availableSecondaryMemory(tablaDest, processSize)) {
			createProcess(tablaDest, pSize, pid);	
			closeProcess(tablaOr, pSize, pid);
			return 1;
		}
		else {
			return 0;
			printf("Not enough memmory, close one or more process so you could make swapping \n");
		}
	}
	else {
		return -1;
		printf("ID not found in memory \n");
	}
}

int swappingToPrimaryMem(segmentTable* tablaOr, segmentTable* tablaDest, pid_t pid, double processSize) {
	double pSize = searchPID(tablaOr, pid);
	if (pSize) {
		if (availablePrimaryMemory(tablaDest, processSize)) {
			createProcess(tablaDest, pSize, pid);	
			closeProcess(tablaOr, pSize, pid);
			return 1;
		}
		else {
			return 0;
			printf("Not enough memmory, close one or more process so you could make swapping \n");
		}
	}
	else {
		return -1;
		printf("ID not found in memory \n");
	}
}

void showMenu() {
	printf(":::::::::::::::::::::::::::::::::::::::::::\n");
	printf("Simulador de administracion de memoria\n");
	printf("Que quieres hacer:\n");
	printf("1) Crear nuevo poceso\n");
	printf("2) Swapping\n");
	printf("3) Cerrar proceso\n");
	printf("4) Recuperar procesos de las memoria secundaria\n");
	printf("5) Mostrar estado de memoria\n");
	printf("6) Salir del simulador\n");	
}

void menu2() {
	printf("Select units: \n");
	printf("1) KB\n");
	printf("2) MB\n");
}

int main(int argc, char const *argv[]) {
	segmentTable* PrimaryMem;
	segmentTable* SecondaryMem;
	int option, mem, available, swap, successful;
	pid_t pid;
	double processSize;
	PrimaryMem = createSegmentTable();
	SecondaryMem = createSegmentTable();
	do {
		showMenu();
		fflush(stdin);
		scanf("%d", &option);
		fflush(stdin);
		switch(option) {
			case 1:	//Crear un nuevo proceso
				menu2();
				fflush(stdin);
				scanf("%d", &mem);
				fflush(stdin);
				if (mem == 1) {
					printf("Enter process size: \n");
					fflush(stdin);
					scanf("%lf", &processSize);
					fflush(stdin);
					processSize /= 1000;
					if (availablePrimaryMemory(PrimaryMem, processSize)) {
						pid_t pid = fork();
						if (pid == 0) {
							pid = getpid();
							successful = createProcess(PrimaryMem, processSize, pid);
							if (successful) {
								printf("Process created successfully, id %d asign \n", pid);
								wait(&pid);	
							}
							else {
								printf("Something went wrong \n");
								wait(&pid);
							}
						}
						else {
							successful = createProcess(PrimaryMem, processSize, pid);
							if (successful) {
								printf("Process created successfully, id %d asign \n", pid);
								wait(&pid);	
							}
							else {
								printf("Something went wrong \n");
								wait(&pid);
							}
						}
					}
					else {
						printf("Not enough memory for a process with that size, free memory closing processes o swapping to secondary memory \n");
					}
				}
				else if (mem == 2) {
					printf("Enter process size: \n");
					fflush(stdin);
					scanf(" %lf ", &processSize);
					fflush(stdin);
					if (availablePrimaryMemory(PrimaryMem, processSize)) {
						pid_t pid = fork();
						if (pid == 0) {
							pid = getpid();
							successful = createProcess(PrimaryMem, processSize, pid);
							if (successful) {
								printf("Process created successfully, id %d asign \n", pid);
								wait(&pid);	
							}
							else {
								printf("Something went wrong \n");
								wait(&pid);
							}
						}
						else {
							successful = createProcess(PrimaryMem, processSize, pid);
							if (successful) {
								printf("Process created successfully, id %d asign \n", pid);
								wait(&pid);	
							}
							else {
								printf("Something went wrong \n");
								wait(&pid);
							}
						}
					}
					else {
						printf("Not enough memory for a process with that size, free memory closing processes o swapping to secondary memory \n");
					}
				}
				else {
					printf("Try again \n");	
				}
			break;
			case 2:	//Swapping
				printf("Enter the id of the process you want to swap to secondary memory \n");
				fflush(stdin);
				scanf("%d", &pid);
				fflush(stdin);
				processSize = searchPID(PrimaryMem, pid);
				if (processSize) {
					swap = swappingToSecondaryMem(PrimaryMem, SecondaryMem, pid, processSize);
					if (swap > 0) {
						printf("Swapped successfully \n");
					}
					else {
						if (swap == 0) {
							printf("Not enoguh memory, close one or more process so you could make the swapping \n");	
						}
						else{
							printf("ID not found in memory \n");
						}
					}
				}
				else{
					printf("ID not found in memory \n");
				}
			break;
			case 3: //Cerrar proceso
				printf("Enter the process id you want to close \n");
				fflush(stdin);
				scanf(" %d ", &pid);
				fflush(stdin);
				processSize = searchPID(PrimaryMem, pid);
				printf(" %lf \n", processSize);
				if (processSize) {
					successful = closeProcess(PrimaryMem, processSize, pid);
					if (successful) {
						printf("Process %d closed successfully \n", pid);
					}
					else {
						printf("Something went wrong \n");
					}
				}
				else {
					processSize = searchPID(SecondaryMem, pid);
					if (processSize) {
						if (availablePrimaryMemory(PrimaryMem, processSize)) {
							swap = swappingToPrimaryMem(SecondaryMem, PrimaryMem, pid, processSize);
							if (swap > 0) {
								printf("Swapped succesfully \n");
								successful = closeProcess(PrimaryMem, processSize, pid);
								if (successful) {
									printf("Process %d closed successfully\n", pid);
								}
								else {
									printf("Something went wrong\n");
								}
							}
							else {
								if (swap == 0) {
									printf("Not enoguh memory, close one or more process so you could make the swapping \n");	
								}
								else {
									printf("ID not found in memory \n");
								}
							}	
						}
						else {
							printf("That process is in secondary memory, you must swap it to primary memory to close it, but first you must free memory in primary memory  \n");
						}
					}
					else {
						printf("ID not found in memory \n");
					}
				}
			break;
			case 4: //Recuperar proceso de memoria secundaria
				printf("Enter the id from the you want to recover from secondary memory \n");
				fflush(stdin);
				scanf(" %d ", &pid);
				fflush(stdin);
				processSize = searchPID(SecondaryMem, pid);
				if (processSize) {
					swap = swappingToPrimaryMem(SecondaryMem, PrimaryMem, pid, processSize);
					if (swap > 0) {
						printf("Swapped succesfully \n");
					}
					else {
						if (swap == 0) {
							printf("Not enoguh memory, close one or more process so you could make the swapping\n");	
						}
						else {
							printf("ID not found in memory \n");
						}
					}
				}
				else {
					printf("ID not found in memory \n");
				}
			break;
			case 5:	//Mostrar memoria
				printf("..:::Primary Memory:::..\n");
				showMemory(PrimaryMem);
				printf("..:::Secondary Memory:::..\n");
				showMemory(SecondaryMem);
			break;
			case 6:	//Cerrar programa
				free(PrimaryMem);
				free(SecondaryMem);
				exit(0);
			break;
			default:
				printf("Try again \n");
			break;
		}
	}
	while(1);
	return 0;
}