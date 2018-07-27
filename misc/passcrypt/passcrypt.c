#include "../../atl-cryp.c"
#include "../../atl-mydb.h"

MYSQL_RES *mysql_result(char *select) {
	if (mysql_query(&mysql,select)) {
		fprintf(stderr,"MYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		fprintf(stderr,"Query: %s",select);
		return NULL;
	}

	if ((result=mysql_store_result(&mysql))==NULL) {
		fprintf(stderr,"MYSQL RESULT ERROR: Unable to store result!\n");
		return NULL;
	}
	return result;
}

char *expand_pass(char *pass) {
	static char out[20];
	sprintf(out,"&%-10s*2=h",pass);
	return out;
}

int main(int argc,char *argv[]) {

	FILE *fp;
	char line[200];
	char adb_name[30];
	char adb_user[30];
	char adb_pass[30];
	char pass_passed[]="PW_CHECK_PASSED";
	char pass_failed[]="PW_CHECK_FAILED";

	if (argc!=4) {
		fprintf(stderr,"Pouzi: %s <config_file> <user> <pass>\n\nOveri uzivatelove heslo.\n\nVystup:\n   %s ak heslo sedi, alebo\n   %s ak heslo nesedi.\n\n",argv[0],pass_passed,pass_failed);
		return 1;
	}

	if (!(fp=fopen(argv[1],"r"))) {
		fprintf(stderr,"CHYBA: nenajdeny subor '%s' s konfiguraciou!\n",argv[1]);
		return 1;
	}

	fgets(line,199,fp);
	line[strlen(line)-1]='\0';
	strncpy(adb_name,line,29);
	fgets(line,199,fp);
	line[strlen(line)-1]='\0';
	strncpy(adb_user,line,29);
	fgets(line,199,fp);
	line[strlen(line)-1]='\0';
	fclose(fp);
	strncpy(adb_pass,line,29);

	if (mysql_init(&mysql)==NULL) {
		fprintf(stderr,"MYSQL: Nedostatok pamate pre vytvorenie objektu!\n");
		return 1;
	}

	if (!mysql_real_connect(&mysql,DBHOSTNAME,adb_user,adb_pass,NULL,0,NULL,0)) {
		fprintf(stderr,"MYSQL: Zlyhalo pripojenie k databaze: %s\n",mysql_error(&mysql));
		return 1;
	}

	if (mysql_select_db(&mysql,adb_name)) {
		fprintf(stderr,"MYSQL: Nemozno vybrat zakladnu databazu!\n");
		return 1;
        }

	sprintf(query,"SELECT `passwd` FROM `users` WHERE `username` = '%s'",argv[2]);

	if(!(result=mysql_result(query))) {
		fprintf(stderr,"MYSQL: Zlyhal dopyt '%s' na databazu!\n",query);
		return 1;
	}

	if(!(row=mysql_fetch_row(result))) {
		fprintf(stderr,"CHYBA: Userko sa nenasiel v databaze!\n");
		return 1;
	}

	mysql_free_result(result);

	if(strcmp((char *)md5_crypt(expand_pass(argv[3]),argv[2]),row[0]) != 0) {
		printf(pass_failed);
	} else {
		printf(pass_passed);
	}

	return 0;
}
