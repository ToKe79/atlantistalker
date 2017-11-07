/***************************************************************************
         IRC KLIENT pre Atlantis (C) 29.12.1999-3.1.2000 Spartakus
         Status line (C) Rider of LoneStar Software
****************************************************************************/
#include "atl-head.h"
#include "atl-mydb.h"

char *get_ircserv_name(char *id)
{
static char nazov[100];
char iden[15], server[50], port[8];
FILE *fp;

nazov[0]='\0';
if ((fp=fopen(IRC_SERVERS, "r"))==NULL) return nazov;
fscanf(fp, "%s %s %s", iden, server, port);
while (!feof(fp)) {
	if (!strcmp(id, iden)) {
		sprintf(nazov, "%s %s", server, port); 
		fclose(fp);
		return nazov;
		}
	fscanf(fp, "%s %s %s", iden, server, port);
	}

fclose(fp);
return nazov;
}

/*** Attempt to get '\n' terminated line of input from a character
     mode client else store data read so far in user buffer. ***/
int get_charclient_line(user,inpstr,len)
UR_OBJECT user;
char *inpstr;
int len;
{
int l;

if ((user->statline==CHARMODE) && (user->doom) && (len==1) && !(user->buffpos)) {
   switch(inpstr[0]) {
     case ' ': strcpy(inpstr,".doom x\n");
     	       len=8;
	       break;
     default : break;
     }
   }

if ((user->statline==CHARMODE) && (user->filepos) && (len==1) && (!user->buffpos)) {
   switch(inpstr[0]) {
     case 'b':
     case 'B': strcpy(inpstr,"b\n");
     	       len=2;
     	       break;
     case 'r':
     case 'R': strcpy(inpstr,"r\n");
     	       len=2;
     	       break;
     case 'e':
     case 'E': strcpy(inpstr,"e\n");
     	       len=2;
     	       break;
     default:  break;
     }
   }

for(l=0;l<len;++l) {
      /* see if delete entered */
      if (inpstr[l]==8 || inpstr[l]==127) {
            if (user->buffpos) {
                  user->buffpos--;
                  if (user->charmode_echo && user->statline!=CHARMODE) write_user(user,"\b \b");
                  }
            continue;
            }

      if (user->statline==CHARMODE && inpstr[l]==9) {
      		  if (user->actual_remote_socket!=0 && user->actual_remote_socket==user->ircsocknum) {
      		  	sprintf(user->buff,"/msg ");
      		  	}
      		  else sprintf(user->buff,".tell %s ",user->lasttellfrom);
      		  user->buffpos=strlen(user->buff);
      		  text_statline(user,0);
      		  return 0;
                  }

      user->buff[user->buffpos]=inpstr[l];
      /* See if end of line */
      if ((inpstr[l]>0 && inpstr[l]<32) || user->buffpos+2==BUFSIZE) {
            terminate(user->buff);
            strcpy(inpstr,user->buff);
            if (user->charmode_echo && user->statline!=CHARMODE) write_user(user,"\n");
            else if (user->statline==CHARMODE) text_statline(user,1);
            return 1;
            }
      if (inpstr[l]!=0) user->buffpos++;
      }
if (user->charmode_echo && user->statline!=CHARMODE
    && ((user->login!=2 && user->login!=1) || password_echo)) write(user->socket,inpstr,len);

if (user->statline==CHARMODE) text_statline(user,0);
return 0;
}

/************* STATUS LAJNA ************/

void statline(UR_OBJECT user)
{

if (user->akl || user->socket>1000) {
	write_user(user,"Prepac, tato funkcia nieje dostupna ked sa pripajas cez Atlantis Klienta.\n");
	return;
	}
	
 if (user->statline!=NONEMODE) {
    if (user->statline==CHARMODE) {
       write2sock(user,user->socket,"\033[?25h\033[?7h",0);
       sprintf(text,"%c%c%c%c%c%c%c%c%c%c%c%c%c",IAC,DO,TELOPT_ECHO,IAC,DO,TELOPT_LINEMODE,IAC,SB,TELOPT_LINEMODE,LM_MODE,MODE_EDIT,IAC,SE);
       write_user(user,text);
       }
 
    user->statline=NONEMODE;
    echo_on(user);

    if (user->statlcount) {
        user->lines=user->statlcount;
	if (user->doom) user->lines-=D_LINES;
        user->statlcount=0;
        }

    if (user->doom) {
       sprintf(text,"\033[%d;%dr",D_LINES+1,user->lines+D_LINES);
       write2sock(user,user->socket,text,0);
       cls(user,D_LINES);
       }
    else write2sock(user,user->socket,"\033[2;1r\033c\033[?7h",0); /* CRT oblaf ;-) */
    cls(user,0);
    write_user(user,"Stavovy riadok bol ~FRZRUSENY~FW!\n\n");
    }
 else {
    write_user(user,"Stlac ~OLMEDZERU~RS (+ ~OLENTER~RS, ak treba) pre zapojenie stavoveho riadku!\n");
    user->statline=UNKNOWN;
    sprintf(text,"%c%c%c%c%c%c%c%c%c",IAC,WILL,TELOPT_SGA,IAC,WILL,TELOPT_ECHO,IAC,DONT,TELOPT_LINEMODE);
    write_user(user,text); /* pridat IAC, WONT, TELOPT_ECHO */
    if (user->statlcan==2) boot_statline(user);
   }
}

void boot_statline(UR_OBJECT user)
{
 user->statlcount=user->lines;
 user->statlcan=2;
 if (user->doom) user->statlcount+=D_LINES;
 user->lines-=2;
 /*** zapojenie statline ***/
 init_statline(user);
 write_user(user,"Stavovy riadok ~FGZAPOJENY~FW!\n");
 user->statline=CHARMODE;
}


void init_statline(UR_OBJECT user)
{
char smalbuf[100];

sprintf(smalbuf,"\033[?25l\033[%d;%dr",((user->doom)?(D_LINES+1):1),(user->statlcount)-2);
write2sock(user,user->socket,smalbuf,0); /* zapni scrollregion */
echo_off(user); /* vypni charecho (escape) */
if (user->doom) cls(user,D_LINES);
cls(user,0);      /* vymaz screen */
show_statline(user);
text_statline(user,1);
}

