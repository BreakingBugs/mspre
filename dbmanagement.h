/* 
 * File:   dbmanagement.h
 * Author: ascend
 *
 * Created on 28 de abril de 2014, 09:43 AM
 */

#ifndef DBMANAGEMENT_H
#define	DBMANAGEMENT_H
#define LOG_IN 0
#define LOG_OUT 1
#define TX_CURSO 0
#define TX_FIN 1
#define TX_FALLA 2
#define TX_CONSULTA 3
#include "libpq-fe.h" //Base de datos PostgreSQL
#include "entities.h"

PGconn *connectDB();
void closeConnection(PGconn *conn);
char *getpassword(PGconn *conn, unsigned int idUssd);
int getCiudades(PGconn *conn, unsigned int dpto, ptrCiudad **ciudades);
int getDepartamentos(PGconn *conn, ptrDepartamento **departamentos);
int getEnfermedades(PGconn *conn, ptrEnfermedad **enfermedades);
int getRegistros(PGconn *conn, ptrRegistro **registros);
int getTransacciones(PGconn *conn, ptrTransaccion **transacciones);
int getLogs(PGconn *conn, ptrLog **logs);
int getCiudadById(PGconn *conn, unsigned int id, ptrCiudad *ciudad);
int getDepartamentoById(PGconn *conn, unsigned int id, ptrDepartamento *departamento);
int getEnfermedadById(PGconn *conn, unsigned int id, ptrEnfermedad *enfermedad);
int getTransaccionById(PGconn *conn, unsigned int id, ptrTransaccion *transaccion);
int getLogById(PGconn *conn, unsigned int id, ptrLog *log);
int getRegistroById(PGconn *conn, unsigned int id, ptrRegistro *registro);
int getUssdById(PGconn *conn, unsigned int id, ptrUssd *ussd);
int getUssds(PGconn *conn, ptrUssd **ussds);
int addCiudad(PGconn *conn, char *nombre, unsigned int idDepartamento);
int addDepartamento(PGconn *conn, char *nombre);
int addEnfermedad(PGconn *conn, char *nombre);
int addUSSD(PGconn *conn, char *hash);
int addTransaccion(PGconn *conn, unsigned int idUSSD, char *fechaClient,
        char *fechaServer, unsigned short estado);
int addRegistro(PGconn *conn, unsigned int idTransaccion, unsigned int idCiudad,
        unsigned int idEnfermedad);
int addLog(PGconn *conn, unsigned int idTransaccion, char *fechaCliente, 
        char *fechaServidor, unsigned int inOut, char *paquete);
int cambiarEstadoTransaccion(PGconn *conn, unsigned int id, unsigned short estado);
int getTopDepartamentos(PGconn *conn, ptrTopDepartamentos **departamentos);
int getTopEnfermedades(PGconn *conn, ptrTopEnfermedades **enfermedades);
#endif	/* DBMANAGEMENT_H */

