/* 
 * File:   paquetes.h
 * Author: santiago
 *
 * Created on May 2, 2014, 3:03 PM
 */



#ifndef PAQUETES_H
#define	PAQUETES_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct paquete {
        char fecha [14]; //fecha: la fecha que fue enviado el paquete
        int idx; //identificador de comunicacion
        int id_ussd; //identificador del cliente
        char pass [41]; // hash de la contrasenha
        int id_departamento; //id del departamento en el menu
        int id_ciudad; //id de la ciudad en el menu
        int tipo; //tipo de paquete
        int id_enfermedad; //id de la enfermedad
        int seleccion; //seleccion del cliente
        char menu [1024]; //cadena para el menu
        char id_menu[12]; //ahora el id es char
    };
#define PAQUETE_VACIO  {"",0,0,"",0,0,0,0,0,"",""};

    /**
     * @brief Parsea un paquete recibido
     *  Parsea un paquete recibido y devuelve un struct paquete
     *   con campos asignados.
     * @param paquete: puntero al paquete recibido
     * @return struct paquete con campos asignados
     **/
    struct paquete identificadorPaquetesCliente(char *paquete);

    /**
     * @brief Parsea un paquete recibido
     *  Parsea un paquete recibido y devuelve un struct paquete
     *    con campos asignados.
     * @param paquete: puntero al paquete recibido
     * @return struct paquete con campos asignados
     **/
    struct paquete identificadorPaquetesServidor(char *paquete);
    /**
     * @brief Crea un paquete para el envio
     *  Parsea un paquete recibido y devuelve un struct paquete
     *   con campos asignados.
     * @param paquete: puntero al paquete recibido
     * @return struct paquete con campos asignados
     **/
    char* prepararPaquete(struct paquete *packet, char *buf);
     /**
     * @brief Devuelve los datos del paquete recibido.
     * @param aux: puntero a la cadena de caracteres que recibe del recv 
     * @return un dato del paquete, para quitar los demas datos hay que 
      * hacer varias llamadas
     **/
    char* divisorDeEstructuras(char* aux);
     /**
     * @brief Cuenta la cantidad de cmpos que tiene la cadena de caracteres recibido
     * @param paquete: puntero a la cadena de caracteres aux
     * @return entero que representa la cantidad de datos que tiene el paquete
     **/
    int contador(char* aux);
    /**
     * @brief Pone una estructura de paquete a cero
     * @param 
     * @return una estructura de paquete todo a cero
     **/
    struct paquete cerarPaquete();



#ifdef	__cplusplus
}
#endif

#endif	/* PAQUETES_H */