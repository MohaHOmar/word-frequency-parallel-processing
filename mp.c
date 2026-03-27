//multiprocessing Mohammad Omar 1221332
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>

#define MAX_WORD_LENGTH 100 // Increased to handle longer words
#define INITIAL_SIZE 270000 // Maximum array size

typedef struct node {
   int frequency;
   char word[MAX_WORD_LENGTH];
} WordNode;

typedef struct sharedData {
   int size;
   WordNode NewArray[INITIAL_SIZE];
} sharedData;

// Global semaphore
sem_t soso;

int fileSize() {
   FILE* file = fopen("test.txt", "r");
   if (!file) {
       printf("ERROR: File not found\n");
       exit(1);
   }

   int counter = 0;
   char file_data[MAX_WORD_LENGTH];
   while (fscanf(file, "%99s", file_data) != EOF) { // Safer fscanf with limit
       counter++;
   }
   fclose(file);
   return counter;
}

WordNode* loadFile(int BigArraySize) {
   WordNode* my_array = (WordNode*)malloc(BigArraySize * sizeof(WordNode));
   if (!my_array) {
       printf("ERROR: malloc failed\n");
       exit(1);
   }

   FILE* file = fopen("test.txt", "r");
   if (!file) {
       printf("ERROR: File not found\n");
       exit(1);
   }

   for (int i = 0; i < BigArraySize; i++) {
       if (fscanf(file, "%99s", my_array[i].word) != 1) {
           printf("ERROR: Reading word failed\n");
           exit(1);
       }
       my_array[i].frequency = 1;
   }
   fclose(file);
   return my_array;
}

int comp(const void* a, const void* b) {
   WordNode* aa = (WordNode*)a;
   WordNode* bb = (WordNode*)b;
   return (bb->frequency - aa->frequency); // Descending order
}

void myWork(int startIdx, int endIdx, WordNode* bigArray, WordNode* smallArray, int* localSize) {
   for (int i = startIdx; i < endIdx; i++) {
       int found = 0;
       for (int j = 0; j < *localSize; j++) {
           if (strcmp(smallArray[j].word, bigArray[i].word) == 0) {
               smallArray[j].frequency++;
               found = 1;
               break;
           }
       }
       if (!found) {
           if (*localSize >= INITIAL_SIZE) {
               printf("ERROR: Local array size exceeded\n");
               exit(1);
           }
           strcpy(smallArray[*localSize].word, bigArray[i].word); // Copy word
           smallArray[*localSize].frequency = 1; // Initialize frequency
           (*localSize)++;
       }
   }
}

void mergeArrays(WordNode* resultArray, int* resultSize, WordNode* childArray, int childSize) {
   for (int i = 0; i < childSize; i++) {
       int found = 0;

       // Search if the word already exists in the resultArray
       for (int j = 0; j < *resultSize; j++) {
           if (strcmp(resultArray[j].word, childArray[i].word) == 0) {
               sem_wait(&soso); // Use the global semaphore
               resultArray[j].frequency += childArray[i].frequency;
               found = 1;
               sem_post(&soso);
               break;
           }
       }

       if (!found) {
           sem_wait(&soso); // Use the global semaphore
           if (*resultSize >= INITIAL_SIZE) {
               printf("ERROR: Merged array size exceeded\n");
               exit(1);
           }
           strcpy(resultArray[*resultSize].word, childArray[i].word); // Copy word
           resultArray[*resultSize].frequency = childArray[i].frequency; // Copy frequency
           (*resultSize)++;
           sem_post(&soso);
       }
   }
}

void sortWords(WordNode* array, int size) {
   qsort(array, size, sizeof(WordNode), comp);
}

double time_diff(struct timeval start, struct timeval end) {
   return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main() {
   int num_of_childs, BigArraySize;
   printf("Enter the number of child processes you want: ");
   scanf("%d", &num_of_childs);

   // Initialize the global semaphore
   if (sem_init(&soso, 1, 1) != 0) {
       printf("ERROR: Semaphore initialization failed\n");
       exit(1);
   }

   struct timeval start, end;
   gettimeofday(&start, NULL);

   BigArraySize = fileSize();
   WordNode* bigArray = loadFile(BigArraySize);

   // Shared memory for result array
   sharedData* shared_memory = mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if (shared_memory == MAP_FAILED) {
       printf("ERROR: mmap failed\n");
       exit(1);
   }
   shared_memory->size = 0;

   int chunkSize = BigArraySize / num_of_childs;
   pid_t pids[num_of_childs];

   for (int i = 0; i < num_of_childs; i++) {
       pids[i] = fork();

       if (pids[i] == 0) { // Child process
           int startIdx = i * chunkSize;
           int endIdx = (i == num_of_childs - 1) ? BigArraySize : startIdx + chunkSize;

           WordNode* smallArray = (WordNode*)malloc(INITIAL_SIZE * sizeof(WordNode));
           if (!smallArray) {
               printf("ERROR: malloc failed in child\n");
               exit(1);
           }

           int localSize = 0;
           myWork(startIdx, endIdx, bigArray, smallArray, &localSize);

           mergeArrays(shared_memory->NewArray, &shared_memory->size, smallArray, localSize);

           free(smallArray);
           exit(0); // Child exits
       }
   }

   // Parent process waits for children
   for (int i = 0; i < num_of_childs; i++) {
       waitpid(pids[i], NULL, 0);
   }

   // Sort the merged array
   sortWords(shared_memory->NewArray, shared_memory->size);

   // Print the top 10 words
   printf("Top 10 most frequent words:\n");
   for (int i = 0; i < 10 && i < shared_memory->size; i++) {
       printf("%s: %d\n", shared_memory->NewArray[i].word, shared_memory->NewArray[i].frequency);
   }

   // Clean up
   free(bigArray);
   munmap(shared_memory, sizeof(sharedData));
   sem_destroy(&soso); // Destroy the global semaphore

   gettimeofday(&end, NULL);
   double elapsed_time_seconds = time_diff(start, end);
   double elapsed_time_minutes = elapsed_time_seconds / 60.0;
   printf("Elapsed time: %.2f minutes\n", elapsed_time_minutes);

   return 0;
}
