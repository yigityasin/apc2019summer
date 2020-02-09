#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define RED     "\033[31m"      //Red
#define RESET     "\033[0m"      //Reset
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//to be passed to threads as parameter
typedef struct data {
    int id;
    int d_counter;
    int n_threads;
    void *func;
} data;

int C = 0;

static pthread_barrier_t barrier;

typedef struct {
    int ticket;
    int current_ticket_number;
} lock_t;

lock_t lock = {-1, 0};

//our lock 
//this implementation looks like waiting in a queue at the bank
void acquire(lock_t *lock) {
    int my_ticket_number = __sync_add_and_fetch(&lock->ticket, 1);  //get a ticket from the kiosk
    while (lock->current_ticket_number != my_ticket_number);
}

//increase the ticket number on the table
void release(lock_t *lock) {
    lock->current_ticket_number += 1;
}

// three diffent incrementer function
void *increment_with_ticket_lock() {
    acquire(&lock);
    C++;
    release(&lock);
}

void *increment_with_pthread_lock() {
    pthread_mutex_lock(&mutex);
    C++;
    pthread_mutex_unlock(&mutex);
}

void *increment_atomic() {
    __sync_add_and_fetch(&C, 1);
}

void *increment(void *d) {
    data *my_data = (data *) d;;

    fflush(stdout);
    int my_id = my_data->id;
    int dc = my_data->d_counter;
    int t = my_data->n_threads;
    void (*inc_func)() = my_data->func;

    fflush(stdout);
    setbuf(stdout, NULL);
    int i;
    int j = 0;
    pthread_barrier_wait(&barrier);

    for (i = 0; i < dc / t; i++) {
        inc_func();
    }

    return (void *) NULL;  //return clock ticks
}


int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf(RED "Please enter array size, number of threads and lock type type\n"
               "Please enter which lock type you want\n"
               "For atomic increment, enter A\n"
               "For pthread lock, enter P\n"
               "For ticket lock, enter T \n"RESET);
        return 0;
    }

    int i;
    int desired_counter = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (desired_counter % num_threads != 0) {
        printf("Please choose correct values !!!\n" RESET);
        return 0;
    }
    char *choice = argv[3];
    printf("%d %d %s\n", desired_counter, num_threads, choice);
    struct timeval t0, t1;
    void *incrementer;

    int counter = 0;
    pthread_t threads[num_threads];

    // assign selected lock type to function pointer
    if (strcmp(choice, "A") == 0) {
        printf("Atomic add and fetch was selected.\n");
        incrementer = &increment_atomic;
    } else if (strcmp(choice, "P") == 0) {
        printf("Pthread lock was selected.\n");
        incrementer = &increment_with_pthread_lock;
    } else if (strcmp(choice, "T") == 0) {
        printf("Ticket lock was selected.\n");
        incrementer = &increment_with_ticket_lock;
    } else {
        printf("Atomic add and fetch was selected as default.\n");
        incrementer = &increment_atomic;
    }

    pthread_barrier_init(&barrier, NULL, num_threads);
    gettimeofday(&t0, NULL);
    for (i = 0; i < num_threads; i++) {
        data d = {i, desired_counter, num_threads, incrementer};
        pthread_create(&threads[i], NULL, increment, &d);
    }

    void *status;
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], &status);
    }
    gettimeofday(&t1, NULL);

    //write result to file named with given parameters
    FILE *res_file;
    char f_name[100];
    sprintf(f_name, "%s_%d_%d.csv", choice, desired_counter, num_threads);
    res_file = fopen(f_name, "a");
    fprintf(res_file, "%d, %.5f\n", C,
            (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) 1e6);
    printf("%d, %.5f\n", C,
            (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) 1e6);
    fclose(res_file);
    return 0;
}