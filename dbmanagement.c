#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "util.h"
#include "dbmanagement.h"


//Manejo de base de datos


/*realiza la conexión a la base de datos.
 Devuelve un puntero a PGconn que contiene los parámetros de la conexión
 establecida.*/
PGconn *connectDB() {
    PGconn *conn = NULL;

    conn = PQconnectdb("user=postgres password=root dbname=mspre hostaddr=127.0.0.1 port=5432");

    //chequeamos si se realizó la conexión
    if (PQstatus(conn) != CONNECTION_OK) {
        escribirLog("Fallo la conexion a la base de datos.\n", __func__, LOG_ERROR);
        return NULL;
    }

    return conn;
}

/*cierra la conexión con la base de datos apuntada por 'conn'*/
void closeConnection(PGconn *conn) {
    PQfinish(conn);
}

char *getpassword(PGconn *conn, unsigned int idUssd) {
    char *valor;
    ptrUssd ussd;
    
    if(getUssdById(conn, idUssd, &ussd)!=0) {
        valor = malloc(sizeof(char)*41);
        strcpy(valor, ussd->hash);
        
        free(ussd);
        return valor;
    }
    
    char buffer[100];
    sprintf(buffer,"No se pudo obtener la información del ussd con id: %d.\n",
            idUssd);
    escribirLog(buffer, __func__, LOG_ERROR);
    return NULL;
}

/* obtiene todas las ciudades encontradas en la base de datos
 * correspondientes al departamento con el ID especificado y las devuelve
 * de forma de arreglo al puntero al que hace referencia 'ciudades'
 * La función devuelve la cantidad de registros encontrados o 0 si es que
 * no se encontraron registros u ocurrió algún error.
 */
int getCiudades(PGconn *conn, unsigned int dpto, ptrCiudad **ciudades) {
    char instruccion[60];
    ptrCiudad *cities;
    int recCount, row, col;

    //seleccionamos las ciudades ubicadas en el departamento indicado
    sprintf(instruccion, "SELECT * FROM ciudades WHERE id_departamento=%d ORDER BY nombre", dpto);
    PGresult *res = PQexec(conn, instruccion);


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    } else {
        recCount = PQntuples(res); //cantidad de registros encontrados
        cities = malloc(recCount * sizeof (ptrCiudad)); //arreglo de ciudades

        for (row = 0; row < recCount; row++) {
            cities[row] = malloc(sizeof (Ciudad));
            char *tmp = NULL;
            //se almacena el valor devuelto en una variable temporal para
            //luego convertir en un valor entero
            tmp = PQgetvalue(res, row, 2);
            cities[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id_ciudad
            tmp = PQgetvalue(res, row, 0);
            strncpy(cities[row]->nombre, tmp, 30); //nombre
            tmp = PQgetvalue(res, row, 1);
            cities[row]->idDepartamento = (int) strtol(tmp, (char **) NULL, 10); //id_dpto
        }
    }
    PQclear(res);
    *ciudades = cities;
    return recCount;
}

