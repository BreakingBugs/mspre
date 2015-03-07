/* 
 * File:   util.h
 * Author: ascend
 *
 * Created on 3 de mayo de 2014, 04:20 PM
 */

#ifndef UTIL_H
#define	UTIL_H 
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>

#define LOGPATH "/var/log/"
#define LOGFILE "mspre.log"

#define LOG_ERROR 1
#define LOG_INF 2
#define MAX_PATH_LENGTH 128
#define MAX_FILENAME_LENGTH 24
#define MAX_CONFIG_SIZE 512

typedef struct properties {
    int port;
    int threads;
    int timeout;
    char logpath[MAX_PATH_LENGTH];
    char logfile[MAX_FILENAME_LENGTH];
} Properties;

#define DEFAULT_PROP {8000, 10, 10, "/etc/mspre/", "mspred.log"};

int escribirLog(char *mensaje, const char *funcion, int tipo);
void write_sys(char *mensaje, const char *funcion, int tipo);
void convertirHora(time_t hora, char **resultado);
void settimeouts(int sock, int secs);

/**
 * Carga la configuracion del archivo .prop
 * @param proppath path del archivo .prop
 * @param p estructura de configuracion
 */
void config_read(char *proppath, Properties *p);

/**
 * Parsea una linea del archivo de configuracion
 * @param buffer linea leida del archivo
 * @param p estructura de configuracion
 * @return 1 si es valida, 0 en caso contrario.
 */
int parse_config(char *buffer, Properties *p);

/**
 * Carga y guarda los valores por defecto
 * @param proppath direccion del .prop
 * @param p estructura donde se guardan los valores
 */
void config_default(char *proppath, Properties *p);

/**
 * Guarda la configuracion actual en el archivo .prop
 * @param proppath path del archivo de configuracion
 * @param p estructura que contiene la configuracion actual
 */
void config_save(char *proppath, Properties *p);

/**
 * Crear un int en el heap
 * @param  numero a asignar
 * @return puntero al int creado
 */
int *new_integer(int n);

#endif	/* UTIL_H */

