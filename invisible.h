#ifndef INVISIBLE_H
#define INVISIBLE_H

#include "mailer.h"

#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/bio.h>

#define EXPORT __attribute__((visibility("default")))

extern verbose_printer_t verbose_printer;

/** in socket.c */
int unblock_socket(int socket);
int restore_socket(int socket, int original_settings);

mlrStatus connect_socket(int socket, const struct sockaddr *addr, socklen_t addrlen);

/** in ssl.c */
void write_ssl_connect_warning(FILE *f, SSL *ssl);

mlrStatus open_ssl_handle(mlrSSL *handle, int socket);
void close_ssl_handle(mlrSSL *handle);

/** in messages.c */
int interpret_sockaddr(const struct sockaddr *sa, int *port, char *buffer, int buffer_len);
   

#endif
