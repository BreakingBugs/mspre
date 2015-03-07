/* 
 * File:   server.h
 * Author: jordan
 *
 * Created on May 18, 2014, 6:35 PM
 */

#ifndef SERVER_H
#define	SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

//Inicia la configuracion del daemon
void loadConfig();
//Instala los handlers de senhales
void install_handlers();
void stop_handler();
void reload_handler();
void *handle_request(void* arg);
int almacenarRegistro(PGconn *conn, struct paquete paquete);
int paqueteError(PGconn *conn, char *original, struct paquete *pservidor, char* menu, int idTx);
int recibirPaquete(PGconn *conn, char *paquete, int socket, struct paquete *est);
int enviarPaquete(PGconn *conn, char *paquete, int socket, struct paquete est);

void generarInformeDepartamentos(PGconn *conn, struct paquete * paq);
void generarInformeEnfermedades(PGconn *conn, struct paquete * paq);
void generarInformeMeses(PGconn *conn, struct paquete * paq);
int generarMenuCiudades(PGconn *conn, unsigned int orden, struct paquete * paq);
void generarMenuDepartamentos(PGconn *conn, struct paquete * paq);
int generarMenuEnfermedades(PGconn *conn, struct paquete * paq);
void generarMenuEstadisticas(struct paquete * paq);
int user_auth(PGconn *conn, unsigned int user, char *pass);


#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_H */

