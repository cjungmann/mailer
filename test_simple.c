#include "mailer.h"
#include <stdio.h>

#include <readargs.h>

const char *host_url = "smtp.gmail.com";
int host_port = 587;
const char *user_name = NULL;
const char *password = NULL;
int use_ssl = 0;
int show_verbose = 0;

raOpt options[] = {
   {'h', "help",     NULL,     "This help screen",      &ra_show_help_agent, NULL },
   {'u', "url",      "STRING", "URL of email account",  &ra_string_agent,    &host_url },
   {'P', "port",     "NUMBER", "SMTP port number",      &ra_int_agent,       &host_port },
   {'n', "name",     "STRING", "SMTP Account Name",     &ra_string_agent,    &user_name },
   {'p', "password", "STRING", "SMTP Account Password", &ra_string_agent,    &password },
   {'v', "verbose",  NULL,     "Provide verbose status",&ra_flag_agent,      &show_verbose }
};

int main(int argc, const char **argv)
{
   ra_set_scene(argv, argc, options, OPTS_COUNT(options));

   if (ra_process_arguments())
   {
      if (show_verbose)
         mlr_set_verbose_reporting(printf);

      mlrConn conn;
      mlr_init_connection(&conn);
      if (mlr_open_connection(&conn, host_url, host_port, user_name, password, 1))
         printf("Failed to make a connection.\n");
      else
         printf("Made a connection with %s at port %d.\n", host_url, host_port);
   }

   return 0;
}

