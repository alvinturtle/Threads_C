#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

static int sum = 0;
static pthread_mutex_t lock;

typedef struct {
    unsigned int size;
    unsigned int *part_int_arr;
} f_args;

void *compute_max(void*);
void *compute_sum(void*);

void print_info(unsigned int *args, unsigned int *arr) {
    int i;
    printf("Number of elements: %d\n", args[0]);
    printf("Number of threads: %d\n", args[1]);
    printf("Seed: %d\n", args[2]);
    printf("Task: %s\n", (args[3] == 1 ? "Max" : "Sum"));
    printf("Print: %s\n\n", (args[4] ? "Yes" : "No"));
    printf("Random elements:\n");
    for (i = 0; i < args[0]; i++) {
        printf(" %4d: %8u\n", i, arr[i]);
    }
}

int main(int argc, char **argv) {
    unsigned int args[5], *int_arr, i, pos = 0;
    int q_nelmt, r_nelmt;
    void *(*func)(void*);
    enum { NELMT, NTHRD, SEED, TASK, PRNT };
    pthread_t *tid_arr;
    f_args *func_arguments;
    void *result;

    pthread_mutex_init(&lock, NULL); 
    if (argc != 6) {
        printf("Fuck you\n");
        exit(0);
    }
	
/*
	while (arg != 0) with a pre decrement
*/
    while (--argc) {
        args[argc - 1] = atoi(argv[argc]);
    }

    int_arr = malloc(args[NELMT] * sizeof(unsigned int));
    tid_arr = malloc(args[NTHRD] * sizeof(pthread_t));
    func_arguments = calloc(args[NTHRD], sizeof(f_args));

    srand(args[SEED]);
    for (i = 0; i < args[NELMT]; i++) {
        int_arr[i] = rand()%100;
    }

    print_info(args, int_arr);

    func = args[TASK] == 1 ? compute_max : compute_sum;
    q_nelmt = args[NELMT] / args[NTHRD];
    r_nelmt = args[NELMT] % args[NTHRD];

/*
	instantiating threads
*/
    for (i = 0; i < args[NTHRD]; i++) {
        (func_arguments + i)->size = (unsigned int)q_nelmt + (r_nelmt-- > 0);
        (func_arguments + i)->part_int_arr = int_arr + pos;
        printf("Thread %u: size=%u, arr_start=%u\n", i, (func_arguments + i)->size, *((func_arguments + i)->part_int_arr));
        printf(" arr_address: %p, pos: %u\n", (func_arguments + i)->part_int_arr, pos);
        pthread_create(tid_arr + i, 0, func, func_arguments + i);
        pos += (func_arguments + i)->size;
    }
/*
	preparing to execute
*/

    func_arguments->size = args[NTHRD];
    func_arguments->part_int_arr = malloc(args[NTHRD] * sizeof(unsigned int));
    for (i = 0; i < args[NTHRD]; i++) {
        pthread_join(tid_arr[i], &result);
        func_arguments->part_int_arr[i] = *(unsigned int*)result;
        printf("Thread %u result: %u\n", i, func_arguments->part_int_arr[i]);
        free(result);
    }

    if (args[PRNT]) {
        printf("\nCOMPUTING RESULT\n");
        unsigned int *answer = (unsigned int*)func(func_arguments);

        printf("RESULT:\n");
        printf("%s: %u\n", (args[TASK] == 1 ? "Max" : "Sum"), *answer);
        free(answer);
    }

	pthread_mutex_destroy(&lock);
    free(int_arr);
    free(tid_arr);
    free(func_arguments->part_int_arr);
    free(func_arguments);
    return 0;
}

void *compute_max(void *args) {
    unsigned int i = 0, *max = malloc(sizeof(unsigned int));
    printf(">> Size: %u\n", ((f_args*)args)->size);
    printf(">> Array start: %u\n   Address: %p\n", *(((f_args*)args)->part_int_arr), ((f_args*)args)->part_int_arr);

    for (*max = 0; i < ((f_args*)args)->size; i++) {
        *max = ((f_args*)args)->part_int_arr[i] > *max ?
               ((f_args*)args)->part_int_arr[i] : *max;
    }
    printf("Max: %u\n", *max);

    return max;
}

void *compute_sum(void *args) {
    unsigned int i = 0;
	unsigned int *sum_p;
	sum_p = malloc(1 * sizeof(unsigned int));

    for (i = 0; i < ((f_args*)args)->size; i++) {
	printf("adding %u\n", ((f_args*)args)->part_int_arr[i]);
	pthread_mutex_lock(&lock);
	sum += ((f_args*)args)->part_int_arr[i];
	pthread_mutex_unlock(&lock);
/*
        *sum = (*sum + ((f_args*)args)->part_int_arr[i]) % 1000000;
*/
    }
	*sum_p = sum;
    printf("Sum: %u\n", sum);
    return sum_p;
}
