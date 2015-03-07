/* 
 * File:   server.c
 * Author: santiago, jordan , guille
 *
 * Created on April 27, 2014, 8:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "paquetes.h"
#include "md5.h"
#include "thpool.h"
#include "util.h"
#include "entities.h"
#include "dbmanagement.h"
#include <signal.h>
#include <errno.h>
#include "server.h"
#include <fcntl.h>

int argn;
char **args;

Properties p = DEFAULT_PROP;
static volatile sig_atomic_t stop_flag = 0;
static volatile sig_atomic_t reload_flag = 0;

int main(int argc, char **argv) {
    struct sockaddr_in sin;
    struct sockaddr_in pin;
    int sock_descriptor;
    int address_size;
    int temp_sockfd;
    pid_t pid, sid;
    int on, ret;

    argn = argc;
    args = argv;
    stop_flag = 0;

    pid = fork();
    if (pid < 0) {
        escribirLog("Error en el fork", __func__, LOG_ERROR);
        exit(EXIT_FAILURE);
    }
    if (pid > 0) //en el padre, liberamos
        exit(EXIT_SUCCESS);
    //En el hijo...
    //Primero, iniciamos una nueva sesion
    if ((sid = setsid()) < 0) {
        escribirLog("Error al crear sesion", __func__, LOG_ERROR);
        exit(EXIT_FAILURE);
    }
    //A conitnuacion, hacemos de / el dir actual
    if ((chdir("/")) < 0) {
        escribirLog("Error en chdir", __func__, LOG_ERROR);
        exit(EXIT_FAILURE);
    }
    //restablecemos el modo de archivo
    umask(0);
    //Cerramos los descriptores de archivos innecesarios
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    //Felicidades, tiene un daemon
    
    install_handlers();
    while (!stop_flag) {
        loadConfig();
        reload_flag = 0;
        //Primero debemos definir el descriptor de socket
        sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
        on = 1;
        ret = setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
        if (sock_descriptor == -1 || ret == -1) {
            escribirLog("llamada a socket\n", __func__, LOG_ERROR);
            exit(EXIT_FAILURE);
        }
        //Entonces debemos rellenar los campos necesarios de la estructura 
        //sockaddr_in sin
        bzero(&sin, sizeof (sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(p.port); //utilizamos el puerto 8000

        //Ahora estamos preparados para unir el nuevo socket con el puerto
        if (bind(sock_descriptor, (struct sockaddr *) &sin, sizeof (sin)) == -1) {
            escribirLog("llamada a bind\n", __func__, LOG_ERROR);
            exit(EXIT_FAILURE);
        }

        //Para terminar, necesitamos escuchar en el nuevo socket en el 
        if (listen(sock_descriptor, p.threads) == -1) {
            escribirLog("llamada a listen\n", __func__, LOG_ERROR);
            exit(EXIT_FAILURE);
        }

        /*Creacion de pool de hilos*/
        thr_pool_t *pool = thr_pool_create(p.threads, p.threads, p.timeout, NULL);
        if (pool == NULL) {
            escribirLog("Creacion de threadpool\n", __func__, LOG_ERROR);
            exit(EXIT_FAILURE);
        }

        escribirLog("Servidor corriendo.\n", __func__, LOG_INF);

        while (!stop_flag && !reload_flag) {
            //Obtiene un socket temporal para administrar las peticiones cliente
            address_size = sizeof (pin);
            temp_sockfd = accept(sock_descriptor, (struct sockaddr *) &pin, &address_size);
            if (temp_sockfd == -1) {
                if (stop_flag || reload_flag) {
                    escribirLog("Se bloquean las conexiones\n", __func__, LOG_INF);
                    continue; //es normal que falle el accept
                } else {
                    escribirLog("llamada para aceptar\n", __func__, LOG_ERROR);
                    exit(EXIT_FAILURE);
                }
            }
            //Se envia la peticion a los hilos trabajadores
            if (thr_pool_queue(pool, handle_request, (void*) new_integer(temp_sockfd)) == -1)
                escribirLog("Encolar tarea\n", __func__, LOG_ERROR);
        }
        if (reload_flag)
            escribirLog("Recargando servidor\n", __func__, LOG_INF);
        thr_pool_wait(pool);
        thr_pool_destroy(pool);
        close(sock_descriptor);
    }
    escribirLog("Apagando servidor\n", __func__, LOG_INF);
    return 0;
}

