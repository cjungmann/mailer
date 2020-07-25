#include "mailer.h"
#include "invisible.h"

#include <string.h>
#include <alloca.h>

SKEYW smtp_keywords[] =
{
   { "STARTTLS",            mlr_ci_start_tls },
   { "7BITMIME",            mlr_ci_7_bit_mime },
   { "8BITMIME",            mlr_ci_8_bit_mime },
   { "ATRN",                mlr_ci_authenticated_turn },
   { "BINARYMIME",          mlr_ci_binary_mime },
   { "CHUNKING",            mlr_ci_chunking },
   { "DELIVERYBY",          mlr_ci_deliver_by },
   { "DSN",                 mlr_ci_delivery_status_notification },
   { "ENHANCEDSTATUSCODES", mlr_ci_enhanced_status_codes },
   { "EXPN",                mlr_ci_expand },
   { "ETRN",                mlr_ci_extended_turn },
   { "HELP",                mlr_ci_help },
   { "ONEX",                mlr_ci_one_message },
   { "PIPELINING",          mlr_ci_pipelining },
   { "REQUIRETLS",          mlr_ci_require_tls },
   { "SMTPUTF8",            mlr_ci_smtp_utf8 },
   { "VERB",                mlr_ci_verbose }
};

int count_smtp_keywords = sizeof(smtp_keywords) / sizeof(SKEYW);



EXPORT int mlr_seek_cap_index(const char *name, int name_len)
{
   // Make temporary name for substring if name_len is set.
   // Use stack memory so the memory is returned when done.
   if (name_len > 0)
   {
      name_len = strlen(name);
      char *tname = (char*)alloca(name_len + 1);
      memcpy(tname, name, name_len);
      tname[name_len] = '\0';
      name = tname;
   }

   SKEYW *ptr = smtp_keywords;
   SKEYW *end = ptr + count_smtp_keywords;

   while (ptr < end)
   {
      if (strcmp(name, ptr->name) == 0)
         return ptr->index;

      ++ptr;
   }

   return -1;
}

EXPORT const char* mlr_cap_name_from_index(int index)
{
   const SKEYW *ptr = smtp_keywords;
   const SKEYW *end = ptr + count_smtp_keywords;
   while (ptr < end)
   {
      if (ptr->index == index)
         return ptr->name;
      ++ptr;
   }

   return NULL;
}

EXPORT int mlr_smtp_cap_get(mlrSmtpCaps *caps, int index)
{
   uint64_t mask = 1 << index;
   return (mask & caps->flags) != 0;
}

EXPORT int mlr_smtp_cap_set(mlrSmtpCaps *caps, int index)
{
   uint64_t mask = 1 << index;
   int rval = (mask & caps->flags) != 0;
   caps->flags |= mask;
   return rval;
}

EXPORT int mlr_request_smtp_caps(mlrSmtpCaps *caps, LRScope *scope, mlrConn *conn, const char *url)
{
   int code;
   int index;
   const char *line;
   const char *line_end;
   int count = 0;

   memset(caps, 0, sizeof(mlrSmtpCaps));

   // Send EHLO message to get new set of capabilities:
   mlr_connection_send_concat_line(conn, "EHLO ", url, NULL);

   // Read response
   ctt_reset_line_reader(scope);
   while (mlr_get_smtp_line(scope, &code, &line, &line_end))
   {
      if (code == 250)
      {
         if (strncmp(line, "SIZE", 4) == 0)
         {
            caps->size = atoi(&line[5]);
            ++count;
         }
         else if ((index = mlr_seek_cap_index(line, line_end - line)) >= 0)
         {
            mlr_smtp_cap_set(caps, index);
            ++count;
         }
         else
         {
            if (verbose_printer)
            {
               int line_len = line_end - line;
               char *temp = (char*)alloca(line_len + 1);
               memcpy(temp, line, line_len);
               temp[line_len] = '\0';
               
               verbose_print_concat_line(verbose_printer, "Unknown SMTP verb \"", temp, "\"");
            }
         }
      }
   }

   return count;
}
