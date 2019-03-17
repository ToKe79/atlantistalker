/*
 *  NAME
 *    atl-libs.h
 *  DESCRIPTION
 *    Obsluzne funkcie pre pluginy. Volane zvycajne z jadra talkra.
 *  COPYRIGHT
 *    (c) Dusan "Dusky" Gallo dusky@hq.alert.sk
 *    (c) Martin "Wolcaan" Karas wolcano@talker.sk
 */
#ifndef _ATL_LIBS_H
#define _ATL_LIBS_H
#include "atl-libapi.h"
#define MAXLIBS 10
#define LIBS_ROOT_DIR "libs"

struct lib_s {
	char *lib_name;
	void *lib_handle;
	unsigned int lib_id;
	int (*lib_init)(void *, int, char **);
	int (*lib_destroy)(int);

	struct hook_s hooks;
};
extern struct lib_s lib_array[];
int init_libs();
int destroy_libs();
int try_to_load(const char *fname);
int try_to_unload(const char *fname);
void lib_run_heartbeat();
void lib_run_room_create(char *name);
void lib_run_room_destroy(char *name, char *qmsg);
void lib_run_room_link(char *name1, char *name2);
void lib_run_room_unlink(char *name1, char *name2);
void lib_run_command(char *username, char *libname, char *argv);
#if DEBUG
void lib_debug();
#endif
#endif /* _ATL_LIBS_H */
