/*
 *Autor: santiago
 *Prueba con strings al recibir distintos paquetes especiales
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "paquetes.h"
#include "util.h"
/*
 *Paquete Cliente: contiene: 
 *<fecha_hora>::<idx>::<id_ussd>::<id_menu>::<seleccion>
 *Paquete Servidor contiene:
 *<fecha_hora>::<idx>::<id_ussd>::<id_menu>::<longitud_menu>::<menu>
 *Paquete Identificador contiene:
 *<fecha_hora>::<id_ussd>::<pass>
 *Paquete final contiene:
 *<fecha_hora>::<idx>::<id_ussd>::<id_departamento>::<id_ciudad>::<id_enfermedad>::<pass>
 */
/*#define PAQUETE_CLIENTE "20140421144000::0::santiago::1::3"
#define PAQUETE_SERVIDOR "20140425151515::0::santiago::0::40::Bienvenido al programa\n1.Ver\n2.Registrar\n"
#define PAQUETE_IDENTIFICADOR "20140423152000::santiago::holasant"
#define PAQUETE_FINAL "20140428124523::0::santiago::1::2::1::holasant"*/
/*Estructura para soportar el paquete 
 *Para no tener que utilizar varios paquetes utilizamos solamente una estrutura
 *con un identificador tipo para saber a que tipo de paquete corresponde
 * utilizamos las mismas variables en algunos casos para no tener exceso de variables 
 */

/*IdentificadorPaquetesCliente: identifica el tipo de paquete
 *Recibe como argumento el paquete que es una cadena de caracteres
 *y devuelve un struct de paquete que indica el tipo de paquete cliente
 *junto con los datos del paquete
 *Se tiene en cuneta las siguientes especificaciones:
 * Para el servidor
 * Paquete Cliente = 2;
 * Paquete identificador = 1;
 * Paquete final = 3;
 */

struct paquete identificadorPaquetesCliente(char * charPaquete) {
    int cant;
    char s[] = "::", * ptr, var[256];
    //estructura donde se amacenara los datos
    struct paquete packet = PAQUETE_VACIO;
    //la funcion divisorDeEstructuras modifica la cadena recibida por tanto copiamos
    // en otra variable los datos
    strcpy(var, charPaquete);
    cant = contador(charPaquete);
    //si la cantidad de datos es 4  se sabe que es el paquete deseado
    if (cant == 3) {
        //<fecha_hora>::<id_ussd>::<pass>
        packet.tipo = 1;
        ptr = divisorDeEstructuras(var); //fecha_hora
        strcpy(packet.fecha, ptr);
        ptr = divisorDeEstructuras(var); //id_ussd
        packet.id_ussd = atoi(ptr);
        ptr = divisorDeEstructuras(var); //pass
        strcpy(packet.pass, ptr);
    } else if (cant == 4) {
        //<fecha_hora>::<idx>::<id_menu>::<seleccion>
        packet.tipo = 2;
        ptr = divisorDeEstructuras(var);
        strcpy(packet.fecha, ptr);
        ptr = divisorDeEstructuras(var);
        packet.idx = atoi(ptr);
        ptr = divisorDeEstructuras(var);
        strcpy(packet.id_menu, ptr);
        ptr = divisorDeEstructuras(var);
        packet.seleccion = atoi(ptr);
    } else {
        escribirLog("Error en el paquete recibido\n", __func__, LOG_ERROR);
    }
    return packet;
}

/*
 * identificadorPaquetesServidor
 *Para el cliente, al recibir un paquete necesita saber 
 * que tipo de paquete es para poder manipularlo
 * Paquete servidor registro = 1;
 * <fecha_hora>::<idx>::<id_ussd>::<id_menu>::<longitud_menu>::<menu>
 * Paquete servidor ver = 2;(si llega a ser diferente
 *  el paquete de ver estadisticas)
 */
struct paquete identificadorPaquetesServidor(char *charPaquete) {
    int cant;
    char s[] = "::", * ptr, var[256];
    //estructura donde se amacenara los datos
    struct paquete packet = PAQUETE_VACIO;