/*Maneja una peticion del cliente*/
void *handle_request(void* arg) {
    int temp_sockfd = *((int*) arg);
    free(arg);
    struct paquete paquete = cerarPaquete(), paquete_servidor = cerarPaquete();
    struct paquete paqaux = cerarPaquete();
    char strPaq[16384], resp[16384];
    int len, salir = 0;
    int idLog, idTx = 1;

    PGconn *conn;
    while (salir == 0) {
        conn = connectDB();
        len = recibirPaquete(conn, strPaq, temp_sockfd, &paquete);
        if (len == -1) paquete.tipo = 0;
        sleep(1);
        if (paquete.tipo == 1) {
            time_t hora = time(NULL);
            char *fechaServer;
            convertirHora(hora, &fechaServer);
            if (user_auth(conn, paquete.id_ussd, paquete.pass)) {
                strcpy(paquete_servidor.id_menu, "principal");
                strcpy(paquete_servidor.menu, "Bienvenido al Sistema MPRSE:\n1. Encuesta\n2. Estadisticas\n");
                idTx = addTransaccion(conn, paquete.id_ussd, paquete.fecha,
                        fechaServer, TX_CURSO);
                paquete_servidor.idx = idTx;
                idLog = addLog(conn, paquete_servidor.idx, paquete.fecha, fechaServer, LOG_IN, strPaq);
            } else {
                idTx = addTransaccion(conn, paquete.id_ussd, paquete.fecha,
                        fechaServer, TX_FALLA);
                paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tAcceso denegado\n\n", idTx);
                if (cambiarEstadoTransaccion(conn, idTx, TX_FALLA) == -1)
                    escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                salir = 1;
            }
        } else if (paquete.tipo == 2) {
            //Paquete intermedio <fecha_hora>::<idx>::<id_menu>::<seleccion>
            if ((strcmp(paquete.id_menu, "principal")) == 0) {
                paquete_servidor.idx = paquete.idx;
                switch (paquete.seleccion) {
                    case 1:
                        generarMenuDepartamentos(conn, &paquete_servidor);
                        break;
                    case 2:
                        generarMenuEstadisticas(&paquete_servidor);
                        break;
                    default:
                        paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tSeleccion no Valida\n\n", paquete.idx);
                        if (cambiarEstadoTransaccion(conn, paquete.idx, TX_FALLA) == -1)
                            escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                        salir = 1;
                        break;
                }
            } else if ((strcmp(paquete.id_menu, "departamento")) == 0) {
                if (paquete.seleccion >= 1 && (generarMenuCiudades(conn,
                        paquete.seleccion, &paquete_servidor)) == 1)
                    paqaux.id_departamento = paquete.seleccion;
                else {
                    paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tSeleccion no Valida\n\n", paquete.idx);
                    if (cambiarEstadoTransaccion(conn, paquete.idx, TX_FALLA) == -1)
                        escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                    salir = 1;
                }
            } else if ((strcmp(paquete.id_menu, "ciudad")) == 0) {
                ptrDepartamento depaux;
                ptrCiudad ciudadaux;

                /*Comprobamos que la opción elegida por el usuario corresponda a una
                 *ciudad registrada en la base de datos antes de derivar al siguiente
                 *menú*/
                if (paquete.seleccion >= 1 && getDepartamentoByOrden(conn, (paqaux.id_departamento) - 1, &depaux) == 1
                        && getCiudadByOrden(conn, depaux->id, (paquete.seleccion) - 1, &ciudadaux) == 1) {
                    generarMenuEnfermedades(conn, &paquete_servidor);
                    paqaux.id_ciudad = paquete.seleccion;
                    free(ciudadaux);
                    free(depaux);
                }//en caso contrario generamos un mensaje de error
                else {
                    escribirLog("Se seleccionó una ciudad que no existe.\n", __func__, LOG_INF);
                    paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tSeleccion no Valida\n\n", paquete.idx);
                    if (cambiarEstadoTransaccion(conn, paquete.idx, TX_FALLA) == -1)
                        escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                    salir = 1;
                }
            } else if ((strcmp(paquete.id_menu, "enfermedad")) == 0) {
                ptrEnfermedad enfaux;
                if (paquete.seleccion >= 1 & getEnfermedadByOrden(conn, (paquete.seleccion) - 1, &enfaux) == 1) {
                    free(enfaux);
                    paqaux.id_enfermedad = paquete.seleccion;
                    paqaux.idx = paquete.idx;
                    almacenarRegistro(conn, paqaux);
                    cambiarEstadoTransaccion(conn, paqaux.idx, TX_FIN);
                    strcpy(paquete_servidor.id_menu, "fin");
                    strcpy(paquete_servidor.menu, "\n\t\tGracias Por utilizar Nuestro Sistema\n");
                } else {
                    escribirLog("Se seleccionó una enfermedad que no existe.\n", __func__, LOG_INF);
                    paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tSeleccion no Valida\n\n", paquete.idx);
                    if (cambiarEstadoTransaccion(conn, paquete.idx, TX_FALLA) == -1)
                        escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                }
                salir = 1;
            } else if ((strcmp(paquete.id_menu, "estadisticas")) == 0) {
                switch (paquete.seleccion) {
                    case 1:
                        generarInformeEnfermedades(conn, &paquete_servidor);
                        if (cambiarEstadoTransaccion(conn, paquete.idx, TX_CONSULTA) == -1)
                            escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                        salir = 1;
                        break;
                    case 2:
                        generarInformeDepartamentos(conn, &paquete_servidor);
                        if (cambiarEstadoTransaccion(conn, paquete.idx, TX_CONSULTA) == -1)
                            escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                        salir = 1;
                        break;
                    case 3:
                        generarInformeMeses(conn, &paquete_servidor);
                        if (cambiarEstadoTransaccion(conn, paquete.idx, TX_CONSULTA) == -1)
                            escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                        salir = 1;
                        break;
                    default:
                        paqueteError(conn, strPaq, &paquete_servidor, "\n\t\tSeleccion no Valida\n\n", paquete.idx);
                        if (cambiarEstadoTransaccion(conn, paquete.idx, TX_FALLA) == -1)
                            escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
                        salir = 1;
                        break;
                }
            } else {
                paqueteError(conn, strPaq, &paquete_servidor, "\n\t\t\tId Menu Incorrecto\n\n", paquete.idx);
                salir = 1;
            }
            time_t hora = time(NULL);
            char *fechaServer;
            convertirHora(hora, &fechaServer);
            addLog(conn, paquete.idx, paquete.fecha, fechaServer, LOG_IN, strPaq);
        } else if (paquete.tipo == 0) {
            /*
             *El tiempo de espera para recibir termino
             * el servidor envia un paquete de error avisandole que sobrepaso el tiempo 
             * de espera
             */
            paqueteError(conn, strPaq, &paquete_servidor, "\n\t\t\tTiempo de espera sobrepasado\n\n", idTx);
            if (cambiarEstadoTransaccion(conn, idTx, TX_FALLA) == -1)
                escribirLog("Error al cambiar estado de transaccion.\n", __func__, LOG_ERROR);
            salir = 1;
        } else {
            //El paquete recibido no es el correcto, enviamos un mensaje de error
            paqueteError(conn, strPaq, &paquete_servidor, "\n\t\t\tError en el paquete recibido\n\n", idTx);
            salir = 1;
        }
        prepararPaquete(&paquete_servidor, resp);
        //devuelve datos al cliente:
        enviarPaquete(conn, resp, temp_sockfd, paquete_servidor);
    }
    close(temp_sockfd);
    closeConnection(conn);
}