void show_statline(UR_OBJECT user)
{
char sequencia[800];
int x;
int mins;

sprintf(sequencia,"\0337\033[%d;1H",(user->statlcount)-1); /* save+goto Y,1 */
write2sock(user,user->socket,sequencia,0);
user->newline=0;

//sprintf(sequencia,"~BB    ~OL~FW%-12.12s      [ ~FYATLANTIS talker~FW ]                 %-16.16s      ",user->name,user->room->name);
mins=(int)(time(0) - user->last_login)/60;      
if (user->actual_remote_socket && user->actual_remote_socket==user->ircsocknum) {
	sprintf(sequencia,"~BB~OL~FW %-12.12s ~FR * ~FG%02d:%02d %03d ~FY%3dpp ~FR* ~OL~FW%-16.16s ~FR* ~FY    Atlantis TALKER       ", user->irc_nick, thour, tmin, mins, user->pp, user->irc_chan);
	}
	else sprintf(sequencia,"~BB~OL~FW %-12s ~FR * ~FG%02d:%02d %03d ~FY%3dpp ~FR* ~OL~FW%-16s ~FR* ~FY    Atlantis TALKER       ", user->name, thour, tmin, mins, user->pp, user->room->name);

if (user->colms>80 && user->colms<600) /* baxa na overflow :) */
   for (x=user->colms; x>80; x--) strcat(sequencia," ");
strcat(sequencia,"~BK~FW~RS\n\n");

write_user(user,sequencia);
write2sock(user,user->socket,"\0338",0); /* restore cursor */
}

void text_statline(UR_OBJECT user, int crflag)
{
char sequencia[150];
char *ptr;
int term, i,j, k, oddel;
RN_OBJECT remote;

sprintf(sequencia,"\0337\033[%d;1H",user->statlcount); /* save+goto Y,1 */
write2sock(user,user->socket,sequencia,0);
//write_sock(user->socket,"[TTT] ");

i=0;
text[0]='\0';
if (user->buffpos && !isalpha(user->buff[0]) && !isdigit(user->buff[0]) && !strchr("!@$<>-:/,",user->buff[0])) { /* aby to tam neleezlo furt */
	for (remote=remote_start;remote!=NULL;remote=remote->next) {
		if (remote->shortcut==user->buff[0]) {
			for (j=0;j<MAX_CONNECTIONS;j++) if (user->remote[j]==remote) {
				//i=j; break;
				sprintf(text,"[%-3.3s] ", remote->desc);
				write2sock(user,user->socket,text,0);
				i=1; 
				break;
				}
			}
		}
	if (user->buff[0]=='+') {
		write2sock(user,user->socket,"[ATL] ",0);
		i=1;
		}
	}

k=-1;
if ((!i || crflag) && user->actual_remote_socket!=0) {
	for (j=0;j<MAX_CONNECTIONS;j++)
	if (user->remote_socket[j]==user->actual_remote_socket) {
		sprintf(text,"[%-3.3s] ", user->remote[j]->desc);
		write2sock(user,user->socket,text,0);		
		k=j;
		}
	}

oddel=78;
if (i || k!=-1) oddel-=7;
if (!crflag) {
  ptr=user->buff;
  term=user->buffpos;
  while (term>oddel) {
     term-=60;
     ptr+=60;
     }
  if (ptr==user->buff && i) { term--; ptr++; }
  strncpy(sequencia,ptr,100);
  sequencia[term]='\0';
  strcat(sequencia,"\033[7m \033[0m"); /* kurzor! :) */
  }
else {
	strcpy(sequencia,"\r");
	if (k!=-1) strcat(sequencia, text);
	strcat(sequencia,"\033[K\033[7m \033[0m"); /* cr+zmaz_do_konca+kurzor */
	}

write2sock(user,user->socket,sequencia,0);
write2sock(user,user->socket,"\033[K\0338",0);
}


void irc(UR_OBJECT user)
{

if (word_count<2) {
	write_user(user,"Pouzi: .irc [<connect> | <open>] [<disconnect> | <close>]\n");
	write_user(user,"       .irc connect      - pripoji ta na IRC\n");
	write_user(user,"       .irc disconnect   - odpoji ta z IRC\n");
	return;
	}
	
if (word[1][1]=='o' || word[1][0]=='o') {
	/* CONNECT */
	word_count=3;
	strcpy(word[1],"connect");
	strcpy(word[2],"IRC");
	view_remote(user);
	return;
	}
if (word[1][0]=='d' || word[1][1]=='l') {
	/* DISCONNECT */
	word_count=3;
	strcpy(word[1],"disconnect");
	strcpy(word[2],"IRC");
	view_remote(user);
 	return;
	}
write_user(user,"Pouzi: .irc [<connect> | <open>] [<disconnect> | <close>]\n");	
write_user(user,"       .irc connect      - pripoji ta na IRC\n");
write_user(user,"       .irc disconnect   - odpoji ta z IRC\n");

}


void load_irc_details(UR_OBJECT user)
{
int ok=1;

sprintf(query,"select defnick,channel,server,name from irc where userid='%d';",user->id);
if (!(result=mysql_result(query))) ok=0;
if (ok==1) {
  if ((row=mysql_fetch_row(result))) {
    if (row[0]) strcpy(user->irc_defnick,row[0]); else strcpy(user->irc_defnick, user->name);
    if (row[1]) strcpy(user->irc_chan,row[1]); else strcpy(user->irc_chan,"#atlantis");
    if (row[2]) strcpy(user->irc_serv,row[2]); else strcpy(user->irc_serv,"nextra1");
    if (row[3]) strcpy(user->irc_name,row[3]); else sprintf(user->irc_name,"%s z Atlantidy", user->name);
    ok=2;
   }
  mysql_free_result(result);
 }
if (ok<2) {
  strcpy(user->irc_defnick, user->name);
  sprintf(user->irc_name,"%s z Atlantidy", user->name);
  strcpy(user->irc_serv,"nextra1");
  strcpy(user->irc_chan,"#atlantis");
 }
}

