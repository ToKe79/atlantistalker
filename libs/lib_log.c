#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include "../atl-libapi.h"

/*
 * Atlantis talker module
 *    (c) Dusan "Dusky" Gallo dusky@hq.alert.sk
*/

const unsigned int LIBID=0x00010001;

char *filename=NULL;

#define DEFNAME "log/lib.log"

int (*my_api_fnc_reg)(unsigned int, struct hook_s *hooks) = NULL;
int (*my_api_fnc_unreg)(unsigned int) = NULL;

void write_log(char *str) {
  FILE *fp;
  
  if (filename && (fp=fopen(filename, "a"))!=NULL) {
    fprintf(fp, "%d [modid=%08X]: %s\n", (int)time(NULL), LIBID, str);
    fclose(fp);
  }
}

int my_lib_run_socket_read(char *dummy1, int dummy2) {
  write_log("LIBHOOK_SOCKET_READ");
  return LIBERR_OK;
}

int my_lib_run_socket_write(char *dummy1, int dummy2) {
  write_log("LIBHOOK_SOCKET_WRITE");
  return LIBERR_OK;
}

int my_lib_run_heartbeat() {
  write_log("LIBHOOK_HEARTBEAT");
  return LIBERR_OK;
}

int my_lib_run_login() {
  write_log("LIBHOOK_LOGIN");
  return LIBERR_OK;
}

int my_lib_run_logout() {
  write_log("LIBHOOK_LOGOUT");
  return LIBERR_OK;
}

int my_lib_run_command(char *username, char *argv) {
  char line[256];
  snprintf(line, sizeof(line), "LIBHOOK_COMMAND [%s][%s]", username, argv);
  write_log(line);
  return LIBERR_OK;
}

int my_lib_run_change_param() {
  write_log("LIBHOOK_CHANGE_PARAM");
  return LIBERR_OK;
}

int my_lib_run_room_create(char *name) {
  char line[256];
  snprintf(line, sizeof(line), "LIBHOOK_ROOM_CREATE [%s]", name);
  write_log(line);
  return LIBERR_OK;
}

int my_lib_run_room_destroy(char *name, char *quit_msg) {
  char line[256];
  snprintf(line, sizeof(line), "LIBHOOK_ROOM_DESTROY [%s][%s]", name, quit_msg);
  write_log(line);
  return LIBERR_OK;
}

int my_lib_run_room_link(char *name1, char *name2) {
  char line[256];
  snprintf(line, sizeof(line), "LIBHOOK_ROOM_LINK [%s -> %s]", name1, name2);
  write_log(line);
  return LIBERR_OK;
}

int my_lib_run_room_unlink(char *name1, char *name2) {
  char line[256];
  snprintf(line, sizeof(line), "LIBHOOK_ROOM_UNLINK [%s -> %s]", name1, name2);
  write_log(line);
  return LIBERR_OK;
}


int lib_init(void *lib, int argc, char **argv) {
  struct hook_s hooks;

  if (argc<0) argc=0;

  switch (argc) {
    case 0:
      filename=strdup(DEFNAME);
      break;
    case 1:
      filename=strdup(argv[0]);
      break;
    default:
      filename=strdup(DEFNAME);
  }

  *(void **)(&my_api_fnc_reg) = dlsym(lib, "api_fnc_reg");
  *(void **)(&my_api_fnc_unreg) = dlsym(lib, "api_fnc_unreg");

  if (my_api_fnc_reg == NULL
		  || my_api_fnc_unreg == NULL) {
	  return LIBERR_INITIALIZATION;
  }

  memset((void *) &hooks, 0, sizeof(struct hook_s));

  hooks.lib_socket_read = my_lib_run_socket_read;
  hooks.lib_socket_write = my_lib_run_socket_write;
  hooks.lib_heartbeat = my_lib_run_heartbeat;
  hooks.lib_command = my_lib_run_command;
  hooks.lib_room_create = my_lib_run_room_create;
  hooks.lib_room_destroy = my_lib_run_room_destroy;
  hooks.lib_room_link = my_lib_run_room_link;
  hooks.lib_room_unlink = my_lib_run_room_unlink;
  /*
  hooks.lib_login = my_lib_run_login;
  hooks.lib_logout = my_lib_run_logout;
  hooks.lib_change_param = my_lib_run_change_param;
  */

  my_api_fnc_reg(LIBID, &hooks);

  return LIBERR_OK;
}

int lib_destroy(int force)
{
  int i;

  i = my_api_fnc_unreg(LIBID);
  if ((i == LIBERR_OTHER) && (force != 1)) return LIBERR_OTHER;

  my_api_fnc_reg=NULL;
  my_api_fnc_unreg=NULL;

  if (filename) free(filename);
  filename=NULL;

  return LIBERR_OK;
}