//TODO: quitar esta funcion de la version final

void agregarUsuario() {
    PGconn *conn = connectDB();
    puts("se conecto");
    char encriptado[41];
    md5(getpass("Clave: "), encriptado);

    int idUSSD = addUSSD(conn, encriptado);
    if (idUSSD != -1) {
        printf("Se agregó un cliente con ID %d\n", idUSSD);
    }
    closeConnection(conn);
}

int almacenarRegistro(PGconn *conn, struct paquete paquete) {
    int idTx = paquete.idx;
    int idCiudad, seleccionCiudad;
    int idDepto, seleccionDepto;
    int idEnf, seleccionEnf;
    ptrCiudad ciudad;
    ptrEnfermedad enf;
    ptrDepartamento dep;

    seleccionDepto = paquete.id_departamento;
    getDepartamentoByOrden(conn, seleccionDepto - 1, &dep);
    idDepto = dep->id;
    seleccionCiudad = paquete.id_ciudad;
    getCiudadByOrden(conn, idDepto, seleccionCiudad - 1, &ciudad);
    idCiudad = ciudad->id;
    seleccionEnf = paquete.id_enfermedad;
    getEnfermedadByOrden(conn, seleccionEnf - 1, &enf);
    idEnf = enf->id;

    addRegistro(conn, idTx, idCiudad, idEnf);
    return 0;
}

int paqueteError(PGconn *conn, char *original, struct paquete *pservidor, char* menu, int idTx) {
    time_t tiempo = time(NULL);
    char *fecha;
    *pservidor = cerarPaquete();
    pservidor->idx = idTx;
    convertirHora(tiempo, &fecha);
    strcpy(pservidor->fecha, fecha);
    strcpy(pservidor->id_menu, "error");
    strcpy(pservidor->menu, menu);
    return 0;
}

