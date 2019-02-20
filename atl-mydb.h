/* Hlavne definicie */
#include <mysql/mysql.h>

/* Globalne premenne */

MYSQL mysql;
MYSQL_ROW row;
MYSQL_RES *result;
char query[8192],parsed[8192];

/* Deklaracie funkcii */

extern MYSQL_RES *mysql_result(char *select);
extern int init_database(void);
extern void shutdown_database(void);
extern int  db_save_user_details(UR_OBJECT user, int save_current);
extern int  db_load_user_details(UR_OBJECT user, int repository);
extern char *db_user_name(char *username);
extern char *db_username_by_id(int uid);
extern int  db_user_exists(char *username);
extern int  db_deluser(char *username);
char       *dbf_string(char *string);
int         db_load_user_password(UR_OBJECT user);
int         db_load_user_level(char *username);
int         mysql_kvery(char *select);
int         db_userid(char *username);
int         query_to_int(char *kvery);
int         db_user_switch(int uid,int svitch);

/*********************[ CO TO ROBI? ] ***************************
 * MYSQL_RES *mysql_result -> spravi select a vrati result
 * init_database -> inicializuje pripojenie na databazu
 * shutdown_database -> ukoncenie prace s databazou
 * db_save_user_details -> nahrada za save_user_details
 * db_load_user_details -> nahrada za load_user_details
 * db_user_exists -> vrati 1 ak user existuje, 0 ak neexistuje
 * db_deluser -> vrati 0 ak bol user zmazany, 1 ak cosi kdesi...
 * db_load_user_level -> vycucne level usera, -1 ak neexistuje
 ****************************************************************/

