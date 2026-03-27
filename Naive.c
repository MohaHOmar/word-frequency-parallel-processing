//naive Mohammad Omar 1221332
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define MAX_WORD_LENGTH 100 // Increased to handle longer words
#define INITIAL_SIZE 270000 // Maximum array size

typedef struct node {
   int frequency;
   char word[MAX_WORD_LENGTH];
} WordNode;

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

void myWork(WordNode* bigArray, WordNode* smallArray, int BigArraySize, int* localSize) {
   for (int i = 0; i < BigArraySize; i++) {
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

void sortWords(WordNode* array, int size) {
   qsort(array, size, sizeof(WordNode), comp);
}

double time_diff(struct timeval start, struct timeval end) {
   return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main() {
   struct timeval start, end;
   gettimeofday(&start, NULL);

   int BigArraySize = fileSize();
   WordNode* bigArray = loadFile(BigArraySize);

   // Allocate memory for the array to store word frequencies
   WordNode* smallArray = (WordNode*)malloc(INITIAL_SIZE * sizeof(WordNode));
   if (!smallArray) {
       printf("ERROR: malloc failed\n");
       exit(1);
   }

   int smallArraySize = 0;

   // Count word frequencies
   myWork(bigArray, smallArray, BigArraySize, &smallArraySize);

   // Sort the words by frequency
   sortWords(smallArray, smallArraySize);

   // Print the top 10 words
   printf("Top 10 most frequent words:\n");
   for (int i = 0; i < 10 && i < smallArraySize; i++) {
       printf("%s: %d\n", smallArray[i].word, smallArray[i].frequency);
   }

   // Clean up
   free(bigArray);
   free(smallArray);

   gettimeofday(&end, NULL);
   double elapsed_time_seconds = time_diff(start, end);
   double elapsed_time_minutes = elapsed_time_seconds / 60.0;
   printf("Elapsed time: %.2f minutes\n", elapsed_time_minutes);

   return 0;
}