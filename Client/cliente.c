/* 
 * File:   cliente.c
 * Author: santiago
 *
 * Created on April 27, 2014, 8:52 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include "paquetes.h"
#include "md5.h"

char HOST_NAME[32] = "127.0.0.1"; //localhost
int PORT = 8000;

int getInfoUssd(int *id, char **clave);

void main(int argc, char *argv[]) {
    if (argc == 3) {
        strcpy(HOST_NAME, argv[1]);
        PORT = atoi(argv[2]);
    }
    char buf[8192];
    char message[256], m2[128], fin[4] = "";
    int socket_descriptor, x;
    int idUssd;
    char *claveOrig, hash[41];
    struct sockaddr_in pin;
    struct hostent *server_host_name;
    struct paquete paquete_cliente = {"", 0, 1, "", 0, 0, 0, 0, 0, "NULL", "NULL"};
    time_t tiempo = time(0);
    struct tm *tiempolocal;

    if ((server_host_name = gethostbyname(HOST_NAME)) == 0)
        handle_error("error al resolver el host local\n");

    if (getInfoUssd(&idUssd, &claveOrig) == -1) {
        puts("Error al leer el archivo.");
        exit(EXIT_FAILURE);
    }
    md5(claveOrig, hash);
    paquete_cliente.id_ussd = idUssd;
    strcpy(paquete_cliente.pass, hash);
    //ahora que tenemos esa informacion podemos rellenar los datos de la
    // estructura struct sockaddr_in pin:
    bzero(&pin, sizeof (pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = htonl(INADDR_ANY);
    pin.sin_addr.s_addr = ((struct in_addr *) (server_host_name->h_addr))->s_addr;
    pin.sin_port = htons(PORT);

    //estamos ppreparados para construir una conexion socket con el host
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        handle_error("error al abrir el socket\n");

    //Para terminar, podemos conectar con el host utilizando este socket
    if (connect(socket_descriptor, (void *) &pin, sizeof (pin)) == -1)
        handle_error("Error de conexion con el socket\n");

    tiempolocal = localtime(&tiempo);
    strftime(paquete_cliente.fecha, 128, "%Y%m%d%H%M%S", tiempolocal);

    prepararPaquete(&paquete_cliente, message);
    printf("\t\t\tPlataforma USSD en ejecucion...\n");

    while (1) {
        //<fecha_hora>::<idx>::<id_menu>::<menu>
        //Si el servidor esta ocupado esta llamada podria bloquearse un rato
        x = strlen(message) + 1;
        if (send(socket_descriptor, message, x, 0) == -1)
            handle_error("Error en send\n");

        //Recibimos la respuesta del servidor
        if (recv(socket_descriptor, buf, 8192, 0) == -1)
            handle_error("Error al recibir la respuesta del servidor\n");
        struct paquete paquete = identificadorPaquetesServidor(buf);
        paquete_cliente = cerarPaquete();
        //Imprimimos el menu
        system("clear");
        //Esperamos respuesta del cliente
        printf("\n\n%s\n", paquete.menu);
        if ((strcmp(paquete.id_menu, "fin") == 0) || (strcmp(paquete.id_menu, "error") == 0)) {
            break;
        } else if ((strcmp(paquete.id_menu, "estadistica1") == 0)
                || (strcmp(paquete.id_menu, "estadistica2") == 0)
                || (strcmp(paquete.id_menu, "estadistica3") == 0)) {
            break;
        } else {
            scanf("%d", &paquete_cliente.seleccion);
            system("clear");
            //Crear paquete que asigne a la estructura los datos mientras tanto se queda afuera
            paquete_cliente.idx = paquete.idx;
            strcpy(paquete_cliente.id_menu, paquete.id_menu);
            prepararPaquete(&paquete_cliente, message);
            agregarSeleccion(&paquete_cliente, message);

        }
    }


    close(socket_descriptor);
}

int getInfoUssd(int *id, char **clave) {
    FILE *f = NULL;
    int uId;
    char uClave[100];
    //Actualmente el archivo debe encontrarse en el mismo directorio que el ejecutable
    f = fopen("auth.dat", "r");
    if (f != NULL) {
        fscanf(f, "id_ussd=%d\nclave=%s", &uId, uClave);
        fclose(f);
        *id = uId;
        *clave = malloc(sizeof (char)*100);
        strncpy(*clave, uClave, 100);
        return 0;
    }
    return -1;
}