    strcpy(var, charPaquete);
    cant = contador(charPaquete);

    //si la cantidad de datos es 4  el tipo de paquete es  cliente
    if (cant == 4) {
        //<fecha_hora>::<idx>::<id_menu>::<menu>
        packet.tipo = 2;
        ptr = divisorDeEstructuras(var);
        strcpy(packet.fecha, ptr);
        ptr = divisorDeEstructuras(var);
        packet.idx = atoi(ptr);
        ptr = divisorDeEstructuras(var);
        strcpy(packet.id_menu, ptr);
        ptr = divisorDeEstructuras(var);
        strcpy(packet.menu, ptr);
    } else {
        strcpy(packet.id_menu, "fin");
        strcpy(packet.menu, "Error en el paquete recibido");
    }
    return packet;
}

/*Crea un String de los campos concatenados del paquete */
char* prepararPaquete(struct paquete *packet, char *buf) {
    time_t tiempo = time(0);
    struct tm *tiempolocal;
    char var[1024];

    //TODO: ver si no se esta reescribiendo la fecha en el packet
    tiempolocal = localtime(&tiempo);
    strftime(packet->fecha, 128, "%Y%m%d%H%M%S", tiempolocal);
    strcpy(buf, packet->fecha);
    //    strcat(buf, "::");

    if (packet->idx != 0) {
        sprintf(var, "::%d", packet->idx);
        strcat(buf, var);
    }
    if (packet->id_ussd != 0) {
        sprintf(var, "::%d", packet->id_ussd);
        strcat(buf, var);
    }
    if (strcmp(packet->pass, "NULL") != 0) {
        strcpy(var, packet->pass);
        strcat(buf, "::");
        strcat(buf, var);
    }
    if (packet->id_departamento != 0) {
        sprintf(var, "::%d", packet->id_departamento);
        strcat(buf, var);
    }
    if (packet->id_ciudad != 0) {
        sprintf(var, "::%d", packet->id_ciudad);
        strcat(buf, var);
    }
    if (packet->id_enfermedad != 0) {
        sprintf(var, "::%d", packet->id_enfermedad);
        strcat(buf, var);
    }
    if (strcmp(packet->id_menu, "NULL") != 0) {
        strcpy(var, packet->id_menu);
        strcat(buf, "::");
        strcat(buf, var);
    }
    if (strcmp(packet->menu, "NULL") != 0) {
        strcat(buf, "::");
        strcat(buf, packet->menu);
    }
    if (packet->seleccion != 0) {
        sprintf(var, "::%d", packet->seleccion);
        strcat(buf, var);
    }
    return buf;
}

char* divisorDeEstructuras(char* aux) {
    char aux2[256];
    char* resp = (char *) malloc(sizeof (char)*256);
    int len, k, l = 0, i, p = 0;
    len = strlen(aux);
    for (i = 0; i <= len; i++) {
        if (aux[i] == ':' && aux[i + 1] == ':') {
            for (k = i + 2; k <= len; k++)
                aux2[l++] = aux[k];
            aux2[l] = '\0';
            i = len + 1;
        } else
            resp[p++] = aux[i];
    }
    resp[p] = '\0';
    strcpy(aux, aux2);
    return resp;
}

int contador(char* aux) {
    int k = 1;
    char *pos = aux;
    while ((pos = strstr(pos + 2, "::")) != NULL)
        k++;
    return k;
}

struct paquete cerarPaquete() {
    struct paquete paquete;
    strcpy(paquete.fecha, "NULL");
    paquete.id_ciudad = 0;
    paquete.id_departamento = 0;
    paquete.id_enfermedad = 0;
    strcpy(paquete.id_menu, "NULL");
    paquete.id_ussd = 0;
    paquete.idx = 0;
    strcpy(paquete.menu, "NULL");
    strcpy(paquete.pass, "NULL");
    paquete.seleccion = 0;
    paquete.tipo = 0;
    return paquete;
}