/**************************************************************
 * Class:  CSC-415-01 Summer 2022
 * Name: Daniel Guo
 * Student ID: 913290045
 * GitHub ID: yiluun
 * Project: Assignment 4 – Word Blast
 *
 * File: Guo_Daniel_HW4_main.c
 *
 * Description:
 * A program that parses a War and Peace text file
 * and returns the top 10 most repeated words in the text file.
 * The program will only use Linux functions and will utilize
 * multithreading to divide the parsing effort.
 *
 **************************************************************/

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// You may find this Useful
char *delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

// a mutex lock to help us protect critical sections
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// structure that will store each word individually and their associated count
typedef struct wordStruct
{
    char *word;
    int count;
} wordStruct;

// array initialization
struct wordStruct wordArray[10000];

// count will keep track of our array
int count = 0;
size_t chunks = 0;
int file;

// -------------------------------- function for tallying -------------------------------- //
void *wordSort()
{
    char *tokens;
    size_t size = chunks;

    // initializing buffer
    char *buffer = malloc(size);

    // after buffer is initialized, we read the file into it
    read(file, buffer, size);

    // need a variable to store info outside of for loop
    int compareValue;

    // tokenization throughout the buffer
    while (tokens = strtok_r(buffer, delim, &buffer))
    {

        // checking token size to start tallying words that meet the criteria
        if (strlen(tokens) >= 6)
        {

            // iterating through the array to check if the token is already located inside
            for (int i = 0; i < 5000; i++)
            {

                // have to save info to a global variable because "i" can't be accessed outside the loop
                compareValue = strcasecmp(wordArray[i].word, tokens);

                // if the token exists within the word array, this will eventually == 0 and all we
                // have to do is to update the count variable for the structure object
                if (strcasecmp(wordArray[i].word, tokens) == 0)
                {

                    // critical section needs lock/semaphores
                    pthread_mutex_lock(&lock);

                    wordArray[i].count++;

                    pthread_mutex_unlock(&lock);

                    break;
                }
            }

            // uses to the global variable to see if the word was found
            if (compareValue != 0)
            {
                // checking to make sure there is no memory issues from array size being exceeded
                if (count < 10000)
                {
                    // critical section needs lock/semaphores
                    pthread_mutex_lock(&lock);

                    // strcpy to add the new word to the array
                    strcpy(wordArray[count].word, tokens);

                    // incrementing count for the first time
                    wordArray[count].count++;

                    pthread_mutex_unlock(&lock);

                    // array size tracker
                    count++;
                }
            }
        }
    }
}

// -------------------------------- function for tallying -------------------------------- //

int main(int argc, char *argv[])
{
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures

    // allocating size for the word in each structure
    for (int i = 0; i < 10000; i++)
    {
        wordArray[i].word = malloc(20);
    }

    // getting the file name from the first CL argument and making it open as "read only"
    file = open(argv[1], O_RDONLY);

    // lseek() takes a file descriptor which is obtained by using open()
    // this gives us the file size we need to determine chunk allocation
    off_t fileSize = lseek(file, 0, SEEK_END);

    // moves pointer back to the start
    lseek(file, 0, SEEK_SET);

    // convert second CL argument into an int which tells us how many threads to create
    int threadCount = atoi(argv[2]);

    // lseek gives us file size which we divide by number of threads to determine chunk allocation
    chunks = fileSize / threadCount;

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    // creating thread array based on thread count
    pthread_t thread[threadCount];

    // individual creation of threads
    for (int i = 0; i < threadCount; i++)
    {
        // initializes thread attributes with default attribute value to input into pthread_create()
        // https://man7.org/linux/man-pages/man3/pthread_attr_init.3.html
        pthread_attr_t attribute;
        pthread_attr_init(&attribute);
        int createdThread = pthread_create(&thread[i], &attribute, wordSort, NULL);
    }

    // joining all threads
    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(thread[i], NULL);
    }

    // ***TO DO *** Process TOP 10 and display

    // https://cs50.stackexchange.com/questions/39508/how-to-swap-elements-in-struct-data-type
    // how to swap structures:
    // temp = a;
    // a = b;
    // b = temp;

    // creating a temp structure to help sort/swap
    wordStruct temp;

    // nested for loop to compare each structure then swap
    for (int a = 0; a < 10000; a++)
    {
        for (int b = a + 1; b < 10000; b++)
        {
            if (wordArray[a].count < wordArray[b].count)
            {
                temp = wordArray[a];
                wordArray[a] = wordArray[b];
                wordArray[b] = temp;
            }
        }
    }

    // printing data with sample output formatting
    printf("\n\nWord Frequency Count on %s with %s threads\n", argv[1], argv[2]);
    printf("Printing top 10 words 6 characters or more.\n");

    // printing the top 10 most repeated words in the text
    for (int i = 0; i < 10; i++)
    {
        printf("Number %d is %s with a count of %d\n", i + 1, wordArray[i].word, wordArray[i].count);
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
    close(file);

    // http://git.savannah.gnu.org/cgit/hurd/libpthread.git/tree/sysdeps/generic/pt-mutex-destroy.c
    pthread_mutex_destroy(&lock);

    return 0;
}