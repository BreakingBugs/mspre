#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include "util.h"

char logpath[MAX_PATH_LENGTH], logfile[MAX_FILENAME_LENGTH];

void settimeouts(int sock, int secs) {
    struct timeval tv;
    tv.tv_sec = secs;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
        escribirLog("Error al poner timer\n", __func__, LOG_ERROR);
    }
}

/* Función que convierte la fecha y hora que recibe de un time_t
 * a una cadena que escribe en el puntero al que hace referencia 'resultado'
 */
/**
 * Convierte la fecha y hora que recibe de un time_t a una cadena que escribe
 * en el puntero al que hace referencia resultado
 * @param hora información a convertir
 * @param resultado string donde se almacena la salida
 */
void convertirHora(time_t hora, char **resultado) {
    struct tm *tmPtr;
    char cadena[15];
    setlocale(LC_TIME, "");
    tmPtr = localtime(&hora);
    strftime(cadena, 15, "%Y%m%d%H%M%S", tmPtr);
    //TODO: memory leak, puede ser que no sea muy importante
    *resultado = malloc(sizeof (char)*15);
    strncpy(*resultado, cadena, 15);
}

/**
 * carga la información especificada en el archivo de configuración
 * para el log del demonio
 * @param p estructura que contiene la configuracion actual
 */
int load_logpath(Properties p) {
    if(p.logpath!=NULL && p.logfile!=NULL) {
        strcpy(logpath, p.logpath);
        strcpy(logfile, p.logfile);
    }
}



/**
 * Función que ecribe en el log de eventos del demonio los eventos ocurridos.
 * Mediante 'tipo' se especifica el tipo de mensaje a agregar,y el mensaje se
 * especifica en el argumento 'mensaje'.
 * La función devuelve 0 si tuvo éxito o -1 en caso contrario.
 * @param mensaje cadena a escrbir en el log
 * @param funcion se especifica dónde se produjo el evento
 * @param identificador de tipo de mensaje a guardar
 */
