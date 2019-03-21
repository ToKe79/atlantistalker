#include "atl-head.h"
#include "atl-mydb.h"
#include "atl-xml.h"

char *colcom2[NUM_COLS]={
	/* Standard stuff */
	"RS",
	"OL",
	"UL",
	"LI",
	"RV",
	/* Foreground colour */
	"FK",
	"FR",
	"FG",
	"FY",
	"FB",
	"FM",
	"FT",
	"FW",
	/* Background colour */
	"BK",
	"BR",
	"BG",
	"BY",
	"BB",
	"BM",
	"BT",
	"BW",
	"HK",
	/* Highlighted foreground */
	"HR",
	"HG",
	"HY",
	"HB",
	"HM",
	"HT",
	"HW",
	/* Dark foreground (autoOLreset) */
	"DK",
	"DR",
	"DG",
	"DY",
	"DB",
	"DM",
	"DT",
	"DW",
	/* Beep */
	"LB"
};

char *colcode2[NUM_COLS]={
	/* Standard stuff */
	"\033[0m",
	"\033[1m",
	"\033[4m",
	"\033[5m",
	"\033[7m",
	/* Foreground colour */
	"\033[30m",
	"\033[31m",
	"\033[32m",
	"\033[33m",
	"\033[34m",
	"\033[35m",
	"\033[36m",
	"\033[37m",
	/* Background colour */
	"\033[40m",
	"\033[41m",
	"\033[42m",
	"\033[43m",
	"\033[44m",
	"\033[45m",
	"\033[46m",
	"\033[47m",
	/* Highlighted foreground */
	"\033[1m\033[30m",
	"\033[1m\033[31m",
	"\033[1m\033[32m",
	"\033[1m\033[33m",
	"\033[1m\033[34m",
	"\033[1m\033[35m",
	"\033[1m\033[36m",
	"\033[1m\033[37m",
	/* Dark foreground (autoOLreset) */
	"\033[0m\033[30m",
	"\033[0m\033[31m",
	"\033[0m\033[32m",
	"\033[0m\033[33m",
	"\033[0m\033[34m",
	"\033[0m\033[35m",
	"\033[0m\033[36m",
	"\033[0m\033[37m",
	/* Beep */
	"\007"
};

int init_database(void)
{
	if (mysql_init(&mysql)==NULL) {
		fprintf(stderr,"MYSQL: Nedostatok pamate pre vytvorenie objektu!\n");
		boot_exit(1);
	}
	if (!mysql_real_connect(&mysql,DBHOST,DBUSER,DBPASS,NULL,0,NULL,0)) {
		fprintf(stderr,"MYSQL: Zlyhalo pripojenie k databaze: %s\n",mysql_error(&mysql));
		boot_exit(1);
	}
	if (mysql_select_db(&mysql,DBNAME)) {
		fprintf(stderr,"MYSQL: Nemozno vybrat zakladnu databazu!\n");
		boot_exit(1);
	}
	sprintf(query,"truncate table `web_index`");
	mysql_kvery(query);
	sprintf(query,"truncate table `web_in`");
	mysql_kvery(query);
	sprintf(query,"truncate table `web_out`");
	mysql_kvery(query);
	return 1;
}

void shutdown_database(void)
{
	mysql_close(&mysql);
}

char *dbf_string(char *string)
{
	int poz=0;
	parsed[0]='\0';

	if (string==NULL)
		return "";
	while (*string && poz<8192){
		if (*string=='\'') {
			*(parsed+poz)='\\'; poz++;
			*(parsed+poz)='\''; poz++;
		}
		else if (*string=='"') {
			*(parsed+poz)='\\'; poz++;
			*(parsed+poz)='"'; poz++;
		}
		else if (*string=='\\') {
			*(parsed+poz)='\\'; poz++;
			*(parsed+poz)='\\'; poz++;
		}
		else {
			*(parsed+poz)=*string;
			poz++;
		}
		string++;
	}
	*(parsed+poz)='\0';
	return parsed;
}

MYSQL_RES *mysql_result(char *select)
{
	MYSQL_RES *loc_result;

	if (mysql_query(&mysql,select)) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		write_level(KIN,1,text,NULL);
		write_level(KIN,1,query,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		write_syslog(select,1);
		return NULL;
	}
	if ((loc_result=mysql_store_result(&mysql))==NULL) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: Unable to store result!\n");
		write_level(KIN,1,text,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		return NULL;
	}
	return loc_result;
}

int mysql_kvery(char *select)
{
	if (mysql_query(&mysql,select)) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		write_level(KIN,1,text,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		return 0;
	}
	return 1;
}

/*** Save a users stats ***/
extern int db_save_user_details(UR_OBJECT user, int save_current)
{
	int ch,lastloginlen,lastlogin,online=0;
	char *lastsite;
	char uid[15];
	char ppass[PASS_LEN*2+100],pdesc[DESC_CHAR_LEN*2+2];
	char pwizpass[PASS_LEN*2+100],pinphr[PHRASE_LEN*2+2],poutphr[PHRASE_LEN*2+2];
	char plogoutmsg[122],pprompt[PROMPT_LEN*2+4];
	char phomepage[HOMEPAGE_LEN*2+2];
	char shoutmsg[21],shoutmsg2[21],gossipmsg[21],gossipmsg2[21];
	char saymsg[21],saymsg2[21],tellmsg[21],tellmsg2[21];
	char wizshmsg[21],wizshmsg2[21];

	if (save_current==1)
		online=1;
	if (save_current==8)
		save_current=1;
	strcpy(ppass,dbf_string(user->pass));
	strcpy(pdesc,dbf_string(user->desc));
	strcpy(pwizpass,dbf_string(user->wizpass));
	strcpy(pinphr,dbf_string(user->in_phrase));
	strcpy(poutphr,dbf_string(user->out_phrase));
	strcpy(plogoutmsg,dbf_string(user->logoutmsg));
	strcpy(pprompt,dbf_string(user->prompt_string));
	strcpy(phomepage,dbf_string(user->homepage));
	strcpy(shoutmsg,dbf_string(user->shoutmsg));
	strcpy(shoutmsg2,dbf_string(user->shoutmsg2));
	strcpy(gossipmsg,dbf_string(user->gossipmsg));
	strcpy(gossipmsg2,dbf_string(user->gossipmsg2));
	strcpy(saymsg,dbf_string(user->saymsg));
	strcpy(saymsg2,dbf_string(user->saymsg2));
	strcpy(tellmsg,dbf_string(user->tellmsg));
	strcpy(tellmsg2,dbf_string(user->tellmsg2));
	strcpy(wizshmsg,dbf_string(user->wizshmsg));
	strcpy(wizshmsg2,dbf_string(user->wizshmsg2));
	/* strcpy(,dbf_string()); */
	if (user->type==CLONE_TYPE) return 0;
	/* Zlozi ignore cislo */
	user->ignore=0;
	if (user->ignshout)
		user->ignore+=1;
	if (user->igntell)
		user->ignore+=2;
	if (user->ignall)
		user->ignore+=4;
	if (user->ignsys)
		user->ignore+=8;
	if (user->ignfun)
		user->ignore+=16;
	if (!user->igngossip)
		user->ignore+=32;
	if (user->ignportal)
		user->ignore+=64;
	if (user->ignio)
		user->ignore+=128;
	if (user->ignlook)
		user->ignore+=256;
	if (user->ignbeep)
		user->ignore+=512;
	if (user->ignzvery)
		user->ignore+=1024;
	if (user->igncoltell)
		user->ignore+=2048;
	if (!user->ignportalis)
		user->ignore+=4096;
	if (!user->ignafktell)
		user->ignore+=8192;
	if (user->mailnotify)
		user->ignore+=16384;
	if (user->igngames)
		user->ignore+=32768;
	if (user->ignautopromote)
		user->ignore+=65536;
	if (user->ignxannel)
		user->ignore+=131072;
	if (user->ignnongreenhorn)
		user->ignore+=262144;
	user->switches=0;
	if (user->shortcutwarning)
		user->switches+=1;
	if (user->misc_op==33)
		user->switches+=2;
	if (user->mail2sms)
		user->switches+=4;
	if (user->com_priority)
		user->switches+=8;
	if (user->timeinrevt)
		user->switches+=16;
	if (user->ignblink)
		user->switches+=32;
	if (user->ignspell)
		user->switches+=64;
	if (user->ignpict)
		user->switches+=128;
	/* Okej, teraz to ideme tlacit do databazy... Huh! */
	if (save_current==1) {
		lastsite=user->site;
		lastlogin=(int)(time(0));
		lastloginlen=(int)(time(0)-user->last_login);
		if (lastloginlen>user->longestsession)
			user->longestsession=lastloginlen;
	}
	else {
		lastsite=user->last_site;
		lastlogin=(int)user->last_login;
		lastloginlen=user->last_login_len;
	}
	if (user->doom)
		user->lines+=D_LINES; /* D00M - o D_LINES je menej */
	if (user->statlcount)
		user->lines=user->statlcount; /* statline */
	ch=pohaluz(user);
	if (user->id>1000000000)
		uid[0]='\0';
	else
		sprintf(uid,"'%d',",user->id);
	sprintf(query,"REPLACE INTO `%s` (\
		%s                    `username`,        `passwd`,            `last_login`,         `total_login`,              `last_login_len`,\
		`read_mail`,          `level`,           `prompt`,            `muzzletime`,         `charmode_echo`,            `command_mode`,\
		`colour`,             `jailed`,          `sex`,               `pp`,                 `gold`,                     `sayswears`,\
		`ignore_bits`,        `lines_count`,     `wrap`,              `who_type`,           `rt_on_exit`,               `lastm`,\
		`autofwd`,            `mana`,            `pagewho`,           `examine`,            `first_login`,              `col0`,\
		`col1`,               `col2`,            `col3`,              `col4`,               `col5`,                     `col6`,\
		`lang`,               `goafkafter`,      `killmeafter`,       `ch`,                 `commused0`,                `commused1`,\
		`commused2`,          `commused3`,       `commused4`,         `commused5`,          `commused6`,                `commused7`,\
		`commused8`,          `commused9`,       `totaljailtime`,     `totalmuzzletime`,    `nontimejails`,             `killed`,\
		`commused10`,         `muzzled`,         `team`,              `shoutswears`,        `smsssent`,                 `smsday`,\
		`allowsms`,           `switches`,        `smsgate`,           `agecode`,            `longestsession`,           `idletime`,\
		`last_site`,          `user_desc`,       `in_phrase`,         `out_phrase`,         `email`,                    `homepage`,\
		`prompt_string`,      `room`,            `requestemail`,      `mobile`,             `wizpass`,                  `smsfollow`,\
		`logoutmsg`,          `online`,          `prevname`,          `shoutmsg`,           `shoutmsg2`,                `gossipmsg`,\
		`gossipmsg2`,         `saymsg`,          `saymsg2`,           `tellmsg`,            `tellmsg2`,                 `wizshmsg`,\
		`wizshmsg2`,          `request`,         `name2`,             `name3`,              `name4`\
		) VALUES (\
		%s                    '%s',              '%s',                FROM_UNIXTIME(%d),    FROM_UNIXTIME(%d),          '%d',\
		FROM_UNIXTIME(%d),    '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 FROM_UNIXTIME(%d),          '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%d',                 '%d',              '%d',                '%d',                 '%d',                       '%d',\
		'%s',                 '%s',              '%s',                '%s',                 '%s',                       '%s',\
		'%s',                 '%s',              '%s',                '%s',                 '%s',                       '%s',\
		'%s',                 '%d',              '%s',                '%s',                 '%s',                       '%s',\
		'%s',                 '%s',              '%s',                '%s',                 '%s',                       '%s',\
		'%s',                 '%s',              '%s',                '%s',                 '%s');",
		save_current==2?"repository":"users",
		uid[0]?"id,":"",
		uid,                  user->name,        ppass,               lastlogin,             (int)user->total_login,    lastloginlen,
		(int)user->read_mail, user->level,       user->prompt,        user->muzzletime,      user->charmode_echo,       user->command_mode,
		user->colour,         user->jailed,      user->sex,           user->pp,              user->gold,                user->sayswears,
		user->ignore,         user->lines,       user->wrap,          user->who_type,        user->rt_on_exit,          user->lastm,
		user->autofwd,        user->mana,        user->pagewho,       user->examine,         (int)user->first_login,    user->col[0],
		user->col[1],         user->col[2],      user->col[3],        user->col[4],          user->col[5],              user->col[6],
		user->lang,           user->goafkafter,  user->killmeafter,   ch,                    user->commused[0],         user->commused[1],
		user->commused[2],    user->commused[3], user->commused[4],   user->commused[5],     user->commused[6],         user->commused[7],
		user->commused[8],    user->commused[9], user->totaljailtime, user->totalmuzzletime, user->nontimejails,        user->killed,
		user->commused[10],   user->muzzled,     user->team,          user->shoutswears,     user->smsssent,            user->smsday,
		user->allowsms,       user->switches,    user->smsgate,       user->agecode,         (int)user->longestsession, (int)user->idletime,
		lastsite,             pdesc,             pinphr,              poutphr,               user->email,               phomepage,
		pprompt,              user->room->name,  user->requestemail,  user->mobile,          pwizpass,                  user->smsfollow,
		plogoutmsg,           online,            user->prevname,      user->shoutmsg,        user->shoutmsg2,           user->gossipmsg,
		user->gossipmsg2,     user->saymsg,      user->saymsg2,       user->tellmsg,         user->tellmsg2,            user->wizshmsg,
		user->wizshmsg2,      user->request,     user->sname[0],      user->sname[1],        user->sname[2]);
		if (mysql_query(&mysql,query)) {
			sprintf(text,"~OL~FR~BBSAVE_USER_STATS: Nepodarilo sa ulozit data uzivatela \"%s\":%s ~BK\n",user->name,mysql_error(&mysql));
			write_level(KIN,1,text,NULL);
			colour_com_strip(text);
			write_syslog(text,1);
			return 0;
		}
		if (user->statlcount)
			user->lines-=2; /* statline */
		if (user->doom)
			user->lines-=D_LINES; /* Obnoviime... */
		return 1;
}