/* obtiene todos los departamentos necontradas en la base de datos
 * y los devuelve de forma de arreglo al puntero al que hace referencia 
 * 'departamentos'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getDepartamentos(PGconn *conn, ptrDepartamento **departamentos) {
    ptrDepartamento *deptos;
    int recCount, row, col;

    //seleccionamos todos los departamentos existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM departamentos ORDER by nombre");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    deptos = malloc(recCount * sizeof (ptrDepartamento)); //arreglo de ciudades

    for (row = 0; row < recCount; row++) {
        deptos[row] = malloc(sizeof (Departamento));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 1);
        deptos[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id_ciudad
        tmp = PQgetvalue(res, row, 0);
        strncpy(deptos[row]->nombre, tmp, 30); //nombre
    }

    PQclear(res);
    *departamentos = deptos;
    return recCount;
}

/* obtiene todas las enfermedades necontradas en la base de datos
 * y las devuelve de forma de arreglo al puntero al que hace referencia 
 * 'enfermedades'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getEnfermedades(PGconn *conn, ptrEnfermedad **enfermedades) {
    ptrEnfermedad *diseases;
    int recCount, row, col;

    //seleccionamos todos los departamentos existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM enfermedades ORDER BY nombre");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    diseases = malloc(recCount * sizeof (ptrEnfermedad)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        diseases[row] = malloc(sizeof (Enfermedad));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 1);
        diseases[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id_ciudad
        tmp = PQgetvalue(res, row, 0);
        strncpy(diseases[row]->nombre, tmp, 30); //nombre
    }

    PQclear(res);
    *enfermedades = diseases;
    return recCount;
}

int getTopEnfermedades(PGconn *conn, ptrTopEnfermedades **enfermedades) {
    ptrTopEnfermedades *diseases;
    int recCount, row, col, idEnf;

    //seleccionamos todos los departamentos existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT id_enfermedad, COUNT(id_enfermedad) "
            "FROM enfermedades_registro GROUP BY id_enfermedad ORDER BY count DESC");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    diseases = malloc(recCount * sizeof (ptrTopEnfermedades)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        diseases[row] = malloc(sizeof (TopEnfermedades));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        //obtenemos el ID y luego la enfermedad a la que corresponde
        //para almacenar en la estructura
        idEnf = (int) strtol(tmp, (char **) NULL, 10);
        getEnfermedadById(conn, idEnf, &(diseases[row]->enfermedad));
        tmp = PQgetvalue(res, row, 1);        
        diseases[row]->cantidad = (int) strtol(tmp, (char **) NULL, 10); 
    }

    PQclear(res);
    *enfermedades = diseases;
    return recCount;
}

int getTopDepartamentos(PGconn *conn, ptrTopDepartamentos **departamentos) {
    ptrTopDepartamentos *deptos;
    int recCount, row, col, idDep;

    //seleccionamos todos los departamentos existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT ciudades.id_departamento, "
            "COUNT(ciudades.id_departamento) FROM enfermedades_registro LEFT"
            " JOIN ciudades ON enfermedades_registro.id_ciudad=ciudades.id GROUP"
            " BY ciudades.id_departamento ORDER BY count DESC");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    deptos = malloc(recCount * sizeof (ptrTopDepartamentos)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        deptos[row] = malloc(sizeof (TopDepartamentos));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        //obtenemos el ID y luego la enfermedad a la que corresponde
        //para almacenar en la estructura
        idDep = (int) strtol(tmp, (char **) NULL, 10);
        getDepartamentoById(conn, idDep, &(deptos[row]->departamento));
        tmp = PQgetvalue(res, row, 1);        
        deptos[row]->cantidad = (int) strtol(tmp, (char **) NULL, 10); 
    }

    PQclear(res);
    *departamentos = deptos;
    return recCount;
}

int getTopMeses(PGconn *conn, ptrTopMeses **meses) {
    ptrTopMeses *months;
    int recCount, row, col;

    //seleccionamos todos los departamentos existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT SUBSTRING(fecha_trx_server from 0 for "
            "7), count(*) FROM transacciones WHERE estado=1 GROUP BY substring(fecha_trx_server "
            "from 0 for 7) ORDER BY count DESC");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    months = malloc(recCount * sizeof (ptrTopMeses)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        months[row] = malloc(sizeof (TopMeses));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        //obtenemos el ID y luego la enfermedad a la que corresponde
        //para almacenar en la estructura
        strcpy(months[row]->mes, tmp);
        tmp = PQgetvalue(res, row, 1);        
        months[row]->cantidad = (int) strtol(tmp, (char **) NULL, 10); 
    }

    PQclear(res);
    *meses = months;
    return recCount;
}

/* obtiene todas las transacciones necontradas en la base de datos
 * y las devuelve de forma de arreglo al puntero al que hace referencia 
 * 'transacciones'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getTransacciones(PGconn *conn, ptrTransaccion **transacciones) {
    ptrTransaccion *transactions;
    int recCount, row, col;

    //seleccionamos todas las transacciones existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM transacciones");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    transactions = malloc(recCount * sizeof (ptrTransaccion)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        transactions[row] = malloc(sizeof (Transaccion));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        transactions[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id
        tmp = PQgetvalue(res, row, 1);
        transactions[row]->idUssd = (int) strtol(tmp, (char **) NULL, 10); //idUSSD
        tmp = PQgetvalue(res, row, 2);
        strncpy(transactions[row]->fechaClient, tmp, 15); //fecha de cliente
        tmp = PQgetvalue(res, row, 3);
        strncpy(transactions[row]->fechaServer, tmp, 15); //fecha de server
        tmp = PQgetvalue(res, row, 4);
        transactions[row]->estado = (short) strtol(tmp, (char **) NULL, 10); //estado
    }

    PQclear(res);
    *transacciones = transactions;
    return recCount;
}

/* obtiene todos los logs enccontrados en la base de datos
 * y los devuelve de forma de arreglo al puntero al que hace referencia 
 * 'logs'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getLogs(PGconn *conn, ptrLog **logs) {
    ptrLog *registros;
    int recCount, row, col;

    //seleccionamos todas las transacciones existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM log");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    registros = malloc(recCount * sizeof (ptrLog)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        registros[row] = malloc(sizeof (Log));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        registros[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id
        tmp = PQgetvalue(res, row, 1);
        registros[row]->idTransaccion = (int) strtol(tmp, (char **) NULL, 10); //idTrans
        tmp = PQgetvalue(res, row, 2);
        strncpy(registros[row]->fechaCliente, tmp, 15); //fechaCliente
        tmp = PQgetvalue(res, row, 3);
        strncpy(registros[row]->fechaServidor, tmp, 15); //fechaServidor
        tmp = PQgetvalue(res, row, 4);
        registros[row]->inOut = (int) strtol(tmp, (char **) NULL, 10); //inOut
        tmp = PQgetvalue(res, row, 5);
        strncpy(registros[row]->paquete, tmp, 4097); //fechaServidor
    }

    PQclear(res);
    *logs = registros;
    return recCount;
}

/* obtiene todos los registros de enfermedad enccontrados en la base de datos
 * y los devuelve de forma de arreglo al puntero al que hace referencia 
 * 'logs'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getRegistros(PGconn *conn, ptrRegistro **registros) {
    ptrRegistro *records;
    int recCount, row, col;

    //seleccionamos todas las transacciones existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM enfermedades_registro");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    records = malloc(recCount * sizeof (ptrRegistro)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        records[row] = malloc(sizeof (Registro));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        records[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id
        tmp = PQgetvalue(res, row, 1);
        records[row]->idCiudad = (int) strtol(tmp, (char **) NULL, 10); //idCiudad
        tmp = PQgetvalue(res, row, 2);
        records[row]->idEnfermedad = (int) strtol(tmp, (char **) NULL, 10); //idEnferm
        tmp = PQgetvalue(res, row, 3);
        records[row]->idTransaccion = (int) strtol(tmp, (char **) NULL, 10); //idTrans
    }

    PQclear(res);
    *registros = records;
    return recCount;
}

/* obtiene todos los USSDS enccontrados en la base de datos
 * y los devuelve de forma de arreglo al puntero al que hace referencia 
 * 'ussds'. La función devuelve la cantidad de registros encontrados o 
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getUssds(PGconn *conn, ptrUssd **ussds) {
    ptrUssd *clientes;
    int recCount, row, col;

    //seleccionamos todas las transacciones existentes en la base de datos
    PGresult *res = PQexec(conn, "SELECT * FROM ussd");


    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //cantidad de registros encontrados
    clientes = malloc(recCount * sizeof (ptrUssd)); //arreglo de enfermedades

    for (row = 0; row < recCount; row++) {
        clientes[row] = malloc(sizeof (Ussd));
        char *tmp = NULL;
        //se almacena el valor devuelto en una variable temporal para
        //luego convertir en un valor entero
        tmp = PQgetvalue(res, row, 0);
        strncpy(clientes[row]->hash, tmp, 41); //hash
        tmp = PQgetvalue(res, row, 1);
        clientes[row]->id = (int) strtol(tmp, (char **) NULL, 10); //id
    }

    PQclear(res);
    *ussds = clientes;
    return recCount;
}

/* obtiene la ciudad con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'ciudad'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getCiudadById(PGconn *conn, unsigned int id, ptrCiudad *ciudad) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM ciudades WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *ciudad = malloc(sizeof (Ciudad));
        tmp = PQgetvalue(res, 0, 0);
        strncpy((*ciudad)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, 0, 1);
        (*ciudad)->idDepartamento = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
        tmp = PQgetvalue(res, 0, 2);
        (*ciudad)->id = (int) strtol(tmp, (char **) NULL, 10); //idCiudad
    }
    PQclear(res);
    return recCount;
}

int getCiudadByOrden(PGconn *conn, unsigned int idDepto, unsigned int orden, ptrCiudad *ciudad) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM ciudades WHERE id_departamento=%d ORDER BY nombre",
            idDepto);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); 
    
    if (orden < recCount) {
        char *tmp;

        *ciudad = malloc(sizeof (Ciudad));
        tmp = PQgetvalue(res, orden, 0);
        strncpy((*ciudad)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, orden, 1);
        (*ciudad)->idDepartamento = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
        tmp = PQgetvalue(res, orden, 2);
        (*ciudad)->id = (int) strtol(tmp, (char **) NULL, 10); //idCiudad
        PQclear(res);
        return 1;
    }
    PQclear(res);
    return 0;
}

/* obtiene la ciudad que ordenada de forma alfabética perteneciente al derpatamento
 * con id 'dptoId' aparece en la posición 'orden' (empezando desde 0) dentro de
 * dentro de la lista entregada por la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'ciudad'. La función devuelve el id de la ciudad si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getCiudadByDptoId(PGconn *conn, unsigned int dptoId, unsigned int orden, ptrCiudad *ciudad) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM ciudades WHERE id_departamento=%d ORDER"
            " BY nombre", dptoId);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount >= 1) {
        char *tmp;

        *ciudad = malloc(sizeof (Ciudad));
        tmp = PQgetvalue(res, orden, 0);
        strncpy((*ciudad)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, orden, 1);
        (*ciudad)->idDepartamento = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
        tmp = PQgetvalue(res, orden, 2);
        (*ciudad)->id = (int) strtol(tmp, (char **) NULL, 10); //idCiudad
        PQclear(res);
        return (*ciudad)->id;
    }
    PQclear(res);
    return 0;
}

/* obtiene el departamento con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'departamento'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getDepartamentoByOrden(PGconn *conn, unsigned int orden, ptrDepartamento *departamento) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM departamentos ORDER BY nombre");
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); 
    if (orden < recCount) {
        char *tmp;

        *departamento = malloc(sizeof (Departamento));
        tmp = PQgetvalue(res, orden, 0);
        strncpy((*departamento)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, orden, 1);
        (*departamento)->id = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
        PQclear(res);
        return 1;
    }
    PQclear(res);
    return 0;
}

int getDepartamentoById(PGconn *conn, unsigned int id, ptrDepartamento *departamento) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM departamentos WHERE id=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *departamento = malloc(sizeof (Departamento));
        tmp = PQgetvalue(res, 0, 0);
        strncpy((*departamento)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, 0, 1);
        (*departamento)->id = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
    }
    PQclear(res);
    return recCount;
}
/* obtiene la enfermedad con el id especificado por el parámetro 'id'
 * dentro de la base de datos y la devuelve mediante el puntero al que hace
 * referencia el parámetro 'enfermedad'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getEnfermedadById(PGconn *conn, unsigned int id, ptrEnfermedad *enfermedad) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM enfermedades WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *enfermedad = malloc(sizeof (Enfermedad));
        tmp = PQgetvalue(res, 0, 0);
        strncpy((*enfermedad)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, 0, 1);
        (*enfermedad)->id = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
    }
    PQclear(res);
    return recCount;
}

int getEnfermedadByOrden(PGconn *conn, unsigned int orden, ptrEnfermedad *enfermedad) {
    char instruccion[60];
    int recCount, row, col;
    sprintf(instruccion, "SELECT * FROM enfermedades ORDER BY nombre");
    PGresult *res = PQexec(conn, instruccion);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (orden < recCount) {
        char *tmp;

        *enfermedad = malloc(sizeof (Enfermedad));
        tmp = PQgetvalue(res, orden, 0);
        strncpy((*enfermedad)->nombre, tmp, 30); //nombre
        tmp = PQgetvalue(res, orden, 1);
        (*enfermedad)->id = (int) strtol(tmp, (char **) NULL, 10); //idDepartamento
        PQclear(res);
        return 1;
    }
    PQclear(res);
    return 0;
}
/* obtiene el ussd con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'ussd'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getUssdById(PGconn *conn, unsigned int id, ptrUssd *ussd) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM ussd WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *ussd = malloc(sizeof (Ussd));
        tmp = PQgetvalue(res, 0, 0);
        strncpy((*ussd)->hash, tmp, 41); //hash
        tmp = PQgetvalue(res, 0, 1);
        (*ussd)->id = (int) strtol(tmp, (char **) NULL, 10); //idUssd

    }
    PQclear(res);
    return recCount;
}

/* obtiene la transacción con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'transacción'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getTransaccionById(PGconn *conn, unsigned int id, ptrTransaccion *transaccion) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM transacciones WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *transaccion = malloc(sizeof (Transaccion));
        tmp = PQgetvalue(res, 0, 0);
        (*transaccion)->id = (unsigned int) strtol(tmp, (char **) NULL, 10); //idTransaccion
        tmp = PQgetvalue(res, 0, 1);
        (*transaccion)->idUssd = (unsigned int) strtol(tmp, (char **) NULL, 10); //idTransaccion
        tmp = PQgetvalue(res, 0, 2);
        strncpy((*transaccion)->fechaClient, tmp, 15);
        tmp = PQgetvalue(res, 0, 3);
        strncpy((*transaccion)->fechaServer, tmp, 15);
        tmp = PQgetvalue(res, 0, 4);
        (*transaccion)->estado = (unsigned short) strtol(tmp, (char **) NULL, 10); //estado
    }
    PQclear(res);
    return recCount;
}

/* obtiene el log con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'log'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getLogById(PGconn *conn, unsigned int id, ptrLog *log) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM log WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *log = malloc(sizeof (Log));
        tmp = PQgetvalue(res, 0, 0);
        (*log)->id = (int) strtol(tmp, (char **) NULL, 10); //idTransaccion
        tmp = PQgetvalue(res, 0, 1);
        (*log)->idTransaccion = (int) strtol(tmp, (char **) NULL, 10); //idTransaccion
        tmp = PQgetvalue(res, 0, 2);
        strncpy((*log)->fechaCliente, tmp, 15); //fechaCliente
        tmp = PQgetvalue(res, 0, 3);
        strncpy((*log)->fechaServidor, tmp, 15); //fechaServidor
        tmp = PQgetvalue(res, 0, 4);
        (*log)->inOut = (int) strtol(tmp, (char **) NULL, 10);
        tmp = PQgetvalue(res, 0, 5);
        strncpy((*log)->paquete, tmp, 4097);
    }
    PQclear(res);
    return recCount;
}

/* obtiene el registro de enfermedad con el id especificado por el parámetro 'id'
 * dentro de la base de datos y lo devuelve mediante el puntero al que hace
 * referencia el parámetro 'registro'. La función devuelve 1 si tuvo éxito o
 * 0 si es que no se encontraron registros u ocurrió algún error.
 */
