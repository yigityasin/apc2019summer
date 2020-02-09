#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#define RED     "\033[31m"      //Red
#define RESET     "\033[0m"      //Reset

typedef struct data {
    int  id;
    int  s;
    int  e;
    int  * array_address;
} data;


int is_power_of_two(int i)
{
    return ((i != 0) && ((i & (~i + 1)) == i));
}

void * load_from_mem(void * d)
{
    data *my_data =(data *) d;;
    volatile int*  load_to;
    
    fflush(stdout);
    int my_id = my_data->id;
    int start = my_data->s;
    int end = my_data->e;
    load_to = calloc(end-start+1, sizeof(int));

    int * my_array =  my_data->array_address;
    setbuf(stdout, NULL);
    int i;
    int j=0;
    for (i=start; i<=end; i++)
    {
        load_to[j] =  my_array[i];
        j++;
    }

    return (void*) NULL;  //return clock ticks
}
int main (int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Please enter array size and thread number\n");
        return 0;
    }

    int num_threads = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    pthread_t threads[num_threads];
    
    if(is_power_of_two(array_size) == 0)
    {
        printf(RED "Array size should be power of two!!!\n" RESET);
        return 0;
    }
    printf("Number of threads %d, array size %d\n", num_threads, array_size);

    int *array;
    array = calloc(array_size, sizeof(int));
    
    int i;
    for(i=0;i<array_size;i++) //create the array given size
    {
        array[i] = rand()%array_size;
    }

    //divide array almost equeal chunks, send to thread as a parameter
    int chunk_size = array_size / num_threads;
    int s = 0;
    int e = chunk_size-1;
    int remainder = array_size % num_threads;

    struct timeval t0, t1;
    
    gettimeofday(&t0, NULL);
    for (i = 0; i < num_threads; i++)
    {
        if (remainder)
        {
            //printf("remainder %d\n", remainder);
            remainder--;
            e++;
        }
        data d = {i,s,e, array}; //id, start index, end index and array are sent
        pthread_create(&threads[i], NULL, load_from_mem, &d);
        s = e+1;
        e = s+chunk_size-1;
    }

    void* status;
    for(i = 0; i<num_threads; i++)
    {
        pthread_join(threads[i], &status);
    }
    gettimeofday(&t1, NULL);
    
    FILE *res_file;
    char f_name[100];
    sprintf(f_name,"volatile_array_result%d.csv", array_size);
    res_file = fopen(f_name,"a");
    fprintf(res_file, "%.5f\n", (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) 1e6);
    fclose(res_file);
    return 0;
}