int db_load_user_details(UR_OBJECT user,int repository)
{
	int i;

	user->name[USER_NAME_LEN]='\0';
	for (i=0;i<USER_NAME_LEN;i++)
		if (user->name[i]=='\'' || user->name[i]==';')
			user->name[i]=' ';
	sprintf(query,"SELECT \
		`passwd`,     UNIX_TIMESTAMP(`last_login`),  UNIX_TIMESTAMP(`total_login`), `last_login_len`, UNIX_TIMESTAMP(`read_mail`),\
		`level`,      `prompt`,                      `muzzletime`,                  `charmode_echo`,  `command_mode`,\
		`colour`,     `jailed`,                      `sex`,                         `pp`,             `gold`,\
		`sayswears`,  `ignore_bits`,                 `lines_count`,                 `wrap`,           `who_type`,\
		`rt_on_exit`, `lastm`,                       `autofwd`,                     `mana`,           `pagewho`,\
		`examine`,    UNIX_TIMESTAMP(`first_login`), `col0`,                        `col1`,           `col2`,\
		`col3`,       `col4`,                        `lang`,                        `goafkafter`,     `killmeafter`,\
		`ch`,         `commused0`,                   `commused1`,                   `commused2`,      `commused3`,\
		`commused4`,  `commused5`,                   `commused6`,                   `commused7`,      `commused8`,\
		`commused9`,  `totaljailtime`,               `totalmuzzletime`,             `nontimejails`,   `killed`,\
		`commused10`, `col5`,                        `muzzled`,                     `team`,           `shoutswears`,\
		`smsssent`,   `smsday`,                      `allowsms`,                    `switches`,       `smsgate`,\
		`agecode`,    `longestsession`,              `idletime`,                    `last_site`,      `user_desc`,\
		`in_phrase`,  `out_phrase`,                  `email`,                       `homepage`,       `prompt_string`,\
		`room`,       `requestemail`,                `mobile`,                      `wizpass`,        `smsfollow`,\
		`logoutmsg`,  `id`,                          `username`,                    `prevname`,       `shoutmsg`,\
		`shoutmsg2`,  `gossipmsg`,                   `gossipmsg2`,                  `saymsg`,         `saymsg2`,\
		`tellmsg`,    `tellmsg2`,                    `wizshmsg`,                    `wizshmsg2`,      `col6`,\
		`request`,    `name2`,                       `name3`,                       `name4` \
		FROM `%s` WHERE `username`='%s'",repository?"repository":"users",user->name);
	if (!(result=mysql_result(query)))
		return 0;
	if (!(row=mysql_fetch_row(result))) { /* no such user */
		mysql_free_result(result);
		return 0;
	}
	sstrncpy(user->pass,row[0],sizeof(user->pass)-1);
	user->last_login	=	(time_t)atol(row[1]);
	user->total_login	=	(time_t)atol(row[2]);
	user->read_mail		=	(time_t)atol(row[4]);
	user->last_login_len	=	atoi(row[3]);
	user->level		=	atoi(row[5]);
	user->prompt		=	atoi(row[6]);
	user->muzzletime	=	atoi(row[7]);
	user->charmode_echo	=	atoi(row[8]);
	user->command_mode	=	atoi(row[9]);
	user->colour		=	atoi(row[10]);
	user->jailed		=	atoi(row[11]);
	user->sex		=	atoi(row[12]);
	user->pp		=	atoi(row[13]);
	user->gold		=	atoi(row[14]);
	user->sayswears		=	atoi(row[15]);
	user->ignore		=	atoi(row[16]);
	user->lines		=	atoi(row[17]);
	user->wrap		=	atoi(row[18]);
	user->who_type		=	atoi(row[19]);
	user->rt_on_exit	=	atoi(row[20]);
	user->lastm		=	atoi(row[21]);
	user->autofwd		=	atoi(row[22]);
	user->mana		=	atoi(row[23]);
	user->pagewho		=	atoi(row[24]);
	user->examine		=	atoi(row[25]);
	user->first_login	=	(time_t)atol(row[26]);
	user->col[0]		=	atoi(row[27]);
	user->col[1]		=	atoi(row[28]);
	user->col[2]		=	atoi(row[29]);
	user->col[3]		=	atoi(row[30]);
	user->col[4]		=	atoi(row[31]);
	user->lang		=	atoi(row[32]);
	user->goafkafter	=	atoi(row[33]);
	user->killmeafter	=	atoi(row[34]);
	if (pohaluz(user)!=atoi(row[35]))
		user->gold=0; /* V) Okradli usera! parchanti! ;) */
	user->commused[0]	=	atoi(row[36]);
	user->commused[1]	=	atoi(row[37]);
	user->commused[2]	=	atoi(row[38]);
	user->commused[3]	=	atoi(row[39]);
	user->commused[4]	=	atoi(row[40]);
	user->commused[5]	=	atoi(row[41]);
	user->commused[6]	=	atoi(row[42]);
	user->commused[7]	=	atoi(row[43]);
	user->commused[8]	=	atoi(row[44]);
	user->commused[9]	=	atoi(row[45]);
	user->totaljailtime	=	atoi(row[46]);
	user->totalmuzzletime	=	atoi(row[47]);
	user->nontimejails	=	atoi(row[48]);
	user->killed		=	atoi(row[49]);
	user->commused[10]	=	atoi(row[50]);
	user->col[5]		=	atoi(row[51]);
	user->muzzled		=	atoi(row[52]);
	user->team		=	atoi(row[53]);
	user->shoutswears	=	atoi(row[54]);
	user->smsssent		=	atoi(row[55]);
	user->smsday		=	atoi(row[56]);
	user->allowsms		=	atoi(row[57]);
	user->switches		=	atoi(row[58]);
	user->smsgate		=	atoi(row[59]);
	user->agecode		=	atoi(row[60]);
	user->longestsession	=	atoi(row[61]);
	user->idletime		=	atoi(row[62]);
	if (user->goafkafter<3 || user->goafkafter>10)
		user->goafkafter=10;
	if (user->killmeafter<10 || user->killmeafter>120)
		user->killmeafter=120;
	if (user->killed<0)
		user->killed=0;
	if (user->totaljailtime<0)
		user->totaljailtime=0;
	if (user->nontimejails<0)
		user->nontimejails=0;
	if (user->col[5]<0)
		user->col[5]=0;
	if (user->muzzled<0)
		user->muzzled=0;
	if (user->team<0 || user->team>4)
		user->team=0;
	if (teamname[user->team][0]=='\0')
		user->team=0;
	if (user->allowsms==-1)
		user->allowsms=0;
	if (user->smsgate<0)
		user->smsgate=0;
	if (user->switches<0)
		user->switches=0;
	if (user->ignore>=262144) {
		user->ignnongreenhorn=1;
		user->ignore-=262144;
	}
	if (user->ignore>=131072) {
		user->ignxannel=1;
		user->ignore-=131072;
	}
	if (user->ignore>=65536) {
		user->ignautopromote=1;
		user->ignore-=65536;
	}
	if (user->ignore>=32768) {
		user->igngames=1;
		user->ignore-=32768;
	}
	if (user->ignore>=16384) {
		user->mailnotify=1;
		user->ignore-=16384;
	}
	if (user->ignore>=8192) {
		user->ignafktell=0;
		user->ignore-=8192;
	}
	if (user->ignore>=4096) {
		user->ignportalis=0;
		user->ignore-=4096; }
	if (user->ignore>=2048) {
		user->igncoltell=1;
		user->ignore-=2048;
	}
	if (user->ignore>=1024) {
		user->ignzvery=1;
		user->ignore-=1024;
	}
	if (user->ignore>=512) {
		user->ignbeep=1;
		user->ignore-=512;
	}
	if (user->ignore>=256) {
		user->ignlook=1;
		user->ignore-=256;
	}
	if (user->ignore>=128) {
		user->ignio=1;
		user->ignore-=128;
	}
	if (user->ignore>=64) {
		user->ignportal=1;
		user->ignore-=64;
	}
	if (user->ignore>=32) {
		user->igngossip=0;
		user->ignore-=32;
	}
	if (user->ignore>=16) {
		user->ignfun=1;
		user->ignore-=16;
	}
	if (user->ignore>=8) {
		user->ignsys=1;
		user->ignore-=8;
	}
	if (user->ignore>=4) {
		user->ignall=1;
		user->ignore-=4;
	}
	/* switch ingore all off */
	user->ignall=0;
	if (user->ignore>=2) {
		user->igntell=1;
		user->ignore-=2;
	}
	if (user->ignore>=1) {
		user->ignshout=1;
		user->ignore-=1;
	}
	if (user->switches>=128) {
		user->ignpict=1;
		user->switches-=128;
	}
	if (user->switches>=64) {
		user->ignspell=1;
		user->switches-=64;
	}
	if (user->switches>=32) {
		user->ignblink=1;
		user->switches-=32;
	}
	if (user->switches>=16) {
		user->timeinrevt=1;
		user->switches-=16;
	}
	if (user->switches>=8) {
		user->com_priority=1;
		user->switches-=8;
	}
	if (user->switches>=4) {
		user->mail2sms=1;
		user->switches-=4;
	}
	if (user->switches>=2) {
		user->wizactivity=10;
		user->switches-=2;
	}
	if (user->switches>=1) {
		user->shortcutwarning=1;
		user->switches-=1;
	}
	sstrncpy(user->last_site,row[63],sizeof(user->last_site)-1);
	sstrncpy(user->desc,row[64],sizeof(user->desc)-1);
	sstrncpy(user->in_phrase,row[65],sizeof(user->in_phrase)-1);
	sstrncpy(user->out_phrase,row[66],sizeof(user->out_phrase)-1);
	sstrncpy(user->email,row[67],sizeof(user->email)-1);
	sstrncpy(user->homepage,row[68],sizeof(user->homepage)-1);
	sstrncpy(user->prompt_string,row[69],sizeof(user->prompt_string)-1);
	if ((user->room=get_room(row[70],NULL))==NULL) user->room=room_first;
	sstrncpy(user->requestemail,row[71],sizeof(user->requestemail)-1);
	sstrncpy(user->mobile,row[72],sizeof(user->mobile)-1);
	if (strlen(user->mobile)==10) {
		user->mobile[0]='1';
		sprintf(texthb,"+42%s",user->mobile);
		strcpy(user->mobile,texthb);
	}
	sstrncpy(user->wizpass,row[73],sizeof(user->wizpass)-1);
	sstrncpy(user->smsfollow,row[74],sizeof(user->smsfollow)-1);
	sstrncpy(user->logoutmsg,row[75],sizeof(user->logoutmsg)-1);
	user->id=atoi(row[76]);
	sstrncpy(user->name,row[77],sizeof(user->name)-1);
	sstrncpy(user->prevname,row[78],sizeof(user->prevname)-1);
	sstrncpy(user->shoutmsg,row[79],sizeof(user->shoutmsg)-1);
	sstrncpy(user->shoutmsg2,row[80],sizeof(user->shoutmsg2)-1);
	sstrncpy(user->gossipmsg,row[81],sizeof(user->gossipmsg)-1);
	sstrncpy(user->gossipmsg2,row[82],sizeof(user->gossipmsg2)-1);
	sstrncpy(user->saymsg,row[83],sizeof(user->saymsg)-1);
	sstrncpy(user->saymsg2,row[84],sizeof(user->saymsg2)-1);
	sstrncpy(user->tellmsg,row[85],sizeof(user->tellmsg)-1);
	sstrncpy(user->tellmsg2,row[86],sizeof(user->tellmsg2)-1);
	sstrncpy(user->wizshmsg,row[87],sizeof(user->wizshmsg)-1);
	sstrncpy(user->wizshmsg2,row[88],sizeof(user->wizshmsg2)-1);
	user->col[6]=atoi(row[89]);
	sstrncpy(user->request,row[90],sizeof(user->request)-1);
	sstrncpy(user->sname[0],row[91],sizeof(user->sname[0])-1);
	sstrncpy(user->sname[1],row[92],sizeof(user->sname[1])-1);
	sstrncpy(user->sname[2],row[93],sizeof(user->sname[2])-1);
	mysql_free_result(result);
	if (user->agecode>1)
		user->age=get_age(user->agecode);
	else
		user->age=0;
	load_irc_details(user);
	colour_com_strip(user->homepage); /* DOCASNE */
	colour_com_strip(user->email); /* DOCASNE */
	if (check_iophrase(user->in_phrase)) {
		sprintf(text,"%s sem", pohl(user,"vstupil","vstupila"));
		sstrncpy(user->in_phrase, text, PHRASE_LEN);
	}
	if (check_iophrase(user->out_phrase)) {
		sprintf(text,"nas %s", pohl(user,"opustil","opustila"));
		sstrncpy(user->out_phrase, text, PHRASE_LEN);
	}
	if (strstr(user->desc,"nema nastaveny") && user->level>1) {
		strcpy(user->desc,"spekuluje s .desc");
	}
	return 1;
}

int db_load_user_password(UR_OBJECT user)
{
	sprintf(query,"SELECT `passwd`,`level` FROM `users` WHERE `username`='%s'",user->name);
	if (!(result=mysql_result(query)))
		return 0; /* bad select or something */
	if (!(row=mysql_fetch_row(result))) { /* no such user */
		mysql_free_result(result);
		return 0;
	}
	sstrncpy(user->pass,row[0],sizeof(user->pass)-1);
	user->level=atoi(row[1]);
	mysql_free_result(result);
	return 1;
}

int db_load_user_level(char *username)
{
	int level;

	sprintf(query,"SELECT `level` FROM `users` WHERE `username`='%s'",username);
	if (!(result=mysql_result(query)))
		return -2; /* bad select or something */
	if (!(row=mysql_fetch_row(result))) {         /* no such user */
		mysql_free_result(result);
		return -1;
	}
	level=atoi(row[0]);
	mysql_free_result(result);
	return level;
}

int db_userid(char *username)
{
	int uid;

	sprintf(query,"SELECT `id` FROM `users` WHERE `username`='%s'",username);
	if (!(result=mysql_result(query)))
		return -2; /* bad select or something */
	if (!(row=mysql_fetch_row(result))) {         /* no such user */
		mysql_free_result(result);
		return -1;
	}
	uid=atoi(row[0]);
	mysql_free_result(result);
	return uid;
}

int db_user_exists(char *username)
{
	int i;
	username[USER_NAME_LEN]='\0';
	for (i=0;i<USER_NAME_LEN;i++)
		if (username[i]=='\'' || username[i]==';')
			username[i]=' ';
	sprintf(query,"SELECT `username` FROM `users` WHERE `username`='%s'",username);
	if (!(result=mysql_result(query))) return 0; /* bad select or something */
	if ((row=mysql_fetch_row(result)) && row[0]) {
		mysql_free_result(result);
		return 1;
	}
	mysql_free_result(result);
	return 0;
}

char *db_user_name(char *username)
{
	int i;
	static char sensitivename[USER_NAME_LEN+2];

	sensitivename[0]='\0';
	username[USER_NAME_LEN]='\0';
	for (i=0;i<USER_NAME_LEN;i++)
		if (username[i]=='\'' || username[i]==';')
			username[i]=' ';
	sprintf(query,"SELECT `username` FROM `users` WHERE `username`='%s'",username);
	if (!(result=mysql_result(query))) return sensitivename; /* bad select or something */
	if ((row=mysql_fetch_row(result)) && row[0])
		strcpy(sensitivename,row[0]);
	mysql_free_result(result);
	return sensitivename;
}

