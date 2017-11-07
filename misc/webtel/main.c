
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <poll.h>

#include <mysql/mysql.h>


#define DEBUG 0

#define WEB_INDEX "web_index"
#define WEB_IN "web_in"
#define WEB_OUT "web_out"

// pri komunikovani s testovacim toolom 'telmysql'
//#define WEB_INDEX "telmysql_web_index"
//#define WEB_IN "telmysql_web_in"
//#define WEB_OUT "telmysql_web_out"

#define SESSIONID_LEN 32
#define DEBUG_FILE "/tmp/webtel.log"

#if DEBUG
#define CONFIG_FILE "webtel.conf"
#else
#define CONFIG_FILE "/usr/sbin/webtel.conf"
#endif

int log(const char *s)
{
#if DEBUG
	FILE *f;
	f = fopen(DEBUG_FILE, "a");
	fwrite(s, 1, strlen(s), f);
	fclose(f);
#endif
	return 1;
}

/* loads login credentials from file */
int load_cfg(char *filename, char host[256], char name[256], char user[256], char pass[256])
{
	FILE *fp;
	char *s;

	if ((fp = fopen(filename, "r")) == NULL) {
		return 1;
	}

	if (fgets(host, 256, fp) == NULL) {
		return 2;
	}
	if (fgets(name, 256, fp) == NULL) {
		return 3;
	}
	if (fgets(user, 256, fp) == NULL) {
		return 4;
	}
	if (fgets(pass, 256, fp) == NULL) {
		return 5;
	}

	fclose(fp);

	if ((s = strchr(host, '\n')) != NULL) {
		*s = '\0';
	}
	if ((s = strchr(name, '\n')) != NULL) {
		*s = '\0';
	}
	if ((s = strchr(user, '\n')) != NULL) {
		*s = '\0';
	}
	if ((s = strchr(pass, '\n')) != NULL) {
		*s = '\0';
	}

	return 0;
}

int init_database (MYSQL * m)
{
	char dbhost[256];
	char dbname[256];
	char dbuser[256];
	char dbpass[256];
	int i;

	i = load_cfg(CONFIG_FILE, dbhost, dbname, dbuser, dbpass);

	if (i) {
		char logs[8096];
		sprintf(logs, "Chyba %d pri parsovani config suboru '%s'.\n", i, CONFIG_FILE);
		log(logs);
		return 4;
	}

	if (mysql_init (m) == NULL) {
		log ("MYSQL: Nedostatok pamate pre vytvorenie objektu!\n");
		return 1;
	}

	if (!mysql_real_connect (m, dbhost, dbuser, dbpass, NULL, 0, NULL, 0)) {
		log ("MYSQL: Zlyhalo pripojenie k databaze: ");
		log(mysql_error (m));
		log("\n");
		return 2;
	}
	if (mysql_select_db (m, dbname)) {
		log ("MYSQL: Nemozno vybrat zakladnu databazu!\n");
		return 3;
	}
	return 0;
}

void shutdown_database (MYSQL * m)
{
	mysql_close (m);
}

int make_sessionid(char *s)
{
	int j;

	j = getpid();
	j = sprintf (s, "webtel-%05i-", j);

	if (j != 13) {
		s[0] = '\0';
		return 1;
	}

	/* dogenerujeme na dlzku sessionid */
	for (; j < SESSIONID_LEN; j++) {
		sprintf(s+j, "%x", rand() % 16);
	}

	return 0;
}

MYSQL_RES * mysql_result (MYSQL * m, const char *select)
{
	MYSQL_RES *r;

	if (mysql_query (m, select)) {
		printf(mysql_error(m));
		return NULL;
	}

	if ((r = mysql_store_result (m)) == NULL) {
		return NULL;
	}
	return r;
}

int get_ip(int fd, char ip[16], char site[81], char port[6]) {
	struct sockaddr_in acc_addr;
	socklen_t socklen;
	struct hostent * hs;

	socklen = sizeof(struct sockaddr_in);

	if (!getpeername(fd, (struct sockaddr *) &acc_addr, &socklen)) {
		snprintf(port, 6, "%i", acc_addr.sin_port);
		snprintf(ip, 16, "%s", inet_ntoa(acc_addr.sin_addr));
		if ((hs = gethostbyaddr(&(acc_addr.sin_addr), sizeof(acc_addr.sin_addr), PF_INET))) {
			snprintf(site, 81, "%s", hs->h_name);
		}
		else {
		}
		return 0;
	}

	return 1;
}