int escribirLog(char *mensaje, const char *funcion, int tipo) {
    int fd;
    int  i;
    short use_sys = 0;
    char ruta[1024];
    //si no está cargada la configuración escribimos en el log del sistema
    if(strcmp(logpath,"")==0 || strcmp(logfile,"")==0) 
        use_sys = 1;
    
    if(!use_sys) {
        sprintf(ruta, "%s%s", logpath, logfile);
        char buffer[1024];

        fd = open(ruta, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd >= 0) {
            //obtenemos fecha y hora actual
            struct tm *tmPtr;
            char cadena[20];
            time_t hora = time(NULL);

            setlocale(LC_TIME, "");
            tmPtr = localtime(&hora);
            strftime(cadena, 20, "%x %X", tmPtr);

            switch (tipo) {
                case LOG_ERROR:
                    sprintf(buffer, "%s ERROR %s: %s", cadena, funcion, mensaje);
                    write(fd, buffer, strlen(buffer));
                    break;
                default:
                    sprintf(buffer, "%s INFO %s: %s", cadena, funcion, mensaje);
                    write(fd, buffer, strlen(buffer));
                    break;
            }
            if (close(fd) < 0) {
                openlog("mspre", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
                syslog(LOG_ERR, "No se pudo cerrar el archivo de log.");
                closelog();
            } else
                return 0;
        } else {
            openlog("mspre", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
            syslog(LOG_ERR, "No se pudo abrir el archivo de log.");
            closelog();
            return -1;
        }
    }
    else {
        write_sys(mensaje, funcion, tipo);
    }
}

/**
 * Escribe en el log de sistema cuando no está disponible el log propio.
 * @param mensaje cadena a escrbir en el log
 * @param funcion se especifica dónde se produjo el evento
 * @param identificador de tipo de mensaje a guardar
 */
void write_sys(char *mensaje, const char *funcion, int tipo) {
    openlog("mspre", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    switch(tipo) {
        case LOG_ERROR:
            syslog(LOG_ERR, "%s: %s", funcion, mensaje);
            break;
        default:
            syslog(LOG_INFO, "%s: %s", funcion, mensaje);
            break;
    }
    closelog();
}

/* Función que crea un int en el heap y le asigna el valor de 'n'
 * retorna el puntero al int creado
 */
int *new_integer(int n) {
    int *i = malloc(sizeof (int));
    if (i == NULL)
        escribirLog("Error al crear un int en el heap", __func__, LOG_ERROR);
    *i = n;
    return i;
}

/**
 * Guarda la configuracion actual en el archivo .prop
 * @param proppath path del archivo de configuracion
 * @param p estructura que contiene la configuracion actual
 */
void config_save(char *proppath, Properties *p) {
    FILE *fptr = fopen(proppath, "w");
    if (fptr == NULL) {
        if (errno == EACCES)
            escribirLog("Error, no tiene permisos para guardar la configuracion\n", __func__, LOG_ERROR);
        else
            escribirLog("Error inesperado al leer el archivo de configuracion\n", __func__, LOG_ERROR);
        exit(EXIT_FAILURE);
    }
    fputs("#------------Archivo de configuracion de mspred----------------\n", fptr);
    fputs("# Cada parametro, sin importar el orden,\n", fptr);
    fputs("# debe estar en una linea siguiendo la sintaxis: ", fptr);
    fputs("# parametro=valor\n", fptr);
    fputs("# Los parametros son: port, threads, timeout, logpath y logfile\n", fptr);
    fprintf(fptr, "port=%d\nthreads=%d\ntimeout=%d\nlogpath=%s\nlogfile=%s\n",
            p->port, p->threads, p->timeout, p->logpath, p->logfile);
    fclose(fptr);
    char var[64];
    sprintf(var, "Configuracion guardada (%d;%d;%d)\n",
            p->port, p->threads, p->timeout);
    escribirLog(var, __func__, LOG_INF);
}

/**
 * Carga y guarda los valores por defecto
 * @param proppath direccion del .prop
 * @param p estructura donde se guardan los valores
 */
void config_default(char *proppath, Properties *p) {
    Properties def = DEFAULT_PROP;
    memcpy(p, &def, sizeof (Properties));
    escribirLog("Cargar configuracion por defecto\n", __func__, LOG_INF);
    config_save(proppath, p);
}

/**
 * Funcion que parsea una linea del archivo de configuracion
 * Retorna 1 si es valida, 0 en caso contrario.
 */
int parse_config(char *buffer, Properties *p) {
    char *argname = buffer, *arg = strchr(buffer, '=');
    if (buffer[0] == '#') return 0; //Es una linea de comentario
    if (arg == NULL) return 1; //si no esta el separador =
    buffer[strlen(buffer) - 1] = '\0'; //elimina el newline
    *(arg++) = '\0'; //se separan los strings
    if (strcmp(argname, "port") == 0)
        p->port = atoi(arg);
    else if (strcmp(argname, "threads") == 0)
        p->threads = atoi(arg);
    else if (strcmp(argname, "timeout") == 0)
        p->timeout = atoi(arg);
    else if (strcmp(argname, "logpath") == 0)
        strcpy(p->logpath, arg);
    else if (strcmp(argname, "logfile") == 0)
        strcpy(p->logfile, arg);
    else {
        char var[42];
        sprintf(var, "Parametro %s invalido en .prop\n", argname);
        escribirLog(var, __func__, LOG_INF);
    }
    return 0;
}

/**
 * Funcion que lee el archivo de configuracion .prop
 * y lo parsea, guardando los valores en una estructura
 * Properties.
 */
void config_read(char *proppath, Properties *p) {
    FILE *fptr = fopen(proppath, "r");
    if (fptr == NULL) {
        escribirLog("Error al leer configuracion\n", __func__, LOG_ERROR);
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_PATH_LENGTH];
    int line = 1;
    while (fgets(buffer, MAX_PATH_LENGTH, fptr) != NULL) {
        if (parse_config(buffer, p)) {
            sprintf(buffer, "Error en el archivo de configuracion L%d\n", line);
            escribirLog(buffer, __func__, LOG_ERROR);
            exit(EXIT_FAILURE);
        }
        line++;
    }
    load_logpath(*p);
    sprintf(buffer, "Configuracion cargada (%d;%d;%d)\n",
            p->port, p->threads, p->timeout);
    escribirLog(buffer, __func__, LOG_INF);
    fclose(fptr);
}