/* 
 * File:   util.h
 * Author: jordan
 *
 * Created on May 3, 2014, 2:33 PM
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    /* Manejo de errores
     * Escribir en el log el error
     */
    void handle_error(const char *msg);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