void save_irc_details(UR_OBJECT user)
{

if (user->irc_defnick[0]=='\0' || user->irc_name[0]=='\0') /*sstrncpy(user->irc_defnick, user->name, 20);*/ return;
if (user->level<SAG) return;

if (user->irc_chan[0]=='\0') strcpy(user->irc_chan,"#atlantis"); 
if (user->irc_serv[0]=='\0') strcpy(user->irc_serv,"nextra1");
sprintf(query,"replace into irc (userid,defnick,channel,server,name) values ('%d','",user->id);
strcat(query,dbf_string(user->irc_defnick)); strcat(query,"','");
strcat(query,dbf_string(user->irc_chan)); strcat(query,"','");
strcat(query,dbf_string(user->irc_serv)); strcat(query,"','");
strcat(query,dbf_string(user->irc_name)); strcat(query,"');");
mysql_kvery(query);
}


void irc_prikaz(UR_OBJECT user, char *inpstr)
{
char out[700];
int wc;
int i;

endstring(inpstr);
inpstr[500]='\0';
sstrncpy(out,inpstr, 512);

//clear_words();
//wc=wordfind(inpstr);
wc=word_count;
if (!wc) return;

if (word[0][0]!='/') {
	if (!user->irc_chan[0]) {
		write_user(user,"~OL~FWNiesi pripojeny na ziadny kanal, pouzi: /join #kanal\n");
		return;
		}
	sprintf(out,"PRIVMSG %s :%s\n", user->irc_chan, inpstr);
	sprintf(text,"~FM<~FW%s:~FT%s~FM> ~FW%s\n", user->irc_chan, user->irc_nick, inpstr);
	record_irc(user,text);
	write_user(user,text);
	}

strtolower(word[0]);
	
if (!strncmp(word[0],"/ch",3)) {
	if (wc<2) {
		if (user->irc_chan[0]=='\0') write_user(user,"~OL~FB=~FW*~FB=~RS~FW Este niesi na ziadnom kanali (pouzi /join)\n");
			else {
			sprintf(text,"~OL~FB=~FW*~FB=~RS~FW [~OL%s~RS] ", user->irc_chan);
			for (i=0; i<10; i++) if (user->channels[i][0]!='\0' && strcmp(user->channels[i], user->irc_chan)) {
				strcat(text,user->channels[i]);
				strcat(text," ");
				}
			strcat(text,"\n");
			write_user(user,text);			
			}
		return;
		}
		else {
		for (i=0; i<10; i++) if (!strcmp(user->channels[i], word[1])) {
			strcpy(user->irc_chan, user->channels[i]);
			sprintf(text,"~OL~FB=~FW*~FB= [~FY%s~FB]~RS~FW\n", word[1]);
			write_user(user,text);
			return;
			}
		}
	sprintf(text,"~OL~FB=~FW*~FB=~RS~FW Na kanal ~OL%s~RS niesi %s.\n", word[1], pohl(user,"prihlaseny","prihlasena"));
	write_user(user,text);
	return;
	}	
	
/* JOIN */
if (!strncmp(word[0],"/j",2)) {
	if (wc<2) {
		write_user(user,"Pouzi: /join <kanal[,kanal]> [kluc,[kluc]]\n");
		return;
		}
	if (!user->irc_reg) {
		write_user(user,"Musis sa najprv registrovat (prikaz: /nick)\n");
		return;
		}
	sstrncpy(user->irc_chan, word[1], 49);
	if (user->statline==CHARMODE) show_statline(user);
	if (wc<3) sprintf(out, "JOIN %s\n", word[1]);
		else sprintf(out, "JOIN %s %s\n", word[1], word[2]);
	sprintf(text,"~OL~FB=~FW*~FB= [~FY%s~FB]~RS~FW\n", user->irc_chan);
	write_user(user,text);
	}

/* NICK */
if (!strncmp(word[0],"/ni", 3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /nick <novy nick>\n");
		return;
		}
	sprintf(out,"NICK %s\n", word[1]);
	if (!user->irc_reg) {
		sstrncpy(user->irc_nick,word[1],20);
		if (user->statline==CHARMODE) show_statline(user);
		}
	}
	
/* QUIT */
if (!strncmp(word[0],"/quit",5)) {
	if (wc<2) sprintf(out,"QUIT\n");
		else {
			inpstr=remove_first(inpstr);
			sprintf(out,"QUIT :%s\n", inpstr);
			}
	}
	
/* ADMIN */	
if (!strncmp(word[0],"/ad",3)) {
	if (wc>1) sprintf(out, "ADMIN %s\n", word[1]);
		else sprintf(out,"ADMIN\n");
	}

/* AWAY */
if (!strncmp(word[0],"/aw",3)) {
	inpstr=remove_first(inpstr);
	if (wc>1) sprintf(out, "AWAY :%s\n", inpstr);
		else sprintf(out, "AWAY\n");	
	}

/* NOTICE */
if (!strncmp(word[0],"/no",3)) {
	if (wc<3) {
		write_user(user,"Pouzi: /notice <nick[,nick]> <text>\n");
		return;
		}
	inpstr=remove_first(remove_first(inpstr));
	sprintf(out,"NOTICE %s :%s\n", word[1], inpstr);
	sprintf(text,"~OL~FK-~RS~FRnotice(->~OL~FR%s~RS~FR)~OL~FK-~RS~FW %s\n", word[1], inpstr);
	record_irc(user,text);
	write_user(user,text);
	}

/* MSG, PRIVMSG */
if (!strncmp(word[0],"/ms",3) || !strncmp(word[0],"/pr",3) || !strcmp(word[0],"/m")) {
	if (wc<3) {
		write_user(user,"Pouzi: /msg <nick[,nick] {kanal,[kanal]}> <text>\n");
		return;
		}
	inpstr=remove_first(remove_first(inpstr));
	sprintf(out,"PRIVMSG %s :%s\n", word[1], inpstr);
	if (word[1][0]=='#' || word[1][0]=='&') {
		sprintf(text,"~FM<~FW%s:%s~FM> ~FW%s\n", word[1], user->irc_nick, inpstr);
		}
		else sprintf(text,"~OL~FK[~RS~FRmsg(->~OL~FR%s~RS~FR)~OL~FK]~RS~FW %s\n", word[1],inpstr);
	record_irc(user,text);
	write_user(user,text);
	}

/* INFO */
if (!strncmp(word[0],"/inf", 4)) sprintf(out,"INFO\n");

/* INVITE */
if (!strncmp(word[0],"/inv", 4)) {
	if (wc<2) {
		write_user(user,"Pouzi: /invite <nick> [kanal]\n");
		return;
		}
	if (wc<3) {
		sprintf(out,"INVITE %s %s\n", word[1], user->irc_chan);
		}
		else sprintf(out,"INVITE %s %s\n", word[1], word[2]);
	}

/* KICK (ufff :-) */

if (!strncmp(word[0],"/k",2)) {
	if (wc<2) {
		write_user(user,"Pouzi: /kick [kanal] <nick> [komentar]\n");
		return;
		}
	if (word[1][0]=='#' || word[1][0]=='&') {
		if (wc<3) {
			write_user(user,"Pouzi: /kick [kanal] <nick> [komentar]\n");
			return;
			}
		if (wc>3) {
			inpstr=remove_first(remove_first(remove_first(inpstr)));
			sprintf(out,"KICK %s %s :%s\n", word[1], word[2], inpstr);
			}
			else sprintf(out,"KICK %s %s\n", word[1], word[2]);
		}
		else {
			if (wc>2) {
				inpstr=remove_first(remove_first(inpstr));
				sprintf(out,"KICK %s %s :%s\n", user->irc_chan, word[1], inpstr);
				}
				else sprintf(out,"KICK %s %s\n", user->irc_chan, word[1]);
			}
	}
	
/* ISON */
if (!strncmp(word[0],"/is",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /ison <nick>\n");
		return;
		}
	sprintf(out,"ISON %s\n", word[1]);
	}

/* LIST */
if (!strncmp(word[0],"/li",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /list <parameter>\n");
		return;
		}
	sprintf(out,"LIST %s\n", word[1]);
	}

/* LUSERS */
if (!strncmp(word[0],"/lu",3)) {
	sprintf(out,"LUSERS\n");
	}

/* MODE */
if (!strncmp(word[0],"/mod",4)) {
	if (wc<2) {
		write_user(user,"Pouzi: /mode [kanal] {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<nick>] [<banmask>]\n");
		return;
		}
	if (word[1][0]=='#' || word[1][0]=='&') {
		if (wc<4) sprintf(out,"MODE %s %s\n", word[1], word[2]);
			else sprintf(out,"MODE %s %s %s\n", word[1], word[2], word[3]);
		}
		else {
			if (wc<3) sprintf(out,"MODE %s %s\n", user->irc_chan, word[1]);
				else sprintf(out,"MODE %s %s %s\n", user->irc_chan, word[1], word[2]);
			}		
	}

/* MOTD */
if (!strncmp(word[0],"/mot",4)) {
	sprintf(out,"MOTD\n");
	}

/* NAMES */
if (!strncmp(word[0],"/na", 3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /names [<kanal>{,<kanal>}]\n");
		return;
		}
	sprintf(out,"NAMES %s\n", word[1]);
	}


/* LEAVE / PART */
if (!strncmp(word[0],"/pa",3) || !strncmp(word[0],"/le",3)) {
	if (wc<2) sprintf(out,"PART %s\n", user->irc_chan);
		
	if (word[1][0]=='#' || word[1][0]=='&') {
		if (wc<3) sprintf(out,"PART %s\n", word[1]);
			else {
				inpstr=remove_first(remove_first(inpstr));
				sprintf(out,"PART %s :%s\n", word[1], inpstr);
				}			
		}
	else {
		inpstr=remove_first(inpstr);
		sprintf(out,"PART %s :%s\n", user->irc_chan, inpstr);
		}
	
	//user->irc_chan[0]='\0';
	}


/* TOPIC */

if (!strncmp(word[0],"/top",4)) {
	if (wc<2) sprintf(out,"TOPIC %s\n", user->irc_chan);
		else if (word[1][0]=='#' || word[1][0]=='&') {
			if (wc<3) sprintf(out,"TOPIC %s\n", word[1]);
			else {
				inpstr=remove_first(remove_first(inpstr));
				sprintf(out,"TOPIC %s :%s\n", word[1], inpstr);
				}
			}
			else {
				inpstr=remove_first(inpstr);
				sprintf(out,"TOPIC %s :%s\n", user->irc_chan, inpstr);
				}
	}


/* TIME */
if (!strncmp(word[0],"/ti",3)) {
	sprintf(out,"TIME\n");
	}
	
/* VERSION */
if (!strncmp(word[0],"/ve",3)) {
	sprintf(out,"VERSION\n");
	}

/* WHOIS */
if (!strncmp(word[0],"/whoi",5) || !strncmp(word[0],"/wi",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /whois [<server>] <nick>[,<nick>,[...]]\n");
		return;
		}
	else if (wc<3) sprintf(out,"WHOIS %s\n", word[1]);
		else sprintf(out,"WHOIS %s %s\n", word[1], word[2]);
	goto SEND;
	}

/* WHOWAS */
if (!strncmp(word[0],"/whow",5) || !strncmp(word[0],"/ww",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /whowas <nick> [<pocet> [<server>]]\n");
		return;
		}
	else if (wc<3) sprintf(out,"WHOWAS %s\n", word[1]);
		else if (wc<4) sprintf(out,"WHOWAS %s %s\n", word[1], word[2]);
			else sprintf(out,"WHOWAS %s %s %s\n", word[1], word[2], word[3]);
	goto SEND;
	}



/* WHO */
if (!strncmp(word[0],"/w",2)) {
	if (wc<2) {
		write_user(user,"Pouzi: /who <filter>\n");
		return;
		}
	sprintf(out,"WHO %s\n", word[1]);
	goto SEND;
	}


/* ME (EMOTE) */
if (!strcmp(word[0],"/me") || !strncmp(word[0],"/em",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /me [kanal] <hlaska>\n");
		return;
		}
	if (word[1][0]=='#') inpstr=remove_first(remove_first(inpstr));		
		else inpstr=remove_first(inpstr);	
	sprintf(text,"#ACTION %s", inpstr);
	text[strlen(text)-1]='\001'; text[0]='\001'; 
	if (word[1][0]=='#') {
		sprintf(out,"PRIVMSG %s :%s\n", word[1], text);
		sprintf(text,"%s:~OL%s~RS %s\n", word[1], user->irc_nick, inpstr);
		}
		else {
		sprintf(out,"PRIVMSG %s :%s\n", user->irc_chan, text);
		sprintf(text,"%s:~OL%s~RS %s\n", user->irc_chan, user->irc_nick, inpstr);
		}
	record_irc(user,text);
	write_user(user,text);
	}

/*
if (!strncmp(word[0],"/pi",3)) {
	if (wc<2) {
		write_user(user,"Pouzi: /ping <nick>\n");
		return;
		}	
	sprintf(text,"#PING %d", (int)time(0));
	text[strlen(text)-1]='\001'; text[0]='\001';
	sprintf(out, "PRIVMSG %s :%s\n", word[1],text);
	sprintf(text,"~OL~FB=~FW*~FB=~RS~FW PING request sent to ~FT%s.\n", word[1]);
	record_irc(user,text);
	write_user(user,text);
	}
*/

if (!strncmp(word[0],"/oaber",6)) {
	for (i=0; i<10; i++) {
		sprintf(text,"%d: %s\n", i, user->channels[i]);
		write_user(user,text);
		}
	return;
	}

if (!strncmp(word[0],"/rev",4)) {
	inpstr=remove_first(inpstr);
	revirc_command(user, inpstr);
	return;
	}

if (!strncmp(word[0],"/h",2)) {
	word_count=2;
	strcpy(word[1],"irccom");
	help(user,0);
	return;
	}

SEND:
write(user->ircsocknum,out,strlen(out));
return;
}