int main()
{
	MYSQL_ROW row;
	MYSQL_RES *result;
	MYSQL m;

	char query[4096];
	char query_escaped[2*sizeof(query) + 1];
	char input_line[3072];

	char sessionid[SESSIONID_LEN + 1];
	unsigned long int web_id; /* web_* ID */
	char *username = "WEBTEL";
	unsigned long l;

	struct pollfd pfd[3];

	char readquery[1024];
	char killquery[1024];

	short int quit;

	srand (time (0));

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	make_sessionid(sessionid);

	init_database(&m);

	/*
	// nemame prave DELETE
	sprintf(query, "delete from " WEB_INDEX " where sessionid=\'%s\'", sessionid);
	mysql_result(&m, query);
	*/

	{
		char ip[16];
		char site[81];
		char port[6];
		char webtelsite[sizeof(site) + 8];
		char escusername[sizeof(username)*2 + 1];

		if (get_ip(0, ip, site, port)) {
			/* no peername? run from tty! */
			sprintf(ip, "127.0.0.1");
			sprintf(site, "localhost");
			sprintf(port, "0");
			//exit(5);
		}
		ip[sizeof(ip) - 1] = '\0';
		site[sizeof(site) - 1] = '\0';
		port[sizeof(port) - 1] = '\0';
		sprintf(webtelsite, "WEBTEL:%s", site);

		mysql_real_escape_string(&m, escusername, username, strlen(username));
		mysql_real_escape_string(&m, query_escaped, webtelsite, strlen(webtelsite));

		l = snprintf(query, sizeof(query), "insert into " WEB_INDEX " (status,sessionid,username,ip,site,port) values ('1','%s','%s','%s','%s','%s');",
				sessionid, "WEBTEL", ip, query_escaped, port);

		if (l > sizeof(query)) {
			/* pretiekol nam buffer, zahodime vsetko */
			query[0] = '\0';
		}
		else {
			mysql_query(&m, query);
		}
	}

	sprintf(query, "select ifnull(max(id), 0) from " WEB_INDEX " where sessionid='%s';", sessionid);
	result = mysql_result(&m, query);

	web_id = 0;

	if (result != NULL) {
		if((row = mysql_fetch_row(result)) != NULL) {
			web_id = atoi(row[0]);
		}
		mysql_free_result(result);
	}

	if (web_id == 0) {
		log("Can't get ID!\n");
		exit(6);
	}

	memset(input_line, '\0', 1024);

	short int errcond = POLLERR | POLLHUP | POLLNVAL;

	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLIN | POLLPRI | POLLERR | POLLHUP;
	pfd[0].revents = 0;
	pfd[1].fd = STDOUT_FILENO;
	pfd[1].events = errcond;
	pfd[2].fd = STDERR_FILENO;
	pfd[2].events = errcond;
	sprintf(readquery, "select lineid, line from " WEB_OUT " where id = '%ld' order by lineid asc", web_id);
	sprintf(killquery, "select ifnull(max(status), 3) from " WEB_INDEX " where id='%ld';", web_id);

	quit = 0;

	while ((l = poll(pfd, 3, 1000)) >= 0) {
		/* nacitame co poslal user */
		if (l > 0) {
			if ((pfd[0].revents & POLLIN)) {
				if((fgets(input_line, sizeof(input_line), stdin)) != NULL) {
					mysql_real_escape_string(&m, query, input_line, strlen(input_line));

					sprintf(query_escaped, "insert into " WEB_IN " (id,line) values ('%ld','%s')", web_id, query);
					mysql_query(&m, query_escaped);

					memset(input_line, '\0', sizeof(input_line));
				}
				else {
					quit = 1;
					log("Read 0 bytes. This means user closed connection.\n");
				}
			}

			if ((pfd[0].revents & errcond)
					|| (pfd[1].revents & errcond)
					|| (pfd[2].revents & errcond)) {
				log("ERROR on FD, closing connection!\n");
				quit = 1;
			}

			if (quit) {
				/* "stratene spojenie" */
				sprintf(query_escaped, "update " WEB_INDEX " set idle=20 where id='%ld';", web_id);
				mysql_query(&m, query_escaped);
				break;
			}

		}

		/* pozrieme co poslal talker */
		result = mysql_result(&m, readquery);
		if (result) {
			if (mysql_num_rows(result) > 0) {
				//unsigned long *lens;
				while ((row = mysql_fetch_row(result))) {
					sprintf(query, "delete from " WEB_OUT " where id = '%ld' and lineid = '%s'", web_id, row[0]);
					mysql_query(&m, query);
					printf(row[1]);
				}
			}
			mysql_free_result(result);
		}
		else {
			//log("No input.\n");
		}

		/* pozrieme sa ci niesme quitnuti :) */
		result = mysql_result(&m, killquery);
		if (result) {
			if (mysql_num_rows(result) > 0) {
				row = mysql_fetch_row(result);
				if (row[0][0] == '3') {
					mysql_free_result(result);
					printf("Atlantis closed connection.\n");
					break;
				}
			}
			mysql_free_result(result);
		}

		pfd[0].revents = 0;
		pfd[1].revents = 0;
		pfd[2].revents = 0;
	}


	shutdown_database(&m);

	log("Mainloop dokoncene.\n");

	return 0;
}
