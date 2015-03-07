/* 
 * File:   main.c
 * Author: jordan
 *
 * Created on April 26, 2014, 12:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <crypt.h>
#include <unistd.h>
#include <string.h>
#include "thpool.h"
#include "md5.h"

#define TASKS 15
#define MIN_THR 2
#define MAX_THR 5
#define TIME_OUT 15

typedef struct packet {
    time_t fecha_hora;
    unsigned id_ussd;
    char *password;
} Packet;

void* task(void *a) {
    long i = (long) a;
    int s = 2 + rand() % 10;
    printf("Th(%u): Realizando tarea %ld por %d seg...\n", (int) pthread_self(), i, s);
    sleep(s);
    printf("-Th(%u): Termino tarea %ld\n", (int) pthread_self(), i);
}

#ifdef COMPILAR
int main(int argc, char** argv) {
    /* Creacion de un pool de hilos*/
    thr_pool_t *pool = thr_pool_create(MIN_THR, MAX_THR, TIME_OUT, NULL);
    long i;
    int ret;

    char pass[34];
    md5("admin", pass);
    while (!md5_compare(pass, getpass("Password : ")));
    puts("Welcome");
    exit(0);

    printf("Iniciando %d tareas\n\n", TASKS);
    for (i = 1; i <= TASKS; i++) {
        /*Encolamiento de tareas*/
        ret = thr_pool_queue(pool, task, (void*) i);
        if (ret == -1) {
            printf("----!Ocurrio un error con la tarea %ld\n", i);
        }
    }
    /*Se espera a que todas las tareas terminen*/
    thr_pool_wait(pool);

    return (EXIT_SUCCESS);
}
#endif