void prihlas_irc(UR_OBJECT user)
{
int i;

if (user->remote_login_socket==0) return;
user->irc_reg=0;
for (i=0; i<10; i++) user->channels[i][0]='\0';      
user->irc_chan[0]='\0';
if (user->irc_name[0]=='\0') sprintf(user->irc_name,"%s z Atlantidy", user->name);
sprintf(text,"USER %s atlantis.sk atlantis.sk :%s\n", user->name, user->irc_name);
write(user->remote_login_socket,text,strlen(text));
sleep (1);
if (user->irc_defnick[0]!='\0') {
	sprintf(text,"NICK %s\n", user->irc_defnick);
	strcpy(user->irc_nick, user->irc_defnick);
	}
	else {
	sprintf(text,"NICK %s\n", user->name);
	strcpy(user->irc_nick, user->name);
	}
write(user->remote_login_socket,text,strlen(text));
if (user->statline==CHARMODE) show_statline(user);
//strcpy(user->irc_nick, user->name);
return;
}

void spracuj_irc_vstup(UR_OBJECT user, char *inpstr)
{
int wc=0;
int curw, lastw;
char text2[2500];
char rnick[20], rident[200];
char *tmp;
int i;
//int y;

inpstr[512]='\0';
clear_irc_words();
wc=irc_wordfind(inpstr);
lastw=wc-1;
//endstring(lastw);
if (!wc) return;
curw=0;

text2[0]='\0';
/*
for(i=0; i<wc; i++) {
	sprintf(text2,"%s: ", irc_word[i]);
	for (y=0; y<strlen(irc_word[i]); y++) {
		sprintf(text,"%c(%d) ", irc_word[i][y], irc_word[i][y]);		
		strcat(text2,text);
		}
	strcat(text2,"\n");
	write_user(user,text2);
	}
*/
 
rnick[0]='\0'; rident[0]='\0';

sprintf(text,"%s", inpstr);
if (irc_word[0][0]==':') {
	/* zistime remote nick */
	if (strchr(irc_word[0],'!')) {
		sstrncpy(rident,strchr(irc_word[0],'!')+1,100);
		sstrncpy(rnick,&irc_word[0][1],18);
		tmp=strchr(rnick,'!');
		*tmp='\0';
		}		
	curw++;
	}

if (!strcmp(irc_word[curw],"PING")) {
	curw++;
	sprintf(text,"PONG %s\n", irc_word[curw]);
	write_irc(user, text);
	return;
	}	

if (!strcmp(irc_word[curw],"NICK")) {
	if (!strcmp(rnick, user->irc_nick)) {
		sstrncpy(user->irc_nick, irc_word[lastw],20);
		if (user->statline==CHARMODE) show_statline(user);
		}
		else { }
	sprintf(text,"~OL~FB=~FW*~FB=~FW %s ~RSis now known as ~FT%s\n", rnick, irc_word[lastw]);
	}

if (!strncmp(irc_word[curw],"00",2) || !strncmp(irc_word[curw],"25",2)) {
	//curw++;
	user->irc_reg=1;
	sprintf(text,"~FB[~OL~FY*~RS~FB] ~FW%s\n", irc_word[lastw]);
	}
if (!strcmp(irc_word[curw],"375") || !strcmp(irc_word[curw],"372")) {
	sprintf(text,"~FB[~OL~FY*~RS~FB] ~OL~FW%s\n", irc_word[lastw]);
	}
if (!strcmp(irc_word[curw],"376")) {
	user->irc_reg=1;
	sprintf(text,"~FB[~OL~FY*~RS~FB] =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	}
	
if (!strcmp(irc_word[curw],"NOTICE")) {	
	if (rnick[0]) {
		sprintf(text,"~OL~FK-~FM%s~RS~FM(<-notice~FM)~OL~FK-~RS~FW %s\n", rnick, irc_word[lastw]);
		}
	else sprintf(text,"~OL~FG!~RS~FG%s ~RS~FW%s\n", &irc_word[0][1], irc_word[lastw]);
	}
	
if (!strcmp(irc_word[curw],"JOIN")) {	
	sprintf(text,"~OL~FB=~FW*~FB= ~FY%s ~RS~FW(~FT%s~FW) has joined %s\n", rnick, rident , irc_word[curw+1]);
	if (!strcmp(rnick, user->irc_nick)) {
		for (i=0; i<10; i++) if (user->channels[i][0]=='\0') {
			sstrncpy(user->channels[i], irc_word[curw+1], 49);
			if (user->statline==CHARMODE) show_statline(user);
			break;
			}
		}
	}

if (!strcmp(irc_word[curw],"TOPIC")) {	
	sprintf(text,"~OL~FB=~FW*~FB= ~FW%s ~RShas changed the topic on %s to: %s\n", rnick, irc_word[curw+1], irc_word[lastw]);
	}
	
/* po JOINe */
//if (!strcmp(irc_word[curw],"353")) {
//	sprintf(text,"~OL~FB=~FW*~FB= ~FYUsers on %s: ~RS~FG%s\n", irc_word[curw+3], irc_word[lastw]);
//	}

/* WHO */
if (!strcmp(irc_word[curw],"352")) {       
	sprintf(text,"~OL~FY%-10s ~FW%-10s ~RS~FT%-3s ~FW%s@%s (%s)\n", irc_word[curw+2], irc_word[curw+6], irc_word[curw+7], irc_word[curw+3], irc_word[curw+4], irc_word[lastw]);
	}
	
if (!strcmp(irc_word[curw],"366") || !strcmp(irc_word[curw],"315")
 /*|| !strcmp(irc_word[curw],"369")*/) {
	text[0]='\0';
	}
	
if (!strcmp(irc_word[curw],"PRIVMSG")) {
	if (irc_word[curw+1][0]!='#' && irc_word[curw+1][0]!='&') {	
		sprintf(text,"~OL~FK[~FM%s~RS~FM(<-msg)~OL~FK]~RS~FW %s\n", rnick, irc_word[lastw]);
		}
	else sprintf(text,"~OL~FB<~RS~FW%s:~FT%s~OL~FB>~RS~FW %s\n", irc_word[curw+1], rnick, irc_word[lastw]);
	}

if (!strncmp(irc_word[lastw]+1,"ACTION",6)) {
	if (irc_word[lastw][0]=='\001') {
		sprintf(text,"%s:~OL%s~RS %s\n", irc_word[curw+1], rnick, irc_word[lastw]+8);
		}
	}

if (!strncmp(irc_word[lastw]+1,"PING",4)) {
	if (irc_word[lastw][0]=='\001') {
		sprintf(text,"~OL~FB=~FW*~FB=~OL~FG %s ~RS~FG[%s] requested PING.\n", rnick, rident);
		record_irc(user,text);
		write_user(user,text);
		sprintf(text2,"#PING %s\n", irc_word[lastw]+6);
		text2[0]='\001';
		sprintf(text,"NOTICE %s :%s\n", rnick, text2);		
		write_irc(user,text);
		return;
		}
	}

if (!strcmp(irc_word[curw],"INVITE")) {
	sprintf(text,"~OL~FB=~FW*~FB=~FW %s~RS invites you to~OL %s~RS\n", rnick, irc_word[lastw]);
	}

if (!strncmp(irc_word[curw],"31",2) || !strcmp(irc_word[curw],"301") || !strcmp(irc_word[curw],"369")) {  /* /WHOIS */
	if (irc_word[curw][2]=='1') sprintf(text,"~OL~FB-> ~FW%-9s~RS~FW  (%s@%s)\n~OL~FB-> ~RS~FWIRCname  : %s\n", irc_word[curw+2], irc_word[curw+3], irc_word[curw+4], irc_word[lastw]);
	if (irc_word[curw][2]=='9') sprintf(text,"~OL~FB-> ~RS~FWChannels : %s\n", irc_word[lastw]);
	if (irc_word[curw][2]=='2') sprintf(text,"~OL~FB-> ~RS~FWServer   : ~OL%s ~RS~FW(%s)\n", irc_word[curw+3], irc_word[lastw]);
	if (irc_word[curw][1]=='0') sprintf(text,"~OL~FB-> ~RS~FWAway     : %s\n", irc_word[lastw]);
	if (irc_word[curw][2]=='3') sprintf(text,"~OL~FB-> ~RS~FWOperator : %s is an IRC operator.\n", irc_word[curw+2]);
	if (irc_word[curw][2]=='4') sprintf(text,"~OL~FB-> ~FW%s ~RS~FWwas (%s@%s)\n~OL~FB-> ~RS~FWIRCname  : %s\n", irc_word[curw+2], irc_word[curw+3], irc_word[curw+4], irc_word[lastw]);
	if (irc_word[curw][2]=='8' || irc_word[curw][1]=='6') sprintf(text,"~OL~FB-> =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	if (irc_word[curw][2]=='7') {
		int since_int, idle;
		if (irc_word[curw+4][0]) since_int=atoi(irc_word[curw+4]);
			else since_int=0;
		idle=atoi(irc_word[curw+3]);
		if (since_int) sprintf(text,"~OL~FB-> ~RS~FWIdle     : ~OL%d~RS day ~OL%d~RS hr, ~OL%d~RS min (login: %s)\n", idle/86400, (idle%86400)/3600, (idle%3600)/60, zobraz_datum((time_t *)&since_int,1));
			else sprintf(text,"~OL~FB-> ~RS~FWIdle     : ~OL%d~RS day ~OL%d~RS hr, ~OL%d~RS min\n", idle/86400, (idle%86400)/3600, (idle%3600)/60);
		}
	}

if (!strcmp(irc_word[curw],"KICK")) {
	if (irc_word[lastw][0]) sprintf(text,"~OL~FB=~FW*~FB= ~OL~FW%s~RS was kicked off %s by ~FT%s ~FR(~OL~FR%s~RS~FR)\n", irc_word[curw+2], irc_word[curw+1], rnick, irc_word[lastw]);
		else sprintf(text,"~OL~FB=~FW*~FB= ~OL~FW%s~RS was kicked off %s by ~FT%s~RS\n", irc_word[curw+2], irc_word[curw+1], rnick);
	if (!strcmp(irc_word[curw+2], user->irc_nick)) {
		for (i=0; i<10; i++) if (!strcmp(user->channels[i], irc_word[curw+1])) {
			user->channels[i][0]='\0';
			break;
			}
		if (!strcmp(user->irc_chan, irc_word[curw+1])) {
			user->irc_chan[0]='\0';
			for (i=0; i<10; i++) if (user->channels[i][0]!='\0') {
				strcpy(user->irc_chan, user->channels[i]);
			        sprintf(text2,"~OL~FB=~FW*~FB= [~FY%s~FB]~RS~FW\n", user->irc_chan);
				break;
				}
			}
		if (user->statline==CHARMODE) show_statline(user);
		}

	}

/* topik */
if (!strcmp(irc_word[curw],"331")) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWNo topic is set.\n");
if (!strcmp(irc_word[curw],"332")) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWTopic for ~FT%s~FW:~OL %s~RS\n", irc_word[curw+2], irc_word[lastw]);
if (!strcmp(irc_word[curw],"333")) {
	int since_int;
	since_int=atoi(irc_word[curw+4]);
	sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWTopic set by ~FT%s~FW (~FT%s~FW)\n", irc_word[curw+3], zobraz_datum((time_t *)&since_int, 1));
	}

/* odchod */
if (!strcmp(irc_word[curw],"PART")) {
	if (irc_word[curw+2][0]>32) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FW%s has left channel %s [~OL%s~RS]\n", rnick, irc_word[curw+1], irc_word[lastw]);
		else sprintf(text,"~OL~FB=~FW*~FB= ~RS~FW%s has left channel %s\n", rnick, irc_word[curw+1]);
	if (!strcmp(rnick, user->irc_nick)) {
		for (i=0; i<10; i++) if (!strcmp(user->channels[i], irc_word[curw+1])) {
			user->channels[i][0]='\0';
			break;
			}
		if (!strcmp(user->irc_chan, irc_word[curw+1])) {
			user->irc_chan[0]='\0';
			for (i=0; i<10; i++) if (user->channels[i][0]!='\0') {
				strcpy(user->irc_chan, user->channels[i]);
				sprintf(text2,"~OL~FB=~FW*~FB= [~FY%s~FB]~RS~FW\n", user->irc_chan);
				break;
				}
			}
		if (user->statline==CHARMODE) show_statline(user);
		}
	}

/* modes */
if (!strcmp(irc_word[curw],"MODE")) {
	if (!irc_word[curw+3][0]) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWmode ~FT%s ~FW[~OL%s~RS] by ~OL%s~RS\n", irc_word[curw+1], irc_word[curw+2], rnick);
		else if (rnick[0]!='\0') sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWmode ~FT%s ~FW[~OL%s %s~RS] by ~OL%s~RS\n", irc_word[curw+1], irc_word[curw+2], irc_word[curw+3], rnick);
			else sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWmode change ~FW[~OL%s~RS] for user ~FT%s\n", irc_word[curw+2], irc_word[curw+1]);
	}

/* quit */
if (!strcmp(irc_word[curw],"QUIT")) {
	if (irc_word[curw+1][0]>32) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWSignoff: ~OL%s~RS (%s)\n", rnick, irc_word[lastw]);
		else sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWSignoff: ~OL%s~RS\n", rnick);
	}
	
/* XYBY */
if (!strcmp(irc_word[curw],"401")) sprintf(text,"~OL%s~RS: no such nick.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"402")) sprintf(text,"~OL%s~RS: no such server.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"403")) sprintf(text,"~OL%s~RS: no such channel.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"404")) sprintf(text,"~OL%s~RS: cannot send to channel.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"405")) sprintf(text,"~OL%s~RS: you have joined too many channels.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"406")) sprintf(text,"~OL%s~RS: there was no such nickname.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"407")) sprintf(text,"~OL%s~RS: duplicate recipients.\n", irc_word[curw+2]);
/* 409 */
if (!strcmp(irc_word[curw],"411")) sprintf(text,"no recipient given.\n");
if (!strcmp(irc_word[curw],"412")) sprintf(text,"no text to send.\n");
if (!strcmp(irc_word[curw],"413")) sprintf(text,"~OL%s~RS: no toplevel domain specified.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"414")) sprintf(text,"~OL%s~RS: wildcard in toplevel domain.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"421")) sprintf(text,"~OL%s~RS: unknown command.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"422")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW MOTD file is missing.\n");
if (!strcmp(irc_word[curw],"423")) sprintf(text,"~OL%s~RS: no administrative info available.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"423")) sprintf(text,"file error.\n");
if (!strcmp(irc_word[curw],"431")) sprintf(text,"no nickname given.\n");
if (!strcmp(irc_word[curw],"432")) sprintf(text,"~OL%s~RS: invalid nickname.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"433")) sprintf(text,"~OL%s~RS: nick already in use.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"436")) sprintf(text,"~OL%s~RS: nickname collision KILL.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"438")) sprintf(text,"~OL%s~RS: %s\n", irc_word[curw+2], irc_word[lastw]);
if (!strcmp(irc_word[curw],"441")) sprintf(text,"~OL%s/%s~RS: they aren't on that channel.\n", irc_word[curw+3], irc_word[curw+2]);
if (!strcmp(irc_word[curw],"442")) sprintf(text,"~OL%s~RS: you aren't on that channel.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"443")) sprintf(text,"~OL%s/%s~RS: is already on channel.\n", irc_word[curw+3], irc_word[curw+2]);	
if (!strcmp(irc_word[curw],"444")) sprintf(text,"~OL%s~RS: user not logged in.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"445")) sprintf(text,"SUMMON has been disabled.\n");
if (!strcmp(irc_word[curw],"446")) sprintf(text,"USERS has been disabled.\n");
if (!strcmp(irc_word[curw],"451")) {
	sprintf(text,"you have not registered.\n");
	sprintf(text2,"Pouzi: /nick <novy nick>\n");
	}

if (!strcmp(irc_word[curw],"461")) sprintf(text,"~OL%s~RS: not enough parameters.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"462")) sprintf(text,"you may not reregister.\n");
if (!strcmp(irc_word[curw],"463")) sprintf(text,"your host isn't among the privileged.\n");
if (!strcmp(irc_word[curw],"464")) sprintf(text,"password incorrect.\n");
if (!strcmp(irc_word[curw],"465")) sprintf(text,"you are banned from this server.\n");
if (!strcmp(irc_word[curw],"467")) sprintf(text,"~OL%s~RS: channel key already set.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"471")) sprintf(text,"~OL%s~RS: cannot join channel (limited channel).\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"472")) sprintf(text,"~OL%s~RS: is unknown mode char to me.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"473")) sprintf(text,"~OL%s~RS: cannot join channel (invite only)\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"474")) sprintf(text,"~OL%s~RS: cannot join channel (banned from channel!)\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"475")) sprintf(text,"~OL%s~RS: cannot join channel (key required!)\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"481")) sprintf(text,"Permission Denied- You're not an IRC operator.\n");
if (!strcmp(irc_word[curw],"482")) sprintf(text,"~OL%s~RS: you're not channel operator.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"483")) sprintf(text,"you can't kill a server.\n");
if (!strcmp(irc_word[curw],"491")) sprintf(text,"No O-lines for your host.\n");
if (!strcmp(irc_word[curw],"501")) sprintf(text,"Unknown MODE flag.\n");
if (!strcmp(irc_word[curw],"502")) sprintf(text,"Cant change mode for other users.\n");

/* 300, 302 niesu */
/* mame: 301 */
if (!strcmp(irc_word[curw],"303")) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWCurrently online: %s\n", irc_word[lastw]);
if (!strcmp(irc_word[curw],"305")) sprintf(text,"You're no longer marked as being away.\n");
if (!strcmp(irc_word[curw],"306")) sprintf(text,"You have been marked as being away.\n");

/* 311,312,313,317,318,319,314,369 */

if (!strcmp(irc_word[curw],"321")) sprintf(text,"~OL~FWChannel              #Users Topic\n");
if (!strcmp(irc_word[curw],"322")) sprintf(text,"~OL~FY%-20s   ~RS~FT%-3s   %s\n", irc_word[curw+2], irc_word[curw+3], irc_word[lastw]);
if (!strcmp(irc_word[curw],"323")) sprintf(text,"~OL~FW---\n");

if (!strcmp(irc_word[curw],"334")) sprintf(text,"~FB[~OL~FY*~RS~FB] ~FT%s\n", irc_word[lastw]);

/* 331, 332 */

if (!strcmp(irc_word[curw],"324")) sprintf(text,"%-20s %-5s %s\n", irc_word[curw+2], irc_word[curw+3], irc_word[curw+4]);
if (!strcmp(irc_word[curw],"341")) sprintf(text,"~FTInviting ~OL%s~RS~FT to %s.\n", irc_word[curw+2], irc_word[curw+3]);
if (!strcmp(irc_word[curw],"342")) sprintf(text,"~OL%s~RS: summoning user to IRC.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"351")) sprintf(text,"~OL%s~RS: %s (~FT%s~FW)\n", irc_word[curw+3], irc_word[curw+2], irc_word[lastw]);

/* 352, 315 */

if (!strcmp(irc_word[curw],"353")) sprintf(text,"~OL~FB=~FW*~FB= ~RS~FWUsers on ~FG[~OL~FG%s~RS~FG]~FW: ~OL~FB%s\n", irc_word[curw+3], irc_word[lastw]);

/* 366 */

/* nebude: 364, 367, 368 */

if (!strcmp(irc_word[curw],"371")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW %s\n", irc_word[lastw]);
if (!strcmp(irc_word[curw],"374")) sprintf(text,"~FB[~OL~FY*~RS~FB] =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

/* 375, 372, 376 */

if (!strcmp(irc_word[curw],"381")) sprintf(text,"You are now an IRC operator\n");

/* nebude: 382, 392, 393, 394, 395, 200, 201, 204, 205, 206, 208,
           261, 211, 212, 213, 214, 215, 216, 218, 221 */

if (!strcmp(irc_word[curw],"391")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW %s\n", irc_word[lastw]);
if (!strcmp(irc_word[curw],"252")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW ~OL%s~RS operator(s) online.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"253")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW ~OL%s~RS unknown connection(s).\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"254")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW ~OL%s~RS channels formed.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"254")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW ~OL%s~RS channels formed.\n", irc_word[curw+2]);
if (!strcmp(irc_word[curw],"256")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW ~OL%s~RS Administrative info\n", irc_word[curw+2]);

if (!strcmp(irc_word[curw],"265")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW %s\n", irc_word[lastw]);
if (!strcmp(irc_word[curw],"266")) sprintf(text,"~FB[~OL~FY*~RS~FB]~FW %s\n", irc_word[lastw]);

record_irc(user,text);
if (!user->ignall) write_user(user,text);
if (text2[0]!='\0') {
	record_irc(user,text2);
	if (!user->ignall) write_user(user,text2);
	}
}

void write_irc(UR_OBJECT user, char *text)
{
write(user->ircsocknum, text, strlen(text));
return;
}


int irc_wordfind(inpstr)
char *inpstr;
{
int wn,wpos,ende;

wn=0; wpos=0; ende=0;
do {
      while(*inpstr<33) if (!*inpstr++) return wn;
      while((*inpstr>32 && wpos<IRC_WORD_LEN-1) || (wpos<IRC_WORD_LEN-1 && *inpstr>0 && *inpstr!='\n' && *inpstr!='\r' && ende)) {
      	    if (*inpstr==':' && !wpos && wn && !ende) {
      	    	ende=1;
      	    	(void)(*inpstr++);
      	    	}
            irc_word[wn][wpos]=*inpstr++;  wpos++;
            }
      irc_word[wn][wpos]='\0';
      wn++;  wpos=0;
      } while (wn<MAX_IRC_WORDS);
return wn-1;
}

void clear_irc_words()
{
int w;
for(w=0;w<MAX_IRC_WORDS;++w) irc_word[w][0]='\0';
}