int getRegistroById(PGconn *conn, unsigned int id, ptrRegistro *registro) {
    char instruccion[60];
    int recCount, row, col;

    sprintf(instruccion, "SELECT * FROM enfermedades_registro WHERE ID=%d", id);
    PGresult *res = PQexec(conn, instruccion);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("No se encontró ningún registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return 0;
    }

    recCount = PQntuples(res); //debería devolver solo 1 o 0
    if (recCount == 1) {
        char *tmp;

        *registro = malloc(sizeof (Registro));
        tmp = PQgetvalue(res, 0, 0);
        (*registro)->id = (int) strtol(tmp, (char **) NULL, 10); //idTransaccion
        tmp = PQgetvalue(res, 0, 1);
        (*registro)->idCiudad = (int) strtol(tmp, (char **) NULL, 10); //idCiudad
        tmp = PQgetvalue(res, 0, 2);
        (*registro)->idEnfermedad = (int) strtol(tmp, (char **) NULL, 10); //idEnfermedad
        tmp = PQgetvalue(res, 0, 3);
        (*registro)->idTransaccion = (int) strtol(tmp, (char **) NULL, 10); //idTransaccion
    }
    PQclear(res);
    return recCount;
}

/* agrega una nueva ciudad a la base de datos tomando como parámetros
 * los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar la ciudad o 0 si se
 * insertó con éxito.
 */
