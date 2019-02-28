/****h* API/Master
 *  NAME
 *    atl-libapi.c
 *  DESCRIPTION
 *    Implementacia funkcii exportovanych talkerom pre pouzitie v pluginoch.
 *  COPYRIGHT
 *    (c) Dusan "Dusky" Gallo dusky@hq.alert.sk
 *    (c) Martin "Wolcaan" Karas wolcano@talker.sk
 *********/

#include "atl-head.h"
#include "atl-libapi.h"
#include "atl-libs.h"

#ifdef __cplusplus
/* Exportovane funkcie pre C */
extern "C" {
	int api_room_info(char *);
	int api_room_create(char *);
	int api_room_destroy(char *, char *);
	int api_room_link(char *, char *);
	int api_room_unlink(char *, char *);
	int api_room_write(char *, char *);
	int api_room_set_desc(char *, int, char *);
	int api_room_get_first(char *);
	int api_room_get_next(char *, char *);
	int api_room_get_user_first(char *, char *);
	int api_room_get_user_next(char *, char *, char *);
	int api_user_set_dead(char *, char *);
	int api_user_get_idletime(char *);
	int api_user_write(char *, char *);
	int api_user_get_pad(char *, char *, int);
	int api_user_get_first(char *);
	int api_user_get_next(char *, char *);
	int api_command_reg(int, int, int, int, const char *, int, int, int (*)(char *,char *), int, int, int, int);
	int api_fnc_reg(unsigned int, struct hook_s *);
	int api_fnc_unreg(unsigned int);
}
#endif


/****f* API/api_room_info
 *  NAME
 *    api_room_info
 *  SYNOPSIS
 *    int api_room_info(char *name)
 *  INPUTS
 *    name - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *  BUGS
 *    Momentalne je tato funkcia vhodna jedine na skontrolovanie ci existuje miestnost s takymto nazvom a ziadne "info" nevracia.
 *****/
int api_room_info(char *name) {
  RM_OBJECT rm;
  
  if (!(rm = get_room (name, NULL)))
    return -1;
  return 0;
}

/****f* API/api_room_create
 *  NAME
 *    api_room_create
 *  SYNOPSIS
 *    int api_room_create(char *name)
 *  INPUTS
 *    name - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_create(char *name) {
  RM_OBJECT rm;
  if (!(rm = create_room (name, name))) return -1;
  return 0;
}

/****f* API/api_room_destroy
 *  NAME
 *    api_room_destroy
 *  SYNOPSIS
 *    int api_room_destroy(char *name, char *qmsg)
 *  INPUTS
 *    name - nazov miestnosti
 *    qmsg - hlaska s ktorou ma logoutnut userov
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_destroy(char *name, char *qmsg) {
  RM_OBJECT rm;
  UR_OBJECT u, n;
  
  if (!(rm = get_room (name, NULL)))
    return -1;

  u=user_first;
  while (u!=NULL) {
    n=u->next;
    if (u->room==rm) {
      logout_user (u, qmsg);
      inyjefuc = 1;
    }
    u=n;
  }
  /* destruct_room(rm); */ /* oops nemame */
  return 0;
}