int recibirPaquete(PGconn *conn, char *paquete, int socket, struct paquete *est) {
    char buf[16384] = "";
    //recibe los datos del cliente
    settimeouts(socket, p.timeout);
    if (recv(socket, buf, 16384, 0) == -1) {
        escribirLog("llamada para recibir\n", __func__, LOG_ERROR);
        return -1;
    }
    strcpy(paquete, buf);

    //Metemos el paquete en el desempaquetador
    *est = identificadorPaquetesCliente(buf);
    return 0;
}

int enviarPaquete(PGconn *conn, char *paquete, int socket, struct paquete est) {
    int x = strlen(paquete) + 1;
    if (send(socket, paquete, x, 0) == -1) {
        escribirLog("llamada para enviar\n", __func__, LOG_ERROR);
        return -1;
    }
    //convertir hora
    time_t hora = time(NULL);
    char *fechaServer;
    convertirHora(hora, &fechaServer);
    int idLog = addLog(conn, est.idx, est.fecha, fechaServer, LOG_OUT, paquete);
    if (idLog == -1)
        return -1;
    return 0;
}

void install_handlers() {
    struct sigaction stop_action, reload_action;

    /*----------Handler para stop----------*/
    sigfillset(&stop_action.sa_mask);
    sigdelset(&stop_action.sa_mask, SIGTSTP);
    sigdelset(&stop_action.sa_mask, SIGINT);
    stop_action.sa_handler = stop_handler;
    stop_action.sa_flags = 0;
    sigaction(SIGTSTP, &stop_action, NULL);
    sigaction(SIGINT, &stop_action, NULL);

    /*---------Handler para reload---------*/
    sigfillset(&reload_action.sa_mask);
    sigdelset(&reload_action.sa_mask, SIGHUP);
    reload_action.sa_handler = reload_handler;
    reload_action.sa_flags = 0;
    sigaction(SIGHUP, &reload_action, NULL);
}

void stop_handler() {
    if (stop_flag) return; //ya se levanto la bandera
    stop_flag = 1;
}

void reload_handler() {
    if (reload_flag) return;
    reload_flag = 1;
}

int user_auth(PGconn *conn, unsigned int user, char *pass) {
    ptrUssd ussd;

    if (getUssdById(conn, user, &ussd) == 1) {
        free(ussd);
        return md5_compare(getpassword(conn, user), pass);
    } else
        return 0;
}

int generarMenuCiudades(PGconn *conn, unsigned int orden, struct paquete * paq) {
    ptrCiudad *ciudades;
    ptrDepartamento departamento;
    char buf[256];
    char var[100];
    int i, idDepto;

    if (getDepartamentoByOrden(conn, orden - 1, &departamento) == 0) {
        escribirLog("Departamento seleccionado no existente.\n", __func__, LOG_INF);
        return 0;
    }

    idDepto = departamento->id;
    int cant = getCiudades(conn, idDepto, &ciudades);
    sprintf(buf, "Seleccione la ciudad:\n");
    for (i = 0; i < cant; i++) {
        sprintf(var, "%d- %s\n", i + 1, ciudades[i]->nombre);
        strcat(buf, var);
    }
    strcpy(paq->id_menu, "ciudad");
    strcpy(paq->menu, buf);
    return 1;
}

void generarMenuDepartamentos(PGconn *conn, struct paquete * paq) {
    ptrDepartamento *departamentos;
    char buf[256];
    char var[100];
    int i;

    int cant = getDepartamentos(conn, &departamentos);
    sprintf(buf, "Seleccione el departamento:\n");

    for (i = 0; i < cant; i++) {
        sprintf(var, "%d- %s\n", i + 1, departamentos[i]->nombre);
        strcat(buf, var);
    }
    strcpy(paq->id_menu, "departamento");
    strcpy(paq->menu, buf);
}

void generarMenuEstadisticas(struct paquete * paq) {
    char buf[256];
    char var[100];

    sprintf(buf, "Seleccione la estadistica deseada:\n");

    sprintf(var, "1- Principales enfermedades registadas\n");
    strcat(buf, var);

    sprintf(var, "2- Principales departamentos afectados\n");
    strcat(buf, var);

    sprintf(var, "3- Principales meses de envio de respuestas\n");
    strcat(buf, var);

    strcpy(paq->id_menu, "estadisticas");
    strcpy(paq->menu, buf);
}

