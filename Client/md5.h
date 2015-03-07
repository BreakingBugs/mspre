/* 
 * File:   md5.h
 * Author: jordan
 *
 * Created on May 3, 2014, 1:59 PM
 */

#ifndef MD5_H
#define	MD5_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

    /**
     * @brief Hashea usando md5
     * 
     * Realiza el hash md5 de 'string'
     * Thread-safe
     * @param
     *  string: string a ser hasheado
     *  mdString: debe ser un buffer de 33 chars inicializado previamente
     */
    char *md5(char* string, char *mdString);

    /**
     * @brief  compara dos hashes md5
     * 
     * @param  h1: primer hash
     *         h2: segundo hash
     * 
     * @return 1 si son iguales, 0 si son diferentes
     */
    int md5_compare(char *h1, char *h2);


#ifdef	__cplusplus
}
#endif

#endif	/* MD5_H */

