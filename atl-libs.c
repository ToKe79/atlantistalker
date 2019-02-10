/*
 *  NAME
 *    atl-libs.c
 *  DESCRIPTION
 *    Obsluzne funkcie pre pluginy. Volane zvycajne z jadra talkra.
 *  COPYRIGHT
 *    (c) Dusan "Dusky" Gallo dusky@hq.alert.sk
 *    (c) Martin "Wolcaan" Karas wolcano@talker.sk
 */

#include <dlfcn.h>
#include "atl-head.h"
#include "atl-libs.h"

struct lib_s lib_array[MAXLIBS];
void *this_lib;

int destroy_libs()
{
	int i;
	for (i = 0; i < MAXLIBS; i++) {
		try_to_unload(lib_array[i].lib_name);
	}

	dlclose(this_lib);
	this_lib = NULL;

	return 1;
}

int init_libs()
{
	/*struct stat fst;
	char s[1024];
	int i;*/

	this_lib = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);

	return 1;

/* autoload nemame */
}

int try_to_load(const char *fname) {
  char *error=NULL;
  char f[1024];
  int i;
  size_t si;
  unsigned int *libid;

  si = strspn(fname, "abcdefghijklmnopqrstuvwxyz0123456789_");
  if (si != strlen(fname)) {
	  return LIBERR_BAD_NAME;
  }
  snprintf(f, sizeof(f), "%s/lib%s.so", LIBS_ROOT_DIR, fname);
  
  for (i=0;i<MAXLIBS;i++)
    if (!(lib_array[i].lib_handle)) continue;
    else if (!(strcmp(lib_array[i].lib_name, fname))) break;
  
  if (i<MAXLIBS)
    return LIBERR_ALREADY_LOADED;

  for (i=0;i<MAXLIBS;i++)
    if (!(lib_array[i].lib_handle)) break;
  
  if (i>=MAXLIBS)
    return LIBERR_LIBARRAY_EXHAUSTED;

  memset((void *) &(lib_array[i].hooks), 0, sizeof(struct hook_s));

  lib_array[i].lib_handle = dlopen (f, RTLD_LAZY);
  if (!lib_array[i].lib_handle)
    return LIBERR_DLOPEN_ERROR;

  dlerror();
  
  libid=(unsigned int*)(dlsym(lib_array[i].lib_handle, "LIBID"));
  if ((error = dlerror()) != NULL || !libid)  {
    dlclose(lib_array[i].lib_handle);
    lib_array[i].lib_handle=NULL;
    return LIBERR_MISSING_LIBID;
  }
  lib_array[i].lib_id=*libid;
  
  *(void **) (&lib_array[i].lib_init) = dlsym(lib_array[i].lib_handle, "lib_init");
  if ((error = dlerror()) != NULL)  {
    dlclose(lib_array[i].lib_handle);
    lib_array[i].lib_handle=NULL;
    return LIBERR_MISSING_INIT;
  }

  *(void **) (&lib_array[i].lib_destroy) = dlsym(lib_array[i].lib_handle, "lib_destroy");
  if ((error = dlerror()) != NULL)  {
    dlclose(lib_array[i].lib_handle);
    lib_array[i].lib_handle=NULL;
    lib_array[i].lib_init=NULL;
    return LIBERR_MISSING_DESTROY;
  }
  lib_array[i].lib_name=strdup(fname);
  if (lib_array[i].lib_init(this_lib, 0, NULL)<0) {
    free(lib_array[i].lib_name);
    lib_array[i].lib_init=NULL;
    lib_array[i].lib_destroy=NULL;
    dlclose(lib_array[i].lib_handle);
    lib_array[i].lib_handle=NULL;
    return LIBERR_INITIALIZATION;
  }

  return LIBERR_OK;
}

int try_to_unload(const char *fname) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (!(lib_array[i].lib_handle)) continue;
    else if (!(strcmp(lib_array[i].lib_name, fname))) break;
  
  if (i>=MAXLIBS) {
    return LIBERR_NOT_LOADED;
  }

  lib_array[i].lib_destroy(1);
  free(lib_array[i].lib_name);
  lib_array[i].lib_init=NULL;
  lib_array[i].lib_destroy=NULL;
  dlclose(lib_array[i].lib_handle);
  lib_array[i].lib_handle=NULL;
  return LIBERR_OK;
}