int generarMenuEnfermedades(PGconn *conn, struct paquete * paq) {
    ptrEnfermedad *enfermedades;
    char buf[256];
    char var[100];
    int i;

    int cant = getEnfermedades(conn, &enfermedades);
    sprintf(buf, "Seleccione la enfermedad:\n");
    for (i = 0; i < cant; i++) {
        sprintf(var, "%d- %s\n", i + 1, enfermedades[i]->nombre);
        strcat(buf, var);
    }

    strcpy(paq->id_menu, "enfermedad");
    strcpy(paq->menu, buf);
}

void generarInformeEnfermedades(PGconn *conn, struct paquete * paq) {
    ptrTopEnfermedades *enfermedades;
    char buf[1024];
    char var[256];
    int i, cantTotal = 0;
    float porcentaje = 0;
    int cant = getTopEnfermedades(conn, &enfermedades);
    sprintf(buf, "Enfermedades más reportadas:\n");
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    sprintf(var, "|%-20s|\t%-20s|\t%-20s|\n", "Enfermedad", "Cantidad", "Porcentaje");
    strcat(buf, var);
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);

    for (i = 0; i < cant; i++)
        cantTotal = cantTotal + enfermedades[i]->cantidad;

    for (i = 0; i < cant; i++) {
        porcentaje = (float) (enfermedades[i]->cantidad * 100) / cantTotal;
        sprintf(var, "|%20s|\t%20d|\t%20.2f|\n", (enfermedades[i]->enfermedad)->nombre,
                enfermedades[i]->cantidad, porcentaje);
        strcat(buf, var);
    }
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    strcpy(paq->id_menu, "estadistica2");
    strcpy(paq->menu, buf);
}

void generarInformeDepartamentos(PGconn *conn, struct paquete * paq) {
    ptrTopDepartamentos *departamentos;
    char buf[1024];
    char var[256];
    int i;
    int cantTotal = 0;
    float porcentaje = 0;

    int cant = getTopDepartamentos(conn, &departamentos);
    sprintf(buf, "Departamentos más afectados:\n");
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    sprintf(var, "|%-20s|\t%-20s|\t%-20s|\n", "Departamento", "Cantidad", "Porcentaje");
    strcat(buf, var);
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);

    for (i = 0; i < cant; i++)
        cantTotal = cantTotal + departamentos[i]->cantidad;

    for (i = 0; i < cant; i++) {
        porcentaje = (float) (departamentos[i]->cantidad * 100) / cantTotal;
        sprintf(var, "|%20s|\t%20d|\t%20.2f|\n", (departamentos[i]->departamento)->nombre,
                departamentos[i]->cantidad, porcentaje);
        strcat(buf, var);
    }
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    strcpy(paq->id_menu, "estadistica3");
    strcpy(paq->menu, buf);
}

void generarInformeMeses(PGconn *conn, struct paquete * paq) {
    ptrTopMeses *meses;
    char buf[1024];
    char var[256];
    int i;
    int cantTotal = 0;
    float porcentaje = 0;

    int cant = getTopMeses(conn, &meses);
    sprintf(buf, "Meses con mayor cantidad de reportes:\n");

    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    sprintf(var, "|%-20s|\t%-20s|\t%-20s|\n", "Mes", "Cantidad", "Porcentaje");
    strcat(buf, var);
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);

    for (i = 0; i < cant; i++)
        cantTotal = cantTotal + meses[i]->cantidad;

    for (i = 0; i < cant; i++) {
        porcentaje = (float) (meses[i]->cantidad * 100) / cantTotal;

        sprintf(var, "|%20s|\t%20d|\t%20.2f|\n", meses[i]->mes,
                meses[i]->cantidad, porcentaje);
        strcat(buf, var);
    }
    sprintf(var, "---------------------------------------------------------------------\n");
    strcat(buf, var);
    strcpy(paq->id_menu, "estadistica3");
    strcpy(paq->menu, buf);
}

void loadConfig() {
    char path[512];
    switch (argn) {
        case 2:
            strcpy(path, args[1]);
            break;
        case 1:
            sprintf(path, "%s.prop", args[0]);
            break;
        default:
            perror("Argumentos invalidos para el servidor.\n");
            exit(EXIT_FAILURE);
    }
    FILE *fptr;
    if ((fptr = fopen(path, "r")) == NULL) {
        switch (errno) {
            case ENOENT:
                escribirLog("No existe el archivo de conf\n", __func__, LOG_INF);
                config_default(path, &p);
                return;
            default:
                escribirLog("Error inesperado al cargar configuracion\n", __func__, LOG_ERROR);
                exit(EXIT_FAILURE);
        }
    }
    fclose(fptr);
    config_read(path, &p);
}

