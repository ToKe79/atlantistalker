/*
 *  NAME
 *    atl-libapi.h
 *  DESCRIPTION
 *    Implementacia funkcii exportovanych talkerom pre pouzitie v pluginoch.
 *  COPYRIGHT
 *    (c) Dusan "Dusky" Gallo dusky@hq.alert.sk
 *    (c) Martin "Wolcaan" Karas wolcano@talker.sk
 */

#ifndef _ATL_LIBAPI_H
#define _ATL_LIBAPI_H
#define LIBERR_ALREADY_LOADED -1
#define LIBERR_NOT_LOADED -2
#define LIBERR_LIBARRAY_EXHAUSTED -3
#define LIBERR_DLOPEN_ERROR -4
#define LIBERR_MISSING_LIBID -5
#define LIBERR_MISSING_INIT -6
#define LIBERR_MISSING_DESTROY -7
#define LIBERR_INITIALIZATION -8
#define LIBERR_OTHER -9
#define LIBERR_BAD_NAME -10
#define LIBERR_OK 0

struct hook_s {
	int (*lib_socket_read)(char *, int);
	int (*lib_socket_write)(char *, int);
	int (*lib_heartbeat)();
	int (*lib_room_create)(char *);
	int (*lib_room_destroy)(char *, char *);
	int (*lib_room_link)(char *, char *);
	int (*lib_room_unlink)(char *, char *);
	int (*lib_command)(char *, char *);
	/*
	int (*lib_login)(char *);
	int (*lib_logout)(char *);
	int (*lib_change_param)(int, char *);
	int (*lib_room_write)(char *, char *);
	int (*lib_room_set_desc)(char *, int, char *);
	*/
};
#endif