char *db_username_by_id(int uid)
{
	static char name[USER_NAME_LEN+2];

	name[0]='\0';
	sprintf(query,"SELECT `username` FROM `users` WHERE `id`='%d'",uid);
	if (!(result=mysql_result(query)))
		return name; /* bad select or something */
	if ((row=mysql_fetch_row(result)) && row[0])
		strcpy(name,row[0]);
	mysql_free_result(result);
	return name;
}

extern int db_deluser(char *username)
{
	int i;

	username[USER_NAME_LEN]='\0';
	for (i=0;i<USER_NAME_LEN;i++)
		if (username[i]=='\'' || username[i]==';')
			username[i]=' ';
	sprintf(query,"DELETE FROM `users` WHERE `username`='%s'",username);
	if (mysql_query(&mysql,query))
		return 1;
	return 0;
}

void query_com(UR_OBJECT user,char *inpstr)
{
	FILE *fp;
	MYSQL_RES *result;
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	char filename[81];
	int i;
	int num;

	if (user!=NULL) {
		if (word_count<2) {
			write_user(user,"Pouzi: .query SELECT ...\n");
			return;
		}
		strtoupper(word[1]);
		if (strcmp(word[1],"SELECT")) {
			write_user(user,"~OL~FRObmedzene na 'SELECT'!\n");
			return;
		}
		sprintf(filename,"%s%c%s%s",TMPFOLDER,DIRSEP,user->name,TMPSUFFIX);
		if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
			write_user(user,"Chyba: problem s otvaranim docasneho suboru.\n");
			return;
		}
		if (mysql_query(&mysql,inpstr)) {
			sprintf(text,"~OL~FRMySQL error: %s\n",mysql_error(&mysql));
			write_user(user,text);
			fclose(fp);
			return;
		}
		if ((result=mysql_store_result(&mysql))==NULL) {
			write_user(user,"~OL~FRMySQL error: Nemozno ulozit vysledok query!\n");
			fclose(fp);
			return;
		}
		if (mysql_num_rows(result)==0) {
			write_user(user,"~OLPrazdny vysledok.\n");
			fclose(fp);
			mysql_free_result(result);
			return;
		}
		fputs("~OL",fp);
		while ((field=mysql_fetch_field(result))) {
			fputs(field->name,fp);
			fputs("\t",fp);
		}
		fputs("\n",fp);
		while ((row=mysql_fetch_row(result))) {
			num=mysql_num_fields(result);
			i=0;
			while (row[i]!=NULL && i<num) {
				fputs(row[i],fp);
				fputs("\t",fp);
				i++;
			}
			fputs("\n",fp);
		}
		num=mysql_num_rows(result);
		sprintf(text,"~OL%d %s.\n",num,skloncislo(num,"riadok","riadky","riadkov"));
		fputs(text,fp);
		fclose(fp);
		mysql_free_result(result);
		switch (more(user,user->socket,filename)) {
			case 0:
				write_user(user,"Chyba pri citani resultu!\n");
				break;
			case 1:
				user->misc_op=2;
		}
	}
}

int note_vote(UR_OBJECT user)
{
	int votes=0,possible=0;
	int has_to_vote=0;

	sprintf (query,"select `id` from `vote_i` where `level`<'%d' and `closed`=0;",user->level+1);
	if (!(result=mysql_result(query))) {
		return 0;
	}
	possible=mysql_num_rows(result);
	mysql_free_result(result);
	sprintf (query,"select `id` from `vote_i`,`vote_v` where `level`<'%d' and `closed`=0 and `userid`='%d' and `voteid`=`id` order by (`level`>6);",user->level+1,user->id);
	if (!(result=mysql_result(query)))
		return 0;
	votes=mysql_num_rows(result);
	mysql_free_result(result);
	has_to_vote=possible-votes;
	return has_to_vote;
}

void votenew(UR_OBJECT user)
{
	char label[21];
	int id,total=0,already_voted=0,choice=0,closed=0,wizz=0,level=0;
	int uvoted[101],cnt=0,i;

	if (word_count==1) {
		sprintf (query,"select `id` from `vote_i`,`vote_v` where `level`<'%d' and `closed`=0 and `userid`='%d' and `voteid`=`id` order by (`level`>6);",user->level+1,user->id);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		i=0;
		while (cnt<100 && (row=mysql_fetch_row(result))) {
			if (row[0]!=NULL) {
				uvoted[cnt]=atoi(row[0]);
				cnt++;
			}
		}
		mysql_free_result(result);
		sprintf (query,"select `label`,`description`,`level`,`closed`,`id` from `vote_i` where `level`<'%d' and `closed`<2 order by (`level`>6),(1-`closed`),`id`;",user->level+1);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		while ((row=mysql_fetch_row(result))) {
			if (row[0]!=NULL) {
				closed=atoi(row[3]);
				id=atoi(row[4]);
				level=atoi(row[2]);
				if (closed==1 && total==0) {
					write_user(user,"~OL~FY+-~FWUzavrete hlasovania:~FY---------------------------------------------------------+\n");
					total=1;
				}
				if (total<2 && closed==0) {
					write_user(user,"~OL~FY+-~FWAktualne hlasovania:~FY---------------------------------------------------------+\n");
					total=2;
				}
				if (wizz==0 && level>6) {
					write_user(user,"~OL~FY+-~FWKralovske hlasovania:~FY--------------------------------------------------------+\n");
					wizz=1;
				}
				already_voted=0;
				if (closed==0) {
					for(i=0;i<cnt;i++)
						if (id==uvoted[i])
							already_voted=1;
				}
				sprintf(text," %s.vote %-16s~OL~FY| ~OL~FT%s\n",(closed || already_voted)?"~FG":"~OL~FG",row[0],row[1]?row[1]:"");
				write_user(user,text);
			}
		}
		mysql_free_result(result);
		if (total==0)
			write_user(user,"Momentalne neprebieha ziadne hlasovanie.\n");
		else
			write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		return;
	}
	if (word_count==2) {
		sprintf (query,"select `id`,`question`,`label`,`closed` from `vote_i` where `level`<'%d' and `label` LIKE '%s%%' and `closed`<2 order by `id`;",user->level+1,word[1]);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		if((row=mysql_fetch_row(result))) {
			id=atoi(row[0]);
			if (row[1])
				strncpy(text,row[1],T_SIZE-10);
			if (row[2])
				strncpy(label,row[2],20);
			closed=atoi(row[3]);
			label[20]='\0';
			mysql_free_result(result);
		}
		else {
			write_user(user,"Pouzi: .vote <tema hlasovania>\n");
			mysql_free_result(result);
			return;
		}
		if (!closed) {
			sprintf (query,"select `userid` from `vote_v` where `userid`='%d' and `voteid`='%d';",user->id,id);
			if (!(result=mysql_result(query))) {
				write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
				return;
			}
			if (mysql_num_rows(result))
				already_voted=1;
			mysql_free_result(result);
		}
		else
			already_voted=1;
		strcpy(texthb,"\n~OL~FW");
		strcat(texthb,text);
		write_user(user,texthb);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		sprintf (query,"select `optionid`,`votes`,`opt` from `vote_o` where `id`='%d' order by `optionid`;",id);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		while ((row=mysql_fetch_row(result))) {
			if (row[2]!=NULL) {
				if (already_voted || closed)
					sprintf(text,"~OL~FG .vote %s %2s (~FW%4s~FG %-6s) ~FY|~FW %s\n",label,row[0],row[1],skloncislo(atoi(row[1]),"hlas","hlasy","hlasov"),row[2]);
				else
					sprintf(text,"~OL~FG .vote %s %2s ~FY|~FW %s\n",label,row[0],row[2]);
				write_user(user,text);
				total+=atoi(row[1]);
			}
		}
		mysql_free_result(result);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		sprintf(texthb,"~FW%d~FR %s.",total,skloncislo(total,"uzivatel","uzivatelia","uzivatelov"));
		sprintf(text,"~OL~FY| ~FRHlasovanie je ANONYMNE. Zatial sa do hlasovania zapojilo %-26s~FY|\n",texthb);
		write_user(user,text);
		if (closed)
			sprintf(text,"~OL~FY| ~FW%-77s~FY|\n","Hlasovanie je uzavrete.");
		else
			sprintf(text,"~OL~FY| ~FW%-77s~FY|\n",already_voted?pohl(user,"Uz si hlasoval. ","Uz si hlasovala."):pohl(user,"Este si nehlasoval.","Este si nehlasovala."));
		write_user(user,text);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
	}
	if (word_count==3) {
		sprintf (query,"select `id`,`question`,`label`,`closed` from `vote_i` where `level`<'%d' and `label` LIKE '%s%%' and `closed`<2 order by `id`;",user->level+1,word[1]);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		if((row=mysql_fetch_row(result))) {
			id=atoi(row[0]);
			if (row[1])
				strncpy(text,row[1],T_SIZE);
			if (row[2])
				strncpy(label,row[2],20);
			closed=atoi(row[3]);
			label[20]='\0';
			mysql_free_result(result);
		}
		else {
			write_user(user,"Takato tema hlasovania neexistuje!\n");
			mysql_free_result(result);
			return;
		}
		if (closed) {
			write_user(user,"Hlasovanie uz je uzavrete, mozes si len pozriet vysledky.\n");
			return;
		}
		sprintf (query,"select `userid` from `vote_v` where `userid`='%d' and `voteid`='%d';",user->id,id);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		if (mysql_num_rows(result))
			already_voted=1;
		mysql_free_result(result);
		if (already_voted) {
			sprintf(text,"Ved uz si hlasoval%s!\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		choice=atoi(word[2]);
		sprintf (query,"select `opt` from `vote_o` where `id`='%d' and `optionid`='%d';",id,choice);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba 1 pri citani hlasovacieho listka.\n");
			return;
		}
		if((row=mysql_fetch_row(result))) {
			sprintf(texthb,"Dakujeme za Tvoj hlas! ==> %s\n",row[0]?row[0]:"");
			mysql_free_result(result);
			sprintf (query,"insert into `vote_v` values('%d','%d');",user->id,id);
			if (!mysql_kvery(query)) {
				write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
				return;
			}
			sprintf (query,"update `vote_o` set `votes`=`votes`+1 where `id`='%d' and `optionid`='%d';",id,choice);
			if (!mysql_kvery(query)) {
				write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
				return;
			}
			already_voted=1;
		}
		else {
			sprintf(texthb,"Zvolil%s si neexistujucu moznost!\n",pohl(user,"","a"));
			mysql_free_result(result);
		}
		strcat(text,"\n");
		write_user(user,text);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		sprintf (query,"select `optionid`,`votes`,`opt` from `vote_o` where `id`='%d' order by `optionid`;",id);
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani hlasovacieho listka.\n");
			return;
		}
		while ((row=mysql_fetch_row(result))) {
			if (row[2]!=NULL) {
				if (already_voted)
					sprintf(text,"~OL~FG .vote %s %s (~FW%4s~FG %-6s) ~FY|~FW %s\n",label,row[0],row[1],skloncislo(atoi(row[1]),"hlas","hlasy","hlasov"),row[2]);
				else
					sprintf(text,"~OL~FG .vote %s %s ~FY|~FW %s\n",label,row[0],row[2]);
				write_user(user,text);
				total+=atoi(row[1]);
			}
		}
		mysql_free_result(result);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		sprintf(query,"~FW%d~FR %s.",total,skloncislo(total,"uzivatel","uzivatelia","uzivatelov"));
		sprintf(text,"~OL~FY| ~FRHlasovanie je ANONYMNE. Zatial sa do hlasovania zapojilo %-26s~FY|\n",query);
		write_user(user,text);
		write_user(user,"~OL~FY+------------------------------------------------------------------------------+\n");
		write_user(user,texthb);
	}
}

