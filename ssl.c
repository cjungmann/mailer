#include "invisible.h"

#include <signal.h>  // for raise(SIGINT) to trigger breakpoint

void debug_write_ssl_connect_warning(FILE *f, SSL *ssl)
{
   char msg[1024];
   ERR_error_string_n(ERR_get_error(), msg, sizeof(msg));

   fprintf(f, "msg: [32;1m%s[m, "
           "lib error: [32;1m%s[m, "
           "func error: [32;1m%s[m, "
           "reason: [32;1m%s[m, "
           "verify result: [32;1m%ld[m\n",
           msg,
           ERR_lib_error_string(0),
           ERR_func_error_string(0),
           ERR_reason_error_string(0),
           SSL_get_verify_result(ssl));
}


mlrStatus open_ssl_handle(mlrSSL *handle, int socket)
{
   const SSL_METHOD *method;
   SSL_CTX *context;
   SSL *ssl;
   int connect_outcome;

   mlrStatus retval = MLR_FAILURE;

   OpenSSL_add_all_algorithms();
   /* err_load_bio_strings(); */
   ERR_load_crypto_strings();
   SSL_load_error_strings();

   /* openssl_config(null); */
   SSL_library_init();

   method = SSLv23_client_method();
   if (method)
   {
      context = SSL_CTX_new(method);

      if (context)
      {
         // following two not included in most recent example code i found.
         // it may be appropriate to uncomment these lines as i learn more.
         /* SSL_CTX_set_verify(context, SSL_VERIFY_NONE, NULL); */
         /* SSL_CTX_set_verify_depth(context, 4); */

         // we could set some flags, but i'm not doing it until i need to and i understand 'em
         /* const long ctx_flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION; */
         /* SSL_CTX_set_options(context, ctx_flags); */
         /* SSL_CTX_set_options(context, SSL_OP_NO_SSLv2); */

         ssl = SSL_new(context);
         if (ssl)
         {
            SSL_set_fd(ssl, socket);

            int saved_settings = unblock_socket(socket);
            connect_outcome = SSL_connect(ssl);

            restore_socket(socket, saved_settings);

            if (connect_outcome == -1)
            {
               if (verbose_printer)
                  (*verbose_printer)("SUCCESS: prepared SSL handle .\n");
#ifdef DEBUG
               debug_write_ssl_connect_warning(stderr, ssl);
#else
               retval = MLR_SSL_NO_SSL;
#endif               
            }
            else if (connect_outcome == 1)
            {
               handle->context = context;
               handle->ssl = ssl;

               if (verbose_printer)
                  (*verbose_printer)("SUCCESS: prepared SSL handle .\n");

               // Return before code that frees ssl and context.
               return MLR_SUCCESS;
            }
            else if (connect_outcome == 0)
               retval = MLR_SSL_CONNECTION_SHUTDOWN;
            else
               retval = MLR_SSL_CONNECTION_FAILED;

            SSL_free(ssl);
         }
         else
            retval = MLR_SSL_NO_SSL;

         SSL_CTX_free(context);
      }
      else
         retval = MLR_SSL_NO_CONTEXT;
   }

   return retval;

}

EXPORT void mlr_close_ssl(mlrSSL *handle)
{
   if (handle && handle->context)
   {
      SSL_CTX_free(handle->context);
      if (handle->ssl)
         SSL_free(handle->ssl);
   }
}
              
