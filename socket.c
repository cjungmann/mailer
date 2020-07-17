#include "mailer.h"
#include "invisible.h"


#include <stdio.h>
#include <errno.h>

#include <string.h>      // for memset()

#include <netdb.h>       // For getaddrinfo() and supporting structures
#include <arpa/inet.h>   // Functions that convert addrinfo member values.
#include <unistd.h>      // for close() function

#include <fcntl.h>
#include <sys/select.h>  // support controlable socket timeout.

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/** Set nonblock flag, returns previous settings. */
int unblock_socket(int socket)
{
   int settings = fcntl(socket, F_GETFL, NULL);
   if (settings >= 0)
      fcntl(socket, F_SETFL, settings | O_NONBLOCK);
   return settings;
}

/** Set socket flags.  Use to restore from unblock_socket() */
int restore_socket(int socket, int original_settings)
{
   return fcntl(socket, F_SETFL, original_settings);
}


/** Called by mlr_get_connected_socket() to attempt to connect to a socket. */
mlrStatus connect_socket(int socket, const struct sockaddr *addr, socklen_t addrlen)
{
   int retval = MLR_FAILURE;
   int result;
   int saved_errno = 0;
   int socket_settings = unblock_socket(socket);
   if (socket_settings >= 0)
   {
      if ((result = connect(socket, addr, addrlen)) == 0)
         retval = MLR_SUCCESS;
      else if (errno == EINPROGRESS)
      {
         // Refer to `man 2 lect_tut`
         struct timeval timeout;
         timeout.tv_sec = 2;
         timeout.tv_usec = 0;

         fd_set wait_set;
         FD_ZERO(&wait_set);
         FD_SET(socket, &wait_set);

         select(socket+1, NULL, &wait_set, NULL, &timeout);

         if (FD_ISSET(socket, &wait_set))
            retval = MLR_SUCCESS;
         else
            retval = MLR_CONNECT_TIMED_OUT;
      }
      else
         saved_errno = errno;

      restore_socket(socket, socket_settings);

      // Restore in case fcntl sets errno:
      errno = saved_errno;
      return retval;
   }
   else
   {
      // If blocking not possible, at least give it one try:
      result = connect(socket, addr, addrlen)==0;
      if (result == 0)
         return MLR_SUCCESS;
      else
         return MLR_SYSTEM_ERROR;
   }
}

EXPORT mlrStatus mlr_get_connected_socket(int *sock, const char *host_url, int host_port)
{
   char port_string[20];
   sprintf(port_string, "%d", host_port);

   struct addrinfo hints;
   struct addrinfo *ai_chain, *rp;

   *sock = -1;
   int result = -1;
   mlrStatus retval = MLR_FAILURE;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;

   if ((result = getaddrinfo(host_url, port_string, &hints, &ai_chain)))
   {
      switch(result)
      {
         case EAI_BADFLAGS:   // invalid ai_flags value
            retval = MLR_ADDRESS_BAD_FLAGS;
            break;

         case EAI_AGAIN:      // name resolution temporary failure
            retval = MLR_ADDRESS_TEMPORARY_FAILURE;
            break;

         case EAI_FAIL:       // name resolution permanent failure
            retval = MLR_ADDRESS_PERMANENT_FAILURE;
            break;

#ifdef __USE_GNU
         case EAI_ADDRFAMILY: // host no family addresses
         case EAI_NODATA:     // host lacks addresses
#endif
         case EAI_FAMILY:     // host says family unavailable
         case EAI_NONAME:     // host lacks requested service
         case EAI_SERVICE:    // host lacks service for socket type
         case EAI_SOCKTYPE:   // host socket type not supported
            retval = MLR_HOST_REJECTS_REQUEST;
            break;

         case EAI_MEMORY:     // system out of memory
         case EAI_SYSTEM:     // system error, check errno for more info
         case EAI_OVERFLOW:   // "Argument buffer overflow"
            retval = MLR_SYSTEM_ERROR;
            break;
      }

      if (verbose_printer)
         (*verbose_printer)("FAIL: unable to get address for %s:%d (%s).\n",
                            host_url,
                            host_port,
                            mlr_get_status_string(retval));
   }
   else
   {
      rp = ai_chain;
      while (rp)
      {
         if ((rp->ai_family == PF_INET || rp->ai_family == PF_INET6)
             && rp->ai_socktype == SOCK_STREAM
             && rp->ai_protocol == IPPROTO_TCP )
         {
            result = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (result != -1)
            {
               if (MLR_SUCCESS == (retval = connect_socket(result, rp->ai_addr, rp->ai_addrlen)))
               {
                  if (verbose_printer)
                  {
                     char buffer[16];
                     int resolved_port;
                     interpret_sockaddr(rp->ai_addr, &resolved_port, buffer, sizeof(buffer));
                     (*verbose_printer)("SUCCESS: request for %s:%d resolved to %s:%d\n",
                                        host_url,
                                        host_port,
                                        buffer,
                                        resolved_port);
                  }

                  *sock = result;
               }
               else
                  close(result);
            }
            break;
         }

         rp = rp->ai_next;
      }

      freeaddrinfo(ai_chain);
   }

   return retval;
}


