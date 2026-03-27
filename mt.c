//multithreading Mohammad Hasan Omar
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_WORD_LENGTH 100
#define INITIAL_SIZE 260000

typedef struct node {
   int freq;
   char word[MAX_WORD_LENGTH];
} WordNode;

typedef struct {
   int start;
   int end;
   int size;
   WordNode* bigArray;
} ThreadArgs;

WordNode sharedMemory[INITIAL_SIZE];
int sharedSize = 0;
pthread_mutex_t m;

int fileSize() {
   FILE* file = fopen("test.txt", "r");
   if (!file) {
       printf("ERROR: File not found\n");
       exit(1);
   }

   int counter = 0;
   char file_data[MAX_WORD_LENGTH];
   while (fscanf(file, "%99s", file_data) != EOF) {
       counter++;
   }
   fclose(file);
   return counter;
}

WordNode* loadFile(int size) {
   WordNode* my_array = (WordNode*)malloc(size * sizeof(WordNode));
   if (!my_array) {
       printf("ERROR: malloc failed\n");
       exit(1);
   }
   FILE* file = fopen("test.txt", "r");
   if (!file) {
       printf("ERROR: File not found\n");
       free(my_array);
       exit(1);
   }

   for (int i = 0; i < size; i++) {
       if (fscanf(file, "%99s", my_array[i].word) != 1) {
           printf("ERROR: Reading word failed\n");
           free(my_array);
           fclose(file);
           exit(1);
       }
       my_array[i].freq = 1;
   }
   fclose(file);
   return my_array;
}

int comp(const void* a, const void* b) {
   WordNode* aa = (WordNode*)a;
   WordNode* bb = (WordNode*)b;
   return (bb->freq - aa->freq); // Descending order
}

double time_diff(struct timeval start, struct timeval end) {
   return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void threadJob(int start, int end, WordNode* bigArray) {
   WordNode* localArray = (WordNode*)malloc(INITIAL_SIZE * sizeof(WordNode));
   int localSize = 0;
   for (int i = start; i < end; i++) {
       int found = 0;
       for (int j = 0; j < localSize; j++) {
           if (strcmp(localArray[j].word, bigArray[i].word) == 0) {
               localArray[j].freq++;
               found = 1;
               break;
           }
       }
       if (!found) {
           if (localSize >= INITIAL_SIZE) {
               printf("ERROR: Local array size exceeded\n");
               exit(1);
           }
           strcpy(localArray[localSize].word, bigArray[i].word);
           localArray[localSize].freq = 1;
           localSize++;
       }
   }
   // Merge
   for (int i = 0; i < localSize; i++) {
       int found = 0;
       for (int j = 0; j < sharedSize; j++) {
           if (strcmp(sharedMemory[j].word, localArray[i].word) == 0) {
               sharedMemory[j].freq += localArray[i].freq;
               found = 1;
               break;
           }
       }
       if (!found) {
           if (sharedSize >= INITIAL_SIZE) {
               printf("ERROR: Shared array size exceeded\n");
               pthread_mutex_unlock(&m);
               exit(1);
           }
           strcpy(sharedMemory[sharedSize].word, localArray[i].word);
           sharedMemory[sharedSize].freq = localArray[i].freq;
           sharedSize++;
       }
   }
   pthread_mutex_unlock(&m);
   free(localArray);
}

void* threadWork(void* arg) {
   ThreadArgs* args = (ThreadArgs*)arg;
   threadJob(args->start, args->end, args->bigArray);
   return NULL;
}

int main() {
   int numThreads;
   printf("Enter the number of threads you want: ");
   scanf("%d", &numThreads);

   struct timeval start, end;
   gettimeofday(&start, NULL);

   int bigArraySize = fileSize();
   WordNode* bigArray = loadFile(bigArraySize);

   pthread_t threads[numThreads];
   ThreadArgs threadArgs[numThreads];
   pthread_mutex_init(&m, NULL);

   int chunkSize = bigArraySize / numThreads;
   for (int i = 0; i < numThreads; i++) {
       threadArgs[i].start = i * chunkSize;

       // Handle last thread, ensuring it gets the remaining elements
       if (i == numThreads - 1) {
           threadArgs[i].end = bigArraySize;
       } else {
           threadArgs[i].end = (i + 1) * chunkSize;
       }
       threadArgs[i].bigArray = bigArray;
       threadArgs[i].size = threadArgs[i].end - threadArgs[i].start; // Set the size for each thread

       if (pthread_create(&threads[i], NULL, threadWork, &threadArgs[i]) != 0) {
           printf("ERROR: Failed to create thread %d\n", i);
           exit(1);
       }
   }

   for (int i = 0; i < numThreads; i++) {
       pthread_join(threads[i], NULL);
   }

   // Sort the sharedArray
   qsort(sharedMemory, sharedSize, sizeof(WordNode), comp);
   pthread_mutex_destroy(&m);

   // Print the top 10 words
   printf("Top 10 most frequent words:\n");
   for (int i = 0; i < 10 && i < sharedSize; i++) {
       printf("%s: %d\n", sharedMemory[i].word, sharedMemory[i].freq);
   }

   free(bigArray);
   gettimeofday(&end, NULL);
   double elapsed_time_seconds = time_diff(start, end);
   double elapsed_time_minutes = elapsed_time_seconds / 60.0;
   printf("Elapsed time: %.2f minutes\n", elapsed_time_minutes);

   return 0;
}