void lib_run_heartbeat() {
  int i;
  
  for (i=0;i<MAXLIBS;i++) {
    if (lib_array[i].lib_handle && lib_array[i].hooks.lib_heartbeat) {
      lib_array[i].hooks.lib_heartbeat();
    }
  }
}

void lib_run_room_create(char *name) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (lib_array[i].lib_handle && lib_array[i].hooks.lib_room_create) {
      lib_array[i].hooks.lib_room_create(name);
    }
}

void lib_run_room_destroy(char *name, char *qmsg) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (lib_array[i].lib_handle && lib_array[i].hooks.lib_room_destroy) {
      lib_array[i].hooks.lib_room_destroy(name, qmsg);
    }
}

void lib_run_room_link(char *name1, char *name2) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (lib_array[i].lib_handle && lib_array[i].hooks.lib_room_link) {
      lib_array[i].hooks.lib_room_link(name1, name2);
    }
}

void lib_run_room_unlink(char *name1, char *name2) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (lib_array[i].lib_handle && lib_array[i].hooks.lib_room_unlink) {
      lib_array[i].hooks.lib_room_unlink(name1, name2);
    }
}

void lib_run_command(char *username, char *libname, char *argv) {
  int i;
  
  for (i=0;i<MAXLIBS;i++)
    if (lib_array[i].lib_handle && !strcmp(lib_array[i].lib_name, libname) && lib_array[i].hooks.lib_command) {
      lib_array[i].hooks.lib_command(username, argv);
    }
}

#if DEBUG
void lib_debug() {
  int i;
  
  for (i=0;i<MAXLIBS;i++) {
    if (lib_array[i].lib_handle)
      printf("%d: %08X %08X %s\n", i, lib_array[i].lib_id, (int)(lib_array[i].lib_handle), lib_array[i].lib_name);
  }
}
#endif

void cmd_module(UR_OBJECT user, char *inpstr)
{
#if DEBUG
  if (word_count == 1) {
    lib_debug();
    return;
  }
#endif
  if (word_count<=2) {
    write_user (user, "Pouzi: .module [<load|unload|command> <filename> [cmd_with_parameters]]\n");
    return;
  }

  if (!strcmp(word[1], "load")) {
    switch (try_to_load(word[2])) {
      case LIBERR_BAD_NAME:  
        write_user (user, "Nepovolene znaky v nazve kniznice!\n");
        break;
      case LIBERR_ALREADY_LOADED:  
        write_user (user, "Tato kniznica je uz nahrata!\n");
        break;
      case LIBERR_LIBARRAY_EXHAUSTED:
        write_user (user, "Prilis vela kniznic!\n");
        break;
      case LIBERR_DLOPEN_ERROR:
        write_user (user, dlerror());
        write_user (user, "\n"); /* dlopen usually doesn't add EOL */
        break;
      case LIBERR_MISSING_LIBID :
        write_user(user, "Nemozem najst 'LIBID'.\n");
        break;
      case LIBERR_MISSING_INIT:
        write_user(user, "Nemozem najst 'lib_init' funkciu.\n");
        break;
      case LIBERR_MISSING_DESTROY:
        write_user(user, "Nemozem najst 'lib_destroy' funkciu.\n");
        break;
      case LIBERR_INITIALIZATION:
        write_user(user, "Zlyhala inicializacia kniznice.\n");
        break;
      case LIBERR_OK:
        write_user(user, "Kniznica uspesne nahrata.\n");
        break;
      default:
        write_user(user, "Neznama chyba.\n");
    }
    return;
  }
   
  if (!strcmp(word[1], "unload")) {
    switch (try_to_unload(word[2])) {
      case LIBERR_NOT_LOADED:
        write_user (user, "Tato kniznica predsa nie je nahrata!\n");
        break;
      case LIBERR_OK:
        write_user(user, "Kniznica uspesne odpojena.\n");
        break;
      default:
        write_user(user, "Neznama chyba.\n");
    }
    return;
  }
   
  if (!strcmp(word[1], "command") && word_count>3) {
    inpstr = remove_first (inpstr);
    inpstr = remove_first (inpstr);
    lib_run_command(user->name, word[2], inpstr);
    return;
  }
   
  write_user (user, "Pouzi: .module [<load|unload|command> <filename|cmd_with_parameters>]\n");
}