void winners(UR_OBJECT user,int type_def,int pocet)
{
	int kolko,type=0,i=0,current=0,mode=0;
	char label[30],lab[10];

	if (type_def==0) {
		if (word_count<2) {
			write_user(user,"Pouzi: .winners <arena | piskvorky | hangman | doom | letun\n");
			write_user(user,"                 quest | reversi | dama | miny> [n]\n");
			write_user(user,"       [n] moze byt cislo od 1 po 500, standardne 10.\n");
			return;
		}
		if (!(kolko=atoi(word[2])))
			kolko=10;
		if (kolko>500) {
			write_user(user,"Cislo musi byt z rozsahu 1-500!\n");
			return;
		}
		strtolower(word[1]);
		if (!strncmp(word[1],"ar",2)) {
			type=DB_ARENA;
			mode=1;
			strcpy(lab,"arena");
			strcpy(label,"bojovnikov");
		}
		if (!strncmp(word[1],"pi",2)) {
			type=DB_PISKVORKY;
			strcpy(lab,"pisk");
			strcpy(label,"piskvorkarov");
		}
		if (!strncmp(word[1],"ha",2)) {
			type=DB_HANGMAN;
			strcpy(lab,"hang");
			strcpy(label,"obesencov");
		}
		if (!strncmp(word[1],"qu",2)) {
			type=DB_QUEST;
			mode=1;
			strcpy(lab,"quest");
			strcpy(label,"questerov");
		}
		if (!strncmp(word[1],"do",2)) {
			type=DB_DOOM;
			strcpy(lab,"doom");
			strcpy(label,"zabijakov");
		}
		if (!strncmp(word[1],"le",2)) {
			type=DB_LETUN;
			mode=2;
			strcpy(lab,"fly");
			strcpy(label,"pilotov");
		}
		if (!strncmp(word[1],"re",2)) {
			type=DB_REVERSI;
			strcpy(lab,"reve");
			strcpy(label,"reverzistov");
		}
		if (!strncmp(word[1],"da",2)) {
			type=DB_DAMA;
			strcpy(lab,"dama");
			strcpy(label,"hracov damy");
		}
		if (!strncmp(word[1],"mi",2)) {
			type=DB_MINY;
			strcpy(lab,"miny");
			strcpy(label,"hladacov");
		}
		if (!strncmp(word[1],"br",2)) {
			type=DB_BRUTALIS;
			strcpy(lab,"brut");
			strcpy(label,"masovych vrahov");
		}
		if (type==0) {
			write_user(user,"Pouzi: .winners <arena | piskvorky | hangman | doom | letun\n");
			write_user(user,"                 quest | reversi | dama | miny> [n]\n");
			write_user(user,"       [n] moze byt cislo od 1 po 99, standardne 10.\n");
			return;
		}
	}
	else {
		type=type_def;
		kolko=pocet;
	}
	texthb[0]='\0';
	if (mode==1) sprintf(query,"select `users`.`username`,`winners`.`%s` from `winners`,`users` where `users`.`id`=`winners`.`userid` order by `%s` desc limit %d;",lab,lab,kolko);
	else if (mode==2) sprintf(query,"select `users`.`username`,`fly_w`,`fly_l`,`fly_p`,`fly_s` from `winners`,`users` where `users`.`id`=`winners`.`userid` order by `fly_s` desc limit %d;",kolko);
	else {
		if (type==DB_BRUTALIS)
			sprintf(query,"select `users`.`username`,`winners`.`%s_w`,`winners`.`%s_l` from `winners`,`users` where `users`.`id`=`winners`.`userid` and (`%s_w`-`%s_l`<>0) order by (`%s_w`-`%s_l`) desc limit %d;",lab,lab,lab,lab,lab,lab,kolko);
		else
			sprintf(query,"select `users`.`username`,`winners`.`%s_w`,`winners`.`%s_l` from `winners`,`users` where `users`.`id`=`winners`.`userid` order by (`%s_w`-`%s_l`) desc limit %d;",lab,lab,lab,lab,kolko);
	}
	if (!(result=mysql_result(query))) {
		write_user(user,"Nastala chyba pri citani tabulky.\n");
		return;
	}
	kolko=mysql_num_rows(result);
	while ((row=mysql_fetch_row(result))) {
		if (row[0]!=NULL) {
			i++;
			if (i==1) {
				if (mode==1) {
					sprintf(text,"~OL~FB=~RS~FTTabulka %d najlepsich %s~OL~FB-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n",kolko,label);
					text[53]='\n';text[54]='\0';
				}
				else {
					sprintf(text,"~OL~FB=-=-=-=~RS~FTTabulka %d najlepsich %s~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n",kolko,label);
					text[65+mode*5]='\n';
					text[66+mode*5]='\0';
				}
				write_user(user,text);
				if (mode==1)
					write_user(user,"~OL~FB| ~FWtop~FB |     ~FWmeno~FB     |   ~FWskore ~FB   |\n~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
				else if (mode==2)
					write_user(user,"~OL~FB| ~FWtop~FB | ~FWmeno pilota~FB  | ~FWuspech~FB | ~FWneuspech~FB | ~FWpal. ~FB| ~FWskore ~FB|\n~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
				else
					write_user(user,"~OL~FB| ~FWtop~FB |     ~FWmeno~FB     | ~FWvyhry~FB | ~FWprehry~FB | ~FWskore ~FB|\n~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
			}
			if (mode==1)
				sprintf(text,"~OL~FB|~RS~FW%4d ~OL~FB| ~FY%s%-12s~FB | ~FW%7s~FB    |\n",i,strcmp(row[0],user->name)?"":"~FR",row[0],row[1]);
			else if (mode==2)
				sprintf(text,"~OL~FB|~RS~FW%4d ~OL~FB| ~FY%s%-12s~FB |~RS~FW%6s~OL~FB  |~RS~FW%7s~OL~FB   |~RS~FW%5s~OL~FB |~FW%6s ~FB|\n",i,strcmp(row[0],user->name)?"":"~FR",row[0],row[1],row[2],row[3],row[4]);
			else
				sprintf(text,"~OL~FB|~RS~FW%4d ~OL~FB| ~FY%s%-12s~FB |~RS~FW%6s~OL~FB |~RS~FW%7s~OL~FB |~FW%6d~FB |\n",i,strcmp(row[0],user->name)?"":"~FR",row[0],row[1],row[2],atoi(row[1])-atoi(row[2]));
			write_user(user,text);
			if (!strcmp(row[0],user->name))
				current=1;
		}
	}
	mysql_free_result(result);
	if (i==0)
		write_user(user,"Tabulka je prazdna.\n");
	else {
		if (mode==1)
			write_user(user,"~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
		else if (mode==2)
			write_user(user,"~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
		else
			write_user(user,"~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n");
		if (!current) {
			if (mode==1)
				sprintf(query,"select `%s` from `winners` where `userid`='%d';",lab,user->id);
			else if (mode==2)
				sprintf(query,"select `fly_w`,`fly_l`,`fly_p`,`fly_s` from `winners` where `userid`='%d';",user->id);
			else
				sprintf(query,"select `winners`.`%s_w`,`winners`.`%s_l` from `winners` where `userid`='%d';",lab,lab,user->id);
			if (!(result=mysql_result(query)))
				return;
			if ((row=mysql_fetch_row(result))) {
				if (mode==1)
					sprintf(text,"~OL~FB|     | ~FR%-12s~FB | ~FW%7s~FB    |\n~OL~FB|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|\n",user->name,row[0]);
				else if (mode==2)
					sprintf(text,"~OL~FB|     | ~FR%-12s~FB |~RS~FW%6s~OL~FB  |~RS~FW%7s~OL~FB   |~RS~FW%5s~OL~FB |~FW%6s ~FB|\n~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n",user->name,row[0],row[1],row[2],row[3]);
				else
					sprintf(text,"~OL~FB|     | ~FR%-12s~FB |~RS~FW%6s~OL~FB |~RS~FW%7s~OL~FB |~FW%6d~FB |\n~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n",user->name,row[0],row[1],atoi(row[0])-atoi(row[1]));
				write_user(user,text);
			}
			mysql_free_result(result);
		}
	}
}

/* arena_check() - zistuje fight-pointy pre daneho usera */
int arena_check(UR_OBJECT user)
{
	int points;

	sprintf(query,"select `arena` from `winners` where `userid`='%d';",user->id);
	if (!(result=mysql_result(query)))
		return 0;
	if ((row=mysql_fetch_row(result))) {
		points=atoi(row[0]);
		mysql_free_result(result);
		return points;
	}
	mysql_free_result(result);
	return 0;
}

void add_point(UR_OBJECT user,int type,int amount,int fuel)
{
	int wins=0,losts=0;
	int arena=0,quest=0,pisk_w=0,pisk_l=0,hang_w=0,hang_l=0,doom_w=0,doom_l=0,reve_w=0,reve_l=0,dama_w=0,dama_l=0,miny_w=0,miny_l=0,brut_w=0,brut_l=0,fly_w=0,fly_l=0,fly_p=0,fly_s=0;

	if (type==DB_ARENA)
		user->ap+=amount;
	if (amount<0)
		losts=-amount;
	else wins=amount;
	sprintf(query,"select `arena`,`quest`,`pisk_w`,`pisk_l`,`hang_w`,`hang_l`,`doom_w`,`doom_l`,`reve_w`,`reve_l`,`dama_w`,`dama_l`,`miny_w`,`miny_l`,`brut_w`,`brut_l`,`fly_w`,`fly_l`,`fly_p`,`fly_s` from `winners` where `userid`='%d';",user->id);
	if (!(result=mysql_result(query)))
		return;
	if ((row=mysql_fetch_row(result))) {
		arena=atoi(row[0]);
		quest=atoi(row[1]);
		pisk_w=atoi(row[2]);
		pisk_l=atoi(row[3]);
		hang_w=atoi(row[4]);
		hang_l=atoi(row[5]);
		doom_w=atoi(row[6]);
		doom_l=atoi(row[7]);
		reve_w=atoi(row[8]);
		reve_l=atoi(row[9]);
		dama_w=atoi(row[10]);
		dama_l=atoi(row[11]);
		miny_w=atoi(row[12]);
		miny_l=atoi(row[13]);
		brut_w=atoi(row[14]);
		brut_l=atoi(row[15]);
		fly_w=atoi(row[16]);
		fly_l=atoi(row[17]);
		fly_p=atoi(row[18]);
		fly_s=atoi(row[19]);
	}
	mysql_free_result(result);
	if (type==DB_PISKVORKY) {
		pisk_w+=wins;
		pisk_l+=losts;
	}
	else if (type==DB_HANGMAN) {
		hang_w+=wins;
		hang_l+=losts;
	}
	else if (type==DB_DOOM) {
		doom_w+=wins;
		doom_l+=losts;
	}
	else if (type==DB_REVERSI) {
		reve_w+=wins;
		reve_l+=losts;
	}
	else if (type==DB_DAMA) {
		dama_w+=wins;
		dama_l+=losts;
	}
	else if (type==DB_MINY) {
		miny_w+=wins;
		miny_l+=losts;
	}
	else if (type==DB_BRUTALIS) {
		brut_w+=wins;
		brut_l+=losts;
	}
	else if (type==DB_ARENA) {
		arena+=wins;
	}
	else if (type==DB_QUEST) {
		quest+=wins;
	}
	else if (type==DB_LETUN) {
		fly_w+=wins;
		fly_l+=losts;
		if (fuel>fly_p)
			fly_p=fuel;
		if ((fly_p-90)<0)
			fly_s=fly_w-fly_l;
		else
			fly_s=(fly_w-fly_l)+((fly_p-90)*(fly_p-90));
	}
	sprintf(query,"replace into `winners` (`userid`,`arena`,`quest`,`pisk_w`,`pisk_l`,`hang_w`,`hang_l`,`doom_w`,`doom_l`,`reve_w`,`reve_l`,`dama_w`,`dama_l`,`miny_w`,`miny_l`,`brut_w`,`brut_l`,`fly_w`,`fly_l`,`fly_p`,`fly_s`) values ('%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d');", user->id,arena,quest,pisk_w,pisk_l, hang_w,hang_l,doom_w,doom_l,reve_w,reve_l,dama_w,dama_l, miny_w,miny_l,brut_w,brut_l,fly_w,fly_l,fly_p,fly_s);
	mysql_kvery(query);
}

void handle_webinput(void)
{
	UR_OBJECT user,next;
	char inpstr[ARR_SIZE+1];

	user=user_first;
	while (user!=NULL) {
		inyjefuc=0;
		wrtype=0;
		next=user->next;
		if (user->type!=USER_TYPE || user->socket<1000) {
			user=next;
			continue;
		}
		if (user->websecond!=tsec) {
			user->websecond=tsec;
			strncpy(inpstr,get_web_input(user),sizeof(inpstr)-3);
			if (inpstr[0])
				got_line(user,inpstr);
		}
		if (inyjefuc)
			user=user->next;
		else
			user=next;
	}
	dump_all_users_data();
}

char *get_web_input(UR_OBJECT user)
{
	int lineid=0;
	static char ret[ARR_SIZE+1];

	ret[0]='\0';
	if (user->socket<1000)
		return ret;
	sprintf(query,"select `line`,`lineid` from `web_in` where `id`='%d' order by `lineid` limit 1;",user->socket-1000);
	if (!(result=mysql_result(query)))
		return ret;
	if ((row=mysql_fetch_row(result))) {
		strncpy(ret,row[0],ARR_SIZE);
		lineid=atoi(row[1]);
		if (!ret[0])
			strcpy(ret,"\n");
	}
	mysql_free_result(result);
	sprintf(query,"delete from `web_in` where `id`='%d' and `lineid`='%d' limit 1;",user->socket-1000,lineid);
	mysql_kvery(query);
	return ret;
}

void write_web(int socket,char *str)
{
	sprintf(query,"insert into `web_out` (`id`,`line`) values ('%d','%s');",socket-1000,dbf_string(str));
	mysql_kvery(query);
}

void eliminate_webuser(int socket)
{
	sprintf(text,"%c",223);
	write_web(socket,text); /* chcipni web klient */
	sprintf(query,"update `web_index` set `status`=3 where `id`='%d';",socket-1000);
	mysql_kvery(query);
}

void do_webusers()
{
	int id=0;
	UR_OBJECT user,create_user();
	int pocet_loginov;
	char filename[200],site[81],ipcka[20];
	int port=0;
	UR_OBJECT u;
	FILE *fp;

	sprintf(query,"select `id`,`ip`,`site`,`port` from `web_index` where `status`=1;");
	if (!(result=mysql_result(query)))
		return;
	while ((row=mysql_fetch_row(result))) {
		id=atoi(row[0]);
		strncpy(site,row[2],sizeof(site)-1);
		strncpy(ipcka,row[1],sizeof(ipcka)-1);
		port=atoi(row[3]);
		if (site_banned(site)) {
			write_web(id,"~OL~FRPrepac, z tejto adresy momentalne nie je mozne prihlasovanie.\n");
			mysql_free_result(result);
			eliminate_webuser(id);
			return;
		}
		pocet_loginov=0;
		for(u=user_first;u!=NULL;u=u->next) {
			if (u->login)
				if (!strcmp(u->site,site))
					pocet_loginov++;
		}
		if (pocet_loginov>=MAX_POCET_LOGINOV) {
			mysql_free_result(result);
			eliminate_webuser(id);
			sprintf(filename,"%s%c%s",MISCFILES,DIRSEP,SITEBAN);
			if (!(fp=ropen(filename,"a"))) {
				return;
			}
			fprintf(fp,"%s\n",site);
			fclose(fp);
			sprintf(text,"Adresa %s bola automaticky vybanovana! (web flood)\n",site);
			write_syslog(text,1);
			return;
		}
		sprintf(text,"%c%c",222,200);
		write_web(id+1000,text);
		write_web(id+1000,motd);
		sprintf(text,"%c%c",222,200);
		write_web(id+1000,text);
		if (num_of_users+num_of_logins>=max_users) {
			write_web(id+1000,"~OL~FRPrepac, talker je momentalne plny.\n");
			sprintf(text,"~OL~FRMaximalny pocet naraz prihlasenych pouzivatelov je %d. Skus neskor!\n\n",max_users);
			write_web(id+1000,text);
			mysql_free_result(result);
			eliminate_webuser(id+1000);
			return;
		}
		if ((user=create_user())==NULL) {
			sprintf(text,"~OL~FR%s: Nepodarilo sa vytvorit pripojenie.\n",syserror);
			write_web(id+1000,text);
			mysql_free_result(result);
			eliminate_webuser(id+1000);
			return;
		}
		/* REM0TE */
		/* if ((user->fds=ADD_FDS(accept_sock,POLLIN|POLLPRI))==0) {
		write_syslog ("~OL~FRPreplnene pole struktur fds, zvys pocet MAX_POLL\n",1);
		destruct_user(user);
		mysql_free_result(result);
		eliminate_webuser(id);
		return;
		} */
		user->socket=id+1000;
		user->login=3;
		user->last_input=time(0);
		user->port=-1;
		sstrncpy(user->site,site,80);
		user->site_port=port;
		echo_on(user);
		sprintf(text,"~FW                  %08d navstevnikov od %s\n",counter_db(0),DATESINCE);
		write_user(user,text);
		sprintf(text,"%c%c",222,200);
		write_web(id+1000,text);
		write_user(user,"~FWTvoja prezyvka: ");
		num_of_logins++;
		user->zaradeny=1;
		strcpy(user->ipcka,ipcka);
		sprintf(query,"update `web_index` set `status`=2 where `id`='%d';",id);
		mysql_kvery(query);
	}
	mysql_free_result(result);
}

void kick_webusers()
{
	int cnt=0;
	UR_OBJECT user,next;
	int id[50],i;

	sprintf(query,"select `id` from `web_index` where `idle`>1 limit 50;");
	if (!(result=mysql_result(query)))
		return;
	cnt=0;
	while ((row=mysql_fetch_row(result))) {
		id[cnt]=atoi(row[0]);
		cnt++;
	}
	mysql_free_result(result);

	if (cnt)
		for (i=0;i<cnt;i++) {
			user=user_first;
			while (user!=NULL) {
				next=user->next;
				if (user->type!=USER_TYPE || user->socket<1000 || (int)(time(0)-user->last_input)<30) {
					user=next;
					continue;
				}
				if (user->socket!=id[i]+1000) {
					user=next;
					continue;
				}
				if (user->login) {
					disconnect_user(user,3,NULL);
				}
				else {
					sprintf(text,"~FRstratene spojenie");
					logout_user(user,text);
				}
				user=next;
			}
			sprintf(query,"delete from `web_index` where `id`='%d';",id[i]);
			mysql_kvery(query);
			sprintf(query,"delete from `web_out` where `id`='%d';",id[i]);
			mysql_kvery(query);
			sprintf(query,"delete from `web_in` where `id`='%d';",id[i]);
			mysql_kvery(query);
		}
	sprintf(query,"update `web_index` set `idle`=`idle`+1;");
	mysql_kvery(query);
}

void pictell(UR_OBJECT user,char *inpstr)
{
	UR_OBJECT u;
	char *name,tmp[100];
	int cnt=0;

	if (word_count==1) {
		write_user(user,"~OLZoznam dostupnych obrazkov:\n");
		sprintf(query,"select `filename` from `files` where `type`=1 order by `filename`;");
		if (!(result=mysql_result(query))) {
			write_user(user,"Zoznam obrazkov nieje k dispozicii.\n");
			return;
		}
		strcpy(text,"  ");
		while ((row=mysql_fetch_row(result))) {
			if (row[0]) {
				sprintf(tmp,"%12s",row[0]);
				strcat(text,tmp);
			}
			cnt++;
			if (cnt==6) {
				cnt=0;
				strcat(text,"\n");
				write_user(user,text);
				strcpy(text,"  ");
			}
		}
		mysql_free_result(result);
	}
	else {
		if (word_count==2) {
			strcpy(word[2],word[1]);
			strcpy(word[1],user->name);
		}
		if (!(u=get_user(word[1]))) {
			write_user(user,notloggedon);
			return;
		}
		if (word_count>2) {
			if (user->muzzled) {
				write_user(user,"Si umlcany - ziadne obrazky sa posielat nebudu!\n"); return;
			}
			if (u->afk) {
				write_user(user,"Uzivatel je teraz mimo klavesnice.\n"); return;
			}
			if (u->ignall) {
				if (u->malloc_start!=NULL)
					sprintf(text,"%s prave nieco pise v editore.\n",u->name);
				else
					sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
				write_user(user,text);
				return;
			}
			if (check_ignore_user(user,u)) { /* ignorovanie single usera */
				sprintf(text,"%s ta ignoruje.\n",u->name);
				write_user(user,text);
				return;
			}
			if (u->ignpict) {
				write_user(user,"Uzivatel ma nastaveny .ignore pictell!\n");
				return;
			}
		}
		if (strlen(word[2])>99) {
			write_user(user,"Prilis dlhe meno obrazka!\n");
			return;
		}
		sprintf(query,"select `body` from `files` where `filename`='%s' and (`type`=1 or `type`=2);",dbf_string(word[2]));
		if (!(result=mysql_result(query))) {
			write_user(user,"Nastala chyba pri citani obrazku.\n");
			return;
		}
		if ((row=mysql_fetch_row(result))) {
			if (row[0])
				write_user(u,row[0]);
		}
		else {
			write_user(user,"Taky obrazok neexistuje.\n");  /*APPROVED*/
			mysql_free_result(result);
			return;
		}
		mysql_free_result(result);
		if (user->vis)
			name=user->name;
		else
			name=invisname(user);
		if (word_count>3) {
			inpstr=remove_first(inpstr);
			inpstr=remove_first(inpstr);
			sprintf(text,"~OL%s~RS~FW ti posiela obrazok: %s\n",name,inpstr);
			write_user(u,text);
			sprintf(text,"~OL~FW\252C3%s~RS~FW ti poslal%s obrazok: %s\n",name,pohl(user,"","a"),inpstr);
			record_tell(u,text);
			sprintf(text,"Poslal%s si %s zvoleny obrazok: %s\n",pohl(user,"","a"),sklonuj(u,3),inpstr);
			write_user(user,text);
			sprintf(text,"Poslal%s si ~OL~FW\252C3%s~RS~FW obrazok: %s\n",pohl(user,"","a"),sklonuj(u,3),inpstr);
			record_tell(user,text);
		}
		else {
			if (word_count>2) {
				sprintf(text,"~FWObrazok ti posiela: ~OL%s~RS\n",name);
				write_user(u,text);
				sprintf(text,"~OL~FW\252C3%s~RS~FW ti poslal%s obrazok..\n",name,pohl(user,"","a"));
				record_tell(u,text);
				sprintf(text,"Poslal%s si ~OL~FW\252C3%s~RS~FW zvoleny obrazok.\n",pohl(user,"","a"),sklonuj(u,3));
				write_user(user,text);
				sprintf(text,"Poslal%s si ~OL~FW\252C3%s~RS~FW obrazok..\n",pohl(user,"","a"),sklonuj(u,3));
				record_tell(user,text);
			}
		}
	}
}

int load_macros(MACRO *list,int id)
{
	int pocet=0;

	sprintf(query,"select `name`,`value` from `macros` where `userid`='%d';",id);
	if (!(result=mysql_result(query)))
		return -1;
	while ((row=mysql_fetch_row(result))) {
		if (row[0] && row[1]) {
			add_macro(list,row[0],row[1]);
			pocet++;
		}
	}
	mysql_free_result(result);
	return pocet;
}

int doom_points_check(UR_OBJECT user)
{
	int cnt=0;

	sprintf(query,"select `doom_w`,`doom_l` from `winners` where `userid`='%d';",user->id);
	if (!(result=mysql_result(query)))
		return 0;
	if ((row=mysql_fetch_row(result)) && row[0] && row[1])
		cnt=atoi(row[0])-atoi(row[1]);
	mysql_free_result(result);
	return cnt;
}

int db_user_switch(int uid,int svitch)
{
	sprintf(query,"select `userid` from `people` where `userid`='%d' and `switch`='%d'",uid,svitch);
	if ((result=mysql_result(query))) {
		if ((row=mysql_fetch_row(result)) && row[0]) {
			mysql_free_result(result);
			return 1;
		}
		mysql_free_result(result);
	}
	return 0;
}

int query_to_int(char *kvery)
{
	int rezult=0;

	if ((result=mysql_result(kvery))) {
		if ((row=mysql_fetch_row(result)) && row[0]) {
			rezult=atoi(row[0]);
		}
		mysql_free_result(result);
	}
	return rezult;
}

void mymail(UR_OBJECT user,int self)
{
	int ret;

	if (word_count!=2 && !self) {
		sprintf(text,"Pouzi '.mymail <uzivatel>' ak si chces pozriet postu ktoru si mu poslal%s.\n",pohl(user,"","a"));
		write_user(user,text);
		return;
	}
	if (self && (word_count!=2 || !strcasecmp(user->name,word[1]))) {
		sprintf(text,"Pouzi '.rmail <uzivatel>' ak si chces pozriet postu ktoru od neho mas.\n");
		write_user(user,text);
		return;
	}
	if (self) {
		strncpy(user->subject,word[1],30);
		user->browsing=11;
	}
	else {
		strcpy(word[1],db_user_name(word[1]));
		if (!word[1][0]) {
			write_user(user,"Taky uzivatel neexistuje.\n");
			return;
		}
		sprintf(user->subject,"%d",db_userid(word[1]));
		user->browsing=12;
	}
	user->rjoke_from=1;
	user->messnum=1;
	ret=mailmore(user);
	if (ret==1) user->misc_op=223;
	return;
}

int is_webuser(UR_OBJECT user)
{
	if (user->socket>1000)
		return 1;
	else
		return 0;
}

void highlight_write2sock(UR_OBJECT user,char *buff,size_t num)
{
	char *tmp2,*tmp;
	int web,cnt=0;

	if (user->findstr[0] && strstr(buff,user->findstr) && user->colour) {
		web=is_webuser(user);
		strcpy(parsed,buff);
		parsed[num]='\0';
		tmp=parsed;
		tmp2=strstr(tmp,user->findstr);
		while (tmp2!=NULL) {
			cnt++;
			memmove(tmp2+5-web*2,tmp2,strlen(tmp2)+1);
			if (web)
				strncpy(tmp2,"~BR",3);
			else
				strncpy(tmp2,"\033[41m",5);
			tmp2+=strlen(user->findstr)+5-web*2;
			memmove(tmp2+5-web*2,tmp2,strlen(tmp2)+1);
			if (web)
				strncpy(tmp2,"~BR",3);
			else
				strncpy(tmp2,"\033[40m",5);
			tmp=tmp2+5-web*2;
			tmp2=strstr(tmp,user->findstr);
		}
		write2sock(user,user->socket,parsed,num+10*cnt-web*4*cnt);
	}
	else {
		write2sock(user,user->socket,buff,num);
	}
}

void construct_query(UR_OBJECT user,int max)
{
	if (max==-666) {
		switch (user->browsing) {
			case 2:
			case 10:
				sprintf(query,"select length(`message`) from `mailbox` where `userid`='%d' order by `time` limit %d,10000",user->id,user->messnum-1);
				break;
			case 3:
				sprintf(query,"select length(`message`) from `board` where `room`='%s' and `deleted`=0 order by `time` limit %d,10000",user->subject,user->messnum-1);
				break;
			case 4:
				sprintf(query,"select length(`message`) from `board` where `deleted`=0 and `message` like('%%%s%%') order by `time` limit %d,10000",user->findstr,user->messnum-1);
				break;
			case 11:
				sprintf(query,"select length(`message`) from `mailbox` where `userid`='%d' and `sender`='%s' order by `time` limit %d,10000",user->id,user->subject,user->messnum-1);
				break;
			case 12:
				sprintf(query,"select length(`message`) from `mailbox` where `userid`='%s' and `sender`='%s' order by `time` limit %d,10000",user->subject,user->name,user->messnum-1);
				break;
			case 20:
				sprintf(query,"select length(`joke`) from `jokes` order by `id` desc limit %d,10000",user->messnum-1);
				break;
			case 21:
				sprintf(query,"select length(`joke`) from `jokes` order by `id` desc limit 1");
				break;
			default:
				if (user->browsing>100)
					sprintf(query,"select length(`message`) from `board` where `deleted`=0 and `time`>FROM_UNIXTIME('%d') order by `time` limit %d,10000",(int)time(0)-(user->browsing-100)*86400,user->messnum-1);
				break;
		}
		return;
	}
	switch (user->browsing) {
		case 2:
		case 10:
			sprintf(query,"select `message`,`sender`,`time`,UNIX_TIMESTAMP(`time`) from `mailbox` where `userid`='%d' order by `time` limit %d,%d",user->id,user->messnum-1,max);
			break;
		case 3:
			sprintf(query,"select `message`,`autor`,`time`,`dow`,`status` from `board` where `room`='%s' and `deleted`=0 order by time limit %d,%d",user->subject,user->messnum-1,max);
			break;
		case 4:
			sprintf(query,"select `message`,`autor`,`time`,`dow`,`status`,`room` from `board` where `deleted`=0 and `message` like('%%%s%%') order by `time` limit %d,%d",user->findstr,user->messnum-1,max);
			break;
		case 11:
			sprintf(query,"select `message`,`sender`,`time`,UNIX_TIMESTAMP(`time`) from `mailbox` where `userid`='%d' and `sender`='%s' order by `time` limit %d,%d",user->id,user->subject,user->messnum-1,max);
			break;
		case 12:
			sprintf(query,"select `message`,`sender`,`time`,UNIX_TIMESTAMP(`time`) from `mailbox` where `userid`='%s' and `sender`='%s' order by `time` limit %d,%d",user->subject,user->name,user->messnum-1,max);
			break;
		case 20:
			sprintf(query,"select `joke`,`autor` from `jokes` order by `id` desc limit %d,%d",user->messnum-1,max);
			break;
		case 21:
			sprintf(query,"select `joke`,`autor` from `jokes` order by `id` desc limit 1");
			break;
		default:
			if (user->browsing>100)
				sprintf(query,"select `message`,`autor`,`time`,`dow`,`status`,`room` from `board` where `deleted`=0 and `time`>FROM_UNIXTIME('%d') order by `time` limit %d,%d",(int)time(0)-(user->browsing-100)*86400,user->messnum-1,max);
			break;
	}
}

int mailmore(UR_OBJECT user)
{
	int i,buffpos,lines=0;
	int retval,totalines, fperc,buffold=0;
	char buff[OUT_BUFF_SIZE+8]; /* *colour_com_strip() */
	int webuser=0,max,silent=0,eor=0,pos=0,fetch=0;
	char tmp[40];
	PAGER pom;

	if (user->socket>1000)
		webuser=1;
	totalines=user->lines-1;
	if (!user->messsize) {
		if (user->ignall==1 && !user->has_ignall)
			user->has_ignall=1;
		user->ignall=2;
		user->pageno=0;
		user->miscoptime=thour*3600+tmin*60+tsec;
		user->messsize=0;
		user->filepos=0;
		construct_query(user,-666);
		if ((result=mysql_result(query))) {
			while ((row=mysql_fetch_row(result)) && row[0]) {
				user->messsize+=atoi(row[0]);
			}
			mysql_free_result(result);
		}
		totalines-=2;
	}
	else {
		if (user->filepos==0 && user->messnum==user->rjoke_from) {
			totalines-=2;
		}
	}
	if (user->skip>0) {
		max=totalines*user->skip;
		user->skip--;
	}
	else
		max=totalines;
	buffpos=0;
	buffold=0;
	retval=1;
	construct_query(user,max);
	if ((result=mysql_result(query))) {
		if (!(row=mysql_fetch_row(result)))
			eor=1;
		fetch++;
		if (user->findstr[0]) {
			while (!eor && !strstr(row[0],user->findstr)) {
				if (!(row=mysql_fetch_row(result)))
					eor=1;
				else {
					fetch++;
					user->messnum++;
				}
				if (fetch==max && eor) {
					fetch=1;
					user->messnum++;
					construct_query(user,max);
					mysql_free_result(result);
					if ((result=mysql_result(query)))
						if ((row=mysql_fetch_row(result)))
							eor=0;
				}
			}
		}
		if (eor) {
			mysql_free_result(result);
			user->messsize=0;
			user->ignall=user->has_ignall;
			user->has_ignall=0;
			user->pageno=0;
			switch (user->browsing) {
				case 2:
					if (user->findstr[0]) {
						sprintf(text,"Ziadna z nasledujucich sprav neobsahuje slovo '%s'.\n",user->findstr);
					}
					else {
						sprintf(query,"select count(`msgid`) from `mailbox` where `userid`='%d'",user->id);
						max=query_to_int(query);
						if (max==0)
							sprintf(text,"Nemas ziadnu postu.\n");
						else
							sprintf(text,"V schranke mas len %d %s.\n",max,skloncislo(max,"spravu","spravy","sprav"));
					}
					write_user(user,text);
					break;
				case 10:
					if (user->findstr[0]) {
						sprintf(text,"Ziadna z novych sprav neobsahuje '%s'.\n",user->findstr);
						write_user(user,text);
					}
					else {
						write_user(user,"Bohuzial, neboli zaznamenane ziadne nove spravy.\n");
					}
					break;
				case 11:
					sprintf(text,"Nemas ziadnu postu od uzivatela '%s'.\n",user->subject);
					write_user(user,text);
					break;
				case 12:
					sprintf(text,"%s od teba nema ziadnu postu.\n",word[1]);
					write_user(user,text);
					break;
				case 3:
					if (user->findstr[0])
						sprintf(text,"Ziadna z nasledujucich sprav neobsahuje slovo '%s'.\n",user->findstr);
					else {
						sprintf(query,"select count(`msgid`) from `board` where `room`='%s' and `deleted`=0",user->subject);
						max=query_to_int(query);
						if (max)
							sprintf(text,"Na nastenke je len %d %s.\n",max,skloncislo(max,"sprava","spravy","sprav"));
						else
							sprintf(text,"Nastenka je prazdna.\n");
					}
					write_user(user,text);
					break;
				case 4:
					sprintf(text,"Ziadna nastenka neobsahuje slovo '%s'.\n",user->findstr);
					write_user(user,text);
					break;
				case 20:
				case 21:
					if (user->findstr[0])
						sprintf(text,"Ziadny z nasledujucich vtipov neobsahuje slovo '%s'.\n",user->findstr);
					else {
						sprintf(query,"select count(`id`) from `jokes`");
						max=query_to_int(query);
						if (max)
							sprintf(text,"Na jokeboard je len %d vtip%s.\n",max,skloncislo(max,"","y","ov"));
						else
							sprintf(text,"Na jokeboarde niesu ziadne vtipy ;(\n");
					}
					write_user(user,text);
					break;
				default:
					if(user->browsing>100) {
						sprintf(text,"Neboli najdene ziadne spravy napisne za posledn%s %d %s.\n",skloncislo(user->browsing-100,"y","e","ch"),user->browsing-100,skloncislo(user->browsing-100,"den","dni","dni"));
						write_user(user,text);
					}
					break;
			}
			user->findstr[0]='\0';
			user->subject[0]='\0';
			return 0;
		}
	}
	else {
		user->filepos=0;
		user->messnum=0;
		user->ignall=user->has_ignall;
		user->has_ignall=0;
		user->messsize=0;
		user->pageno=0;
		user->findstr[0]='\0';
		user->subject[0]='\0';
		return 0;
	}
	if (user->filepos<=strlen(row[0]))
		pos=user->filepos;
	else
		pos=0;
	lines=0;
	while(!eor) {
		if (lines>=totalines) { /* skipovanie */
			if (user->skip>0) {
				user->skip--;
				lines=0;
				user->filepos=pos;
				if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
					sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v mailmore()!\n");
					write_syslog(text,1);
					mysql_free_result(result);
					user->pageno=0;
					user->findstr[0]='\0';
					user->subject[0]='\0';
					return 2;
				}
				user->pageno++;
				pom->filepos=pos;
				pom->messnum=user->messnum;
				pom->next=user->pager;
				user->pager=pom;
				buffpos=0;
				buffold=0;
				retval=1;
			}
			else
				break;
		}
		if (!row[0][pos]) { /* fetchovanie */
			pos=0;
			if (!(row=mysql_fetch_row(result)))
				eor=1;
			fetch++;
			if (user->findstr[0]) {
				if (fetch>=max && eor) {
					fetch=1;
					mysql_free_result(result);
					construct_query(user,max);
					if ((result=mysql_result(query)))
						if ((row=mysql_fetch_row(result)))
							eor=0;
				}
				while (!eor && !strstr(row[0],user->findstr)) {
					if (!(row=mysql_fetch_row(result)))
						eor=1;
					else {
						fetch++;
						user->messnum++;
					}
					if (fetch>=max && eor) {
						fetch=1;
						mysql_free_result(result);
						construct_query(user,max);
						if ((result=mysql_result(query)))
							if ((row=mysql_fetch_row(result)))
								eor=0;
					}
				}
			}
			user->messnum++;
			continue;
		}
		if (user->skip>0)
			silent=1;
		else
			silent=0;
		if (pos==0) { /* tekvice (hlavicky) */
			*(buff+buffpos)='\0';
			if (!silent) {
				switch(user->browsing) {
					case 2:
					case 10:
					case 11:
					case 12:
						if (row[3] && atoi(row[3])>=user->read_mail) { 
							i=1;
							user->read_mail=atoi(row[3])+1;
						}
						else
							i=0;
						if (user->colour) {
							if (webuser) {
								sprintf(text,"~H%s===%s%s~HW%d~H%s===~DYOd: ~HW%s ~DYprijata %s ~H%s======================================",i?"T":"B",user->messnum>99?"":"=",user->messnum>9?"":"=",user->messnum,i?"T":"B",row[1]?row[1]:"",row[2]?sqldatum(row[2],1,0):"??.??.???? ??:??",i?"T":"B");
								text[80+29]='\0';
								strcat(text,"\r\n~RS~FW");
							}
							else {
								sprintf(text,"%s===%s%s\033[1m\033[37m%d%s===\033[0m\033[33mOd: \033[1m\033[37m%s \033[0m\033[33mprijata \033[1m\033[37m%s %s======================================",i?"\033[1m\033[36m":"\033[1m\033[34m",user->messnum>99?"":"=",user->messnum>9?"":"=",user->messnum,i?"\033[1m\033[36m":"\033[1m\033[34m",row[1]?row[1]:"",row[2]?sqldatum(row[2],2,0):"??.??.???? ??:??",i?"\033[1m\033[36m":"\033[1m\033[34m");
								text[80+98]='\0';
								strcat(text,"\r\n\033[0m\033[37m");
							}
						}
						else {
							sprintf(text,"===%s%s%d===Od: %s prijata %s ======================================",user->messnum>99?"":"=",user->messnum>9?"":"=",user->messnum,row[1]?row[1]:"",row[2]?sqldatum(row[2],0,0):"??.??.???? ??:??");
							text[79]='\0';
							strcat(text,"\r\n");
						}
						break;
					case 20:
					case 21:
						i=pocet_vtipov+1-user->messnum;
						if (user->colour) {
							if (webuser) {
								sprintf(text,"~HB=-=-=%s%s%s%s~HW%d~HB=-=-=-=-=-=-=-=-=-=~DG Vtip napisal: ~HY%s ~HB=-=-=-=-=-=-=-=-=-=-=-=-",user->messnum>9999?"":"-",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"=",user->messnum,row[1]?row[1]:"Neznamy");
								text[80+5]='\0';
								sprintf(texthb,"%s%s%s%s~HW%d~HB=-=-=-=\r\n~RS~FW",i>9999?"":"-",i>999?"":"=",i>99?"":"-",i>9?"":"=",i);
								strcat(text,texthb);
							}
							else {
								sprintf(text,"\033[1m\033[34m=-=-=%s%s%s%s\033[1m\033[37m%d\033[1m\033[34m=-=-=-=-=-=-=-=-=-=\033[0m\033[32m Vtip napisal: \033[1m\033[33m%s \033[1m\033[34m=-=-=-=-=-=-=-=-=-=-=-=-",user->messnum>9999?"":"-",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"=",user->messnum,row[1]?row[1]:"Neznamy");
								text[80+41]='\0';
								sprintf(texthb,"%s%s%s%s\033[1m\033[37m%d\033[1m\033[34m=-=-=-=\r\n\033[0m\033[37m",i>9999?"":"-",i>999?"":"=",i>99?"":"-",i>9?"":"=",i);
								strcat(text,texthb);
							}
						}
						else {
							sprintf(text,"=-=-=%s%s%s%s%d=-=-=-=-=-=-=-=-=-= Vtip napisal: %s =-=-=-=-=-=-=-=-=-=-=-=-",user->messnum>9999?"":"-",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"=",user->messnum,row[1]?row[1]:"Neznamy");
							text[67]='\0';
							sprintf(texthb,"%s%s%s%s%d=-=-=-=\r\n",i>9999?"":"-",i>999?"":"=",i>99?"":"-",i>9?"":"=",i);
							strcat(text,texthb);
						}
						break;
					default:
						if (user->browsing>2) {
							texthb[0]='\0';
							i=0;
							if (row[4]) {
								if (row[4][strlen(row[4])-1]=='1')
									strcpy(texthb,"Neznamy");
								else if (row[4][strlen(row[4])-1]=='2')
									strcpy(texthb,"Neznama");
								else if (row[4][strlen(row[4])-1]=='3') {
									if (user->colour) {
										if (webuser) {
											sprintf(texthb,"%s ~HW(WEB)",row[1]);
											i=3;
										}
										else {
											sprintf(texthb,"%s \033[1m\033[37m(WEB)",row[1]);
											i=9;
										}
									}
									else {
										sprintf(texthb,"%s (WEB)",row[1]);
										i=0;
									}
								}
								else strcpy(texthb,row[1]);
							}
							tmp[0]='\0';
							fperc=0;
							if ((user->browsing==4 || user->browsing>100) && row[5]) {
								if (user->colour) {
									if (webuser) {
										sprintf(tmp,"~HW%s~HM",row[5]);
										fperc=6;
									}
									else {
										sprintf(tmp,"\033[1m\033[37m%s\033[1m\033[35m",row[5]);
										fperc=18;
									}
								}
								else {
									sprintf(tmp,"%s",row[5]);
								}
							}
							if (user->colour) {
								if (webuser) {
									sprintf(text,"~HM=-=-%s%s%s~HW%d~HM=-=-=-=-=-=-=-=-=-=",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"-",user->messnum);
									text[37-strlen(texthb)+i]='\0';
									sprintf(query," ~DYOd: ~HY%s ~DT[ %s ~DT] ~HM=-=%s-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",texthb,sqldatum(row[2],0,row[3]?atoi(row[3])+1:0),tmp);
									strcat(text,query);
									text[112+i+fperc]='\0';
									strcat(text,"\r\n~RS~FW");
								}
								else {
									sprintf(text,"\033[1m\033[35m=-=-%s%s%s\033[1m\033[37m%d\033[1m\033[35m=-=-=-=-=-=-=-=-=-=",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"=",user->messnum);
									text[51-strlen(texthb)+i]='\0';
									sprintf(query," \033[0m\033[33mOd: \033[1m\033[33m%s \033[0m\033[36m[ %s \033[0m\033[36m] \033[1m\033[35m=-=%s-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",texthb,sqldatum(row[2],2,row[3]?atoi(row[3])+1:0),tmp);
									strcat(text,query);
									text[178+i+fperc]='\0';
									strcat(text,"\r\n\033[0m\033[37m");
								}
							}
							else {
								sprintf(text,"=-=-%s%s%s%d=-=-=-=-=-=-=-=-=-=",user->messnum>999?"":"=",user->messnum>99?"":"-",user->messnum>9?"":"-",user->messnum);
								text[28-strlen(texthb)]='\0';
								sprintf(query," Od: %s [ %s ] =-=%s-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",texthb,sqldatum(row[2],0,row[3]?atoi(row[3])+1:0),tmp);
								strcat(text,query);
								text[79]='\0';
								strcat(text,"\r\n");
							}
						}
						break;
				}
				if (buffpos>(int)(OUT_BUFF_SIZE-strlen(text)-1)) {
					highlight_write2sock(user,buff,OUT_BUFF_SIZE);
					buffpos=0;
					buffold=0;
					buff[0]='\0';
				}
				strcat(buff,text);
				buffpos+=strlen(text);
				buffold=buffpos;
			}
			lines++;
			if (lines>0 && (lines)%totalines==0)
				continue;
		}
		if (!silent && (user->statline==CHARMODE) && (user->newline)) {
			*(buff+buffpos)='\r';
			*(buff+buffpos+1)='\n'; /* CR/LF! */
			buffpos+=2;
			user->newline=0;
		}
		while (row[0][pos]) {
			if (lines>=totalines)
				break;
			if (row[0][pos]=='\n') {
				if (silent) {
					pos++;
					lines++;
				}
				else {
					*(buff+buffpos)='\0';
					if (user->ignword[0] && strstr(buff,user->ignword))
						buffpos=buffold;
					else
						buffold=buffpos;
					if (buffpos>OUT_BUFF_SIZE-8) {
						highlight_write2sock(user,buff,buffpos);
						buffpos=0;
						buffold=0;
						lines++;
					}
					/* Reset terminal before every newline */
					if (!silent && user->colour) {
						if (!webuser) {
							memcpy(buff+buffpos,"\033[0m",4);
							buffpos+=4;
						}
					}
					if (user->statline==CHARMODE && (row[0][pos+1])==0) {
						user->newline=1;
						pos++;
					}
					else {
						*(buff+buffpos)='\r';
						*(buff+buffpos+1)='\n'; /* CR/LF! */
						buffpos+=2;
						pos++;
						lines++;
					}
					break;
				}
			}
			else {
				if (row[0][pos]>=127)
					row[0][pos]=' '; /* UMBA */
				if (!webuser && row[0][pos]=='~') {
					if (buffpos>OUT_BUFF_SIZE-8) {
						highlight_write2sock(user,buff,buffpos);
						buffpos=0;
						buffold=0;
					}
					pos++;
					for(i=0;i<NUM_COLS;++i) {
						if (row[0][pos]==colcom2[i][0] && row[0][pos+1]==colcom2[i][1]) {
							if ((pos>1)!=0 && row[0][pos-2]=='~') {
								if (!silent)
									*(buff+buffpos)=row[0][pos];
								goto CONT;
							}
							if (!silent) {
								if (user->colour && !(user->colour==2 && !strcmp(colcom2[i],"LI")) && !(user->ignbeep && !strcmp(colcom2[i],"LB")) && !(user->ignblink && !strcmp(colcom2[i],"LI"))) {
									memcpy(buffpos+buff,colcode2[i],strlen(colcode2[i]));
									buffpos+=strlen(colcode2[i])-1;
								}
								else
									buffpos--;
							}
							pos++;
							goto CONT;
						}
					}
					pos--;
					if (!silent)
						*(buff+buffpos)=row[0][pos];
				}
				else {
					if (!silent)
						*(buff+buffpos)=row[0][pos];
				}
CONT:
				if (!silent) ++buffpos;
				++pos;
			}
			if (buffpos==OUT_BUFF_SIZE) {
				highlight_write2sock(user,buff,OUT_BUFF_SIZE);
				buffpos=0;
				buffold=0;
			}
		}
	}
	if (buffpos && user->socket!=-1) {
		highlight_write2sock(user,buff,buffpos);
	}
	mysql_free_result(result);
	if (eor) {
		if (user->skip>0) {
			user->skip=0;
			if (user->pageno>1) {
				for (i=0;i<1;i++) {
					if (user->pager!=NULL) {
						pom=user->pager;
						user->pager=user->pager->next;
						free ((void *) pom);
						user->pageno--;
					}
				}
				user->filepos=user->pager->filepos;
				user->messnum=user->pager->messnum;
				return mailmore(user);
			}
			else {
				write_user(user,"~FM====+~OL+~RS~FM+==================================================================+~OL+~RS~FM+====\n");
				sprintf(text,"                              ~BB~OL~FW Pocet stranok: ~FY%d ~BK\n",user->pageno+1);
				write_user(user,text);
			}
		}
		else {
			write_user(user,"~FM====+~OL+~RS~FM+==================================================================+~OL+~RS~FM+====\n");
		}
		user->filepos=0;
		user->messsize=0;
		no_prompt=0;
		retval=2;
		zrus_pager_haldu(user);
		user->ignall=user->has_ignall;
		user->has_ignall=0;
		user->lastm=user->messnum;
		user->browsing=0;
		user->pageno=0;
		user->skip=0;
		if (user->newtell) {
			show_recent_tells(user);
			user->newtell=0;
			user->tellpos=0;
		}
		show_recent_notifies(user);
		user->findstr[0]='\0';
		user->subject[0]='\0';
	}
	else {
		user->filepos=pos;
		if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
			sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v more()!\n");
			write_syslog(text,1);
			user->messsize=0;
			user->pageno=0;
			user->findstr[0]='\0';
			user->subject[0]='\0';
			return 2;
		}
		user->pageno++;
		pom->filepos=user->filepos;
		pom->messnum=user->messnum;
		pom->next=user->pager;
		user->pager=pom;
		i=0;
		lines=0;
		construct_query(user,-666);
		if ((result=mysql_result(query))) {
			while ((row=mysql_fetch_row(result)) && row[0]) {
				lines++;
				if (lines==1) i=atoi(row[0])-user->filepos;
				else i+=atoi(row[0]);
			}
			mysql_free_result(result);
		}
		if (user->messsize)
			fperc=(int)(i*100)/user->messsize;
		else
			fperc=0;
		fperc=100-fperc;

		sprintf(text,"~BB~OL~FT[%3d%%] ~FWstr:~FT %d, ~FYENTER~FW-dalej, ~FYB~FW-spat, ~FYR~FW-znovu, ~FYn~FW/~FY-n~FW stran, ~FY/~FWhladat, ~FYE~FW-koniec~RS~BK ",fperc,user->pageno);
		write_user(user,text);
		if (user->statline==CHARMODE)
			user->newline=1;
		no_prompt=1;
	}
	return retval;
}

/* Write on the message board */
void write_board(UR_OBJECT user,char *inpstr,int done_editing)
{
	int max=0,g=0;
	char *ptr,*name;

	if (user->muzzled) {
		write_user(user,"Si umlcany, a preto nemozes pisat na nastenku!\n");
		return;
	}
	if (!done_editing && !strcmp(user->room->name,"cintorin")) {
		write_user(user,"Nie je tu ziadna nastenka, nemas kam pisat.\n");
		return;
	}
	if (!done_editing && word_count>1 && (user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
		write_user(user,noswearing);
		sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s pisat skaredo na nastenku!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
		writesys(WIZ,1,text,user);
		sprintf(text,"~FT%02d>~RS",user->edit_line);
		write_user(user,text);
		return;
	}
	if (!done_editing) {
		if (word_count<2) {
			write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie spravy na nastenku ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
			user->misc_op=3;
			editor(user,NULL);
			return;
		}
		strcat(inpstr,"\n");
		ptr=inpstr;
	}
	else
		ptr=user->malloc_start;
	if (user->vis)
		name=user->name;
	else
		name=invisname2(user);
	sprintf(query,"select max(`msgid`) from `board` where `room`='%s'",user->room->name);
	max=query_to_int(query);
	if (!user->vis) {
		if (user->sex)
			g=1;
		else
			g=2;
	}
	sprintf(query,"insert into `board` (`room`,`msgid`,`time`,`autor`,`dow`,`status`,`message`) values ('%s','%d',FROM_UNIXTIME('%d'),'%s','%d','%d','%s')",user->room->name,max+1,(int)time(0),user->name,twday,g,dbf_string(ptr));
	if (mysql_query(&mysql,query)) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		write_level(KIN,1,text,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		write_user(user,"Chyba pri zapisovani spravy.\n");
		return;
	}
	user->room->mesg_cnt++;
	sprintf(text,"%s si spravu na nastenku (#%d).\n",pohl(user,"Napisal","Napisala"), user->room->mesg_cnt);
	write_user(user,text);
	sprintf(text,"%s %s spravu na nastenku (#%d).\n",name,pohl(user,"napisal","napisala"), user->room->mesg_cnt);
	write_room_except(user->room,text,user);
}

void wipe_board(UR_OBJECT user)
{
	int from=0,to=0,cnt,fromid=0,toid=0;
	int vyfluslo;
	char *name;

	if (word_count<2 || ((atoi(word[1]))<1 && strcmp(word[1],"all"))) {
		write_user(user,"Pouzi: .wipe [<cislo spravy>] [<od> <do>] [all]\n");
		return;
	}
	from=atoi(word[1]);
	if ((word_count==2) && (from>=1))
		to=from; /* Uprava -> iba JEDEN message */
	if (((word_count==3) && (((from=atoi(word[1]))<1) || ((to=atoi(word[2]))<1)))) {
		write_user(user,"Pouzi: .wipe [<cislo spravy>] [<od> <do>] [all]\n");
		return;
	}
	if (user->vis)
		name=user->name;
	else
		name=invisname(user);
	if (!strcmp(word[1],"all")) {
		if (user->level>=KIN) {
			sprintf(query,"delete from `board` where `room`='%s' and `deleted`=0",user->room->name);
			mysql_kvery(query);
			vyfluslo=mysql_affected_rows(&mysql);
			write_user(user,"Vsetky spravy boli zmazane.\n");
		}
		else {
			sprintf(query,"delete from `board` where `room`='%s' and `autor`='%s' and `deleted`=0",user->room->name,user->name);
			mysql_kvery(query);
			vyfluslo=mysql_affected_rows(&mysql);
			if (vyfluslo) {
				sprintf(text,"%s zmazal%s z nastenky %d sprav%s.\n",name,pohl(user,"","a"),vyfluslo,skloncislo(vyfluslo,"u","y",""));
				write_room_except(user->room,text,user);
				sprintf(text,"Zmazal%s si %d sprav%s z nastenky.\n",pohl(user,"","a"),vyfluslo,skloncislo(vyfluslo,"u","y",""));
			}
			else
				sprintf(text,"Na nastenke nie su od teba ziadne spravy.\n");
			write_user(user,text);
		}
		sprintf(text,"%s vymazal%s %d sprav z nastenky v miestnosti %s.\n",user->name,pohl(user,"","a"),vyfluslo,user->room->name);
		write_syslog(text,1);
		sprintf(query,"select count(`msgid`) from `board` where `room`='%s' and `deleted`=0",user->room->name);
		cnt=query_to_int(query);
		user->room->mesg_cnt=cnt;
		if (cnt==0) {
			sprintf(text,"%s zmazal%s nejake spravy z nastenky, nastenka je teraz prazdna.\n",name,pohl(user,"","a"));
			write_room_except(user->room,text,user);
		}
		else {
			sprintf(text,"%s zmazal%s nejake spravy z nastenky.\n",name,pohl(user,"","a"));
			write_room_except(user->room,text,user);
		}
		return;
	}
	if (from>to) {
		write_user(user,"Prve cislo musi byt mensie (alebo rovne) ako druhe!\n");
		return;
	}
	sprintf(query,"select `msgid` from `board` where `room`='%s' and `deleted`=0 order by `time` limit %d,1",user->room->name,from-1);
	if ((result=mysql_result(query))) {
		if ((row=mysql_fetch_row(result)) && row[0])
			fromid=atoi(row[0]);
		else {
			mysql_free_result(result);
			sprintf(query,"select count(`msgid`) from `board` where `room`='%s' and `deleted`=0",user->room->name);
			cnt=query_to_int(query);
			sprintf(text,"Na nastenke %s len %d sprav%s.\n",skloncislo(cnt,"je","su","je"),cnt,skloncislo(cnt,"a","y",""));
			write_user(user,text);
			return;
		}
	}
	else {
		write_user(user,"Nastala chyba pri mazani z nastenky.\n");
		return;
	}
	if (from==to)
		toid=fromid;
	else {
		sprintf(query,"select `msgid` from `board` where `room`='%s' and `deleted`=0 order by `time` limit %d,1",user->room->name,to-1);
		if ((result=mysql_result(query))) {
			if ((row=mysql_fetch_row(result)) && row[0])
				toid=atoi(row[0]);
			else
				toid=0;
			mysql_free_result(result);
		}
		else {
			write_user(user,"Nastala chyba pri mazani z nastenky!\n");
			return;
		}
	}
	if (user->level>=KIN)
		texthb[0]='\0';
	else
		sprintf(texthb," `autor`='%s' and",user->name);

	if (toid==0)
		sprintf(query,"delete from `board` where `room=`'%s' and `deleted`=0 and%s %d<=`msgid`",user->room->name,texthb,fromid);
	else
		sprintf(query,"delete from `board` where `room`='%s' and `deleted`=0 and%s %d<=`msgid` and `msgid`<=%d",user->room->name,texthb,fromid,toid);
	mysql_kvery(query);
	vyfluslo=mysql_affected_rows(&mysql);
	sprintf(query,"select count(`msgid`) from `board` where `room`='%s' and `deleted`=0",user->room->name);
	cnt=query_to_int(query);
	user->room->mesg_cnt=cnt;
	sprintf(text,"%s %s %d sprav z nastenky v miestnosti %s.\n",user->name,pohl(user,"vymazal","vymazala"),vyfluslo,user->room->name);
	write_syslog(text,1);
	if (cnt==0) {
		sprintf(text,"Na nastenke %s %d sprav%s, schranka je teraz prazdna.\n",skloncislo(vyfluslo,"bola","boli","bolo"),vyfluslo,skloncislo(vyfluslo,"a","y",""));
		write_user(user,text);
		sprintf(text,"%s zmazal%s nejake spravy z nastenky, nastenka je teraz prazdna.\n",name,pohl(user,"","a"));
		write_room_except(user->room,text,user);
	}
	else {
		if (vyfluslo) {
			sprintf(text,"%d %s, zostalo %d.\n",vyfluslo,skloncislo(vyfluslo,"sprava bola zmazana","spravy boli zmazane","sprav bolo zmazanych"),cnt);
			write_user(user,text);
			sprintf(text,"%s zmazal%s nejake spravy z nastenky.\n",name,pohl(user,"","a"));
			write_room_except(user->room,text,user);
		}
		else {
			write_user(user,"Mozes mazat iba svoje spravy.\n");
		}
	}
}

/* Search all the boards for the words given in the list. Rooms fixed to
 * private will be ignore if the users level is less than gatecrash_level
 */
void search_boards(UR_OBJECT user)
{
	int ret,num;

	if (word_count<2) {
		write_user(user,"Pouzi: .search <slovo>     - zobrazi spravy na nastenkach obsahujuce dane slovo\n");
		write_user(user,"       .search <pocet dni> - zobrazi spravy ktore niesu starsie ako pocet dni\n");
		return;
	}
	if (is_number(word[1])) {
		num=atoi(word[1]);
		if (num<1)
			num=1;
		if (99<num)
			num=99;
		user->browsing=100+num;
	}
	else {
		user->browsing=4;
		strncpy(user->findstr,word[1],19);
		user->findstr[20]='\0';
	}
	user->rjoke_from=1;
	user->messnum=1;
	ret=mailmore(user);
	if (ret==1)
		user->misc_op=223;
}

void write_joke(UR_OBJECT user,int done_editing)
{
	int max;
	char *ptr;

	if (user->muzzled) {
		write_user(user,"Si umlcany, a to nieje vtip! ;)\n");
		return;
	}
	if (!done_editing) {
		write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie vtipu na JokeBoard ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
		user->misc_op=8;
		editor(user,NULL);
		return;
	}
	else {
		ptr=user->malloc_start;
	}
	sprintf(query,"select max(`id`) from jokes");
	max=query_to_int(query);
	sprintf(query,"insert into `jokes` (`id`,`autor`,`joke`) values ('%d','%s','%s')",max+1,user->name,dbf_string(ptr));
	if (mysql_query(&mysql,query)) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		write_level(KIN,1,text,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		write_user(user,"Chyba pri zapisovani vtipu.\n");
		return;
	}
	sprintf(text,"%s si vtip na JokeBoard.\n",pohl(user,"Napisal","Napisala"));
	write_user(user,text);
	sprintf(text,"~OL~FT:^) ~RS~FG%s napisal%s vtip na JokeBoard!~OL~FT :^)\n",user->name, pohl(user,"","a"));
	write_room_except(NULL,text,user);
	pocet_vtipov+=1;
}

void read_joke(UR_OBJECT user)
{
	char *name;
	int ret;

	if (word_count>1) {
		user->rjoke_from=atoi(word[1]);
		if (user->rjoke_from<1)
			user->rjoke_from=1;
	}
	else
		user->rjoke_from=1;
	sprintf(text,"\n~FM<~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM>  ~OL~FR(-~LI:~RS~OL~FT The JokeBoard! ~LI~FR:~RS~OL~FR-)  ~RS~FM<~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM>\n");
	write_user(user,text);
	if (!strcmp(word[1],"last")) {
		user->rjoke_from=query_to_int("select max(`id`) from `jokes`");
		user->browsing=21;
		user->messnum=user->rjoke_from;
		ret=mailmore(user);
		if (ret==1)
			user->misc_op=223;
		return;
	}
	user->browsing=20;
	user->messnum=user->rjoke_from;
	ret=mailmore(user);
	if (ret==1) {
		user->misc_op=223;
		if (user->vis)
			name=user->name;
		else
			name=invisname(user);
		sprintf(text,"%s s usmevom cita JokeBoard.\n",name);
		write_room_except(user->room,text,user);
	}
}

int countjoke(void)
{
	int cnt;

	sprintf(query,"select count(`id`) from `jokes`");
	cnt=query_to_int(query);
	printf("%d vtipov.\n",cnt);
	return cnt;
}

void dmail(UR_OBJECT user)
{
	int from=0,to=0,cnt,fromid=0,toid=0;
	int vyfluslo;

	if (word_count<2 || ((atoi(word[1]))<1 && strcmp(word[1],"all"))) {
		write_user(user,"Pouzi: .dmail [<cislo spravy>] [<od> <do>] [all]\n");
		return;
	}
	from=atoi(word[1]);
	if ((word_count==2) && (from>=1))
		to=from; /* Uprava -> iba JEDEN message */
	if (((word_count==3) && (((from=atoi(word[1]))<1) || ((to=atoi(word[2]))<1)))) {
		write_user(user,"Pouzi: .dmail [<cislo spravy>] [<od> <do>] [all]\n");
		return;
	}
	if (!strcmp(word[1],"all")) {
		sprintf(query,"delete from `mailbox` where `userid`='%d'",user->id);
		mysql_kvery(query);
		write_user(user,"Vsetka posta bola zmazana.\n");
		user->lastm=0;
		return;
	}
	if (from>to) {
		write_user(user,"Prve cislo musi byt mensie (alebo rovne) ako druhe!\n");
		return;
	}
	sprintf(query,"select `msgid` from `mailbox` where `userid`='%d' order by `time` limit %d,1",user->id,from-1);
	if ((result=mysql_result(query))) {
		if ((row=mysql_fetch_row(result)) && row[0]) {
			fromid=atoi(row[0]);
		}
		else {
			mysql_free_result(result);
			sprintf(query,"select count(`msgid`) from `mailbox` where `userid`='%d'",user->id);
			sprintf(text,"V schranke mas len %d sprav.\n",query_to_int(query));
			write_user(user,text);
			return;
		}
	}
	else {
		write_user(user,"Nastala chyba pri mazani posty!\n");
		return;
	}

	if (from==to)
		toid=fromid;
	else {
		sprintf(query,"select `msgid` from `mailbox` where `userid`='%d' order by `time` limit %d,1",user->id,to-1);
		if ((result=mysql_result(query))) {
			if ((row=mysql_fetch_row(result)) && row[0])
				toid=atoi(row[0]);
			else
				toid=0;
			mysql_free_result(result);
		}
		else {
			write_user(user,"Nastala chyba pri mazani posty!\n");
			return;
		}
	}
	if (toid==0)
		sprintf(query,"delete from `mailbox` where `userid`='%d' and %d<=`msgid`",user->id,fromid);
	else
		sprintf(query,"delete from `mailbox` where `userid`='%d' and %d<=`msgid` and `msgid`<=%d",user->id,fromid,toid);
	mysql_kvery(query);
	vyfluslo=mysql_affected_rows(&mysql);
	sprintf(query,"select count(`msgid`) from `mailbox` where `userid`='%d'",user->id);
	cnt=query_to_int(query);
	if (cnt==0) {
		sprintf(text,"V schranke %s %d sprav%s, schranka je teraz prazdna.\n",skloncislo(vyfluslo,"bola","boli","bolo"),vyfluslo,skloncislo(vyfluslo,"a","y",""));
		user->lastm=0;
		write_user(user,text);
		return;
	}
	else {
		sprintf(text,"%d %s, zostalo %d.\n",vyfluslo,skloncislo(vyfluslo,"sprava bola zmazana","spravy boli zmazane","sprav bolo zmazanych"),cnt);
		user->lastm=cnt;
		write_user(user,text);
	}
}

/* Show list of people your mail is from without seeing the whole lot */
int mail_from(UR_OBJECT user,int echo) /* vrati pocet sprav (V) */
{
	int cnt=0,zobraz=0,i=0,mailboxsize=0;
	double dbl;

	sprintf(query,"select `sender`,`time`,UNIX_TIMESTAMP(`time`) from `mailbox` where `userid`='%d' order by `time`",user->id);
	if ((result=mysql_result(query))) {
		while ((row=mysql_fetch_row(result))) {
			cnt++;
			sprintf(text,"~OL~FY%3d:~DY Sprava od:~HW %s ~DYprijata ~HW%s ~HB===\n",cnt,row[0]?row[0]:"",row[1]?sqldatum(row[1],user->colour?1:0,0):"??.??.???? ??:??");
			if (word[1][0]!='n' || atoi(row[2])>user->read_mail) {
				if (cnt==1 && echo && word[1][0]!='n') write_user(user,"\n~RS~FM=== ~FGOdosielatelia sprav v Tvojej postovej schranke: ~FM===\n\n");
				if (zobraz==0 && echo && word[1][0]=='n')
					write_user(user,"\n~RS~FM=== ~FGOdosielatelia NOVYCH sprav v Tvojej postovej schranke: ~FM===\n\n");
				if (echo)
					write_user(user,text);
				zobraz++;
			}
		}
		mysql_free_result(result);
	}
	if (!echo)
		return cnt;
	if (word[1][0]=='n') {
		if (!zobraz) {
			write_user(user,"~FGNeboli najdene ziadne nove spravy.\n");
			return cnt;
		}
		else {
			sprintf(text,"\n~FGPocet novych sprav : ~FT%d\n",zobraz);
			write_user(user,text);
		}
	}
	else {
		if (!cnt) {
			if (echo)
				write_user(user,"Nemas ziadnu postu.\n");
			return 0;
		}
		else {
			sprintf(text,"\n~FGCelkovy pocet sprav: ~FT%d\n",cnt);
			write_user(user,text);
		}
	}
	mailboxsize=0;
	sprintf(query,"select sum(length(`message`)) from `mailbox` where `userid`='%d'",user->id);
	mailboxsize=query_to_int(query);
	i=db_user_switch(user->id,2); /* no quota userlist */
	if (mailboxsize) {
		dbl=(mailboxsize*100)/(MAIL_QUOTA*(1+i*20));
		sprintf(text,"~FGVyuzitie schranky: ~FT%5.1f%%\n",dbl);
		write_user(user,text);
	}
	if (mailboxsize>MAIL_LIMIT*(1+i*20))
		write_user(user,"~OL~FRPOZOR: ~FYTvoja postova schranka je plna, vycisti si ju prikazom .dmail!\n");
	else if (mailboxsize>MAIL_QUOTA*(1+i*20))
		write_user(user,"~OL~FRPOZOR: ~FYMas preplnenu postovu schranku; pokial si ju nevycistis (prikaz .dmail)\n       ~OL~FYnebudes moct dostavat dalsiu postu!\n");
	return cnt;
}

/* See if user has unread mail, mail file has last read time on its first line */
int has_unread_mail(UR_OBJECT user)
{
	sprintf(query,"select max(UNIX_TIMESTAMP(`time`)) from `mailbox` where `userid`='%d'",user->id);
	if (query_to_int(query)>(int)user->read_mail)
		return 1;
	return 0;
}

/* This is function that sends mail to other users */
void send_mail(UR_OBJECT user,char *to, char *ptr)
{
	char emailadr[102];
	char rov[60];
	char header[100];
	UR_OBJECT u;
	unsigned int i,max=0,uid=0;

	sstrncpy(emailadr, to, 100);
	if (strstr(emailadr,"@")) { /* Je to E-MAIL sprava. */
		if (strpbrk(emailadr," ;$/+*[]\\|<>")) {
			write_user(user,"Nespravne zadana email adresa!\n");
			return;
		}
		if ((!strstr(emailadr,".")) || (!strstr(emailadr,"@")) || (!isalpha(emailadr[strlen(emailadr)-1])) || (strlen(emailadr)<7)) {
			write_user(user,"Nespravne zadana email adresa!\n");
			return;
		}
		send_email(user,emailadr,ptr);
		write_user(user,"E-mail bol odoslany.\n");
		return;
	}
	/* Local mail */
	uid=db_userid(to);
	sprintf(query,"select max(`msgid`) from `mailbox` where `userid`='%d'",uid);
	max=query_to_int(query);
	if (user==NULL) {
		sprintf(query,"insert into `mailbox` (`userid`,`msgid`,`time`,`sender`,`message`) values ('%d','%d',FROM_UNIXTIME('%d'),'%s','%s')",uid,max+1,(int)time(0),"MAILER",dbf_string(ptr));
	}
	else {
		rov[0]='\0';
		strcpy(rov,"=");
		for (i=1; i<=6+(12-strlen(user->name)); i++)
			strcat(rov,"=");
		sprintf(query,"insert into `mailbox` (`userid`,`msgid`,`time`,`sender`,`message`) values ('%d','%d',FROM_UNIXTIME('%d'),'%s','%s')",uid,max+1,(int)time(0),user->name,dbf_string(ptr));
	}
	if (mysql_query(&mysql,query)) {
		sprintf(text,"~OL~FRMYSQL RESULT ERROR: %s\n",mysql_error(&mysql));
		write_level(KIN,1,text,NULL);
		colour_com_strip(text);
		write_syslog(text,1);
		write_user(user,"Chyba pri dorucovani posty.\n");
		return;
	}
	if (user->lang)
		write_user(user,"~OL~FWMail sent.~RS~FW\n");
	else
		write_user(user,"~OL~FWPosta odoslana.~RS~FW\n");
	u=get_user_exact(to);
	if (u!=NULL)
		write_user(u,"~LB~FT~OL** ~LIPRISLA TI NOVA POSTA!~RS~OL~FT (pouzi: ~FY.rmail new~FT) **\n");
	sprintf(header,"%s [%s, %02d:%02d]",user->name, long_date(4),thour,tmin);
	forward_email(to,header,user->name,ptr);
	return;
}

/*** Read the message board ***/
void read_board(UR_OBJECT user)
{
	RM_OBJECT rm=NULL;
	char filename[80],*name;
	int ret;

	if (word_count<2)
		rm=user->room;
	if ((user->rjoke_from=atoi(word[1]))!=0) {
		rm=user->room;
	}
	else {
		if (word_count>1) {
			if ((rm=get_room(word[1],user))==NULL) {
				write_user(user,nosuchroom);
				return;
			}
			if (!has_room_access(user,rm)) {
				write_user(user,"Ta miestnost je sukromna, nemozes tam citat nastenku.\n");
				return;
			}
		}
	}
	if (!strcmp(rm->name,"cintorin")) {
		write_user(user,title("~FW~OLTu odpocivaju v pokoji:","~OL~FK"));
		sprintf(filename,"%s%csuicides",DATAFILES,DIRSEP);
		if (!(ret=more(user,user->socket,filename))) {
			write_user(user,"Tabula s umrtnymi oznameniami je prazdna.\n\n");
			user->browsing=0;
		}
		else if (ret==1)
			user->misc_op=2;
		if (user->vis)
			name=user->name;
		else
			name=invisname(user);
		if (rm==user->room) {
			sprintf(text,"%s cita tabulu s umrtnymi oznameniami.\n",name);
			write_room_except(user->room,text,user);
		}
		return;
	}
	if (word_count>2) {
		user->rjoke_from=atoi(word[2]);
	}
	if (user->rjoke_from<1)
		user->rjoke_from=1;
	sprintf(text,"\n~FW--~OL-> ~RS~FWNastenka v miestnosti ~OL%s~RS~FW\n\n",rm->name);
	write_user(user,text);
	strcpy(user->subject,rm->name);
	user->browsing=3;
	user->messnum=user->rjoke_from;
	user->messsize=0;
	if (!(ret=mailmore(user)))
		user->browsing=0;
	else if (ret==1)
		user->misc_op=223;
	if (user->vis)
		name=user->name;
	else
		name=invisname(user);
	if (rm==user->room) {
		sprintf(text,"%s cita nastenku.\n",name);
		write_room_except(user->room,text,user);
	}
}

void delete_joke(UR_OBJECT user)
{
	int from=0,to=0,fromid=0,toid=0;
	int vyfluslo;

	if (word_count<2 || (atoi(word[1])<1)) {
		write_user(user,"Pouzi: .djoke [<cislo spravy>] [<od> <do>]\n");
		return;
	}
	from=atoi(word[1]);
	if ((word_count==2) && (from>=1))
		to=from;
	if (((word_count==3) && (((from=atoi(word[1]))<1) || ((to=atoi(word[2]))<1)))) {
		write_user(user,"Pouzi: .djoke [<cislo spravy>] [<od> <do>]\n");
		return;
	}
	if (from>to) {
		write_user(user,"Prve cislo musi byt mensie (alebo rovne) ako druhe!\n");
		return;
	}
	sprintf(query,"select `id` from `jokes` order by `id` desc limit %d,1",from-1);
	if ((result=mysql_result(query))) {
		if ((row=mysql_fetch_row(result)) && row[0])
			fromid=atoi(row[0]);
		else {
			mysql_free_result(result);
			sprintf(query,"select count(`id`) from `jokes`");
			sprintf(text,"Na jokeboarde je len %d vtipov!\n",query_to_int(query));
			write_user(user,text);
			return;
		}
	}
	else {
		write_user(user,"Nastala chyba pri mazani vtipov!\n");
		return;
	}
	if (from==to)
		toid=fromid;
	else {
		sprintf(query,"select `id` from `jokes` order by `id` desc limit %d,1",to-1);
		if ((result=mysql_result(query))) {
			if ((row=mysql_fetch_row(result)) && row[0])
				toid=atoi(row[0]);
			else
				toid=0;
			mysql_free_result(result);
		}
		else {
			write_user(user,"Nastala chyba pri mazani vtipov!\n");
			return;
		}
	}
	if (toid==0) {
		write_user(user,"Davaj pozor co mazes !\n");
		return;
	}
	sprintf(query,"delete from `jokes` where %d<=`id` and `id`<=%d",toid,fromid); /* sorted descending */
	mysql_kvery(query);
	vyfluslo=mysql_affected_rows(&mysql);
	sprintf(query,"select count(`id`) from jokes");
	pocet_vtipov=query_to_int(query);
	if (pocet_vtipov==0) {
		sprintf(text,"Jokeboard je teraz prazdny! TY BARBAR!\n");
		write_user(user,text);
		return;
	}
	else {
		sprintf(text,"%d %s, zostalo %d.\n",vyfluslo,skloncislo(vyfluslo,"vtip bol zmazany","vtipy boli zmazane","vtipov bolo zmazanych"),pocet_vtipov);
		user->lastm=pocet_vtipov;
		write_user(user,text);
	}
}

int counter_db(int add) {

	int cntr;
	MYSQL_RES *loc_result;
	MYSQL_ROW loc_row;

	sprintf(query,"select `value` from `config` where `parameter` = 'counter'");
	if ((loc_result=mysql_result(query))) {
		loc_row=mysql_fetch_row(loc_result);
		cntr=atoi(loc_row[0]);
		mysql_free_result(loc_result);
	}
	else {
		return 0;
	}
	if (add) {
		cntr++;
		sprintf(query,"update `config` set `value` = '%d' where `parameter` = 'counter'",cntr);
		mysql_kvery(query);
		return cntr;
	}
	else {
		return cntr;
	}
}
