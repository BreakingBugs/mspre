/* 
 * File:   entities.h
 * Author: ascend
 *
 * Created on 29 de abril de 2014, 11:07 AM
 */

#ifndef ENTITIES_H
#define	ENTITIES_H

typedef struct ciudad {
    unsigned int id;
    char nombre[31];
    unsigned int idDepartamento;
} Ciudad;

typedef Ciudad *ptrCiudad;


typedef struct departamento {
    unsigned int id;
    char nombre[21];
} Departamento;

typedef Departamento *ptrDepartamento;


typedef struct enfermedad {
    unsigned int id;
    char nombre[51];
} Enfermedad;

typedef Enfermedad *ptrEnfermedad;

typedef struct topEnfermedades {
    ptrEnfermedad enfermedad;
    int cantidad;
} TopEnfermedades;

typedef TopEnfermedades *ptrTopEnfermedades;

typedef struct topDepartamentos {
    ptrDepartamento departamento;
    int cantidad;
} TopDepartamentos;

typedef TopDepartamentos *ptrTopDepartamentos;

typedef struct topMeses {
    char mes[7];
    int cantidad;
} TopMeses;

typedef TopMeses *ptrTopMeses;

typedef struct ussd {
    unsigned int id;
    char hash[41];
} Ussd;

typedef Ussd *ptrUssd;


typedef struct transaccion {
    unsigned int id;
    unsigned int idUssd;
    char fechaClient[15];
    char fechaServer[15];
    unsigned short estado;
} Transaccion;

typedef Transaccion *ptrTransaccion;

typedef struct log {
    unsigned int id;
    unsigned int idTransaccion;
    unsigned int inOut;
    char fechaCliente[15];
    char fechaServidor[15];
    char paquete[4097];
} Log;

typedef Log *ptrLog;

typedef struct registro {
    unsigned int id;
    unsigned int idCiudad;
    unsigned int idEnfermedad;
    unsigned int idTransaccion;
} Registro;

typedef Registro *ptrRegistro;
#endif	/* ENTITIES_H */


