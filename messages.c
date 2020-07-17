#include "mailer.h"
#include "invisible.h"

#include <arpa/inet.h>

typedef struct msg_index
{
   mlrStatus index;
   const char *message;
} MIndex;

MIndex emessages[] = {
   { MLR_SUCCESS,                   "success" },
   { MLR_FAILURE,                   "failure" },
   { MLR_ADDRESS_BAD_FLAGS,         "bad getaddrinfo flags" },
   { MLR_ADDRESS_TEMPORARY_FAILURE, "temporary failure to get address.  Try again later." },
   { MLR_ADDRESS_PERMANENT_FAILURE, "permanent failure to get address." },
   { MLR_HOST_REJECTS_REQUEST,      "host rejects connection." },
   { MLR_SYSTEM_ERROR,              "system error" },
   { MLR_URL_UNRESOLVED,            "unresolved URL" },
   { MLR_NO_SUITABLE_SOCKET,        "suitable socket not available" },
   { MLR_CONNECT_FAILED,              "connection request failed" },
   { MLR_CONNECT_TIMED_OUT,           "connection request timed-out" },
   { MLR_SSL_NO_METHOD,               "failed to get SSL method" },
   { MLR_SSL_NO_CONTEXT,              "failed to generate SSL context" },
   { MLR_SSL_NO_SSL,                  "failed to create an SSL handle" },
   { MLR_SSL_CONNECTION_FAILED,       "failed to make SSL connection" },
   { MLR_SSL_CONNECTION_SHUTDOWN,     "SSL connection failed with orderly shutdown" }
};

MIndex *emessages_last = emessages + sizeof(emessages);

EXPORT const char *mlr_get_status_string(mlrStatus status)
{
   MIndex *ptr = emessages;
   while (ptr < emessages_last)
   {
      if (ptr->index == status)
         return ptr->message;
      ++ptr;
   }

   return "unknown status value";
}

int interpret_sockaddr(const struct sockaddr *sa, int *port, char *buffer, int buffer_len)
{
   const char *pptr = sa->sa_data;

   uint16_t tport = (uint16_t)*pptr++ << 8;
   tport += (uint16_t)*pptr++;

   const char *result = inet_ntop(sa->sa_family, pptr, buffer, buffer_len);
   if (result)
   {
      *port = tport;
      return 1;
   }
   else
      return 0;
}
