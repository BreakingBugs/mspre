#include "md5.h"

/* Realiza el hash md5 de 'string' y lo guarda en 'mdString'
 * 'mdString' debe ser un buffer de 41 chars inicializado previamente
 * Thread-safe
 */
char *md5(char* string, char *mdString) {
    unsigned char digest[SHA_DIGEST_LENGTH];
    int i;

    bzero(mdString, sizeof (mdString));
    SHA1((unsigned char*) string, strlen(string), digest);
    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int) digest[i]);
    //printf("%s -> %s\n", string, mdString);
    return mdString;
}

/* Compara un hash md5 previo 'h1' con 'h2'
 * Retorna 1 si son iguales, 0 si son diferentes
 * Thread-safe
 */
int md5_compare(char *h1, char *h2) {
    return strncmp(h1, h2, 40) == 0 ? 1 : 0;
}