/****f* API/api_room_link
 *  NAME
 *    api_room_link
 *  SYNOPSIS
 *    int api_room_link(char *name1, char *name2)
 *  INPUTS
 *    name1 - nazov miestnosti
 *    name2 - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_link(char *name1, char *name2) {
  RM_OBJECT rm1, rm2;
  
  if (!(rm1 = get_room (name1, NULL))) return -1;
  if (!(rm2 = get_room (name2, NULL))) return -1;
  link_room (rm1, rm2);
  return 0;
}

/****f* API/api_room_unlink
 *  NAME
 *    api_room_unlink
 *  SYNOPSIS
 *    int api_room_unlink(char *name1, char *name2)
 *  INPUTS
 *    name1 - nazov miestnosti
 *    name2 - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_unlink(char *name1, char *name2) {
  RM_OBJECT rm1, rm2;
  
  if (!(rm1 = get_room (name1, NULL))) return -1;
  if (!(rm2 = get_room (name2, NULL))) return -1;
  unlink_room (rm1, rm2);
  return 0;
}

/****f* API/api_room_write
 *  NAME
 *    api_room_write
 *  SYNOPSIS
 *    int api_room_write(char *name, char *text)
 *  INPUTS
 *    name - nazov miestnosti
 *    text - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_write(char *name, char *text) {
  RM_OBJECT rm;
  
  if (!(rm = get_room (name, NULL)))
    return -1;
  write_room(rm, text);
  return 0;
}

/****f* API/api_room_set_desc
 *  NAME
 *    api_room_set_desc
 *  SYNOPSIS
 *    int api_room_set_desc(char *name, int lng, char *desc)
 *  INPUTS
 *    name - nazov miestnosti
 *    lng - 
 *    desc - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_set_desc(char *name, int lng, char *desc) {
  RM_OBJECT rm;

  if (!(rm = get_room (name, NULL)))
    return -1;

  if ((lng < 0) || (lng > 2)) {
	  return -2;
  }

  if ((lng == 0) || (lng == 2)) {
	  if (rm->desc_sk != NULL) {
		  free (rm->desc_sk);
	  }
	  rm->desc_sk=strdup(desc);
  }

  if ((lng == 1) || (lng == 2)) {
	  if (rm->desc_en != NULL)
		  free (rm->desc_en);
	  rm->desc_en=strdup(desc);
  }

  return 0;
}

/****f* API/api_room_get_first
 *  NAME
 *    api_room_get_first
 *  SYNOPSIS
 *    int api_room_get_first(char *dst)
 *  INPUTS
 *    dst - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_get_first(char *dst) {
	/* RM_OBJECT rm; */

	if (room_first == NULL) {
		return -1;
	}
	strncpy(dst, room_first->name, ROOM_NAME_LEN);

	return 0;
}

/****f* API/api_room_get_next
 *  NAME
 *    api_room_get_next
 *  SYNOPSIS
 *    int api_room_get_next(char *dst, char *name)
 *  INPUTS
 *    dst - 
 *    name - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_get_next(char *dst, char *name) {
	RM_OBJECT rm;
	char tmpname[ROOM_NAME_LEN + 1];
	strncpy(tmpname, name, ROOM_NAME_LEN);

	if (!(rm = get_room (tmpname, NULL))) {
		return -1;
	}
	if (rm->next == NULL) {
		return -1;
	}

	strncpy(dst, rm->next->name, ROOM_NAME_LEN);

	return 0;
}

/****f* API/api_room_get_user_first
 *  NAME
 *    api_room_get_user_first
 *  SYNOPSIS
 *    int api_room_get_user_first(char *dst, char *name)
 *  INPUTS
 *    dst - 
 *    name - nazov miestnosti
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_get_user_first(char *dst, char *name) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];

	u = user_first;
	while (u) {
		if (u->room && !strcasecmp(u->room->name, name)) {
			strncpy(dst, u->name, sizeof(tmpname));
			return 0;
		}
		u = u->next;
	}

	return -1;
}

/****f* API/api_room_get_user_next
 *  NAME
 *    api_room_get_user_next
 *  SYNOPSIS
 *    int api_room_get_user_next(char *dst, char *roomname, char *username)
 *  INPUTS
 *    dst - 
 *    roomname - 
 *    username - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_room_get_user_next(char *dst, char *roomname, char *username) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];

	strncpy(tmpname, username, USER_NAME_LEN);

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}

	u = user_first;
	while (u) {
		if (u->room && !strcasecmp(u->room->name, roomname)) {
			strncpy(dst, u->name, USER_NAME_LEN);
			return 0;
		}
		u = u->next;
	}

	return -1;
}

/****f* API/api_user_get_pad
 *  NAME
 *    api_user_get_pad
 *  SYNOPSIS
 *    int api_user_get_pad(char *dst, char *name, int pad)
 *  INPUTS
 *    dst - 
 *    name - 
 *    pad - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_get_pad(char *dst, char *name, int pad) {
	UR_OBJECT u;
	char *s;
	char tmpname[USER_NAME_LEN + 1];
	strncpy(tmpname, name, USER_NAME_LEN);

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}
	
	s = sklonuj(u, pad);

	strncpy(dst, s, USER_NAME_LEN);

	return 0;
}

/****f* API/api_user_set_dead
 *  NAME
 *    api_user_set_dead
 *  SYNOPSIS
 *    int api_user_set_dead(char *name, char *text)
 *  INPUTS
 *    name - 
 *    text - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_set_dead(char *name, char *text) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];
	strncpy(tmpname, name, sizeof(tmpname));

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}

	/* u->dead = DEAD_CUSTOM_MESSAGE; */
	/* snprintf(u->death_custom_message, 255, "%s", text); */
	quit_user(u, text);
	
	return 0;
}

