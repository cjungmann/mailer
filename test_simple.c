#include "mailer.h"
#include <stdio.h>

#include <readargs.h>

#include <ctt.h>

const char *host_url = "smtp.gmail.com";
int host_port = 587;
const char *user_name = NULL;
const char *password = NULL;
int use_ssl = 0;
int show_verbose = 0;

raOpt options[] = {
   {'h', "help",     NULL,     "This help screen",      &ra_show_help_agent,  NULL },
   {'u', "url",      "STRING", "URL of email account",  &ra_string_agent,     &host_url },
   {'P', "port",     "NUMBER", "SMTP port number",      &ra_int_agent,        &host_port },
   {'n', "name",     "STRING", "SMTP Account Name",     &ra_string_agent,     &user_name },
   {'p', "password", "STRING", "SMTP Account Password", &ra_string_agent,     &password },
   {'v', "verbose",  NULL,     "Provide verbose status",&ra_flag_agent,       &show_verbose },
   {'s', "show",     NULL,     "Show values",           &ra_show_values_agent,NULL },
   {'S', "ssl/tls",  NULL,     "Use SSL/TLS",           &ra_flag_agent,       &use_ssl }
};

int connection_reader(void *source, char *buffer, int bytes_to_read)
{
   mlrConn *conn = (mlrConn*)source;
   return mlr_connection_read(conn, buffer, bytes_to_read);
}

int process_connection(mlrConn *conn, const char *url, const char *user)
{
   char buffer[2048];
   LRScope scope;

   const char *greeting = "EHLO";

   // "return" parameter variables
   int code;
   int final_line;
   const char *line;
   const char *line_end;
   
   // Get connection response
   ctt_init_line_reader(&scope, buffer, sizeof(buffer), connection_reader, conn);

   while (mlr_get_smtp_line(&scope, &code, &final_line, &line, &line_end))
   {
      fwrite(line, 1, line_end - line, stdout);
      fputc('\n', stdout);

      if (final_line)
         break;
   }

   if (show_verbose)
      printf("Initializing connection with \"%s %s\"\n", greeting, url);

   mlr_connection_send_concat_line(conn, greeting, " ", url, NULL);

   while (mlr_get_smtp_line(&scope, &code, &final_line, &line, &line_end))
   {
      fwrite(line, 1, line_end - line, stdout);
      fputc('\n', stdout);

      if (final_line)
         break;
   }

   if (show_verbose)
      printf("Finished reading from server.\n");

   return 1;
}


int main(int argc, const char **argv)
{
   ra_set_scene(argv, argc, options, OPTS_COUNT(options));

   if (ra_process_arguments())
   {
      if (show_verbose)
         mlr_set_verbose_reporting(printf);

      mlrConn conn;
      mlr_init_connection(&conn);
      if (mlr_open_connection(&conn, host_url, host_port, user_name, password, use_ssl, 0))
         printf("Failed to make a connection.\n");
      else
      {
         process_connection(&conn, host_url, user_name);

         printf("Made a connection with %s at port %d.\n", host_url, host_port);
         mlr_close_connection(&conn);
      }
   }

   return 0;
}

