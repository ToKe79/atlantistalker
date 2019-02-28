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

	char pass_passed[]="PW_CHECK_PASSED";
	char pass_failed[]="PW_CHECK_FAILED";

	if (argc!=3) {
		fprintf(stderr,"Pouzi: %s <user> <pass>\n\nOveri uzivatelove heslo.\n\nVystup:\n   %s ak heslo sedi, alebo\n   %s ak heslo nesedi.\n\n",argv[0],pass_passed,pass_failed);
		return 1;
	}

	/* Nickname must begin with an uppercase letter */
	argv[1][0]=toupper(argv[1][0]);

	if (mysql_init(&mysql)==NULL) {
		fprintf(stderr,"MYSQL: Nedostatok pamate pre vytvorenie objektu!\n");
		return 1;
	}

	if (!mysql_real_connect(&mysql,DBHOST,DBUSER,DBPASS,NULL,0,NULL,0)) {
		fprintf(stderr,"MYSQL: Zlyhalo pripojenie k databaze: %s\n",mysql_error(&mysql));
		return 1;
	}

	if (mysql_select_db(&mysql,DBNAME)) {
		fprintf(stderr,"MYSQL: Nemozno vybrat zakladnu databazu!\n");
		return 1;
        }

	sprintf(query,"SELECT `passwd` FROM `users` WHERE `username` = '%s'",argv[1]);

	if(!(result=mysql_result(query))) {
		fprintf(stderr,"MYSQL: Zlyhal dopyt '%s' na databazu!\n",query);
		return 1;
	}

	if(!(row=mysql_fetch_row(result))) {
		fprintf(stderr,"CHYBA: Userko sa nenasiel v databaze!\n");
		return 1;
	}

	mysql_free_result(result);

	if(strcmp((char *)md5_crypt(expand_pass(argv[2]),argv[1]),row[0]) != 0) {
		printf("%s",pass_failed);
	} else {
		printf("%s",pass_passed);
	}

	return 0;
}