/****f* API/api_user_get_first
 *  NAME
 *    api_user_get_first
 *  SYNOPSIS
 *    int api_user_get_first(char *dst)
 *  INPUTS
 *    dst - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_get_first(char *dst) {
	if (user_first == NULL) {
		return -1;
	}
	strncpy(dst, user_first->name, USER_NAME_LEN);

	return 0;
}

/****f* API/api_user_get_next
 *  NAME
 *    api_user_get_next
 *  SYNOPSIS
 *    int api_user_get_next(char *dst, char *name)
 *  INPUTS
 *    dst - 
 *    name - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_get_next(char *dst, char *name) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];
	strncpy(tmpname, name, USER_NAME_LEN);

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}
	if (u->next == NULL) {
		return -1;
	}

	strncpy(dst, u->next->name, USER_NAME_LEN);

	return 0;
}

/****f* API/api_user_get_idletime
 *  NAME
 *    api_user_get_idletime
 *  SYNOPSIS
 *    int api_user_get_idletime(char *name)
 *  INPUTS
 *    name - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_get_idletime(char *name) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];
	strncpy(tmpname, name, USER_NAME_LEN);

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}

	return u->idletime;
}

/****f* API/api_user_write
 *  NAME
 *    api_user_write
 *  SYNOPSIS
 *    int api_user_write(char *name, char* text)
 *  INPUTS
 *    name - nazov miestnosti
 *    text - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_user_write(char *name, char* text) {
	UR_OBJECT u;
	char tmpname[USER_NAME_LEN + 1];
	strncpy(tmpname, name, sizeof(tmpname));

	if (!(u = get_user_exact (tmpname))) {
		return -1;
	}

	write_user(u, text);
	return 0;
}

/****f* API/api_command_reg
 *  NAME
 *    api_command_reg
 *  SYNOPSIS
 *    int api_command_reg (
 *      int order,
 *      int lngord,
 *      int casenum,
 *      int isprefix,
 *      const char *name,
 *      int deflevel,
 *      int level,
 *      int (*cmd_exec)(char *,char *),
 *      int language,
 *      int multiple,
 *      int group,
 *      int badge
 *      )
 *  INPUTS
 *    order -
 *    lngord -
 *    casenum -
 *    isprefix -
 *    name -
 *    deflevel -
 *    level -
 *    cmd_exec -
 *    language -
 *    multiple -
 *    group -
 *    badge -
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_command_reg (
/*		int order,
		int lngord,
		int casenum,
		int isprefix,
		const char *name,
		int deflevel,
		int level,
		int (*cmd_exec)(char *,char *),
		int language,
		int multiple,
		int group,
		int badge*/
		void
		) {
    return -1;
}

/****f* API/api_fnc_reg
 *  NAME
 *    api_fnc_reg
 *  SYNOPSIS
 *    int api_fnc_reg(unsigned int libid, int type, t_api_pfnc *fnc)
 *  INPUTS
 *    libid - 
 *    hooks -
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_fnc_reg(unsigned int libid, struct hook_s *hooks) {
  int i;

  if (hooks == NULL) {
	  return -1;
  }

  for (i=0;i<MAXLIBS;i++) {
    if (!lib_array[i].lib_handle) continue;
    if (lib_array[i].lib_id==libid) break;
  }
  if (i==MAXLIBS) return -1;

  memcpy((void *)(&(lib_array[i].hooks)), (void *) hooks, sizeof(struct hook_s));

  return 0;
}

/****f* API/api_fnc_unreg
 *  NAME
 *    api_fnc_unreg
 *  SYNOPSIS
 *    int api_fnc_unreg(unsigned int libid)
 *  INPUTS
 *    libid - 
 *  RESULT
 *    *  0 - v pripade uspechu
 *    * -1 - ak nastala chyba
 *****/
int api_fnc_unreg(unsigned int libid) {
  int i;
  
  for (i=0;i<MAXLIBS;i++) {
    if (!lib_array[i].lib_handle) continue;
    if (lib_array[i].lib_id==libid) break;
  }
  if (i==MAXLIBS) return -1;

  memset((void *) &(lib_array[i].hooks), 0, sizeof(struct hook_s));

  return 0;
}