int addCiudad(PGconn *conn, char *nombre, unsigned int idDepartamento) {
    char bufftemp[1048];
    sprintf(bufftemp, "INSERT INTO ciudades(nombre, id_departamento) VALUES ('%s', %d)",
            nombre, idDepartamento);
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        escribirLog("Error al insertar la ciudad.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }
    PQclear(res);
    return 0;
}

/* agrega un nuevo departamento a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar la ciudad o 0 si se
 * insertó con éxito.
 */
int addDepartamento(PGconn *conn, char *nombre) {
    char bufftemp[1048];
    sprintf(bufftemp, "INSERT INTO departamentos(nombre) VALUES ('%s')",
            nombre);
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        escribirLog("Error al insertar la transaccion.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }
    PQclear(res);
    return 0;
}

/* agrega una nueva enfermedad a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar la enfermedad o 0 si se
 * insertó con éxito.
 */
int addEnfermedad(PGconn *conn, char *nombre) {
    char bufftemp[1048];
    int recCount, id;

    sprintf(bufftemp, "INSERT INTO enfermedades(nombre) VALUES ('%s') RETURNING id",
            nombre);
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("Error al insertar la enfermedad.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }
    PQclear(res);
    return 0;
}

/* agrega un nuevo USSD a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar el USSD o 0 si se
 * insertó con éxito.
 */
int addUSSD(PGconn *conn, char *hash) {
    char bufftemp[1048];
    int recCount, id;

    sprintf(bufftemp, "INSERT INTO ussd(clave) VALUES ('%s') RETURNING id",
            hash);
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("Error al insertar el USSD.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }
    recCount = PQntuples(res);

    //si no hay 1 registro, quiere decir que hubo un problema
    if (recCount < 1) {
        PQclear(res);
        return -1;
    }

    char *tmp;

    //obtenemos el id devuelto
    tmp = PQgetvalue(res, 0, 0);
    id = (int) strtol(tmp, (char **) NULL, 10);
    PQclear(res);
    return id;
}

/* agrega una nueva transacción a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar la transacción o el ID de
 * la nueva transacción si se insertó con éxito.
 */
int addTransaccion(PGconn *conn, unsigned int idUSSD, char *fechaClient,
        char *fechaServer, unsigned short estado) {
    char bufftemp[1048];
    int recCount, id;

    sprintf(bufftemp, "INSERT INTO transacciones(id_ussd, fecha_trx_client, "
            "fecha_trx_server, estado) VALUES "
            "(%d, '%s', '%s', %d) RETURNING id", idUSSD, fechaClient, fechaServer, estado);

    //obtenemos el id del nuevo registro generado
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("Error al insertar la transaccion.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }

    recCount = PQntuples(res);

    //si no hay 1 registro, quiere decir que hubo un problema
    if (recCount < 1) {
        PQclear(res);
        return -1;
    }

    char *tmp;

    //obtenemos el id devuelto
    tmp = PQgetvalue(res, 0, 0);
    id = (int) strtol(tmp, (char **) NULL, 10);
    PQclear(res);
    return id;

}
 

/* agrega un nuevo registro de enfermedad a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * La función devuelve -1 si hubo un error al insertar el registro o el ID del
 * nuevo registro si se insertó con éxito.
 */
int addRegistro(PGconn *conn, unsigned int idTransaccion, unsigned int idCiudad,
        unsigned int idEnfermedad) {
    char bufftemp[1048];
    int id, recCount;


    sprintf(bufftemp, "INSERT INTO enfermedades_registro(id_ciudad, "
            "id_enfermedad, id_trx) VALUES (%d, %d, %d) RETURNING id",
            idCiudad, idEnfermedad, idTransaccion);
    PGresult *res = PQexec(conn, bufftemp);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        escribirLog("Error al insertar el registro.\n", __func__, LOG_ERROR);
        escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }

    recCount = PQntuples(res);

    //si no hay 1 registro, quiere decir que hubo un problema
    if (recCount < 1) {
        escribirLog("Error al insertar el registro.\n", __func__, LOG_ERROR);
        PQclear(res);
        return -1;
    }

    char *tmp;

    //obtenemos el id devuelto
    tmp = PQgetvalue(res, 0, 0);
    id = (int) strtol(tmp, (char **) NULL, 10);
    PQclear(res);
    return id;
}

/* agrega un nuevo log a la base de datos tomando como 
 * parámetros los distintos campos necesarios.
 * Los valores permitidos para el parámetro inOut son LOG_IN y LOG_OUT.
 * La función devuelve -1 si hubo un error al insertar el log o 0 si se
 * insertó con éxito.
 */

int addLog(PGconn *conn, unsigned int idTransaccion, char *fechaCliente,
            char *fechaServidor, unsigned int inOut, char *paquete) {
    char bufftemp[1048];
    int id, recCount;

    if (inOut == LOG_IN || inOut == LOG_OUT) {

        sprintf(bufftemp, "INSERT INTO log(id_trx, fecha_cliente, "
                "fecha_servidor, entrada_salida, paquete) VALUES (%d, '%s', '%s', "
                "%d, '%s') RETURNING id", 
                idTransaccion, fechaCliente, fechaServidor, inOut, paquete);
        PGresult *res = PQexec(conn, bufftemp);


        if(PQresultStatus(res) != PGRES_TUPLES_OK) {
            escribirLog("Error al insertar el log.\n", __func__, LOG_ERROR);
            escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
            PQclear(res);
            return -1;
        }

        recCount = PQntuples(res);

        //si no hay 1 registro, quiere decir que hubo un problema

        if (recCount < 1) {
            PQclear(res);
            return -1;

        } 

        char *tmp;

        //obtenemos el id devuelto
        tmp = PQgetvalue(res, 0, 0);
        id = (int) strtol(tmp, (char **) NULL, 10);
        PQclear(res);
        return id;
    }
    return -1;
}

/* cambia el estado de la transacción con el ID indicado al estado
 * que se pasa como parámetro.
 * Los valores permitidos para el estado son TX_CURSO y TX_FIN.
 * La función devuelve -1 si hubo un error al cambiar el estado o 0 si se
 * realizó con éxito.
 */
int cambiarEstadoTransaccion(PGconn *conn, unsigned int id, unsigned short estado) {
    char bufftemp[1048];
    int recCount;
    
    if(estado==TX_CURSO || estado==TX_FALLA || estado==TX_FIN || estado==TX_CONSULTA ) {
        time_t hora = time(NULL);
        char *fechaServer;
        convertirHora(hora, &fechaServer);
        
        sprintf(bufftemp, "UPDATE transacciones SET estado=%d, fecha_trx_server='%s' WHERE id=%d", 
                estado, fechaServer, id);

        PGresult *res = PQexec(conn, bufftemp);

        if(PQresultStatus(res) != PGRES_COMMAND_OK) {
            escribirLog("Error al modificar el estado de la transacción.\n", 
                    __func__, LOG_ERROR);
            escribirLog(PQresultErrorMessage(res), __func__, LOG_ERROR);
            PQclear(res);
            return -1;
        }



        PQclear(res);
        return 0;

    }
    return -1;
}