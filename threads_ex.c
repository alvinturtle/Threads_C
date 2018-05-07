#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <sys/time.h> 
#include <sys/resource.h> 
#include <unistd.h> 

typedef struct { 
    unsigned int size; 
    unsigned int *part_int_arr; 
} f_args; 

struct timeval tv_delta(struct timeval start, struct timeval end) { 
    struct timeval delta = end; 

    delta.tv_sec -= start.tv_sec; 
    delta.tv_usec -= start.tv_usec; 
    if (delta.tv_usec < 0) { 
        delta.tv_usec += 1000000; 
        delta.tv_sec--; 
    } 

    return delta; 
} 

void *compute_max(void*); 
void *compute_sum(void*); 

int main(int argc, char **argv) { 
    unsigned int args[5], *int_arr, i, q_nelmt, r_nelmt, pos = 0; 
    void *(*func)(void*); 
    enum { NELMT, NTHRD, SEED, TASK, PRNT }; 
    pthread_t *tid_arr; 
    f_args func_arguments; 
    void *result; 

    struct timeval tv0, tv1, dtv, dru_user, dru_sys; 
    struct rusage ru0, ru1; 

    if (argc != 6) { 
        printf("Fuck you\n"); 
        exit(0); 
    } 

    while (--argc) { 
        args[argc - 1] = atoi(argv[argc]); 
    } 

    if (!(int_arr = malloc(args[NELMT] * sizeof(unsigned int)))) { 
        perror("Something bad happened"); 
        exit(1); 
    } 
    if (!(tid_arr = malloc(args[NTHRD] * sizeof(pthread_t)))) { 
        perror("Where am I? Who is this?! What's going on???"); 
        free(int_arr); 
        exit(1); 
    } 

    srand(args[SEED]); 
    for (i = 0; i < args[NELMT]; i++) { 
        int_arr[i] = rand(); 
    } 

    func = args[TASK] == 1 ? compute_max : compute_sum; 
    q_nelmt = args[NELMT] / args[NTHRD]; 
    r_nelmt = args[NELMT] % args[NTHRD]; 

    gettimeofday(&tv0, 0); 
    getrusage(RUSAGE_SELF, &ru0); 

    for (i = 0; i < args[NTHRD]; i++) { 
        func_arguments.size = q_nelmt + (r_nelmt-- > 0); 
        func_arguments.part_int_arr = int_arr + i*pos; 
        if (pthread_create(tid_arr + i, 0, func, &func_arguments)) { 
            perror("To shreds you say?"); 
            free(int_arr); 
            free(tid_arr); 
            exit(1); 
        }
        printf("\n");
        pos += func_arguments.size; 
    } 
    free(int_arr); 

    func_arguments.size = args[NTHRD]; 
    func_arguments.part_int_arr = malloc(args[NTHRD] * sizeof(unsigned int)); 
    for (i = 0; i < args[NTHRD]; i++) { 
        if (pthread_join(tid_arr[i], &result)) { 
            perror("Lol you fucked up after all this you fucking asshole"); 
            free(tid_arr); 
            exit(1); 
        } 
        func_arguments.part_int_arr[i] = *(unsigned int*)result;
        free(result); 
    } 

    gettimeofday(&tv1, 0); 
    getrusage(RUSAGE_SELF, &ru1); 
    dru_user = tv_delta(ru0.ru_utime, ru1.ru_utime); 
    dru_sys = tv_delta(ru0.ru_stime, ru1.ru_stime); 
    dtv = tv_delta(tv0, tv1); 

    free(tid_arr); 

    if (args[PRNT]) {
        unsigned int *answer = (unsigned int*)func(&func_arguments);
    
        printf("%s: %u\n", (args[TASK] == 1 ? "Max" : "Sum"), *answer);
        free(answer);
    } 
    free(func_arguments.part_int_arr);
    printf("User time: %ld.%06ld\n", dru_user.tv_sec, dru_user.tv_usec); 
    printf("System time: %ld.%06ld\n", dru_sys.tv_sec, dru_sys.tv_usec); 
    printf("Wall time: %ld.%06ld\n", dtv.tv_sec, dtv.tv_usec); 

    return 0; 
} 

void *compute_max(void *args) { 
    unsigned int i = 0, *max = malloc(sizeof(unsigned int)); 

    for (*max = 0; i < ((f_args*)args)->size; i++) { 
        *max = ((f_args*)args)->part_int_arr[i] > *max ? 
               ((f_args*)args)->part_int_arr[i] : *max; 
    } 

    return max; 
} 

void *compute_sum(void *args) { 
    unsigned int i = 0, *sum = malloc(sizeof(unsigned int)); 

    for (*sum = 0; i < ((f_args*)args)->size; i++) { 
        *sum = (*sum + ((f_args*)args)->part_int_arr[i]) % 1000000; 
    } 

    return sum; 
} 