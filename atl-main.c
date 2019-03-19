/*) Atlantis Talker (----------------------------------------------------------
   ____   ____                  .__           ______
   \   \ /   /__________________|__|____     |  ____|
    \   Y   // __ \_  __ \___   /  \__  \    |___  \      \\//
     \     /\  ___/|  | \//    /|  |/ __ \_   ___>  | _  ==**==
      \___/  \___  >__|  /_____ \__(____  /  <_____/ |_|  //\\

----------------------------------------------------------) hadiq relase (-----
                                         (c) 1997-2000 by Atlantis Talker group
*/

#define ATLANTIS "5.40 Videopoll Release" /* nech je to po nohe.. (V) */
/*
--==< To Do List >==--
    zacyklovaci bug - syslog
    parser makier .. $x
Bru:
    no-notify notification ked usera niekto fajtne + ked sa prihlasi do 5 min. po fajte
    spell roomlink
    spell zwjera
+--------------------------------------------------------+
*/

#include "atl-head.h" /* Aktulany header!! - ako sa to skratilo :) */
#include "atl-list.h" /* Niektore tabulkove konstanty */
#include "atl-mydb.h" /* yeah! mySQL.. */
#include "atl-xml.h"
#include "atl-libs.h"

void logout_user(UR_OBJECT user, char *reason)
{
char txt[200];

if (user->vis) {
        if (reason!=NULL) { 
          strncpy(user->logoutmsg,reason,59);
          user->logoutmsg[59]='\0';
         }
        else strcpy(user->logoutmsg,"-");
	sprintf(txt,"~OL~FBLOGOUT: %s [%s~FB]~RS\n", user->name, reason);
	if (!user->level) wrtype=WR_NEWBIE;
	writesys(0,1,txt,user);
	wrtype=0;
	}
disconnect_user(user,3, reason);
}

void poprehadzuj_prikazy(UR_OBJECT user)
{
FILE *fp;
int i;
char comm[20];
int level;

if ((fp=ropen(COM_LEVEL,"r"))==NULL) {
	sprintf(text,"Subor '%s' neexistuje.\n",COM_LEVEL);
	if (user==NULL) {
		printf("%s",text);
	} else {
		write_user(user,text);
	}
	return;
}
fscanf(fp,"%s %d", comm, &level);
while (!feof(fp)) {
	sprintf(text,"Prikaz: %-10s Level: %d\n", comm, level);
	if (user==NULL) printf("%s",text);
		else write_user(user,text);
	i=0;
	while (command[i][0]!='*') {
		if (!strcmp(command[i],comm)) {
			com_level[i]=level;
			break;
			}
		i++;
		}
	fscanf(fp,"%s %d", comm, &level);
	}
fclose(fp);
}

void vwrite_user(UR_OBJECT user, char *str, ...)
{
va_list args;
char vtext[ARR_SIZE*2];

vtext[0]='\0';
va_start(args,str);
vsprintf(vtext,str,args);
va_end(args);
write_user(user,vtext);
}

int charsavail(int fd)
{
  fd_set tmp;
  struct timeval tv;
  int ret;
  FD_ZERO(&tmp);
  FD_SET(fd,&tmp);
  tv.tv_sec=0;
  tv.tv_usec=10;
  ret=select(fd+1,&tmp,NULL,NULL,&tv);
  if (ret<0) return -1;
  return ((int)FD_ISSET(fd,&tmp));
}

void setpp(UR_OBJECT user, int amount)
{
if (amount<0) user->pp=0;
if (amount>MAXPP) user->pp=MAXPP;
else user->pp=amount;
if (user->statline==CHARMODE) show_statline(user);
return;
}

void view_history(user)
UR_OBJECT user;
{
UR_OBJECT u, u1;
int i,line/* ,cnt */;
char filename[80];
FILE *fp;

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
  fclose(fp);
  deltempfile(filename);
 }
if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
  write_user(user,"Chyba: Nastal problem so zapisovanim docasneho suboru.\n");
  return;
 }

u1=NULL;
if (word_count>1) u1=get_user(word[1]);
for (u=user_first;u!=NULL;u=u->next) {
	if (u1!=NULL && u!=u1) continue;
        if (u1==NULL && u!=user && u->level==GOD) continue;
        if (u1!=NULL && u!=user && u->level==GOD) {
          fputs("Operation not permited.\n",fp);
          continue;
         }
	fprintf(fp,"~BB~OL~FY %s ~RS~FW~BK\n", u->name);
	/* cnt=0; */
	for(i=0;i<HISTORY_LINES;i++) {      
	      line=(u->histpos+i)%HISTORY_LINES;
	      if (u->history[line][0]) {
		    fputs(u->history[line],fp);
		    fputs("\n",fp);
		    }
	      }
	fputs("\n",fp);
	}		
fclose(fp);

switch(more(user,user->socket,filename)) {
	      case 0: write_user(user,"Chyba pri citani history!\n");  break;
              case 1: user->misc_op=2;
	     }	
}

void resc_save()
{
FILE *fp;
int i;

if ((fp=ropen(RESC_CACHE,"w"))==NULL) return; /*APPROVED*/
for (i=0; i<rescn; i++) {
	fprintf(fp, "%s %s\n", rescip[i], rescho[i]);
	}
fclose(fp);

sprintf(text,"RESOLVER_CACHE: Ukladam %d prednahratych adries.\n", i+1);
write_syslog(text,1);
}

void resc_load()
{
FILE *fp;

if ((fp=ropen(RESC_CACHE,"r"))==NULL) return; /*APPROVED*/
fscanf(fp,"%s %s", rescip[rescn], rescho[rescn]);
while (!feof(fp)) {
	rescn++;
	if (rescn>(RESCNUM-1)) rescn=0;
	fscanf(fp,"%s %s", rescip[rescn], rescho[rescn]);	
	}
fclose(fp);
}


char *sstrncpy(char *dst, char *src, long len) {
char *ptr;

ptr=dst; /* koli return value */
while (*src && len) { *ptr=*src; src++; ptr++; len--; }
*ptr=0;
return dst;
}

char nxtznak(char *string) 
{
char curznak;
char *start;

while (isdigit(*string))
 {
 start=string;
 while (*string)
  {
  *string=*(string+1);
  string++;
  }
 string=start;
 }
curznak=*string;
while (*string) 
 {
 *string=*(string+1);
 string++;
 }
return curznak; 
}

char *striend(char *string, char *from, char *to)
{
char tmp[USER_NAME_LEN+10];
static char meno[USER_NAME_LEN+10];

strcpy(meno,string);
strcpy(tmp,string);
strtolower(tmp);
if (strlen(from) >= strlen(meno)) return meno;
if (!strcmp(&tmp[strlen(tmp)-strlen(from)],from))
    strcpy(&meno[strlen(tmp)-strlen(from)],to);
return meno;
}

int lastchar(char *string, char *last)
{
char tmp[USER_NAME_LEN+2];

sstrncpy(tmp, string, USER_NAME_LEN);
strtolower(tmp);
if (strlen(last) >= strlen(tmp)) return 0;
if (!strcmp(&tmp[strlen(tmp)-strlen(last)],last))
    return 1;
return 0;
}

/* FINGER */
void finger(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char filename[80];

if (word_count<2 || strlen(inpstr)<3) {
	write_user(user,"Pouzi: .finger <uzivatel@server>\n");
	return;
	}
if (strpbrk(inpstr,"<->%^&*()$#!;|")) {       /* Make sure they can't redirect */
	write_user(user,"Nespravna adresa!\n");
	return;
	}
if (strlen(inpstr)>60) {
	write_user(user,"Prilis dlha adresa!\n");
	return;
	}
	
sprintf(filename,"%s/%s.F",MAILSPOOL,user->name);
switch(double_fork()) {
	case -1: unlink(filename);
		 return;            /* :( */
	case 0 : sprintf(text,"finger %s > %s",inpstr,filename);
		 system(text);
		 showfile(user,filename);
		 unlink(filename);
		 _exit(1);
	}
}		 		 

int user_banned(char *name)
{
FILE *fp;
char line[82],filename[80];

sprintf(filename,"%s",NICKBLOCK);
if (!(fp=ropen(filename,"r"))) return 0;  /*APPROVED*/
fscanf(fp,"%s",line);
while(!feof(fp)) {
	if (line[0]!='#' && line[0]) {
		if (!strcmp(line,name)) {  fclose(fp);  return 1;  }
		}
	fscanf(fp,"%s",line);
	}
fclose(fp);
return 0;
}


/* ICQ page ... */
void icqpage(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
FILE *fp;
char filename[81];
int icq_num=0;

if (word_count<3) {
	write_user(user,"Pouzi: .icq <ICQ cislo> <sprava>\n");
	return;
	}
	
sprintf(filename,"%s/%s.icq",MAILSPOOL,user->name);
icq_num=atoi(word[1]);

if (icq_num<2) {
	write_user(user,"Nespravne ICQ cislo!\n");
	return;
	}
	
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	sprintf(text,"Chyba: Nemozem otvorit docasny subor ...\n");
	write_user(user,text);
	return;
	}
	
inpstr=remove_first(inpstr);	
fprintf(fp,"Sprava od: %s@%s:\n",user->name,TALKER_TELNET_HOST);
fprintf(fp,"%s\n",inpstr);
fprintf(fp,"--=[ Atlantis talker (telnet %s %d, %s ]=--\n", TALKER_TELNET_HOST,port[0],TALKER_WEB_SITE);
fclose(fp);
send_icq_page(user,icq_num,filename);
return;
}

void send_icq_page(user,icq_num,fname)
UR_OBJECT user;
int icq_num;
char *fname;
{
switch(double_fork()) {
	case -1: return;      /* zlyhanie vidlicky */
	case 0 : sprintf(text,"~FWPosielam ICQ spravu na cislo ~OL#%d~RS ...\n",icq_num);
		 write_user(user,text);
		 write_user(user,"Pokial je adresat online, dostane spravu o malu chvilku. Inak spravu dostane\npri najblizsom prihlaseni.\n");
		 sprintf(text,"%s %s ICQ-page na cislo %d\n",user->name,pohl(user,"poslal","poslala"),icq_num);
		 write_syslog(text,1);
		 sprintf(text,"mail -s \"ICQ page from %s\" %d@pager.mirabilis.com < %s",user->name,icq_num,fname);
		 system(text);
		 unlink(fname);
		 _exit(1);
		 return;
	 }
}

void idletime(user)
UR_OBJECT user;
{
UR_OBJECT u;
int idle,mins,stat,temp;
char attn[3][7]={"~FW","~FY","~OL~FR"};
char attb[3][7]={"~BW","~BY","~BR"};
char attn2[3][2]={"-","/","!"};

	write_user(user,"~OL~FB======================== ~FWIdle vzhladom na cas pripojenia ~FB======================\n\n");
	write_user(user,"                   ~RS~FTUzivatel      je idle    z celkoveho casu\n");
	write_user(user,"                   ~RS~FB^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        for (u=user_first;u!=NULL;u=u->next) {
                if (u->login || u->type!=USER_TYPE) continue;
                if (user->level<u->level && !u->vis) continue;
                mins=(int)(time(0) - u->last_login)/60;
                if (mins<(user_idle_time/60)) continue;
                temp=(int)(time(0) - u->last_input)/60;
                idle = u->idle + temp;
                stat=0;
                if (mins) temp=(int)(idle*100)/mins;
                	else temp=0;
                if ((mins/2)<idle) stat=1;  /* Idled 50% of total time on. */
                if (((mins/4)*3)<idle) stat=2;  /* Idled 75% of total time on. */
                if (idle<(user_idle_time/60)) stat=0;
                sprintf(text,"                   ~RS%s%-12s  %3d min.   %4d min.  (%3d%%)  ~RS%s%s%s~BK~RS\n",attn[stat],u->name,idle,mins, temp, attb[stat], attn2[stat], attn2[stat]);
		write_user(user,text);
                }       
return;
}

 
/*** This function calls all the setup routines and also contains the
      main program loop ***/
int main(int argc,char *argv[])
{

if (argc>1) {
	printf("Argumenty boli ignorovane:\n");
	while (argc>1) {
		argc--;
		printf("%s\n",argv[argc]);
	}
}

/* fd_set readmask; */
unsigned int i,len,s;
/* int y; */

char inpstr[ARR_SIZE+1];
char rinpstr[ARR_SIZE*5]; /* pre remote vstup */
/* char pstr[ARR_SIZE+5]; */
/* char *remove_first(); */
UR_OBJECT user,next;

sstrncpy(progname,argv[0],39);

/* Startup */

printf("\n                              --==[ \033[1mSTARTUJE\033[0m ]==--\n");
printf("                    \033[1m_____ ____ __   _____ __ __ ____ __ ____\033[0m\n");
printf("                    \033[1m||_||  ||  ||   ||_|| ||\\||  ||  || ||__\033[0m\n");
printf("             Talker \033[1m|| ||  ||  ||__ || || || ||  ||  || __||\033[0m Talker\n");
printf("             ~~~~~~ -->--==( %-22.22s )==--<-- ~~~~~~\n\n", ATLANTIS);

if (!init_database()) {
  printf("Chyba: problem s pripojenim databazy.\n");
  boot_exit(1);
 }

init_globals();
set_date_time();


daily_num=tmday;
every_min_num=tmin;
burkamin=tmin;

sprintf(text,"\n*** SERVER BOOTING, Atlantis version %s ***\n",ATLANTIS);
write_syslog(text,1);
init_signals();
init_remote_connections(NULL); /* REM0TE */
force_listen=0;
load_and_parse_config();
load_and_parse_predmets(NULL);
/* tu bolo init_sockets(); */


/* vynulovanie rev-bufferov */
revshoutline=0;
revwizshoutline=0;
revgossipline=0;
revquestline=0;
revbcastline=0;
revsosline=0;
lastlogline=0;
max_users_was=0;
all_logins=0;

pocet_vtipov=countjoke();
newusers=0;
doplnovanie_predmetov=1;
ryxlost_doplnovania=0;
predmety_time=0;
zistuj_identitu=1;             /*DOCASNE*/ /* docasne na 3 roky :) (V) */
ship_timer=1; /* ()STROV */
resolve_ip=1; /* ci ma byt zapnuty revolver! */
strcpy(motd2_file,"motd2"); /*defaultny motd2*/
strcpy(flyer.pilot,"*");

/* Inicializacia guest modu */
guest.on=0;
guest.getuser=0;
guest.moderator=NULL;
guest.user=NULL;
guest.talk=NULL;
guest.name[0]='\0';

/*if (logcommands)*/ log_commands("SERVER BOOTING","",1);

quest.queston=0;
quest.lastquest=0;
quest.lq_time[0]='\0';
quest.lq_winner[0]='\0';
quest.lq_leader[0]='\0';


rescn=0;
for (i=0; i<RESCNUM; i++) {
	rescip[i][0]='\0';
	rescho[i][0]='\0';
	}
resc_resolved=0;
resc_cached=0;
resc_load();

testroom=get_room("modra_laguna",NULL);
/* D00M boot init. */
if (doom_init()) {
        printf("Chyba v inicializacii DOOMu!\n");
        doom_status=0;
        }
 else {
       printf("DOOM bol uspesne inicializovany.\n");
       doom_status=1;
       doom_players=0;
       }
        
lab_room.dostupny=0;

if (lab_load()) {
	printf("Chyba v inicializacii labyrintu!\n");
	lab_room.dostupny=0;
	}
 else {	
 	printf("Labyrint bol uspesne inicializovany.\n");
 	lab_room.dostupny=1;
 	}

amfiteater(NULL,0);    /* Inicializacia amfiteatru!!!!(R) */
printf("Prehadzujem prikazy do levelov podla %s ...\n", COM_LEVEL);
poprehadzuj_prikazy(NULL);      /* zmena levelov niektoryx prikazov */


init_sockets();
check_messages_night(1);

/* Run in background automatically. */
switch(fork()) {
      case -1: boot_exit(11);  /* fork failure */
      case  0: break; /* child continues */
      default: sleep(1); exit(0);  /* parent dies */
      }

printf("\n                        --==) \033[1mUspesne, PID: %5d\033[0m (==--\n",getpid());
/* ten riadok nad tymto je zakerna bestia... a nemal by tu byt :( namiesto 
   toho by sa mali uzavriet stdin/out/err, ale... let it be...*/
/* takto ? */
/* fclose(stdin); */
/* fclose(stdout); */
   
sprintf(text,"*** Uspesne nastartovane pod PID %d %s ***\n\n",getpid(),long_date(1));
write_syslog(text,0);
crash_smsnotice();

hb_can=1;
reset_alarm();
hb_sec=0;

setlocale(LC_NUMERIC,"sk_SK.UTF-8");

/**** Main program loop. *****/

while(1) {
      /* set up mask then wait */
      /* setup_readmask(&readmask); */
      /* if (select(FD_SETSIZE,&readmask,0,0,0)==-1) continue; */
      
      set_date_time();
      if (tsec!=hb_sec) {
      	if (tsec%2==0) do_events(); 
	brutalis_wars();
        do_predmety(0);
        do_userhands();
        handle_webinput();
        check_death();
        if (tsec%30==0) kick_webusers();
      	hb_sec=tsec;
       }
      lib_run_heartbeat();

      usleep(0);
      if (poll(fds,size_fds+1,1000)<=0) continue;
       
      /* check for connection to listen sockets */
      for(i=0;i<2;++i) { /*LYNUZ*/
        /* if (FD_ISSET(listen_sock[i],&readmask)) */
        if (fds[i].revents & (POLLIN | POLLPRI)) accept_connection(listen_sock[i],i);
      }

    /* REM0TE */    
    for (user=user_first;user!=NULL;user=user->next)
      {
	if (user->type!=USER_TYPE || user->login || user->socket>1000) continue;	
	for (i=0;i<MAX_CONNECTIONS;i++)
	  {
	    if (user->remote_socket[i]==0) continue;
	    switch (fds[user->remote_fds[i]].revents)
	      {
	      case POLLHUP:
	      	user->hangupz=100000; user->hangups=100000;
		if (user->remote_socket[i]==user->ircsocknum) sprintf(text,"~FT[IRC server ~OL%s~RS~FT uzavrel spojenie.]\n", get_ircserv_name(user->irc_serv));
			else sprintf(text,"~FT[~OL%s %d~RS~FT uzavrel spojenie.]\n",user->remote[i]->name,user->remote[i]->port);
		write_user(user,text);
		if (user->actual_remote_socket==user->remote_socket[i]) user->actual_remote_socket=0;
		user->remote_login=0;
		user->remote_login_socket=0;
		remote_close(user,i);
		if (user->statline==CHARMODE) text_statline(user,1);
		break;
	      case POLLERR: pollerror++; break;
	      case POLLNVAL:write_syslog("Poll(): Invalid request\n",1); break;
	      case POLLIN:
	      case POLLPRI:
	      	if (user->remote_socket[i]==user->ircsocknum) {
	      		s=0;
       		        inpstr[0]='\0';
       		        do { 
       		            len=read(user->remote_socket[i], inpstr+s, 1);
			    if (len==0) { /* VIDEOPOLL */
	      				user->hangupz=100000; user->hangups=100000;
				}
       		            s++;
       		            } while (len && inpstr[s-1]!='\n' && s<(sizeof(inpstr)-5));
	      		inpstr[s]='\0';
	      		spracuj_irc_vstup(user, inpstr);
	      		}
	    	else {
			len=read(user->remote_socket[i],rinpstr,sizeof(rinpstr)-100);
			parse_remote_ident(user,rinpstr,len,i);
			rinpstr[len]='\0';
	    	        if (user->statline==CHARMODE) {
	    	           if (spracuj_remote_vstup(rinpstr)) break;
	    	         }
			if (user->newline) { write2sock(user,user->socket,"\r\n",0); user->newline=0;}
			/*
			if (user->statline==CHARMODE) {
				for (y=1; y<=5; y++) {
					if (len-y==0) break;
					if (rinpstr[len-y]=='\r') rinpstr[len-y]='\0';
					if (rinpstr[len-y]=='\n') {
						rinpstr[len-y]='\0';
						user->newline=1;
						break;
					}
				}
			}
			log_commands("REM:", rinpstr,1);
			*/
			write2sock(user,user->socket,rinpstr,0);
			}
		break;
/*	      default: */
	      }
	  }
      }
                    
      /* Cycle through users. Use a while loop instead of a for because
          user structure may be destructed during loop in which case we
          may lose the user->next link. */
      /* No jo, ale ty blbej Neile 's to nedomyslel, vole vozralej!! */
      user=user_first;
      while(user!=NULL) {
      	    inyjefuc=0;
      	    wrtype=0;
      	    
            next=user->next; /* store in case user object is destructed */
            /* If remote user or clone ignore */
            if (user->type!=USER_TYPE || user->socket>1000) {  user=next;  continue; }
          /*
            if (!FD_ISSET(user->socket,&readmask)) { user=next;  continue; }
          */

       /* see if any data on socket else continue */
       switch (fds[user->fds].revents)
 	{
	    case POLLHUP: if (!user->login) { 
	    		  	sprintf(text,"~FRzrusene spojenie");
 			  	logout_user(user,text);                       
                          	}
                          else disconnect_user(user,3,NULL);
                          user=next;
                          break;
	    /* case POLLERR: write_syslog("Poll(): Error conditon\n",1); break; */
	    case POLLNVAL:write_syslog("Poll(): Invalid request\n",1); break;
	    case POLLIN:
  	    case POLLPRI:                      
            inpstr[0]='\0';
            
            len=read(user->socket,inpstr,ARR_SIZE);
			    
	    if (len==0) { /* VIDEOPOLL */
	      	user->hangupz=100000; user->hangups=100000;
		user=next;
		continue;
		}

            /* ignore telnet protocol control code replies */
            if (len && (unsigned char)inpstr[0]==255) {
                parse_telnet_chars(user,inpstr,len);
             	user=next; 
             	continue; 
             	}

	     if (len==1 && inpstr[0]=='\014' && user->statline==CHARMODE) {
 	        init_statline(user); /* CTRL + L handling */
 	        if (user->buffpos) text_statline(user,0);
 	 	user=next;
 	 	continue;
 	        }

            /* Deal with input chars. If the following if test succeeds we
               are dealing with a character mode client so call function. */
            if (inpstr[len-1]>=32 || inpstr[len-1]==0 || user->buffpos || (inpstr[len-1]==9 && user->statline==CHARMODE)) {
            	  if (user->statline==UNKNOWN) {
			boot_statline(user);
			user=next; continue;
			}

		  if (user->statline==CHARMODE && len==3 && 
		      inpstr[0]=='\033' && inpstr[1]=='[' &&
		      inpstr[2]>='A' && inpstr[2]<='D') {
			   switch (inpstr[2]) {
		           case 'A': /* sipka HORE */
		           	     if (user->filepos) { /* Editor - back */
		           	         strcpy(inpstr,"b\n");
		           	         }
		           	     else if (user->doom) { /* DOOM */
		           	         strcpy(inpstr,".doom f\n");
		           	         }
		           	     else {
		           	          if (user->level>WIZ) { sstrncpy(user->buff, user->inpstr_old, BUFSIZE-1);
		           	          	user->buffpos=strlen(user->buff);
		           	          	text_statline(user,0);
		           	          	}		           	          	
		           	          user=next; continue;		           	          	
		           	          }
		           	     break;
			   case 'B': /* sipka DOLE */
		           	     if (user->filepos) { /* Editor - fwd */
		           	         strcpy(inpstr,"\n");
		           	         }
		           	     else if (user->doom) { /* DOOM */
		           	         strcpy(inpstr,".doom b\n");
		           	         }
		           	     else { user=next; continue; } 
				     break;
			   case 'C': /* sipka VPRAVO */
		           	     if (user->filepos) { /* Editor - end */
		           	         strcpy(inpstr,"e\n");
		           	         }
		           	     else if (user->doom) { /* DOOM */
		           	         strcpy(inpstr,".doom r\n");
		           	         }
		           	     else { user=next; continue; }
				     break;
			   case 'D': /* sipka VLAVO */
		           	     if (user->filepos) { /* Editor - refr */
		           	         strcpy(inpstr,"r\n");
		           	         }
		           	     else if (user->doom) { /* DOOM */
		           	         strcpy(inpstr,".doom l\n");
		           	         }
		           	     else { user=next; continue; }
				     break;
			   }
			len=strlen(inpstr);
			}

                  if (!get_charclient_line(user,inpstr,len)) {
                  	user=next;
                  	continue; 
                  	}
                  }
            else {
		  if (user->statline==UNKNOWN) {
			write_user(user,"Stavovy riadok v line-mode nie je implementovany... ZATIAL!\n");
			/* user->statline=LINEMODE; */
			user->statline=NONEMODE;
			}
                  terminate(inpstr);
                 }

            /* GOT_LINE: */
            got_line(user,inpstr);
	    if (!destructed || inyjefuc) {
		    dump_user_data(user);
	    }
            
/*        default: */
       } /* koniec case ;-) */
            
    if (inyjefuc) user=user->next;
    else user=next;
    }
    
  } /* end while */
}

/************ MAIN LOOP FUNCTIONS ************/

void got_line(UR_OBJECT user,char *inpstr)
{
char *inpstr_w;
char histrecord[20];
char pstr[ARR_SIZE+5];

            inpstr_w=inpstr;
            while(*inpstr_w) {
            	if (*inpstr_w>=127) *inpstr_w=0;
            	inpstr_w++;
            	}
            no_prompt=0;
            com_num=-1;
            force_listen=0;
            destructed=0;
            user->buff[0]='\0';
            user->buffpos=0;
            if (*inpstr || (!(*inpstr) && user->misc_op==2) || user->afk) {
	            user->idle=user->idle + ((int)(time(0) - user->last_input)/60);
                    user->lastidle2=user->lastidle;
                    user->lastidle3=user->lastidle2;
                    user->lastidle=(int)(time(0)-user->last_input);
		    if (user->lastidle>0) user->cps++;
        	    user->last_input=time(0);
        	    }
            if (user->login) {
            	clear_words();
                login(user,inpstr);  
                return;
                }

          	
            /* If a dot on its own then execute last inpstr unless its a misc
               op or the user is on a remote site */
            if (!user->misc_op) {
		  if (!strcmp(inpstr,".") && user->inpstr_old[0] && (user->level>=KIN) && user->actual_remote_socket==0) {
                        sstrncpy(inpstr, user->inpstr_old, REVIEW_LEN);
                        sprintf(text,"%s\n",inpstr);
			write_user(user,text);
                        }
                  /* else save current one for next time */
                  else {
                        if (inpstr[0]) sstrncpy(user->inpstr_old,inpstr,REVIEW_LEN);
                        }                        
                  }

            clear_words(); /* vycisti slova ... */
			
            word_count=wordfind(inpstr);
           
            /* REM0TE */
       /*     if (user->actual_remote_socket!=0)
	    	if (exec_remote_com(user,inpstr)) {
                           return;
			  }
        */    
	  
	  if (user->actual_remote_socket==user->ircsocknum) {
	  	if (misc_ops(user,inpstr)) {
                         return;
	  		}
	  	}
          if (exec_remote_com(user,inpstr)) {
                 return;
          	}
          	
	 if (word_count) {
                user->pridavacas=1;
		if (!strncmp(user->lastcommand,inpstr,20)
                && !strncmp(user->lastcommand2,inpstr,20))
                 user->pridavacas=0;
                if (abs(user->lastidle-user->lastidle2)<2 && user->lastidle>29) user->pridavacas=-1;
                if (user->pridavacas==-1 && abs(user->lastidle2-user->lastidle3)<2 && user->lastidle2>29)
                 user->pridavacas=-3; /* hajzel je to :) */
                strcpy(user->lastcommand2,user->lastcommand);
		sstrncpy(user->lastcommand,inpstr,21);
                user->lastcommand[22]='\0';
		} 

            if (user->afk) {
                  if (user->afk==2) {
                        if (!word_count) {
                              if ((user->command_mode) || (user->prompt)) prompt(user);
                               return;
                              }
			    if (strcmp((char *)md5_crypt(expand_password(word[0]),user->name),user->pass)) {
	                              write_user(user,"Nespravne heslo.\n");
	                              prompt(user);
	                              return;
	                              }                                    
                        sprintf(text,"%s si sa naspat ku klavesnici a %s svoje konto.\n",pohl(user,"Vratil","Vratila"),pohl(user,"odomkol","odomkla"));
                        write_user(user,text);
                        echo_on(user);
                        }
                  else {
                  	sprintf(text,"%s si sa ku klavesnici.\n",pohl(user,"Vratil","Vratila"));
                  	write_user(user,text);
                        if (!word_count && user->lastidle2>599) user->pridavacas=-2;
                  	}
            if (user->newtell)
               {
                show_recent_tells(user);
                user->newtell=0;
                user->tellpos=0;
               }                  	
            show_recent_notifies(user);
                  user->afk_mesg[0]='\0';
                  if (user->vis) {
                        sprintf(text,"%s sa %s ku klavesnici.\n",user->name, pohl(user,"vratil","vratila"));
                        write_room_except(user->room,text,user);
                        }
                  if (user->afk==2) {		
                        user->afk=0;
                        prompt(user);
                        return;
                        }
                  user->afk=0;
                  }
                  
            if (!word_count) {
            	  sprintf(histrecord,"%02d:%02d -enter-", tmin, tsec);
            	  record_history(user, histrecord);            	
                  if (misc_ops(user,inpstr))  {  return;  }
                  if ((user->command_mode) || (user->prompt)) prompt(user);
                  return;
                  }
                  
            if (misc_ops(user,inpstr))  {  return; }
            com_num=-1;

       /* M@CRO - vseeeetko odtialto prec, nahradi to tento jeden
         riadok: */         
            
            /* Tu rozdeli skratku od slova ak su nalepene */
            if (strchr("!@$<>-:,`",inpstr[0]) && inpstr[1]!=' ') 
			{
			pstr[0]=inpstr[0];
			pstr[1]=' ';
			sstrncpy (pstr+2,inpstr+1, ARR_SIZE-5);
			sstrncpy (inpstr,pstr, ARR_SIZE-2); 
			}

        if (*inpstr) {
                       user->com_counter=MAX_COMMANDS;
                       parse(user,inpstr,0);
                      }
            else {
            	sprintf(histrecord,"%02d:%02d -enter-", tmin, tsec);
            	record_history(user, histrecord);
            	}
           
                      
      
        if ((!destructed) || (destructed && inyjefuc)) {
                  if (user->room!=NULL)  prompt(user);
                  }

}

void check_death()
{
UR_OBJECT user,next;

  user=user_first;
  while(user!=NULL) {
    inyjefuc=0;
    wrtype=0;
    next=user->next; /* store in case user object is destructed */
    /* If remote user or clone ignore */
    if (user->type!=USER_TYPE) {  user=next;  continue; }
    /*Ak je dead, odhlasi ho s pa3cnou hlaskou!*/
    if (user->dead) {
      if (user->dead==1) sprintf(text,"~FRbombovy utok");
      if (user->dead==2) {
        if (user->afk && user->afk_mesg[0] && strcmp(user->afk_mesg,"auto-afk"))
          sprintf(text,"~FRcas vyprsal - ~OL~FW%-.60s",user->afk_mesg);
        else sprintf(text,"~FRcas vyprsal");
       }
      if (user->dead==3) sprintf(text,"~FRhavaria letuna");
      if (user->dead==5) sprintf(text,"~FRsmrt v bazine");
      if (user->dead==6) sprintf(text,"~FRobet kamikaze");
      if (user->dead==7) sprintf(text,"~FRotraven%s",pohl(user,"y","a"));
      if (user->dead==8) sprintf(text,"~FRobet dravych zvierat");
      if (user->dead==9) sprintf(text,"~FRzasypan%s",pohl(user,"y","a"));
      if (user->dead==10) sprintf(text,"~FRpad zo skaly");
      if (user->dead==11) sprintf(text,"~FRstratene spojenie");
      if (user->dead==12) sprintf(text,"~FRroztrhan%s na kusy",pohl(user,"y","a"));
      if (user->dead==13) sprintf(text,"~FRsecurity");
      if (user->dead==666) sprintf(text,"~FRnewban");
      if (user->dead==14) {
        sprintf(text,"~OL~FYPodlah%s si smrtelnym zraneniam.\n",pohl(user,"ol","la"));
        write_user(user,text);
        if (user->deathtype>-1 && predmet[user->deathtype]!=NULL
        && predmet[user->deathtype]->error!=NULL)
          sprintf(text,"~FR%s",parse_phrase(predmet[user->deathtype]->error,user,NULL,NULL,0));
        else sprintf(text,"~FRzahynul%s v boji",pohl(user,"","a"));
       }
      if (user->dead==15) sprintf(text,"~FRobet ludozrutov");
      if (user->dead!=4) logout_user(user,text);	    	
      else disconnect_user(user,3,NULL);
     }
    user=next;
   }
}

/*** Set up readmask for select ***/
void setup_readmask(mask)
fd_set *mask;
{
UR_OBJECT user;
int i;

FD_ZERO(mask);
for(i=0;i<2;++i) FD_SET(listen_sock[i],mask); /*LYNUZ*/
/* Do users */
for (user=user_first;user!=NULL;user=user->next)
      if (user->type==USER_TYPE) FD_SET(user->socket,mask);

}

/*** Accept incoming connections on listen sockets ***/
void accept_connection(lsock,num)
int lsock,num;
{
UR_OBJECT user,create_user();
char *get_ip_address(), *get_ip_address2(), site[82];
struct sockaddr_in acc_addr;
int accept_sock;
unsigned int size;
int pocet_loginov;
char filename[200];
UR_OBJECT u;
FILE *fp;

size=sizeof(struct sockaddr_in);
accept_sock=accept(lsock,(struct sockaddr *)&acc_addr,&size);
 
sstrncpy(site,get_ip_address(acc_addr), 80);

if (site_banned(site)) {
      write2sock(NULL,accept_sock,"\r\nPrepac, z tejto adresy momentalne nie je mozne prihlasovanie.\r\n\n",0);
      close(accept_sock);
     /* sprintf(text,"Attempted login from banned site %s.\n",site);
      write_syslog(text,1); */
      return;
      }
      
/* Kontroluje ci sa neflooduje */
pocet_loginov=0;
for(u=user_first;u!=NULL;u=u->next) {
	if (u->login) if (!strcmp(u->site,site)) pocet_loginov++;
	}
		
if (pocet_loginov>=MAX_POCET_LOGINOV) {
	/*write_sock(accept_sock,"\r\nPrepac, momentalne nieje mozne sa prihlasit.\r\n\n");*/
	close(accept_sock);
	sprintf(filename,"%s/%s",DATAFILES,SITEBAN);
	if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
	      return;
	      	}
	
	fprintf(fp,"%s\n",site);
	fclose(fp);
	
	sprintf(text,"Adresa %s bola automaticky vybanovana! (flood)\n",site);
	write_syslog(text,1);
	return;
	}
     
/* ZMENA - niekto sa pokusa prihlasit na plny talker */
sprintf(filename,"select body from files where filename='motd1'");
sqlmore(NULL,accept_sock,filename); /* Posle uvodny obrazok */
if (num_of_users+num_of_logins>=max_users && !num) {
      write2sock(NULL,accept_sock,"\nPrepac, talker je momentalne plny.\n",0);
      sprintf(text,"\nMaximalny pocet naraz prihlasenych pouzivatelov moze byt len %d. Skus neskor!\n\n",max_users);
      write2sock(NULL,accept_sock,text,0);
      close(accept_sock);
      return;
      }
if ((user=create_user())==NULL) {
      sprintf(text,"\r\n %s: Nepodarilo sa vytvorit pripojenie.\r\n\n",syserror);
      write2sock(NULL,accept_sock,text,0);
      close(accept_sock);
      return;
      }
      
/* REM0TE */
  if ((user->fds=ADD_FDS(accept_sock,POLLIN|POLLPRI))==0) 
    { 
      write_syslog ("Preplnene pole struktur fds, zvys pocet MAX_POLL\n",1); 
      close(accept_sock);
      destruct_user(user);
      return;
    }
           
user->socket=accept_sock;
user->login=3;
user->last_input=time(0);
if (!num) user->port=port[0];
else {
      user->port=port[1];
      }
sstrncpy(user->site,site,80);
user->site_port=(int)ntohs(acc_addr.sin_port);
echo_on(user);
sprintf(text,"                  %08d navstevnikov od %s\n",counter_db(0),DATESINCE); 
write_user(user,text);
if (user->port==port[1]) {
	write_user(user,"Wizport login: ");
	user->vis=0;
	}
	else write_user(user,"Tvoja prezyvka: ");
num_of_logins++;
user->zaradeny=1;
strcpy(user->ipcka, ipcka_glob);
}

/*** Get net address of accepted connection ***/
char *get_ip_address(acc_addr)
struct sockaddr_in acc_addr;
{
static char site[52];
char ipcka[20];
struct hostent *host;
int i;
pid_t pid;  /* pid-ko dietatka - ak ho bude treba zawrazdit...     */
int status; /* status dietatka - mozno pre buducnost, zatial nanic */
int loop;   /* test zamrznutia, teda skratka sluckovac...          */
int fd[2];

sstrncpy(site,(char *)inet_ntoa(acc_addr.sin_addr),50); /* get number addr */
strncpy(ipcka, site, 16);
strncpy(ipcka_glob, site, 16);

/*if (resolve_ip && logcommands) {
	sprintf(text,"ToReslv: %s",site);
	log_commands(text,"",0);
	}
  uz to na tom nepada, zbytocne to zralo diskspace (V)
*/
/* prehlada CACHE */
for (i=0; i<RESCNUM; i++) {
	if (rescip[i][0]!='\0') if (!strcmp(site, rescip[i]))
		{
		strncpy(site, rescho[i], 50);
/*		if (resolve_ip && logcommands) {
			sprintf(text,"OKCReslv: %s",site);
			log_commands(text,"",0);
		}
*/		resc_cached++;
		return site;
		}
	}
	
if (resolve_ip) 
    {
     loop=NUMBER_OF_LOOPS; /* nastavenie sluckovaca */
     pipe(fd);		   /* spravi si tu ruuuru... */
     pid=fork();           /* heh, a mame tu dva krat Atlantis! */ 
     if (pid==0) {         /* deticko! */
         close(fd[0]);
         if ((host=gethostbyaddr((char *)&acc_addr.sin_addr,4,AF_INET))!=NULL)
	  	     sstrncpy(site,host->h_name,50);
	 strtolower(site);
	 write2sock(NULL,fd[1],site,0); /* posle tatickovi data cez ruru */
	 close(fd[1]);
         _exit(0);          /* deticko tutohla chcipne... */
         }
     else if (pid > 0) {   /* foter, skratka original Atlantis  */
         close(fd[1]);
         while (loop) {
           if (waitpid (pid, &status, WNOHANG) == pid) {  /* zresolvovalo? */
             bzero(site,sizeof(site));
             read(fd[0],site,sizeof(site)); /* naruruj... */
             close(fd[0]);
             /* site[sizeof(site)]='\0'; */
             break;
             }
           usleep(10);
           loop--;
           }
         if (!loop)
         	{
         	kill(pid,9);
         	close(fd[0]);
         	waitpid(pid, &status, 0);
          	}
         }
     /* ak je pid < 0 doslo ku chybe vo forkovani - takze sa neresolvuje */
    }                     
    
/*if (resolve_ip && logcommands) {
	sprintf(text,"OKReslv: %s",site);
	log_commands(text,"",0);
	}            
*/	             
/* ulozit do cache */
strncpy(rescip[rescn], ipcka, 16);
strncpy(rescho[rescn], site, 50);
rescn++;
if (rescn>(RESCNUM-1)) rescn=0;
resc_resolved++;
	
return site;
}

/*** See if users site is banned ***/
int site_banned(site)
char *site;
{
FILE *fp;
char line[82],filename[80];

sprintf(filename,"misc/%s",SITEBAN);
if (!(fp=ropen(filename,"r"))) return 0; /*APPROVED*/

fscanf(fp,"%s",line);
while(!feof(fp)) {
      if (strstr(site,line)) {  fclose(fp);   return 1;  }
      fscanf(fp,"%s",line);
      }
fclose(fp);

return 0;
}

void crash_smsnotice()
{
FILE *fp;
char tmp[101];

 if ((fp=ropen("misc/crash","r"))) {
   fgets(tmp,100,fp);
   fclose(fp);
   unlink("misc/crash");
   tmp[strlen(tmp)-1]='\0';
   sprintf(text,"ALERT!! Booting %d:%02d:%02d. %s",thour,tmin,tsec,tmp);
  }
 else sprintf(text,"ALERT!! Booting %d:%02d:%02d. Shutdown reason unknown.",thour,tmin,tsec);
 if (!disable_web) send_sms("+421903959955",text,0);
}

/*** newuser_siteban ***/
int newuser_siteban(site)
char *site;
{
FILE *fp;
char line[82],filename[80];

sprintf(filename,"misc/%s",NEWUSER_SITEBAN);
if (!(fp=ropen(filename,"r"))) return 0; /*APPROVED*/

fscanf(fp,"%s",line);
while(!feof(fp)) {
      if (strstr(site,line)) {  fclose(fp);   return 1;  }
      fscanf(fp,"%s",line);
      }
fclose(fp);

return 0;
}

/*** Put string terminate char. at first char < 32 ***/
void terminate(str)
char *str;
{
int i;
for (i=0;i<ARR_SIZE;++i)  {
  if (*(str+i)<32) {  *(str+i)=0;  return;  }
 }
str[i-1]=0;
}

/*** Get words from sentence. This function prevents the words in the
     sentence from writing off the end of a word array element. This is
     difficult to do with sscanf() hence I use this function instead. ***/
unsigned int wordfind(char *inpstr)
{
unsigned int wn,wpos;

wn=0; wpos=0;
do {
      while(*inpstr<33) if (!*inpstr++) return wn;
      while(*inpstr>32 && wpos<WORD_LEN-1) {
            word[wn][wpos]=*inpstr++;  wpos++;
            }
      word[wn][wpos]='\0';
      wn++;  wpos=0;
      } while (wn<MAX_WORDS);
return wn-1;
}

/*** clear word array etc. ***/
void clear_words()
{
int w;
for(w=0;w<MAX_WORDS;++w) word[w][0]='\0';
word_count=0;
}

/************ PARSE CONFIG FILE **************/

void load_and_parse_config()
{
FILE *fp;
char line[451]; /* Should be long enough */ /* :((((((((( */
char filename[80];
int i,predm;
RM_OBJECT rm1,rm2;
 
 motd=NULL;
 sprintf(query,"select `body` from `files` where `filename`='motd1'");
 if ((result=mysql_result(query))) {
   if ((row=mysql_fetch_row(result)) && row[0]!=NULL) {
     motd=(char *) malloc ((strlen(row[0])*sizeof(char))+2);
     if (motd!=NULL) strcpy(motd,row[0]);
    }
   mysql_free_result(result);
  }
 if (motd==NULL) printf("\nNUTS: Could not load MOTD.\n");

 sprintf(query,"update `users` set `online`=0");
 mysql_kvery(query);
  
 max_id=0;
 sprintf(query,"select max(`id`) from `users`");
 if ((result=mysql_result(query))) {
   if ((row=mysql_fetch_row(result)) && row[0]!=NULL) {
     max_id=atoi(row[0])+1;
    }
   mysql_free_result(result);
  }
 if (max_id<1) {
   perror("\nBRUTAL ERROR: Could not get max userID.\n");
   boot_exit(1);
  }
 
 sprintf(query,"select `parameter`,`value` from `config`");
 if (!(result=mysql_result(query))) {
   perror("\nNUTS: Can't read from config table");
   boot_exit(1);
  }
 while ((row=mysql_fetch_row(result))) {
   if (row[0]!=NULL && row[1]!=NULL) {
     parse_init_section();
    }
  }
 mysql_free_result(result);

 
 parse_rooms_section();

/* See if required sections were present (SITES is optional) and if
   required parameters were set. */
/*if (!got_init) {
      fprintf(stderr,"\nNUTS: INIT sekcia chyba v konfiguracnom subore.\n");
      boot_exit(1);
      }
if (!got_rooms) {
      fprintf(stderr,"\nNUTS: ROOMS sekcia chyba v konfiguracnom subore.\n");
      boot_exit(1);
      }*/
if (!port[0]) {
      fprintf(stderr,"\nNUTS: V konfiguracnom subore chyba cislo portu.\n");
      boot_exit(1);
      }
if (!port[1]) {
      fprintf(stderr,"\nNUTS: V konfiguracnom subore chyba cislo Wiz-portu.\n");
      boot_exit(1);
      }
if (port[0]==port[1]) {
      fprintf(stderr,"\nNUTS: Cisla portov nemozu byt totozne.\n");
      boot_exit(1);
      }
if (room_first==NULL) {
      fprintf(stderr,"\nNUTS: V konfiguracnom subore nie su nijake miestnosti.\n");
      boot_exit(1);
      }

/* Parsing done, now check data is valid. Check room stuff first. */
for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
  for(i=0;i<MAX_LINKS;++i) {
    if (!rm1->link_label[i][0]) break;
    if (!strcmp(rm1->link_label[i],NOEXITS)) break;
    for(rm2=room_first;rm2!=NULL;rm2=rm2->next) {
      if (rm1==rm2) continue;
      if (!strcmp(rm1->link_label[i],rm2->label)) {
        rm1->link[i]=rm2;
        break;
       }
     }
    if (rm1->link[i]==NULL) {
/*      fprintf(stderr,"\nNUTS: Miestnost %s nema definovane navestie linky '%s'.\n",rm1->name,rm1->link_label[i]); */
      printf("WARNING: Miestnost %s nema definovane navestie linky '%s'.\n",rm1->name,rm1->link_label[i]);
/*      boot_exit(1); */
     }
   }
 }
printf("OK\n");
load_rooms_desc(NULL);

for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
  for (predm=0; predm<MPVM; ++predm) {
    rm1->predmet[predm]=-1;
    rm1->dur[predm]=0;
   }
 }
      
/* Defaultne rozlozenie predmetov */
/*sprintf(filename,"%s/%s",ROZLOZENIE_P);
if (!(fp=ropen(filename,"r"))) {
	printf("CHYBA: Nenasiel som subor s rozlozenim predmetov (%s)\n",filename);
	}
else {

i=0;
fscanf(fp,"%s",line);
while (!feof(fp)) {
	for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
		if (!strcmp(line,rm1->name)) {
		fscanf(fp,"%s",line);
				while (strcmp(line,"*")) {
					put_in_room(rm1,expand_predmet(line),default_dur(expand_predmet(line)));
					fscanf(fp,"%s",line);
					i=1;					
					}
				}		
			}
		if (!i) while (strcmp(line,"*")) fscanf(fp,"%s",line);
		fscanf(fp,"%s",line);
		i=0;
		}
 fclose(fp);
 }
*/
/* Io frazy miestnosti (do husteho lesa, z husteho lesa, v hustom lese) */
sprintf(filename,"%s/%s",DATAFILES,"ioroom");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
 printf("Chyba - Nenasiel som subor s io frazami miestnosti (%s)\n",filename);
}
else
 {
  fgets(line,81,fp);
  line[strlen(line)-1]='\0';
  while (!feof(fp)) {
   for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
    if (!strcmp(line,rm1->name)) { 
     fgets(line,81,fp);
     line[strlen(line)-1]='\0';
     strcpy(rm1->into,line);
     fgets(line,81,fp);
     line[strlen(line)-1]='\0';
     strcpy(rm1->from,line);
     fgets(line,81,fp);
     line[strlen(line)-1]='\0';
     strcpy(rm1->where,line);
    }
   }
   fgets(line,81,fp);
   line[strlen(line)-1]='\0';
  }
  fclose(fp);
 }
global_macros(NULL);


 i=0;
 sprintf(query,"select `value` from `config` where `parameter` like ('team%%') order by `parameter`");
 if ((result=mysql_result(query))) {
   while ((row=mysql_fetch_row(result)) && row[0]!=NULL) {
     i++;
     if (i==5) break;
     printf("%d - %s\n",i,row[0]);
     strcpy(teamname[i],row[0]);
    }
   mysql_free_result(result);
  }
}

void global_macros(UR_OBJECT user)
{
int i;

sprintf(text,"Vytvaram defaultne makra ... macros.userid=0");
if (user==NULL) printf("%s",text);
else write_user(user,text);

if (user!=NULL) free_macrolist(&(defaultmacrolist));

defaultmacrolist = NULL;
i=load_macros(&(defaultmacrolist),0);
sprintf(text,"OK (%d ks)\n",i);
if (user==NULL) printf("%s",text);
else write_user(user,text);
}

/* Load room descriptions */
void load_rooms_desc(user)
UR_OBJECT user;
{
RM_OBJECT rm1;
FILE *fp;
char c,filename[80];
int i;
char tempdesc[4000]; /* docasne ulozime sem ... */

if (user==NULL) fprintf(stderr,"Nacitavam popisky miestnosti...");
else write_user(user,"Nacitavam popisky miestnosti...");

for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
  sprintf (query,"select desc_sk,desc_en from rooms where name_sk='%s'",rm1->name);
  if (!(result=mysql_result(query))) {
    sprintf(text,"CHYBA\n- Nemozem najst popis pre miestnost %s.\n",rm1->name);
    if (user==NULL) fprintf(stderr,"%s",text);
    else write_user(user,text);
    write_syslog(text,0);
    continue;
   }
  if ((row=mysql_fetch_row(result))) {
    if (row[0]!=NULL) {
      i=strlen(row[0]);
      if (rm1->desc_sk!=NULL) free ((void *) rm1->desc_sk); /* ak tam uz nieco je, zrus to */
      if ((rm1->desc_sk=(char *) malloc ((i*sizeof(char))+1))==NULL) {
        sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre popis miestnosti!\n");
        write_syslog(text,0);
        if (user==NULL) fprintf(stderr,"%s", text);
        break;
       }
      strcpy(rm1->desc_sk,row[0]);
     }
    if (row[1]!=NULL) {
      i=strlen(row[1]);
      if (rm1->desc_en!=NULL) free ((void *) rm1->desc_en); /* ak tam uz nieco je, zrus to */
      if ((rm1->desc_en=(char *) malloc ((i*sizeof(char))+1))==NULL) {
      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre anglicky popis miestnosti!\n");
      	write_syslog(text,0);
      	if (user==NULL) fprintf(stderr,"%s", text);
      	break;
       }
      strcpy(rm1->desc_en,row[1]);
     }
   }
  mysql_free_result(result);
 }

/* Nacitavanie lokaci s lodickou - ()STROV ----> */
/* Najskor ostrovcek */
if (user==NULL) fprintf(stderr,"...alternativne...");
   else write_user(user,"...alternativne...");
if (alt_island_desc!=NULL) free ((void *) alt_island_desc);   
sprintf(filename,"%s/%s.S",DATAFILES,SHIPPING_ISLAND);
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
      sprintf(text,"CHYBA\n- Nemozem otvorit alt. popis pre miestnost %s.\n",SHIPPING_ISLAND);
      if (user==NULL) fprintf(stderr,"%s",text);
      else write_user(user,text);
      write_syslog(text,0);
      if ((rm1=get_room(SHIPPING_ISLAND,NULL))!=NULL) {
	      if ((alt_island_desc=(char *) malloc ((strlen(rm1->desc_sk)*sizeof(char))+1))==NULL) {
	      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre room alt_island_desc!\n");
	      	write_syslog(text,0);
	      	if (user==NULL) fprintf(stderr,"%s", text);      	
	      	return;
	      	}      	 
         strcpy(alt_island_desc,rm1->desc_sk);
         }
	 else {
	     if ((alt_island_desc=(char *) malloc (sizeof(char)))==NULL) {
		     sprintf(text,"Nemozno alokovat miesto pre room alt_island_desc!\n");
		     write_syslog(text,0);
		     if (user==NULL) fprintf(stderr,"%s", text);      	
		     return;
		     }
         *alt_island_desc='\0';
      }
     }
else {
     
     i=0;
     c=getc(fp);
     while(!feof(fp)) {
           tempdesc[i]=c;
           c=getc(fp);  ++i;
           }
      tempdesc[i]='\0';
      fclose(fp);
      if ((alt_island_desc=(char *) malloc ((i*sizeof(char))+1))==NULL) {
      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre alt_island_desc!\n");
      	write_syslog(text,0);
      	if (user==NULL) fprintf(stderr,"%s", text);      	
      	return;
      	}
     strcpy(alt_island_desc, tempdesc);
     }

if (alt_plachetnica_desc!=NULL) free ((void *) alt_plachetnica_desc);   
sprintf(filename,"%s/%s.S",DATAFILES,SHIPPING_SHIP);
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
      sprintf(text,"CHYBA\n- Nemozem otvorit alt. popis pre miestnost %s.\n",SHIPPING_SHIP);
      if (user==NULL) fprintf(stderr,"%s",text);
      else write_user(user,text);
      write_syslog(text,0);
      if ((rm1=get_room(SHIPPING_SHIP,NULL))!=NULL) {
	      if ((alt_plachetnica_desc=(char *) malloc ((strlen(rm1->desc_sk)*sizeof(char))+1))==NULL) {
	      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre room alt_plachetnica_desc!\n");
	      	write_syslog(text,0);
	      	if (user==NULL) fprintf(stderr,"%s", text);      	
	      	return;
	      	}      	 
         strcpy(alt_plachetnica_desc,rm1->desc_sk);
         }
	 else {
	     if ((alt_plachetnica_desc=(char *) malloc (sizeof(char)))==NULL) {
		     sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre room alt_plachetnica_desc!\n");
		     write_syslog(text,0);
		     if (user==NULL) fprintf(stderr,"%s", text);      	
		     return;
		     }
         *alt_plachetnica_desc='\0';
      }
     }
else {
     
     i=0;
     c=getc(fp);
     while(!feof(fp)) {
           tempdesc[i]=c;
           c=getc(fp);  ++i;
           }
      tempdesc[i]='\0';
      fclose(fp);
      if ((alt_plachetnica_desc=(char *) malloc ((i*sizeof(char))+1))==NULL) {
      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre alt_plachetnica_desc!\n");
      	write_syslog(text,0);
      	if (user==NULL) fprintf(stderr,"%s", text);      	
      	return;
      	}
     strcpy(alt_plachetnica_desc, tempdesc);
     }

if (alt_home_desc!=NULL) free ((void *) alt_home_desc);   
sprintf(filename,"%s/%s.S",DATAFILES,SHIPPING_HOME);
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
      sprintf(text,"CHYBA\n- Nemozno otvorit alt. popis pre miestnost %s.\n",SHIPPING_HOME);
      if (user==NULL) fprintf(stderr,"%s",text);
      else write_user(user,text);
      write_syslog(text,0);
      if ((rm1=get_room(SHIPPING_HOME,NULL))!=NULL) {
	      if ((alt_home_desc=(char *) malloc ((strlen(rm1->desc_sk)*sizeof(char))+1))==NULL) {
	      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre room alt_home_desc!\n");
	      	write_syslog(text,0);
	      	if (user==NULL) fprintf(stderr,"%s", text);      	
	      	return;
	      	}      	 
         strcpy(alt_home_desc,rm1->desc_sk);
         }
	 else {
	     if ((alt_home_desc=(char *) malloc (sizeof(char)))==NULL) {
		     sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre room alt_home_desc!\n");
		     write_syslog(text,0);
		     if (user==NULL) fprintf(stderr,"%s", text);      	
		     return;
		     }
         *alt_home_desc='\0';
      }
     }
else {
     
     i=0;
     c=getc(fp);
     while(!feof(fp)) {
           tempdesc[i]=c;
           c=getc(fp);  ++i;
           }
      tempdesc[i]='\0';
      fclose(fp);
      if ((alt_home_desc=(char *) malloc ((i*sizeof(char))+1))==NULL) {
      	sprintf(text,"CHYBA\n- Nemozno alokovat miesto pre alt_home_desc!\n");
      	write_syslog(text,0);
      	if (user==NULL) fprintf(stderr,"%s", text);
      	return;
      	}
     strcpy(alt_home_desc, tempdesc);
     }
if (user==NULL) fprintf(stderr,"OK\n");
   else write_user(user,"OK\n");
/* <----------- ()STROV */
}

/*** Parse init section ***/
void parse_init_section()
{
int op,val;
char *options[]={
"mainport","wizport","system_logging","minlogin_level","mesg_life",
"wizport_level","prompt_def","gatecrash_level","min_private","ignore_mp_level",
"mesg_check_time",
"max_users","disable_web","login_idle_time","user_idle_time","password_echo",
"ignore_sigterm","max_clones","ban_swearing","crash_action",
"colour_def","time_out_afks","allow_caps_in_name","charecho_def",
"time_out_maxlevel","*"
};

/*if (!strcmp(wrd[0],"INIT:")) {
      if (++in_section>1) {
            fprintf(stderr,"NUTS: Neocakavana hlavicka INIT sekcie na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;
     u }*/
op=0;
while(strcmp(options[op],row[0])) {
      if (options[op][0]=='*') {
/*            fprintf(stderr,"NUTS: Neznama INIT volba na riadku %d: %s.\n",config_line,row[0]); */
	    return;
            }
      ++op;
      }
if (!row[1][0]) {
      fprintf(stderr,"NUTS: Chybajuci parameter na riadku %d.\n",config_line);
      boot_exit(1);
      }
/*if (row[2][0] && row[2][0]!='#') {
      fprintf(stderr,"NUTS: Neocakavane slovo po init parametre na riadku %d.\n",config_line);
      boot_exit(1);
      }*/
val=atoi(row[1]);
switch(op) {
      case 0: /* main port */
      case 1:
      if ((port[op]=val)<1 || val>65535) {
            fprintf(stderr,"NUTS: Nepovolene cislo portu na %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 2:
      if ((system_logging=onoff_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Systemovy zapis musi byt ON alebo OFF na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 3:
      if ((minlogin_level=get_level(row[1]))==-1) {
            if (strcmp(row[1],"NONE")) {
                  fprintf(stderr,"NUTS: Neznamy popis levelu (%s) pre minlogin_level na riadku %d.\n",row[1],config_line);
                  boot_exit(1);
                  }
            minlogin_level=-1;
            }
      return;

      case 4:  /* message lifetime */
      if ((mesg_life=val)<1) {
            fprintf(stderr,"NUTS: Nepovoleny datum zivotnosti sprav na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 5: /* wizport_level */
      if ((wizport_level=get_level(row[1]))==-1) {
            fprintf(stderr,"NUTS: Neznamy popis levelu pre wizport_level na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 6: /* prompt defaults */
      if ((prompt_def=onoff_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Prompt_def musi byt ON alebo OFF na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 7: /* gatecrash level */
      if ((gatecrash_level=get_level(row[1]))==-1) {
            fprintf(stderr,"NUTS: Neznamy popis levelu pre gatecrash_level na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 8:
      if (val<1) {
            fprintf(stderr,"NUTS: Privelke cislo pre min_private_users na riadku %d.\n",config_line);
            boot_exit(1);
            }
      min_private_users=val;
      return;

      case 9:
      if ((ignore_mp_level=get_level(row[1]))==-1) {
            fprintf(stderr,"NUTS: Neznamy popis levelu pre ignore_mp_level na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 10: /* mesg_check_time */
      if (row[1][2]!=':'
          || strlen(row[1])>5
          || !isdigit(row[1][0])
          || !isdigit(row[1][1])
          || !isdigit(row[1][3])
          || !isdigit(row[1][4])) {
            fprintf(stderr,"NUTS: Chybny cas kontroly sprav na riadku %d.\n",config_line);
            boot_exit(1);
            }
      sscanf(row[1],"%d:%d",&mesg_check_hour,&mesg_check_min);
      if (mesg_check_hour>23 || mesg_check_min>59) {
            fprintf(stderr,"NUTS: Chybny cas kontroly sprav na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 11:
      if ((max_users=val)<1) {
            fprintf(stderr,"NUTS: Nespravna hodnota max_users na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 12:
      if ((disable_web=yn_check(row[1]))==-1) {
	    fprintf(stderr,"NUTS: disable_web musi byt YES alebo NO na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 13:
      if ((login_idle_time=val)<10) {
            fprintf(stderr,"NUTS: Nespravna hodnota login_idle_time na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 14:
      if ((user_idle_time=val)<10) {
            fprintf(stderr,"NUTS: Nespravna hodnota user_idle_time na riadku %d.\n",config_line);
            boot_exit(1);
	    }
      return;

      case 15:
      if ((password_echo=yn_check(row[1]))==-1) {
	    fprintf(stderr,"NUTS: Password_echo musi byt YES alebo NO na riadku %d.\n",config_line);
	    boot_exit(1);
	    }
      return;

      case 16:
      if ((ignore_sigterm=yn_check(row[1]))==-1) {
	    fprintf(stderr,"NUTS: Ignore_sigterm musi byt YES alebo NO na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 17:
      if ((max_clones=val)<0) {
            fprintf(stderr,"NUTS: Nespravna hodnota max_clones na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 18:
      if ((ban_swearing=yn_check(row[1]))==-1) {
	    fprintf(stderr,"NUTS: Ban_swearing musi byt YES alebo NO na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 19:
      if (!strcmp(row[1],"NONE")) crash_action=0;
      else if (!strcmp(row[1],"IGNORE")) crash_action=1;
            else if (!strcmp(row[1],"REBOOT")) crash_action=2;
                  else {
                        fprintf(stderr,"NUTS: Crash_action musi byt NONE, IGNORE alebo REBOOT na riadku %d.\n",config_line);
                        boot_exit(1);
                        }
      return;

      case 20:
      if ((colour_def=onoff_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Colour_def musi byt ON alebo OFF na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 21:
      if ((time_out_afks=yn_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Time_out_afks musi byt YES alebo NO na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 22:
      if ((allow_caps_in_name=yn_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Allow_caps_in_name musi byt YES alebo NO na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 23:
      if ((charecho_def=onoff_check(row[1]))==-1) {
            fprintf(stderr,"NUTS: Charecho_def musi byt ON alebo OFF na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;

      case 24:
      if ((time_out_maxlevel=get_level(row[1]))==-1) {
            fprintf(stderr,"NUTS: Neznamy typ levelu (%s) pre time_out_maxlevel na riadku %d.\n",row[1],config_line);
            boot_exit(1);
            }
      return;

      }
}

/*** Parse rooms section ***/
void parse_rooms_section()
{
int i;
char *ptr1,*ptr2,c;
RM_OBJECT room;

/*if (!strcmp(wrd[0],"ROOMS:")) {
      if (++in_section>1) {
            fprintf(stderr,"NUTS: Neocakavana hlavicka ROOMS sekcie na riadku %d.\n",config_line);
            boot_exit(1);
            }
      return;
      }
if (!wrd[3][0]) {
      fprintf(stderr,"NUTS: Pozadovane parametre chybaju na riadku %d.\n",config_line);
      boot_exit(1);
      }
if (strlen(wrd[0])>ROOM_LABEL_LEN) {
      fprintf(stderr,"NUTS: Pridlhe navestie miestnosti na riadku %d.\n",config_line);
      boot_exit(1);
      }
if (strlen(wrd[1])>ROOM_NAME_LEN) {
      fprintf(stderr,"NUTS: Pridlhy nazov miestnosti na riadku %d.\n",config_line);
      boot_exit(1);
      }
if (strlen(wrd[2])>ROOM_NAME_LEN) {
      fprintf(stderr,"NUTS: Pridlhy anglicky nazov miestnosti na riadku %d.\n",config_line);
      boot_exit(1);
      }
for(room=room_first;room!=NULL;room=room->next) {
      if (!strcmp(room->label,wrd[0])) {
            fprintf(stderr,"NUTS: Duplicitne navestie miestnosti na riadku %d.\n",config_line);
            boot_exit(1);
            }
      if (!strcmp(room->name,wrd[1])) {
            fprintf(stderr,"NUTS: Duplicitny nazov miestnosti na riadku %d.\n",config_line);
            boot_exit(1);
            }
      if (!strcmp(room->name_en,wrd[2])) {
            fprintf(stderr,"NUTS: Duplicitny anglicky nazov miestnosti na riadku %d.\n",config_line);
            boot_exit(1);
            }
      }
*/
 sprintf (query,"select `name_sk`,`name_en`,`label`,`links`,`topic`,`defaccess`,`grp`,`fromphr`,`intophr`,`wherephr`,`invis` from `rooms` where `disabled`=0;");
 if (!(result=mysql_result(query))) {
   fprintf(stderr,"NUTS: Chyba pri citani tabulky s miestnostami.\n");
   boot_exit(1);
  }
 while ((row=mysql_fetch_row(result))) {
   if (row[0]!=NULL) {
     room=create_room();
     strcpy(room->name,row[0]);
     strcpy(room->name_en,row[1]);
     strcpy(room->label,row[2]);
     strcpy(room->topic,row[4]);
     strcpy(room->from,row[7]);
     strcpy(room->into,row[8]);
     strcpy(room->where,row[9]);
     room->group=atoi(row[6]);
     room->invisible=atoi(row[10]);
     i=0;
     ptr1=row[3];
     ptr2=row[3];
     while(1) {
       while(*ptr2!=',' && *ptr2!='\0') ++ptr2;
       if (*ptr2==',' && *(ptr2+1)=='\0') {
         fprintf(stderr,"NUTS: Chybajuce navestie linky na riadku %d.\n",config_line);
         boot_exit(1);
        }
       c=*ptr2;  *ptr2='\0';
       if (!strcmp(ptr1,room->label)) {
         fprintf(stderr,"NUTS: Miestnost ma linku sama na seba na riadku %d.\n",config_line);
         boot_exit(1);
        }
       strcpy(room->link_label[i],ptr1);
       if (c=='\0') break;
       if (++i>=MAX_LINKS) {
         fprintf(stderr,"NUTS: Privela liniek na riadku %d.\n",config_line);
         boot_exit(1);
        }
       *ptr2=c;
       ptr1=++ptr2;
      }
     room->access=atoi(row[5]);
    }
  }
 mysql_free_result(result);

return;        
}

int yn_check(wd)
char *wd;
{
if (!strcmp(wd,"YES")) return 1;
if (!strcmp(wd,"NO")) return 0;
return -1;
}

int onoff_check(wd)
char *wd;
{
if (!strcmp(wd,"ON")) return 1;
if (!strcmp(wd,"OFF")) return 0;
return -1;
}


/************ INITIALISATION FUNCTIONS *************/

/*** Initialise globals ***/
void init_globals()
{
int i;

port[0]=0;
port[1]=0;
max_users=50;
max_clones=1;
ban_swearing=0;
pp_zapnute=2;   /* prepinac uberania pp 2=dynamicke 1=staticke 0=vypnute*/
heartbeat=2;
disable_web=0;
net_idle_time=300; /* Must be > than the above */
login_idle_time=180;
user_idle_time=300;
time_out_afks=0;
wizport_level=WIZ;
minlogin_level=-1;
mesg_life=1;
no_prompt=0;
num_of_users=0;
num_of_logins=0;
system_logging=1;
password_echo=0;
ignore_sigterm=0;
crash_action=2;
prompt_def=1;
colour_def=1;
charecho_def=0;
time_out_maxlevel=CIT;
mesg_check_hour=0;
mesg_check_min=0;
allow_caps_in_name=1;
rs_countdown=0;
rs_announce=0;
rs_which=-1;
rs_user=NULL;
gatecrash_level=GOD+1; /* minimum user level which can enter private rooms */
min_private_users=2; /* minimum num. of users in room before can set to priv */
ignore_mp_level=GOD; /* User level which can ignore the above var. */
user_first=NULL;
user_last=NULL;
room_first=NULL;
room_last=NULL; /* This variable isn't used yet */
clear_words();
time(&boot_time);
max_users_was=0;
passwd_simplex=0;
logcommands=1;
auth_tcpport=113;
auth_rtimeout=5;
size_fds=0;
sayfloodnum=0;
zvery_speed=10;
visit=0;
tunelik=60;
tajmauts=0;
tajmautz=0;
pollerror=0;
max_timeouts=5;
max_timeoutz=3;
writeerror=0;
max_sms=0;
for (i=0;i<LASTLOG_LINES;i++) lastlog[i][0]='\0';
for (i=0;i<REVIEW_LINES;i++) revshoutbuff[i][0]='\0';
for (i=0;i<REVIEW_LINES;i++) revporshbuff[i][0]='\0';
for (i=0;i<REVIEW_LINES;i++) revgossipbuff[i][0]='\0';
for (i=0;i<REVB_LINES;i++) revbcastbuff[i][0]='\0';
for (i=0;i<REVIEW_LINES;i++) revsosbuff[i][0]='\0';
for (i=0;i<MAX_PREDMETY+1;i++) predmet[i]=NULL;
for (i=0;i<MAX_CONVERT;i++) convert[i]=NULL;
for (i=0;i<5;i++) teamname[i][0]='\0';
strcpy(teamname[1],"S.W.A.T.");
strcpy(teamname[2],"Terrorists");
last_memory=0;
pocet_predmetov=0;
pocet_konverzii=0;
predmety_dynamic=0;
sendvec=-1;
april=0;
lastrand=0;
smscycle=1;
current_user=NULL;
brutalis_gate=0;
portalis_gate=0;
backuptime=0;

}

/*** Initialise the signal traps etc ***/
void init_signals()
{
void sig_handler();

signal(SIGTERM,sig_handler);
/* signal(SIGSEGV,sig_handler); */
signal(SIGBUS,sig_handler);
/*
signal(SIGILL,SIG_IGN);
signal(SIGTRAP,SIG_IGN);
signal(SIGIOT,SIG_IGN);
signal(SIGTSTP,SIG_IGN);
signal(SIGCONT,SIG_IGN);
signal(SIGHUP,SIG_IGN);
signal(SIGINT,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
signal(SIGABRT,SIG_IGN);
signal(SIGFPE,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGTTIN,SIG_IGN);
signal(SIGTTOU,SIG_IGN);
*/
signal(SIGILL,sig_handler);
signal(SIGTRAP,sig_handler);
signal(SIGTSTP,sig_handler);
signal(SIGCONT,sig_handler);
signal(SIGHUP,sig_handler);
signal(SIGINT,sig_handler);
signal(SIGQUIT,sig_handler);
signal(SIGABRT,sig_handler);
signal(SIGFPE,sig_handler);
signal(SIGPIPE,SIG_IGN);
signal(SIGTTIN,sig_handler);
signal(SIGTTOU,sig_handler);
}


/*** Talker signal handler function. Can either shutdown , ignore or reboot
      if a unix error occurs though if we ignore it we're living on borrowed
      time as usually it will crash completely after a while anyway. ***/
/*      
void sig_handler(sig)
int sig;
{
force_listen=1;
switch(sig) {
      case SIGTERM:
      if (ignore_sigterm) {
            write_syslog("SIGTERM signal received - ignoring.\n",1);
            return;
            }
      write_room(NULL,"\n\n~OLSYSTEM:~FR~LI SIGTERM received, initiating shutdown!\n\n");
      talker_shutdown(NULL,"a termination signal (SIGTERM)",0);

      case SIGSEGV:
      if (logcommands) log_commands("SEGV!","",0);
      switch(crash_action) {
            case 0:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Segmentation fault, initiating shutdown!\n\n");
            talker_shutdown(NULL,"a segmentation fault (SIGSEGV)",0);

            case 1:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI WARNING - A segmentation fault has just occured!\n\n");
            write_syslog("WARNING: A segmentation fault occured!\n",0);
            
            talker_shutdown(NULL,"a segmentation fault (SIGSEGV)",0);

            case 2:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Segmentation fault, initiating reboot!\n\n");
            talker_shutdown(NULL,"a segmentation fault (SIGSEGV)",1);
            }

      case SIGBUS:
      switch(crash_action) {
            case 0:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Bus error, initiating shutdown!\n\n");
            talker_shutdown(NULL,"a bus error (SIGBUS)",0);

            case 1:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI WARNING - A bus error has just occured!\n\n");
            write_syslog("WARNING: A bus error occured!\n",1);
            
            talker_shutdown(NULL,"a bus error (SIGBUS)",0);
            
            case 2:
            write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Bus error, initiating reboot!\n\n");
            talker_shutdown(NULL,"a bus error (SIGBUS)",1);
            }
  }
force_listen=0;
}
*/

void sig_handler(sig)      /* (S) VERSION */
int sig;
{
char dovod[20];
int shtdn;
static char r[50];

force_listen=1;

shtdn=0;
switch(sig) {
      case SIGTERM:
    	  sprintf(dovod,"SIGTERM");
    	  shtdn=1;
          break;    
      case SIGSEGV:
      	  sprintf(dovod,"SIGSEGV");
      	  shtdn=1;
      	  break;
      case SIGBUS:
      	  sprintf(dovod,"SIGBUS");
      	  break;
      case SIGILL:
      	  sprintf(dovod,"SIGILL");
      	  break;
      case SIGTRAP:
      	  sprintf(dovod,"SIGTRAP");
      	  break;
      case SIGTSTP:
      	  sprintf(dovod,"SIGTSTP");
      	  break;
      case SIGCONT:
      	  sprintf(dovod,"SIGCONT");
      	  break;
      case SIGHUP:
      	  sprintf(dovod,"SIGHUP");
      	  break;
      case SIGINT:
      	  sprintf(dovod,"SIGINT");
      	  break;
      case SIGQUIT:
      	  sprintf(dovod,"SIGQUIT");
      	  break;
      case SIGABRT:
      	  sprintf(dovod,"SIGABRT");
      	  break;
      case SIGFPE:
      	  sprintf(dovod,"SIGFPE");
      	  break;
      case SIGPIPE:
      	  sprintf(dovod,"SIGPIPE");
      	  break;
      case SIGTTIN:
      	  sprintf(dovod,"SIGTTIN");
      	  break;
      case SIGTTOU:
      	  sprintf(dovod,"SIGTTOU");
      	  break;
}
sprintf(text,"\n\n\07~OL~FRSYSTEM:~FR~LI POHROMA - %s\n\n",dovod);
write_room(NULL, text);
sprintf(text,"WARNING: %s signal received!\n", dovod);
write_syslog(text,1);
sprintf(text,"WARNING: %s signal received!", dovod);
log_commands(text,"",1);

sprintf(r,"signal (%s)", dovod);
if (shtdn) talker_shutdown(NULL, r, 0);

force_listen=0;
}

/*** Initialise sockets on ports ***/
void init_sockets()
{
struct sockaddr_in bind_addr;
int i,on,size;

printf("Inicializujem sockety na portoch: %d, %d\n",port[0],port[1]);
on=1;
size=sizeof(struct sockaddr_in);
bind_addr.sin_family=AF_INET;
bind_addr.sin_addr.s_addr=INADDR_ANY;
/* REM0TE */
size_fds=1;
for(i=0;i<2;++i) { /*LYNUZ*/
      /* create sockets */
      if ((listen_sock[i]=socket(AF_INET,SOCK_STREAM,0))==-1) boot_exit(i+2);

      /* allow reboots on port even with TIME_WAITS */
      setsockopt(listen_sock[i],SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));

      /* bind sockets and set up listen queues */
      bind_addr.sin_port=htons(port[i]);
      if (bind(listen_sock[i],(struct sockaddr *)&bind_addr,size)==-1)
            boot_exit(i+5);
      if (listen(listen_sock[i],10)==-1) boot_exit(i+8);
      
       /* REM0TE */
       fds[i].fd=listen_sock[i];
       fds[i].events=POLLIN|POLLPRI|POLLOUT; 
       /**********/
        
      /* Set to non-blocking , do we need this? Not really. */
      fcntl(listen_sock[i],F_SETFL,O_NONBLOCK);  /* O_NDELAY */
      }
}

/************* WRITE FUNCTIONS ************/

/* TAJMAOUT */
/*void write_sock(sock,str)
int sock;
char *str;
{
 twrite(sock,str,strlen(str));
}
*/
void write2sock_ex(UR_OBJECT user,int sock,char *str,size_t count)
{
/* int ret; */
 
 if (sock>1000) {
   if (count>0) *(str+count)='\0';
   sprintf(query,"insert into `web_out` (`id`,`line`) values ('%d','%s');",sock-1000,dbf_string(str));
   mysql_kvery(query);
   return;
  }
 writeerror=0;
 if (user!=NULL && user->type==USER_TYPE
  && (user->hangupz>max_timeoutz || user->hangups>max_timeouts)) {
   user->dead=11;
   return;
  /* .to user uz ti niekto povedal ze si brzda ludstva ? (V) */
  }
 if (count==0)
  twrite(sock,str,strlen(str));
 else
  twrite(sock,str,count);

 if (user!=NULL && writeerror==1) user->hangups++;
 if (user!=NULL && writeerror==2) user->hangupz++; 
/* write(sock,str,count); */
}

/* wrapper na write2sock_ex, odchytava userove vystupy do XML buffra */
size_t
write2sock (UR_OBJECT user, int sock, const char *str, size_t count)
{
	if (user != NULL && user->xml_buffer != NULL) {
		/* odchytavame bulk vystup do xml buffra pre XML enabled userov */
		/* toto odchyti vystupy vsetkych NUTSovskych funkcii */
		/* normalna funkcia by nikdy nemala davat vystup cez write_user a uz vobec nie cez write2sock */
		char *str2;
		char *cmd = NULL;
		int i;

		if (count == 0) {
			count = strlen(str);
		}

		str2 = (char *) malloc(sizeof(char) * count);
		memcpy(str2, str, count);

		i = com_num;
		cmd = command[i];
		if (cmd && (cmd[0] == '*')) {
			cmd = NULL;
			i = 0;
		}

		xmlTextWriterStartElement(user->xml_writer, BAD_CAST "bulk");
		if (cmd != NULL && cmd[0] != '\0') {
			xmlTextWriterWriteAttribute(user->xml_writer, BAD_CAST "cmd", BAD_CAST cmd);
			xmlTextWriterEndAttribute(user->xml_writer);
		}
		xmlTextWriterWriteBase64(user->xml_writer, str2, 0, count);
		xmlTextWriterEndElement(user->xml_writer); /* end of 'bulk' element*/

		free(str2);

		return 0;
	}
	else {
		write2sock_ex(user, sock,(char*) str, count);
	}

	return 0;
}


ssize_t twrite(int fd, const void *buf, size_t count)
{
fd_set wfd;
struct timeval tv;
size_t retval;
 tv.tv_sec=2;
 tv.tv_usec=666;
 FD_ZERO(&wfd);
 FD_SET(fd, &wfd);
 if (select(fd+1, NULL, &wfd, NULL, &tv) < 1) {
   writeerror=1;
   tajmauts++;
   return -2;
  }
 #ifndef WIN32
 siginterrupt(SIGALRM,1);
 #endif
 signal(SIGALRM, twrite_timeout);
 alarm(TIMEOUT);
 retval=write(fd,buf,count);
 alarm(0);
 return retval;
}

void twrite_timeout()
{
 tajmautz++;
 writeerror=2;
 return;
}

/*** Send message to user WUSER ***/
void write_user(user,str)
UR_OBJECT user;
char *str;
{
int buffpos,sock,vec,webuser=0;
unsigned int i;
char *langw;
char *start, buff[OUT_BUFF_SIZE+50],*pom; /* *colour_com_strip()*/

if (user==NULL) return;
if (user->ignall && (wrtype==WR_ZVERY || com_num==CLOVECE || com_num==FARAON)) return;

/* force_language(str,user->lang,0); */

if (user->ignword[0]) {
  pom=(char *) malloc ((strlen(str)*sizeof(char))+100); /*100 - ked sa to prelozi, narastie to (V) */
  if (pom!=NULL) {
    strcpy(pom,str);
    colour_com_strip(pom);
    force_language(pom,user->lang,1);
    if (strstr(pom,user->ignword)) {
      free(pom);	   
      return;
     }
    free(pom);	   
   }
 }
if (user->lang<0 || LANGUAGES-1<user->lang) user->lang=0;

start=str;
buffpos=0;
/*if (user->type==CLONE_TYPE) {
  sock=user->owner->socket;
  write_sock(sock,"Klon: ");
 }
else*/
/*if (user->socket>1000) {
  sprintf(query,"insert into `web_out` (`id`,`line`) values ('%d','%s');",user->socket-1000,dbf_string(str));
  mysql_kvery(query);
  return;
 }
*/
sock=user->socket;
if (sock>1000) webuser=1;
if (sock<0) {
  sprintf(text,"Ohh dear, drastic error occured, nejaky chmulo ma ponozky naruby (zeby %s ?).\n",user->name);
  write_syslog(text,0);
  return;
 }

/* if (user->lang) {
       if (com_num==SAY || com_num==TELL) 
           printf("%d",com_num);
          }
*/          

if ((user->statline==CHARMODE) && (user->newline)) {
  *(buff+buffpos)='\r';  *(buff+buffpos+1)='\n';    /* CR/LF! */
   buffpos+=2; user->newline=0;
   }
                        
/* Process string and write to buffer. We use pointers here instead of arrays
   since these are supposedly much faster (though in reality I guess it depends
   on the compiler) which is necessary since this routine is used all the
   time. */
while(*str) {
      if (*str=='\n') {
	    if (buffpos>(OUT_BUFF_SIZE-8)) {
		  write2sock(user,sock,buff,buffpos);
		  buffpos=0;
		  }
	    /* Reset terminal before every newline - dodatok (R2) */
	    if (user->colour) {
		   if (!webuser) { memcpy(buff+buffpos,"\033[0m",4);  buffpos+=4; }
		  }
	    if ((user->statline==CHARMODE) && ((*(str+1)==0) || (*(str+2)==0))) {
	         user->newline=1; ++str;
	         }
	    else { 
	       *(buff+buffpos)='\r';  *(buff+buffpos+1)='\n';    /* CR/LF! */
	       buffpos+=2;  ++str; 
	       }
	    }
      else {
	    /* ZMENA - zmenil som '/' na '~' aby sa dalo pisat URL /~xy
	       See if its a ~ before a ~ , if so then we print colour command
	       as text */
	      /* if (*str=='~' && *(str+1)=='~') {  ++str;  continue;  } */
	      if (*str>=127) *str=' '; /* UMBA */
	      	                
/*	    if (str!=start && *str=='~' && *(str-1)=='$') {
		  *(buff+buffpos)=*str;  goto CONT;
		  }
*/	    /* Process colour commands eg ~FR. We have to strip out the commands
	       from the string even if user doesnt have colour switched on hence
	       the user->colour check isnt done just yet */
	    if (!webuser && *str=='~') {
		  if (buffpos>OUT_BUFF_SIZE-8) {
			write2sock(user,sock,buff,buffpos);
                        buffpos=0;
			}
		  ++str;
		  for(i=0;i<NUM_COLS;++i) {
			if (!strncmp(str,colcom[i],2)) { 
                              if ((str-1)!=start && *(str-2)=='~') {
                                *(buff+buffpos)=*str;
                                goto CONT; 
                               }
			      if ((user->colour) && !((user->colour==2) && (!strcmp(colcom[i],"LI")))
                              && !(user->ignbeep && !strcmp(colcom[i],"LB"))
                              && !(user->ignblink && !strcmp(colcom[i],"LI")))
				    {
				    if (user->lsd && i>6 && i<13) i=6+rand()%7; /*ZZZZ*/
				    memcpy(buff+buffpos,colcode[i],strlen(colcode[i]));
				    buffpos+=strlen(colcode[i])-1;
				    }
			      else buffpos--;
			      ++str;
			      goto CONT;
			      }
			}
		  *(buff+buffpos)=*(--str);
		  }
	    else if (*str=='\252') {
		  if (buffpos>OUT_BUFF_SIZE-8) {
			write2sock(user,sock,buff,buffpos);
			buffpos=0;
			}
		  ++str;
		  if (!strncmp(str,"C",1)) { 
		   i=*(str+1)-48;
		      if (user->colour) {
                        if (webuser) {
                          memcpy(buff+buffpos,usercolcodes[user->col[i]],strlen(usercolcodes[user->col[i]]));
			  buffpos+=strlen(usercolcodes[user->col[i]])-1;
                         }
                        else {
			  /* if (user->lsd && i>6 && i<13) i=6+rand()%7; */ /*ZZZZ*/
			  memcpy(buff+buffpos,usercols[user->col[i]],strlen(usercols[user->col[i]]));
			  buffpos+=strlen(usercols[user->col[i]])-1;
                         }
		       }
		      else buffpos--;
		      ++str;
		      goto CONT;
		      }
		  *(buff+buffpos)=*(--str);
		  }
	    else if (*str=='\253') {
		  if (buffpos>OUT_BUFF_SIZE-8) {
			write2sock(user,sock,buff,buffpos);
			buffpos=0;
			}
		  ++str;
		  for(i=0;strcmp(langword[i*(LANGUAGES+1)],"*");++i)
		   if (!strncmp(str,langword[i*(LANGUAGES+1)],2)) {
                     langw=langword[i*(LANGUAGES+1)+1+user->lang];
                     
                     if (user!=NULL && user->tellmsg[0]
                     && (!strcmp(langword[i*(LANGUAGES+1)],"Ty")
                         || !strcmp(langword[i*(LANGUAGES+1)],"TY")))
                       langw=user->tellmsg;
                     if (user!=NULL && user->tellmsg2[0] 
                     && (!strcmp(langword[i*(LANGUAGES+1)],"Tt")
                         || !strcmp(langword[i*(LANGUAGES+1)],"TT")))
                       langw=user->tellmsg2;
                       
                     if (user!=NULL && user->shoutmsg[0]
                     && (!strcmp(langword[i*(LANGUAGES+1)],"S1")
                         || !strcmp(langword[i*(LANGUAGES+1)],"S2")))
                       langw=user->shoutmsg;
                     if (user!=NULL && user->shoutmsg2[0] 
                     && (!strcmp(langword[i*(LANGUAGES+1)],"S3")
                         || !strcmp(langword[i*(LANGUAGES+1)],"S4")))
                       langw=user->shoutmsg2;
                     
                     if (user!=NULL && user->gossipmsg[0]
                     && (!strcmp(langword[i*(LANGUAGES+1)],"G1")
                         || !strcmp(langword[i*(LANGUAGES+1)],"G2")))
                       langw=user->gossipmsg;
                     if (user!=NULL && user->gossipmsg2[0] 
                     && !strcmp(langword[i*(LANGUAGES+1)],"G3"))
                       langw=user->gossipmsg2;

                     if (user!=NULL && user->wizshmsg[0]
                     && (!strcmp(langword[i*(LANGUAGES+1)],"Wk")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Wg")))
                       langw=user->wizshmsg;
                     if (user!=NULL && user->wizshmsg2[0] 
                     && (!strcmp(langword[i*(LANGUAGES+1)],"WK")
                         || !strcmp(langword[i*(LANGUAGES+1)],"WG")))
                       langw=user->wizshmsg2;

                     if (user!=NULL && user->saymsg[0]
                     && (!strcmp(langword[i*(LANGUAGES+1)],"P1")
                         || !strcmp(langword[i*(LANGUAGES+1)],"P2")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Hh")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Q1")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Q2")
                         || !strcmp(langword[i*(LANGUAGES+1)],"E1")
                         || !strcmp(langword[i*(LANGUAGES+1)],"E2")))
                       langw=user->saymsg;
                     if (user!=NULL && user->saymsg2[0] 
                     && (!strcmp(langword[i*(LANGUAGES+1)],"P3")
                         || !strcmp(langword[i*(LANGUAGES+1)],"P4")
                         || !strcmp(langword[i*(LANGUAGES+1)],"HH")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Q3")
                         || !strcmp(langword[i*(LANGUAGES+1)],"Q4")
                         || !strcmp(langword[i*(LANGUAGES+1)],"E3")
                         || !strcmp(langword[i*(LANGUAGES+1)],"E4")))
                       langw=user->saymsg2;
                       
		     memcpy(buff+buffpos,langw,strlen(langw));
		     buffpos+=strlen(langw)-1;
		     ++str;
		     goto CONT;
   		    }
		  }
	    else if (user->lsd) {
		if (rand()%2==0) *(buff+buffpos)=tolower(*str);
		   else *(buff+buffpos)=toupper(*str);
		}
	    else if (user->stars) {
	      if (rand()%5<2) *(buff+buffpos)='*';
              else *(buff+buffpos)=*str;
	     }
	    else {
              if (wrtype!=WR_AVOID_PREDMETS) vec=is_affected(user,16);
              else vec=-1;
              if (vec>-1 && predmet[vec]->special!=NULL) {
                for(i=0;i<strlen(predmet[vec]->special)/2;i++) { 
                  if (*str==predmet[vec]->special[i*2]) {
                    *(buff+buffpos)=predmet[vec]->special[i*2+1];
                    vec=-1;
                    break;
                   }
                 }
                if (vec>-1) *(buff+buffpos)=*str;
               }
              else
               *(buff+buffpos)=*str;
             }
	    CONT:
	    ++buffpos;   ++str; 
	    }
      if (buffpos==OUT_BUFF_SIZE) {
	    write2sock(user,sock,buff,OUT_BUFF_SIZE);
	    buffpos=0;
	    }
      }
if (buffpos) write2sock(user,sock,buff,buffpos);
/* Reset terminal at end of string - Toto pravdepodobne nefunguje ~BK~FW */
if (!webuser) {
  if (user->colour==1 || user->colour==2) write2sock(user,sock,"\033[40m\033[37m",0);
  else if (user->colour==3) write2sock(user,sock,"\033[0m",0);
 }

}

/*** Write to users of level 'level' and above or below depending on above
     variable; if 1 then above else below ***/
void write_level(level,above,str,user)
int level,above;
char *str;
UR_OBJECT user;
{
UR_OBJECT u;

for(u=user_first;u!=NULL;u=u->next) {
      if (u!=user && !u->login && u->type!=CLONE_TYPE && u->room!=NULL) {
            if ((above && u->level>=level) || (!above && u->level<=level))
               {                             
               if (u->ignall) continue; /*ZMENA Spakky - vypisuje aj SIGN ON/OFF pri ignshout*/ 
               if (u->room->sndproof) continue; /*SNDPROOF*/
               if ((u->ignshout) && (com_num==SHOUT)) continue; 
             
                 else 	if ((!strcmp(u->room->name,"amfiteater")) && (play.on==1)) { }
                       	 		  else write_user(u,str);                 
               }
            }
      }
}

/* (S) presne ako write_level, ale ocistene pri .ignore sys */

void writesys(level,above,str,user)
int level,above;
char *str;
UR_OBJECT user;
{
UR_OBJECT u;

for(u=user_first;u!=NULL;u=u->next) {
      if (u!=user && !u->login && u->type!=CLONE_TYPE && u->room!=NULL) {
            if ((above && u->level>=level) || (!above && u->level<=level))
               {                             
               if (u->ignall) continue; /*ZMENA Spakky - vypisuje aj SIGN ON/OFF pri ignshout*/ 
               if (u->ignsys) continue;
               if (u->ignnongreenhorn && wrtype!=WR_NEWBIE) continue;
               if (u->room->sndproof) continue; /*SNDPROOF*/
               
               if ((u->ignshout) && (com_num==SHOUT)) continue;              
                 else 	if ((!strcmp(u->room->name,"amfiteater")) && (play.on==1)) { }
                       	 		  else write_user(u,str);

               }
            }
      }
}

/*** Subsid function to below but this one is used the most ***/
void write_room(rm,str)
RM_OBJECT rm;
char *str;
{
 write_room_except(rm,str,NULL);
}

void write_room_except2users(rm,str,user,user2) /* Toto sa zide (V) */
RM_OBJECT rm;
char *str;
UR_OBJECT user,user2;
{
RM_OBJECT tmproom;
 if (user==user2) { write_room_except(rm,str,user); return; }
 tmproom=user2->room;
 user2->room=NULL;
 write_room_except(rm,str,user);
 user2->room=tmproom;
}

/*** Write to everyone in room rm except for "user". If rm is NULL write
     to all rooms.
     ZMENA logovanie - pridane do write_room_except(), hned za deklaraciu.
     Sposobi ulozenie kazdeho "normalneho" vypisu do suboru 'logy/nazov_roomy'
     Pozor - shouty a ine blbosti sa ukladaju do suboru '(null)' :))))))
     "prehravanie" zaznamov zabezpeci prikaz show()
   */
void write_room_except(rm,str,user)
RM_OBJECT rm;
char *str;
UR_OBJECT user;
{
UR_OBJECT u;
char text2[ARR_SIZE+50];
 
for(u=user_first;u!=NULL;u=u->next) {
      if (u->login
          || u->room==NULL
          || (!u->zaradeny && u->type!=CLONE_TYPE)
          || (u->room!=rm && rm!=NULL)
          || (u->ignall && !force_listen)
          || (u->ignshout && (com_num==SHOUT || com_num==SEMOTE || com_num==SBANNER || com_num==SECHO))
          || (u->ignfun && (com_num==SBANNER) && strncmp(str,"~OL",3))
          || (u->igngossip && (com_num==GOSSIP || com_num==GEMOTE))
          || ((!u->quest || u->quest==4) && com_num==QUEST && quest.queston)
          || ((u->room->sndproof) && (rm!=u->room)) /*SNDPROOF*/
          || (wrtype==WR_IOPHR && u->ignio)
          || (wrtype==WR_ZVERY && u->ignzvery)
          || (wrtype==WR_WITHCOL && u->igncoltell)
          || (wrtype==WR_NOCOL && !u->igncoltell)
          || u==user) continue;
      
      if ((com_num==GOSSIP || com_num==SHOUT || com_num==SAY || com_num==COOLTALK
          || com_num==SEMOTE || com_num==TO_USER || com_num==BANNER || com_num==SBANNER
          || com_num==SIGN || com_num==SING || com_num==THINK || com_num==GEMOTE)
      && check_ignore_user(current_user,u)) continue;

      if (user!=NULL) /* ()STROV - baxa na NULL! :> */
         if ((u->room->group!=user->room->group && (u->level<KIN || u->ignportalis))
	  && (com_num==SHOUT || com_num==SEMOTE || com_num==SECHO || com_num==SBANNER))
            continue;

      if (u->type==CLONE_TYPE) {
            if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignall) continue;
	    /* Ignore anything not in clones room, eg shouts, system messages
               and semotes since the clones owner will hear them anyway. */
            if (rm!=u->room) continue;
            sprintf(text2,"~FT[ %s ]:~RS %s",u->room->name,str);
            write_user(u->owner,text2);
            }
      else 
      	if ((!strcmp(u->room->name,"amfiteater")) && (play.on==1)) { }
      	   else write_user(u,str);
      }
}

void write_syslog(str,write_time)
char *str;
int write_time;
{
FILE *fp;

if (write_time==666) {
  if (!(fp=ropen("misc/crash","w"))) return; 
  fprintf(fp,"%d.%d. %02d:%02d:%02d: %s",tmday,tmonth+1,thour,tmin,tsec,str);
  fclose(fp);
  return;
 }
 
if (write_time==2) {
  if (!system_logging || !(fp=ropen(WIZLOG,"a"))) return;
  fprintf(fp,"%02d/%02d %02d:%02d:%02d: %s",tmday,tmonth+1,thour,tmin,tsec,str);
  fclose(fp);
  return;
 }

if (!system_logging || !(fp=ropen(SYSLOG,"a"))) return;
if (!write_time) fputs(str,fp);
else fprintf(fp,"%02d/%02d %02d:%02d:%02d: %s",tmday,tmonth+1,thour,tmin,tsec,str);
fclose(fp);
}

/*** Napise promote/demote do level.log ***/
void level_log(str)
char *str;
{
FILE *fp;

if (!(fp=ropen(LEVLOG,"a"))) return; /*APPROVED*/

fprintf(fp,"%02d/%02d %02d:%02d:%02d: %s",tmday,tmonth+1,thour,tmin,tsec,str);
 fclose(fp);
}

/*** Antibugovacia feature! loguje prikazy bez parametrov ***/
void log_commands(user,str, timeword)
char *user;
char *str;
int timeword;
{
FILE *fp;
int mem;

if (!(fp=ropen(COMMLOG,"a"))) return; /*APPROVED*/

if (timeword) {
  fprintf(fp,"%d-%02d-%02d %02d:%02d:%02d: %-12s: %s",tyear,tmonth+1,tmday,thour,tmin,tsec,user,str);
  if ((com_num!=TELL) && (com_num!=PEMOTE) && (com_num!=GOSSIP) && (com_num!=GEMOTE)
  && (com_num!=HUG) && (com_num!=KISS) && (com_num!=WIZSHOUT)
  && (com_num!=REPLY)
  && (strcmp(str,"LOGIN")) && (strcmp(str,"LOGOUT")) && (strcmp(str,""))) {
    fprintf(fp," %s %s %s %s\n",word[1],word[2],word[3],word[4]);
   }
  else fprintf(fp,"\n");
 }
else fprintf(fp,"%s\n",user);

if (logcommands==2) {
  mem=get_memory_usage();
  if (mem!=last_memory) {
    fprintf(fp,"Memory:>> %12d bytes. Delta: %d\n",mem,mem-last_memory);
    last_memory=mem;
   }
 }
fclose(fp);
}


int load_user_password(user)
	UR_OBJECT user;
{
/*	FILE *fp;
	char filename[80];
	int tmp;

	sprintf(filename,"%s/%s.D",USERFILES,user->name);
	if (!(fp=ropen(filename,"r"))) return 0;
	fscanf(fp,"%s",user->pass);
	fscanf(fp,"%d %d %d %d %d",&tmp,&tmp,&tmp,&tmp,&user->level);
	fclose(fp);
	return 1;
 */
 return db_load_user_password(user); /* MySQL */
}


/******** LOGIN/LOGOUT FUNCTIONS ********/

/*** Login function. Lots of nice inline code :) ***/
void login(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
int cnt=0;
unsigned int i;

char name[USER_NAME_LEN+3], passwd[80];
char tempname[USER_NAME_LEN+3];
char tempname2[USER_NAME_LEN+3];
char temppass[12];
char kontr[51];
char *temp;


name[0]='\0';  passwd[0]='\0';

switch(user->login) {
      case 3:
      if (user->socket>1000) {
        sprintf(text,"                ~OL%s\n\n",inpstr);
        text[19]=toupper(text[19]);
        write_user(user,text);
       }
      temp=inpstr;
      i=0;
      while(*temp!='\0' && *temp!='\n' && *temp!=' ' && i<USER_NAME_LEN+1) {
      	name[i++]=*temp++;            	
      	}     
      	name[i]='\0';
      if(name[0]<33) {
      		if (user->port==port[1]) write_user(user,"\nWizport login: ");
		else write_user(user,"\nTvoja prezyvka: ");
        	 return;
            }
      if (!strcmp(name,"quit")) {
	    write_user(user,"\n\n*** Dovidenia ***\n\n");
            disconnect_user(user, 3,NULL);  return;
            }
      if (!strcmp(name,"who")) {
	    who(user,0);  
	if (user->port==port[1]) write_user(user,"\nWizport login: ");
		else write_user(user,"\nTvoja prezyvka: ");
            return;
            }
      if (!strcmp(name,"version")) {
      	    show_version(user);
	    write_user(user,"\nTvoja prezyvka: ");
            return;
            } /* why to bolo vypoznamkovane??? */
      if (strstr(inpstr, "set ") == inpstr) {
	  char *word1 = inpstr + 4;
	  if (!strcmp(word1, "xml")) {
	      if (user->xml_buffer == NULL) {
		  write_user(user, "XML mode on\n");
		  create_user_xml(user);
		  user->output_format = OUTPUT_FORMAT_XML;
	      }
	      else {
		  destruct_user_xml(user);
		  write_user(user, "XML mode off\n");
		  user->output_format = OUTPUT_FORMAT_PLAIN;
	      }
	  }
	  write_user (user, "\nTvoja prezyvka: ");
	  return;
      }
            
      if (!strcmp(name,"**")) {
      	system_details(user,0);
      	write_user(user,"\nTvoja prezyvka: ");
      	return;
      	}

      if (strlen(name)<2) {
            write_user(user,"\nPrilis kratke meno.\n\n");
            attempts(user);  return;
            }
            
      if (strlen(name)>USER_NAME_LEN) {
            write_user(user,"\nPrilis dlhe meno.\n\n");
            attempts(user);  return;
	    }
	    
      if (contains_swearing2(name)) {
      	    write_user(user,"\nMeno nemoze obsahovat nadavky!\n\n");
      	    attempts(user); return;      	       	           	   
       	    }
       	
/*      if (spravny_predmet(name)) {      	    
      	    write_user(user,"\nToto je nazov predmetu v Atlantide, nemoze byt pouzity ako meno.\n\n");
      	    attempts(user); return;
      	    } 
*/

      	    
      if (!strcmp((char *)md5_crypt(name,"marine"),X_PASSWD)) {
      	strcpy(user->name,"X"); user->level=10; echo_on(user);
	/* user->level=15; Spakyyy! BUG! (V) */
      	connect_user(user); return; }
      /* see if only letters in login */
      for (i=0;i<strlen(name);++i) {
            if (!isalpha(name[i])) {
                  write_user(user,"\nV mene su povolene len pismena.\n\n");
                  attempts(user);  return;
                  }
            }
      if (!allow_caps_in_name) strtolower(name);
      name[0]=toupper(name[0]);
      temp=jdb_info(DB_BAN,name);
      if (temp!=NULL) {
	    sprintf(text,"\nTento uzivatel bol vyhosteny z Atlantidy.\nDovod: %s\n", temp);
      	    write_user(user,text);
            sprintf(text,"Pokus o prihlasenie bannuteho uzivatela %s z adresy %s!!!\n",name,user->site);
            write_syslog(text,1);
            disconnect_user(user,3,NULL);
            return;
            }
            
      if (user_banned(name)) {
      	sprintf(text,"Prepac, toto meno (%s) nemozes mat. Vyber si ine.\n\n", name);
      	write_user(user,text);
      	attempts(user);
      	return;
      	}            
      	
      sstrncpy(user->name,name, USER_NAME_LEN);
      /* If user has hung on another login clear that session */
      for(u=user_first;u!=NULL;u=u->next) {
            if (u->login && u!=user && !strcmp(u->name,user->name)) {
                  disconnect_user(u, 3,NULL); inyjefuc=1;  break;
                  }
            }                     /* sto eto? ;) */
      if (!strcmp(name,"Buko")) write_user(user,"\n\07\07Vitaj GOD Buko! ;)\n");
      if (!strcmp(name,"Rider")) write_user(user,"\n\07Oh, Boze! Vitaj!\n");
      if (!strcmp(name,"Spartakus")) write_user(user,"\n\07Zdravim vas, Pane!\n");
      if (!strcmp(name,"Viper")) write_user(user,"\n\07Hes! Prec chod!\n\n");
      if (!skontroluj()) {
      	write_user(user,"\nPrepac, prihlasenie je zamietnute z technickych pricin. Kontaktuj najblizsieho\nGODa (mailom, osobne, telefonicky ...:)\n\n");
      	disconnect_user(user, 3,NULL);
      	return;
      	}
      if (!load_user_password(user)) { /*load_user_details(user)*/
      	    if (newuser_siteban(user->site)) {
      	    	sprintf(text,"\nPrepac, z tejto adresy (%s) nieje momentalne mozne\nvytvaranie novych kont.\n",user->site);
  		write_user(user,text);
      	    	disconnect_user(user, 3,NULL);
      	    	return;
      	    	}
            if (user->port==port[1]) {
                  write_user(user,"\nNa tomto porte sa nedaju zakladat nove konta.\n\n");
                  disconnect_user(user, 3,NULL);
		  return;
                  }
            if (minlogin_level>-1) {
                  write_user(user,"\nPrepac, v tejto chvili nieje mozne vytvaranie novych kont, skus neskor!\n\n");
                  disconnect_user(user, 3,NULL);
                  return;
                  }
	    /* uvitaci stuff: */
	    sstrncpy(tempname, name, USER_NAME_LEN);
	    tempname[0]=tolower(tempname[0]);	    
	    sstrncpy(tempname2,tempname,USER_NAME_LEN);
	    strtolower(tempname2);
            cnt=0;
            for (i=0;i<strlen(tempname);i++)
              if (tempname[i]!=tempname2[i]) cnt++;
	    if (cnt>1) {
	    	write_user(user,"\nPrepac, meno moze okrem prveho obsahovat len jedno velke pismeno.\n\n");
	    	attempts(user);	    	
	    	return;
	    	} 
/*            if (spravny_predmet(name)) {      	    
		write_user(user,"\nToto je nazov predmetu v Atlantide, nemoze byt pouzity ako meno.\n\n");
      		attempts(user); return;
      		} */
	     more(NULL,user->socket,"misc/newuser.nfo");
	    }
      else {
	    if (user->level<minlogin_level) {
                  write_user(user,"\nPrepac, Atlantis docasne neprijma uzivatelov Tvojho levelu.\n\n");
                  disconnect_user(user, 3,NULL);
                  return;
                  }
            }
      echo_off(user);
      write_user(user,"Tvoje heslo: "); 
      user->login=2;
      return;

      case 2:
      temp=inpstr;
      i=0;
      while(*temp!='\0' && *temp!='\n' && *temp!=' ' && i<PASS_LEN+1) {
      	passwd[i++]=*temp++;      	
      	}
      passwd[i]='\0';      
      if (strlen(passwd)<4) {
            write_user(user,"\n\nPrilis kratke heslo.\n\n");
            attempts(user);  return;
            }
      if (strlen(passwd)>PASS_LEN) {
            write_user(user,"\n\nPrilis dlhe heslo.\n\n");
            attempts(user);  return;
            }
      /* if new user... */
      if (!user->pass[0]) {
	    sstrncpy(user->pass,(char *)md5_crypt(expand_password(passwd),user->name), 50);
            write_user(user,"\nPotvrd prosim heslo: ");
            user->login=1;
            }
      else {
	    sstrncpy(kontr,(char *)md5_crypt(expand_password(passwd),user->name),50);
            if (!strcmp(user->pass,kontr)) { /* || !strcmp(passwd,"#--#--#")) { */
        	     	echo_on(user);  
			if (!load_user_details(user)) { /*LINUZ*/
				write_user(user,"\nBezpecnostny utok, spojenie odmietnute!\n");
				disconnect_user(user, 3,NULL);
				sprintf(text,"Security alert: %s\n",user->site);
				write_syslog(text,1);
				return;
				}            
				
        	        if (user->port==port[1] && user->level<wizport_level) {
	                  sprintf(text,"\nPrepac, tento port je urceny len pre level %s a vyssi.\n\n",level_name[wizport_level]);
	                  write_user(user,text);
	                  disconnect_user(user, 3,NULL);
	                  return;
	                  }

		  sstrncpy(tempname,user->name,12);
		  sstrncpy(temppass,passwd,10);
		  strtolower(tempname); strtolower(temppass);
		  if (strstr(temppass, tempname)) passwd_simplex=2;
			  else passwd_simplex=check_passwd_simplex(passwd);
                  connect_user(user);  return;
                  }
            write_user(user,"\n\nNespravne heslo - prihlasenie zlyhalo.\n\n");
            sprintf(text,"Incorrect login (%s) from %s\n",user->name, user->site);
            write_syslog(text,1);
            attempts(user);
            }
      return;

      case 1:
      temp=inpstr;
      i=0;
      while(*temp!='\0' && *temp!='\n' && *temp!=' ' && i<PASS_LEN+1) {
      	passwd[i++]=*temp++;      	
      	}
      passwd[i]='\0';
      if (strcmp(user->pass,(char *)md5_crypt(expand_password(passwd),user->name))) {
	    write_user(user,"\n\nHesla nesuhlasia, skus to znovu.\n\n");
	    attempts(user);
	    return;
	    }
      /* Novy user... NOVYUSER*/
      /* ?? .. site */
      strcpy(user->desc,"nema nastaveny .desc");
      strcpy(user->in_phrase,"vstupil(a) sem");
      strcpy(user->out_phrase,"nas opustil(a)");
      strcpy(user->email,"Nema ziadny email");
      strcpy(user->homepage,"Nema ziadnu homepage");
      strcpy(user->reserved,"-");
      strcpy(user->requestemail,"N/A");
      strcpy(user->wizpass,"-");
      user->sex=2; /* Standardne muz(1), ale 2 aby sme vedeli ze si este nezmenil pohlavie;) */
      user->pridavacas=1;
      
      user->ignlook=0;
      user->type=USER_TYPE;
      user->buffpos=0;
      user->filepos=0;
      user->vis=1;
      user->ignall=0;
      user->misc_op=0;
      user->edit_line=0;
      user->charcnt=0;
      user->warned=0;
      user->accreq=0;
      user->last_login_len=0;
      user->ignall_store=0;
      user->clone_hear=0;
      user->afk=0;
      user->edit_op=0;
      user->ignore=0;
      user->last_input=time(0);
      user->last_login=time(0);
      user->read_mail=time(0);
      user->malloc_start=NULL;
      user->malloc_end=NULL;
      user->lasttellfrom[0]='\0';
      user->lasttellto[0]='\0';
      user->lasttg=-1;
      user->follow[0]='\0';
      user->buff[0]='\0';
      user->mail_to[0]='\0';
      user->page_file[0]='\0';
      user->inpstr_old[0]='\0';
      user->afk_mesg[0]='\0';
    
    /* tieto 3 r. vzdy nexat */  
      user->lastcommand[0]='\0';
      user->last_site[0]='\0';
      user->total_login=0;
      user->idletime=0;
      user->first_login=time(0);
     
     /* tieto 2 nemusia byt */   
     user->room=NULL;
     user->invite_room=NULL;
    
      
      user->level=0;
      user->muzzled=0;
      user->muzzletime=0;
      user->muzzle_t=-1;
      user->command_mode=0;
      user->prompt=prompt_def;
      user->colour=2;      /* !!!!!!!!!!!!!!!!!!!!!!! */
      /* user->colour=1; */             /* UFF */
      user->jailed=0;
      user->charmode_echo=charecho_def;
      user->pp=MAXPP; /*PP*/
      user->mana=0; /*SPELL*/
      user->newpass[0]='\0';
      user->chanp[0]='\0';
      user->has_ignall=0;
      user->alarm=-1;
      
      user->uname[0]='\0';
      user->game=0;
      user->joined=0;
      user->moves=1;
      user->last_x=0;
      user->last_y=0;
      user->tah=0;      
      
      user->hang_word[0]='\0';
      user->hang_word_show[0]='\0';
      user->hang_guess[0]='\0';
      user->hang_stage=-1;      
      user->lines=24;
      user->colms=80;
      user->wrap=0;
      user->who_type=1;
      user->rt_on_exit=0;      
      for (i=0;i<HANDS;i++) user->predmet[i]=-1;
      for (i=0;i<HANDS;i++) user->dur[i]=0;
      user->zuje=0;
      user->call[0]='\0';    
      user->glue=0;
      user->prehana=0;
      user->zuje_t=0;
      user->lieta=0;
      user->viscount=0;
      user->doom=0; /* D00M */
      user->doom_energy=0;
      user->doom_score=0;
      user->doom_ammo[0]=0;
      user->doom_ammo[1]=D_AMMO;
      user->doom_ammo[2]=1;      
      user->doom_weapon=0;
      user->doom_loading=0;
      user->doom_heading=0;
      user->doom_x=0;
      user->doom_y=0;
      user->doom_sayon=0;
      
      user->shmode=0;
      user->shbuild=0;
      user->shstatus=0;
 
      user->newtell=0;
      user->browsing=0;
      user->messnum=0;
      user->lab=0;
      user->afro=0;
      user->real_ident[0]='\0';
      user->remote_ident[0]='\0';
      user->subject[0]='\0';
      user->revline=0; /* sem som TERAZ pridal! */
      user->revircline=0;
               
      for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
      for (i=0; i<10; i++) user->channels[i][0]='\0';      
 	
      user->lastm=0;
      user->autofwd=0;      
      user->pagewho=1;
      user->examine=1;
      
      user->com_counter=MAX_COMMANDS;
      user->igngossip=1;
      user->quest=0;
      user->macrolist=NULL; /*M@CRO*/
      user->notifylist=NULL;
      user->ignorelist=NULL;
      user->combanlist=NULL;
      user->pager=NULL;
     
      user->flood=0;
      user->dead=0;
      user->prompt_string[0]='\0';
      user->rjoke_from=1;
      user->ignportal=0;
      user->ignio=0;
      user->ignword[0]='\0';
      user->saturate=0;
      user->idle=0;
      user->autosave=AUTOSAVE;
      for(i=0;i<REVTELL_LINES;i++) user->revbuff[i]=NULL;      
      for(i=0;i<HISTORY_LINES;i++) user->history[i][0]='\0';
      for(i=0;i<REVIRC_LINES;i++) user->revirc[i]=NULL;            
      user->age=0;
      user->agecode=0;
      user->histpos=0;
      user->lang=0;
      
      /* REM0TE */
      user->remote_login=0;
      user->remote_login_socket=0;
      user->actual_remote_socket=0;
      user->remote_name[0]='\0';
      user->remote_passwd[0]='\0';
      for(i=0;i<MAX_CONNECTIONS;++i) {
      	user->remote_socket[i]=0;
        user->remote[i]=NULL;
        user->remote_fds[i]=0;
        }
      user->ircsocknum=0;
      user->akl=0;
      user->cloak=0;
      user->irc_chan[0]='\0';
      user->lynx=0;
      user->irc_nick[0]='\0';
      user->irc_defnick[0]='\0';
      user->sayflood=0;
      user->irc_reg=0;
      user->irc_name[0]='\0';
      user->irc_serv[0]='\0';
      user->statline=NONEMODE;
      user->statlcount=0;
      user->statlcan=0;
      user->newline=0;
      user->bj_game=NULL; /*BJACK*/
      user->remtelopt=0;
      reversi_koniechry(user);
      user->dama=NULL;
      user->dama_opp=NULL;
      user->dhrac=0;
      user->miny_tab=NULL;
      user->ignbeep=0;
      user->ignzvery=0;
      user->ignportalis=1;
      user->ignafktell=1;
      user->igngames=0;
      user->ignautopromote=0;
      user->mailnotify=0;
      user->chrac=0;
      user->clovece=NULL;
      user->gold=0; /* qoli klonom zaciname od 0 a peniaze az od Vojaka */
      user->shoutswears=0;
      user->sayswears=0;
      user->ignsys=1;
      user->shortcutwarning=0;
      user->switches=0;
      /* user->ipcka[0]='\0'; */
      echo_on(user);
      
      /* USERSTRUCT */                         
      user->id=max_id;
      max_id++;
      save_user_details(user,1); 
      sprintf(text,"New user \"%s\" created.\n",user->name); 
      write_syslog(text,1); 
      newusers+=1;      
      connect_user(user); 
      } 
}

/*STRCPYEND*/
/*** Count up attempts made by user to login ***/
void attempts(user)
UR_OBJECT user;
{
user->attempts++;
if (user->attempts==5) { /* Bude 5 pokusov... ;) */
        if (user->login==2) {
        	sprintf(text,"~OL~FY~LIPOZOR~RS - ~OL~FGniekto sa pokusal (neuspesne) prihlasit na Tvoje konto!~FW~BK~RS\n~FGUtok prichadzal z adresy: ~OL%s\n",user->site);
        	send_mail(user,user->name,text);
		}
      write_user(user,"\nPrilis vela neplatnych pokusov o prihlasenie, dovidenia!\n\n");
      disconnect_user(user, 3,NULL);
      /* ZMENA - Vystraha, ak niekto na posledny pokus napisal zle heslo! */    
      return;
      }
user->login=3;
user->pass[0]='\0';
if (user->port==port[1]) write_user(user,"Wizport login: ");
	else write_user(user,"Tvoja prezyvka: ");
echo_on(user);
}

/* Tuto je endstring co by to mal vylepsit ;) ;) */
void endstring(inpstr)
char *inpstr;
{
unsigned int i;
for (i=0;i<strlen(inpstr);i++)
	if (inpstr[i]=='\n') {
		inpstr[i]=0;
		return;
		}
}

                                                       
/*** Load the users details ***/
int load_user_details(user)
UR_OBJECT user;
{
 return db_load_user_details(user,0); /* MySQL */
}

/*** Save a users stats (S)-ochrana proti stratam userfilov ***/
int save_user_details(UR_OBJECT user,int save_current)
{
 return db_save_user_details(user,save_current); /* mysql */
}

/*** Connect the user to the talker proper ***/
void connect_user(user)    /*QUQUQU*/
UR_OBJECT user;
{
UR_OBJECT u,u2;
RM_OBJECT rm;
/* char temp[100]; */
int user_counter;
/* int ilist; */
char filename[100];
char newbie[30];
/* struct stat statbuf; */
int mailboxsize=0;
char smalbuf[4];
int i;

newbie[0]='\0';

/* See if user already connected */
for(u=user_first;u!=NULL;u=u->next) {
      if (user!=u && user->type!=CLONE_TYPE && !strcmp(user->name,u->name)) {
            if (u->multiply==1) { u->multiply=0; continue; }
	    rm=u->room;
	    sprintf(text,"\n\nUz si raz %s z %s, rusim predchadzajuce spojenie ...\n", pohl(user,"prihlaseny","prihlasena"),u->site);
	    write_user(user,text);
	    sprintf(text,"\n\n~OLPrihlasil%s si sa znova z %s. Rusim spojenie ...\n", pohl(u,"","a"),user->site);
	    write_user(u,text);
	    if (u->statline==CHARMODE) {
	      u->statline=NONEMODE;
              write2sock(NULL,user->socket,"\033[?25h\033c\033[?7h",0);
              write2sock(NULL,u->socket,"\033[?25h\033c\033[?7h",0);
              u->lines=u->statlcount;
	      u->statlcount=0;
	      }
            if (u->socket>1000) eliminate_webuser(u->socket);
  	    else close(u->socket);
	    u->socket=user->socket;
            if (user->socket<1000) u->fds=user->fds;
	    strcpy(u->site,user->site);
	    u->site_port=user->site_port;

	    if (!zistuj_identitu || user->socket>1000) strcpy(u->real_ident,"(off)");
	    else {
	      strcpy(u->real_ident,"(none)");
	      cmon_auth(u);
	      sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_NEW_ENVIRON);
	      write2sock(user,user->socket,smalbuf,0);
	      u->remote_ident[0]='\0';
      	      for (i=0;i<MAX_CONNECTIONS;i++)
	        if (u->remote_socket[i]!=0 && u->remote_socket[i]!=u->ircsocknum) {
	          sprintf(smalbuf,"%c%c%c",IAC,WILL,TELOPT_NEW_ENVIRON);
	          write2sock(u,u->remote_socket[i],smalbuf,0);
	        }
	      }
            destruct_user(user);
            num_of_logins--;
            sprintf(text,"~OLRELOG:~RS %s %s\n",u->name,u->desc);
            if (u->vis) writesys(0,1,text,u);
            sprintf(text,"%s swapped session (%s)\n",u->name, u->site);
            write_syslog(text,1);
            if (!u->ignlook) look(u);
            if (u->level>=KIN && u->wizactivity>20) {
              u->wizactivity--;
              write_user(u,"~OL~FRZadaj svoje sluzobne heslo!\n");
             }
            else prompt(u);
	          	
            /* Reset the sockets on any clones */
            for(u2=user_first;u2!=NULL;u2=u2->next) {
                  if (u2->type==CLONE_TYPE && u2->owner==user) {
                        u2->socket=u->socket;  u->owner=u;
                        }
                  }
            return;
            }
      }

/* M@CRO>> */
user->macrolist = NULL;
user->macro_num=load_macros(&(user->macrolist),user->id);
/* >>M@CRO */

user->notifylist=NULL;
user->notify_num=load_notifylist(&(user->notifylist),user->id,1);

user->ignorelist=NULL;
load_notifylist(&(user->ignorelist),user->id,0);

user->combanlist=NULL;
load_notifylist(&(user->combanlist),user->id,2);

load_irc_details(user);

user->special=db_user_switch(user->id,9);

if (user->room==NULL) user->room=room_first;

if (!strcmp(user->room->name,"afroditin_chram")) user->room=get_room("namestie",NULL);
if (!strcmp(user->room->name,"tunel")) user->room=get_room("ostrov",NULL);
if (!strcmp(user->room->name,FLYER_ROOM)) user->room=get_room(FLYER_HOME,NULL);

if (user->room->access==GOD_PRIVATE || 
   ((user->room->access==FIXED_PRIVATE || user->room->access==PRIVATE)
    && (user->level<WIZ && !user->special))) {
	if (user->room->group==1) user->room=get_room("namestie",NULL);
	if (user->room->group==2 || user->room->group==3) user->room=get_room("ostrov",NULL);
	if (user->room->group==4) user->room=get_room("obchod",NULL);
	}

if ((!user->jailed)&&(user->room==get_room("zalar",NULL)))
   user->room=room_first;
if (user->jailed)
   if ((user->room=get_room("zalar",NULL))==NULL)
      user->room=room_first;

/* Je to guest ? */
if (!strcmp(user->name,"Guest") && guest.name[0]!='\0') {
	/* ejha! */
	sprintf(user->desc, "~OL~FW%s", guest.name);
	if ((rm=get_room(GUEST_ROOM,NULL))==NULL) user->room=get_room("namestie",NULL);
		else user->room=rm;
	guest.on=1;
	user->room->sndproof=1;
	guest.user=user;
	if (guest.moderator!=NULL) move_user(guest.moderator, user->room, 3);
	}

if (user->vis) {
	if (strcmp(user->room->name,ARENA_ROOM)) {  /* Ak nie je v arene, tak zobraz SIGN ON: */
		if (!user->level) { strcpy(newbie,"~FTNOVACIK!~FM"); wrtype=WR_NEWBIE; }
	  	else strcpy(newbie,user->room->name);
		if (user->level>KIN) sprintf(text,"~OL~FMVynara sa z hlbin cyberspace:~FY %s~FM %s ~RS\n",user->name,user->desc);
	 	else
	  	sprintf(text,"~OL~FMLOGIN: %s %s ~RS~OL~FM(%s)~RS\n",user->name,user->desc,user->room->name);
		writesys(PRI,0,text,NULL);
		if (user->level>KIN) sprintf(text,"~OL~FMVynara sa z hlbin cyberspace:~FY %s~FM %s ~RS\n",user->name,user->desc);
		else
		sprintf(text,"~OL~FMLOGIN: %s %s ~RS~OL~FM(%s) ~RS~FT(%s)\n",user->name,user->desc,newbie,user->site);
		writesys(WIZ,1,text,NULL);
		wrtype=0;
	}
	else {  /* Inak vypis iba v arene a ostatnym daj pokoj */
	        sprintf(text,"~OL~FM%s DO ARENY: %s %s ~RS\n",pohl(user,"VBEHOL","VBEHLA"),user->name,user->desc);
		write_room_except(user->room,text,user);
             }
   }
	     	     		
num_of_users++;
num_of_logins--;
user->login=0;

if (!zistuj_identitu) strcpy(user->real_ident,"(off)");
	else {
	strcpy(user->real_ident,"(none)");
	cmon_auth(user);
        sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_NEW_ENVIRON);
        write2sock(user,user->socket,smalbuf,0);
	}

if (logcommands) log_commands(user->name,"LOGIN",1);

/* checkni ci niekto usera nema v notify liste: */
if (user->vis && user->port==port[0]) check_notify(user);

if (!strcmp(user->name,"Poseidon")) { user->colour=0; user->ignfun=1; }
if (!strcmp(user->name,"Smsbot")) { user->ignfun=1; }

/* send post-login message and other logon stuff to user */
if (user->wrap) write_user(user,"\033[?7h");  /* Tuto posiela esc na wrap lines!!! */
write_user(user,"\n");

/*
if (user->last_site[0]) {
      sprintf(temp,"%s\n",zobraz_datum((time_t *)&user->last_login, 1));
      temp[strlen(temp)-1]=0;
      sprintf(text,"\nVitaj ~OL%s~RS ...\nNaposledy si %s dna %s z %s\n",user->name,pohl(user,"tu bol","tu bola"),temp,user->last_site);
      }
else sprintf(text,"\nVitaj ~OL%s~RS ...\n",user->name);
write_user(user,text);
*/

sprintf(text,"~OL~FB+~RS~FB-~OL~FY Vitaj v Atlantide, %s! ~RS~FB", user->name);
for(i=12-strlen(user->name); i; i--) chrcat(text,'-');
strcat(text,"------------~OL+~RS~FB------------------------------~OL+\n");
write_user(user,text);
vwrite_user(user,"~FB|~FW Uroven: ~FT%-7.7s ~FB|~FW Momentalne online: ~FT%-3d ~FWludi ~FB|~FW Posledne prihlasenie:        ~FB|\n", level_name[user->level], num_of_users);
vwrite_user(user,"~FB|~FW Notify: ~FT%-3d~FWludi ~FB|~FW Vtipov na jokeboarde: ~FT%-5d ~FB|~FT %-28.28s ~FB|\n",user->notify_num, pocet_vtipov, zobraz_datum((time_t *)&user->last_login, 3)); 
vwrite_user(user,"~FB|~FW Makier: ~FT%-4d    ~FB|~FW Miestnost: ~FT%-16.16s ~FB|~FT %-28.28s ~FB|\n",user->macro_num, user->room->name, user->last_site);
write_user(user,"~OL~FB+~RS~FB-----------------~OL+~RS~FB-----------------------------~OL+~RS~FB------------------------------~OL+\n");  
      
if ((user_counter=counter_db(1))!=0)
{
	sprintf(text,"Si navstevnikom cislo %d, gratulujeme!\n",user_counter);
	user->visitor=user_counter;
}
 else sprintf(text,"Vznikla chyba pri inicializovani pocitadla!!!\n");
write_user(user,text);
/* write_user(user,"\033]0;ATLANTIS Talker\007\r"); */ /* Titulka :) */
sprintf(text,"%c%c%c",IAC,DO,TELOPT_NAWS);  /* posli suradnice terminalu */
write_user(user,text);

user->last_login=time(0); /* set to now */

/* sprintf(text,"~FTTvoj level je~OL~FW %s~RS~FT, ",level_name[user->level]); */
/* write_user(user,text); */

/*
switch(nlist) {
     case 0: write_user(user,"~FTa mas prazdny notify zoznam.\n"); break;
     case 1: write_user(user,"~FTa mas ~OL~FWjedneho~RS~FT cloveka v notify zozname.\n"); break;
     default: sprintf(text,"~FTa mas ~OL~FW%d~RS~FT ludi v notify zozname.\n",nlist);
     	    write_user(user,text);
     	    break;
	}
*/

if (!user->ignlook) look(user);

/* ZMENA - sem pridam vypis poctu vtipov na JokeBoard... */

/*
if (user->level) {
	sprintf(text,"~FWJokeBoard ");
	switch(pocet_vtipov) {
		case 0: sprintf(temp,"neobsahuje ziaden vtip. ;(\n");
			break;
		case 1: sprintf(temp,"obsahuje ~OL1~RS~FW vtip.\n");
			break;
		case 2:
		case 3:
		case 4: sprintf(temp,"obsahuje ~OL%d~RS~FW vtipy.\n",pocet_vtipov);
			break;
		default: sprintf(temp,"obsahuje ~OL%d~RS~FW vtipov.\n",pocet_vtipov);
			break;
		}
	strcat(text,temp);
	write_user(user,text);
     }
*/
/* ------------ end of jokes ------------- */

if ((user->level) && (user->notify_num)) who_from_notify_is_online(user);

if ((user->level>=KIN) && (minlogin_level!=-1)) {
	write_user(user,"~OL~FRPozor: je nastaveny minlogin!\n");
	}

if (user->level) hint(user, 1);	

if (user->level) {
	sprintf(filename,"select body from files where filename='%s'",motd2_file);
	switch(sqlmore(user,user->socket,filename)) {   /* ZMENA !!! */
	              case 0: break;
	              case 1: user->misc_op=222;
	              }   	                   
	if (user->jailed) {
          sprintf(text,"                      ~OL~FRS I   U V A Z N E N %s   V   Z A L A R I !~RS\n",pohl(user,"Y","A"));
          write_user(user,text);
         }
	if (user->sex==2) write_user(user,"~OL~FYNASTAV SI PROSIM POHLAVIE PRIKAZOM ~FT.set gender muz~FY ALEBO ~FT.set gender zena~RS~FW\n"); 
	     }

/* write to syslog and set up some vars */
sprintf(text,"%s %s logged in from %s (#%d)\n",level_name[user->level],user->name,user->site,user_counter);
write_syslog(text,1);

if (num_of_users>max_users_was) max_users_was=num_of_users;
all_logins++;
/* ZMENA - zistuje dvojitu identitu... ;)) */

if (check_double_identity(user)!=NULL && (resolve_ip)) {
                  sprintf(text,"~FRPOZOR: ~OL%s~RS~FR ma asi dvojitu identitu (~OL%s~RS~FR). Skontroluj to!~RS\n",user->name,check_double_identity(user)->name);
		  writesys(WIZ,1,text,NULL);
                  }

/*
if ((zistuj_identitu) && (check_double_real_identity(user)!=NULL)) {
                  sprintf(text,"~FRPOZOR: ~OL%s~RS~FR ma dvojitu REALNU identitu (~OL%s~RS~FR) - %s~RS\n",user->name,check_double_real_identity(user)->name,user->real_ident);
		    writesys(WIZ,1,text,NULL);
                  }
*/

if ((has_unread_mail(user)) && (user->level)) {
  write_user(user,"~LB~FT~OL** ~LIMAS NOVU POSTU!~RS~OL~FT (precitas prikazom ~FY.rmail~FT) **~RS\n");
 }


mailboxsize=0;
sprintf(query,"select sum(length(message)) from mailbox where userid='%d'",user->id);
mailboxsize=query_to_int(query);
i=db_user_switch(user->id,2); /* no quota userlist */
if (mailboxsize>MAIL_LIMIT*(1+i*20)) {
  write_user(user,"~OL~FRPOZOR: ~FYTvoja postova schranka je plna, vycisti si ju prikazom .dmail!\n");
 }
else if (mailboxsize>MAIL_QUOTA*(1+i*20)) {
  write_user(user,"~OL~FRPOZOR: ~FYMas preplnenu postovu schranku; pokial si ju nevycistis (prikaz .dmail)\n       ~OL~FYnebudes moct dostavat dalsiu postu!\n");
 }

/*sprintf(filename,"%s/%s.M",MAILFILES,user->name);
if (!stat(filename,&statbuf) && quotacheck(user->name)) {
     if ((statbuf.st_size>MAIL_QUOTA) && (statbuf.st_size<=MAIL_LIMIT)) {
     		 write_user(user,"~OL~FRPOZOR: ~FYTvoja postova schranka je plna, vycisti si ju prikazom .dmail!\n");
     		 }
     if (statbuf.st_size>MAIL_LIMIT) {    
     		 write_user(user,"~OL~FRPOZOR: ~FYMas preplnenu postovu schranku; pokial si ju nevycistis (prikaz .dmail)\n       ~OL~FYnebudes moct dostavat dalsiu postu!\n");
     		 }      		 
	}
*/
if (ismacro(&(user->macrolist),NULL,"auto")) /* Automacro!!! (R) */
    {
     user->com_counter=MAX_COMMANDS;
     parse(user,"auto",0);
    }

if (user==guest.user && guest.on) {
	sprintf(text,"~OL~FY%s, vitame Ta v Atlantide!\n", guest.name);
	write_room(user->room, text);		
	}

word_count=0;    
who_for_web();    

if (user->level>=KIN && user->wizactivity>0) {
  police_freeze(user,0);
  return;
 }
if (passwd_simplex && user->level>=WIZ && !strstr(user->desc,"--!--")) {
	write_user(user,"\n~OL~FROd levelu CARODEJ musis pouzivat zlozitejsie heslo - aspon 1 nepismenovy znak,\n~OL~FRdlzka aspon 6 znakov a ziadne slovo zo slovnika ...\n");
	change_pass(user);
	}
	/*else if (passwd_simplex==2 && user->level>=CIT) {
		write_user(user,"\n~OL~FRZmen si heslo - nesmie obsahovat tvoj nick!\n");
		change_pass(user);
		}
	*/
	else prompt(user);
 user->ap=arena_check(user);
 sprintf(query,"update `users` set `online`=1 where `id`='%d'",user->id);
 mysql_kvery(query);
 
 
 if (april) {
   if (april & 2) user->cloak=1;
   if (april & 4) {
     user->cloak=1+rand()%8;
     if (user->cloak==6) user->cloak=8;
    }
   if (april & 8) {
     if (user->level>PRI) user->cloak=1;
     else user->cloak=8;
    }
  }
}

/* checkuje, ci nejde uzivatel z rovnakej IP adresy/servera a rovn. portu */
UR_OBJECT check_double_identity(user)
UR_OBJECT user;
{
static UR_OBJECT u;

if (isalpha(user->site[0])) return NULL;  
for(u=user_first;u!=NULL;u=u->next) {  
	if (u->type!=USER_TYPE || u->login || u->room==NULL || u==user) continue;
	if (!strcmp(u->site,user->site))  return u;
	}
return NULL;
}

/* Zistuje ci niekto nejde z rovnakeho servera a aj konta */
UR_OBJECT check_double_real_identity(user)
UR_OBJECT user;
{
UR_OBJECT u;

  
if (!strcmp(user->real_ident,"(off)") || !strncmp(user->real_ident,"<",1)) return NULL;
for(u=user_first;u!=NULL;u=u->next) {  
	if (u->type!=USER_TYPE || u->login || u->room==NULL || u==user) continue;  
	if (!strcmp(u->real_ident,user->real_ident))  return u;  
	}
return NULL;
}

/* checkuje prihlasovanie userov v notify liste */
int check_notify(user)
UR_OBJECT user;
{
UR_OBJECT u;

for(u=user_first;u!=NULL;u=u->next) {
	if (u->type!=USER_TYPE || u->login || u->room==NULL || u->ignall || u==user) continue;	
	if (check_notify_user(u,user)) {
		sprintf(text,"~LB~OL~FG~LI%s~RS~OL~FG sa ti prave %s!\n",user->name,pohl(user,"prihlasil","prihlasila"));
	        write_user(u,text);
		}
	}
return 0;
}

/*** Get user struct pointer from name ***/
UR_OBJECT get_user(name)    /* tuto bug so zlym rozpoznanim userov ?! */
char *name;
{
UR_OBJECT u;

if (!name) return NULL;
name[0]=toupper(name[0]);
/* Search for exact name */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->login || u->type!=USER_TYPE) continue;
      if (!strcasecmp(u->name,name)) {
      	if (u==guest.user && !guest.getuser) return NULL;
      		else { strcpy(name,u->name); return u; }
      	}
      }
/*
if (com_num==TELL) for(u=user_first;u!=NULL;u=u->next) {
      if (u->login || u->type!=USER_TYPE) continue;
      if (!strcasecmp(sklonuj(u,2),name) || !strcasecmp(sklonuj(u,3),name) || !strcasecmp(sklonuj(u,4),name)) {
      	if (u==guest.user && !guest.getuser) return NULL;
  	else { strcpy(name,u->name); return u; }
      	}
      }
*/
/* Search for close match name */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->login || u->type!=USER_TYPE) continue;
      if (!strncasecmp(u->name,name,strlen(name)))  {
      	if (u==guest.user && !guest.getuser) return NULL;
        else { strcpy(name,u->name); return u; }
      	}
      }
return NULL;
}

UR_OBJECT get_user_by_sock(int sock)
{
UR_OBJECT u;

if (sock<1) return NULL;
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login || u->type!=USER_TYPE) continue;
  if (u->socket==sock) return u;
 }
return NULL;
}

UR_OBJECT get_user_in_room(char *name,UR_OBJECT user)
{
UR_OBJECT u;

if (!name) return NULL;
name[0]=toupper(name[0]);

for(u=user_first;u!=NULL;u=u->next) {
      if (u==user || u->login || u->type!=USER_TYPE || u->room!=user->room) continue;
      if (!strcasecmp(u->name,name)) {
      	if (u==guest.user && !guest.getuser) return NULL;
      		else { strcpy(name,u->name); return u; }
      	}
      }

for(u=user_first;u!=NULL;u=u->next) {
      if (u==user || u->login || u->type!=USER_TYPE || u->room!=user->room) continue;
      if (!strncasecmp(u->name,name,strlen(name)))  {
      	if (u==guest.user && !guest.getuser) return NULL;
      		else { strcpy(name,u->name); return u; }
      	}
      }
return NULL;
}

/*** Disconnect user from talker ***/

void disconnect_user(user,message,dovod1) /*arriba*/
UR_OBJECT user;
int message;
char *dovod1;
{
RM_OBJECT rm;
UR_OBJECT u2,u;
int mins,minutes,hours,days, i,y,pocet=0;
FILE *fp;
char tempfilename[80];
int nukehim=0;
int status;
char *dovod;
char *name;

if (dovod1!=NULL) {
	dovod=(char*)malloc((strlen(dovod1)*sizeof(char))+1);
	strcpy(dovod, dovod1);
	}
	else dovod=NULL;
if (user->level==0 && !strcmp(user->email,"Nema ziadny email")) nukehim=1; 
if (!strcmp(user->name,"Poseidon")) { user->colour=0; /* user->level=9; */}

  dump_user_data(user);

rm=user->room;
if (user->login) {
      if (user->socket>1000) eliminate_webuser(user->socket);
      else {
        close(user->socket);
        user->socket=0;
        user->fds=0;
        UPDATE_FDS();
       }
      destruct_user(user);
      num_of_logins--;
      if (dovod!=NULL) free(dovod);
      return;
      }
      
      /* set_goto_room(user); */      
      user->commused[9]++;
      if (!nukehim) save_user_details(user,8);
      if (user->hangups>0 || user->hangupz>0)
       sprintf(text,"%s %s logged out with %d/%d wtos.\n",level_name[user->level],user->name,user->hangups,user->hangupz);
      else
       sprintf(text,"%s %s logged out.\n",level_name[user->level],user->name);
      write_syslog(text,1);

/* M@CRO>> netreba, macra sa pridavaju do DB on the fly (V) */
/*sprintf(tempfilename,"%s/%s.MC",MACRODIR,user->name);
if (user->macrolist) save_macros(&(user->macrolist),tempfilename);
	else { 
		if ((fp=ropen(tempfilename,"r"))!=NULL) {
			fclose(fp);
			unlink(tempfilename);
			}	
		}
*/		
free_macrolist(&(user->macrolist));
/* >>M@CRO */

zrus_pager_haldu(user);

if (user->lynx) {
/*	#ifndef WIN32 */
/*	siginterrupt(SIGKILL,1); */ /* CO MU OPEROVALI?! */
/*	#endif */
	kill(-user->lynx,9);
	waitpid(user->lynx, &status, 0);
        }
        
sprintf(tempfilename,"%s%s-lynx%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(text,"r"))!=NULL) { fclose(fp); deltempfile(tempfilename); }

if (user->hang_stage!=-1) {
  add_point(user,DB_HANGMAN,-1,0);
 }

miny_done(user);
if (user->dhrac>0) {
	if (/*talkershutd==0*/ 0 ) {
		word_count=2;
	    	strcpy(word[1],"stop");
	    	dama(user);
	   	}
	else {
		word_count=2;
	    	strcpy(word[1],"save");
	    	dama(user);
	    	sprintf(text,"~FTNiekto drgol do stola a rozsypali sa vam vsetky figurky.\n");
    	    	write_user(user,text); 
	    	write_user(user->dama->hrac[2-user->dhrac],text);
    	    	user->dama->hrac[2-user->dhrac]->dama=NULL;
    	    	user->dama->hrac[2-user->dhrac]->dhrac=0;
            	destruct_dama(user);
	    	user->dhrac=0;
	   	}    
}

if (user->chrac>0) {
	word_count=2;
        strcpy(word[1],"koniec");
        clovece(user, NULL);
        }

if (user->fhrac>-1) {
	word_count=2;
        strcpy(word[1],"koniec");
        farar(user, NULL);
        }

if (user->xannel!=NULL) {
	word_count=2;
        strcpy(word[1],"leave");
        xannel(user,"");
        }

if ((user->game==2) || (user->game==3)) {
	if (!(u=get_user(user->uname))) {		
		}
	   else {	   
	   	if ((user->game==2) && (user->moves>2)) {
                        add_point(u,DB_PISKVORKY,1,0);
                        add_point(user,DB_PISKVORKY,-1,0);
		   	u->game=0;
		   	u->joined=0;
		   	u->moves=0;
		   	write_user(u,"Spoluhrac sa ti odhlasil a tym padom prehral!\n");
		   	}
		if ((user->game==3) || ((user->game==2) && (user->moves<=2))) {
			write_user(u,"Spoluhrac ti odisiel.\n");
			u->game=0;
			u->joined=0;
			u->moves=0;
			}
	   	}
	}

if (user->reversi_cislotahu) {
    /* hra reversi */
    write_user(user->reversi_sh,"Tvoj spoluhrac v reversi sa odhlasil.\n");
    reversi_koniechry(user->reversi_sh);
    reversi_koniechry(user);
}

	
if (user->doom) doom_quit(user);

eject(user);

if (user->vis) name=user->name; else name=invisname(user);
/* strcpy(text,""); */
if (user->lieta>1 && is_free_in_hands(user)>-1 && (dovod==NULL || (dovod!=NULL && strcmp(dovod1,"~FRpad do mora"))) && !user->dead)
 put_in_hands(user,expand_predmet("kridla"),user->lieta-1);
text[0]='\0';
for(i=0;i<BODY;i++)
 if (user->predmet[i]>-1 && is_free_in_room(user->room)>-1) {
   sprintf(text,"%s tu %s ",name,pohl(user,"nechal","nechala"));
   break;
  }
if (text[0]) {
  for(i=0;i<BODY;i++)
   if (user->predmet[i]>-1)
    {
     if (is_free_in_room(user->room)>-1) {
      put_in_room(user->room,user->predmet[i],user->dur[i]);
      strcat(text,predmet[user->predmet[i]]->akuzativ);
      pocet=0;
      for (y=i+1;y<BODY;y++)
       if (user->predmet[y]>-1) pocet++;
      if (pocet>is_free_in_room(user->room)) pocet=is_free_in_room(user->room);
      if (pocet>1) strcat(text,", ");
      else if (pocet==1) strcat(text," a ");
      else if (pocet==0) strcat(text,".\n");
     }
    }
  write_room_except(user->room,text,user);
 }

/* GUEEST */
if (guest.on && user==guest.user) {
	if ((rm=get_room(GUEST_ROOM,NULL))!=NULL) {
		sprintf(text,"~OLHost sa nam odhlasil ...\n");
		write_room(rm,text);		
		}
	guest.user=NULL;
	}
	
if (user==guest.moderator) {
	if ((rm=get_room(GUEST_ROOM,NULL))!=NULL) {
		sprintf(text,"~OLModerator nam vypadol!\n");
		write_room(rm,text);		
		}
	guest.on=0;
	guest.name[0]='\0';
	guest.moderator=NULL;	
	guest.talk=NULL;
	guest.user=NULL;
	}

if (user==guest.talk && guest.on && guest.moderator!=NULL) {
	sprintf(text,"~OL%s %s slovo a %s sa, udel slovo niekomu inemu!\n", user->name, pohl(user,"mal","mala"), pohl(user,"odhlasil","odhlasila"));
	write_user(guest.moderator,text);
	guest.talk=NULL;
	/* return; */
	}

/* REM0TE */
for (i=0;i<MAX_CONNECTIONS;i++)
	if (user->remote_socket[i]!=0) {
		sprintf (text,"~FT[Odpajam: ~OL%s~RS~FT]\n",user->remote[i]->desc);
		write_user(user,text);
		remote_close(user,i);
     	 	}
     	 	
if (user->rt_on_exit&8) send_recent_tells(user,0);

if (!message && !nukehim) write_user(user,"\n~OL~FBDovidenia nabuduce!\n\n");
 else 
   if (nukehim) {
     if (user->dead!=666) 
       write_user(user,"\n~OL~FRNepodal(a) si si .request, nick bol zruseny!\n\n");
    }
     else {
	mins=(int)(time(0) - user->last_login)/60;
	days=user->total_login/86400;
	hours=(user->total_login%86400)/3600;
	minutes=(user->total_login%3600)/60;

	if (user->statline==CHARMODE) write2sock(user,user->socket,"\033[?25h\033c\033[?7h",0);

        if (user->rt_on_exit&1) revtell(user,""); /*TOSLOV :-) */
	if (user->newtell) {
		/* if(user->rt_on_exit&4 && user->rt_on_exit&2) send_recent_tells(user,2); */
		if(user->rt_on_exit&4) send_recent_tells(user,1);
		if(user->rt_on_exit&2 && !(user->rt_on_exit&8)) send_recent_tells(user,0);
	 }

	sprintf(text,"\n~OL~FB.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._~RS\n");
	write_user(user,text);
	  sprintf(text,"~OL~FT   Odhlasuje sa ~FW~BB%s %s~RS~FT~OL dna ~BB~FW%s~RS~FT~OL miestneho casu.~RS\n",level_name[user->level], user->name,long_date(3));
	writecent(user,text);
	  sprintf(text,"~OL~FT   %s si tu ~BB~FW%d~RS~FT~OL minut.",pohl(user,"Stravil","Stravila"),mins);

	  if (user->muzzled && !user->jailed) strcat(text,"Mas nahubok.~RS");
		else if (user->jailed && !user->muzzled) strcat(text,"Si v base.~RS");
	  if (user->jailed && user->muzzled) strcat(text,"Mas nahubok a si v base.~RS");
	  strcat(text,"\n");

	writecent(user,text);
	  sprintf(text,"~OL~FTCelkovo mas na Atlantise odvisenych ~FW~BB%d~RS~FT~OL dni, ~FW~BB%d~RS~FT~OL hodin a ~FW~BB%d~RS~FT~OL minut.~RS\n",days,hours,minutes);
	writecent(user,text);
	write_user(user,"~FR--------------------------------------------------------------------------------~RS\n");
	if(user->socket>1000) {
	  sprintf(text,"~FWToto bol online rozhovor na talkri ~OL%s ~RS~FW- %s, %s~RS\n",TALKER_WEB_SITE,TALKER_CITY,TALKER_COUNTRY);
	} else {
	  sprintf(text,"~FWToto bol online rozhovor na talkri ~OL%s %d ~RS~FW- %s, %s~RS\n",TALKER_TELNET_HOST,user->port,TALKER_CITY,TALKER_COUNTRY);
	}
	writecent(user,text);
	  sprintf(text,"~FW%s nas navstevnik cislo ~BB ~OL%d ~RS~RS~FW, tesime sa na dalsiu navstevu! :)~RS\n",pohl(user,"Bol si","Bola si"),user->visitor);
	writecent(user,text);
	  sprintf(text,"~OL~FB-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.~RS\n");
	write_user(user,text);
      }

for (i=0; i<REVTELL_LINES; i++) if (user->revbuff[i]!=NULL) {
	free ((void *)user->revbuff[i]);
	user->revbuff[i]=NULL;
	}

for (i=0; i<REVIRC_LINES; i++) if (user->revirc[i]!=NULL) {
	free ((void *)user->revirc[i]);
	user->revirc[i]=NULL;
	}

mins=(int)(time(0) - user->last_login)/60; /* FIIIX! :> */
if (user->level<KIN || user->vis) record_lastlog(user,mins,dovod);

dump_user_data(user);

if (user->socket>1000) eliminate_webuser(user->socket);
else close(user->socket);

if (user->quest==2) {
     sprintf(text,"~FG~OL%s je questleader a %s sa! Bohuzial, quest sa musi zastavit...~RS~FW\n",user->name,pohl(user,"odhlasil","odhlasila"));
     write_room_except(NULL,text,user);
     quest_logger(text);
     quest.queston=0;
     }

    if (user->vis) {   
     if ((strcmp(user->room->name,ARENA_ROOM) || message!=2) && message!=3) {
      	if (user->level>KIN) sprintf(text,"~OL~FBMizne v hlbinach cyberspace:~FY %s~FB %s ~RS\n",user->name,user->desc);
      	else sprintf(text,"~OL~FBLOGOUT: %s %s ~RS\n",user->name,user->desc);
	if (!user->level) wrtype=WR_NEWBIE;
      	writesys(0,1,text,NULL);
	wrtype=0;
       }
     else if (message!=3) {                         
      	sprintf(text,"~OL~FB%s TO V ARENE: %s %s ~RS\n",pohl(user,"DOSTAL","DOSTALA"),user->name,user->desc);
      	write_room_except(user->room,text,user);
       }
     strcpy(user->logoutmsg,"-");
    }

      for(u2=user_first;u2!=NULL;u2=u2->next) {
      if (u2->type!=USER_TYPE || u2->login || u2->room==NULL || u2->ignall) continue;
  
      if (user->vis) {   
	      if (check_notify_user(u2,user) && u2!=user) {
	      			if (dovod!=NULL && (u2->ignsys || u2->ignnongreenhorn)) {
	      				sprintf(text,"~LB~OL~FR%s~RS~FR sa ti prave %s [~OL%s~RS~FR]\n",user->name,pohl(user,"odhlasil","odhlasila"), dovod);
	      				}
				else sprintf(text,"~LB~OL~FR%s~RS~FR sa ti prave %s.\n",user->name,pohl(user,"odhlasil","odhlasila"));
				write_user(u2,text);
				}
		}
      }
      
if (dovod!=NULL) free(dovod);
	
/*sprintf(tempfilename,"%s/%s.notify",USERFILES,user->name);
if (user->notifylist) save_notifylist(&(user->notifylist),tempfilename);
	else { 
		if ((fp=ropen(tempfilename,"r"))!=NULL) {
			fclose(fp);
			unlink(tempfilename);
			}	
		}
*/
free_notifylist(&(user->notifylist));

/*sprintf(tempfilename,"%s/%s.ign",USERFILES,user->name);
if (user->ignorelist) save_notifylist(&(user->ignorelist),tempfilename);
	else { 
		if ((fp=ropen(tempfilename,"r"))!=NULL) {
			fclose(fp);
			unlink(tempfilename);
			}	
		}
*/
free_notifylist(&(user->ignorelist));

free_notifylist(&(user->combanlist));

save_irc_details(user);
                 
sprintf(tempfilename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(tempfilename,"r"))!=NULL) { /*APPROVED*/
	fclose(fp);
	deltempfile(tempfilename);
	}
sprintf(tempfilename,"%s%s-lynx%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(tempfilename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(tempfilename);
	}

if (nukehim) {	
/*	sprintf(tempfilename,"userfiles/%s.D",user->name);
	if ((fp=ropen(tempfilename,"r"))!=NULL) {
		fclose(fp);
		unlink(tempfilename);
		}
*/              
        sprintf(query,"delete from `mailbox` where `userid`='%d'",user->id);
        mysql_kvery(query);

        db_deluser(user->name); /* MySQL */
        
/*	sprintf(tempfilename,"%s/%s.M",MAILFILES,user->name);
	if ((fp=ropen(tempfilename,"r"))!=NULL) { 
		fclose(fp);
		unlink(tempfilename);
		}
*/               
	}


user->socket=0;
user->fds=0;

if (logcommands) log_commands(user->name,"LOGOUT",1);
if (user->malloc_start!=NULL) free(user->malloc_start);
num_of_users--;

destruct_blackjack_game(user);
destroy_user_clones(user);

if (mins>3 && strcmp(user->smsfollow,"-")) {
  sprintf(text,"%d:%02d %s logged out after %d mins.",thour,tmin,user->name,mins);
  send_sms(user->smsfollow,text,0);
 }

UPDATE_FDS();
destruct_user(user);
reset_access(rm);
who_for_web();
/*destructed=0;*/
}

/*** Tell telnet not to echo characters - for password entry ***/
void echo_off(user)
UR_OBJECT user;
{
char seq[4];

if (user->socket>1000) {
  sprintf(seq,"%c%c",222,201);
  write_web(user->socket,seq);
  return;
 }
if (password_echo || user->statline==CHARMODE) return;
sprintf(seq,"%c%c%c",IAC,WILL,TELOPT_ECHO);
write_user(user,seq);
}


/*** Tell telnet to echo characters ***/
void echo_on(user)
UR_OBJECT user;
{
char seq[4];

if (user->socket>1000) {
  sprintf(seq,"%c%c",222,202);
  write_web(user->socket,seq);
  return;
 }
if (password_echo || user->statline==CHARMODE) return;
sprintf(seq,"%c%c%c",IAC,WONT,TELOPT_ECHO);
write_user(user,seq);
}
/** *********** **/
void show_recent_tells(UR_OBJECT user)
{
int i, line;

for(i=0;i<(user->newtell+1);++i) {

      line=(user->revline-(user->newtell-i)-1)%REVTELL_LINES;
      if (line<0) line=REVTELL_LINES+line;
      if (line>=REVTELL_LINES) line=0;
      if (user->revbuff[line]!=NULL) {
	   write_user(user,user->revbuff[line]);
	    }
      }

}

void send_recent_tells(UR_OBJECT user, int sp)  /* 0 = smail; 1 = mail */
{
int i, line, size,cnt=0;
FILE *fp;
time_t akt_cas;
char *ptr;
char filename[81];
/* char email[255]; */

size=0;
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);

if (sp) {
  if ((fp=ropen(filename,"w"))==NULL) return;
  time (&akt_cas);
  fprintf(fp,"From: Atlantis Talker <%s@%s>\n",TALKER_EMAIL_ALIAS,TALKER_EMAIL_HOST);
  fprintf(fp,"To: %s <%s>\n", user->name,user->email);
  fprintf(fp,"Subject: revtell (%s)\n\n", zobraz_datum(&akt_cas,5));
 }

 for(i=0;i<REVTELL_LINES;++i) {
   line=(user->revline+i)%REVTELL_LINES;
   if (user->revbuff[line]!=NULL) {
     cnt++;
     if (sp) {
       ptr=(char*)malloc((strlen(user->revbuff[line])+200)*sizeof(char));
       if (ptr!=NULL) {
         strcpy(ptr,user->revbuff[line]);
         colour_com_strip(ptr);
         force_language(ptr,user->lang,1);
         fprintf(fp,"%s",ptr);
         free(ptr);
        }
      }
     size+=strlen(user->revbuff[line]);
     size++;
    }
  }
 
 if (cnt && sp!=1) {
   ptr=(char*) malloc ((size+200)*sizeof(char));
   if (ptr!=NULL) {
     *ptr='\0';
     for(i=0;i<REVTELL_LINES;++i) {
       line=(user->revline+i)%REVTELL_LINES;
       if (user->revbuff[line]!=NULL) {
         strcat(ptr,user->revbuff[line]);
         /* strcat(ptr,"\n"); */
        }
      }	
     /* colour_com_strip(ptr); */
     force_language(ptr,user->lang,1);
     send_mail(user,user->name,ptr);
     free(ptr);
    }
  }
 if (sp) {
   fclose(fp);
   if (cnt) send_forward_email(user->email,filename);
   deltempfile(filename);
  }

/*
 for(i=0;i<(user->newtell+1);++i) {
   line=(user->revline-(user->newtell-i)-1)%REVTELL_LINES;
   if (line<0) line=REVTELL_LINES+line;
   if (line>=REVTELL_LINES) line=0;
   if (user->revbuff[line]!=NULL) {
     if (sp) {
       ptr=(char*)malloc((strlen(user->revbuff[line])+200)*sizeof(char));
       if (ptr!=NULL) {
         strcpy(ptr,user->revbuff[line]);
         colour_com_strip(ptr);
         force_language(ptr,user->lang,1);
         fprintf(fp, "%s", ptr);
         free(ptr);
        }
      }
     size+=strlen(user->revbuff[line]);
     size++;
    }
  }
 
 if (sp!=1) {
   ptr=(char*) malloc ((size+200)*sizeof(char));
   if (ptr!=NULL) {
     *ptr='\0';
     for(i=0;i<(user->newtell+1);++i) {
       line=(user->revline-(user->newtell-i)-1)%REVTELL_LINES;
       if (line<0) line=REVTELL_LINES+line;
        if (line>=REVTELL_LINES) line=0;
        if (user->revbuff[line]!=NULL) {
          strcat(ptr,user->revbuff[line]);
*/
	   /* strcat(ptr,"\n"); */
/*
	  }
       }
*/
      /* colour_com_strip(ptr); */
/*
      force_language(ptr,user->lang,1);
      send_mail(user,user->name,ptr);      
      free(ptr);
     }
   }
  if (sp) {
    fclose(fp);
    send_forward_email(user->email,filename);
   }
*/
}


/************ MISCELLANIOUS FUNCTIONS *************/

/*** Stuff that is neither speech nor a command is dealt with here ***/
int misc_ops(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int bogus;
PAGER pom;
char tempname[15],temppass[20];
char histrecord[20],temp[20];
int del_count=0,a=0,z=0;
unsigned int i,b=0;

if (user->misc_op) {
	sprintf(histrecord,"%02d:%02d *%d", tmin, tsec, user->misc_op);
	record_history(user, histrecord);
	}

switch(user->misc_op) {
      case 1:
      if (toupper(inpstr[0])=='Y') {
            if (rs_countdown && !rs_which) {
                  if (rs_countdown>60)
                        sprintf(text,"\n~LB~OLSYSTEM: ~FR~LISHUTDOWN INICIALIZOVANY, pad o %d minut(y) a %d sekund(y)!\n\n",rs_countdown/60,rs_countdown%60);
                  else sprintf(text,"\n~LB~OLSYSTEM: ~FR~LISHUTDOWN INICIALIZOVANY, pad o %d sekund(y)!\n\n",rs_countdown);
                  write_room(NULL,text);
                  sprintf(text,"%s inicializoval(a) %d-sekundovy odpocet SHUTDOWNu.\n",user->name,rs_countdown);
                  write_syslog(text,1);
                  rs_user=user;
                  rs_announce=time(0);
                  user->misc_op=0;
                  prompt(user);
                  return 1;
                  }
            talker_shutdown(user,NULL,0);
            }
      /* This will reset any reboot countdown that was started, oh well */
      rs_countdown=0;
      rs_announce=0;
      rs_which=-1;
      rs_user=NULL;
      user->misc_op=0;
      prompt(user);
      return 1;

      case 2:
      case 222:
      case 223:
      if (toupper(inpstr[0])=='E') {
            user->misc_op=0;
            user->filepos=0;
            user->messsize=0;
            deltempfile(user->page_file);
            user->page_file[0]='\0';
            user->subject[0]='\0';
            zrus_pager_haldu(user);
            if (user!=NULL) { 
            	user->ignall=user->has_ignall;
            	user->has_ignall=0;
            	user->browsing=0;
            	}
            if (user->newtell)
               {
                show_recent_tells(user);                
                user->newtell=0;
                user->tellpos=0;
               }
            show_recent_notifies(user);
            prompt(user);            
      return 1;
      }
    else if (toupper(inpstr[0])=='R') {
      del_count=1;
     }
    if (!del_count && word_count) {
      if (is_number(word[0]) || (word[0][0]=='-' && is_number((word[0]+1)))) {
        user->skip=atoi(word[0]);
/*        if (user->skip>0) user->skip++; */
        if (user->skip<0) { del_count=-user->skip+1; user->skip=0; }
       }
      else if (word[0][0]=='/') {
        for(i=1;i<strlen(word[0]) && i<20;i++)
         user->findstr[i-1]=word[0][i];
        user->findstr[i-1]='\0';
        del_count=1;
       }
     }
    if (toupper(inpstr[0])=='B' || del_count>0) {
      if (del_count==0) del_count=2;
      for (bogus=0; bogus<del_count;bogus++) {
        if (user->pager!=NULL) {
          pom=user->pager;
	  user->pager=user->pager->next;
	  free ((void *) pom);
          user->pageno--;
	 }
       }        
      if (user->pager!=NULL) {
      	user->filepos=user->pager->filepos;
       	user->messnum=user->pager->messnum;
       }
      else {
       	user->filepos=0;
/*        user->messsize=0; */
      	user->messnum=user->rjoke_from;
        user->pageno=0;
       }          	    
     }
    if ((user->misc_op==2 && more(user,user->socket,user->page_file)!=1)
        || (user->misc_op==222 && sqlmore(user,user->socket,user->page_file)!=1)
        || (user->misc_op==223 && mailmore(user)!=1)) {
          user->misc_op=0;
          user->filepos=0;
          user->messsize=0;
          user->page_file[0]='\0';
          zrus_pager_haldu(user);
          if (user!=NULL) { 
            user->ignall=user->has_ignall;
            user->has_ignall=0;
            user->browsing=0;
           }
          if (user->newtell) {
            /* sprintf(text,"~OL~LI~FTPrisli ti nove telly (%d), pouzi .revtell~LB~RS\n",user->newtell);
            write_user(user,text); */
            show_recent_tells(user);
            user->newtell=0;
            user->tellpos=0;
           }          
          show_recent_notifies(user);
          prompt(user);
         }
    return 1;
    
      case 3: /* writing on board */
      case 4: /* Writing mail */
      case 5: /* doing profile */
      case 8: /* writing on JokeBoard  - ZMENA - pridane! */
      case 11: /* Editovanie bcastu */
      case 16: /* pridavanie nejakeho notice */
      case 18: /* editovanie predmetu */
      case 19: /* vytvaranie predmetu */
      case 20: /* pisanie sms spravy */
      case 21: /* editovanie predmetu */
      editor(user,inpstr);  return 1;

      case 6:
      bogus=atoi(inpstr);
      if (bogus==user->kontrola) {
        user->exams--;
        if (user->level>=GOD) {
          write_user(user,"~OL~FRPopulous - Atlantis Edition warning: ~FWYou're a ~FGGOD~FW. You're ~FGimmortal~FW!\n");
          user->misc_op=0;
          return 1;
         }
        if (user->exams>0) {
          switch(user->exams) {
            case 1:
            case 4:
              a=2;
              b=8+rand()%20;
              user->kontrola=1;
              for (i=0;i<b;i++) user->kontrola=user->kontrola*2;
              z='^';
            break;
            case 3:
              a=10+rand()%40;
              b=10+rand()%40;
              user->kontrola=a*b;
              z='*'; 
            break;
            case 2:
              user->kontrola=50+rand()%50;
              b=10+rand()%10;
              a=user->kontrola*b;
              z='/'; 
            break;
           }
          sprintf(text,"Spravne, dalsi priklad:\n%d.) ~OL%d %c %d = ?\n",6-user->exams,a,z,b);
          write_user(user,text);
          return 1;
         }
        wipe_user(user->name);
        delete_user(user,1);
       }
      else {  user->misc_op=0; write_user(user,"~FRSamovrazda sa ti nepodarila!\n");  prompt(user);  }
      return 1;

      case 7:
      if (toupper(inpstr[0])=='Y') {
            if (rs_countdown && rs_which==1) {
                  if (rs_countdown>60)
                        sprintf(text,"\n~LB~OLSYSTEM: ~FY~LIREBOOT INICIALIZOVANY, restart o %d minut(y) a %d sekund(y)!\n\n",rs_countdown/60,rs_countdown%60);
                  else sprintf(text,"\n~LB~OLSYSTEM: ~FY~LIREBOOT INICIALIZOVANY, restart o %d sekund(y)!\n\n",rs_countdown);
                  write_room(NULL,text);
                  sprintf(text,"%s inicializoval(a) %d-sekundovy odpocet REBOOTu.\n",user->name,rs_countdown);
                  write_syslog(text,1);
                  rs_user=user;
                  rs_announce=time(0);
                  user->misc_op=0;
                  prompt(user);
                  return 1;
                  }
            talker_shutdown(user,NULL,1);
            }
      if (rs_which==1 && rs_countdown && rs_user==NULL) {
            rs_countdown=0;
            rs_announce=0;
            rs_which=-1;
            }
      user->misc_op=0;
      prompt(user);
      return 1;

      /* CASE 9 a 10 Zmena Spakky - veci okolo requestof*/
      case 9:
	/*for(i=0;i<strlen(inpstr);++i)*/ {
	       if ((strpbrk(inpstr," ;$/+*[]\\<>&|")) || (strlen(inpstr)>EMAIL_LEN))
		{
		write_user(user,"\n~OL~FRNepovolene znaky alebo prilis dlha adresa! Zopakuj .request\n");
		write_user(user,"Tvoja E-mail adresa: ");
		user->misc_op=9;
		return 1; /* NEBOLO! */
	       }
	 }
	 if (inpstr[0]=='\0') {
		write_user(user,"\n~OL~FRPodavanie requestu bolo zrusene.\n");
		user->ignall=user->has_ignall;
		user->has_ignall=0;
		user->misc_op=0;
		prompt(user);
		return 1; /* NEBOLO! */
		}

	sstrncpy(user->email,inpstr,EMAIL_LEN);
	sprintf(text,"Pouzije sa adresa: ~OL%s~RS",user->email);
	write_user(user,"\n");
	writecent(user,text);
	write_user(user,"\n\n");
	write_user(user,"Je to URCITE spravne (~OLA~RSno/~OLN~RSie)? ");
	user->misc_op=10;
	return 1; /* NEBOLO!! */

      case 10:
	if ((toupper(inpstr[0])=='Y') || (toupper(inpstr[0])=='A')) {
		user->ignall=user->has_ignall;
		user->has_ignall=0;
		user->misc_op=0;
		posielanie(user);		
		prompt(user);
		return 1;
		}
	write_user(user, "\n~OL~FRPodavanie requestu bolo zrusene. Skus to znovu prikazom .request\n\n");
	strcpy(user->email,"Nema ziadny email");
	user->ignall=user->has_ignall;
	user->has_ignall=0;
	user->misc_op=0;
	prompt(user);
	return 1;

/* (S) zmena - hesloviny - 12,13,14 ... */

     case 12:
	if (strcmp((char *)md5_crypt(expand_password(word[0]),user->name),user->pass)) {
			write_user(user,"\n~OL~FRNespravne heslo.\n");
			user->misc_op=0;
			user->ignall=user->has_ignall;
			user->has_ignall=0;
			echo_on(user);
			prompt(user);
			return 1;
			}
		user->misc_op=13;
                if (user->socket>1000) echo_off(user);
		write_user(user,"\n~FTNove heslo: ");
		return 1; 
			
      case 13:		      				
      	        if (!strlen(word[0])) {
      	        	write_user(user,"\n~OL~FRZmena hesla nebola vykonana.\n");
			user->misc_op=0;
			user->ignall=user->has_ignall;
			user->has_ignall=0;
			echo_on(user);
			prompt(user);
			return 1;      	        
      	        	}
		if (strlen(word[0])<4) {
			if (strlen(word[0])) write_user(user,"\n~OL~FRPrilis kratke heslo (musi mat aspon 4 znaky)\n");
			user->misc_op=13;
                       if (user->socket>1000) echo_off(user);
			write_user(user,"\n~FTNove heslo: ");		
			return 1; 						
		        }
			
		if (strlen(word[0])>10) {
			write_user(user,"\n~OL~FRPrilis dlhe heslo (max. 10 znakov)\n");
			user->misc_op=13;
                       if (user->socket>1000) echo_off(user);
			write_user(user,"\n~FTNove heslo: ");					
			return 1;
			}
		if (user->level>=WIZ && check_passwd_simplex(word[0]) && user->chanp[0]=='\0') {
			write_user(user,"\n~OL~FRLevel CARODEJ a vyssi musi mat aspon 6-znakove heslo, aspon 1 nepismenovy znak,\n~OL~FRa nesmie obsahovat ziadne slovo zo slovnika.\n");
			user->misc_op=13;
                        if (user->socket>1000) echo_off(user);
			write_user(user,"\n~FTNove heslo: ");		
			return 1;
			}
		
		strcpy(tempname,user->name);
		strcpy(temppass,word[0]);
		strtolower(tempname);
		strtolower(temppass);
		if (strstr(temppass, tempname)) {
			write_user(user,"\n~OL~FRHeslo nemoze obsahovat Tvoje meno.\n");
			user->misc_op=13;
                        if (user->socket>1000) echo_off(user);
			write_user(user,"\n~FTNove heslo: ");		
			return 1; 						
			}
			
		user->misc_op=14;
                if (user->socket>1000) echo_off(user);
		write_user(user,"\n~FTPotvrd (zopakuj) nove heslo: ");
		sstrncpy(user->newpass,word[0], PASS_LEN);
		prompt(user);
		return 1; 
					
     case 14:			
	zmenheslo(user,0);
	return 1;

      case 15:
	if ((toupper(inpstr[0])=='Y') || (toupper(inpstr[0])=='A') || (inpstr[0]=='\0')) {
		send_mailinglist_request(user);
		user->misc_op=17;
		write_user(user,"Aby sa vypisy (oslovenia apod.) zobrazovali gramaticky spravne, potrebujeme\n");
		write_user(user,"vediet Tvoje pohlavie (zadaj: muz (male) alebo zena (female))\nPohlavie: ");
		return 1;
		}
	write_user(user, "\n~FWNevadi, prihlasit sa mozes aj neskor (.pomoc mlist)\n\n");
	
        write_user(user,"Aby sa vypisy (oslovenia apod.) zobrazovali gramaticky spravne, potrebujeme\n");
	write_user(user,"vediet Tvoje pohlavie - zadaj: muz (male) alebo zena (female))\nPohlavie: ");
	
	user->misc_op=17;	
	return 1;

      case 17:
      strtolower(inpstr);
      if (strcmp(inpstr,"muz") && strcmp(inpstr,"zena") && strcmp(inpstr,"male") && strcmp(inpstr,"female")) {
        write_user(user,"\nPohlavie moze byt: muz (male) alebo zena (female):\nPohlavie: ");
        user->misc_op=17;
        return 1;
      	}
      if (!strcmp(inpstr,"muz") || !strcmp(inpstr,"male")) {
      	user->sex=1;      	
      	}
      else user->sex=0;
      write_user(user,"\nDalsie parametre si podla potreby mozes nastavit prikazom .set (.pomoc set)\n");
      write_user(user,"Ak si nebudes vediet s niecim rady, skus prikaz .help, alebo prikazom .sos\n");
      write_user(user,"privolaj na pomoc niektoreho Wizarda. Prijemne talkovanie!\n");
      user->ignall=0;
      user->has_ignall=0;
      user->misc_op=0;
      prompt(user);
      return 1;
     case 30: /* wizzpasswd (V) */
        sprintf(temp,"Secure%s",user->name);
	if (strcmp((char *)md5_crypt(expand_password(word[0]),temp),user->wizpass)) {
			write_user(user,"\n~OL~FRNespravne heslo.\n");
			user->misc_op=0;
			user->ignall=user->has_ignall;
			user->has_ignall=0;
			echo_on(user);
			prompt(user);
                        police_freeze(user,2);
			return 1;
			}
		user->misc_op=31;
		write_user(user,"\n~FTNove bezpecnostne heslo: ");
		return 1; 
      case 31:
      	        if (!strlen(word[0])) {
      	        	write_user(user,"\n~OL~FRZmena bezpecnostneho hesla nebola vykonana.\n");
			user->misc_op=0;
			user->ignall=user->has_ignall;
			user->has_ignall=0;
			echo_on(user);
			prompt(user);
			return 1;      	        
      	        	}
		if (strlen(word[0])<3) {
			if (strlen(word[0])) write_user(user,"\n~OL~FRBezpecnostne heslo musi mat aspon 3 znaky.\n");
			user->misc_op=31;
			write_user(user,"\n~FTNove bezpecnostne heslo: ");
			return 1; 						
		        }
		if (strlen(word[0])>10) {
			write_user(user,"\n~OL~FRPrilis dlhe heslo (max. 10 znakov)\n");
			user->misc_op=31;
			write_user(user,"\n~FTNove bezpecnostne heslo: ");
			return 1;
			}
                if (!strcmp(word[0],"none")) {
                  write_user(user,"\n~FRBezpecnostne heslo bolo zrusene.\n");
                  strcpy(user->wizpass,"-");
		  user->misc_op=0;
		  user->ignall=user->has_ignall;
		  user->has_ignall=0;
		  echo_on(user);
		  prompt(user);
	  	  return 1;
                 }
		user->misc_op=32;
		write_user(user,"\n~FTZopakuj nove bezpecnostne heslo: ");
		sstrncpy(user->newpass,word[0],PASS_LEN);
		prompt(user);
		return 1; 
					
     case 32:
	zmenheslo(user,1);
	return 1;
     case 33: /* police! */
        sprintf(temp,"Secure%s",user->name);
	if (strcmp((char *)md5_crypt(expand_password(word[0]),temp),user->wizpass)) {
                        user->wizactivity++;
                        if (user->wizactivity>21) {
                          write_user(user,"\n~OL~FTStraze: ~FWZatykame vas za odcudzenie hesla.\n");
                          sprintf(text,"~OL~FTStraze: ~FWPozor, %s pravdepodobne niekto odchytil heslo.\n",sklonuj(user,3)); 
                          jdb_zarad(DB_BAN,user->id,0,"Pravdepodobne doslo k zneuzitiu tvojho konta.");
                          write_level(KIN,1,text,user); 
                          sprintf(text,"%s was AUTOBANNED by security guard.\n",user->name);
                          write_syslog(text,1);
                          user->dead=13;
        		  user->misc_op=0;
		          user->ignall=user->has_ignall;
        		  user->has_ignall=0;
                          user->wizactivity=3;
		          echo_on(user);
                          return 1;
                         }
			write_user(user,"\n~OL~FRNeplatne sluzobne heslo, zadajte vase sluzobne heslo, inac budeme strielat:\n");
			user->misc_op=33;
			return 1;
			}
		user->misc_op=0;
		write_user(user,"\n~OL~FTStraze: ~FWV poriadku, prepacte ze sme Vas obtazovali..\n");
		user->ignall=user->has_ignall;
		user->has_ignall=0;
                user->wizactivity=-10;
		echo_on(user);
		prompt(user);
		return 1;       
      }  /* end switch() */
return 0;
}

/******** request stuff - misc_op 9 a 10, posielanie() je procedura, ktora
	  zapise usera do suboru a posle na jeho adresu email s heslom, co
	  vykona script posli.
********/
void posielanie(user)    /* vola sa v misc_op==9 */
UR_OBJECT user;
{
char samo[7],spolu[18],heslo[7],adr[70];
FILE *fp;
char filename[80];
char userheslo[7],usermeno[13], bogus[70];
char par[200];
char text[255];
time_t t,akt_cas;

strcpy(adr,user->email);
strtolower(adr);

/* takze teraz ide prisna kontrola :))) */
if ((!strstr(adr,".")) || (!strstr(adr,"@")) || (!isalpha(adr[strlen(adr)-1])) ||
   (strlen(adr)<7)) {
	write_user(user,"~OL~FRNespravne zapisana E-mail adresa!\n");
	strcpy(user->email,"Nema ziadny email");
	user->has_ignall=user->ignall;
	user->ignall=1;
	user->misc_op=9;
        write_user(user,"Tvoja E-mail adresa: ");
	no_prompt=1;
	return;
   	}

sprintf(filename,"%s",ZOZNAM_USEROV);
if (!(fp=ropen(filename,"r"))) {
	sprintf(text,"Neexistuje subor '%s' s usermi, pokusim sa ho vytvorit\n",filename);
	write_syslog(text,1);
	if (!(fp=ropen(filename,"w"))) {
		sprintf(text,"Nie je mozne vytvorit subor '%s' s usermi!\n",filename);
		write_syslog(text,1);
		write_user(user,"Prepac, nastal problem, kontaktuj prosim strazcov Atlantisu! :-(\n");
		sprintf(text,"~OL~FRCHYBA: ~OL~FWNie je mozne vytvorit subor '%s'! Userko %s nemoze dokoncit .request!~RS\n",filename,user->name);
		writesys(WIZ,1,text,NULL);
		return;
	}
	fclose(fp);
	sprintf(text,"Subor '%s' uspesne vytvoreny!\n",filename);
	write_syslog(text,1);
	if (!(fp=ropen(filename,"r"))) {
		write_user(user,"Prepac, nastal problem, kontaktuj prosim strazcov Atlantis! :-(\n");
		sprintf(text,"Subor '%s' nie je mozne otvorit na citanie!\n",filename);
		write_syslog(text,1);
		sprintf(text,"~OL~FRCHYBA: ~OL~FWNie je mozne otvorit subor '%s'! Userko %s nemoze dokoncit .request!~RS\n",filename,user->name);
		writesys(KIN,1,text,NULL);
		return;
	}
}

fscanf(fp,"%s %s %s", userheslo, usermeno, bogus);
while(!feof(fp)) {		
	strtolower(bogus);
	if ((!strcmp(usermeno,"*")) && (strstr(adr,bogus))) {
		fclose(fp);
		write_user(user,"~OL~FRPrepac, ale tato adresa/domena je vyblokovana, nieje na nu mozne podat request.\n");
		strcpy(user->email,"Nema ziadny email");
		user->has_ignall=user->ignall;
		user->ignall=1;
		user->misc_op=9;
		write_user(user,"Tvoja E-mail adresa: ");
		no_prompt=1;
		return;
		}
	if (strstr(adr,bogus)) { /*TAKY EMAIL UZ EXISTUJE !*/
		sprintf(text,"\n~OL~FRPrepac, ale na tuto adresu je uz registrovany iny uzivatel.\nAk si myslis ze to nieje mozne, kontaktuj GODov Atlantisu (%s@%s)\n",TALKER_EMAIL_ALIAS,TALKER_EMAIL_HOST);
		write_user(user,text);
		strcpy(user->email,"Nema ziadny email");
		fclose(fp);
                sprintf(filename,"%s",MULTI_REQUEST);
                if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
                 return;
                 } 
                time (&akt_cas);
                sprintf(text,"%12s  %-12s %-12s %s",zobraz_datum(&akt_cas,4),user->name,usermeno,bogus);
                fprintf(fp,"%s\n",text);
		fclose(fp);
		return;
		}
	fscanf(fp,"%s %s %s", userheslo, usermeno, bogus);
	}
fclose(fp);

write_user(user, "\n~OLVyborne, uspesne si sa zaregistroval. ~RSKazdu chvilu ocakavaj email v ktorom sa\n");
write_user(user, "dozvies svoje request-heslo. Potom zadaj prikaz ~OL.request <heslo>~RS a stanes sa\n");
write_user(user, "plnopravnym obcanom Atlantisu.\n\n");      		      		
strcpy(samo, "aeiouy");
strcpy(spolu,"bcdfghjklmnprstvz");		
srand((unsigned) time(&t)); /* zasrandujeme si ;) */

heslo[0]=spolu[(rand()%17)];
heslo[1]=samo[(rand()%6)];
heslo[2]=spolu[(rand()%17)];
heslo[3]=samo[(rand()%6)];
heslo[4]=spolu[(rand()%17)];
heslo[5]=samo[(rand()%6)];
heslo[6]='\0';
strcpy(user->request,heslo);

user->accreq=1;
save_user_details(user,1);
strcpy(filename,ZOZNAM_USEROV);
if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
	write_user(user,"Prepac, nastal problem, kontaktuj prosim strazcov Atlantis! :-(\n");
	sprintf(text,"~OL~FRCHYBA: ~OL~FWNie je mozne pridavat do suboru '%s'! Userko %s nemoze dokoncit .request!~RS\n",filename,user->name);
	writesys(KIN,1,text,NULL);
	return;
}

fprintf(fp,"%-6s %-12s %s\n",heslo,user->name, user->email);
fclose(fp);

sprintf(par,"./posli %s %s %s %s",user->name,user->email,heslo,user->site);

  switch(double_fork()) {
    case -1 : sprintf(text,"~OL~FRCHYBA VO FORKOVANI PRI POKUSE POSLAT REQUEST HESLO UZIVATELOVI %s!\n", user->name);
    	      writesys(KIN,1,text,NULL);
    	      write_syslog(text,1); return;
    case  0 : system(par);	      
	      _exit(1);
    }

sprintf(text,"~OL~FY%s ~FGuspesne dokoncil%s request, email: ~FR%s\n",user->name,pohl(user,"","a"),user->email);
wrtype=WR_NEWBIE;
writesys(WIZ, 1, text, NULL);
wrtype=0;
sprintf(text,"Request from: %s, email: %s\n",user->name,user->email);
write_syslog(text,1);
strcpy(user->requestemail,user->email);/* toto sa zide pamatat si priamo u
luzera lebo si to vacsinou menia, napr. aj na vlastnu adresu na atlantise */
}

/*void adv_edit(UR_OBJECT user)
{
int i;
FILE *fp;
char epl[201],filename[81],ret[2000];

if ((user->misc_op==18 || user->misc_op==19) && user->ep_line>0) {
  if (user->misc_op==18) sprintf(filename,"%s/%s.P",PREDMET_DIR,user->mail_to);
  else sprintf(filename,"%s/%s.P",PREDMET_DIR,PREDMET_TEMP);
  if (!(fp=ropen(filename,"r"))) {
    return;
   }
  i=0;
  ret[0]='\0';
  fgets(epl,200,fp);
  while(!feof(fp)) {
    i++;
    if (i==user->ep_line) break;
    fgets(epl,200,fp);
   }
  if (feof(fp)) { user->ep_line=0; fclose(fp); return; }
  if (user->misc_op==19) {
    i=0;
    while ((epl[strlen(epl)-2]=='\\') && (!(feof(fp)))) {
      epl[strlen(epl)-2]='\n';
      epl[strlen(epl)-1]='\0';
      strcat(ret,epl);
      strcat(ret,"~FT");
      user->ep_line++;
      i=1;
      fgets(epl,200,fp);
     }
   }
  strcat(ret,epl);
  fclose(fp);
  i=user->edit_line+1;
  if (user->misc_op==18) sprintf(text,"~FT%02d>~RS%s",i,ret);
  else sprintf(text,"~FT%s",ret);
  write_user(user,text);
  user->ep_line++;
 }
}

char *adv_e_str(UR_OBJECT user)
{
int i;
FILE *fp;
char filename[81];
static char epl[201];

epl[0]='\0';
if (user->ep_line<2) return epl;
  sprintf(filename,"%s/%s.P",PREDMET_DIR,user->mail_to);
  if (!(fp=ropen(filename,"r"))) {
    return epl;
   }
  i=0;
  fgets(epl,200,fp);
  while(!feof(fp)) {
    i++;
    if (i==user->ep_line-1) break;
    fgets(epl,200,fp);
   }
  if (feof(fp)) { user->ep_line=0; strcpy(epl,""); }
  fclose(fp);
  if (epl[strlen(epl)-1]=='\n') epl[strlen(epl)-1]='\0';
 return epl;
}
*/

/*** The editor used for writing profiles, mail and messages on the boards ***/
void editor(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int cnt,line;
char *edprompt="(~OLU~RS)loz, (~OLO~RS)dloz, o(~OLD~RS)loz&uloz, (~OLP~RS)rerob, u(~OLK~RS)az, (~OLZ~RS)rus: "; /*EDPROMPT*/
char *ptr,/* *ptr2, */*c;
char *temp; /* (S) - pri operacii s riadkami ... */
int redoline, linecount,i,webuser=0; /* ^^ detto */
/*
int cnt2;
FILE *fp;
char filename[80];
char lyne[451];
*/

if (user->socket>1000) webuser=1;
if (user->edit_op) {
      switch(toupper(*inpstr)) {
            case 'D':
            sprintf(query,"replace into `postponed` (`userid`,`text`) values ('%d','",user->id);
            c=user->malloc_start;
            while(c!=user->malloc_end) {
              if (*c=='\'') strcat(query,"\\'");
              else if (*c=='"') strcat(query,"\\\"");
              else if (*c=='\\') strcat(query,"\\\\");
              else chrcat(query,*c);
              ++c;
             }
            strcat(query,"');");
            if (mysql_kvery(query))
              write_user(user,"Text bol ulozeny, neskor ho mozes nacitat v editore prikazom .ins\nalebo si ho pozriet prikazom .postponed (.pos)\n");
            else 
              write_user(user,"Chyba: nepodarilo sa ulozit rozpisany text.\n");
            /* +++ */
	    case 'S': /* Koli kompatidebilite s minulostou ;) */
            case 'U':
	    sprintf(text,"%s %s pisanie v editore.\n",user->name, pohl(user,"ukoncil","ukoncila"));
	    write_room_except(user->room,text,user);
	    switch(user->misc_op) {
		  case 3: write_board(user,NULL,1);  break;
		  case 4: smail(user,NULL,1);  break;
		  case 5: enter_profile(user,1);  break;
		  case 8: write_joke(user,1); break;
		  case 11: bcast(user,inpstr,1); break;
		  case 16: write_noticeboard(user,1,1); break;
/*		  case 18: zobraz_predmety(user,1); break; */
/*		  case 19: zobraz_predmety(user,2); break; */
		  case 20: sms(user,1); break;
/*		  case 21: predmet_write(user,1); break; */
		  }
	    editor_done(user);
	    return;

            case 'P':
            user->edit_op=0;
            user->edit_line=1;
            user->charcnt=0;
            user->malloc_end=user->malloc_start;
            *user->malloc_start='\0';
            sprintf(text,"\nOprava spravy...\n\n~FT%02d>~RS",user->edit_line);
            write_user(user,text);
            return;
	   
            case 'Z':
            write_user(user,"\nSprava bola zrusena.\n");
	    sprintf(text,"%s %s pisanie v editore.\n",user->name, pohl(user,"zrusil","zrusila"));
	    write_room_except(user->room,text,user);
	    user->subject[0]='\0';
	    editor_done(user);
	    return;

            case 'O':
            sprintf(query,"replace into `postponed` (`userid`,`text`) values ('%d','",user->id);
            c=user->malloc_start;
            while(c!=user->malloc_end) {
              if (*c=='\'') strcat(query,"\\'");
              else if (*c=='"') strcat(query,"\\\"");
              else if (*c=='\\') strcat(query,"\\\\");
              else chrcat(query,*c);
              ++c;
             }
            strcat(query,"');");
            if (mysql_kvery(query))
              write_user(user,"Text bol ulozeny, neskor ho mozes nacitat v editore prikazom .ins\nalebo si ho pozriet prikazom .postponed (.pos)\n");
            else 
              write_user(user,"Chyba: nepodarilo sa ulozit rozpisany text.\n");
	    sprintf(text,"%s %s pisanie v editore.\n",user->name, pohl(user,"zrusil","zrusila"));
	    write_room_except(user->room,text,user);
	    user->subject[0]='\0';
	    editor_done(user);
	    return;

	    case 'K': 	   
            write_user(user,"\n~RS~FM.=-=-=-=-=-=-=-=-=-=-=-= ~OLTakto vyzera napisana sprava:~RS~FM =-=-=-=-=-=-=-=-=-=-=-=.\n");
	    /* ptr2=user->malloc_start; */

	    /*cnt2=0;
	    sprintf(filename,"misc/%s.editor_tmp",user->name);
	    if (!(fp=ropen(filename,"w"))) { 
	    	write_user(user,"nemozem otvorit potrebny subor.\n");
	    	return;
	    	}	    
	    while(*ptr2!='\0') {
		      putc(*ptr2,fp);
		      if (*ptr2=='\n') cnt=0; else ++cnt2;
		      if (cnt2==80) { putc('\n',fp); cnt2=0; }
	      ++ptr2;
            }	     
            fclose(fp);
            if (!(fp=ropen(filename,"r"))) {
            	write_user(user,"nemozem otvorit ulozeny subor.\n");
            	return;
            	}
            fgets(lyne,450,fp);            
            while(!feof(fp)) {
            	write_user(user,lyne);
            	fgets(lyne,450,fp);
            	}
            fclose(fp);         
            unlink(filename);
            */

            write_user(user, user->malloc_start);
            write_user(user,"~RS~FM`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='\n");
            write_user(user,edprompt);
            return; 

            default:
            write_user(user,edprompt);            
            return;
            }
      }


/* Allocate memory if user has just started editing */
if (user->malloc_start==NULL) {
      if ((user->malloc_start=(char *)malloc(MAX_LINES*81))==NULL) {
            sprintf(text,"%s: chyba pri alokacii pamate.\n",syserror);
            write_user(user,text);
            write_syslog("CHYBA: Zlyhlala alokacia pamate v editor().\n",0);
            user->misc_op=0;
            prompt(user);
            return;
            }
      user->ignall_store=user->ignall;
      user->ignall=1; /* Preco to nefunguje? Dont want chat fucking up the edit screen */  
      user->miscoptime=thour*3600+tmin*60+tsec;
      user->edit_line=1;  
      user->charcnt=0;  
      user->malloc_end=user->malloc_start;  
      *user->malloc_start='\0';
      if (user->misc_op==18 || user->misc_op==19) {
        sprintf(text,"~OL~FW%s",user->mail_to);
        write_user(user,title(text,"~FG"));
        if (user->misc_op==18) write_user(user,"~FTAk nechces menit riadok, udri Enter.\n");
       }
      else if (user->misc_op==20) {
        sprintf(text,"~FT(Maximum %3d znakov, ukoncis samotnou bodkou '.' v riadku, '.help' pre pomoc,\n~FT'.rev' zobrazi zatial napisany text, '.redo <riadok>' navrat k zadanemu riadku)\n",user->smschars);
        write_user(user,text);
       }
      else {
        write_user(user,"~RS~FM.=-=-=-=-=-=-=-=-=-=-=-=-=-=--- ~OLAtlantis Editor ~RS~FM---=-=-=-=-=-=-=-=-=-=-=-=-=-=.\n");
        sprintf(text,"~FT(Maximum %-2d riadkov, ukoncis samotnou bodkou '.' v riadku, '.help' pre pomoc,\n~FT'.rev' zobrazi zatial napisany text, '.redo <riadok>' navrat k zadanemu riadku)\n\n",MAX_LINES);
        writecent(user,text);
       }
      if (user->misc_op!=3 && strlen(user->subject)>1) {
      	sprintf(text,"~FGSubject: ~FW%s\n",user->subject);
      	write_user(user,text);
      	}
/*      user->edit_line--; */
/*      adv_edit(user); */
/*      user->edit_line++; */
      if (!webuser) write_user(user,"~FT01>~FW");
      sprintf(text,"%s %s nieco pisat v editore...\n",user->name, pohl(user,"zacal","zacala"));  
      if (user->misc_op!=21) write_room_except(user->room,text,user);  
      return;  
      }  
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);  
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s pisat skaredo v editore!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      sprintf(text,"~FT%02d>~RS",user->edit_line);
      if (!webuser) write_user(user,text);
      return;
      }
   if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);  
      sprintf(text,"~FT%02d>~RS",user->edit_line);
      if (!webuser) write_user(user,text);
      return;
      }

 
/* Check for empty line */
if ((user->misc_op!=18 || user->ep_line==0) && !word_count) {
      if (!user->charcnt) {
            /* adv_edit(user); */
            if (user->statline!=CHARMODE) {
                  sprintf(text,"~FT%02d>~RS",user->edit_line);
                  if (!webuser) write_user(user,text);
                  }
            return;
            }
      *user->malloc_end++='\n';
      *user->malloc_end='\0';      
      if (user->edit_line==MAX_LINES) goto END;
      sprintf(text,"~FT%02d>~RS",++user->edit_line);
     write_user(user,text);
      user->charcnt=0;
     return;
      }
/* If nothing carried over and a dot is entered then end */
if (!user->charcnt && !strcmp(inpstr,".")) goto END;

if (!user->charcnt && !strncmp(word[0],".cop",4)) {
	temp=user->malloc_start;
	if (!(redoline=atoi(word[1]))) { /*redoline = riadok kt. sa ide kopirovat*/
		sprintf(text,"CHYBA: Pouzi: .copy <cislo riadku>\n%d",user->edit_line);
		write_user(user,text);
		return;
		}
	if (redoline>=user->edit_line) {
		sprintf(text,"CHYBA: Riadok c. %d este nieje napisany!\n%02d>",redoline, user->edit_line);
		write_user(user,text);
		return;
		}
	linecount=1;
	while (*temp && redoline!=linecount) {
		if (*temp++=='\n') linecount++;
		}
	while (*temp && *temp!='\n') {
		*user->malloc_end++=*temp++;
		if (user->malloc_end-user->malloc_start>=MAX_LINES*81) goto END;
		}
	*user->malloc_end++='\n';
	*user->malloc_end='\0';	
	if (++user->edit_line>MAX_LINES) goto END;
        sprintf(text,"~FT%02d>",user->edit_line);
	if (!webuser) write_user(user,text);
	return;
	}

if (!user->charcnt && !strncmp(word[0],".hel",4)) {
	write_user(user,"~FM==============================================================\n");
	write_user(user,".rev                 - zobrazi zatial napisany text\n");
	write_user(user,".ins                 - vlozi odlozeny text\n");
	write_user(user,".postponed           - zobrazi odlozeny text\n");
	write_user(user,".redo cislo_riadku   - navrat k editovaniu od zadaneho riadku\n");
	write_user(user,".copy cislo_riadku   - skopirovanie riadku na aktualnu poziciu\n");
	write_user(user,".                    - ukoncenie editora\n");
	write_user(user,"~FM==============================================================\n");
        sprintf(text,"~FT%02d>",user->edit_line);	
        if (!webuser) write_user(user,text);
	return;
	}

if (!user->charcnt && !strncmp(word[0],".pos",4)) {
  sprintf(query,"select `text` from `postponed` where `userid`='%d'",user->id);
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result))) {
      if (row[0]) {
	write_user(user,"~FM=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Odlozeny text -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        write_user(user,row[0]);
	write_user(user,"~FM=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
       }
     }
    else {
      write_user(user,"Nemas odlozeny ziadny rozpisany text.\n");
      mysql_free_result(result);
      return;
     }
    mysql_free_result(result);
   }
  else {
    sprintf(text,"Chyba! Problem s citanim odlozenych sprav.\n");
    write_user(user,text);
   }
  return;
 } 
if (!user->charcnt && !strncmp(word[0],".ins",4)) {
  sprintf(query,"select `text` from `postponed` where `userid`='%d'",user->id);
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result))) {
      c=row[0];
      while (*c) {
        *user->malloc_end++=*c;
	if (user->malloc_end-user->malloc_start>=MAX_LINES*81-2) {
	  *user->malloc_end='\0';	
          mysql_free_result(result);
          goto END;
     	 }
        if (*c=='\n') user->edit_line++;
        ++c;
       }
      *user->malloc_end='\0';	
      if (user->edit_line>MAX_LINES) {
        mysql_free_result(result);
        goto END;
       }
      strcpy(word[0],".rev");
      mysql_free_result(result);
     }
    else {
      write_user(user,"Nemas odlozeny ziadny rozpisany text.\n");
      mysql_free_result(result);
      return;
     }
   }
  else {
    sprintf(text,"Chyba! Problem s citanim odlozenych sprav.\n~FT%02d>~RS",user->edit_line);
    write_user(user,text);
    return;
   }
 }
if (!user->charcnt && !strncmp(word[0],".red",4)) {
	if (!(redoline=atoi(word[1]))) {
		sprintf(text,"CHYBA: Pouzi .redo <cislo riadku>\n~FT%02d>~RS",user->edit_line);
		write_user(user,text);
		return;
		}
	if (redoline>=user->edit_line) {
		sprintf(text,"CHYBA: tolko riadkov este nieje napisanych!\n~FT%02d>~RS",user->edit_line);
		write_user(user,text);
		return;
		}
	linecount=1;
	temp=user->malloc_start; /* s tymto budeme pracovatj */
	while (*temp && redoline!=linecount) {
		if (*temp++=='\n') linecount++;
		}
	user->edit_line=redoline;
	user->malloc_end=temp;
	*user->malloc_end='\0';
	sprintf(text,"~FT%02d>~RS",user->edit_line);
	if (!webuser) { 
          write_user(user,text);
  	  return;
         }
        else strcpy(word[0],".rev");
	}
if (!user->charcnt && !strncmp(word[0],".rev",4)) {
        if (user->statline==CHARMODE) write_user(user,"\n");
	write_user(user,"~FM=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	write_user(user,user->malloc_start);
	sprintf(text,"~FT%02d>~RS",user->edit_line);	
	if (!webuser) write_user(user,text);
	return;
	}

if (user->statline==CHARMODE) {
    write_user(user,inpstr);
    write_user(user,"\n");
    }

/*if (user->misc_op==18 && user->ep_line>0 && !word_count) {
  strcpy(lyne,adv_e_str(user));
  inpstr=lyne;
 }*/
line=user->edit_line;
cnt=user->charcnt;
/* adv_edit(user); */
/* loop through input and store in allocated memory */
if (user->socket>1000) {
  strncpy(text,inpstr,80);
  write_user(user,text);
 }
while(*inpstr) {
      *user->malloc_end++=*inpstr++;
      if (++cnt==80) {  user->edit_line++; *user->malloc_end++='\n'; cnt=0;  }
      if (user->edit_line>MAX_LINES
          || user->malloc_end - user->malloc_start>=MAX_LINES*81) goto END;
      if (user->misc_op==20 && user->malloc_end - user->malloc_start>=user->smschars) goto END;
      }
if (line!=user->edit_line) {
      ptr=(char *)(user->malloc_end-cnt);
      *user->malloc_end='\0';
      if (webuser) {
        sprintf(text,"%s",ptr);
        write_user(user,text);
       }
      else {
        sprintf(text,"~FT%02d>~RS%s",user->edit_line,ptr);
        write_user(user,text);
       }
      user->charcnt=cnt;
      return;
      }
else {
      *user->malloc_end++='\n';
      *user->malloc_end='\0';
      user->charcnt=0;
      }
if (user->edit_line!=MAX_LINES) {
      if (user->misc_op==20) {
        i=user->malloc_end - user->malloc_start;
        sprintf(text,"~FTPocet znakov: %d  Ostava%s este %d %s.\n",i,skloncislo(user->smschars-i,"","ju",""),user->smschars-i,skloncislo(user->smschars-i,"znak","znaky","znakov"));
        write_user(user,text);
       }
      sprintf(text,"~FT%02d>~RS",++user->edit_line);
      if (!webuser) write_user(user,text);
      return;
      }

/* User has finished his message, prompt for what to do now */
END:

*user->malloc_end='\0';
if (*user->malloc_start) {
      write_user(user,"\n~RS~FM`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='\n");
      write_user(user,edprompt);
      user->edit_op=1;  return;
      }
write_user(user,"\nZiadny text.\n");
sprintf(text,"%s %s pisanie v editore.\n",user->name,pohl(user,"zrusil","zrusila"));
write_room_except(user->room,text,user);
editor_done(user);
}


/*** Reset some values at the end of editing ***/
void editor_done(user)
UR_OBJECT user;
{
user->misc_op=0;
user->edit_op=0;
user->edit_line=0;
free(user->malloc_start);
user->malloc_start=NULL;
user->malloc_end=NULL;
user->ignall=user->ignall_store;
if (user->newtell)
   {
    /*sprintf(text,"~OL~LI~FTPrisli ti nove telly (%d), pouzi .revtell~LB~RS\n",user->newtell);
    write_user(user,text);*/
    show_recent_tells(user);
    user->newtell=0;
    user->tellpos=0;
   }
show_recent_notifies(user);
prompt(user);
}


/*** Record speech and emotes in the room. ***/
void record(rm,str)
RM_OBJECT rm;
char *str;
{
wash_bell(str);
if (rm->revbuff[rm->revline]!=NULL) free (rm->revbuff[rm->revline]);
rm->revbuff[rm->revline]=(char *) malloc ((strlen(str)*sizeof(char))+16);
if (rm->revbuff[rm->revline]==NULL) {
  sprintf(text,"~OL~FROUT OF MEMORY IN RECORD(), WE'RE DEEP IN SHIT\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
else {
  sprintf(rm->revbuff[rm->revline],"%2d:%02d %s",thour,tmin,str);
  rm->revline=(rm->revline+1)%REVIEW_LINES;
 }
}

/*** Records tells and pemotes sent to the user. ***/
void record_tell(user,str)
UR_OBJECT user;
char *str;
{
wash_bell(str);
if (user->revbuff[user->revline]!=NULL) free (user->revbuff[user->revline]);
user->revbuff[user->revline]=(char *) malloc ((strlen(str)+16)*sizeof(char));
if (user->revbuff[user->revline]!=NULL) {
  if (user->timeinrevt) {
    sprintf(user->revbuff[user->revline],"\252C4%2d:%02d %s",thour,tmin,str);
   }
  else
    strcpy(user->revbuff[user->revline],str);
  user->revline=(user->revline+1)%REVTELL_LINES;
 }
else {
  sprintf(text,"~OL~FROUT OF MEMORY IN RECORD_TELL(), WE'RE DEEP IN SHIT\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
}

/*** Records tells and pemotes sent to the user. ***/
void record_irc(user,str)
UR_OBJECT user;
char *str;
{

if (user->revirc[user->revircline]!=NULL) free (user->revirc[user->revircline]);
user->revirc[user->revircline]=(char *) malloc ((strlen(str)+2)*sizeof(char));
if (user->revirc[user->revircline]==NULL) {
  sprintf(text,"~OL~FROUT OF MEMORY IN RECORD_IRC(), WE'RE DEEP IN SHIT\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
else {
  strcpy(user->revirc[user->revircline],str);
  user->revircline=(user->revircline+1)%REVIRC_LINES;
 }
}


/*** ZMENA Buko: Records shouts and semote  ***/
void record_shout(str)
char *str;
{
wash_bell(str);
sprintf(revshoutbuff[revshoutline],"%2d:%02d ",thour,tmin);
sstrncpy(revshoutbuff[revshoutline]+6,str,REVIEW_LEN-6);
revshoutbuff[revshoutline][REVIEW_LEN]='\n';
revshoutbuff[revshoutline][REVIEW_LEN+1]='\0';
revshoutline=(revshoutline+1)%REVIEW_LINES;
}

void record_bcast(str)
char *str;
{
wash_bell(str);
sstrncpy(revbcastbuff[revbcastline],str,REVIEW_LEN*2);
revbcastbuff[revbcastline][REVIEW_LEN*2]='\n';
revbcastbuff[revbcastline][REVIEW_LEN*2+1]='\0';
revbcastline=(revbcastline+1)%REVB_LINES;
}

void record_sos(str)
char *str;
{
wash_bell(str);
sstrncpy(revsosbuff[revsosline],str,REVIEW_LEN);
revsosbuff[revsosline][REVIEW_LEN]='\n';
revsosbuff[revsosline][REVIEW_LEN+1]='\0';
revsosline=(revsosline+1)%REVIEW_LINES;
}

void record_portalisshout(char *lbl, char *str)
{
wash_bell(str);
sprintf(revporshbuff[revporshline],"%3s%2d:%02d ",lbl,thour,tmin);
sstrncpy(revporshbuff[revporshline]+9,str,REVIEW_LEN-9);
revporshbuff[revporshline][REVIEW_LEN]='\n';
revporshbuff[revporshline][REVIEW_LEN+1]='\0';
revporshline=(revporshline+1)%REVIEW_LINES;
}

/*** ZMENA Rider: Records gossips  ***/
void record_gossip(str)
char *str;
{
wash_bell(str);
sprintf(revgossipbuff[revgossipline],"%2d:%02d ",thour,tmin);
sstrncpy(revgossipbuff[revgossipline]+6,str,REVIEW_LEN-6);
revgossipbuff[revgossipline][REVIEW_LEN]='\n';
revgossipbuff[revgossipline][REVIEW_LEN+1]='\0';
revgossipline=(revgossipline+1)%REVIEW_LINES;
}

/*** Record History ***/
void record_history(UR_OBJECT user, char *str)
{
sstrncpy(user->history[user->histpos],str,HISTORY_LEN-1);
user->histpos=(user->histpos+1)%HISTORY_LINES;
}

/*** Quest recorder :)  ***/
void record_quest(str)
char *str;
{
wash_bell(str);
sstrncpy(revquestbuff[revquestline],str,REVIEW_LEN);
revquestbuff[revquestline][REVIEW_LEN]='\n';
revquestbuff[revquestline][REVIEW_LEN+1]='\0';
revquestline=(revquestline+1)%REVIEW_LINES;
}

/*** ZMENA Buko: Records wizshouts ***/
void record_wizshout(str)
char *str;
{
wash_bell(str);
sstrncpy(revwizshoutbuff[revwizshoutline],str,REVIEW_LEN);
revwizshoutbuff[revwizshoutline][REVIEW_LEN]='\n';
revwizshoutbuff[revwizshoutline][REVIEW_LEN+1]='\0';
revwizshoutline=(revwizshoutline+1)%REVIEW_LINES;
}

/*** Spakky: record lastlog LLOG ***/
void record_lastlog(user, spent, dovod)
UR_OBJECT user;
int spent;
char *dovod;
{
/*
char lajna[100];
char desc[DESC_CHAR_LEN+1];

strcpy(desc, user->desc);
colour_com_strip(desc);

sprintf(lajna,"~OL~FW%s ~RS%s",user->name,desc);
if (dovod!=NULL) sprintf(lastlog[lastlogline],"~FG[%02d:%02d] %-48.48s ~RS~FT(%d mins.) ~RS~FR[~OL%.22s~RS~FR]~RS\n", thour, tmin, lajna, spent, dovod); 
  	    else sprintf(lastlog[lastlogline],"~FG[%02d:%02d] %-48.48s ~RS~FT(%d mins.)~RS\n", thour, tmin, lajna, spent); 
*/
if (dovod!=NULL) sprintf(lastlog[lastlogline],"~FG[%02d:%02d] ~FW~OL%-12s ~RS~FT(%3d min) ~RS~FR[~OL%.50s~RS~FR]~RS\n", thour, tmin, user->name, spent, dovod); 
  	    else sprintf(lastlog[lastlogline],"~FG[%02d:%02d] ~FW~OL%-12s ~RS~FT(%3d min)~RS\n", thour, tmin, user->name, spent); 
lastlogline=(lastlogline+1)%LASTLOG_LINES;
}
/*------------------------------------------------------*/

/*** Set room access back to public if not enough users in room
     ZMENA - kontroluje GOD_PRIVATE - ale ako??? ;) ***/
void reset_access(rm)
RM_OBJECT rm;
{
UR_OBJECT u;
int cnt;

if (rm==NULL || (rm->access!=PRIVATE && rm->access!=GOD_PRIVATE)) return;
cnt=0;
for(u=user_first;u!=NULL;u=u->next) {
	if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
	if (u->room==rm) ++cnt;
	}
if (cnt<min_private_users) {
      write_room(rm,"Miestnost je odteraz ~FGVEREJNA.\n");
      rm->access=PUBLIC;

      /* Reset any invites into the room & clear review buffer */
      for(u=user_first;u!=NULL;u=u->next) {
	    if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;      
            if (u->invite_room==rm) u->invite_room=NULL;
            }
      clear_revbuff(rm);
      }
}

/*** Exit because of error during bootup ***/
void boot_exit(code)
int code;
{
switch(code) {
      case 1:
      write_syslog("CHBYA PRI STARTE: Chyba v parsovani konfiguracneho suboru.\n",0);
      exit(1);

      case 2:
      perror("NUTS: Nemozno aktivovat hlavny port");
      write_syslog("CHBYA PRI STARTE: Nemozno aktivovat hlavny port.\n",0);
      exit(2);

      case 3:
      perror("NUTS: Nemozno aktivovat wizport");
      write_syslog("CHBYA PRI STARTE: Nemozno aktivovat wizport.\n",0);
      exit(3);

      case 4:
      perror("NUTS: Nemozno aktivovat linkport");
      write_syslog("CHBYA PRI STARTE: Nemozno aktivovat linkport.\n",0);
      exit(4);

      case 5:
      perror("NUTS: Nemozno pouzit hlavny port");
      write_syslog("CHBYA PRI STARTE: Nemozno pouzit hlavny port.\n",0);
      exit(5);

      case 6:
      perror("NUTS: Nemozno pouzit wizport");
      write_syslog("CHBYA PRI STARTE: Nemozno pouzit wizport.\n",0);
      exit(6);

      case 7:
      perror("NUTS: Nemozno pouzit linkport");
      write_syslog("CHBYA PRI STARTE: Nemozno pouzit linkport.\n",0);
      exit(7);

      case 8:
      perror("NUTS: Hlavny port neodpoveda");
      write_syslog("CHBYA PRI STARTE: Hlavny port neodpoveda.\n",0);
      exit(8);

      case 9:
      perror("NUTS: Wizport neodpoveda");
      write_syslog("CHBYA PRI STARTE: Wizport neodpoveda.\n",0);
      exit(9);

      case 10:
      perror("NUTS: Linkport neodpoveda");
      write_syslog("CHBYA PRI STARTE: Linkport neodopveda.\n",0);
      exit(10);

      case 11:
      perror("NUTS: Chyba vo forku");
      write_syslog("CHBYA PRI STARTE: Chyba vo forku.\n",0);
      exit(11);

      case 12:
      write_syslog("CHBYA PRI STARTE: Chyba v parsovani predmetov.\n",0);
      exit(12);

      }
}

/*A CIL NOVY PROMPT!*/
/*** User prompt 
 Zmena Spakky - moznost vybrat z viac typov promptov
 Zmena Rider - definicia vlastneho promptu PROMMMPT ***/

void prompt(user)
UR_OBJECT user;
{
int hr,min,newline=0;
char *s, spromp[BUFSIZE];

if (no_prompt || user->actual_remote_socket!=0) return;
if (user->statline==CHARMODE) return;

if (user->command_mode && !user->misc_op && !user->prompt) {
      if (!user->vis) write_user(user,"~FTCOM+> ");
      else write_user(user,"~FTCOM> ");
      return;
      }
if (user->misc_op) return;
hr=(int)(time(0)-user->last_login)/3600;
min=((int)(time(0)-user->last_login)%3600)/60;

spromp[0]='\0';
if (user->command_mode) {   
	if (!user->vis) strcat(spromp,"~RS~FT[COM+] ~RS~FW");
	   else strcat(spromp,"~RS~FT[COM] ~RS~FW");   
   }
if (!user->prompt) {
	write_user(user,spromp);   
	return;
	}
s=user->prompt_string;
while (*s)
   {
    if (strlen(spromp) > BUFSIZE-100) /* prevencia buffer overflow */
       {
        write_user(user,"Tvoja definicia promptu bola vadna: prompt bude zmazany.\n");
        user->prompt=prompt_def;
        strcpy(user->prompt_string,"");
        return; 
       }
    if (*s=='$')
      {
        s++;
        switch (*s)
          {
           case 'N': strcat(spromp,user->name);       /* Name */
           	     s++;
           	     break;
           case 'R': strcat(spromp,user->room->name); /* Room */
                     s++;
                     break;
           case 'S': sprintf(text,"%02d:%02d",hr,min); /* Session time */
           	     strcat(spromp,text);
                     s++;
                     break;
           case 'T': sprintf(text,"%02d:%02d",thour,tmin);  /* Real time */
           	     strcat(spromp,text);
                     s++;
                     break;
           case 'A': if (user->alarm>0) sprintf(text,"%d:%02d",user->alarm/60,user->alarm%60);  /* Alarm time remaining */
                     else text[0]='\0';
           	     strcat(spromp,text);
                     s++;
                     break;
           case 'F': strcat(spromp, user->follow);     /* Follow */
                     s++;
                     break;
           case 'L': if (user->lasttg==-1) strcat(spromp, user->lasttellfrom);   /* Lasttell */
	             else if (!user->lasttg) strcat(spromp,"\253Iz");
		     else strcat(spromp,"\253Im");
                     s++;
                     break;
           case 'P': sprintf(text,"%d",user->pp);      /* Power Points */
                     strcat(spromp,text);
                     s++;           
                     break;
           case 'H': sprintf(text,"%d",user->health);
                     strcat(spromp,text);
                     s++;           
                     break;
           case 'M': sprintf(text,"%d",user->mana);    /* Mana */
           	     strcat(spromp,text);
           	     s++;
           	     break;
           case 'E': if (newline==0) strcat(spromp,"\n");
                     newline=1;
           	     s++;
           	     break;
           default: strcat(spromp,"$");
           	     break;
          }
      } 
    else strncat(spromp,s++,1);
   }

strcat(spromp,"~RS~FW"); /* Vyhodit: a user si moze volit aj farbu textu:) */
write_user(user,spromp);
}

/*** Page a file out to user. Colour commands in files will only work if
     user!=NULL since if NULL we dont know if his terminal can support colour
     or not. Return values:
              0 = cannot find file, 1 = found file, 2 = found and finished ***/
int more(user,sock,filename)
UR_OBJECT user;
int sock;
char *filename;
{
int buffpos,num_chars,lines,retval,len,totalines, fperc, p=0,buffold=0;
unsigned int i;
long fsize;
char buff[OUT_BUFF_SIZE+8],*str; /* *colour_com_strip() */
/* char text2[83]; */
FILE *fp;
char text4[20],tmptxt[T_SIZE+50],c1[2],*tmp2;
int porcis;
int webuser=0,max,silent=0,hajlajt=0;
PAGER pom;
struct stat stbuf;

if (sock>1000) webuser=1;
if (user!=NULL && !user->filepos) {	
  if (user->ignall==1 && !user->has_ignall) {
    user->has_ignall=1;
   }			
  user->ignall=2;	
  user->pageno=0;
  user->miscoptime=thour*3600+tmin*60+tsec;
 }

if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
      if (user!=NULL) {
        user->filepos=0;
      	user->ignall=user->has_ignall;
      	user->has_ignall=0;
      	}
      return 0;
     }

/* dlzka suboru */
if (stat(filename,&stbuf)==-1) fsize=0;
else fsize=stbuf.st_size;
    
/* jump to reading posn in file */
if (user!=NULL) fseek(fp,user->filepos,0);

text[0]='\0';
c1[0]='\0';
tmptxt[0]='\0';
text4[0]='\0';
buffpos=0;
buffold=0;
num_chars=0;
retval=1;
len=0;
if (user!=NULL) totalines=user->lines-1;
	else totalines=23; /* R2 -> v irc mode je ich o dve menej! */ 

if ((user!=NULL) && (user->browsing==1) && (!user->filepos)) totalines-=1;
/* if ((user!=NULL) && (user->browsing==2) && (!user->filepos)) totalines-=2; */
/* if ((user!=NULL) && (user->browsing==10) && (!user->filepos)) totalines-=2; */


lines=0;
fgets(text,sizeof(text)-10,fp);

/* Go through file */
if (user!=NULL && user->skip>0) {
/*  if (user->skip==-1) max=1000000; */
  max=totalines*user->skip;
  user->skip--;
 }
else max=totalines;
while(!feof(fp) && (lines<max || user==NULL)) {

if (lines>0 && (lines)%totalines==0 && user!=NULL && user->skip>0) {
  user->skip--;
  if (user->skip>0) silent=1;
  else silent=0;
  user->filepos+=num_chars;
  if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
    sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v more()!\n");
    write_syslog(text,1);
    fclose(fp);
    return 2;
   }
  user->pageno++;
  pom->filepos=user->filepos;
  pom->messnum=user->messnum;
  pom->next=user->pager;
  user->pager=pom;
  c1[0]='\0';
  tmptxt[0]='\0';
  text4[0]='\0';
  buffpos=0;
  buffold=0;
  num_chars=0;
  retval=1;
  len=0;
 }

hajlajt=0;
if (user!=NULL && user->findstr[0])
  if (strstr(text,user->findstr)) hajlajt=1;
 
/* if (user!=NULL) force_language(text,user->lang,1); */
/* else force_language(text,0,1); */
len=strlen(text);
num_chars+=len;


/* Tuna hodime pocitanie jokeboardu */
if ((user!=NULL) && (user->browsing==1) && (strstr(text,"IddQD:"))) { 
	user->messnum++;
	for (i=0; i<strlen(text); i++) {
		c1[0]=text[i]; c1[1]='\0';
		strcat(tmptxt,c1);
		if (i==17) {
			sprintf(text4,"~FW%d~FB",user->messnum);
			p=0;			
			strcat(tmptxt,text4);
			if (user->messnum<10) p=1;
			if ((user->messnum>9) && (user->messnum<100)) p=2;
			if ((user->messnum>99) && (user->messnum<1000)) p=3;
			if ((user->messnum>999) && (user->messnum<10000)) p=4;
			if (user->messnum>9999) p=5;			
			i+=p;					
			}
		if (i==102) {
			porcis=(pocet_vtipov-user->messnum)+1;
			sprintf(text4,"~FW%d~FB",porcis);
			p=0;
			strcat(tmptxt,text4);
			if (porcis<10) p=1;
			if ((porcis>9) && (porcis<100)) p=2;
			if ((porcis>99) && (porcis<1000)) p=3;
			if ((porcis>999) && (porcis<10000)) p=4;
			if (porcis>9999) p=5;			
			i+=p;								
			}
		}
	sstrncpy(text,tmptxt,T_SIZE-10);
	tmptxt[0]='\0';
	}

/* Tuna bude pocitanie boardov 
if ((user!=NULL) && (user->browsing==3) && (strstr(text,"~OL~FM=-=-"))) {
	user->messnum++;
	for (i=0; i<strlen(text); i++) {
		c1[0]=text[i]; c1[1]='\0';
		strcat(tmptxt,c1);
		if (i==24) {
			sprintf(text4,"~FW%d~FM",user->messnum);
			p=0;			
			strcat(tmptxt,text4);
			if (user->messnum<10) p=1;
			if ((user->messnum>9) && (user->messnum<100)) p=2;
			if ((user->messnum>99) && (user->messnum<1000)) p=3;
			if ((user->messnum>999) && (user->messnum<10000)) p=4;
			if (user->messnum>9999) p=5;			
			i+=p;		
			}
		}
	sstrncpy(text,tmptxt,T_SIZE-10);
	tmptxt[0]='\0';
	}

if ((user!=NULL) && ((user->browsing==10) || (user->browsing==2))) {
      if (strstr(text,"~OL~FB======= ~RS~FYSprava od:")) {
      	user->messnum++;
      	}
      if (user->browsing==10) {
      	while ((user->messnum<=user->lastm) && (!feof(fp))) {
      		fgets(text,sizeof(text)-10,fp);	
      		len=strlen(text);
	        num_chars+=len;
      		if (strstr(text,"~OL~FB======= ~RS~FYSprava od:")) user->messnum++;
      		}
       if (feof(fp)) { 
       	write_user(user,"Bohuzial, neboli zaznamenane ziadne nove spravy.\n");
      	user->filepos=0; no_prompt=0; retval=2; user->ignall=user->has_ignall;
      	user->has_ignall=0;
      	user->browsing=0;
      	 fclose(fp);
      	return 2; 
      	  }
      	}
    }

if ((user!=NULL) && ((user->browsing==2) || (user->browsing==10)) && (strstr(text,"~OL~FB======= ~RS~FYSprava od:"))) {
	if (strstr(text,"\r")) kde=117; else kde=107;	
	for (i=0; i<strlen(text); i++) {
		c1[0]=text[i]; c1[1]='\0';
		strcat(tmptxt,c1);
		if (i==kde) {
			sprintf(text4,"~FY%d~FB",user->messnum);
			p=0;			
			strcat(tmptxt,text4);
			if (user->messnum<10) p=1;
			if ((user->messnum>9) && (user->messnum<100)) p=2;
			if ((user->messnum>99) && (user->messnum<1000)) p=3;
			if ((user->messnum>999) && (user->messnum<10000)) p=4;
			if (user->messnum>9999) p=5;			
			i+=p;		
			}
		}
	sstrncpy(text,tmptxt, T_SIZE-10);
	tmptxt[0]='\0';	
	}	            
*/
      tmp2=&text[0];
      while(*tmp2) {
        if (*tmp2=='\r' && *(tmp2+1)!='\n')
	 strcpy(text,tmp2+1);
	tmp2++;
       }
      if (hajlajt) {
        tmp2=strstr(text,user->findstr);
        memmove(tmp2+3,tmp2,strlen(tmp2)+1);
        strncpy(tmp2,"~BR",3);
        tmp2+=strlen(user->findstr)+3;
        memmove(tmp2+3,tmp2,strlen(tmp2)+1);
        strncpy(tmp2,"~BK",3);
       }
      str=text;
/*      if (user!=NULL) force_language(str,user->lang,1); */

      /* Process line from file */
      if (user!=NULL
      && (silent || ((user->browsing==1 || user->browsing==2 || user->browsing==3)
      && (user->messnum < user->rjoke_from)))
      ) { }
 	 else {
           if (user!=NULL && (user->statline==CHARMODE) && (user->newline)) {
              *(buff+buffpos)='\r';
              *(buff+buffpos+1)='\n';    /* CR/LF! */
              buffpos+=2;
              user->newline=0;
             }

 	   while(*str) {
            if (*str=='\n') {
                  *(buff+buffpos)='\0';
                  if (user!=NULL && user->ignword[0] && strstr(buff,user->ignword)) buffpos=buffold;
                  else buffold=buffpos;
                  if (buffpos>OUT_BUFF_SIZE-8) {
                        write2sock(user,sock,buff,buffpos);  
                        buffpos=0;
                        buffold=0;
                        }
                  /* Reset terminal before every newline */
                  if (user!=NULL && user->colour) {
                        if (!webuser) { memcpy(buff+buffpos,"\033[0m",4);  buffpos+=4; }
                       }

                  if (user!=NULL && (user->statline==CHARMODE) &&
                       (*(str+1)==0)) {
                        user->newline=1; ++str;
                        }
                  else {
                  *(buff+buffpos)='\r';  *(buff+buffpos+1)='\n';  /* CR/LF! */
                   buffpos+=2;  ++str;
                     }
                  }
            else {
                  /* Process colour commands in the file. See write_user()
		     function for full comments on this code.
		     ZMENA - zmenil som '/' na '~' aby sa dalo pisat URL /~xy */
		 /* if (*str=='~' && *(str+1)=='~') {  ++str;  continue;  } */
		  if (*str>=127) *str=' '; /* UMBA */
/*		  if (str!=text && *str=='~' && *(str-1)=='~') {
			*(buff+buffpos)=*str;  goto CONT;
			}*/
		  if (!webuser && *str=='~') {		  
  			if (buffpos>OUT_BUFF_SIZE-8) {
                          write2sock(user,sock,buff,buffpos);  
			  buffpos=0;
                          buffold=0;
	                 }
			++str;
			for(i=0;i<NUM_COLS;++i) {
			      if (!strncmp(str,colcom[i],2)) {
                                    if ((str-1)!=text && *(str-2)=='~') {
                                      *(buff+buffpos)=*str;
                                      goto CONT; 
                                     }
				    if (user!=NULL && user->colour && !(user->colour==2 && !strcmp(colcom[i],"LI"))
                                    && !(user->ignbeep && !strcmp(colcom[i],"LB"))
                                    && !(user->ignblink && !strcmp(colcom[i],"LI"))) {
					  memcpy(buffpos+buff,colcode[i],strlen(colcode[i]));
					  buffpos+=strlen(colcode[i])-1;
					  }
				    else buffpos--;
				    ++str;
				    goto CONT;
				    }
			      }
			*(buff+buffpos)=*(--str);
			}
		  else *(buff+buffpos)=*str;
		  CONT:
		  ++buffpos;   ++str;
		  }
	    if (buffpos==OUT_BUFF_SIZE) {
                  write2sock(user,sock,buff,OUT_BUFF_SIZE);
		  buffpos=0;
                  buffold=0;
		  }
	    }
	  }
      len-=(colour_com_count(text));
      if (user!=NULL && (user->browsing==1 || user->browsing==2 || user->browsing==3) && (user->messnum < user->rjoke_from)) { }
      	else lines+=len/80+(len<80);
      fgets(text,sizeof(text)-10,fp);
      }
if (buffpos && sock!=-1) {
  write2sock(user,sock,buff,buffpos);  
 }
/* if user is logging on dont page file */
if (user==NULL) {   fclose(fp);  return 2;  };

if (feof(fp)) {
      user->filepos=0;  no_prompt=0;  retval=2;
      zrus_pager_haldu(user);          
      if (user!=NULL) {
      	user->ignall=user->has_ignall;      
      	user->has_ignall=0;
/*      	if ((user->browsing==2) || (user->browsing==10)) user->lastm=user->messnum; */
/*      	if ((user->browsing==2) || (user->browsing==10)) write_user(user,"~FM====+~OL+~RS~FM+==================================================================+~OL+~RS~FM+====\n"); */
/*      	if (user->browsing==3) write_user(user,"~OL~FM=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"); */
      	if (user->browsing==1) write_user(user,"~FM<~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=\n");
      	if (user->browsing==6) {
	 	  write_user(user,"\n~OL~FB-    -  - -- -------< ~FY         Koniec dokumentu          ~FB >------- -- -  -    -\n");
	 	  user->browsing=0;
	 	  fclose(fp);
	 	  unlink(filename);
	 	  if (user->newtell) {
	 	  	show_recent_tells(user);
	 	  	user->newtell=0;
	 	  	user->tellpos=0;
	 	  	}
                  show_recent_notifies(user);
	 	  return 2;
      		}
      	user->browsing=0;
        
        if (user->skip>0) {
          sprintf(text,"                              ~BB~OL~FW Pocet stranok: ~FY%d ~BK\n",user->pageno+1);
          write_user(user,text);
         }
        user->skip=0;
      	if (user->newtell) {
             /*sprintf(text,"~OL~LI~FTPrisli ti nove telly (%d), pouzi .revtell~LB~RS\n",user->newtell);
             write_user(user,text);*/
             show_recent_tells(user);
             user->newtell=0;
             user->tellpos=0;
            }
        show_recent_notifies(user);
      	}
       
      }
      
else  {
      /* store file position and file name */
      user->filepos+=num_chars;      
      
      if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
        sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v more()!\n");
        write_syslog(text,1);
        return 2;
       }
      user->pageno++;
      pom->filepos=user->filepos;
      pom->messnum=user->messnum;
      pom->next=user->pager;
      user->pager=pom;
       
      strcpy(user->page_file,filename);
      if (fsize) fperc=(int)(user->filepos*100)/fsize;
      else fperc=0;
      
      if (user->skip>0) { 
        user->pageno++;
        fclose(fp);
        return more(user,sock,filename);
       }
      sprintf(text,"~BB~OL~FT[%d%%] ~FWstr:~FT %d, ~FYENTER~FW-dalej, ~FYB~FW-spat, ~FYR~FW-znovu, ~FYn~FW/~FY-n~FW stran, ~FY/~FWhladat, ~FYE~FW-koniec~RS~BK ",fperc,user->pageno);
      write_user(user,text);
      if (user->statline==CHARMODE) user->newline=1;
      no_prompt=1;
     }
fclose(fp);
if (retval == 2) { /* finished */
	if (strlen(filename) >= strlen(TMPSUFFIX)) {
		if (!strcmp(strchr(filename,'\0')-strlen(TMPSUFFIX),TMPSUFFIX)) deltempfile(filename);
	}
}
return retval;
}

void deltempfile(char *filename)
{
	if(remove(filename)) {
		sprintf(text,"~BR~OL~FW VAROVANIE: ~RS Nepodarilo sa vymazat docasny subor '%s'!\n",filename);
		write_level(GOD,1,text,NULL);
	}
}

int sqlmore(UR_OBJECT user,int sock,char* queryname)
{
int i,buffpos,num_chars,lines,retval,len,totalines, fperc,buffold=0;
long fsize;
char buff[OUT_BUFF_SIZE+8],*str; /* *colour_com_strip() */
/* char text2[83]; */
char /*text4[20],tmptxt[T_SIZE+50],c1[2], */*tmp2;
int webuser=0,max,silent=0,hajlajt=0,eor=0,pos=0;
PAGER pom;
/* struct stat stbuf; */

if (sock>1000) webuser=1;
if (user!=NULL && !user->filepos) {	
  if (user->ignall==1 && !user->has_ignall) {
    user->has_ignall=1;
   }			
  user->ignall=2;	
  user->pageno=0;
  user->miscoptime=thour*3600+tmin*60+tsec;
 }


  if ((result=mysql_result(queryname))) {
    if (!(row=mysql_fetch_row(result))) { 
      mysql_free_result(result);
      user->filepos=0;
      user->ignall=user->has_ignall;
      user->has_ignall=0;
      return 0;
     }
   }
  else {
    user->filepos=0;
    user->ignall=user->has_ignall;
    user->has_ignall=0;
    return 0;
   }

/* (!(fp=ropen(filename,"r"))) {
      if (user!=NULL) {
        user->filepos=0;
      	user->ignall=user->has_ignall;
      	user->has_ignall=0;
      	}
      return 0;
     }
*/
/* dlzka suboru */
/* if (stat(filename,&stbuf)==-1) fsize=0; */
/* else fsize=stbuf.st_size; */
fsize=strlen(row[0]);
    
/* jump to reading posn in file */
/* if (user!=NULL) fseek(fp,user->filepos,0); */
if (user!=NULL) pos=user->filepos;

text[0]='\0';
/* c1[0]='\0'; */
/* tmptxt[0]='\0'; */
/* text4[0]='\0'; */
buffpos=0;
buffold=0;
num_chars=0;
retval=1;
len=0;
if (user!=NULL) totalines=user->lines-1;
	else totalines=23; /* R2 -> v irc mode je ich o dve menej! */ 

if ((user!=NULL) && (user->browsing==1) && (!user->filepos)) totalines-=1;
if ((user!=NULL) && (user->browsing==2) && (!user->filepos)) totalines-=2;
if ((user!=NULL) && (user->browsing==10) && (!user->filepos)) totalines-=2;


lines=0;
text[0]='\0';
i=0;
while (row[0][pos]!='\n') { 
  text[i]=row[0][pos];
  if (row[0][pos]=='\0') { eor=1; break; }
  pos++;i++;
 }
pos++;text[i]='\n';text[i+1]='\0';

/* Go through file */
if (user!=NULL && user->skip>0) {
  /* if (user->skip==-1) max=1000000; */
  max=totalines*user->skip;
  user->skip--;
 }
else max=totalines;
while(!eor && (lines<max || user==NULL)) {

if (lines>0 && (lines)%totalines==0 && user!=NULL && user->skip>0) {
  user->skip--;
  if (user->skip>0) silent=1;
  else silent=0;
  user->filepos+=num_chars;
  if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
    sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v more()!\n");
    write_syslog(text,1);
    /* fclose(fp); */
    mysql_free_result(result);
    return 2;
   }
  user->pageno++;
  pom->filepos=user->filepos;
  pom->messnum=user->messnum;
  pom->next=user->pager;
  user->pager=pom;
  /* c1[0]='\0'; */
  /* tmptxt[0]='\0'; */
  /* text4[0]='\0'; */
  buffpos=0;
  buffold=0;
  num_chars=0;
  retval=1;
  len=0;
 }

hajlajt=0;
if (user!=NULL && user->findstr[0])
  if (strstr(text,user->findstr)) hajlajt=1;
 
/* if (user!=NULL) force_language(text,user->lang,1); */
/* else force_language(text,0,1); */
len=strlen(text);
num_chars+=len;


      tmp2=&text[0];
      while(*tmp2) {
        if (*tmp2=='\r' && *(tmp2+1)!='\n')
	 strcpy(text,tmp2+1);
	tmp2++;
       }
      if (hajlajt) {
        tmp2=strstr(text,user->findstr);
        memmove(tmp2+3,tmp2,strlen(tmp2)+1);
        strncpy(tmp2,"~BR",3);
        tmp2+=strlen(user->findstr)+3;
        memmove(tmp2+3,tmp2,strlen(tmp2)+1);
        strncpy(tmp2,"~BK",3);
       }
      str=text;
      /* if (user!=NULL) force_language(str,user->lang,1); */

      /* Process line from file */
      if (user!=NULL
      && (silent || ((user->browsing==1 || user->browsing==2 || user->browsing==3)
      && (user->messnum < user->rjoke_from)))
      ) { }
 	 else {
           if (user!=NULL && (user->statline==CHARMODE) && (user->newline)) {
              *(buff+buffpos)='\r';
              *(buff+buffpos+1)='\n';    /* CR/LF! */
              buffpos+=2;
              user->newline=0;
             }

 	   while(*str) {
            if (*str=='\n') {
                  *(buff+buffpos)='\0';
                  if (user!=NULL && user->ignword[0] && strstr(buff,user->ignword)) buffpos=buffold;
                  else buffold=buffpos;
                  if (buffpos>OUT_BUFF_SIZE-8) {
                        write2sock(user,sock,buff,buffpos);  
                        buffpos=0;
                        buffold=0;
                        }
                  /* Reset terminal before every newline */
                  if (user!=NULL && user->colour) {
                        if (!webuser) { memcpy(buff+buffpos,"\033[0m",4);  buffpos+=4; }
                       }

                  if (user!=NULL && (user->statline==CHARMODE) &&
                       (*(str+1)==0)) {
                        user->newline=1; ++str;
                        }
                  else {
                  *(buff+buffpos)='\r';  *(buff+buffpos+1)='\n';  /* CR/LF! */
                   buffpos+=2;  ++str;
                     }
                  }
            else {
                  /* Process colour commands in the file. See write_user()
		     function for full comments on this code.
		     ZMENA - zmenil som '/' na '~' aby sa dalo pisat URL /~xy */
		  /* if (*str=='~' && *(str+1)=='~') {  ++str;  continue;  } */
		  if (*str>=127) *str=' '; /* UMBA */
		  /* if (str!=text && *str=='~' && *(str-1)=='~') {
			*(buff+buffpos)=*str;  goto CONT;
			} */
		  if (!webuser && *str=='~') {		  
  			if (buffpos>OUT_BUFF_SIZE-8) {
                          write2sock(user,sock,buff,buffpos);  
			  buffpos=0;
                          buffold=0;
	                 }
			++str;
			for(i=0;i<NUM_COLS;++i) {
			      if (!strncmp(str,colcom[i],2)) {
                                    if ((str-1)!=text && *(str-2)=='~') {
                                      *(buff+buffpos)=*str;
                                      goto CONT; 
                                     }
				    if (user!=NULL && user->colour && !(user->colour==2 && !strcmp(colcom[i],"LI"))
                                    && !(user->ignbeep && !strcmp(colcom[i],"LB"))
                                    && !(user->ignblink && !strcmp(colcom[i],"LI"))) {
					  memcpy(buffpos+buff,colcode[i],strlen(colcode[i]));
					  buffpos+=strlen(colcode[i])-1;
					  }
				    else buffpos--;
				    ++str;
				    goto CONT;
				    }
			      }
			*(buff+buffpos)=*(--str);
			}
		  else *(buff+buffpos)=*str;
		  CONT:
		  ++buffpos;   ++str;
		  }
	    if (buffpos==OUT_BUFF_SIZE) {
                  write2sock(user,sock,buff,OUT_BUFF_SIZE);
		  buffpos=0;
                  buffold=0;
		  }
	    }
	  }
      len-=(colour_com_count(text));
      if (user!=NULL && (user->browsing==1 || user->browsing==2 || user->browsing==3) && (user->messnum < user->rjoke_from)) { }
      	else lines+=len/80+(len<80);
        text[0]='\0';i=0;
        while (row[0][pos]!='\n') {
          text[i]=row[0][pos];
          if (row[0][pos]=='\0') { eor=1; break; }
          pos++;i++;
         }
        pos++;text[i]='\n';text[i+1]='\0';
        /* fgets(text,sizeof(text)-10,fp); */
      }
if (buffpos && sock!=-1) {
  write2sock(user,sock,buff,buffpos);  
 }
/* if user is logging on dont page file */
if (user==NULL) {   mysql_free_result(result);  return 2;  };

if (eor) {
      user->filepos=0;  no_prompt=0;  retval=2;
      zrus_pager_haldu(user);          
      if (user!=NULL) {
      	user->ignall=user->has_ignall;      
      	user->has_ignall=0;
      	if ((user->browsing==2) || (user->browsing==10)) user->lastm=user->messnum;
      	if ((user->browsing==2) || (user->browsing==10)) write_user(user,"~FM====+~OL+~RS~FM+==================================================================+~OL+~RS~FM+====\n");
      	if (user->browsing==3) write_user(user,"~OL~FM=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
      	if (user->browsing==1) write_user(user,"~FM<~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=~RS~FM><~OL=\n");
      	user->browsing=0;
        
        if (user->skip>0) {
          sprintf(text,"                              ~BB~OL~FW Pocet stranok: ~FY%d ~BK\n",user->pageno+1);
          write_user(user,text);
         }
        user->skip=0;
      	if (user->newtell) {
             /*sprintf(text,"~OL~LI~FTPrisli ti nove telly (%d), pouzi .revtell~LB~RS\n",user->newtell);
             write_user(user,text);*/
             show_recent_tells(user);
             user->newtell=0;
             user->tellpos=0;
            }
        show_recent_notifies(user);
      	}
      }
else  {
      /* store file position and file name */
      user->filepos+=num_chars;      
      
      if ((pom=(struct pager_struct *) malloc (sizeof(struct pager_struct)))==NULL) {
        sprintf(text,"ALLOC: Nemozno alokovat do zasobniku v more()!\n");
        write_syslog(text,1);
        return 2;
       }
      user->pageno++;
      pom->filepos=user->filepos;
      pom->messnum=user->messnum;
      pom->next=user->pager;
      user->pager=pom;
      
      strcpy(user->page_file,queryname);
      if (fsize) fperc=(int)(user->filepos*100)/fsize;
      else fperc=0;
      
      if (user->skip>0) { 
        user->pageno++;
        /* fclose(fp); */
        mysql_free_result(result);
        return sqlmore(user,sock,queryname);
       }
      sprintf(text,"~BB~OL~FT[%d%%] ~FWstr:~FT %d, ~FYENTER~FW-dalej, ~FYB~FW-spat, ~FYR~FW-znovu, ~FYn~FW/~FY-n~FW stran, ~FY/~FWhladat, ~FYE~FW-koniec~RS~BK ",fperc,user->pageno);
      write_user(user,text);
      if (user->statline==CHARMODE) user->newline=1;
      no_prompt=1;
     }
mysql_free_result(result);
/* fclose(fp); */
return retval;
}

/*** Set global vars. hours,minutes,seconds,date,day,month,year ***/
void set_date_time()
{
struct tm *tm_struct; /* structure is defined in time.h */
time_t tm_num;

/* Set up the structure */
time(&tm_num);
tm_struct=localtime(&tm_num);

/* Get the values */
tday=tm_struct->tm_yday;
tyear=1900+tm_struct->tm_year; /* Will this work past the year 2000? Hmm... */
tmonth=tm_struct->tm_mon;      /* Why not ? */
tmday=tm_struct->tm_mday;
twday=tm_struct->tm_wday;
thour=tm_struct->tm_hour;
tmin=tm_struct->tm_min;
tsec=tm_struct->tm_sec;
}

/*** Return pos. of second word in inpstr ***/
char *remove_first(inpstr)
char *inpstr;
{
char *pos=inpstr;
while(*pos<33 && *pos) ++pos;
while(*pos>32) ++pos;
while(*pos<33 && *pos) ++pos;
return pos;
}

/* Tu bola funkcia get_user ktoru som presunul (S) */


/* (S) toto vrati len ked je zadane PRESNE meno! */
UR_OBJECT get_user_exact(name)   
char *name;
{
UR_OBJECT u;

name[0]=toupper(name[0]);
/* Search for exact name */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->login || u->type!=USER_TYPE || u->room==NULL) continue;
      if (!strcasecmp(u->name,name)) {
        if (u==guest.user && !guest.getuser) return NULL;
        else { strcpy(name,u->name); return u; }
       }
      }
return NULL;
}




/*** Get room struct pointer from abbreviated name ***/
RM_OBJECT get_room(char *name,UR_OBJECT user)
{
RM_OBJECT rm;

if (user==NULL || (user!=NULL && sk(user))) {
  for(rm=room_first;rm!=NULL;rm=rm->next)
    if (!strncmp(rm->name,name,strlen(name))) return rm;
 }
else {
  for(rm=room_first;rm!=NULL;rm=rm->next)
    if (!strncmp(rm->name_en,name,strlen(name))) return rm;
  for(rm=room_first;rm!=NULL;rm=rm->next)
    if (!strncmp(rm->name,name,strlen(name))) return rm;
 }
return NULL;
}

RM_OBJECT get_linked_room(char *name,RM_OBJECT room)
{
RM_OBJECT rm;
int i;

for(rm=room_first;rm!=NULL;rm=rm->next) {
  for(i=0;i<MAX_LINKS;++i)
   if(rm->link[i]==room && !strncmp(rm->name,name,strlen(name))) return rm;
 }
return NULL;
}


/*** Return level value based on level name ***/
int get_level(name)
char *name;
{
int i;

i=0;
while(level_name[i][0]!='*') {
      if (!strcmp(level_name[i],name)) return i;
      ++i;
      }
return -1;
}


/*** See if a user has access to a room. If room is fixed to private then
      it is considered a wizroom so grant permission to any user of WIZ and
      above for those. Tu bude ZMENA!! ***/
int has_room_access(user,rm)
UR_OBJECT user;
RM_OBJECT rm;
{
if (rm->access==GOD_PRIVATE) return 0;
if ((rm->access & PRIVATE)
    && user->level<gatecrash_level
    && user->invite_room!=rm
    && !((rm->access & FIXED) && user->level>=WIZ)) return 0;
return 1;
}



/*** See if string contains any swearing ***/
int contains_swearing2(str)
char *str;
{
char *s;
int i,pocet=0;

if ((s=(char *)malloc(strlen(str)+2))==NULL) {
      write_syslog("CHYBA: Zlyhala alokacia pamate pri contains_swearing().\n",0);
      return 0;
      }
strcpy(s,str);
strtolower(s);
i=0;
while(swear_words[i][0]!='*') {
      if (strstr(s,swear_words[i])) pocet++;
      ++i;
      }
free(s);
return pocet;
}


/*** See if string contains any swearing - RIDER VERSION!***/
int contains_swearing(str,user)
char *str;
UR_OBJECT user;
{
char *s, *x;
int i,retval,o,skip;
unsigned int ss;
retval=0;

if ((s=(char *)malloc(strlen(str)+2))==NULL) {
      write_syslog("CHYBA: Zlyhala alokacia pamate pri contains_swearing().\n",0);
      return 0;
      }
strcpy(s,str);
colour_com_strip(s);
strtolower(s);
i=0;
while(swear_words[i][0]!='*') {
      if ((x=strstr(s,swear_words[i]))!=NULL) {
          o=0;
          skip=0;
          while (exceptions[o][0]!='*') {
            if (!strcmp(exceptions[o],swear_words[i]) && strstr(s,exceptions[o+1]))
             { skip=1; break; }
            o+=2;
           }
          if (skip==1) { i++; continue; }
          /* x++; */
          for (ss=0;ss<strlen(swear_words[i]);ss++) *((x++)-s+str)='.';
          strcpy(s,str);
          strtolower(s);
	  retval++;
          }
      else ++i;
      }
free(s);
/* if (str[strlen(str)-1]=='~') str[strlen(str)-1]='.'; */
if (com_num==SHOUT || com_num==SEMOTE || com_num==SECHO) user->shoutswears+=retval;
else user->sayswears+=retval;
return retval;
}

int contains_advert(str)
char *str;
{
char *s;
int i;

if ((s=(char *)malloc(strlen(str)+1))==NULL) {
      write_syslog("CHYBA: Zlyhala alokacia pamate pri contains_advert().\n",0);
      return 0;
      }
strcpy(s,str);
strtolower(s);
i=0;
while(advert_words[i][0]!='*') {
      if (strstr(s,advert_words[i])) {  free(s);  return 1;  }
      ++i;
      }
free(s);
return 0;
}

/*** Count the number of colour commands in a string ***/
int colour_com_count(str)
char *str;
{
char *s;
int i,cnt;

s=str;  cnt=0;
while(*s) {
     if (*s=='~') {
          /* if (*(s+1)=='~') {  s+=2;  continue;  } */
          /* if (s!=str && *(s)=='~' && *(s-1)=='~') { s++; continue; } */
          ++s;
          for(i=0;i<NUM_COLS;++i) {
               if (!strncmp(s,colcom[i],2)) {
                 if ((s-1)!=str && *(s-2)=='~') {
                   s++;
                   cnt++;
                  }
                 else {
                   cnt+=3;
                   s+=2;
                  }
                 break; /*Sorry Rider, ziadne GOTO! ;>*/
                }			  /* Chcscipni! ;> */
             }
          continue;
          }
     ++s;
     }
return cnt;
}

/* FIX ASAP */
/*** Strip out colour commands from string for when we are sending strings
     over a netlink to a talker that doesn't support them ***/
void colour_com_strip(str) /*LOKYDZR bolo char *colour_com_strip(str)*/
char *str;
{
char *s,*t;
/* static char text2[ARR_SIZE]; */
int i;
char *text2;

text2=(char*) malloc ((strlen(str)+1)*sizeof(char));
if (text2!=NULL) {
  s=str;  t=text2;
  while(*s) {
      if (*s=='~') {
            ++s;
            for(i=0;i<NUM_COLS;++i) {
                  if (!strncmp(s,colcom[i],2)) {
                    if ((s-1)!=str && *(s-2)=='~') {
                      *t++='~';
                      *t++=*s;
                      s++;
                      *t++=*s;
                      goto CONT;
                     }
                    s++;
                    goto CONT;
                   }
                  }
            --s;  *t++=*s;
            }
      else *t++=*s;
      CONT:
      s++;
      }
  *t='\0';
  /* sprintf(str,"%s",text2); */
  strcpy(str,text2);
  free(text2);
 }
else {
  sprintf(text,"~OL~FROUT OF MEMORY IN COLOUR_COM_STRIP()\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
}

char *colour_code_show(char *str)
{
char *s,*t;
static char text2[ARR_SIZE];
int i;

s=str;
t=text2;
while(*s) {
  if (*s=='~') {
    ++s;
    for(i=0;i<NUM_COLS;++i) {
      if (!strncmp(s,colcom[i],2)) {
        *t++='~';
        break;
       }
     }
    --s;
   }
  *t++=*s;
  s++;
 }
*t='\0';
return text2;
}

void force_language(char *str,int lang,int stripc)
{
char *s,*t;
int i,buffpos;
char *text2;

if (lang<0 || LANGUAGES-1<lang) lang=0;
text2=(char*) malloc ((strlen(str)+200)*sizeof(char));
if (text2!=NULL) {
  s=str;  t=text2;
  buffpos=0;
  while(*s) {
    if (stripc && *s=='\252') {
      s++;
      s++;
     }
    else if (*s=='\253') {
      ++s;
      for(i=0;strcmp(langword[i*(LANGUAGES+1)],"*");++i) {
        /* if () break; */
        if (!strncmp(s,langword[i*(LANGUAGES+1)],2))
         { 
          memcpy(t+buffpos,langword[i*(LANGUAGES+1)+1+lang],strlen(langword[i*(LANGUAGES+1)+1+lang]));
          buffpos+=strlen(langword[i*(LANGUAGES+1)+1+lang]);
         }
       }
      ++s;
     }
    else 
     { *(t+buffpos)=*s; buffpos++; }
    s++;
   }
  *(t+buffpos)='\0';
  strcpy(str,text2);
  free(text2);
 }
else {
  sprintf(text,"~OL~FROUT OF MEMORY IN FORCE_LANGUAGE()\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
}

/* vyrazi ~LB */
void wash_bell(str)
char *str;
{
char *s,*t;
/* static char text2[ARR_SIZE]; */
/* int i; */
char *text2;

text2=(char*) malloc ((strlen(str)+1)*sizeof(char));
if (text2!=NULL) {
  s=str;  t=text2;
  while(*s) {
      if (*s=='~') {
            ++s;
            if (!strncmp(s,"LB",2)) {  s++;  goto CONT;  }                  
            --s;  *t++=*s;
            }
      else *t++=*s;
      CONT:
      s++;
      }
  *t='\0';

  /* sprintf(str,"%s",text2); */
  strcpy(str,text2);
  free(text2);
 }
else {
  sprintf(text,"~OL~FROUT OF MEMORY IN WASH_BELL\n");
  write_level(KIN,1,text,NULL); 
  colour_com_strip(text);
  write_syslog(text,1);
 }
}


char *colour_com_strip2(str,usercol) 
char *str;
int usercol;
{
char *s,*t;
static char text2[ARR_SIZE];
int i;

s=str;  t=text2;
while(*s) {
      if (usercol && *s=='\252') { s++; s++; s++; }
      if (*s=='~') {
            ++s;
            for(i=0;i<NUM_COLS;++i) {
                  if (!strncmp(s,colcom[i],2)) {
                    if ((s-1)!=str && *(s-2)=='~') {
                      *t++='~';
                      *t++=*s;
                      s++;
                      *t++=*s;
                      goto CONT;
                     }
                    s++;
                    goto CONT;
                   }
                  }
            --s;  *t++=*s;
            }
      else *t++=*s;
      CONT:
      s++;
      }
*t='\0';
return text2;
}

char *sqldatum(char *str,int telnet_cols,int type) 
{
static char dejt[100];
int mi=0,ho=0,da=0,mo=0,ye=0;
 
 if (strlen(str)<19) return "";
 sscanf(str,"%4d-%2d-%2d %2d:%2d",&ye,&mo,&da,&ho,&mi);
 if (type==0) {
   switch (telnet_cols) {
    case 0: 
     sprintf(dejt,"%2d.%2d.%4d o %02d:%02dh",da,mo,ye,ho,mi);
    break;
    case 2:
     sprintf(dejt,"\033[1m\033[37m%2d.%2d.%4d \033[0m\033[33mo \033[1m\033[37m%02d:%02dh",da,mo,ye,ho,mi);
    break;
    case 1: 
     sprintf(dejt,"~HW%2d.%2d.%4d ~DYo ~HW%02d:%02dh",da,mo,ye,ho,mi);
    break;
   }
  }
 else {
   switch (telnet_cols) {
    case 0:
     sprintf(dejt,"%s %2d.%2d.%4d o %02d:%02dh",day[type-1],da,mo,ye,ho,mi);
    break;
    case 2: 
     sprintf(dejt,"%s \033[1m\033[37m%2d.%2d.%4d \033[0m\033[36mo \033[1m\033[37m%02d:%02dh",day[type-1],da,mo,ye,ho,mi);
    break;
    case 1:
     sprintf(dejt,"%s ~HW%2d.%2d.%4d ~DTo ~HW%02d:%02dh",day[type-1],da,mo,ye,ho,mi);
    break;
   }
  }
 return dejt;
}

/*** Date string for board messages, mail, .who and .allclones ***/
char *long_date(which)
int which;
{
static char dstr[80];

if (which==4) {
	sprintf(dstr,"%2d.%2d.%4d",tmday,tmonth+1,tyear);
	return dstr;
	}

if (which==3) {
      sprintf(dstr,"%d. %d. %d  %02d:%02d",tmday,tmonth+1,tyear,thour,tmin);
      return dstr;
      }
if (which==2) {
      sprintf(dstr,"Dnes je %s %d. %s %d. Meniny ma %s.\n~OLGong oznamil %02d:%02d:%02d (skoro) presne ;).\n~OLPocet sekund od 1.1.1970: %d",day[twday],tmday,month[tmonth],tyear,meniny(tmday,tmonth),thour,tmin,tsec, (int)time(0));
      return dstr;
      }
if (which) sprintf(dstr,"on %s %d %s %d o %02d:%02d",day[twday],tmday,month[tmonth],tyear,thour,tmin);
else sprintf(dstr,"[ %s %d %s %d o %02d:%02d ]",day[twday],tmday,month[tmonth],tyear,thour,tmin);
return dstr;
}

/* ZMENA Meniny - ukaze, kto ma v dany den podla slovenskeho kalendara meniny  ;-)
   UPRAVA Buko - 2 parametre mday a mmonth! */

char *meniny(mday,mmonth)
int mday,mmonth;
{
static char men[81];
FILE *fp;
char fname[81];
char line[81];
char denmes[81];

sprintf(denmes,"%02d.%02d",mday, mmonth+1);
sprintf(fname, "%s",MENINY_FILE);

if (!(fp=ropen(fname,"r"))) { /*APPROVED*/
			     sprintf(text,"Nemozem najst subor s meninami!\n");
			     write_syslog(text,0);
			     sprintf(men,"Mr. Nobody");			     
                             return men;
                             }
                            
while (!feof(fp)) {
	fgets(line, 80, fp);
	if(strstr(line, denmes)) { 
		fgets(line,80,fp); strcpy(men,line);
		men[strlen(men)-1]=0;
		 fclose(fp);
		return men; 
		}
	}
 fclose(fp);
sprintf(men,"Mr. Nobody");
return men;
}

/*** Clear the review buffer in the room ***/
void clear_revbuff(rm)
RM_OBJECT rm;
{
int c;
for(c=0;c<REVIEW_LINES;++c) if (rm->revbuff[c]!=NULL) {
	free ((void *) rm->revbuff[c]);
	rm->revbuff[c]=NULL;
	}
rm->revline=0;
}


/*** Clear the screen ***/
void cls(user, howmuch)
UR_OBJECT user;
int howmuch;
{
int i,c;

if (user->level>KIN && !strcmp(word[1],"gossip")) {
  for(c=0;c<REVIEW_LINES;++c) revgossipbuff[c][0]='\0';
  revgossipline=0;
  write_user(user,"~OL~FRGossip buffer zlikvidovany.\n");
  return;
 }
if (user->level>KIN && !strcmp(word[1],"shout")) {
  for(c=0;c<REVIEW_LINES;++c) revshoutbuff[c][0]='\0';
  revshoutline=0;
  write_user(user,"~OL~FRShout buffer zlikvidovany.\n");
  return;
 }
if (!howmuch) howmuch=user->lines;
for(i=0;i<howmuch;++i) write_user(user,"\n");
}


/*** Convert string to upper case ***/
void strtoupper(str)
char *str;
{
while(*str) {  *str=toupper(*str);  str++; }
}


/*** Convert string to lower case ***/
void strtolower(str)
char *str;
{
while(*str) {  *str=tolower(*str);  str++; }
}


/*** Returns 1 if string is a positive number ***/
int is_number(char *str)
{
while(*str) if (!isdigit(*str++)) return 0;
return 1;
}


/************ OBJECT FUNCTIONS ************/

/*** Construct user/clone object ***/
UR_OBJECT create_user()
{
UR_OBJECT user;
int i;

hb_can=0;
if ((user=(UR_OBJECT)malloc(sizeof(struct user_struct)))==NULL) {
      write_syslog("CHYBA: Zlyhanie alokacie pamate pri create_user().\n",0);
      return NULL;
      }

/* Append object into linked list. */
if (user_first==NULL) {
      user_first=user;  user->prev=NULL;
      }
else {
      user_last->next=user;  user->prev=user_last;
      }
user->next=NULL;
user_last=user;

/* initialise user structure */
user->type=USER_TYPE;
user->visitor=0;
user->zaradeny=0;
user->pp=0; /*PP*/
user->mana=0; /* SPELL */
user->name[0]='\0';
user->desc[0]='\0';
user->prevname[0]='\0';
user->in_phrase[0]='\0';
user->out_phrase[0]='\0';
user->email[0]='\0';        /* Baxa na to!!! */
user->lastcommand[0]='\0';
user->lastcommand2[0]='\0';
user->pridavacas=1;
user->lastidle=0;
user->lastidle2=0;
user->lastidle3=0;
user->homepage[0]='\0'; 
user->afk_mesg[0]='\0';
user->pass[0]='\0';
user->wizpass[0]='\0';
user->site[0]='\0';
user->site_port=0;
user->last_site[0]='\0';
user->page_file[0]='\0';
user->mail_to[0]='\0';
user->inpstr_old[0]='\0';
user->buff[0]='\0';
strcpy(user->smsfollow,"-");
user->sex=2; /*SEX*/
user->buffpos=0;
user->filepos=0;
user->pageno=0;
user->read_mail=time(0);
user->room=NULL;
user->invite_room=NULL;
user->port=0;
user->login=0;
user->socket=-1;
user->attempts=0;
user->command_mode=0;
user->level=0;
user->vis=1;
user->ignall=0;
user->ignall_store=0;
user->ignshout=0;
user->ignsys=0;
user->ignore=0;
user->ignfun=0;
user->igntell=0;
user->ignbeep=0;
user->ignzvery=0;
user->ignblink=0;
user->ignspell=0;
user->ignpict=0;
user->muzzled=0;
user->muzzletime=0;
user->muzzle_t=-1;
user->jailed=0;
/* user->remote_com=-1; */
user->last_input=time(0);
user->last_login=time(0);
user->last_login_len=0;
user->total_login=0;
user->idletime=0;
user->first_login=0;
user->prompt=prompt_def;
user->colour=colour_def;
user->charmode_echo=charecho_def;
user->misc_op=0;
user->edit_op=0;
user->edit_line=0;
user->charcnt=0;
user->warned=0;
user->accreq=0;
user->afk=0;
user->revline=0;
user->revircline=0;
user->clone_hear=CLONE_HEAR_ALL;
user->malloc_start=NULL;
user->malloc_end=NULL;
user->owner=NULL;
user->lasttellfrom[0]='\0'; /*LASTEL*/
user->lasttellto[0]='\0';
user->lasttg=-1;
user->follow[0]='\0'; /* koho luzer followuje */
user->newpass[0]='\0';
user->chanp[0]='\0';
user->has_ignall=0;
user->alarm=-1;
user->uname[0]='\0';     
user->game=0;
user->joined=0;
user->moves=1;
user->last_x=0;
user->last_y=0;
user->tah=0;      
user->hang_word[0]='\0';
user->hang_word_show[0]='\0';
user->hang_guess[0]='\0';
user->hang_stage=-1;
user->lines=24;
user->storelines=0;
user->colms=80;
user->wrap=0;
user->who_type=1;
user->rt_on_exit=0;
for (i=0;i<BODY;i++) user->predmet[i]=-1;
for (i=0;i<BODY;i++) user->dur[i]=0;
user->zuje=0;
user->glue=0;
user->call[0]='\0';
user->prehana=0;
user->prehana_t=0;
user->prehana_t2=0;
user->stars=0;
user->lsd=0;
user->zuje_t=0;
user->lieta=0;
user->viscount=0;
user->doom=0; /* D00M */
user->doom_energy=0;
user->doom_score=0;
user->doom_ammo[0]=0;
user->doom_ammo[1]=D_AMMO;
user->doom_ammo[2]=1;
user->doom_weapon=0;
user->doom_loading=0;
user->doom_heading=0;
user->doom_x=0;
user->doom_y=0;
user->doom_sayon=0;

user->shmode=0;
user->shbuild=0;
user->shstatus=0;
user->newtell=0;
user->browsing=0;
user->messnum=0;
user->lab=0;
user->afro=0;
user->real_ident[0]='\0';
user->subject[0]='\0';
user->where[0]='\0';

for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
for (i=0; i<10; i++) user->channels[i][0]='\0';

user->lastm=0;
user->autofwd=0;
user->pagewho=0;
user->examine=1;
user->com_counter=MAX_COMMANDS;
user->igngossip=1;
user->quest=0;
user->macrolist=NULL; /* M@CRO */
user->pager=NULL;
user->flood=0;
for(i=0;i<REVTELL_LINES;i++) user->revbuff[i]=NULL;
for(i=0;i<REVIRC_LINES;i++) user->revirc[i]=NULL;
user->dead=0;
user->prompt_string[0]='\0';
user->rjoke_from=1;
user->ignportal=0;
user->ignword[0]='\0';
user->saturate=0;
user->idle=0;
user->autosave=AUTOSAVE;
for(i=0;i<HISTORY_LINES;i++) user->history[i][0]='\0';
user->histpos=0;
user->lang=0; /* llang */

user->remote_login=0;
user->remote_login_socket=0;
user->actual_remote_socket=0;
user->remote_name[0]='\0';
user->remote_passwd[0]='\0';
for(i=0;i<MAX_CONNECTIONS;++i) {
	user->remote_socket[i]=0;
        user->remote[i]=NULL;
        user->remote_fds[i]=0;
        }
user->ircsocknum=0;
user->akl=0;
user->cloak=0;
user->irc_chan[0]='\0';
user->irc_nick[0]='\0';
user->irc_defnick[0]='\0';
user->irc_serv[0]='\0';
user->lynx=0;
user->sayflood=0;
user->irc_reg=0;
user->irc_name[0]='\0';
user->statline=NONEMODE;
user->statlcount=0;
user->statlcan=0;
user->newline=0;
user->bj_game=NULL;
user->ignio=0;
user->ignlook=0;
user->remtelopt=0;
reversi_koniechry(user);
user->dama=NULL;
user->dama_opp=NULL;
user->dhrac=0;
user->miny_tab=NULL;
user->chrac=0;
user->clovece=NULL;
user->ipcka[0]='\0';

user->fhrac=-1;
user->farar=NULL;
user->farar_starter=NULL;
for(i=0;i<7;++i) user->col[i]=0;
for(i=0;i<11;++i) user->commused[i]=0;
user->reserved[0]='\0';
user->carry=0;
user->igncoltell=0;
user->cps=1;
user->goafkafter=10;
user->killmeafter=120;
user->ignafktell=1;
user->igngames=0;
user->ignautopromote=0;
user->ignportalis=1;
user->mailnotify=0;
user->totaljailtime=0;
user->totalmuzzletime=0;
user->nontimejails=0;
user->killed=0;
user->gold=0;
user->shoutswears=0;
user->sayswears=0;
user->remote_ident[0]='\0';
user->hangups=0;
user->hangupz=0;
/* if (random()%10!=0) user->hangups=random()%200; */ /* 4 testinq */
user->ignxannel=0;
user->xannel=NULL;
user->nox=NULL;
user->invite2xannel=NULL;
user->multiply=0;
user->lastrevt=0;
user->lastrevs=0;
user->lastrevi=0;
user->sell_to[0]='\0';
user->sell_what=-1;
user->sell_price=0;
user->ignnongreenhorn=0;
user->smsssent=0;
user->smsday=0;
user->allowsms=0;
user->smswait=0;
user->smsgate=0;
user->smschars=0;
user->mail2sms=0;
user->team=0;
user->kills=0;
user->deaths=0;
user->way=0;
user->into=NULL;
user->from=NULL;
user->ep_line=0;
user->affected=-1;
user->affecttime=0;
user->affpermanent=-1;
user->weapon=-1;
user->hidden=0;
user->reveal=0;
strcpy(user->mobile,"-");
user->shortcutwarning=0;
user->switches=0;
user->temp=0;
user->wizactivity=0;
user->age=0;
user->agecode=0;
user->longestsession=0;
strcpy(user->logoutmsg,"-");
user->timeinrevt=0;
user->miscoptime=0;
user->macro_num=0;
user->notify_num=0;
user->com_priority=0;
user->attacking=0;
user->health=100;
user->heal=0;
user->deathtype=-1;
user->id=0;
user->ap=0;
user->websecond=0;
user->skip=0;
user->special=0;
user->findstr[0]='\0';
user->shoutmsg[0]='\0';
user->shoutmsg2[0]='\0';
user->gossipmsg[0]='\0';
user->gossipmsg2[0]='\0';
user->saymsg[0]='\0';
user->saymsg2[0]='\0';
user->tellmsg[0]='\0';
user->tellmsg2[0]='\0';
user->wizshmsg[0]='\0';
user->wizshmsg2[0]='\0';
user->request[0]='\0';
user->sname[0][0]='\0';
user->sname[1][0]='\0';
user->sname[2][0]='\0';
user->exams=0;
user->kontrola=0;
user->messsize=0;
user->target=-1;

user->xml_writer = NULL;
user->xml_buffer = NULL;

user->output_format = OUTPUT_FORMAT_PLAIN;


hb_can=1;
return user;
}

/*** Destruct an object. ***/
void destruct_user(user)
UR_OBJECT user;
{
hb_can=0;
if (user==user_first) {
      user_first=user->next;
      if (user==user_last) user_last=NULL;
      else user_first->prev=NULL;
      }
else {
      user->prev->next=user->next;
      if (user==user_last) {
            user_last=user->prev;  user_last->next=NULL;
            }
      else user->next->prev=user->prev;
      }
user->next=NULL;

/* XML output */
destruct_user_xml(user);

free(user);	
destructed=1;
hb_can=1;
}

/*** Construct room object ***/
RM_OBJECT create_room()
{
RM_OBJECT room;
int i;

if ((room=(RM_OBJECT)malloc(sizeof(struct room_struct)))==NULL) {
      fprintf(stderr,"NUTS: Zlyhanie alokacie pamate pri create_room().\n");
      boot_exit(1);
      }
room->name[0]='\0';
room->name_en[0]='\0';
room->label[0]='\0';
room->desc_sk=NULL;
room->desc_en=NULL;
room->topic[0]='\0';
room->topicmaker[0]='\0';
room->access=-1;
room->revline=0;
room->sndproof=0;
room->mesg_cnt=0;
room->next=NULL;
room->logging=0;    /* ZMENA - pre zaciatok dame logging OFF */
room->invisible=0;
for(i=0;i<MAX_LINKS;++i) {
      room->link_label[i][0]='\0';  room->link[i]=NULL;
      }
for(i=0;i<REVIEW_LINES;++i) room->revbuff[i]=NULL;
if (room_first==NULL) room_first=room;
else room_last->next=room;
room_last=room;
return room;
}

/*** Destroy all clones belonging to given user ***/
void destroy_user_clones(user)
UR_OBJECT user;
{
UR_OBJECT u, next;
      
u=user_first;
while (u!=NULL) {
	next=u->next;
	if (u->type==CLONE_TYPE && u->owner==user) {
        	sprintf(text,"Klon %s sa rozplynul v prachu.\n", u->name);
            	write_room(u->room,text);
                destruct_user(u);
                }
        u=next;
        }
}

/*************** START OF COMMAND FUNCTIONS AND THEIR SUBSIDS **************/

/*** Deal with user input ***/
void exec_com(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,len,ii;
char filename[80],*comword=NULL;
int uber;
char multiple[20][80]; /* pluskovanie */
int p,q;
int ktory;
int koll=0;
int mozevykonat=0, odobraty=0;
char temp[WORD_LEN+1], pnv[80];
char histrecord[200];
char pstr[ARR_SIZE+5];
UR_OBJECT u;
NOTIFY bunka;

current_user=user;

 if (max_users_was>300) { /* DEBUG */
   sprintf(text,"max_users_was je %d! Posledny komand: %d\n",max_users_was,com_num);
   if (logcommands) log_commands("BULLSHIT",text,1);
   max_users_was=0;
  }
if (user->health<10) {
  write_user(user,"Umieras a nevladzes uz skoro ani hovorit..\n");
  return;
 }
com_num=-1;
if (word[0][0]=='.') comword=(word[0]+1);
else comword=word[0];
if (!comword[0]) {
      write_user(user,"\253UC.\n");
      sprintf(histrecord,"%02d:%02d -.-", tmin, tsec);
      record_history(user, histrecord);
      return;
      }
/* if (strchr("!@$<>-:/,",inpstr[0]) && inpstr[1]!=' ') */
if (strchr("/",inpstr[0]) && inpstr[1]!=' ')
/* za lomitkom sa dava medzera az po kontrole makier. (V) */
{
 pstr[0]=inpstr[0];
 pstr[1]=' ';
 sstrncpy (pstr+2,inpstr+1, ARR_SIZE-5);
 sstrncpy (inpstr,pstr, ARR_SIZE-2); 
 clear_words();
 word_count=wordfind(inpstr);
}

/* get com_num */
if (!strcmp(word[0],"/")) strcpy(word[0],"to");          /* ZMENA! */
if (!strcmp(word[0],">")) strcpy(word[0],"tell");
if (!strcmp(word[0],"<")) strcpy(word[0],"pemote");
if (!strcmp(word[0],"-")) strcpy(word[0],"echo");
if (!strcmp(word[0],"!")) strcpy(word[0],"shout");
if (!strcmp(word[0],"@")) strcpy(word[0],"gossip");
if (!strcmp(word[0],"$")) strcpy(word[0],"quest");
if (!strcmp(word[0],":")) strcpy(word[0],"reply");
if (!strcmp(word[0],"`")) { strcpy(word[0],"tell"); koll=2; }
if (inpstr[0]==';') strcpy(word[0],"emote");
else if (inpstr[0]=='#') strcpy(word[0],"semote");
      else inpstr=remove_first(inpstr);
if (!strcmp(word[0],",")) {
	strcpy(word[0],"tell");
	/* strcpy(word[1],user->call); */
	koll=1;
	}
/*if (!strcmp(word[0],"'")) {
	strcpy(word[0],"tell");
	strcpy(word[1],user->lasttellto);
	koll=2;
	}
*/
if (user->com_priority==0) { /* user uprednostnuje anglicke komandy a skratky */
  i=0;                  /* takze hladame anglicku skratku */
  ii=-1;
  len=strlen(comword);
  while(command[i][0]!='*' && (ii==-1 || com_num==-1)) { 
    if (!strncasecmp(command[i],comword,len)) {
      if (com_level[i]<=user->level && com_num==-1) { /* ak to je komand ktory je userovi dostupny */
        com_num=i;
        strcpy(pnv,command[i]);
       }
      if (ii==-1) ii=i; /* prvy komand vyhovujuci skratke aj ak nie je dostupny */
     }
    ++i;
   }
  if (com_num==-1 && ii>-1) { /* ak sa nasiel iba komand vysieho levelu, tak ho tam vrazime.. */
    com_num=ii;
    strcpy(pnv,command[ii]);
   }
  
  if (com_num==-1) { /* ak sa nic nenaslo, skusime slovenske prikazy */
    i=0;
    while(command_sk[i][0]!='*') {
      if (!strncasecmp(command_sk[i],comword,len)) {
        com_num=sk_to_en_com[i];
        strcpy(pnv,command[sk_to_en_com[i]]);
        break;
       }
      ++i;
     }
   }
 }
else { /* user ma prioritne slovenske, tak hladame ci to slovenska skratka */
  i=0;
  len=strlen(comword);
  while(command_sk[i][0]!='*') {
    if (!strncasecmp(command_sk[i],comword,len)) {
      com_num=sk_to_en_com[i];
      strcpy(pnv,command[sk_to_en_com[i]]);
      break;
     }
    ++i;
   }
  if (com_num==-1) { /* nenasiel sa slovensky prikaz, tak quqneme ci to nie je anglicky */
    i=0;                            
    ii=-1;
    len=strlen(comword);
    while(command[i][0]!='*' && (ii==-1 || com_num==-1)) {
      if (!strncasecmp(command[i],comword,len)) {
        if (com_level[i]<=user->level && com_num==-1) { 
          com_num=i;
          strcpy(pnv,command[i]);
         }
        if (ii==-1) ii=i;
       }
      ++i;
     }
    if (com_num==-1 && ii>-1) {
      com_num=ii;
      strcpy(pnv,command[ii]);
     }
   }
 }
if (logcommands) log_commands(user->name,comword,1);


if (com_num!=TELL && com_num!=PEMOTE && com_num!=SHOUT && com_num!=GOSSIP && com_num!=GEMOTE
  && com_num!=SEMOTE && com_num!=WIZSHOUT && com_num!=REPLY && com_num!=CALL
  && com_num!=NOTICE) {
	sprintf(histrecord,"%02d:%02d %s %s", tmin, tsec, comword, word[1]);
	}
	else sprintf(histrecord,"%02d:%02d %s", tmin, tsec, comword);

record_history(user, histrecord);

if (com_num!=DOOM && user->level<WIZ) { /* saturate protection */
	if (user->saturate<20) user->saturate++;
	if (user->saturate>3) return;
	}


if ((user->jailed) && (user->level<WIZ) && (com_num!=EXCUSE) && (com_num!=SAY)) {
	if (rand()%10==5) {
		write_user(user,"V zalari vladnu tvrde podmienky; prikaz sa ti nepodarilo vykonat.\n");
		return;
		}
	if (user->pp<50) {
		write_user(user,"V zalari to nieje jednoduche; minimum na vykonanie prikazu je 50 powerpointov!\n");
		return;
		}
	uber=rand()%20;
	if ((uber>=4) && (uber<=6)) uber=0;
	if (uber==15) {
		setpp(user,0);
		write_user(user,"Oops, strazca ta potrestal za tvoju neposlusnost, vsetky pp su v tahu!\n");
		}
	else {
	if (uber) {
	sprintf(text,"V zalari vladnu prisne pravidla; tento prikaz ta stal %d power-pointov.\n",uber);
	write_user(user,text);	
	setpp(user,user->pp-uber);
		}
	    }
	}

/* com_level[com_num] > user->level */
/* Tu sa rozhoduje ci MOZEVYKONAT ak ma zakazany prikaz */

if (com_level[com_num] > user->level) mozevykonat=0;
	else mozevykonat=1;
        
for(bunka=user->combanlist; bunka!=NULL; bunka=bunka->next)
 if (!strcmp(bunka->meno,pnv)) { mozevykonat=0; odobraty=1; }

if (user->room!=NULL && (com_num==-1 || !mozevykonat)) {
      if (odobraty) {
      	write_user(user,"Lutujem, ale tento prikaz ti bol odobrany!\n");
      	return;
      	}
      if (!odobraty && com_num!=-1 && com_level[com_num]<KIN) {
      		sprintf(text,"Tento prikaz je dostupny az od levelu %s.\n", level_name[com_level[com_num]]);
      		write_user(user,text);
      		return;
      	  	}
      write_user(user,"\253UC.\n");
      return;
      }
user->commused[0]++;
/* GUEEST */
if (user==guest.moderator) guest.getuser=1;
	else guest.getuser=0;
if (guest.on && !strcmp(user->room->name, GUEST_ROOM)) {
	if ((com_num==SAY || com_num==EMOTE || com_num==TO_USER
	     || com_num==THINK || com_num==SING || com_num==SIGN)
	     && user!=guest.talk && user!=guest.moderator && user!=guest.user) {
		write_user(user,"Nemas teraz pridelene slovo - pockaj az ti ho moderator prideli!\n");
		return;
		}
	if ((com_num==SHOUT || com_num==SEMOTE || com_num==ECHO ||
	     com_num==PUBCOM || com_num==PRIVCOM || com_num==TOPIC ||
	     com_num==READ || com_num==WRITE || com_num==WIPE || com_num==INVIS ||
	     com_num==FIX || com_num==UNFIX || com_num==COOLTALK ||
	     com_num==COOLTOPIC || com_num==COW || com_num==KIDNAP ||
	     com_num==SECHO || com_num==TELLALL || com_num==POET ||
	     com_num==BANNER || com_num==SBANNER || com_num==SNDPROOF ||
	     com_num==JOIN || com_num==GET || com_num==PUT || com_num==USE)
	    && user!=guest.moderator && user->level<GOD) {
	    	write_user(user,"Nemozes pouzit tento prikaz pri posedeni s hostom.\n");
	    	return;
	    	}
	}

/* ZUVACKA etc ... */
if ((user->zuje) || (user->affected>-1) || (user->affpermanent>-1)) {
  if ((com_num==SAY) || (com_num==SHOUT) || (com_num==REPLY)
   || (com_num==EMOTE) || (com_num==SEMOTE) || (com_num==ECHO) /* || (com_num==BCAST) */
   || (com_num==WRITE) || (com_num==SECHO) || (com_num==SIGN)
   || (com_num==TELLALL) || (com_num==PECHO) || (com_num==GOSSIP) || (com_num==GEMOTE)
   || (com_num==XSAY) || (com_num==THINK) || (com_num==SING))
    do_funct_stuff(user,inpstr,1);
  else if ((com_num==COOLTALK) || (com_num==HUG) || (com_num==WAKE)
   || (com_num==KISS) || (com_num==TELL) || (com_num==TO_USER)
   || (com_num==PEMOTE) || (com_num==CSAY))
    do_funct_stuff(user,inpstr,2);
 }


if (user->room!=NULL && user->room->group==4 && user->hidden==1) {
  if ((com_num==TELL) || (com_num==REPLY)
  || (com_num==PEMOTE) || (com_num==ECHO) || (com_num==PECHO)
  || (com_num==LOOK) || (com_num==EXAMINE) || (com_num==REVIEW)
  || (com_num==REVTELL) || (com_num==REVIRC) || (com_num==REVSHOUT)
  || (com_num==REVGOSSIP) || (com_num==MODE) || (com_num==PROMPT)
  || (com_num==WHO) || (com_num==NOTIFY_CMD) || (com_num==MACROCMD)
  || (com_num==HELP) || (com_num==NEWS) || (com_num==STATUS)
  || (com_num==ROOMS) || (com_num==CLS) || (com_num==CHARECHO)
  || (com_num==POMOC) || (com_num==TIME) || (com_num==RANKS)
  || (com_num==PRIKAZY) || (com_num==COMMANDS) || (com_num==FAQ)
  || (com_num==RULES) || (com_num==SET) || (com_num==PP)
  || (com_num==REMOTE) || (com_num==MAP) || (com_num==WIZZES)
  || (com_num==HIDE) || (com_num==REVBCAST) || (com_num==REVSOS)) {
   }
  else {
    sprintf(text,"~FTPrezradil%s si z ukrytu.\n",pohl(user," si sa a vysiel","a si sa a vysla"));
    write_user(user,text);
    sprintf(text,"~FT%s vyliez%s z ukrytu.\n",user->name,pohl(user,"ol","la"));
    write_room_except(user->room,text,user);
    user->hidden=0;
   }
 }

if (com_num==SHUTDOWN || com_num==REBOOT || com_num==SUICIDE) {
  police_freeze(user,10); 
  if (user->wizactivity>9) return;
 }

  /* pluskovanie */
if (com_num==TELL || com_num==TO_USER || com_num==PEMOTE
 || com_num==MOVE || com_num==KILL || com_num==WAKE
 || com_num==HUG || com_num==BOMB || com_num==KISS
 || com_num==FIGHT || com_num==NOTIFY_CMD || com_num==PICTELL
 || com_num==SMAIL) {

  p=0; ktory=1; q=0;
  for (p=1;p<=11;++p) {
    strcpy(multiple[p],"\n");
   }
  p=0;
  if (strlen(word[1])<100 && strstr(word[1],"+") && word[1][0]!='+' 
  && !strstr(word[1],"++") && word[1][strlen(word[1])-1]!='+') {
    strcpy(temp,word[1]);
    while (temp[p]!='\0') {
      if (temp[p]=='+') {
        p++;
        multiple[ktory][q]='\0';
        ktory++;
        u=get_user(multiple[ktory-1]);
        if (u!=NULL) {
          len=0;
          for(i=1;i<ktory-1;++i)
           if (get_user(multiple[i])==u) len=1;
          if (len==1) ktory--;
         }
        else ktory--;
        if (ktory>10) {
          write_user(user,"Mozes zadat maximalne 10 uzivatelov.\n");
  	  return;
         }
        q=0;
        continue;
       }
      multiple[ktory][q]=temp[p]; p+=1; q+=1;
      if (q>12) { write_user(user,"Prilis dlhe meno v zozname.\n"); return; }
     }
    multiple[ktory][q]='\0';
   }
  u=get_user(multiple[ktory]);
  if (u!=NULL) {
    len=0;
    for(i=1;i<ktory;++i)
      if (get_user(multiple[i])==u) len=1;
    if (len==1) ktory--;
   }
  else ktory--;
  if (ktory>0) {
/*    for(p=1;p<=ktory;++p)
     for(i=p+1;i<=ktory;++i)
      u=get_user(multiple[p]);
      if (!strcasecmp(multiple[p],multiple[i]) || (u!=NULL && u==get_user(multiple[i]))) 
       { write_user(user,"Nemozes zadat jedneho uzivatela viac krat.\n"); return; }*/
    if (com_num==SMAIL) { /* V) toto posle aj: .smail lamer+looser */
      for(p=2;p<=ktory;++p)
      strcpy(word[p-1],multiple[p]);
      p=word_count;
      word_count=ktory;
      copies_to(user);
      word_count=p;
      strcpy(word[1],multiple[1]);
      hb_can=0;
      smail(user,inpstr,0);
      hb_can=1;
      return;
     }
    for(p=1;p<=ktory;++p) {
      strcpy(word[1],multiple[p]);
      sprintf(text,"~RS~FY%s: ",multiple[p]);
      write_user(user,text);
      switch(com_num) {
        case TELL: tell(user,inpstr,0); break;
        case TO_USER : to_user(user,inpstr); break;
        case PEMOTE  : pemote(user,inpstr); break;
        case MOVE    : move(user);  break;
        case KILL    : kill_user(user,inpstr);  break;
        case WAKE    : wake(user,inpstr);  break;
        case HUG     : send_hug(user,inpstr);  break;
        case BOMB    : send_bomb(user); break;
        case KISS    : send_kiss(user,inpstr); break;
        case FIGHT   : fight_user(user); break;
        case NOTIFY_CMD: notify_user(user); break;
        case PICTELL : pictell(user,inpstr); break;
        /* case TBANNER   : tbanner(user); break;*/
        default: write_user(user,"Prepac, tomuto prikazu nemozes zadat viacero uzivatelov.\n"); return; break;
       }
     }
    return;
   }
 }
/* Main switch KOMMANDO*/
switch(com_num) {
      case QUIT: quit_user(user,inpstr);  break; 
      case LOOK: look(user); user->commused[7]++; break;
      case MODE: toggle_mode(user);  break;
      case SAY :
	    if (word_count<2) {
		  write_user(user,"Co chces povedat?\n");  return;
		  }
	    say(user,inpstr,1);
	    break;
      case TO_USER : to_user(user,inpstr); break;
      case SHOUT : shout(user,inpstr);  break;
      case TELL  : if (koll==1) tell(user,inpstr,2);
		   else if (koll==2) tell(user,inpstr,3);
		   else tell(user,inpstr,0);
		   break;
      case REPLY : tell(user,inpstr,1); break;
      case EMOTE : emote(user,inpstr);  break;
      case SEMOTE: semote(user,inpstr); break;
      case PEMOTE: pemote(user,inpstr); break;
      case ECHO  : echo(user,inpstr);   break;
      case GO    : go(user);  break;
      case PROMPT: toggle_prompt(user, inpstr);  break;
      case DESC  : set_desc(user,inpstr);  break;
      case INPHRASE :
      case OUTPHRASE:
	    set_iophrase(user,inpstr);  break;
      case PUBCOM :
      case PRIVCOM: set_room_access(user);  break;
      case LETMEIN: letmein(user);  break;
      case INVITE : invite(user);   break;
      case TOPIC  : set_topic(user,inpstr);  break;
      case MOVE   : move(user);  break;
      case BCAST  : bcast(user,inpstr,0);  break;
      case WHO    : if (user->who_type<1) newwho(user,user->name);
                    if (user->who_type==1) who(user,0);
		    if (user->who_type==2) who_alt1(user);
		    if (user->who_type==3) who_alt2(user);
		    if (user->who_type==4) who_alt3(user);
		    if (user->who_type==5) who_alt4(user);
		    if (user->who_type==6) who_alt5(user);
		    if (user->who_type==7) who_alt6(user);
		    if (user->who_type==8) who_alt7(user);
		    if (user->who_type==9) who_alt8(user);
		    if (user->who_type==10) who_alt9(user);
		    user->commused[5]++;
		    break;
      case PEOPLE : who(user,1);  break;
      case HELP   : help(user,0);  break;
      case POMOC  : help(user,0); break; /* ZMENA */
      case SHUTDOWN: shutdown_com(user);  break;
      case NEWS:
            sprintf(filename,"select body from files where filename='news'");
            switch(sqlmore(user,user->socket,filename)) {
                  case 0: write_user(user,"Ziadne novinky sa nenasli.\n");  break;
                  case 1: user->misc_op=222;
                  }
            break;
      case READ  : read_board(user);  break;
      case WRITE : hb_can=0;
                   write_board(user,inpstr,0);
                   hb_can=1;
      	           break;
      case WIPE  : wipe_board(user);  break;
      case SEARCH: search_boards(user);  break;
      case REVIEW: review(user,inpstr);  break;
      case STATUS: status(user);  break;
      case VER   : show_version(user); break;
      case RMAIL   : rmail(user);  break;                     
      case MYMAIL  : mymail(user,0);  break;
      case SMAIL   : hb_can=0;
      		     smail(user,inpstr,0);
      		     hb_can=1;
      		     break;
      case DMAIL   : hb_can=0;
      		     dmail(user);
      		     hb_can=1;
      		     break;
      case FROM    : mail_from(user,1);  break;
      case PROFILE : enter_profile(user,0);  break;
      case EXAMINE : if (user->examine==0) customexamine(user,user->name,0); else examine(user);  break;
      case ROOMS   : rooms(user); break;
      case PASSWD  : change_pass(user);  break;
      case KILL    : kill_user(user,inpstr);  break;
      case PROMOTE : promote(user,inpstr);  break;
      case DEMOTE  : demote(user,inpstr);  break;
      case LISTBANS: listbans(user);  break;
      case BAN     : ban(user, inpstr);  break;
      case UNBAN   : unban(user);  break;
      case VIS     : visibility(user,1);  break;
      case INVIS   : visibility(user,0);  break;
      case HIDE    : hide(user);  break;
      case SITE    : site(user);  break;
      case WAKE    : wake(user,inpstr);  break;
      case WIZSHOUT: wizshout(user,inpstr);  break;
      case MUZZLE  : muzzle(user);  break;
      case UNMUZZLE: unmuzzle(user);  break;
      case MAP     : show_map(user); break;                              
      case SYSTEM   : system_details(user,1);  break;
      case CHARECHO : toggle_charecho(user,inpstr);  break;
      case CLEARLINE: clearline(user);  break;
      case FIX      : change_room_fix(user,1);  break;
      case UNFIX    : change_room_fix(user,0);  break;
      case VIEWLOG  : viewlog(user, inpstr);  break;
      case ACCREQ   : account_request(user);  break;
      case REVCLR   : revclr(user);  break;
      case CLONE    : create_clone(user);  break;
      case DESTROY  : destroy_clone(user);  break;
      case MYCLONES : myclones(user);  break;
      case ALLCLONES: allclones(user);  break;
      case SWITCH: clone_switch(user);  break;
      case CSAY  : clone_say(user,inpstr);  break;
      case CHEAR : clone_hear(user);  break;
      case AFK   : afk(user,inpstr);  break;
      case EXEC      : exec_command(user,inpstr); break;
      case CLS   : cls(user,0);  break;
      case COLOUR  : toggle_colour(user);  break;
      case SUICIDE : suicide(user);  break;
      case DELETE  : delete_user(user,0);  break;
      case REBOOT  : reboot_com(user);  break;
      case RECOUNT : check_messages(user);  break;
      case REVTELL : revtell(user,inpstr); break;
      case PICTELL : pictell(user,inpstr); break; /* Odtialto dolu - same nove prikazy :) */
      case GODPRIV : set_room_access(user); break;
      case HUG     : send_hug(user,inpstr);  break;
      case BOMB    : send_bomb(user); break;
      case SOS     : send_sos(user, inpstr);  break;
      case COMPLAINT : write_noticeboard(user, 1, 0); break;
      case SUGGESTION: write_noticeboard(user, 2, 0); break;
      case EXCUSE    : write_noticeboard(user, 3, 0); break;
      case NOTICE    : read_notices(user); break;
      case KISS      : send_kiss(user,inpstr); break;
      case TIME      : sprintf(text,"~OL%s~RS\n",long_date(2));
                       write_user(user,text); break;
      case CALENDAR  : calendar(user); break;
      case JAIL      : send_to_jail(user, inpstr); break;
      case FIGHT     : fight_user(user); break;
      case COOLTALK  : cooltalk(user,inpstr); break;
      case COOLTOPIC : cooltopic(user,inpstr); break;
      case CDESC     : cdesc(user,inpstr); break;
      case FORTUNE   : fortune_cookies(user); break;
      case THINK     : think(user,inpstr); break;
      case COW       : cow(user); break;
      case SING      : sing(user,inpstr); break;
      case KIDNAP    : kidnap(user); break;
      case FOLLOW    : follow(user,0); break;
      case UNFOLLOW  : follow(user,1); break;
      case RANKS     : ranks(user); break;
      case SECHO     : secho(user,inpstr); break;
      case WJOKE     : write_joke(user,0); break;
      case RJOKE     : read_joke(user); break;
      case DJOKE     : delete_joke(user); break;
      case SIGN      : sign(user,inpstr); break;
      case TELLALL   : tellall(user,inpstr); break;
      case PRIKAZY   : help_commands(user, 0); break;
      case COMMANDS  : help_commands(user, 1); break;
      case LOGTIME   : logtime(user); break;
      case FAQ       : info(user,1); break;
      case RULES     : info(user,2); break;
      case PECHO     : pecho(user,inpstr); break;
      case REALUSER  : whois(user); break;
      /* case VOTE      : vote(user,inpstr, 1, 0); break; */
      case VOTE      : votenew(user); break;
      case SET       : set(user,inpstr); break; 
      case FMAIL     : fmail(user,0); break;
      case POET      : poetizuj(user); break;
      case REVSHOUT  : revshout(user,inpstr); break;
      case REVWIZ    : revwizshout(user,inpstr); break;
      case REVBCAST  : revbcast(user,inpstr); break;
      case REVSOS    : revsos(user,inpstr); break;
      case LAST      : last(user); break;

/* ZABLOKOVAT */
      case TBANNER   : tbanner(user,inpstr); break;  
      case BANNER    : banner(user,inpstr); break;  
      case SBANNER   : sbanner(user,inpstr); break;  
      
            
      case WFROM     : who_from(user); break;  
      case INSULT    : insult_user(user); break;  
      case CHNAME    : chname(user); break;  
      case PP        : power_points(user); break;
      case WINNERS   : winners(user,0,0); break;
      case WEATHER   : weather(user,inpstr); break;
      case PLAY      : amfiteater(user,1); break;
      case SKRIPT    : skript(user); break;
      case HINT      : hint(user, 0); break;
      case KICK      : kick(user); break;
      case IGNORE    : ignore(user,inpstr); break; /* Univerzalny prikaz */
      case UPTIME    : system_details(user,0); break;
      case SNDPROOF  : set_room_access(user); break;
      case JOIN      : join(user); break;
      case NUKE      : nuke_user(user); break;
      case PISKVORKY : piskvorky(user);break;
      case ALARM     : alarm_clock(user); break;
      case NOTIFY_CMD    : notify_user(user); break;
      case COPIES    : copies_to(user); break;
      case SAVE      : save_users(user); break;
      case HANGMAN   : play_hangman(user,inpstr); break;
      case WIZZES    : wizzes(user); break;
      case GEO       : play_geo(user); break;
      case GET       : get_predmet(user); break;
      case PUT       : put_predmet(user); break;
      case SELL      : sell(user); break;
      case BUY       : buy(user); break;
      case CREATE    : create_predmet(user); break;
      case GIVE      : give_predmet(user); break;
      case DISPOSE   : dispose_predmet(user); break;
      case USE       : use_predmet(user,inpstr); break;
      case PREDMETY  : zobraz_predmety(user,inpstr); break;
      case CALL      : call_user(user); break;
      case PRIDAJ    : p_u_prikaz(user,1,inpstr); break;
      case UBER      : p_u_prikaz(user,0,inpstr); break;
      case LODICKY   : lod(user); break;
      case LABYRINT  : lab(user); break;
      case DOOM      : doom(user,inpstr); break /* D00M */;
      case INFO      : info_users(user); break;
      case UPDATE    : update_web(user); break; /* docasny prikaz - na test */
      /* M@CRO>> */
      case MACROCMD  : macro(&(user->macrolist),inpstr,user,0); break;
      /* >>M@CRO */
      case GOSSIP    : gossip(user,inpstr);break;
      case GEMOTE    : gemote(user,inpstr);break;
      case REVGOSSIP : revgossip(user,inpstr); break;
      case QUEST     : quest_command(user,inpstr);break;
      case TALKERS   : sprintf(filename,"misc/talkers");
            switch(more(user,user->socket,filename)) {
                  case 0: write_user(user,"Ziadne talkre neboli najdene.\n");  break;
                  case 1: user->misc_op=2;
                  }
            break;
      case IDLETIME  : idletime(user); break;
      case GUEST     : guest_command(user, inpstr); break;
      case MAGAZINE  : magazin(user, inpstr); break;
      case ICQ       : icqpage(user, inpstr); break;
      case FINGER    : finger(user, inpstr); break;
      case HISTORY   : view_history(user); break;
      case NEWBAN    : ban_site_for_newuser(user); break;
      case NEWUNBAN  : unban_site_for_newuser(user); break;
      case REMOTE    : view_remote(user); break;
      case LINKS     : links(user); break;
      case NOTE      : note(user, inpstr); break;
      case DNOTE     : dnote(user); break;
      case SPELL     : spell(user, inpstr); break;
      case LYNX      : lynx(user); break;
      case REVIRC    : revirc_command(user, inpstr); break;
      case IRC       : irc(user); break;
      case CLIENT    : statline(user); break;
      case BJACK     : play_blackjack(user); break;
      case COMLEVEL  : poprehadzuj_prikazy(user); break;  
      case REVERSI   : reversi(user); break;
      case MINY      : miny(user); break;
      case DAMA      : dama(user); break;
      case REBIRTH   : rebirth(user); break;
      case GAMES     : games(user); break;
      case CLOVECE   : clovece(user,inpstr); break;
      case FARAON    : farar(user,inpstr); break;
      case SMS       : sms(user,0); break;
      case STATISTIC : statistic(user); break;  
      case ANALYZE   : analyze(user); break;  
      case SCLEROTIC : sclerotic(user); break;  
      case THROW     : throw_predmet(user); break;  
      case GOLD      : gold(user,0,""); break;  
      case XSAY      : xsay(user,inpstr); break;  
      case XANNEL    : xannel(user,inpstr); break;  
      case QUERY_COM : query_com(user,inpstr); break;  
      case CMD_MODULE : cmd_module(user, inpstr); break;  
      default: write_user(user,"Prepac, tento prikaz zatial nefunguje.\n");
      }
}

/* Guest veci ... */
void guest_command(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
RM_OBJECT rm;
UR_OBJECT u;
char gname[13];

strcpy(gname,"Guest"); /* ????? !!!!!!!!!!!!!!!!!!!!!!!! */

if (word_count<2) {
	write_user(user,"Pouzi: .guest [meno hosta] [on] [talk <uzivatel>] [quit | off]\n");
	write_user(user,"       .guest Janko Hrasko       - nastavi meno hosta\n");
	write_user(user,"       .guest on                 - zapne guest mod, ak uz je host nahlaseny\n");
	write_user(user,"       .guest talk Luzer         - Luzer dostane slovo\n");
	write_user(user,"       .guest quit (off)         - ukonci guest mod\n\n");
	if (guest.name[0]!='\0') {
		sprintf(text,"Meno hosta: ~OL%s\n", guest.name);
		write_user(user,text);
		}
	if (guest.moderator!=NULL) {
		sprintf(text,"Moderator : ~OL%s\n", guest.moderator->name);
		write_user(user,text);
		}	
	if (!guest.on) write_user(user,"Guest mode este nieje aktivovany.\n");
		else   write_user(user,"Guest mod je aktivovany!\n");
	return;
	}
	
if (!strcmp(word[1], "quit") || !strcmp(word[1],"off")) {	
	if (user!=guest.moderator) {
		write_user(user,"Len moderator moze zastavit guest-mod.\n");
		return;
		}
	if (guest.user!=NULL) strcpy(guest.user->desc,"- nas host!");
	guest.on=0;
	guest.user=NULL;
	guest.moderator=NULL;
	guest.talk=NULL;
	guest.name[0]='\0';
	if ((rm=get_room(GUEST_ROOM,NULL))!=NULL) {
		rm->sndproof=0;
		sprintf(text,"Diskusia bola zastavena!\n");
		write_room(rm,text);
		return;
		}
	write_user(user,"Guest mod bol zastaveny.\n");
	return;
	}
	
if (!strcmp(word[1],"talk")) {
	if (word_count<3) {
		if (guest.talk!=NULL) {
			sprintf(text,"Momentalne ma slovo: ~OL%s\n", guest.talk->name);
			write_user(user,text);		
			}
		write_user(user,"Pouzi: .guest talk <uzivatel>\n");
		return;
		}
	if (user!=guest.moderator) {
		write_user(user,"Nemozes udelovat slovo - niesi moderator.\n");
		return;
		}
	if ((u=get_user(word[2]))!=NULL) {
		guest.talk=u;
		sprintf(text,"Slovo ma: ~OL%s\n", u->name);
		write_user(user,text);
		sprintf(text,"~OL%s si slovo, mozes hovorit.\n", pohl(u,"Dostal","Dostala"));
		write_user(u,text);
		return;
		}
	else {
		write_user(user,notloggedon);
		return;
	     }
	}

if (!strcmp(word[1],"on")) {
	if (guest.on) {
		write_user(user,"Guest mod je uz aktivovany!\n");
		return;
		}
	if (guest.name[0]=='\0') {
		write_user(user,"Nieje nastavene meno hosta!\n");
		return;
		}
	if ((guest.user=get_user_exact(gname))==NULL) {
		write_user(user,"Uzivatel \"Guest\" nieje prihlaseny!\n");
		return;
		}
		
	sprintf(guest.user->desc, "~OL~FW%s", guest.name);
	if ((rm=get_room(GUEST_ROOM,NULL))==NULL) guest.user->room=get_room("namestie",NULL);
		else guest.user->room=rm;
	guest.on=1;
	guest.user->room->sndproof=1;
	look(guest.user);
	move_user(user, guest.user->room, 3);	
	sprintf(text,"~OL~FY%s, vitame Ta v Atlantide!\n", guest.name);
	write_room(user->room, text);	                 
	return;
	}
		
if (guest.name[0]!='\0') {
	sprintf(text,"Meno hosta uz je nastavene: ~OL%s\n", guest.name);
	write_user(user,text);
	if (!guest.on) write_user(user,"Guest mod este nieje aktivovany (host sa musi nahlasit)\n");	
	return;
	}
	
sstrncpy(guest.name,inpstr,30);
guest.moderator=user;
sprintf(text, "Guest mod bol nastaveny: ~OL%s\n", inpstr);
write_user(user,text);
}

void show_map(user)
UR_OBJECT user;
{
char filename[80];

sprintf(filename,"atlantis.map");

     if (word_count==2 && !strncmp(word[1],"atl",3)) sprintf(filename,"atlantis.map");
else if (word_count==2 && !strncmp(word[1],"por",3)) sprintf(filename,"portalis.map");
else if (word_count==2 && !strncmp(word[1],"oce",3)) sprintf(filename,"ocean.map");
else if (word_count==2 && !strncmp(word[1],"bru",3)) sprintf(filename,"brutalis.map");

else if (user->room->group==1) sprintf(filename,"atlantis.map");
else if (user->room->group==2) sprintf(filename,"portalis.map");
else if (user->room->group==4) sprintf(filename,"brutalis.map");
else if (!strcmp(user->room->name,"plachetnica")) sprintf(filename,"ocean.map");

sprintf(query,"select body from files where filename='%s'",filename);
switch(sqlmore(user,user->socket,query)) {
	case 0: write_user(user,"Nenasla sa mapa.\n");  break;
	case 1: user->misc_op=222;
	}
}

void update_web(user)
UR_OBJECT user;
{
if (word_count>1) {
    if (!strcmp(word[1],"rooms")) {
       load_rooms_desc(user);
       return;
      }
    else if (!strcmp(word[1],"motd")) {
      if (motd!=NULL) free(motd);
      sprintf(query,"select `body` from `files` where `filename`='motd1'");
      if ((result=mysql_result(query))) {
        if ((row=mysql_fetch_row(result)) && row[0]!=NULL) {
          motd=(char *) malloc ((strlen(row[0])*sizeof(char))+2);
          if (motd!=NULL) strcpy(motd,row[0]);
         }
        mysql_free_result(result);
       }
      if (motd==NULL) write_user(user,"~OL~FRCould not load MOTD!\n");
      else write_user(user,"Nacitavam motd.. hotofo.\n");
      return;
     }
    else {
       write_user(user,"Neznamy parameter pre update!\n");
       return;
      }
   }

/*
write_user(user,"~FTVytvaram html subor jokeboardu ... ");
jokeboard_web();
write_user(user,"Hotovo!~FW\n");
*/
/*
write_user(user,"~FTVytvaram html subor amfiteatra ... ");
amfiteater_web();
write_user(user,"Hotovo!~FW\n");

write_user(user,"~FTVytvaram html subor FAQ, rules a news ... ");
rules_faq_web();
write_user(user,"Hotovo!~FW\n");
*/
write_user(user,"~FTUkladam temy v miestnostiach ... ");
save_topic();
write_user(user,"Hotovo!~FW\n");

write_user(user,"Ukladam resolver cache ... ");
resc_save();
write_user(user,"Hotovo!~FW\n");

return;
}

void check_ident(user)
UR_OBJECT user;
{
if (zistuj_identitu) {
	zistuj_identitu=0;
	write_user(user,"Automaticke zistovanie identity ~FRVYPNUTE~FW.\n");
	return;
	}
zistuj_identitu=1;
write_user(user,"Automaticke zistovanie identity ~FGZAPNUTE~FW.\n");
return;
}

/* Forward na mobil (V) */ /*
void forward_smail2sms(char *name,char *from,char *message)
{
FILE *fp;
UR_OBJECT u;
char filename[80];

if ((u=get_user_exact(name))!=NULL) return;
if ((u=create_user())==NULL) {
  return;
 }
strcpy(u->name,name);
if (!load_user_details(u)) {
  destruct_user(u);
  destructed=0;
  return;
 }
if (!strcmp(u->mobile,"-") || !u->mail2sms) {
  destruct_user(u);
  destructed=0;
  return;
 } 

sprintf(filename,"%s/%s.FWD",MAILSPOOL,u->name);
if (!(fp=ropen(filename,"w"))) {
  write_syslog("Nemozem otvorit subor pre forward posty v set_forward_smail2sms()\n",0);
  destruct_user(u);
  destructed=0;
  return;
 }

 colour_com_strip(message);
 if (from==NULL) sprintf(text,"%s",message);
 else sprintf(text,"<.smail od %s> %s",from,message);
 send_sms(u->mobile,text,u->smsgate);
 destruct_user(u);
 destructed=0;
}
*/

/*** Forward na email adriesku ***/
void forward_email(name,from, from2, message)
char *name,*from,*message, *from2;
{
FILE *fp;
UR_OBJECT u;
char filename[80];
int on=0;
char signature[255];

if ((u=get_user_exact(name))!=NULL) {
  on=1;
  goto SKIP;
  }
if ((u=create_user())==NULL) {
  write_syslog("CHYBA: Nemozno vytvorit docasny user object pri forward_email().\n",0);
  return;
  }
    
strcpy(u->name,name);
if (!load_user_details(u)) {
  destruct_user(u);
  destructed=0;
  return;
  }
on=0;
SKIP:

if (!u->autofwd && !u->mailnotify) {
  if (!on) { destruct_user(u); destructed=0; }
  return;
  } 

if (u->mailnotify && on) return;

sprintf(filename,"%s/%s.FWD",MAILSPOOL,u->name);
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
  write_syslog("Nemozem otvorit subor pre forward posty v set_forward_email()\n",0);
  if (!on) { destruct_user(u); destructed=0;}
  return;
  }

fprintf(fp,"From: %s <%s@%s>\n",from2,from2,TALKER_EMAIL_HOST);
fprintf(fp,"To: %s <%s>\n",u->name,u->email);
fprintf(fp,"X-mailer: Atlantis Talker ver. %s\n",ATLANTIS);
if (u->autofwd) {
  fprintf(fp,"Subject: .smail od: %s\n", from);
  fprintf(fp,"\n");
  colour_com_strip(from);
  fputs(from,fp);
  fputs("\n",fp);
  colour_com_strip(message);
  fputs(message,fp);
 }
else {
  fprintf(fp,"Subject: Nova posta na Atlantise (%s)\n",from);
  fprintf(fp,"\n");
  colour_com_strip(from);
  fprintf(fp,"Na atlantis ti prisla nova posta. Odosielatel: %s\n",from);
 }
  fputs("\n\n",fp);
  sprintf(signature, talker_signature,TALKER_TELNET_HOST,port[0],TALKER_WEB_SITE);
  fputs(signature,fp);
 fclose(fp);
send_forward_email(u->email,filename);
if (!on) {
  destruct_user(u);
  destructed=0;
  }
return;
}

/*** Posielanie emailu ... ***/
void send_email(user,to,message)
UR_OBJECT user;
char *to, *message;
{
FILE *fp;
char filename[80];
time_t akt_cas;
char signature[255];

sprintf(filename,"%s/%s.EMAIL",MAILSPOOL,user->name);
if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
  write_syslog("Nemozem otvorit subor pre odoslanie posty vo send_email()\n",0);
  return;
  }
    
fprintf(fp,"From: %s <%s@%s>\n",user->name,user->name,TALKER_EMAIL_HOST);
fprintf(fp,"To: %s <%s>\n",to,to);
fprintf(fp,"X-mailer: Atlantis Talker ver. %s\n",ATLANTIS);
fprintf(fp,"Subject: %s\n", user->subject);
fprintf(fp,"\n");
colour_com_strip(message);
fputs(message,fp);
fputs("\n\n",fp);
sprintf(signature, talker_signature,TALKER_TELNET_HOST,port[0],TALKER_WEB_SITE);
fputs(signature,fp);
fclose(fp);
send_forward_email(to,filename);
user->subject[0]='\0';

/* logne kto kam posielal postu (koli odhaleniu spamov etc.) */
if (!(fp=ropen("log/sentmail","a"))) { /*APPROVED*/
	return;
	}
time (&akt_cas);
fprintf(fp,"%s %-12s %s\n",zobraz_datum(&akt_cas,5),user->name,to);
fclose(fp);
return;
}

void send_forward_email(send_to,mail_file)
char *mail_file,*send_to;
{
  switch(double_fork()) {
    case -1 : unlink(mail_file); return;
    case  0 : sprintf(text,"/usr/sbin/sendmail %s < %s > /dev/null",send_to,mail_file);
              system(text);
	      unlink(mail_file);
	      _exit(1);
    }
 
}

void show_version(user) /* Novy styl vypisu verzie - (S) */
UR_OBJECT user;
{
int exsecs, exrok, exmesiac, exden, secs, days, hours, mins;
char text2[50];

exsecs=(int)time(0)-858956400;
exrok=exsecs/31559040;
exmesiac=(exsecs%31559040)/2629920;
exden=(exsecs%2629920)/87664;

secs=(int)(time(0)-boot_time);
days=secs/86400;
hours=(secs%86400)/3600;
mins=(secs%3600)/60;

/*write_user(user,"~OL~FK==============================================================================\n");
write_user(user,"~OL~FB     ________________.____       _____    __________________._____________\n");
write_user(user,"~OL~FB    /  _  \\__    ___/|    |     /  _  \\   \\      \\__    ___/|   |/   _____/\n");
write_user(user,"~OL~FB   /  /_\\  \\|    |   |    |    /  /_\\  \\  /   |   \\|    |   |   |\\_____  \\\n");
write_user(user,"~OL~FB  /    |    \\    |   |    |___/    |    \\/    |    \\    |   |   |/        \\\n");
write_user(user,"~OL~FB  \\____|__  /____|   |_______ \\____|__  /\\____|__  /____|   |___/_______  /\n");
write_user(user,"~OL~FB          \\/                 \\/       \\/         \\/                     \\/\n");
sprintf(text,"~OL~FTVerzia %s\n",ATLANTIS);
writecent(user,text);
sprintf(text,"~FT%s\n",WORK);
writecent(user,text);
writecent(user,"~FTZalozene na NUTS 3.3.3 od Neila Robersona\n");
write_user(user,"~OL~FK==============================================================================\n");
*/

write_user(user,"~RS~FY     *--------------------------------------------------------------------*\n");
write_user(user,"~RS~FT             ::::\\ ::::::|::|     ::::\\ ::\\ ::|::::::|::::::|.:::::|\n");
write_user(user,"~OL~FB            |||,::|  ::|  ::|    ::|,::|::::::|  ::|    ::|  :::::>\n");
write_user(user,"~RS~FB            ::| ::|  ::|  ::::::|::| ::|::| \\:|  ::|  ::::::|,::::/\n");
sprintf(text,"~OL~FW- ~RS~FTVerzia %s ~OL~FW-~RS\n", ATLANTIS);
writecent(user,text);
write_user(user,"~RS~FY     *--------------------------------------------------------------------*\n");
sprintf(text,"~FTAtlantis uz existuje ~OL~FT%d~RS~FT ", exrok);
if (exrok>4) strcat(text,"rokov, ");
	else strcat(text,"roky, ");
sprintf(text2,"~OL~FT%d~RS~FT ", exmesiac);
strcat(text,text2);
if (!exmesiac || exmesiac>4) strcat(text,"mesiacov a ");
	else if (exmesiac==1) strcat(text,"mesiac a ");
		else strcat(text,"mesiace a ");
		
sprintf(text2,"~OL~FT%d~RS~FT ", exden);
strcat(text,text2);
if (!exden || exden>1) strcat(text,"dni.\n");
	else strcat(text,"den.\n");	
writecent(user,text);

sprintf(text,"~FTMomentalny uptime: ~OL~FT%d~RS~FT ", days);
if (!days || days>1) strcat(text,"dni, ");
	else strcat(text,"den, ");
sprintf(text2,"~OL~FT%d~RS~FT ", hours);
strcat(text,text2);
if (!hours || hours>4) strcat(text,"hodin a ");
	else if (hours==1) strcat(text,"hodina a ");
		else strcat(text,"hodiny a ");
sprintf(text2,"~OL~FT%d~RS~FT ", mins);
strcat(text,text2);
if (!mins || mins>4) strcat(text,"minut.\n");
	else if (mins==1) strcat(text,"minuta.\n");
		else strcat(text,"minuty.\n");
writecent(user,text);

write_user(user,"~FY     +--------------------------------------------------------------------+\n");
write_user(user,"~FY     | ~FTProgram design        :  ~FWRider (rider@lonestar.sk)                 ~FY|\n");
write_user(user,"~FY     | ~FTPovodni programatori  :  ~FWSpartakus, Viper, Buko, Hruza             ~FY|\n");
write_user(user,"~FY     | ~FTSucasny \"programator\" :  ~FWVoodoo                                    ~FY|\n");
write_user(user,"~FY     | ~FTZalozene na NUTS333   :  ~FW.pomoc credits                            ~FY|\n");
write_user(user,"~FY     +--------------------------------------------------------------------+\n");

}

void show_user(user,fajl)
UR_OBJECT user;
char fajl[80];
{
char line[200];
FILE *fp;

if (!(fp=ropen(fajl,"r"))) { return; } /*APPROVED*/

		fgets(line,199,fp);  
		while(!feof(fp)) {  
			write_user(user,line);  
			fgets(line,199,fp);  
		}  
 fclose(fp);
}

void p_u_prikaz(UR_OBJECT user,int pridaj,char *inpstr)
{
UR_OBJECT u;
char komand[20];
int i,len,found=0;
NOTIFY bunka;

if (word_count<3) {
	if (pridaj) write_user(user,"Pouzi: .pridaj <uzivatel> <prikaz>\n");
		else write_user(user,"Pouzi: .uber <uzivatel> <prikaz>\n");
	return;
	}
if (!(u=get_user(word[1]))) {
	write_user(user,notloggedon);
	return;
	}
	
strcpy(komand,"*");
i=0;
len=strlen(word[2]);
while(command[i][0]!='*') {
      if (!strncmp(command[i],word[2],len)) { com_num=i; strcpy(komand,command[i]); break;  }
      ++i;
      }
      
if (!strcmp(komand,"*")) {
	write_user(user,"Taky prikaz neexistuje!\n");
	return;
	}

if (u->level>=user->level) {
	write_user(user,"Uzivatel ma vyssi alebo rovnaky level ako mas ty, nepojde to!\n");
	return;
	}

for(bunka=u->combanlist; bunka!=NULL; bunka=bunka->next)
 if (!strcmp(bunka->meno,komand)) { found=1; break; }

if (pridaj) {
  if (!found) {
    sprintf(text,"%s nema odobraty prikaz .%s!\n",u->name,komand);
    write_user(user,text);
    return;
   }			
  sprintf(query,"select `banner`,`reason` from `comban` where `userid`='%d' and `cmd`='%s';",u->id,komand);
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result)) && row[0]) {
      if (strcmp(user->name,row[0]) && (user->level<GOD || word_count!=4 || strcmp(word[3],"sure"))) {
        sprintf(text,"~OL~FRPrikaz %s odobral(a) ~FW%s~FR.\n~FRDovod: ~FW%s\n~FRLen on(a) mu ho moze vratit.\n",sklonuj(u,3),row[0],(row[1])?row[1]:"");
        write_user(user,text);
        mysql_free_result(result);
        return;
       }
      else {
        delete_notify(&(u->combanlist),komand);
        sprintf (query,"delete from `comban` where `userid`='%d' and `cmd`='%s';",u->id,komand);
        mysql_kvery(query);
	sprintf(text,"~OLPrikaz .%s bol pridany %s.\n",komand,sklonuj(u,3));
	write_user(user,text);
	sprintf(text,"~OL~FY%s ti %s prikaz .%s!\n",user->name,pohl(user,"vratil","vratila"),komand);
	write_user(u,text);
        sprintf(text,"%s PRIDAL%s %s .%s\n",user->name,pohl(user,"","A"),sklonuj(u,3),komand);
        write_syslog(text,2);
       }
     }
    mysql_free_result(result);
    return;
   }
  return;
 }

  if (found) {
    sprintf(text,"%s uz ma odobraty prikaz .%s!\n",u->name,komand);
    write_user(user,text);
    return;
   }			
  if (word_count<4) {
    if (word_count==3) write_user(user,"~OL~FRZadaj plz aj dovod ubratia prikazu kvoli informovanosti ostatnych spravcov.\n");
    write_user(user,"Pouzi: .uber <user> <prikaz> <dovod>\n");
    return;
   }
  inpstr=remove_first(remove_first(inpstr));
  sprintf(query,"insert into `comban` (`userid`,`cmd`,`banner`,`reason`) values ('%d','%s','%s','%s');",u->id,komand,user->name,inpstr);
  mysql_kvery(query);
  add_notify(&(u->combanlist),komand);
  sprintf(text,"~OLPrikaz .%s bol odobrany uzivatelovi %s.\n",komand,u->name);
  write_user(user,text);
  sprintf(text,"~OL~FR%s ti %s prikaz .%s!\n",user->name,pohl(user,"odobral","odobrala"),komand);
  write_user(u,text);
  sprintf(text,"%s UBRAL%s %s .%s\n",user->name,pohl(user,"","A"),sklonuj(u,3),komand);
  write_syslog(text,2);
}


void call_user(user)
UR_OBJECT user;
{
UR_OBJECT u;

if (word_count<2) {
	write_user(user,"Pouzi: .call <user|cancel>\n");
	return;
	}

if (!strcmp(word[1],"cancel")) {
        write_user(user,"Quick-call bol zruseny...\n");
        user->call[0]='\0';
        return;
        }

if (!(u=get_user(word[1]))) {
	write_user(user,notloggedon);
	return;
	}
sstrncpy(user->call,word[1],19);	
sprintf(text,"Quick-call nastaveny na: %s\n",u->name);
write_user(user,text);
sprintf(text,"Tell posielas ciarkou: , nazdar  -je ekvivalentne: .tell %s nazdar\n",u->name);
write_user(user,text);
}

int count_lines(filename)
char *filename;
{
int i,c;
FILE *fp;

i=0;
if (!(fp=ropen(filename,"r"))) return i; /*APPROVED*/

c=getc(fp);
while (!feof(fp)) {
  if (c=='\n') i++;
  c=getc(fp);
  }
 fclose(fp);
return i;
}

void save_users(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt;

if ((user->level>=KIN) && (!strcmp(word[1],"all"))) {
	cnt=0;
	for (u=user_first;u!=NULL;u=u->next) {
	  if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
	  cnt++;
	  save_user_details(u,1);
	  }
	sprintf(text,"%d userfajlov bolo ulozenych.\n",cnt);
	write_user(user,text);
	return;
	}
save_user_details(user,1);
write_user(user,"Tvoj aktualny stav bol ulozeny.\n");
}


/* Posielanie kopii mailu */
void copies_to(user)
UR_OBJECT user;
{
UR_OBJECT u;
int remote,docopy,found,cnt;
unsigned int i=0,j;

if (!strcmp(word[1],"none")) {
  for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
  write_user(user,"Ziadne kopie sa nebudu posielat.\n");  return;
  }
if (word_count<2) {
  text[0]='\0';  found=0;
  for (i=0; i<MAX_COPIES; i++) {
    if (!user->copyto[i][0]) continue;
    if (++found==1) write_user(user,"Kopia buduceho .smail sa posle uzivatelom ...\n");
    strcat(text,"   ");  strcat(text,user->copyto[i]);
    }
  strcat(text,"\n\n");
  if (!found) write_user(user,"Nikomu sa nebude posielat kopia. (pouzi: .copies <user1> <user2>...<user10>)\n");
  else write_user(user,text);
  return;
  }
if (word_count>MAX_COPIES+1) {     
  sprintf(text,"Kopiu mozes poslat maximalne %d ludom.\n",MAX_COPIES);
  write_user(user,text);  return;
  }
write_user(user,"\n");
cnt=0;
for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
for (i=1; i<word_count; i++) {
  remote=0;  docopy=1;

  if (get_user(word[i])==user && user->level<KIN) {
    write_user(user,"Nemozes posielat kopiu sam sebe.\n");
    docopy=0;  goto SKIP;
    }
  word[i][0]=toupper(word[i][0]);
  for (j=1;j<i;j++) /* V) viac krat jednemu userovi kopiu ? to je FLOOD ! */
   if (!strcmp(word[j],word[i]))
    {
     docopy=0;  goto SKIP;
    }
  if (!remote) {
        if ((u=create_user())==NULL) {  
            sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
            write_user(user,text);
            write_syslog("CHYBA: Nemozno vytvorit docasny user object v notify().\n",0);            
            return;
            }  
      sstrncpy(u->name,word[i],USER_NAME_LEN);  
      if (!load_user_details(u)) {  
	    sprintf(text,"Uzivatel '%s' neexistuje!\n",word[i]);
	    write_user(user,text);
            docopy=0;
            }            
      else docopy=1;
      destruct_user(u);  
      destructed=0;
    }
SKIP:
  if (docopy) {
    sstrncpy(user->copyto[cnt],word[i], USER_NAME_LEN);  cnt++;
    }
  }
text[0]='\0';  i=0;  found=0;
for (i=0; i<MAX_COPIES; i++) {
  if (!user->copyto[i][0]) continue;
  if (++found==1) write_user(user,"Kopia buduceho .smail sa posle uzivatelom ...\n");
  strcat(text,"   ");  strcat(text,user->copyto[i]);
  }
strcat(text,"\n\n");
if (!found) write_user(user,"Nikomu sa nebude posielat kopia.\n");
else write_user(user,text);
}

/* Posle kopie smailu ... */
void send_copies(user,ptr)
UR_OBJECT user;
char *ptr;
{
int i,found=0;

for (i=0; i<MAX_COPIES; i++) {
  if (!user->copyto[i][0]) continue;
  if (++found==1) write_user(user,"Posielam kopie .smail ...\n");
  send_mail(user,user->copyto[i],ptr);
  }
for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
}

void alarm_clock(user)
UR_OBJECT user;
{
int pocet;

if (word_count<2) {
	write_user(user,"Pouzi: .alarm <pocet minut> [stop]\n");
	if (user->alarm>0) {
		sprintf(text,"\nZostavajuci cas do spustenia alarmu: %02d:%02d\n",user->alarm/60,user->alarm%60);
		write_user(user,text);
		}
	return;
	}

if (!strcmp(word[1],"stop")) {
	user->alarm=0;
	write_user(user,"Alarm bol zastaveny.\n");
	return;	
	}
	
pocet=atoi(word[1]);
if ((pocet<1) || (pocet>2016)) {  /*Max. 1 tyzden, heheheh*/
	write_user(user,"Nespravne zadany pocet minut!\n");
	return;
	}
	
if (pocet==1) sprintf(text,"Alarm je nastaveny na 1 minutu.\n");
	else if ((pocet>1) && (pocet<5)) sprintf(text,"Alarm je nastaveny na %d minuty.\n",pocet);
	  else sprintf(text,"Alarm je nastaveny na %d minut.\n",pocet);
write_user(user,text);	  
pocet=pocet*60;
user->alarm=pocet;	
}


/* (S) 0h b1@h, wh@t'z d1z fuk1ng f1ng ? Dhuz iz a w13nn@nuck@, l@merz! :>> */ 

void nuke_user(user)
UR_OBJECT user;
{
UR_OBJECT u;
#define dport 139

int sock;
char *str = "Skap";
char server[80];
struct sockaddr_in blah;
struct hostent *he;

if (word_count<2) {
	write_user(user,"Pouzi: .nuke <uzivatel>, alebo .nuke <adresa>   :>>\n");
	return;
	}

if (strstr(word[1],".")) sstrncpy(server,word[1],79);
	else {
	if (!(u=get_user(word[1]))) {
		write_user(user,notloggedon);
		return;
		}
	strcpy(server,u->site);	
	}
	
if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
   write_user(user,"Ponozkovita chyba! (Error in socket)\n");
   return;
     }

     bzero((char *)&blah,sizeof(blah));
     blah.sin_family=AF_INET;
     blah.sin_addr.s_addr=inet_addr(server);
     blah.sin_port=htons(dport);


    if ((he = gethostbyname(server)) != NULL) {
        bcopy(he->h_addr, (char *)&blah.sin_addr, he->h_length);
    }
    else {
         if ((blah.sin_addr.s_addr = inet_addr(server)) == INADDR_NONE) {
           write_user(user,"Chyba v zapise!\n");
           return;
         }
    }
        if (connect(sock,(struct sockaddr *)&blah,16)==-1) {
             write_user(user,"Nemozno sa pripojit!\n");
             close(sock);
             return;
        }        
    sprintf(text,"Nacucnuty na [%s:%d], posielam nuka... ",server,dport);
    write_user(user,text);    

    send(sock,str,strlen(str),MSG_OOB);
    usleep(100000);
    write_user(user,"Hotovka! ;>\n");
    close(sock);
}

/* (S) univerzalny ignore prikaz + nove: ignore sys a ignore fun */
void ignore(user,inpstr)
UR_OBJECT user;
char *inpstr;
{

if (word_count>=2) {
  if (!strcmp(word[1],"shout")) { toggle_ignshout(user); return; }
  if (!strcmp(word[1],"gossip")) { toggle_igngossip(user); return; }
  if (!strcmp(word[1],"tell")) { toggle_igntell(user); return; }
  if (!strcmp(word[1],"all")) { toggle_ignall(user); return; }
  if (!strcmp(word[1],"sys")) { toggle_ignsys(user); return; }
  if (!strcmp(word[1],"fun")) { toggle_ignfun(user); return; }
  if (!strcmp(word[1],"beep")) { toggle_ignbeep(user); return; }
  if (!strcmp(word[1],"ostrov")) { toggle_ignportal(user); return; }
  if (!strcmp(word[1],"word")) { toggle_ignword(user, inpstr); return; }
  if (!strcmp(word[1],"io")) { toggle_io(user); return; }
  if (!strncmp(word[1],"zv",2)) { toggle_zvery(user); return; }
  if (!strncmp(word[1],"col",3)) { toggle_coltell(user); return; }
  if (!strcmp(word[1],"look")) { toggle_ignlook(user); return; }
  if (!strcmp(word[1],"user")) { ignore_user(user); return; }
/*  if (!strcmp(word[1],"spell")) {
    if (user->ignblink) {
      sprintf(text,"Znova mozes carovat a stratil%s si imunitu voci kuzlam.\n",pohl(user,"","a"));
      write_user(user,text);
      user->ignblink=0;
      return;
     }
    sprintf(text,"Odteraz nemozes carovat a si imunn%s voci kuzlam ostatnych.\n",pohl(user,"y","a"));
    write_user(user,text);
    user->ignblink=1;
    return;
   }*/
  if (!strcmp(word[1],"blink")) {
    if (user->ignblink) {
      sprintf(text,"%s si ignorovat blikanie.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignblink=0;
      return;
     }
    write_user(user,"Odteraz ti nebude blikat ziadny text.\n");
    user->ignblink=1;
    return;
   }      
  if (!strncmp(word[1],"pict",4)) {
    if (user->ignpict) {
      sprintf(text,"%s si ignorovat obrazky poslane prikazom .pictell.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignpict=0;
      return;
     }
    write_user(user,"Odteraz ignorujes obrazky poslane prikazom .pictell.\n");
    user->ignpict=1;
    return;
   }      
  if (user->level>=KIN && !strncmp(word[1],"por",3)) {
    if (user->ignportalis) {
      sprintf(text,"%s si ignorovat shouty na portalise.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignportalis=0;
      return;
     }
    write_user(user,"Odteraz ignorujes shouty na portalise.\n");
    user->ignportalis=1;
    return;
   }      
  if (!strcmp(word[1],"games")) {
    if (user->igngames) {
      sprintf(text,"%s si ignorovat hry (piskvorky, reversy, ...).\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->igngames=0;
      return;
     }
    write_user(user,"Odteraz ignorujes hry (piskvorky, reversy, ...).\n");
    user->igngames=1;
    return;
   }      
  if (!strncmp(word[1],"autopro",7)) {
    if (user->ignautopromote) {
      sprintf(text,"Odteraz budes automaticky povysen%s po dosiahnuti potrebneho casu.\n",pohl(user,"y","a"));
      write_user(user,text);
      user->ignautopromote=0;
      return;
     }
    sprintf(text,"Odteraz budes povysen%s po dosiahnuti potrebneho casu len ak o to poziadas.\n",pohl(user,"y","a"));
    write_user(user,text);
    user->ignautopromote=1;
    return;
   }      
  if (!strncmp(word[1],"afk",3)) {
    if (user->ignafktell) {
      sprintf(text,"Zase sa ti budu zobrazovat telly ked budes auto-afk.\n");
      write_user(user,text);
      user->ignafktell=0;
      return;
     }
    write_user(user,"Odteraz sa ti nebudu zobrazovat telly ked budes auto-afk.\n");
    user->ignafktell=1;
    return;
   }      
  if (user->level>=com_level[XANNEL] && !strcmp(word[1],"xannel")) {
    if (user->ignxannel) {
      sprintf(text,"%s si ignorovat pozvania do kanalov.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignxannel=0;
      return;
     }
    write_user(user,"Odteraz ignorujes pozvania do kanalov.\n");
    user->ignxannel=1;
    return;
   }      
  if (user->level>=KIN && !strcmp(word[1],"sys2")) {
    if (user->ignnongreenhorn) {
      sprintf(text,"%s si ignorovat sys2.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignnongreenhorn=0;
      return;
     }
    write_user(user,"Odteraz ignorujes LOGIN/LOGOUT okrem novacikov a otrokov.\n");
    user->ignnongreenhorn=1;
    return;
   }      
 }
sprintf(text,"~FT.ignore shout        ~FW- nevidis vykriky, semote apod.                 ~OL(%s)\n",noyes(user->ignshout));
write_user(user,text);
sprintf(text,"~FT.ignore tell         ~FW- nemozu ti tellovat a pemotovat.               ~OL(%s)\n",noyes(user->igntell));
write_user(user,text);	
sprintf(text,"~FT.ignore sys          ~FW- nevidis hlasky ~OL~FMLOGIN/~FBLOGOUT~RS~FW a pod.            ~OL(%s)\n",noyes(user->ignsys));
write_user(user,text);	
if (user->level>=KIN) {
sprintf(text,"~FT.ignore sys2         ~FW- ignorujes sys okrem hlasok novacikov          ~OL(%s)\n",noyes(user->ignnongreenhorn));
write_user(user,text);
}
sprintf(text,"~FT.ignore fun          ~FW- budik, bozk a objatia iba na jeden riadok.    ~OL(%s)\n",noyes(user->ignfun));
write_user(user,text);	
sprintf(text,"~FT.ignore pict         ~FW- ignorujes obrazky poslane prikazom .pictell   ~OL(%s)\n",noyes(user->ignpict));
write_user(user,text);	
sprintf(text,"~FT.ignore zvierata     ~FW- nevidis behanie zvierat po miestnostiach      ~OL(%s)\n",noyes(user->ignzvery));
write_user(user,text);	
sprintf(text,"~FT.ignore gossip       ~FW- ignorujes gossip (kecy, dristy a nadavky)     ~OL(%s)\n",noyes(user->igngossip));
write_user(user,text);	
sprintf(text,"~FT.ignore all          ~FW- ignorujes uplne vsetko.                       ~OL(%s)\n",noyes(user->ignall));
write_user(user,text);	
sprintf(text,"~FT.ignore ostrov       ~FW- ignorujes telly z ineho ostrova               ~OL(%s)\n",noyes(user->ignportal));
write_user(user,text);
sprintf(text,"~FT.ignore io           ~FW- ignorujes prichody/odchody do/z miestnosti    ~OL(%s)\n",noyes(user->ignio));
write_user(user,text);
sprintf(text,"~FT.ignore look         ~FW- ignorujes vypis .look po prihlaseni           ~OL(%s)\n",noyes(user->ignlook));
write_user(user,text);
sprintf(text,"~FT.ignore beep/blink   ~FW- ignorujes pipanie/blikanie                  ~OL(%s/%s)\n",noyes(user->ignbeep),noyes(user->ignblink));
write_user(user,text);
sprintf(text,"~FT.ignore coloruser    ~FW- ignorujes farby v telli, pemote, gossipe      ~OL(%s)\n",noyes(user->igncoltell));
write_user(user,text);
sprintf(text,"~FT.ignore user <meno>  ~FW- budes usetreny od hlasok uzivatela <meno>.\n");
write_user(user,text);	
sprintf(text,"~FT.ignore word <slovo> ~FW- odignorujes VSETKY vety, obsahujuce slovo <slovo>.\n");
write_user(user,text);
sprintf(text,"~FT.ignore afktell      ~FW- ignorujes zobrazovanie tellov v auto-afk      ~OL(%s)\n",noyes(user->ignafktell)); 
write_user(user,text);
if (user->level>=KIN) {
sprintf(text,"~FT.ignore portalissh   ~FW- ignorujes shouty na portalise (spravcovia)    ~OL(%s)\n",noyes(user->ignportalis));
write_user(user,text);
}
sprintf(text,"~FT.ignore games        ~FW- ignorujes hry (piskvorky, reversy, ...)       ~OL(%s)\n",noyes(user->igngames));
write_user(user,text);
sprintf(text,"~FT.ignore autopromote  ~FW- ignorujes automaticke povysovanie             ~OL(%s)\n",noyes(user->ignautopromote));
write_user(user,text);
if (user->level>=com_level[XANNEL]) {
sprintf(text,"~FT.ignore xannel       ~FW- ignorujes pozvania do kanalov                 ~OL(%s)\n",noyes(user->ignxannel));
write_user(user,text);
}
}

void toggle_ignword(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
if (word_count < 3) {
    if (!strlen(user->ignword)) write_user(user,"Pouzitie: .ignore word <slovo|off>\n");
    else { write_user(user,"Ignorujes frazu: ~OL");
           strcpy(text,user->ignword);
           user->ignword[0]='\0'; /* Musi sa odignorovat, inak sa nezobrazi ;) */
           write_user(user,text);
           write_user(user,"~RS\n");
           strcpy(user->ignword,text); /* Znovu sa zaignoruje */
           }
    return;
    }
if (!strcmp(word[2],"off")) {
    if (!strlen(user->ignword))
         write_user(user,"~FRNeignorujes~FW nijake slovo!\n");
    else {

       sprintf(text,"~FR%s~FW si ignorovat frazu: %s\n", pohl(user,"Prestal","Prestala"), user->ignword);
       user->ignword[0]='\0';
       write_user(user,text);
       }
    return;
    }

inpstr=remove_first(inpstr);

if (strlen(inpstr) < 3) {
     write_user(user,"~FRSlovo musi mat minimalne 3 znaky.~FW\n");
     return;
     }
if (strlen(inpstr) > WORD_LEN) {
     write_user(user,"~FRNechces toho ignorovat trochu vela?~FW\n");
     return;
     }

sprintf(text,"~FG%s~FW si ignorovat frazu: %s\nIgnorovanie tejto frazy vypnes prikazom ~OL.ignore word off~RS\n",pohl(user,"Zacal","Zacala"), inpstr);
write_user(user,text);
sstrncpy(user->ignword,inpstr,WORD_LEN);
}

/* (S) vypise moznych skriptarov */
void skript(user)
UR_OBJECT user;
{
UR_OBJECT u;
int c;

c=0;
text[0]='\0';
for (u=user_first;u!=NULL;u=u->next) {
	if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
	if (u->pridavacas<1) {	
		if (!c) sprintf(text,"~OL~FYToto su potencialni skriptari:\n");
                if (c%5!=0) strcat(text," ");
                if (c%5==4) strcat(text,"\n");
		if (u->pridavacas==0) sprintf(texthb,"%-12s",u->name);	
	        if (u->pridavacas==-1) sprintf(texthb,"~OL%-12s~RS",u->name);
	        if (u->pridavacas==-2) sprintf(texthb,"~OL~FY%-12s~RS",u->name);
	        if (u->pridavacas==-3) sprintf(texthb,"~OL~FR%-12s~RS",u->name);
                strcat (text,texthb);
		c++;
		}
	}
if (!c) sprintf(text,"~OL~FRNeboli najdeni ziadni potencialni skriptari.\n");
else strcat(text,"\n");
write_user(user,text);
}


/* Spakky - FORWARD MAIL - vsetku postu forwardne na zadanu email adresu */
void fmail(UR_OBJECT user,int what)
{
FILE *fp2;
char line[83];
int cnt=0;

if (what) strcpy(word[1],user->email);
else if (word_count<2) {
  write_user(user,"Pouzi: .fmail <email_adresa>\n");
  return;
 }
if (strlen(word[1])>69) {
  write_user(user,"Prilis dlha adresa!\n");
  return;
 }
	
if (strpbrk(word[1],";$/+*[]\\|<>")) {
  write_user(user,"Nespravna email adresa!\n");
  return;
 }

if(!(fp2=ropen("fmail.tmp","w"))) { /*APPROVED*/
  write_user(user,"Nemozem otvorit docasny subor!\n");
  return;
 }

fprintf(fp2,"From: %s <%s@%s>\n",user->name,user->name,TALKER_EMAIL_HOST);
fprintf(fp2,"To: %s\n",word[1]);
switch (what) {
  case 0: fprintf(fp2,"Subject: Tvoj mailbox z Atlantisu\n"); break;
  case 1: fprintf(fp2,"Subject: Tvoj poznamkovy blok z Atlantisu\n"); break;
  case 2: fprintf(fp2,"Subject: Tvoje makra z Atlantisu\n"); break;
 }
fprintf(fp2,"X-mailer: ATLANTIS\n");
fprintf(fp2,"X-user-site: %s\n\n",user->site);
fprintf(fp2,"+--------------------------------------------------------+\n");
switch (what) {
  case 0: fprintf(fp2,"| Toto je posta z talkera Atlantis uzivatela %-12s|\n",user->name); break;
  case 1: fprintf(fp2,"| Toto je notes z talkera Atlantis uzivatela %-12s|\n",user->name); break;
  case 2: fprintf(fp2,"| Toto su makra z talkera Atlantis uzivatela %-12s|\n",user->name); break;
 }
 
fprintf(fp2,"+--------------------------------------------------------+\n");
fprintf(fp2,"\n");

if (what==0) sprintf(query,"select `sender`,`time`,`message` from `mailbox` where `userid`='%d' order by `time`",user->id);
else if (what==1) sprintf(query,"select `note`,`subdir` from `notes` where `userid`='%d' order by `subdir`,`date`",user->id);
else if (what==2) sprintf(query,"select `name`,`value` from `macros` where `userid`='%d' order by `name`",user->id);
cnt=0;
if ((result=mysql_result(query))) {
  while ((row=mysql_fetch_row(result))) { 
    cnt++;
    switch (what) {
      case 0: 
        fprintf(fp2,"===%d=== %s  %s\n%s\n",cnt,row[0]?row[0]:"",row[1]?sqldatum(row[1],0,0):"??.??.???? ??:??",row[2]?row[2]:"");
        /* fprintf(fp2,"%s\n",row[0]?row[0]:""); */
      break;
      case 1:
        if (row[1] && strcmp(line,row[1])) {
          sprintf(text,"\n[%s]\n",row[1]);
          fputs(text,fp2);
          strcpy(line,row[1]);
         }
        fputs(row[0]?row[0]:"",fp2);
        fputs("\n",fp2);
      break;
      case 2: fprintf(fp2,"%-20s %s\n",row[0]?row[0]:"",row[1]?row[1]:"");
      break;
     }
   }
  mysql_free_result(result);
 }
if (!cnt) {
  switch (what) {
    case 0: write_user(user,"Nemas ziadnu postu na forwardovanie.\n");
    break;
    case 1: write_user(user,"Nemas nic zapisane v poznamkovom bloku.\n");
    break;
    case 2: write_user(user,"Nemas ziadne makra!\n");
    break;
   }
  fclose(fp2);
  unlink("fmail.tmp");
  return;
 }
fprintf(fp2,"\n+--------------------------------------------------------+\n");
fclose(fp2);

/* (S) DOUBLE FORKING  */

switch(double_fork()) {
    case -1 : unlink("fmail.tmp"); return; 
    case  0 : sprintf(text,"/usr/sbin/sendmail %s < %s > /dev/null", word[1], "fmail.tmp");
              system(text);
	      unlink("fmail.tmp");
	      _exit(1);
    }

switch (what) {
  case 0:
    sprintf(text,"Tvoja posta bola odoslana na adresu %s\nPOZOR: Ak je adresa nespravna, posta sa nedoruci!\n",word[1]);
    write_user(user,text);
    sprintf(text,"%s forwarded %s mail to: %s\n",user->name,pohl(user,"his","her"),word[1]);
    write_syslog(text,1);
  break;
  case 1:        
    sprintf(text,"Poznamkovy blok bol poslany na tvoju nastavenu email adresu.\n");
    write_user(user,text);
  break;
  case 2:
    sprintf(text,"Makra boli poslane na tvoju nastavenu email adresu.\n");
    write_user(user,text);
  break;
 }
}

/* Dabl forking  */
int double_fork() {
pid_t pid;
int status;

if (!(pid=fork())) {
  switch(fork()) {
    case  0: return 0;
    case -1: _exit(-1);
    default: _exit(0);
    }
  }
if (pid<0||waitpid(pid,&status,0)<0) return -1;
if (WIFEXITED(status))
  if(WEXITSTATUS(status)==0) return 1;
  else errno=WEXITSTATUS(status);
else errno=EINTR;
return -1;
}

/* Dabl forking, wezion 2!!!  */
int double_fork_2() {
pid_t pid;
int status;

if (!(pid=fork())) {
  switch(fork()) {
    case  0: return 0;
    case -1: _exit(-1);
    default: _exit(0);
    }
  }
if (pid<0||waitpid(pid,&status,0)<0) return -1;
if (WIFEXITED(status))
  if(WEXITSTATUS(status)==0) return 1;
  else errno=WEXITSTATUS(status);
else errno=EINTR;
return -1;
}
 
 
 
/* set homepage */ 
void set_homepage(user,inpstr) 
UR_OBJECT user; 
char *inpstr; 
{ 

colour_com_strip(inpstr); 
if (strstr(inpstr," ")) { 
	sprintf(text,"Adresa tvojej homepage by nemala obsahovat medzery!\n"); 
	write_user(user,text); 
	return; 
	} 
if (strlen(inpstr)>HOMEPAGE_LEN) {
        sprintf(text,"Max. dlzka je %d znakov.\n",HOMEPAGE_LEN); 
	write_user(user,text); 
	return; 
	} 	
sstrncpy(user->homepage,inpstr,59);
sprintf(text,"OK, tvoja homepage bola nastavena: ~OL%s\n", user->homepage);
write_user(user,text);
} 
  
int get_age(int agecode)
{
struct tm *tm_struct;
time_t tajm;
int age;

time(&tajm);
tm_struct=localtime(&tajm);
age=tm_struct->tm_year - agecode%1000;
agecode=agecode/1000;
if ((agecode%100>tm_struct->tm_mon+1) || (agecode%100==tm_struct->tm_mon+1 && (agecode/100>tm_struct->tm_mday))) age--;
return age;
}

/* ZMENA: set [gender|homepage....] 
   nastavenia roznych parametrov pre userov...*/ 
 
void set(user,inpstr) 
UR_OBJECT user; 
char *inpstr; 
{  
FILE *fp;
int lines,type,r,year=0,mon=0,day=0,lev;
unsigned int i;
char id[10],serv[40],port[10],filename[81],line[4096],levstr[10];
UR_OBJECT u;

if (user->level>=GOD && !strncmp(word[1],"brutal",6)) {
  if (word_count<3) {
    write_user(user,"Pouzi: .set brutalis [off | on]\n");
    return;
   }
  if (!strcmp(word[2],"on") && portalis_gate==-666) {
    write_user(user,"Brana do brutalisu ~OLak~RStivovana.\n");
    portalis_gate=0;
   }
  if (!strcmp(word[2],"off") && portalis_gate!=-666) {
    write_user(user,"Brana do brutalisu ~OLde~RSaktivovana.\n");
    portalis_gate=-666;
   }
  return;
 }
if (user->level>=KIN && !strncmp(word[1],"name",4)) {
  if (word_count<3) {
    write_user(user,"Pouzi: .set [name2 | name3 | name4] <user> [<pad mena> | reset]\n");
    return;
   }
  if ((u=get_user(word[2]))==NULL) {
    write_user(user, notloggedon);
    return;
   }
  type=word[1][4]-48;
  if (type<2 || 4<type) {
    write_user(user,"Mozes menit len 2, 3 a 4 pad.\n");
    return;
   }
  sprintf(text,"Sklonovanie usera %s: 2:%s  3:%s  4:%s\n",u->name,sklonuj(u,2),sklonuj(u,3),sklonuj(u,4));
  write_user(user,text);
  if (word_count<4) {
    if (u->sname[type-2][0])
     sprintf(text,"%d. pad usera %s je nastaveny na: %s\n",type,u->name,u->sname[type-2]);
    else
     sprintf(text,"%s nema nastaveny %d. pad.\n",u->name,type);
    write_user(user,text);
    return;
   }
  strcpy(word[3],dbf_string(word[3]));
  if (strlen(word[3])>19) {
    write_user(user,"Spadnute meno moze mat dlzku maximalne 19 znakov.\n");
    return;
   }
  if (!strcmp(word[3],"reset")) {
    u->sname[type-2][0]='\0';
    sprintf(text,"%d. pad usera %s bol nastaveny na default.\n",type,u->name);
    write_user(user,text);
    return;
   }
  if (tolower(word[3][0])!=tolower(u->name[0])) {
    sprintf(text,"Nenastavuj tam sprostosti!!!\n");
    write_user(user,text);
    return;
   }
  strcpy(u->sname[type-2],word[3]);
  sprintf(text,"%d. pad usera %s nastaveny na: %s\n",type,u->name,u->sname[type-2]);
  write_user(user,text);
  return;
 }
if (user->level==GOD && !strncmp(word[1],"fly",3)) {
  if ((u=get_user(word[2]))==NULL) {
    return;
   }
  u->lieta=1;
  return;
 }
if (!strncmp(word[1],"msg",3) || !strncmp(word[1],"mess",4)) {
  if (word_count>2) {
    if (!strcmp(word[3],"cancel")) {
      if (!strncasecmp(word[2],"sh",2)) user->shoutmsg[0]='\0';
      if (!strncasecmp(word[2],"go",2)) user->gossipmsg[0]='\0';
      if (!strncasecmp(word[2],"sa",2)) user->saymsg[0]='\0';
      if (!strncasecmp(word[2],"te",2)) user->tellmsg[0]='\0';
      if (!strncasecmp(word[2],"wi",2)) user->wizshmsg[0]='\0';
      if (!strncasecmp(word[2],"mysh",4)) user->shoutmsg2[0]='\0';
      if (!strncasecmp(word[2],"mygo",4)) user->gossipmsg2[0]='\0';
      if (!strncasecmp(word[2],"mysa",4)) user->saymsg2[0]='\0';
      if (!strncasecmp(word[2],"myte",4)) user->tellmsg2[0]='\0';
      if (!strncasecmp(word[2],"mywi",4)) user->wizshmsg2[0]='\0';
     }
    else {
      inpstr=remove_first(remove_first(inpstr));
      if (colour_com_count(inpstr)>0) {
        write_user(user,"Farby pri tychto hlaskach si mozes nastavit pomocou ~OL.set col\n");
        return;
       }
      if (!strncasecmp(word[2],"sh",2)) {
        strncpy(user->shoutmsg,inpstr,20);
        user->shoutmsg[20]='\0';
       }
      if (!strncasecmp(word[2],"go",2)) {
        strncpy(user->gossipmsg,inpstr,20);
        user->gossipmsg[20]='\0';
       }
      if (!strncasecmp(word[2],"sa",2)) {
        strncpy(user->saymsg,inpstr,20);
        user->saymsg[20]='\0';
       }
      if (!strncasecmp(word[2],"te",2)) {
        strncpy(user->tellmsg,inpstr,20);
        user->tellmsg[20]='\0';
       }
      if (!strncasecmp(word[2],"wi",2)) {
        strncpy(user->wizshmsg,inpstr,20);
        user->wizshmsg[20]='\0';
       }
      if (!strncasecmp(word[2],"mysh",4)) {
        strncpy(user->shoutmsg2,inpstr,20);
        user->shoutmsg2[20]='\0';
       }
      if (!strncasecmp(word[2],"mygo",4)) {
        strncpy(user->gossipmsg2,inpstr,20);
        user->gossipmsg2[20]='\0';
       }
      if (!strncasecmp(word[2],"mysa",4)) {
        strncpy(user->saymsg2,inpstr,20);
        user->saymsg2[20]='\0';
       }
      if (!strncasecmp(word[2],"myte",4)) {
        strncpy(user->tellmsg2,inpstr,20);
        user->tellmsg2[20]='\0';
       }
      if (!strncasecmp(word[2],"mywi",4)) {
        strncpy(user->wizshmsg2,inpstr,20);
        user->wizshmsg2[20]='\0';
       }
     }
   }
  else {
    write_user(user,"~FTTymto prikazom si mozes zmenit hlasky ktore sa ti vypisuju pri komunikacii.\n");
    write_user(user,"~FTNastavenie ovplyvni to co sa zobrazi tebe, nie ostatnym.\n");
    if (user->level>=KIN) write_user(user,"~FTPouzi: set msg [tell | shout | say | gossip] <hlaska>\n");
    else write_user(user,"~FTPouzi: set msg [tell | shout | say | gossip | wizzsh ] <hlaska>\n");
    write_user(user,"~FTNapr: ~FW.set msg tell ti sepka do uska\n");
    write_user(user,"~FTNastavenie zrusis prikazom .set msg [tell | shout | say | gossip] cancel\n");
   }
  write_user(user,"Aktualne nastavenie:\n");
  sprintf(text,"~FTTell:~FW    %-20s   ~FTMyTell:~FW    %-20s\n",user->tellmsg,user->tellmsg2);
  write_user(user,text);
  sprintf(text,"~FTShout:~FW   %-20s   ~FTMyShout:~FW   %-20s\n",user->shoutmsg,user->shoutmsg2);
  write_user(user,text);
  sprintf(text,"~FTSay:~FW     %-20s   ~FTMySay:~FW     %-20s\n",user->saymsg,user->saymsg2);
  write_user(user,text);
  sprintf(text,"~FTGossip:~FW  %-20s   ~FTMyGossip:~FW  %-20s\n",user->gossipmsg,user->gossipmsg2);
  write_user(user,text);
  sprintf(text,"~FTWizsh:~FW   %-20s   ~FTMyWizsh:~FW   %-20s\n",user->wizshmsg,user->wizshmsg2);
  if (user->level>=KIN) write_user(user,text);
  return;
 }

if (!strcmp(word[1],"mana") && user->level>KIN) {
  user->mana=atoi(word[2]);
  if (user->mana<0 || user->mana>200) user->mana=100;
  return;
 }
if (!strncmp(word[1],"prio",4)) {
  if (!strcmp(word[2],"en") && user->com_priority==1) {
    user->com_priority=0;
    write_user(user,"Prioritu maju odteraz ~OLanglicke~RS prikazy a ich skratky.\n");
    return;
   }
  if (!strcmp(word[2],"sk") && user->com_priority==0) {
    user->com_priority=1;
    write_user(user,"Prioritu maju odteraz ~OLslovenske~RS prikazy a ich skratky.\n");
    return;
   }
  write_user(user,"Pouzi: set priority [en | sk]  - nastavi ktory jazyk ma prioritu.\n");
  if (user->com_priority)
    write_user(user,"Prioritu maju momentalne ~OLslovenske~RS prikazy a ich skratky.\n");
  else 
    write_user(user,"Prioritu maju momentalne ~OLanglicke~RS prikazy a ich skratky.\n");
  return;
 } 
if (!strncmp(word[1],"narod",5) || !strcmp(word[1],"birth")) {
  if (user->level>=KIN && (!strncmp(word[2],"dis",3) || !strncmp(word[2],"res",3))) {
    if ((u=get_user(word[3]))==NULL) {
      write_user(user, notloggedon);
      return;
     }
    else {
      if (!strncmp(word[2],"dis",3)) {
        u->age=0;
        u->agecode=1;
        sprintf(text,"%s ma odteraz zruseny vek a zablokovane pouzivanie .set birth.\n",u->name);
        write_user(user,text);
       }
      else {
        u->age=0;
        u->agecode=0;
        sprintf(text,"%s si moze znova nastavit datum narodenia.\n",u->name);
        write_user(user,text);
       }
      return;
     }
   }
  if (user->agecode==1) {
    write_user(user,"Toto nastavenie uz nemozes pouzit.\n");
    return;
   }
  if (word_count==3) {
    if (!strcmp(word[2],"off") || !strncmp(word[2],"vyp",3)) {
      if (user->agecode<0) { 
        write_user(user,"Zobrazovanie veku uz je vypnute.\n");
        return;
       }
      sprintf(text,"Vyp%s si zobrazovanie svojho veku.\n",pohl(user,"ol","la"));
      write_user(user,text);
      user->agecode=-user->agecode;
      user->age=0;
      return;
     }
    if (!strcmp(word[2],"on") || !strncmp(word[2],"zap",3)) {
      if (user->agecode==0) {
        write_user(user,"Este nemas nastaveny datum narodenia.\n");
        return;
       }
      if (user->agecode>0) { 
        write_user(user,"Zobrazovanie veku uz je zapnute.\n");
        return;
       }
      sprintf(text,"Zap%s si zobrazovanie svojho veku.\n",pohl(user,"ol","la"));
      write_user(user,text);
      user->agecode=-user->agecode;
      user->age=get_age(user->agecode);
      return;
     }
    if (user->agecode==0) {
      r=0;
      if (strlen(word[2])<8 || strlen(word[2])>10) r=3;
      else for(i=0;i<strlen(word[2]);i++) {
        if (word[2][i]=='.') r++;
        else if (!isdigit(word[2][i])) r=3;
       }
      if (r!=2) {
        sprintf(text,"Datum narodenia musi byt zadany v tvare: 21.3.1997\n");
        write_user(user,text);
        return;
       }
      i=0;r=0;
      sscanf(word[2],"%d.%d.%d",&day,&mon,&year);
      if (day>31) {
        sprintf(text,"Bouhuzial, mesiac moze mat len 31 dni.\n");
        write_user(user,text);
        return;
       }
      if (mon>12) {
        sprintf(text,"Bouhuzial, rok ma len 12 mesiacov.\n");
        write_user(user,text);
        return;
       }
      if (day<1 || mon<1) {
        sprintf(text,"Skus zadat datum..\n");
        write_user(user,text);
        return;
       }
      if (year<1900) {
        sprintf(text,"Pozor, datum narodenia sa da nastavit ~OLLEN RAZ~RS, uved pravdivy datum.\n");
        write_user(user,text);
        return;
       }
      r=(year-1900)+mon*1000+day*100000;
      type=get_age(r);
      if (type<0) {
        sprintf(text,"Este si sa nenarodil%s a uz tukas do klavesnice ? Z teba bude zavisla%s!\n",pohl(user,"","a"),pohl(user,"k","cka"));
        write_user(user,text);
        return;
       }
      if (type>99) {
        sprintf(text,"Pozor, datum narodenia sa da nastavit ~OLLEN RAZ~RS, uved pravdivy datum.\n");
        write_user(user,text);
        return;
       }
      user->agecode=r;
      user->age=type;
      sprintf(text,"Nastavil%s si si datum narodenia: ~OL%s~RS  Tvoj vek je: ~OL%d~RS rokov.\n",pohl(user,"","a"),word[2],user->age);
      write_user(user,text);
      return;
     }
   }
  if (user->agecode!=0) { 
    if (user->agecode<0) {
      r=get_age(-user->agecode);
      sprintf(text,"Tvoj vek je: ~OL%d~RS rokov, zobrazovanie ~OL~FRvypnute~RS~FW. Datum sa uz neda zmenit.\n",r);
      write_user(user,text);
     }
    else {
      sprintf(text,"Tvoj vek je: ~OL%d~RS rokov, zobrazovanie ~OL~FGzapnute~RS~FW. Datum sa uz neda zmenit.\n",user->age);
      write_user(user,text);
     }
    sprintf(text,"Ak chces vypnut/zapnut zobrazovanie veku, pouzi: .set birth off/on.\n");
    write_user(user,text);
    if (user->level>KIN) write_user(user,"~FTZrusenie/obnovenie veku a '.set birth' usera: .set birth [disable|reset] user\n");
    return;
   }
  else {
    sprintf(text,"Nemas nastaveny datum narodenia. Pouzi .set birth <datum> (format: 21.3.1997)\nDatum sluzi na vypocet aktualneho veku a NIE JE pristupny ostatnym uzivatelom.\nDatum sa da nastavit ~OLLEN RAZ~RS.\nZapnut/vypnut zobrazovanie veku je mozne prikazom: .set birth on/off\n");
    write_user(user,text);
    if (user->level>KIN) write_user(user,"~FTZrusenie/obnovenie veku a '.set birth' usera: .set birth [disable|reset] user\n");
    return;
   }
 }
if (user->level>=KIN && !strcmp(word[1],"wizpass")) {
  user->has_ignall=user->ignall;
  user->ignall=1;
  user->chanp[0]='\0';
  echo_off(user);
  if (!strcmp(user->wizpass,"-")) {
    user->misc_op=31;
    write_user(user,"~FTNove bezpecnostne heslo: ");
   }
  else {
    user->misc_op=30;
    write_user(user,"~FTStare (aktualne) bezpecnostne heslo: ");
   }
  return;
 }
if (!strncmp(word[1],"secu",4)) {
  if ((!strcmp(word[2],"on") || !strcmp(word[2],"zap")
  || !strcmp(word[2],"1")) && user->shortcutwarning==0) {
    user->shortcutwarning=1;
    write_user(user,"Varovanie pri telli s neurcitym cielovym uzivatelom ~FGzapnute~FW.\n");
    return;
   }
  if ((!strcmp(word[2],"off") || !strncmp(word[2],"vyp",3)
  || !strcmp(word[2],"1")) && user->shortcutwarning==1) {
    user->shortcutwarning=0;
    write_user(user,"Varovanie pri telli s neurcitym cielovym uzivatelom ~FRvypnute~FW.\n");
    return;
   }
  write_user(user,"Pouzi: set securetell [zap|vyp]\n\nPomocou tohto prikazu sa mozes vyhnut omylom pri posielani tellov sposobenym\npouzivanim skratiek mien. Ak mas securetell zapnuty a pokusis sa niekomu poslat\ntell pomocou skratky, tento tell sa nevykona a vypise varovnu hlasku vtedy, ak\nzadana skratka je aj skratkou ineho uzivatela (nie ak je to jeho presne meno)\nNapr: su prihlaseni 'Mrkva' a 'Mrkvicka', potom '.tell mrk caf petrzlen'\nsa nevykona, ale vypise varovnu hlasku.\n\n");
  if (user->shortcutwarning)
   write_user(user,"Varovanie pri telli s neurcitym cielom je prave ~FGzapnute.\n");
  else 
   write_user(user,"Varovanie pri telli s neurcitym cielom je prave ~FRvypnute.\n");
  return;
 } 
/*if (user->level>=KIN && !strncmp(word[1],"mail2",5)) {
  if ((!strcmp(word[2],"on") || !strcmp(word[2],"zap")
  || !strcmp(word[2],"1")) && user->mail2sms==0) {
    user->mail2sms=1;
    write_user(user,"Dosla posta sa odteraz bude posielat vo forme SMS na tvoj mobil.\n");
    return;
   }
  if ((!strcmp(word[2],"off") || !strncmp(word[2],"vyp",3)
  || !strcmp(word[2],"1")) && user->mail2sms==1) {
    user->mail2sms=0;
    sprintf(text,"Vyp%s si posielanie posty na mobil vo forme SMS.\n",pohl(user,"ol","la"));
    write_user(user,text);
    return;
   }
  write_user(user,"Pouzi: set mail2sms [zap|vyp]\nUmoznuje forwardovat doslu postu na mobil vo forme SMS sprav.\nPosiela sa pocet znakov od zaciatku ktory odoslat umoznuje SMS brana.\n");
  if (user->mail2sms)
   write_user(user,"Posielanie mailov na mobil vo forme SMS je prave ~FGzapnute.\n");
  else 
   write_user(user,"Posielanie mailov na mobil vo forme SMS je prave ~FRvypnute.\n");
  return;
 } 
 */
if (!strncmp(word[1],"timein",6)) {
  if ((!strcmp(word[2],"on") || !strcmp(word[2],"zap")
  || !strcmp(word[2],"1")) && user->timeinrevt==0) {
    user->timeinrevt=1;
    write_user(user,"Odteraz na zaciatku kazdeho riadku v review bufferoch bude uvedeny cas.\n");
    return;
   }
  if ((!strcmp(word[2],"off") || !strncmp(word[2],"vyp",3)
  || !strcmp(word[2],"1")) && user->timeinrevt==1) {
    user->timeinrevt=0;
    sprintf(text,"Vyp%s si zobrazovanie casu v review bufferoch.\n",pohl(user,"ol","la"));
    write_user(user,text);
    return;
   }
  write_user(user,"Pouzi: set timeinrev [on|off]  - zapne/vypne zapisovanie casu v review bufferoch.\n");
  if (user->timeinrevt)
   write_user(user,"Zobrazovanie casu v review bufferoch je prave ~FGzapnute.\n");
  else 
   write_user(user,"Zobrazovanie casu v review bufferoch je prave ~FRvypnute.\n");
  return;
 } 
if (!(strcmp(word[1],"mobil"))) {
  if (word_count==2) {
    if (strcmp(user->mobile,"-")) {
      sprintf(text,"Mas nastavene cislo mobilu: ~OL%s~RS.\nPouzi: '.set mobil zmaz' ak ho chces zmazat.\n",user->mobile);
      write_user(user,text);
      if (user->allowsms==0) sprintf(text,"SMS spravy ti mozu posielat vsetci. (.set sms)\n");
      else if (user->allowsms==1) sprintf(text,"SMS spravy ti mozu posielat len ti ktorych mas v notify. (.set sms)\n");
      else if (user->allowsms==2) sprintf(text,"SMS spravy ti mozu posielat ti, ktorych neignorujes. (.set sms)\n");
      else if (user->allowsms==3) sprintf(text,"SMS spravy ti nemoze posielat nikto. (.set sms)\n");
      write_user(user,text);
     }
    else {
      write_user(user,"Pouzi: '~OL.set mobil <cislo>~RS' na nastavenie cisla mobilu.\nCislo nie je pristupne ostatnym uzivatelom.\n");
     }
    return;
   }
  if (word_count>3) strcat(word[2],word[3]);
  if (!strcmp(word[2],"none") || !strcmp(word[2],"zmaz")) {
    sprintf(text,"Zmazal%s si cislo svojho mobilu.\n",pohl(user,"","a"));
    write_user(user,text);
    strcpy(user->mobile,"-");
    return;
   }
  if (strlen(word[2])!=10 || !is_number(word[2]) || strncmp(word[2],"090",3)) {
    strcpy(filename,word[2]);
    filename[0]='0';
    if (word[2][0]!='+' || !is_number(filename) || strlen(word[2])<10 || strlen(word[2])>15) {
      write_user(user,"Cislo musi byt vo formate 0903123456 alebo vo formate +421903123456.\n");
      return;
     }
    strcpy(user->mobile,word[2]);
   }
  else {
    strcpy(filename,"+42");
    word[2][0]='1';
    strcat(filename,word[2]);
    strcpy(word[2],filename);
    strcpy(user->mobile,filename);
   }
  sprintf(text,"Nastavil%s si si cislo mobilu: ~OL%s~RS.\n",pohl(user,"","a"),user->mobile);
  write_user(user,text);
  return;
 }
if (!(strcmp(word[1],"sms"))) {
  strcpy(serv," momentalne");
  if (word_count==2) {
    if (user->allowsms<0) user->allowsms=0;
    write_user(user,"~FTPouzi: .set sms [vsetci | notify | neignore | nikto]\n");
    write_user(user,".set sms vsetci    ~FT- SMSky ti mozu posielat vsetci.\n");
    write_user(user,".set sms notify    ~FT- SMSky ti mozu posielat len ti ktorych mas v notify.\n");
    write_user(user,".set sms neignore  ~FT- SMSky ti mozu posielat ti, ktorych neignorujes.\n");
    write_user(user,".set sms nikto     ~FT- SMSky ti nemoze posielat nikto.\n\n");
    sprintf(text,"Ak mas nastavene cislo, pomocou ~OL.sms %s~RS ti mozu ludia ktori\nsplnaju nastavene kriterium posielat SMS bez toho aby vedeli tvoje cislo.\nNastavene cislo nie je pristupne tretim osobam.\n",user->name);
    write_user(user,text);
   }
  if (word_count==3) {
    if (!strncmp(word[2],"vse",3) || !strcmp(word[2],"all")) { user->allowsms=0; strcpy(serv," odteraz"); }
    if (!strncmp(word[2],"not",3)) { user->allowsms=1; strcpy(serv," odteraz"); }
    if (!strncmp(word[2],"nei",3) || !strncmp(word[2],"noi",3)) { user->allowsms=2; strcpy(serv," odteraz"); }
    if (!strncmp(word[2],"nik",3) || !strncmp(word[2],"noo",3)) { user->allowsms=3; strcpy(serv," odteraz"); }
   }
  if (user->allowsms==0) sprintf(text,"SMS spravy ti mozu%s posielat vsetci.\n",serv);
  else if (user->allowsms==1) sprintf(text,"SMS spravy ti mozu%s posielat len ti ktorych mas v notify.\n",serv);
  else if (user->allowsms==2) sprintf(text,"SMS spravy ti mozu%s posielat ti, ktorych neignorujes.\n",serv);
  else if (user->allowsms==3) sprintf(text,"SMS spravy ti%s nemoze posielat nikto.\n",serv);
  write_user(user,text);
  return;
 }
if (user->level>=GOD && !(strncmp(word[1],"team",4))) {
  i=word[1][4]-48;
  if (i<1 || 4<i) {
    write_user(user,"Pouzi: .set team1 (az team4) [<nazov> | none]\n");
    return;  
   }
  if (word_count==2) {
    if (teamname[i][0])
     sprintf(text,"Nazov %d. teamu je: %s%s~RS~FW.\n",i,teamcolor[i],teamname[i]);
    else 
     sprintf(text,"Nazov %d. teamu nie je definovany.\n",i);
    write_user(user,text);
    return;
   }
  inpstr=remove_first(inpstr);
  if (strlen(inpstr)>30) {
    write_user(user,"Prilis dlhy nazov teamu. (max 30 znakov)\n");
    return;
   }
  if (!strcmp(word[2],"none")) {
    sprintf(text,"Zrusil si %d. team s nazvom: %s%s~RS~FW.\n",i,teamcolor[i],teamname[i]);
    write_user(user,text);
    teamname[i][0]='\0';
    return;
   }
  strcpy(teamname[i],inpstr);
  sprintf(text,"Nastavil si nazov %d. teamu na: %s%s~RS~FW.\n",i,teamcolor[i],teamname[i]);
  write_user(user,text);
  return;
 }
if (user->level>=GOD && !(strcmp(word[1],"minlogin"))) {
  police_freeze(user,10);
  if (user->wizactivity>9) return;
  clear_words();
  word_count=wordfind(inpstr);
  minlogin(user);
  return;
 }
if (user->level>=GOD && !(strncmp(word[1],"checki",6))) {
  if (!strcmp(word[2],"on") && zistuj_identitu==0) {
    zistuj_identitu=1;
    write_user(user,"Automaticke zistovanie ~FGZAPNUTE.\n");
    sprintf(text,"%s ZAPOL logovanie prikazov.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"off") && zistuj_identitu==1) {
    zistuj_identitu=0;
    write_user(user,"Automaticke zistovanie ~FRVYPNUTE.\n");
    sprintf(text,"%s VYPOL logovanie prikazov.\n",user->name);
    write_syslog(text,1);
    return;
   }
  write_user(user,"Pouzi: set checkident [on|off]\n");
  if (zistuj_identitu)
   write_user(user,"Automaticke zistovanie je prave ~FGZAPNUTE.\n");
  else 
   write_user(user,"Automaticke zistovanie je prave ~FRVYPNUTE.\n");
  return;
 } 
if (user->level>=GOD && !(strcmp(word[1],"swearban"))) {
  if (!strcmp(word[2],"on") && ban_swearing==0) {
    ban_swearing=1;
    write_user(user,"Blokovanie nadavok ~FGZAPNUTE.~FW Prikazy obsahujuce nadavky nebudu vykonane.\n");
    sprintf(text,"%s ZAPOL blokovanie nadavok.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"off") && ban_swearing==1) {
    ban_swearing=0;
    write_user(user,"Blokovanie nadavok ~FRVYPNUTE.~FW Nadavky budu vybodkovane.\n");
    sprintf(text,"%s VYPOL blokovanie nadavok.\n",user->name);
    write_syslog(text,1);
    return;
   }
  write_user(user,"Pouzi: set swearban [on|off]\n");
  if (ban_swearing)
   write_user(user,"Blokovanie nadavok je prave ~FGZAPNUTE.~FW Prikazy s nadavkami nebudu vykonane.\n");
  else 
   write_user(user,"Blokovanie nadavok je prave ~FRVYPNUTE.~FW Nadavky budu vybodkovane.\n");
  return;
 } 
if (user->level>=GOD && !(strncmp(word[1],"logcom",6))) {
  if (!strcmp(word[2],"mem") && logcommands==1) {
    logcommands=2;
    write_user(user,"Logovanie nachamranej pamate ~FGZAPNUTE.\n");
    sprintf(text,"%s ZAPOL logovanie nachamranej pamate.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"on") && logcommands!=1) {
    logcommands=1;
    write_user(user,"Logovanie prikazov ~FGZAPNUTE.\n");
    sprintf(text,"%s ZAPOL logovanie prikazov.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"off") && logcommands>0) {
    logcommands=0;
    write_user(user,"Logovanie prikazov ~FRVYPNUTE.\n");
    sprintf(text,"%s VYPOL logovanie prikazov.\n",user->name);
    write_syslog(text,1);
    return;
   }
  write_user(user,"Pouzi: set logcommands [on|off|mem]\n");
  if (logcommands==0)
   write_user(user,"Logovanie prikazov je prave ~FRVYPNUTE.\n");
  else if (logcommands==1)
   write_user(user,"Logovanie prikazov je prave ~FGZAPNUTE.\n");
  else if (logcommands==2)
   write_user(user,"Logovanie prikazov a nachamranej pamate je ~FGZAPNUTE.\n");
  return;
 } 
if (user->level>=GOD && !(strcmp(word[1],"resolver"))) {
  if (!strcmp(word[2],"on") && resolve_ip==0) {
    resolve_ip=1;
    write_user(user,"Resolver ~FGZAPNUTY.\n");
    sprintf(text,"%s ZAPOL resolver.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"off") && resolve_ip==1) {
    resolve_ip=0;
    write_user(user,"Resolver ~FRVYPNUTY.\n");
    sprintf(text,"%s VYPOL resolver.\n",user->name);
    write_syslog(text,1);
    return;
   }
  write_user(user,"Pouzi: set resolver [on|off]\n");
  if (resolve_ip)
   write_user(user,"Resolver je prave ~FGZAPNUTY.\n");
  else 
   write_user(user,"Resolver je prave ~FRVYPNUTY.\n");
  return;
 } 
if (user->level>=GOD && !(strcmp(word[1],"logging"))) {
  if (!strcmp(word[2],"on") && system_logging==0) {
    system_logging=1;
    write_user(user,"Logovanie ~FGZAPNUTE.\n");
    sprintf(text,"%s ZAPOL systemove logovanie.\n",user->name);
    write_syslog(text,1);
    return;
   }
  if (!strcmp(word[2],"off") && system_logging==1) {
    system_logging=0;
    write_user(user,"Logovanie ~FRVYPNUTE.\n");
    sprintf(text,"%s VYPOL systemove logovanie.\n",user->name);
    write_syslog(text,1);
    return;
   }
  write_user(user,"Pouzi: set logging [on|off]\n");
  if (system_logging)
   write_user(user,"Logovanie je prave ~FGZAPNUTE.\n");
  else 
   write_user(user,"Logovanie je prave ~FRVYPNUTE.\n");
  return;
}

if (user->level>=GOD && !(strcmp(word[1],"loginidle"))) {
  if (word_count!=3) {
    sprintf(text,"Pouzi: .set loginidle <sekundy>\nMomentalne nastavene na: ~OL%d~RS %s.\n",login_idle_time,skloncislo(login_idle_time,"sekundu","sekundy","sekund"));
    write_user(user,text);
    return;
  }
  type=atoi(word[2]);
  if (type<5 || 300<type) {
    write_user(user,"Zadaj cas v rozmedzi od 5 do 300 sekund.\n");
    return;
  }
  login_idle_time=type;
  sprintf(text,"%s si login_idle_time na %d %s.\n",pohl(user,"Nastavil","Nastavila"),type,skloncislo(type,"sekundu","sekundy","sekund"));
  write_user(user,text);
  sprintf(text,"%s %s login_idle_time na %d %s.\n",user->name,pohl(user,"nastavil","nastavila"),type,skloncislo(type,"sekundu","sekundy","sekund"));
  write_level(GOD,1,text,user);
  sprintf(text,"%s set login_idle_time to %d second(s).\n",user->name,type);
  write_syslog(text,1);
  return;
}

if (user->level>=GOD && !(strcmp(word[1],"useridle"))) {
  if (word_count!=3) {
    sprintf(text,"Pouzi: .set useridle <sekundy>\nMomentalne nastavene na: ~OL%d~RS %s.\n",user_idle_time,skloncislo(user_idle_time,"sekundu","sekundy","sekund"));
    write_user(user,text);
    return;
  }
  type=atoi(word[2]);
  if (type<30 || 3600<type) {
    write_user(user,"Zadaj cas v rozmedzi od 30 do 3600 sekund.\n");
    return;
  }
  user_idle_time=type;
  sprintf(text,"%s si user_idle_time na %d %s.\n",pohl(user,"Nastavil","Nastavila"),type,skloncislo(type,"sekundu","sekundy","sekund"));
  write_user(user,text);
  sprintf(text,"%s %s user_idle_time na %d %s.\n",user->name,pohl(user,"nastavil","nastavila"),type,skloncislo(type,"sekundu","sekundy","sekund"));
  write_level(GOD,1,text,user);
  sprintf(text,"%s set user_idle_time to %d second(s).\n",user->name,type);
  write_syslog(text,1);
  return;
}

if (user->level>=GOD && !(strcmp(word[1],"wplevel"))) {
  if (word_count!=3) {
    if (wizport_level==-1) strcpy(levstr,"NONE");
    else strcpy(levstr,level_name[wizport_level]);
    sprintf(text,"Pouzi: .set wplevel <NONE|level_name>\nMomentalne nastavene na: ~OL%s~RS.\n",levstr);
    write_user(user,text);
    return;
  }
  strtoupper(word[2]);
  if ((lev=get_level(word[2]))==-1) {
    if (strcmp(word[2],"NONE")) {
      write_user(user,"Takyto level tu nevedieme...\n");
      return;
    }
    strcpy(levstr,"NONE");
  } else {
    strcpy(levstr,level_name[lev]);
  }
  if (lev>user->level) {
    write_user(user,"Nemozes nastavit na vyssi level, ako mas sam.\n");
    return;
  }
  if(wizport_level==lev) {
    write_user(user,"Ved to je tak nastavene.\n");
    return;
  }
  wizport_level=lev;
  sprintf(text,"V poriadku, nastavene na level ~OL%s~RS.\n",levstr);
  write_user(user,text);
  sprintf(text,"%s %s wizport od levelu ~OL%s~RS.\n",user->name,pohl(user,"nastavil","nastavila"),levstr);
  write_level(GOD,1,text,user);
  sprintf(text,"%s set wizport_level to '%s'\n",user->name,levstr);
  write_syslog(text,1);
  return;
}

if (user->level>=GOD && !(strcmp(word[1],"timeoutmaxlvl"))) {
  if (word_count!=3) {
    if (time_out_maxlevel==-1) strcpy(levstr,"NONE");
    else strcpy(levstr,level_name[time_out_maxlevel]);
    sprintf(text,"Pouzi: .set timeoutmaxlvl <NONE|level_name>\nMomentalne nastavene na: ~OL%s~RS.\n",levstr);
    write_user(user,text);
    return;
  }
  strtoupper(word[2]);
  if ((lev=get_level(word[2]))==-1) {
    if (strcmp(word[2],"NONE")) {
      write_user(user,"Takyto level tu nevedieme...\n");
      return;
    }
    strcpy(levstr,"NONE");
  } else {
    strcpy(levstr,level_name[lev]);
  }
  if (lev>user->level) {
    write_user(user,"Nemozes nastavit na vyssi level, ako mas sam.\n");
    return;
  }
  if(time_out_maxlevel==lev) {
    write_user(user,"Ved to je tak nastavene.\n");
    return;
  }
  time_out_maxlevel=lev;
  sprintf(text,"V poriadku, nastavene na level ~OL%s~RS.\n",levstr);
  write_user(user,text);
  sprintf(text,"%s %s max. uzivat. necinnost na level ~OL%s~RS.\n",user->name,pohl(user,"nastavil","nastavila"),levstr);
  write_level(GOD,1,text,user);
  sprintf(text,"%s set time_out_maxlevel to '%s'\n",user->name,levstr);
  write_syslog(text,1);
  return;
}

if (user->level>=GOD && !(strcmp(word[1],"timeoutafks"))) {
  if (!strcmp(word[2],"on")) {
    if (time_out_afks==0) {
      time_out_afks=1;
      write_user(user,"Max. AFK necinnost ~FGZAPNUTA.\n");
      sprintf(text,"%s ZAPOL max. AFK necinnost.\n",user->name);
      write_syslog(text,1);
      write_level(GOD,1,text,user);
      return;
    } else {
      write_user(user,"Viac to uz zapnut nepojde...\n");
      return;
    }
  }
  if (!strcmp(word[2],"off")) {
    if (time_out_afks==1) {
      time_out_afks=0;
      write_user(user,"Max. AFK necinnost ~FRVYPNUTA.\n");
      sprintf(text,"%s VYPOL max. AFK necinnost.\n",user->name);
      write_syslog(text,1);
      write_level(GOD,1,text,user);
      return;
    } else {
      write_user(user,"Co blaznis, ved to nie je zapnute.\n");
      return;
    }
  }
  sprintf(text,"Pouzi: .set timeoutafks [on|off]\nMomentalny stav: %s\n",time_out_afks?"zapnute":"vypnute");
  write_user(user,text);
  return;
}

if (user->level>=KIN && !strncmp(word[1],"autocre",7)) {
  if (word_count==3) {
    r=atoi(word[2]);
    if (0<r && r<32001) {
      sprintf(text,"Rychlost vytvarania predmetov nastavena na interval ~OL%d~RS sec.\n",r);
      write_user(user,text);
      ryxlost_doplnovania=r;
      return;
     }
    if (!strcmp(word[2],"on")) {
      doplnovanie_predmetov=1;
      sprintf(text,"Automaticke vytvaranie predmetov ~FRZAPNUTE~RS~FW, interval: ~OL%d~RS sec.\n",ryxlost_doplnovania);
      write_user(user,text);
      return;
     }
    if (!strcmp(word[2],"off")) {
      doplnovanie_predmetov=0;
      sprintf(text,"Automaticke vytvaranie predmetov ~FRVYPNUTE~RS~FW.\n");
      write_user(user,text);
      return;
     }
   }
  if (doplnovanie_predmetov) {
    sprintf(text,"Automaticke vytvaranie predmetov je ~FGZAPNUTE~RS~FW, interval: ~OL~FW%d~RS sec.\n",ryxlost_doplnovania);
    write_user(user,text);
    r=ryxlost_doplnovania+(((100-num_of_users)*predmety_dynamic)/100);
    if (r<1) r=1;
    sprintf(text,"Interval dynamickeho doplnovania: ~OL~FW%d~RS sec. (Elasticita: %d)\n",r,predmety_dynamic);
    write_user(user,text);
   }
  else {
    sprintf(text,"Automaticke vytvaranie predmetov je momentalne ~FRVYPNUTE~RS~FW\n");
    write_user(user,text);
   }
  write_user(user,"Pouzi: .set autocreate [on | off | <interval>]  interval <1,32000> sekund.\n");
  return;
 }
if (user->level>=KIN && !strncmp(word[1],"dyna",4)) {
  if (word_count==3) {
    r=atoi(word[2]);
    if (-1<r && r<1001) {
      sprintf(text,"Elasticita autocreate useri:predmety nastavena na: 100:~OL%d~RS.\n",r);
      write_user(user,text);
      predmety_dynamic=r;
      r=ryxlost_doplnovania+(((100-num_of_users)*predmety_dynamic)/100);
      if (r<1) r=1;
      sprintf(text,"Interval dynamickeho doplnovania: ~OL~FW%d~RS sec. (Staticky: %d sec)\n",r,ryxlost_doplnovania);
      write_user(user,text);
      return;
     }
    else {
      write_user(user,"Hondnota moze byt len z intervalu <0,1000>.\n");
      return;
     }
   }
  write_user(user,"Pouzi: .set dynamicautocreate <hodnota>\nHodnoda <0,1000> vyjadruje percentualnu zmenu rychlosti autocreate pri zmene\npoctu userov z poctu 100 ks.\nAk je napr. nastavena na 100:50, pri nalogovani 101-veho usera sa doplnanie\npredmetov zrychli o pol sekundy (napr. z kazdych 60 sekund na 59.5).\nStaticke autocreate sa zabezpeci nastavenim hodnoty na 0.\n");
  sprintf(text,"Elasticita autocreate useri:predmety je momentalne: 100:~OL%d~RS.\n",predmety_dynamic);
  write_user(user,text);
  return;
 }
if (user->level>=GOD && !strcmp(word[1],"gold")) { user->gold=atoi(word[2]); return; }
if (user->level>=GOD && !(strcmp(word[1],"maxtimeouts"))) {
 type=atoi(word[2]);
 if (type<1 || 1000<type) write_user(user,"Povolene hodnoty su 1 az 1000. Nad 666 sa useri nevykopavaju.\n");
 else { 
   max_timeouts=type;
   alter_maxtimeouts(1);
  }
 sprintf(text,"Pocet write-timeoutsov potrebnych na vykopnutie: ~OL%d~RS\n",max_timeouts);
 write_user(user,text);
 return;
}
if (user->level>=GOD && !(strcmp(word[1],"april"))) {
 if (word_count>2) {
   type=atoi(word[2]);
   if (type<0 || 1000<type) write_user(user,"Povolene hodnoty su 0 az 1000.\n");
   else { 
     april=type;
    }
  }
 sprintf(text,"April flag: ~OL%d~RS\n",april);
 write_user(user,text);
 return;
}
if (user->level>=GOD && !(strcmp(word[1],"maxtimeoutz"))) {
 type=atoi(word[2]);
 if (type<1 || 1000<type) write_user(user,"Povolene hodnoty su 1 az 100. Nad 90 sa useri nevykopavaju.\n");
 else { 
   max_timeoutz=type;
   alter_maxtimeouts(1);
  }
 sprintf(text,"Pocet write-timeoutzov potrebnych na vykopnutie: ~FR~OL%d\n",max_timeoutz);
 write_user(user,text);
 return;
}
if (!strcmp(word[1],"!idle!")) { user->last_input=time(0)-atoi(word[2]); return; }
if (user->level>=GOD && !strcmp(word[1],"multiply")) {
  if (user->multiply) {
    user->multiply=0;
    write_user(user,"Tvoje klonovanie bolo ~FRvypnute~FW..\n");
    return;
   }
  user->multiply=1;
  write_user(user,"Tvoje klonovanie bolo ~FGzapnute~FW..\n");
  return;
 }
if (user->level>=GOD && !(strcmp(word[1],"speed"))) {
 type=atoi(word[2]);
 if (type<1 || 500<type) write_user(user,"Povolene hodnoty su 1 az 500. (pomaly:1, rychlo:100, standardne:10)\n");
 else zvery_speed=type;
 sprintf(text,"Aktualna rychlost: ~OL%d\n",zvery_speed);
 write_user(user,text);
 return;
}
if (user->level>=GOD && !(strcmp(word[1],"maxsms"))) {
 if (word_count>2) {
   type=atoi(word[2]);
   if (type<0) write_user(user,"Povolene hodnoty su 1 a viac. 0 - neobmedzene.\n");
   else {
     max_sms=type;
     alter_maxtimeouts(1);
    }
  }
 sprintf(text,"Maximalny pocet SMSiek za den je: ~OL%d\n",max_sms);
 write_user(user,text);
 return;
}
if (user->level>=GOD && !(strcmp(word[1],"tunel"))) {
 if (word_count>2) 
  { 
   type=atoi(word[2]);
   if (type<-120 || 1440<type) write_user(user,"Povolene hodnoty: -120..1444. (o kolko minut sa zavrie(-) || otvori(+) tunel)\n");
   else tunelik=type;
  }
 sprintf(text,"Povolene hodnoty: -120..1444. (o kolko minut sa zavrie(-) || otvori(+) tunel)\nAktualny stav: %d minut.\n",tunelik);
 write_user(user,text);
 return;
}
if (user->level>=GOD && !(strncmp(word[1],"durab",5))) {
 if (word_count>3) 
  { 
   lines=atoi(word[2]);
   type=atoi(word[3]);
   if (-1<lines && lines<MPVM && user->room->predmet[lines]>-1)
    { user->room->dur[lines]=type; return; }
   if (-HANDS-1<lines && lines<0 && user->predmet[-lines-1]>-1)
    { user->dur[-lines-1]=type; return; }
  }
}
if (!(strcmp(word[1],"autoafk"))) {
  if (word_count>2) 
   { 
    type=atoi(word[2]);
    if (user->level<KIN) {
      if (type<3 || 10<type) write_user(user,"Auto-afk mozes nastavit len od 3 do 10 minut.\n");
      else user->goafkafter=type;
     }
    else {
      if (type<3 || 120<type) write_user(user,"Auto-afk mozes nastavit len od 3 do 120 minut.\n");
      else user->goafkafter=type;
     }
   }
  sprintf(text,"Auto-afk sa bude nastavovat po %d minutach.\n",user->goafkafter);
  write_user(user,text);
  return;
 }
if (user->level>=KIN && !(strcmp(word[1],"timeout"))) {
  if (word_count>2) 
   { 
    type=atoi(word[2]);
    if (type<10 || 120<type) write_user(user,"Timeout si mozes nastavit len od 10 do 120 minut.\n");
    else user->killmeafter=type;
   }
  sprintf(text,"Timeout nastaveny na %d minut.\n",user->killmeafter);
  write_user(user,text);
  return;
 }
if (!(strncmp(word[1],"lan",3)))
 {
  texthb[0]='\0';
  for(type=0;type<LANGUAGES;type++)
   {
    if (!strcmp(word[2],langword[type+1]))
     {
      user->lang=type;
      sprintf(text,"~FG\253?L~FW\253?S.\n");
      write_user(user,text);
      return;
     } 
   strcat(texthb,langword[type+1]);
   if (type<LANGUAGES-1) strcat(texthb," | ");
  }
 sprintf(text,"\253?: ~FG\253?L~FW (.set lang <%s>)\n",texthb);
 write_user(user,text);
 return;
}
if (!(strncmp(word[1],"col",3))) {
	if (word_count==2) {
	 write_user(user,"~FTPouzi:~FW .set color <reset | shout | gossip | say | tell | meno | notify> <c.>~RS\n"); 
	 sprintf(text,"~FTCisla farieb:~FW");
 	 for (lines=1;lines<16;lines++) {
	  sprintf(id," %s%d~RS",usercolcodes[lines],lines);
	  strcat (text,id);
	 }
	 strcat(text,".\n");
	 write_user(user,text);
	 sprintf(text,"~FTAktualne nastavenie:\n ");
         write_user(user,text);
         i=0;
         sprintf(text," %s\253S1~RS~FW,  %s\253G1~RS~FW,  %s\253P1~RS~FW,  %sNotify~RS~FW / %s\253Im~RS~FW %s\253Ty~RS~FW.\n",
         user->col[0]?usercolcodes[user->col[0]]:usercolcodes[defaultcols[0]],
         user->col[1]?usercolcodes[user->col[1]]:usercolcodes[defaultcols[1]],
         user->col[2]?usercolcodes[user->col[2]]:usercolcodes[defaultcols[2]],
         user->col[6]?usercolcodes[user->col[6]]:usercolcodes[defaultcols[6]],
         user->col[3]?usercolcodes[user->col[3]]:usercolcodes[defaultcols[3]],
         user->col[4]?usercolcodes[user->col[4]]:usercolcodes[defaultcols[4]]);
	 write_user(user,text);
         return;
	}
        if (!strcmp(word[2],"reset")) {
 	 for (lines=0;lines<7;lines++)
	  user->col[lines]=0;
	 write_user(user,"Standardne farby nastavene.\n"); 
	 return; 
	}
	type=10;
	if (!strncmp(word[2],"sh",2) || !strcmp(word[2],"!")) { type=0; strcpy(serv,"kricania"); }
	if (!strncmp(word[2],"go",2) || !strcmp(word[2],"@")) { type=1; strcpy(serv,"kecania"); } 
	if (!strncmp(word[2],"sa",2) || !strcmp(word[2],"/")) { type=2; strcpy(serv,"hovorenia"); }
	if (!strncmp(word[2],"me",2)) { type=3; strcpy(serv,"mena v telli"); }
	if (!strncmp(word[2],"te",2) || !strcmp(word[2],">")) { type=4; strcpy(serv,"tellu"); }
	if (!strncmp(word[2],"xa",2)) { type=5; strcpy(serv,"v kanali"); }
	if (!strncmp(word[2],"no",2)) { type=6; strcpy(serv,"notify mena v telli"); }
	if (type==10) {
	 write_user(user,"Mozes menit len farbu shoutu, gossipu, sayu, tellu, notify mena a mena v telli.\n");
	 return;
	}
	if (word_count<4) {
	 user->col[type]=0;
	 sprintf(text,"Standardna farba %s%s nastavena.\n",usercols[defaultcols[type]],serv);
	 write_user(user,text);
	 return;
	}
	else
	{
	 lines=atoi(word[3]);
	 if (lines<1 || 15<lines) {
	  write_user(user,"Mozes zadat cislo farby od 1 do 15 (Pouzi '.set color' pre cisla farieb).\n");
	  return;
	 }	  
	 user->col[type]=atoi(word[3]);
 	 sprintf(text,"Farba %s%s~RS~FW nastavena.\n",usercols[user->col[type]],serv);
 	 write_user(user,text);
	 return; 
	} 
} 

if (!strncmp(word[1],"ircs",4)) {
	if (word_count>2) {
		if (strlen(word[2])>10) {
			write_user(user,"Prilis dlhy nazov IRC servera.\n");
			return;
			}
		if ((fp=ropen(IRC_SERVERS,"r"))==NULL) {
			write_user(user,"Sorry, niesu povolene ziadne IRC servre.\n");
			return;
			}
		fscanf(fp,"%s %s %s", id, serv, port);
		while (!feof(fp)) {
			if (!strcmp(word[2],id)) {
				sstrncpy(user->irc_serv,word[2],11);
				sprintf(text,"Nastaveny IRC server: ~OL%s %s~RS.\n", serv, port);
				write_user(user,text);
				fclose(fp);
				return;
				}
			fscanf(fp,"%s %s %s", id, serv, port);
			}
		fclose(fp);
		write_user(user,"Nespravny nazov IRC servera!\n");
		}
	if ((fp=ropen(IRC_SERVERS,"r"))==NULL) {
		write_user(user,"Sorry, niesu povolene ziadne IRC servre.\n");
		return;
		}
	write_user(user,"                    ~OL~FRPovolene IRC servre:\n");
	write_user(user,"~OL~FB.=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-.\n");
	write_user(user,"~OL~FB|         ~OL~FYprikaz          ~FB| ~OL~FY     server               port~FB |\n");
	fscanf(fp,"%s %s %s", id, serv, port);
	while (!feof(fp)) {
		sprintf(text,"~OL~FB| ~RS~FW.set ircserv ~OL%-10s ~FB| ~RS~FT%-25s %s ~OL~FB|~RS\n", id, serv, port);
		write_user(user,text);
		fscanf(fp,"%s %s %s", id, serv, port);        
		}
	fclose(fp);
	write_user(user,"~OL~FB`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'\n");
	return;
	}
	
if (!(strncmp(word[1],"gen",3))) {
        if ((user->sex!=2) && (strcmp(word[3],"sure"))) { 
	sprintf(text,"Uz mas nastavene pohlavie: %s\n",pohl(user,"muz","zena")); 
	write_user(user,text); 
	return; 
	} 
 
        if ((!(strcmp(word[2],"male"))) || (!(strcmp(word[2],"muz")))) { 
	user->sex=1; 
	write_user(user,"Okej, si muz.\n"); 
	if (user->level==CIT) {
	  strcpy(user->in_phrase,"sem vstupil");
	  strcpy(user->out_phrase,"odisiel");
	 }
	return; 
	} 
        if ((!(strcmp(word[2],"female"))) || (!(strcmp(word[2],"zena")))) { 
	user->sex=0; 
	write_user(user,"Okej, si zena.\n"); 
	if (user->level==CIT) {
	  strcpy(user->in_phrase,"sem vstupila");
	  strcpy(user->out_phrase,"odisla");
	 }
	return; 
	} 
        write_user(user,"Ak si muz, napis ~OL.set gender muz (male)\nAk si zena, napis ~OL.set gender zena (female)~RS\n"); 
       return;
	} 
if (!(strncmp(word[1],"hom",3))) { 
  if (word_count<3)
       {
       sprintf(text,"~FTSucasne nastavenie:~FW %s\n",user->homepage);
       write_user(user,text);
       return;
       }	
	inpstr=remove_first(inpstr); 
	set_homepage(user,inpstr); 
	return; 
	} 

if (!(strncmp(word[1],"lin",3))) {
	if (word_count<3) {
		sprintf(text,"Velkost terminaloveho okna mas nastavenu na ~OL%d~RS riadkov.\n",user->lines);
		write_user(user,text);
		return;
		}
	if (!strcmp(word[2],"auto")) {
		write_user(user,"Skusam nastavit pocet riadkov automaticky.\n");
		sprintf(text,"%c%c%c",IAC,DO,TELOPT_NAWS);  /* posli suradnice terminalu */
		write_user(user,text);
		return;
	        }
	lines=atoi(word[2]);
	if ((lines<5) || (lines>150)) {
		write_user(user,"Pocet riadkov musi byt v rozsahu 5 az 150.\n");
		return;
		}
	user->lines=lines;
	sprintf(text,"Nastavil%s si velkost terminalu na ~OL%d~RS riadkov.\n",pohl(user,"","a"),lines);
	write_user(user,text);
	return;
	}

if (!(strncmp(word[1],"wra",3))) {
	if (user->wrap) {
		write_user(user,"Zalamovanie riadkov ~FRzrusene.\n");
		user->wrap=0;
		/* AKY JE OPACNY ESC CHAR ??? :>>> */ /* miesto l das h, lamah! */
		write_user(user,"\033[?7l");  /* zrusi esc */
		return;
		}
	user->wrap=1;
	write_user(user,"Zalamovanie riadkov ~FGnastavene.\n");
	write_user(user,"\033[?7h");  /* Tuto posiela esc na wrap lines!!! */
	return;
	}

if (!(strncmp(word[1],"who",3))) {
  if (word_count==4) {
    if (!strcmp(word[2],"test")) {
      word[3][0]=toupper(word[3][0]);
      word_count=1;
      newwho(user,word[3]);
      return;
     }
    if (!strcmp(word[2],"copy")) {
      word[3][0]=toupper(word[3][0]);
      if (!strcmp(user->name,word[3])) {
        sprintf(text,"Skin si mozes editovat pomocou WHO skin editora na adrese ~OL%s/editor\n",TALKER_WEB_SITE);
        write_user(user,text);
        return;
       }
      if (word[3][0]=='+') type=0; else type=1;
      strcpy(word[4],word[3]);
      sprintf(query,"select `head`,`body`,`legs` from `who` where `username`='%s'",dbf_string(word[4]));
      if (!(result=mysql_result(query))) {
        write_user(user,"Chyba: Nastal problem so zapisovanim who skinu.\n");
        return;
       }
      if (!(row=mysql_fetch_row(result))) {
        if (type==1)
          sprintf(text,"%s nema definovany who skin.\n",word[3]);
        else 
          sprintf(text,"Who skin '%s' neexistuje.\n",word[3]);
        write_user(user,text);
        mysql_free_result(result);
        return;
       }
      sprintf(query,"replace into `who` (`username`,`head`,`body`,`legs`) values ('%s','",user->name);
      strncpy(line,row[0],4096);
      strcat(query,dbf_string(line));
      strcat(query,"','");
      strncpy(line,row[1],4096);
      strcat(query,dbf_string(line));
      strcat(query,"','");
      strncpy(line,row[2],4096);
      strcat(query,dbf_string(line));
      strcat(query,"')");
      mysql_kvery(query);
      if (type==1)
        sprintf(text,"Skopiroval%s si si who skin, ktory ma nastaveny %s.\n",pohl(user,"","a"),word[3]);
      else 
        sprintf(text,"Nastavil%s si si who skin %s.\n",pohl(user,"","a"),word[3]);
      write_user(user,text);
      user->who_type=0;
      mysql_free_result(result);
      return;
     }
   }
  if (word_count==3) {
    if (!strcmp(word[2],"test")) {
      word_count=1;
      testwho(user);
      return;
     }
    if (!strcmp(word[2],"testall") && user->level>=KIN) {
      word_count=1;
      testwhoall(user);
      return;
     }
    switch(toupper(word[2][0])) {
	     case '0': user->who_type=0;
  		       write_user(user,"Vypis .who je nastavny na volitelny who skin.\n");
  		       return; break;
	     case '1': user->who_type=1;
  		       write_user(user,"Vypis .who je nastaveny na typ 1 (basic).\n");
  		       return; break;
	     case '2': user->who_type=2;
  		       write_user(user,"Vypis .who je nastaveny na typ 2 (nature).\n");
  		       return; break;
	     case '3': user->who_type=3;
  		       write_user(user,"Vypis .who je nastaveny na typ 3 (cyberia).\n");
  		       return; break;
	     case '4': user->who_type=4;
  		       write_user(user,"Vypis .who je nastaveny na typ 4 (carneval).\n");
  		       return; break;
	     case '5': user->who_type=5;
  		       write_user(user,"Vypis .who je nastaveny na typ 5 (ocean).\n");
  		       return; break;
	     case '6': user->who_type=6;
  		       write_user(user,"Vypis .who je nastaveny na typ 6 (titanic).\n");
  		       return; break;
	     case '7': user->who_type=7;
  		       write_user(user,"Vypis .who je nastaveny na typ 7 (volcano).\n");
  		       return; break;
	     case '8': user->who_type=8;
  		       write_user(user,"Vypis .who je nastaveny na typ 8 (hell).\n");
  		       return; break;
	     case '9': user->who_type=9;
  		       write_user(user,"Vypis .who je nastaveny na typ 9 (atlantis 2.X).\n");
  		       return; break;
             case 'A': user->who_type=10;
             	       write_user(user,"Vypis .who je nastaveny na typ 10 (playlist).\n");
             	       return; break;
             default:  break;
	     }
	   }
        write_user(user,"~FTPouzitie:~FW .set who <n> - nastavi typ ~OL.who~RS urceny cislom <n>\n");
	sprintf(text,"~FTTypy:~FW     ~OL0~OL - definovatelny who skin!  ~FR%s/editor\n",TALKER_WEB_SITE);
        write_user(user,text);
        write_user(user,"          ~OL1~RS - basic    (zakladny typ, Atlantis)\n");
        write_user(user,"          ~OL2~RS - nature   (typ ladeny do prirodnych farieb)\n");
        write_user(user,"          ~OL3~RS - cyberia  (mierne avangardny sci-fi typ) \n"); 
        write_user(user,"          ~OL4~RS - carneval (karnevalovy typ s girlandami)\n");
        write_user(user,"          ~OL5~RS - ocean    (typ ladeny do farieb oceanu)\n");
        write_user(user,"          ~OL6~RS - titanic  (...was called a ship of dreams)\n");
        write_user(user,"          ~OL7~RS - volcano  (krater duni, lavu budi...)\n");
        write_user(user,"          ~OL8~RS - hell     (stretneme sa v pekle, priatelia)\n");
        write_user(user,"          ~OL9~RS - old one  (stare who z Atlantis v2.X)\n");
        write_user(user,"          ~OLA~RS - playlist (pre milovnikov Winampu!)\n");
        write_user(user,"~FTAk si chces pozriet who skiny prihlasenych ludi, pouzi:  ~OL~FW.set who test\n");
        write_user(user,"~FTUkazka who skinu:   ~OL~FW.set who test <uzivatel>~RS~FT alebo: ~FW~OL.set who test +<skin>\n");
        write_user(user,"~FTSkopirovanie skinu: ~OL~FW.set who copy <uzivatel>~RS~FT alebo: ~FW~OL.set who copy +<skin>\n");

        sprintf(query,"select `username` from `who` where `username` like ('+%%')");
        if ((result=mysql_result(query))) {
          i=0;
          text[0]='\0';
          while ((row=mysql_fetch_row(result))) {
            if (i>0) strcat(text,", ");
            else write_user(user,"~FTK dispozicii su tieto preddefinovane skiny:\n");
            i++;
            if (row[0]) strcat(text,row[0]);
           }
          if (i>0) { 
            strcat(text,"\n");
            write_user(user,text);
           }
          mysql_free_result(result);
         }
        sprintf(text,"~FTMomentalne mas nastaveny who typ~FW %d~FT.\n",user->who_type);
        write_user(user,text);
        return;
       }

if (!(strncmp(word[1],"exa",3))) {
  if (word_count==4) {
    if (!strcmp(word[2],"test")) {
      word[3][0]=toupper(word[3][0]);
      strcpy(word[1],word[3]);
      word_count=2;
      customexamine(user,word[3],0);
      return;
     }
    if (!strcmp(word[2],"copy")) {
      word[3][0]=toupper(word[3][0]);
      if (!strcmp(user->name,word[3])) {
        sprintf(text,"Skin si mozes editovat na adrese ~OL%s/editor\n",TALKER_WEB_SITE);
        write_user(user,text);
        return;
       }
      if (word[3][0]=='+') type=0; else type=1;
      strcpy(word[4],word[3]);
      sprintf(query,"select `examine` from `examines` where `username`='%s'",dbf_string(word[4]));
      if (!(result=mysql_result(query))) {
        write_user(user,"Chyba: Nastal problem so zapisovanim examine skinu.\n");
        return;
       }
      if (!(row=mysql_fetch_row(result))) {
        if (type==1)
          sprintf(text,"%s nema definovany examine skin.\n",word[3]);
        else
          sprintf(text,"Examine skin '%s' neexistuje.\n",word[3]);
        write_user(user,text);
        mysql_free_result(result);
        return;
       }
      sprintf(query,"replace into `examines` (`username`,`examine`) values ('%s','",user->name);
      strncpy(line,row[0],4096);
      strcat(query,dbf_string(line));
      strcat(query,"')");
      mysql_kvery(query);
      if (type==1)
        sprintf(text,"Skopiroval%s si si examine skin, ktory ma nastaveny %s.\n",pohl(user,"","a"),word[3]);
      else 
        sprintf(text,"Nastavil%s si si examine skin %s.\n",pohl(user,"","a"),word[3]);
      write_user(user,text);
      user->examine=0;
      mysql_free_result(result);
      return;
     }
   }
  if (word_count==3) {
    if (!strcmp(word[2],"test")) {
      sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
      if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
        fclose(fp);
        deltempfile(filename);
       }
      word_count=2;
      for(u=user_first;u!=NULL;u=u->next) {
        if (u->type!=USER_TYPE || u->login || u->examine>0) continue;
        strcpy(word[1],u->name);
        customexamine(user,u->name,1);
       }
      switch(more(user,user->socket,filename)) {
        case 0: write_user(user,"Nastal problem s citanim docasneho suboru.\n");  break;
        case 1: user->misc_op=2;
       }
      return;
     }
   }
  switch(word[2][0]) {
    case '0': user->examine=0;
      write_user(user,"Dizajn .examine je nastaveny na typ 0 (definovatelny).\n");
      return;
      break;
     case '1': user->examine=1;
      write_user(user,"Dizajn .examine je nastaveny na typ 1 (novy).\n");
      return;
      break;
    case '2': user->examine=2;
      write_user(user,"Dizajn .examine je nastaveny na typ 2 (stary)\n");
      return;
      break;
    default: break;
   }
  write_user(user,"~FTPouzitie:~FW .set examine <n> - nastavi typ ~OL.examine~RS urceny cislom <n>\n");
  write_user(user,"~FTTypy:~FW     ~OL1~RS - novy     (modro-zlte farby)\n");
  write_user(user,"          ~OL2~RS - stary    (zeleno-biele farby)\n");
  sprintf(text,"          ~OL0~RS - definovatelny examine skin   ~OL~FR%s/editor\n",TALKER_WEB_SITE);
  write_user(user,text);
  write_user(user,"~FTAk si chces pozriet examine skiny prihlasenych ludi, pouzi:  ~OL~FW.set examine test\n");
  write_user(user,"~FTUkazka skinu: ~OL~FW.set examine test <uzivatel>~RS~FT alebo: ~FW~OL.set examine test +<skin>\n");
  write_user(user,"~FTSkopirovanie: ~OL~FW.set examine copy <uzivatel>~RS~FT alebo: ~FW~OL.set examine copy +<skin>\n");

  sprintf(query,"select `username` from `examines` where `username` like ('+%%')");
  if ((result=mysql_result(query))) {
    i=0;
    text[0]='\0';
    while ((row=mysql_fetch_row(result))) {
      if (i>0) strcat(text,", ");
      else write_user(user,"~FTK dispozicii su tieto preddefinovane skiny:\n");
      i++;
      if (row[0]) strcat(text,row[0]);
     }
    if (i>0) { 
      strcat(text,"\n");
      write_user(user,text);
     }
    mysql_free_result(result);
   }
  sprintf(text,"~FTMomentalne mas nastaveny examine typ~FW %d~FT.\n",user->examine);
  write_user(user,text);
  return;
 }
if (!(strncmp(word[1],"rev",3))) {
	if (word_count<3) {
		vwrite_user(user,"Pouzi: .set revt text  -po odhlaseni sa zobrazi cely revtell    (~OL%s~RS)\n",noyes(((user->rt_on_exit&1)!=0)));
		vwrite_user(user,"       .set revt smail -po nasilnom odhlaseni sa revtell posle ako smail (~OL%s~RS)\n",noyes(((user->rt_on_exit&2)!=0)));
		vwrite_user(user,"       .set revt email -po nasilnom odhlaseni sa revtell posle na email  (~OL%s~RS)\n",noyes(((user->rt_on_exit&4)!=0)));
		vwrite_user(user,"       .set revt quit  -po kazdom odhlaseni sa revtell posle ako smail   (~OL%s~RS)\n",noyes(((user->rt_on_exit&8)!=0)));
		return;
		}
	if (!strncmp(word[2],"t",1)) {		
		if (user->rt_on_exit & 1) {
			user->rt_on_exit-=1;
			write_user(user,"Pri odchode sa nebude vypisovat tvoj .revtell\n");
			return;
			}
		user->rt_on_exit+=1;
		write_user(user,"Pri odchode sa vzdy vypise tvoj .revtell buffer.\n");
		return;
		}
	if (!strncmp(word[2],"s",1)) {
		if (user->rt_on_exit & 2) {
			user->rt_on_exit-=2;
			write_user(user,"Posielanie revtellu smailom po nasilnom odhlaseni bolo zrusene.\n");
			return;
			}
		user->rt_on_exit+=2;
		write_user(user,"Pri (nasilnom) odhlaseni sa ti posle revtell buffer ako smail\n");
		return;
		}		

	if (!strncmp(word[2],"e",1)) {
		if (user->rt_on_exit & 4) {
			user->rt_on_exit-=4;
			write_user(user,"Posielanie revtellu e-mailom po nasilnom odhlaseni bolo zrusene.\n");
			return;
			}
		user->rt_on_exit+=4;
		write_user(user,"Pri (nasilnom) odhlaseni sa ti posle revtell buffer na tvoju e-mail adresu.\n");
		return;
		}
                
	if (!strncmp(word[2],"q",1)) {
		if (user->rt_on_exit & 8) {
			user->rt_on_exit-=8;
			write_user(user,"Posielanie revtellu ako smail po kazdom odhlaseni bolo zrusene.\n");
			return;
			}
		user->rt_on_exit+=8;
		write_user(user,"Pri kazdom odhlaseni sa ti posle revtell ako smail.\n");
		return;
		}
	write_user(user,"Nespravny parameter, precitaj si napovedu (.set revtell)\n");
	return;			
	}

if (!(strcmp(word[1],"autofwd"))) {
	if (!user->autofwd) {
		if (user->level<SOL) {
			write_user(user,"Prepac, tuto funkciu moze vyuzivat len uzivatel s levelom VOJAK a vyssim.\n");
			return; /* Aby nevytvarali bogus userov a neblbli s tym */
			}
		if (strpbrk(user->email," <>;$/+*[]\\")) {
			write_user(user,"Mas chybne nastavenu e-mail adresu, pouzi .set email!!\n");
			return;
			}
		if ((!strstr(user->email,".")) || (!strstr(user->email,"@")) || (!isalpha(user->email[strlen(user->email)-1])) || (strlen(user->email)<7)) {
			write_user(user,"Mas chybne nastavenu e-mail adresu, pouzi .set email!\n");
			return;
	   	        } 
	   	user->autofwd=1;
	   	sprintf(text,"Automaticky forward posty je ~FGZAPNUTY~FW, adresa: %s\n",user->email);
	   	write_user(user,text);
	   	write_user(user,"POZOR: Ak je adresa NESPRAVNA, posta sa nedoruci!!!\n");
		user->mailnotify=0;
	   	return;
	   	}
	user->autofwd=0;
	sprintf(text,"Automaticke forwardovanie posty ~FRVYPNUTE~FW.\n");
	write_user(user,text);			
	return;
	}

if (!(strncmp(word[1],"mailn",5))) {
	if (!user->mailnotify) {
		if (user->level<SOL) {
			write_user(user,"Prepac, tuto funkciu moze vyuzivat len uzivatel s levelom VOJAK a vyssim.\n");
			return; /* Aby nevytvarali bogus userov a neblbli s tym */
			}
		if (strpbrk(user->email," <>;$/+*[]\\")) {
			write_user(user,"Mas chybne nastavenu e-mail adresu, pouzi .set email!!\n");
			return;
			}
		if ((!strstr(user->email,".")) || (!strstr(user->email,"@")) || (!isalpha(user->email[strlen(user->email)-1])) || (strlen(user->email)<7)) {
			write_user(user,"Mas chybne nastavenu e-mail adresu, pouzi .set email!\n");
			return;
	   	        } 
		if (user->autofwd) {
			write_user(user,"Tato funkcia je bezvyznamna ak mas zapnuty autoforward.\n");
			return;
			}
	   	user->mailnotify=1;
	   	sprintf(text,"Upozornovanie na novu postu emailom ~FGZAPNUTE~FW, adresa: %s\n",user->email);
	   	write_user(user,text);
	   	write_user(user,"POZOR: Ak je adresa NESPRAVNA, upozornenie o doslej poste sa nedoruci!!!\n");
	   	return;
	   	}
	user->mailnotify=0;
	sprintf(text,"Upozornovanie na novu postu emailom ~FRVYPNUTE~FW.\n");
	write_user(user,text);			
	return;
	}
	
if (!(strncmp(word[1],"mot",3)) && (user->level>=GOD)) {
	if (word_count<3) {
		write_user(user,"Pouzi: .set motd <subor>\n");
		return;
		}
	if (strpbrk(word[2]," /\\.|<>")) write_user(user,"Nespravne zadany subor!\n");
	if (strlen(word[2])>20) write_user(user,"Prilis dlhy nazov suboru!\n");
	sstrncpy(motd2_file,word[2],20);
	sprintf(text,"Subor motd2 nastaveny na: ~OL%s~RS\n",motd2_file);
	write_user(user,text);
	return;
	}	
	
if (!(strncmp(word[1],"ema",3))) {
	if (!user->level) {
		write_user(user,"Ako novy uzivatel musis pouzit prikaz .request!\n");
		return;
		}
	if (word_count<3) {
		sprintf(text,"Nastavena email adresa: %s\n",user->email);
		write_user(user,text);
		if (strpbrk(user->email," <>;$/+*[]\\|")) {
			write_user(user,"POZOR: Adresa je nastavena chybne, nebude fungovat autoforward posty!!\n");
			}
		else 
		if ((!strstr(user->email,".")) || (!strstr(user->email,"@")) || (!isalpha(user->email[strlen(user->email)-1])) || (strlen(user->email)<7)) {
			write_user(user,"POZOR: Adresa je nastavena chybne, nebude fungovat autoforward posty!!\n");
	   	        } 
	   	if (!user->autofwd) write_user(user,"Autoforward posty je ~FRVYPNUTY.\n");
	   		else write_user(user,"Autoforward posty je ~FGZAPNUTY.\n");
		return;
		}

	if (strpbrk(word[2]," ;$/+*[]\\<|>")) {
		write_user(user,"Nespravne zadana email adresa, skus to znovu!!\n");
		return;
		}
	if ((!strstr(word[2],".")) || (!strstr(word[2],"@")) || (!isalpha(word[2][strlen(word[2])-1])) || (strlen(word[2])<7)) {
		write_user(user,"Nespravne zadana email adresa, skus to znovu!!\n");
		return;
  	        } 
  	        
  	sstrncpy(user->email, word[2], EMAIL_LEN);
  	colour_com_strip(user->email);
  	sprintf(text,"%s si svoju email adresu na: %s\n", pohl(user,"Nastavil","Nastavila"), user->email);
  	write_user(user,text);
  	if (!(fp=ropen("misc/users.email","a"))) return; /*APPROVED*/
  	
  	fprintf(fp,"%-12s %s\n",user->name, user->email);
  	 fclose(fp);
	return;
	}

if (!strncmp(word[1],"pag",3)) {
	if (user->pagewho) {
		user->pagewho=0;
		write_user(user,"Vypis .who sa bude vypisovat bez zastavovania. Pozor, tato volba nemusi vzdy\nfungovat korektne. Ak sa zobrazuje neuplny who list, je nutne zapnut\nstrankovanie.\n");
		return;
		}
	user->pagewho=1;
	write_user(user,"Vypis .who sa bude vypisovat po strankach.\n");
	return;
       }
if (!strncmp(word[1],"ircni",5)) {
	if (word_count<3) {
		sprintf(text,"Tvoj aktualny IRC nick: ~OL%s~RS\n", user->irc_defnick);
		write_user(user,text);
		return;
		}
	if (strlen(word[1])>20) {
		write_user(user,"Prilis dlhy nick.\n");
		return;
		}
	sstrncpy(user->irc_defnick, word[2], 20);
	sprintf(text,"IRC nick bol nastaveny na: ~OL%s~RS\n", user->irc_defnick);
	write_user(user,text);
	return;
	}
	
if (!strncmp(word[1],"ircna",5)) {
	if (word_count<3) {
		sprintf(text,"Tvoj aktualny IRCname: ~OL%s~RS\n", user->irc_name);
		write_user(user,text);
		return;
		}
	if (strlen(inpstr)>49) {
		write_user(user,"Prilis dlhy ircname.\n");
		return;
		}
	inpstr=remove_first(inpstr);
	sstrncpy(user->irc_name, inpstr, 49);
	sprintf(text,"IRCname bol nastaveny na: ~OL%s~RS\n", user->irc_name);
	write_user(user,text);
	return;
	}
if (!strncmp(word[1],"adv",3)) {
  write_user(user,"~FGAdvancovane parametre:\n\n");
  if (user->level>=GOD) {
    write_user(user,"~FT.set tunel <cislo>        ~FW- nastavovanie casu tunela\n");
    write_user(user,"~FT.set multiply             ~FW- plnohodnotny klon, mozes sa znova prihlasit.\n");
    write_user(user,"~FT.set speed <rychlost>     ~FW- rychlost pohybu zvierat\n");
    write_user(user,"~FT.set maxtimeouts <pocet>  ~FW- pocet write-timeoutov po ktorych sa vykopava\n");
    write_user(user,"~FT.set maxtimeoutz <pocet>  ~FW- 2 stupen kontroly write-timeoutov\n");
    write_user(user,"~FT.set motd <subor>         ~FW- zmeni subor s novymi spravami (motd2)\n");
    write_user(user,"~FT.set logging [on|off]     ~FW- vypne/zapne systemove logovanie\n");
    write_user(user,"~FT.set resolver [on|off]    ~FW- vypne/zapne resolvovanie IPciek\n");
    write_user(user,"~FT.set logcommands [on|off] ~FW- vypne/zapne logovanie prikazov\n");
    write_user(user,"~FT.set swearban [on|off]    ~FW- vypne/zapne blokovanie nadavok\n");
    write_user(user,"~FT.set checkident [on|off]  ~FW- vypne/zapne zistovanie identity\n");
    write_user(user,"~FT.set minlogin <NONE|level>~FW- nastavi minlogin\n");
    write_user(user,"~FT.set team1 - team4 <meno> ~FW- nastavi meno teamu na brutalise\n");
    write_user(user,"~FT.set maxsms <pocet>       ~FW- max. pocet SMSiek odoslanych za den\n");
    write_user(user,"~FT.set timeoutafks [on|off] ~FW- max. AFK necinnost\n");
    write_user(user,"~FT.set useridle <sekundy>   ~FW- max. uzivatelska necinnost\n");
    write_user(user,"~FT.set loginidle <sekunkdy> ~FW- kedy ukonci spojenie pri prihlasovani\n");
    write_user(user,"~FT.set timeoutmaxlvl <level>~FW- level pre max. uzivatelsku necinnost\n");
    write_user(user,"~FT.set wplevel <ievel>      ~FW- min. level pre prihlasenie cez wizport\n");
   } 
  if (user->level>=KIN) {
   write_user(user,"~FT.set autocreate           ~FW- nastavi rychlost vytvarania predmetov\n");
   write_user(user,"~FT.set dynamic              ~FW- reakcia rychlosti doplnovania na pocet userov\n");
   write_user(user,"~FT.set timeout <minuty>     ~FW- nastavi cas po ktorom ta odpoji\n");
   write_user(user,"~FT.set wizpass              ~FW- nastavi bezpecnostne heslo proti zneuzitiu\n");
   write_user(user,"~FT.set name2 <user> <2.pad> ~FW- nastavi sklonovane meno uzivatela\n");
/*   write_user(user,"~FT.set mail2sms <on|off>    ~FW- nastavi forwardovanie .smailov na mobil.\n\n"); */
  }
  return;
 }
if (!strcmp(word[1],"disp") || !strcmp(word[1],"show")) {
  write_user(user,"Akutalne nastavenia:\n");
  sprintf(text,"~FTgender:~FW     %-28s~FTage:~FW        %d\n",pohl(user,"muz","zena"),user->age);
  write_user(user,text);
  sprintf(text,"~FTexamine:~FW    %-28d~FTwho:~FW        %d %s\n",user->examine,user->who_type,user->pagewho?"(pagewho)":"(no pagewho)");
  write_user(user,text);
  sprintf(text,"~FTlines:~FW      %-28d~FTwrap:~FW       %s\n",user->lines,user->wrap?"on ":"off");
  write_user(user,text);
  sprintf(text,"~FTsecuretell:~FW %-28s~FTtimeinrev:~FW  %s\n",user->shortcutwarning?"Y":"N",user->timeinrevt?"Y":"N");
  write_user(user,text);
  serv[0]='\0';
  if (user->rt_on_exit & 1) strcat(serv,"tell ");
  if (user->rt_on_exit & 2) strcat(serv,"smail ");
  if (user->rt_on_exit & 4) strcat(serv,"email ");
  if (user->rt_on_exit & 8) strcat(serv,"quit");
  sprintf(text,"~FTlang:~FW       %-28s~FTrevtell:~FW    %s\n",langword[user->lang+1],serv);
  write_user(user,text);
  if (user->level<KIN) 
    sprintf(text,"~FTautoafk:~FW    %-28d\n",user->goafkafter);
  else 
    sprintf(text,"~FTautoafk:~FW    %-28d~FTtimeout:~FW    %d\n",user->goafkafter,user->killmeafter);
  write_user(user,text);
  if (user->level>=SOL) {
    if (user->allowsms==0) sprintf(serv,"all");
    else if (user->allowsms==1) sprintf(serv,"notify");
    else if (user->allowsms==2) sprintf(serv,"noignore");
    else if (user->allowsms==3) sprintf(serv,"noone");
    sprintf(text,"~FTsms:~FW        %-28s~FTmobil:~FW      %-13s\n",serv,user->mobile);
    write_user(user,text);
   }
  sprintf(text,"~FTautofwd:~FW    %-28s~FTemail:~FW      %s\n",user->autofwd?"on ":"off",user->email);
  write_user(user,text);
  if (user->level>=SAG) {
    sprintf(text,"~FTircnick:~FW    %-28s~FTircserv:~FW    %s\n",user->irc_defnick,user->irc_serv);
    write_user(user,text);
    strcpy(texthb,user->irc_name);
    if (strchr(texthb,'\n')) for(i=0;i<strlen(texthb);i++) if (texthb[i]=='\n') texthb[i]=' ';
    sprintf(text,"~FTircname:~FW    %s\n",texthb);
    write_user(user,text);
   }
  sprintf(text,"~FThomepage:~FW   %s\n",user->homepage);
  write_user(user,text);
  sprintf(text,"~FTMsg Tell:~FW   %-20s   ~FTMsg MyTell:~FW     %-20s\n",user->tellmsg,user->tellmsg2);
  if (user->tellmsg[0] || user->tellmsg2[0]) write_user(user,text);
  sprintf(text,"~FTMsg Shout:~FW  %-20s   ~FTMsg MyShout:~FW    %-20s\n",user->shoutmsg,user->shoutmsg2);
  if (user->shoutmsg[0] || user->shoutmsg2[0]) write_user(user,text);
  sprintf(text,"~FTMsg Say:~FW    %-20s   ~FTMsg MySay:~FW      %-20s\n",user->saymsg,user->saymsg2);
  if (user->saymsg[0] || user->saymsg2[0]) write_user(user,text);
  sprintf(text,"~FTMsg Gossip:~FW %-20s   ~FTMsg MyGossip:~FW   %-20s\n",user->gossipmsg,user->gossipmsg2);
  if (user->gossipmsg[0] || user->gossipmsg2[0]) write_user(user,text);
  sprintf(text,"~FTMsg Wizsh:~FW  %-20s   ~FTMsg MyWizsh:~FW    %-20s\n",user->wizshmsg,user->wizshmsg2);
  if (user->wizshmsg[0] || user->wizshmsg2[0]) write_user(user,text);

  sprintf(text,"~FTNastavenie farieb (.set color):\n ");
  write_user(user,text);
  sprintf(text," %s\253S1~RS~FW,  %s\253G1~RS~FW,  %s\253P1~RS~FW,  %sNotify~RS~FW / %s\253Im~RS~FW %s\253Ty~RS~FW.\n",
  user->col[0]?usercols[user->col[0]]:usercols[defaultcols[0]],
  user->col[1]?usercols[user->col[1]]:usercols[defaultcols[1]],
  user->col[2]?usercols[user->col[2]]:usercols[defaultcols[2]],
  user->col[6]?usercols[user->col[6]]:usercols[defaultcols[6]],
  user->col[3]?usercols[user->col[3]]:usercols[defaultcols[3]],
  user->col[4]?usercols[user->col[4]]:usercols[defaultcols[4]]);
  write_user(user,text);
  return;
 }	
 
write_user(user,"Pouzi: .set <parameter> [hodnota]\n");
/* write_user(user,"\n~FGParametre:\n\n"); */

write_user(user,"~FT.set gender <muz | zena>  ~FW- nastavi pohlavie\n");
write_user(user,"~FT.set color <prikaz>       ~FW- moznost nastavit si niektore farby (shout atd.)\n");
write_user(user,"~FT.set homepage <adresa>    ~FW- tymto si mozes nastavit adresu svojej homepage\n");
write_user(user,"~FT.set birth <datum>        ~FW- datum narodenia, z ktoreho sa pocita vek (v .ex)\n");
write_user(user,"~FT.set lines <riadky>       ~FW- nastavi velkost terminaloveho okna (pocet riad.)\n");
write_user(user,"~FT.set wrap                 ~FW- nastavi/zrusi zalamovanie riadkov\n");
write_user(user,"~FT.set who [typ]            ~FW- vypis .who bude mat iny design\n");
write_user(user,"~FT.set pagewho              ~FW- vypisy .who sa budu vypisovat po strankach\n");
write_user(user,"~FT.set revtell <te|sm|em>   ~FW- vypis/posielanie revtell bufferu po odhlaseni\n");
write_user(user,"~FT.set email <tvoj_email>   ~FW- nastavi tvoju email adresu - pri autoforwarde\n");
write_user(user,"~FT.set autofwd              ~FW- kazda nova posta (smail) sa ti posle aj na email\n");
write_user(user,"~FT.set examine              ~FW- nastavi styl vypisu .examine\n");
write_user(user,"~FT.set autoafk <minuty>     ~FW- zmeni cas za aky sa nastavi autoafk\n");
if (user->level>=SAG) {
write_user(user,"~FT.set ircnick|ircname|serv ~FW- nastavi tvoj nick,popis,server pre IRC\n");
}
write_user(user,"~FT.set securetell <on|off>  ~FW- ochrana proti omylom pri telloch ak pouzivas skratky\n");
write_user(user,"~FT.set timeinrev <on|off>   ~FW- zapisovanie casu pri kazdom riadku v revtelli\n");
write_user(user,"~FT.set priority <en|sk>     ~FW- nastavi ktore prikazy (a skratky) maju prioritu.\n");
if (user->level>=SOL) {
  write_user(user,"~FT.set mobil                ~FW- nastavi cislo mobilu\n");
  write_user(user,"~FT.set sms vsetci, notify.. ~FW- nastavis si kto ti moze posielat SMS spravy\n");
 }
write_user(user,"~FT.set msg tell,shout,..    ~FW- nastavi hlasky ktore sa ti zobrazia pri telli,..\n");
if (user->level>=KIN)
 write_user(user,"~FWPouzi: ~FG.set advanced ~FWpre dalsie nastavenia.\n");
write_user(user,"~FWPouzi: ~FG.set show~FW pre zobrazenie aktualnych nastaveni.\n");
} 
 
/* INICIALIZACIA Amfiteatra  (R) */

void amfiteater(user, force)
UR_OBJECT user;
int force;
{
char filename[80];

if (!force) { if ((play.time=play_nxt())==ZERO) play.on=2; return; } 

if ((!strcmp(word[1],"update")) && (play.on!=1) && (user->level>KIN))
   {
   write_user(user,"~FTNovy program amfiteatru bol inicializovany!~FW\n");
   if ((play.time=play_nxt())==ZERO) { play.on=2;
	write_user(user,"~OL~FRChyba v inicializacii amfiteatru! :>\n");
	}
   return;
   }
else if ((!strcmp(word[1],"status")) && (user->level>KIN))  
   {
   sprintf(text,"~FTStatus: %d\nNajblizsie predstavenie: %s (%s)\nNajblizi cas predstavenia: %dh%dm\nStatus: (0=caka sa 1=hra sa 2=error)\n",play.on,play.name,play.nazov,play.hodina,play.minuta);
   write_user(user,text);
   return; 
   }
/* vypise bez parametru */


else  {     sprintf(text,"\n\nNajblizsie predstavenie: %s; najblizi cas predstavenia: %dh%dm\n",play.nazov,play.hodina,play.minuta);
            write_user(user,text);   
            sprintf(filename,"amfiteater/predstavenie.atl");
            switch(more(user,user->socket,filename)) {
                  case 0: write_user(user,"Nenasiel sa subor s predstavenim.\n");  break;
                  case 1: user->misc_op=2;
                  }
      }
return; 
}

/************************************************************/
/** Spakky - AUTH FUNKCIE PRE real_user **/
/** NEVYHADZOVAT - NEMENIT :)) **/
/** Trhni si nohof, mas tam bugy f deklaracii :> (RR) **/
/************************************************************/
static void clearsa(struct sockaddr_in *sa);
/* Zadeklarujem to tu, lebo je to static... */

static void clearsa(sa)
    struct sockaddr_in *sa;
{
    register char *x;
    for (x = (char *) sa;x < sizeof(*sa) + (char *) sa;++x)
	*x = 0;
}

int auth_fd2(fd,inlocal,inremote,local,remote)
    register int fd;
    register unsigned long *inlocal;
    register unsigned long *inremote;
    register unsigned short *local;
    register unsigned short *remote;
{
    struct sockaddr_in sa;
    unsigned int dummy;
    
    dummy = sizeof(sa);
    if (getsockname(fd,(struct sockaddr *)&sa,&dummy) == -1)
	return -1;
    if (sa.sin_family != AF_INET)
    {
	errno = EAFNOSUPPORT;
	return -1;
    }
    *local = ntohs(sa.sin_port);
    *inlocal = sa.sin_addr.s_addr;
    dummy = sizeof(sa);
    if (getpeername(fd,(struct sockaddr *)&sa,&dummy) == -1)
	return -1;
    *remote = ntohs(sa.sin_port);
    *inremote = sa.sin_addr.s_addr;
    return 0;
}


static char ruser[SIZ];
static char realbuf[SIZ];
static char *buf;

char *auth_tcpuser(in,local,remote)
    register unsigned long in;
    register unsigned short local;
    register unsigned short remote;
{
    return auth_tcpuser2(0,in,local,remote);
}


#define CLORETS(e) { saveerrno = errno; close(s); errno = saveerrno; return e; }

int auth_tcpsock(inlocal,inremote)
    register unsigned long inlocal;
    register unsigned long inremote;
{
    struct sockaddr_in sa;
    register int s;
    register int fl;
    register int saveerrno;
    
 if (sigsetjmp(save_state,1)!=0) {
	reset_alarm();
	return -1;
	}
		   
    if ((s = socket(AF_INET,SOCK_STREAM,0)) == -1)
	return -1;
    if (inlocal)
    {
	clearsa(&sa);
	sa.sin_family = AF_INET;
	sa.sin_port = 0;
	sa.sin_addr.s_addr = inlocal;
	if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) == -1)
	    CLORETS(-1)
	    }
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
	CLORETS(-1);
    if (fcntl(s,F_SETFL,NBLOCK_CMD | fl) == -1)
	CLORETS(-1);
    clearsa(&sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(auth_tcpport);
    sa.sin_addr.s_addr = inremote;
    #ifndef WIN32
    siginterrupt(SIGALRM,1);
    #endif
    signal(SIGALRM,timeout_auth);
  
    alarm(5);                                                                 
    if (connect(s,(struct sockaddr *)&sa,sizeof(sa)) == -1)
	{
	reset_alarm();
	if (errno != EINPROGRESS) CLORETS(-1);
	}
	
    reset_alarm();
    return s;	
}

void timeout_auth()
{
  sprintf(text,"Time-out pri auth po 10 sekundach\n");
  write_syslog(text,1);
  siglongjmp(save_state,1);
}
      

char *auth_tcpuser2(inlocal,inremote,local,remote)
    register unsigned long inlocal;
    register unsigned long inremote;
    register unsigned short local;
    register unsigned short remote;
{
    register int s;
    
    s = auth_tcpsock(inlocal,inremote);
    if (s == -1)
	return 0;
    return auth_sockuser(s,local,remote);
}

char *auth_sockuser(s,local,remote)
    register int s;
    register unsigned short local;
    register unsigned short remote;
{
    return auth_sockuser2(s, local, remote, auth_rtimeout);
}

char *auth_sockuser2(s,local,remote,rtimeout)
    register int s;
    register unsigned short local;
    register unsigned short remote;
    int rtimeout;
{
    register int buflen;
    register int w;
    register int saveerrno;
    char ch;
    unsigned short rlocal;
    unsigned short rremote;
    register int fl;
    fd_set wfds;
    __sighandler_t old_sig;
    struct timeval rtv;
    
    old_sig = signal(SIGPIPE, SIG_IGN);
    
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);

    rtv.tv_sec = rtimeout;
    rtv.tv_usec = 0;
    
    select(s + 1,
	   (fd_set *) 0,
	   &wfds,(fd_set *) 0,
	   (struct timeval *) &rtv);
    
    /* now s is writable */
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
    {
	signal(SIGPIPE, old_sig);
	CLORETS(0);
    }
    if (fcntl(s,F_SETFL,~NBLOCK_CMD & fl) == -1)
    {
	signal(SIGPIPE, old_sig);
	CLORETS(0);
    }
    buf = realbuf;
    sprintf(buf,"%u , %u\r\n",(unsigned int) remote,(unsigned int) local);
    /* note the reversed order---the example in RFC 931 is misleading */
    buflen = strlen(buf);
    while ((w = twrite(s,buf,buflen)) < buflen)
	if (w == -1) /* should we worry about 0 as well? */
	{
	    signal(SIGPIPE, old_sig);
	    CLORETS(0);
	}
	else
	{
	    buf += w;
	    buflen -= w;
	}
    buf = realbuf;
    
    do
    {
	fd_set rd_fds;
	
	rtv.tv_sec = rtimeout;
	rtv.tv_usec = 0;
    
	FD_ZERO(&rd_fds);
	FD_SET(s, &rd_fds);
	if (select(s+1,
		   &rd_fds,
		   (fd_set *) 0,
		   (fd_set *) 0,
		   &rtv) == 0)
	{
	    w = -1;
	    goto END;
	}
	
	if ((w = read(s,&ch,1)) == 1)
	{
	    *buf = ch;
	    if ((ch != ' ') && (ch != '\t') && (ch != '\r'))
		++buf;
	    if ((buf - realbuf == sizeof(realbuf) - 1) || (ch == '\n'))
		break;
	}
    } while (w == 1);
    
    END: 
    signal(SIGPIPE, old_sig);
    if (w == -1)
	CLORETS(0)
	    *buf = 0;
    
    if (sscanf(realbuf, "%hu,%hu: USERID :%*[^:]:%s",
	       &rremote, &rlocal, ruser) < 3)
    {
	close(s);
	errno = EIO;
	/* makes sense, right? well, not when USERID failed to match ERROR */
	/* but there's no good error to return in that case */
	return 0;
    }
    if ((remote != rremote) || (local != rlocal))
    {
	close(s);
	errno = EIO;
	return 0;
    }
    /* we're not going to do any backslash processing */
    close(s);
    return ruser;
}
 
void whois(user)
UR_OBJECT user;
{
UR_OBJECT u,uu;
char smalbuf[4];

if (word_count<2) {
	write_user(user,"Pouzi: .realuser <uzivatel>\n");
	return;
	}
if (!strncmp(word[1],"same",4)) {
  int num,total=0;
  for(u=user_first;u!=NULL;u=u->next) u->temp=0;
  for(u=user_first;u!=NULL;u=u->next) {  
    if (u->type!=USER_TYPE || u->login || !strchr(u->real_ident,'@')
    || u->temp==-1) continue;
    num=0;
    sprintf(text,"~OL%s~RS: %s",u->real_ident,u->name); 
    for(uu=u->next;uu!=NULL;uu=uu->next) {
      if (uu->type!=USER_TYPE || uu->login || !strchr(uu->real_ident,'@')
      || uu->temp==-1) continue;
      if (!strcmp(u->real_ident,uu->real_ident)) {
        u->temp++;
        uu->temp=-1;
        strcat(text,", ");
        strcat(text,uu->name);
        num++;
       }
     }
    if (num>0) {
      strcat(text,"\n");
      write_user(user,text);
      total++;
     }
   }
  if (total==0) write_user(user,"~FTZo ziadneho konta nie je prihlasenych naraz viac userov.\n");
  return;
 }

if ((u=get_user(word[1]))==NULL) {
	write_user(user, notloggedon);
	return;
	}
sprintf(text,"~FTTotoznost usera %s je: ~OL~FR%s~RS~FW\nNova poziadavka sa spracovava...\n",u->name,u->real_ident);
write_user(user,text);
if (u->remote_ident[0]) {
  sprintf(text,"~FTTotoznost na ~OL~FW%s~RS~FT: ~OL~FR%s~RS~FW\n",u->site,u->remote_ident);
  write_user(user,text);
 }

if (!strcmp(word[2],"remote")) {
   sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_NEW_ENVIRON);
   write2sock(u,u->socket,smalbuf,0);
   return;
   }

cmon_auth(u);
/* sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_NEW_ENVIRON);
write2sock(user,user->socket,smalbuf,0); */

}

void info_users(user)
UR_OBJECT user;
{
UR_OBJECT u;
char emforward[60];

write_user(user,"~RS~FM+--------------+---+---------------------------+---------------------------+\n");
write_user(user,"~RS~FM|     ~OL~FTMeno     ~RS~FM|~OL~FTWho~RS~FM|   ~OL~FTAutoforward na E-mail   ~RS~FM|      ~OL~FTRealna identita      ~RS~FM|\n");
for(u=user_first;u!=NULL;u=u->next) {
	if ((u->type!=USER_TYPE) || (u->room==NULL) || (u->login)) continue;
	if (u->autofwd) sstrncpy(emforward,u->email,25);
		else strcpy(emforward,"-----------------------------------");	
	sprintf(text,"~FM| ~OL~FW%-12s ~RS~FM|~OL~FR%2d ~RS~FM| ~OL~FW%-25.25s ~RS~FM| ~OL~FY%-25.25s ~RS~FM|\n~RS",u->name,u->who_type,emforward, u->real_ident);
	write_user(user,text);
	}
write_user(user,"~FM+--------------+---+---------------------------+---------------------------+\n");

}

/************************************************************
 Spakky - real_user VYPISE KTO JE V SKUTOCNOSTI DANY USER
 Na userovom serveri musi bezat ident daemon (tcp port 113)
*************************************************************/

char *real_user(UR_OBJECT gdo)
{
static char ru[100];
 unsigned long  inlocal;
 unsigned long  inremote;
 unsigned short local;
 unsigned short remote;
 char *real_name;

if (gdo==NULL) { sprintf(ru, "<UNKNOWN>"); return (ru); } 
 
    auth_fd2(gdo->socket, &inlocal, &inremote, &local, &remote);        
    if ((real_name = auth_tcpuser2(inlocal, inremote, local, remote)) == NULL ) 
      {                  
       sprintf(ru, "<UNKNOWN>");
       return(ru);
      } 
     else 
      {      
       if ((strlen(real_name)+strlen(gdo->site)+1)>79) {
       	sprintf(ru,"<UNKNOWN>");
       	return ru;
       	}
       else {
	       sprintf(ru,"%s@%s",real_name, gdo->site);
	       return(ru);
	    }
      }   
 }

void info(user, typ)
UR_OBJECT user;
int typ;
{
char filename[80];
if (typ==1) {
            sprintf(filename,"select body from files where filename='faq'");
            switch(sqlmore(user,user->socket,filename)) {
                  case 0: write_user(user,"Nenasiel sa subor s FAQ!\n");  break;
                  case 1: user->misc_op=222;
                  }
	}

if (typ==2) {
/*	    sprintf(filename,"%s/rules",DATAFILES);
            switch(more(user,user->socket,filename)) { */
	    sprintf(filename,"select body from files where filename='rules'");
            switch(sqlmore(user,user->socket,filename)) {
                  case 0: write_user(user,"Nenasiel sa subor s pravidlami (rules)!\n");  break;
                  case 1: user->misc_op=222;
                  }
	}
}


int ammunition(UR_OBJECT user,int vec)
{
int where=-1,ammo=0;
 
 if (vec<0) return 0;
 if (predmet[vec]==NULL) return 0;
 if (predmet[vec]->function!=9) return 0; 
 where=is_in_hands(user,vec);
 if (where>-1 && predmet[vec]->uinphr!=NULL) {
   ammo=user->dur[where];
   sprintf(text,"%s\n",parse_phrase(predmet[vec]->uinphr,user,NULL,NULL,-ammo-1));
   write_user(user,text);
   return 1;
  }
 return 0;
}

void look(user)
UR_OBJECT user;
{
RM_OBJECT rm;
UR_OBJECT u;
char temp[82],/* null[1], */line[451],filename[81];
char ptr[10];
int i,y, exits,users,ap,vel,vec,myp;
FILE *fp;

if (word_count>1 && com_num==LOOK) { /* skusime predmet */
  vec=expand_predmet(word[1]);
  if (vec>-1) {
    sprintf(filename,"predmety/%s",predmet[vec]->name);  
    if (!(fp=ropen(filename,"r"))) {  /*APPROVED*/
      if (!ammunition(user,vec)) write_user(user,"K tomuto predmetu niet opisu!\n");
      return; 
     }			
    else {  
      fgets(line,440,fp);  
      while(!feof(fp)) {  
        write_user(user,line);  
        fgets(line,440,fp);  
       }  
      fclose(fp);  
     }   
    ammunition(user,vec);	
    return;
   }	
 }

if (word_count==2 && user->level>=GOD) {
  rm=get_room(word[1],user);
  if (rm==NULL) rm=user->room; 
 }
else
 rm=user->room;
if (rm->access & PRIVATE) sprintf(text,"\n~FT+--- ~FG%s: ~OL~FR%s~RS~FT ",langselect(user,"Lokacia","Location"),langselect(user,rm->name,rm->name_en));
else sprintf(text,"\n~FT+--- ~FG%s: ~OL~FW%s~RS~FT ",langselect(user,"Lokacia","Location"),langselect(user,rm->name,rm->name_en));
if (rm->access==GOD_PRIVATE) sprintf(text,"\n~FT+--- ~FG%s: ~OL~FY%s~RS~FT ",langselect(user,"Lokacia","Location"),langselect(user,rm->name,rm->name_en));
strcat(text,"-----------------------------------------------------------------------------");
text[80+colour_com_count(text)]='\0';
strcat(text,"+\n");
write_user(user,text);

/* ()STROV -> zobraz alternativne popisky :) */
if ((!strcmp(rm->name,SHIPPING_HOME)) && ship_timer==2)
  write_user(user,alt_home_desc);
else if ((!strcmp(rm->name,SHIPPING_ISLAND)) && ship_timer==4)
  write_user(user,alt_island_desc);
  	else if ((!strcmp(rm->name,SHIPPING_SHIP)) && ship_timer==1)
  		write_user(user,alt_plachetnica_desc);
else  write_user(user,langselect(user,rm->desc_sk,(rm->desc_en==NULL)?rm->desc_sk:rm->desc_en));
 
if ((!strcmp(rm->name,FLYER_ROOM)) && (!flyer.pozicia)) {
	write_user(user,"\n~FG[Detailnejsie informacie o riadeni letunu dostanes zadanim prikazu ~OL.pomoc letun~RS~FG]\n");
	}
if ((!strcmp(rm->name,FLYER_ROOM)) && (flyer.pozicia)) {
	sprintf(text,"\n~FG[Pilot: ~OL%-12.12s~RS~FG - Sustredenie: ~OL%d~RS~FG Vykon: ~OL%d~RS~FG Vyska: ~OL%d~RS~FG Palivo: ~OL%d~RS~FG]\n",flyer.pilot, flyer.mind,flyer.vykon,flyer.vyska,flyer.palivo);
	write_user(user,text);
	}	

users=0;
if (strcmp(rm->name,"arena")) { /* ak nie je v arene */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->room!=rm || u==user || (!u->vis && u->level>user->level) || u->login)
            continue;
      if (rm->group==4 && user->reveal>0 && u->hidden==1 && (u->team==0 || u->team!=user->team)) continue;
      if (!users++) {
        if (rm==user->room) write_user(user,langselect(user,"\n~FTOkrem teba tu su:\n","\n~FTYou can see:\n"));
        else {
          sprintf(text,"\n~FT%s su:\n",rm->where);
          text[4]=toupper(text[4]);
          write_user(user,text);
         }
       }
      texthb[0]='\0';
      vec=get_vehicle(u);
      if (vec>-1) sprintf(texthb,"~FG%s(na %s)~FW ",farba_p(vec),predmet[vec]->inytiv);
      if (u->afk) strcpy(ptr,"(AFK)"); else strcpy(ptr,"");
      if (!u->vis) sprintf(text,"     ~FR*~RS%s%s~RS~FW %s ~RS %s%s\n",rm->group==4?teamcolor[u->team]:"",u->name,u->desc,ptr,texthb);
      else sprintf(text,"      %s%s~RS~FW %s ~RS %s%s\n",rm->group==4?teamcolor[u->team]:"",u->name,u->desc,ptr,texthb);
      write_user(user,text);
      }
if (!users) {
  if (rm==user->room) write_user(user,langselect(user,"\n~FTNikto okrem Teba tu nie je.\n","\n~FTYou are all alone here.\n"));
  else {
    sprintf(text,"\n~FT%s nikto nie je.\n",rm->where);
    text[4]=toupper(text[4]);
    write_user(user,text);
   }
 }
write_user(user,"\n");
}
else {  /* ak JE v arene - vypise naviac aj LEVEL a ARENA POINTY */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->room!=rm || u==user || (!u->vis && u->level>user->level))
            continue;
      if (!users++) {
        if (rm==user->room) write_user(user,langselect(user,"\n~FTOkrem teba tu su:\n","\n~FTYou can see:\n"));
        else {
          sprintf(text,"\n~FT%s su:\n",rm->where);
          text[4]=toupper(text[4]);
          write_user(user,text);
         }
       }
      if (u->afk) strcpy(ptr,"(AFK)"); else strcpy(ptr,"");
      ap=u->ap;
      if (!u->vis) sprintf(text,"     ~FR*~RS%s %s ~RS[%s] - ~OL~FW%d~RS Arena %s  %s\n",u->name,u->desc,u->cloak?level_name[u->cloak]:level_name[u->level],ap,skloncislo(ap,"bod","body","bodov"),ptr);
      else sprintf(text,"      %s %s ~RS[%s] - ~OL~FW%d ~RSArena %s  %s\n",u->name,u->desc,u->cloak?level_name[u->cloak]:level_name[u->level],ap,skloncislo(ap,"bod","body","bodov"),ptr);
      write_user(user,text);
      }
if (!users) {
  if (rm==user->room) write_user(user,langselect(user,"\n~FTNikto okrem Teba tu nie je.\n","\n~FTYou are all alone here.\n"));
  else {
    sprintf(text,"\n~FT%s nikto nie je.\n",rm->where);
    text[4]=toupper(text[4]);
    write_user(user,text);
   }
 }
write_user(user,"\n");
ap=user->ap;
if (ap>0) {
  sprintf(text,"~FTMas ~OL~FW%d~RS~FT arena %s.\n",ap,skloncislo(ap,"bod","body","bodov"));
  write_user(user,text);
 }
}

exits=0;  /* null[0]='\0'; */
if (rm==user->room) strcpy(text,langselect(user,"~FTMozes ist:","~FTExits are:"));
else strcpy(text,langselect(user,"~FTDa sa ist:","~FTExits are:"));
for(i=0;i<MAX_LINKS;++i) {
      if (rm->link[i]==NULL) break;
   /*   if (strcmp(rm->link[i],"no")==NULL) break; LOKYDZR*/ 
      if (rm->link[i]->access & PRIVATE)
            sprintf(temp," ~FR%s ",langselect(user,rm->link[i]->name,rm->link[i]->name_en));
      else sprintf(temp," ~FG%s ",langselect(user,rm->link[i]->name,rm->link[i]->name_en));
      if (rm->link[i]->access==GOD_PRIVATE) /* ZMENA - pridane */
            sprintf(temp," ~OL~FY%s~RS ",langselect(user,rm->link[i]->name,rm->link[i]->name_en));
      strcat(text,temp);
      ++exits;
      }
if (!exits) strcpy(text,langselect(user,"~FTOdtialto sa nedostanes!","~FTThere are no exits."));
strcat(text,"\n");
write_user(user,text);


/*ZOBRAZENIE PREDMETOV V ROOME***********************************************/
  vel=16; /* 10 */
  y=0;
  strcpy(text,"");
  for (i=0;i<MPVM;++i) {
    if (rm->predmet[i]>-1) {
      if (y==0) strcat(text,langselect(user,"~FTNachadza sa tu:~FW ","~FTThere is:~FW "));
      vel+=strlen(predmet[rm->predmet[i]]->name)+2;
      if (predmet[rm->predmet[i]]->function==24 && rm->dur[i]%10000==user->socket) {
        vel++;
        myp=1;
       }
      else myp=0;
      if (y>0) strcat(text,", ");
      if (vel>78) {
        strcat(text,"\n");
	write_user(user,text);
	strcpy(text,"                ");
	vel=strlen(text)+strlen(predmet[rm->predmet[i]]->name);
       }
      if (predmet[rm->predmet[i]]->type==3 || predmet[rm->predmet[i]]->type==4) strcat(text,"~FW");
      else strcat(text,farba_p(rm->predmet[i]));
      if (myp) strcat(text,"*");
      strcat(text,predmet[rm->predmet[i]]->name);
      y++;
     }
   }
  if (y>0) {
   strcat(text,".\n");
   write_user(user,text);
  }
  
if (!strcmp(word[1],"dbg") && user->level>=GOD)
 {
  for (i=0;i<MPVM;++i) {
    if (i%9==8) sprintf(text,"%2d/%-5d\n",rm->predmet[i],rm->dur[i]);
    else sprintf(text,"%2d/%-5d ",rm->predmet[i],rm->dur[i]);
    write_user(user,text); }
  write_user(user,"\n");
  for (i=0;i<BODY;++i) {
    sprintf(text,"%2d/%-5d ",user->predmet[i],user->dur[i]);
    write_user(user,text); }
  sprintf(text,"carry: %d\n",user->carry);
  write_user(user,text);
  if (!strcmp(user->room->name,"cintorin") || !strcmp(user->room->name,"krater")) {
    sprintf(text,"Portalis_gate: %d   Brutalis_gate: %d\n",portalis_gate,brutalis_gate);
    write_user(user,text);
   }
/*  sprintf(text,"AF: %d  AFT: %d  AFP: %d\n",user->affected,user->affecttime,user->affpermanent); */

 }
/********************************************************/


strcpy(text,langselect(user,"~FTPristup je ","~FTAccess is "));
switch(rm->access) {      
      case PUBLIC:  strcat(text,langselect(user,"~OL~FGVEREJNY~RS~FT","set to ~OL~FGPUBLIC~RS~FT"));  break;
      case PRIVATE: strcat(text,langselect(user,"~OL~FRPRIVATNY~RS~FT","set to ~OL~FRPRIVATE~RS~FT"));  break;
      case FIXED_PUBLIC:  strcat(text,langselect(user,"~OL~FGVEREJNY ~RS~FG(fixne)~FT","fixed to ~OL~FGPUBLIC~RS~FT"));  break;
      case FIXED_PRIVATE: strcat(text,langselect(user,"~OL~FRPRIVATNY ~RS~FR(fixne)~FT","fixed to ~OL~FRPRIVATE~RS~FT"));  break;
      case GOD_PRIVATE: strcat(text,langselect(user,"~OL~FYBOZSKY~RS~FT","set to ~OL~FYGODLIKE~RS~FT")); break;
      }
if (!strcmp(rm->name,"cintorin")) sprintf(temp,"%s",langselect(user," a nachadza sa tu tabula s umrtnymi oznameniami."," and there is a funeral noticeboard."));
else switch(rm->mesg_cnt) /* ZMENA */
{
  case 0:
    sprintf(temp,"%s",langselect(user," a na nastenke nie je ziadna sprava."," and there are no messages on the board."));
    break;
  case 1:
    sprintf(temp,"%s",langselect(user," a na nastenke je ~OL~FW1~RS~FT sprava."," and there is ~OL~FW1~RS~FT message on the board."));
    break;
  case 2:
  case 3:
  case 4:
    if (sk(user)) sprintf(temp," a na nastenke su ~OL~FW%d~RS~FT spravy.",rm->mesg_cnt);
    else  sprintf(temp," and there are ~OL~FW%d~RS~FT messages on the board.",rm->mesg_cnt);
    break;
  default:
    if (sk(user)) sprintf(temp," a na nastenke je ~OL~FW%d~RS~FT sprav.",rm->mesg_cnt);
    else  sprintf(temp," and there are ~OL~FW%d~RS~FT messages on the board.",rm->mesg_cnt);
    break;
}

strcat(text,temp);

if (rm->sndproof) {
	strcat(text,langselect(user,"\n~OL~FMMiestnost je ZVUKOTESNA!\n","\n~OL~FMThis room is SOUNDPROOF!\n"));
	}
	else strcat(text,"\n");
	
write_user(user,text);
	
if (rm->topic[0]) {
      sprintf(text,"~FT%s ~OL~FW%s\n",langselect(user,"Akt. TEMA:","Current topic:"),rm->topic);
      write_user(user,text);
      return;
      }
/* write_user(user,"~FTAkt. TEMA: ~FW<zatial nebola nastavena>\n"); */

}

/*** Switch between command and speech mode ***/
void toggle_mode(user)
UR_OBJECT user;
{
if (user->command_mode) {
      sprintf(text,"Prep%s si naspat do standardneho rezimu.\n",pohl(user,"ol","la"));
      write_user(user,text);
      user->command_mode=0;  return;
      }
sprintf(text,"Prep%s si do COMMAND modu (prikazovy rezim).\n",pohl(user,"ol","la"));
write_user(user,text);
user->command_mode=1;
}
  
/*** Shutdown the talker ***/  
void talker_shutdown(user,str,reboot)
UR_OBJECT user;
char *str;
int reboot;
{
UR_OBJECT u;
int i;
char *ptr;
/* char *args[]={progname,confile,NULL}; */
char linepass[200];

if (user!=NULL) ptr=user->name; else ptr=str;

if (reboot) {
	write_room(NULL,"\07\n~OLSYSTEM:~FR~LI system sa prave RESTARTUJE!\n\n");
	sprintf(text,"*** RESTART inicializoval(a): %s ***\n",ptr);
   }
else {
	write_room(NULL,"\07\n~OLSYSTEM:~FR~LI system sa prave VYPINA!\n\n");
	sprintf(text,"*** SHUTDOWN inicializoval(a): %s ***\n",ptr);
   }
write_syslog(text,0);
i=0;
for(u=user_first;u!=NULL;u=u->next) {
	if (!u->login && u->type==USER_TYPE && u->room!=NULL) save_user_details(u,8);
	u->fds=0;
	close(u->socket);
        i++;
	}
sprintf(text,"%s blew out Atlantis with %d users!",ptr,i);
write_syslog(text,666);
for(i=0;i<2;++i) close(listen_sock[i]); /*LYNUZ*/

free_stuff();
shutdown_database();

if (reboot) sprintf(text,"*** RESTART ukonceny %s, max. pocet %d uzivatelov online. ***\n\n",long_date(1),max_users_was);
else sprintf(text,"*** SHUTDOWN ukonceny %s, max. pocet %d uzivatelov online. ***\n\n",long_date(1),max_users_was);
write_syslog(text,0);

sprintf(linepass,"%s%s", (strncmp(progname,EXECPREFIX,2) ? EXECPREFIX : ""),progname);
if (reboot) switch(fork()) {
      case -1: _exit(1);  /* fork failure */
      case  0: sleep(1); system(linepass); break; /* child */
      default: _exit(0);  /* parent */
      }

exit(0);
}

void free_stuff()
{
int i;
RM_OBJECT rm,nxt;

 i=0;
 while (convert[i]!=NULL) {
   destruct_convert(convert[i]);
   convert[i]=NULL;
   i++;
  }
 i=0;
 while (predmet[i]!=NULL) {
   destruct_vec(predmet[i]);
   predmet[i]=NULL;
   i++;
  }
 rm=room_first;
 while (rm!=NULL) {
   nxt=rm->next;
   if (rm->desc_en!=NULL) free ((void *) rm->desc_en);
   if (rm->desc_sk!=NULL) free ((void *) rm->desc_sk);
   free(rm);
   rm=nxt;
  }
 free_macrolist(&(defaultmacrolist));
 
}

/*** Say user speech. ***/

void say(UR_OBJECT user,char *inpstr,int comm) 
{
char type[50],*name;
char histrecord[20];

if (logcommands) log_commands(user->name,"say",1);
 /* M@CRO sem presunieme to, co sme tam boli zobrali */ /* oko si ustelies.. */

do_funct_stuff(user,inpstr,comm);

if (user->room!=NULL && user->room->group==4 && user->hidden==1) {
  sprintf(text,"~FTPrezradil%s si z ukrytu.\n",pohl(user," si sa a vysiel","a si sa a vysla"));
  write_user(user,text);
  sprintf(text,"~FT%s vyliez%s z ukrytu.\n",user->name,pohl(user,"ol","la"));
  write_room_except(user->room,text,user);
  user->hidden=0;
 }

sprintf(histrecord,"%02d:%02d say", tmin,tsec);
record_history(user, histrecord);

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes rozpravat.\n");  return;
      }
if (user->doom && !user->doom_sayon)
      {
      write_user(user,"Say je pocas hrania DOOMu ~FRzaisteny~FW. Pouzi \".doom say\" pre odistenie.\n");
      return;
      }
if ((word_count<2) && (user->command_mode)) {
      write_user(user,"Co chces povedat?\n");  return;
      }

if (flyer.pozicia && !strcmp(flyer.pilot,user->name))
     {
      strcpy(text,inpstr);
      strtolower(text);
      if (strstr(text,"sustredim sa"))
        {
         if (flyer.mind < 10) flyer.mind++;
         write_user(user,"~FGTvoje myslienky smeruju k pohybu.......\n");
         return;
        }
      else if (!strcmp(word[0],"vykon"))
        {
	 flyer.vykon=atoi(word[1]);
	 if (flyer.vykon > 9) flyer.vykon=9;
	 if (flyer.vykon < 0) flyer.vykon=0;
         sprintf(text,"~FG%s si paku ovladania do polohy ~OL%d~RS~FG!\n",pohl(user,"Presunul","Presunula"),flyer.vykon);
	 write_user(user,text);
	 return;
        }
     }

if (guest.on && !strcmp(user->room->name,GUEST_ROOM) && user!=guest.moderator
    && user!=guest.user && user!=guest.talk) {
    	write_user(user,"Nemas pridelene slovo - pockaj az ti ho moderator prideli!\n");
    	return;
    	}
/* SATURATE */
user->sayflood++;
if (user->level<SAG && user->saturate<20) {
  if (user->saturate<2) user->saturate+=2;
  else {
    user->saturate+=5;
    if (user->saturate>20) user->saturate=20;
   }	
 }

if (user->saturate>3 && user->type!=CLONE_TYPE) return;
if (user->type==CLONE_TYPE) {
    switch(inpstr[strlen(inpstr)-1]) {
         case '?': strcpy(type,"\253Q1");  break;
         case '!': strcpy(type,"\253E1");  break;
         default : strcpy(type,"\253P1");
         }
      sprintf(text,"Klon ~FT\252C2%s~RS~FW %s: %s\n",sklonuj(user,2),type,inpstr);
      write_room(user->room,text);
      record(user->room,text);
      return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo rozpravat %s!]~RS\n",user->name,pohl(user,"pokusil","pokusila"),user->room->where);
      writesys(KIN,1,text,user);
      return;
      }  
  if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }  

user->commused[2]++;
current_user=user;
com_num=SAY;

switch(inpstr[strlen(inpstr)-1]) {
     case '?': strcpy(type,pohl(user,"\253Q3","\253Q4"));  break;
     case '!': strcpy(type,pohl(user,"\253E3","\253E4"));  break;
     default : strcpy(type,pohl(user,"\253P3","\253P4"));
     }
      
sprintf(text,"~FT\252C2%s~RS~FW: %s\n",type,inpstr); 
write_user(user,text);

switch(inpstr[strlen(inpstr)-1]) {
     case '?': strcpy(type,pohl(user,"\253Q1","\253Q2"));  break;
     case '!': strcpy(type,pohl(user,"\253E1","\253E2"));  break;
     default : strcpy(type,pohl(user,"\253P1","\253P2"));
     }

if (user->vis) name=user->name; else name=invisname(user);
	if (user==guest.user && guest.on) sprintf(text,"~FT\252C2%s %s~RS~FW: %s\n", guest.name, type, inpstr);
		else if (user==guest.moderator && guest.on) sprintf(text,"~FT%s %s~RS~FW: %s\n",name,type,inpstr);
			else sprintf(text,"~FT\252C2%s %s~RS~FW: %s\n",name,type,inpstr);
		
write_room_except(user->room,text,user); 
record(user->room,text); 
if (user!=guest.user) user->flood++;
if (user->health<10) user->health=1;
} 
 
/*** ZMENA - .think = Buko thinks: . o O (This is a nice command) ***/
void think(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;

if (user->muzzled) {
      write_user(user,"Si umlcany - nemozes ani mysliet... ;)\n");  return;
      }
if (word_count<2) {
      write_user(user,"O com chces premyslat?\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo mysliet!]~RS\n",user->name,pohl(user,"pokusil", "pokusila"));
      writesys(KIN,1,text,user);
      return;
      }
  if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }
user->commused[2]++;
sprintf(text,"~FT\252C2\253YT~RS~FW: . o O (%s)\n",inpstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FT\252C2%s \253TH~RS~FW: . o O (%s)\n",name,inpstr);
write_room_except(user->room,text,user);
record(user->room,text);
}

/* sing - sings a message in format: "Xy sings: *blah blah*"
   that's all... ;-) */
void sing(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;

if (user->muzzled) {
	write_user(user, "Si umlcany, nemozes si ani pospevovat!\n"); return;
	}

if (word_count<2) {
	write_user(user, "Co si chces zaspievat?\n"); return; 
	}
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo spievat!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
      writesys(KIN,1,text,user);
      return;
      }
  if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }

user->commused[2]++;
sprintf(text,"~FT\252C2%s~RS~FW: ~FY*~FW%s~FY*~FW\n",pohl(user,"\253Z3","\253Z4"),inpstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FT\252C2%s %s~RS~FW: ~FY*~FW%s~FY*~FW\n",name,pohl(user,"\253Z1","\253Z2"),inpstr);
write_room_except(user->room,text,user);
record(user->room,text);
hesh(user->room);
}

/* Zobrazi info o pp */

void power_points(user)
UR_OBJECT user;
{

if (word_count>1 && user->level>KIN) {
/*** Vypne/zapne uberanie pp ***/
strtoupper(word[1]);
if (!strcmp(word[1],"STATIC")) {
     if (pp_zapnute!=1) {
      write_user(user,"Mod uberania PPciek nastaveny na ~FGSTATIC~FW.\n");
      sprintf(text,"%s: prepnutie uberania PP na STATIC.\n",user->name);
      write_syslog(text,1);
      pp_zapnute=1;
      return;
      }
     else { write_user(user,"Mod uberania PPciek je uz nastaveny na ~FGSTATIC~FW\n"); return; }
      }
if (!strcmp(word[1],"DYNAMIC")) {
     if (pp_zapnute!=2) {
      write_user(user,"Mod uberania PPciek je nastaveny na ~FGDYNAMIC.\n");
      sprintf(text,"%s: prepnutie uberania PP na DYNAMIC.\n",user->name);
      write_syslog(text,1);
      pp_zapnute=2;
      return;
      }
     else { write_user(user,"Mod uberania PPciek je uz nastaveny na ~FGDYNAMIC~FW\n"); return; }
      }
if (!strcmp(word[1],"OFF")) {
     if (pp_zapnute) {
      write_user(user,"Uberania PPciek ~FRVYPNUTE.\n");
      sprintf(text,"%s switched PP decreasing OFF.\n",user->name);
      write_syslog(text,1);
      pp_zapnute=0;
      return;
      }
     else { write_user(user,"Uberania PPciek uz je ~FGVYPNUTE!\n"); return; }
      }
write_user(user,"Parametre mozu byt STATIC, DYNAMIC alebo OFF.\n");
return;
}

write_user(user,"~OL~FWPrikaz     Staticke  Minimum  Dynamicke~RS~FW\n");
write_user(user,"~OL~FB---------------------------------------~RS~FW\n\n");
sprintf   (text,"~OL~FY.shout      ~FR%3d       ~FG%3d       ~FT%3d\n",SHOUTDEC, SHOUTCAN, (SHOUTDYN * num_of_users /100));
write_user(user,text);
sprintf   (text,"~OL~FY.semote     ~FR%3d       ~FG%3d       ~FT%3d\n",SEMOTEDEC, SEMOTECAN, (SEMOTEDYN * num_of_users /100));
write_user(user,text);
sprintf   (text,"~OL~FY.secho      ~FR%3d       ~FG%3d       ~FT%3d\n",SECHODEC, SECHOCAN, (SECHODYN * num_of_users /100));
write_user(user,text);
sprintf   (text,"~OL~FY.sbanner    ~FR%3d       ~FG%3d       ~FT%3d\n",SBANNERDEC, SBANNERCAN, (SBANNERDYN * num_of_users /100));
write_user(user,text);
sprintf   (text,"~OL~FY.banner     ~FR%3d       ~FG%3d       ~FT%3d\n",BANNERDEC, BANNERCAN, BANNERDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.hug        ~FR%3d       ~FG%3d       ~FT%3d\n",HUGDEC, HUGCAN, HUGDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.wake       ~FR%3d       ~FG%3d       ~FT%3d\n",WAKEDEC, WAKECAN, WAKEDEC);
write_user(user,text);
if (!strcmp(user->room->name,"arena"))
 sprintf   (text,"~OL~FY.fight      ~FR%3d       ~FG%3d       ~FT%3d\n",FIGHTDECARENA, FIGHTCANARENA, FIGHTDECARENA);
else 
 sprintf   (text,"~OL~FY.fight      ~FR%3d       ~FG%3d       ~FT%3d\n",FIGHTDEC, FIGHTCAN, FIGHTDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.kick       ~FR%3d       ~FG%3d       ~FT%3d\n",KICKDEC, KICKCAN, KICKDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.kiss       ~FR%3d       ~FG%3d       ~FT%3d\n",KISSDEC, KISSCAN, KISSDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.kidnap     ~FR%3d       ~FG%3d       ~FT%3d\n",KIDDEC, KIDCAN, KIDDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.throw      ~FR%3d       ~FG%3d     ~FT%3d-40\n",THROWDEC, THROWCAN, THROWDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.topic      ~FR%3d       ~FG%3d       ~FT%3d\n",TOPICDEC, TOPICCAN, TOPICDEC);
write_user(user,text);
sprintf   (text,"~OL~FY.priv/.pub  ~FR%3d       ~FG%3d       ~FT%3d\n",PRIVPUBDEC, PRIVPUBCAN, PRIVPUBDEC);
write_user(user,text);
sprintf(text,"\n~RS~FWMomentalny mod: ");
switch(pp_zapnute)
 {
 case 0: strcat(text,"~FRVYPNUTE~RS~FW\n");
         break;
 case 1: strcat(text,"~FGSTATICKY~RS~FW\n");
         break;
 case 2: strcat(text,"~FGDYNAMICKY~RS~FW\n");
         break;
 }
write_user(user,text);
sprintf(text,"V tejto chvili mas ~FT%d~FW power-pointov.\n\n",user->pp);
write_user(user,text);
}

/*** Shout something ***/
void shout(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;
int i,swears=0;

if (user->total_login<18000) {
      write_user(user,"Kricat mozes az ked budes mat 5 hodin celkoveho casu (TLT).\n");  return;
      }
if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes kricat.\n");  return;
      }
if (word_count<2) {
      write_user(user,"Co chces zakricat?\n");  return;
      }
if (user->level<MOZENADAVAT) swears=contains_swearing(inpstr,user);
  if ((swears) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo ziapat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(KIN,1,text,user);
      return;
      }
  if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }

if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"Namahat hrdlo v zalari vyzaduje maximum energie!\n");
	return;
	}
if (user->pp < SHOUTCAN) {    /*PP*/
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, SHOUTCAN);
	write_user(user,text); 
	return; 
	} 
user->commused[3]++; 
if (user->vis) name=user->name; else name=invisname(user);
if (!user->vis && user->room->group==1) {
  sprintf(text,"~OL~FW[Invis]~OL~FY\252C0 %s %s..\n",user->name,pohl(user,"\253S1","\253S2"));
  write_level(WIZ,1,text,NULL);
 }
sprintf(text,"~OL~FY\252C0%s:~RS~FW %s\n",pohl(user,"\253S3","\253S4"),inpstr);
write_user(user,text);
sprintf(text,"~OL~FY\252C0%s %s:~RS~FW %s\n",name,pohl(user,"\253S1","\253S2"),inpstr);
if (user->room->group!=1) { /* ()STROV */
  write_room_except(user->room,text,user); /* do roomy...      */
  /* sprintf(texthb,"%s%s",user->room->label,text); */
  record_portalisshout(user->room->label,text);
  for(i=0;i<MAX_LINKS;i++) {               /* a vsetkych okolo */
    if (user->room->link[i]!=NULL) {
      write_room_except(user->room->link[i],text,user);
     }
   }
  hesh(user->room);
  return;
 }
write_room_except(NULL,text,user);
/* ZMENA - tu recorduje shouty */
record_shout(text);

if (user->jailed) decrease_pp(user,MAXPP,0);
	else decrease_pp(user, SHOUTDEC, SHOUTDYN);	
if (user->level<KIN && swears>1) setpp(user,user->pp/swears);
/* HEHEHE, vydelime userove PPcka poctom nadavok v shoute (V) */ 
hesh(user->room);
}

/*** Gossip something - uncenzored Atlantis channel! 
 kecy taraniny zvasty dristy blaboly nadafky a mozno aj viac ;)
***/
void gossip(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nic kecat.\n");  return;
      }

if (user->igngossip) {
      write_user(user,"Nemozes kecat, pokial sam tie kecy ignorujes!\n"); return;
      }
      
if (word_count<2) {
      write_user(user,"O com chces kecat?\n");  return;
      }

if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"Kecanie spoza mrezi zalara vyzaduje maximum energie!\n");
	return;
	}
user->commused[4]++;
sprintf(text,"~OL~FR\252C1\253G3:~RS~FW %s\n",inpstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
wrtype=WR_NOCOL;
sprintf(text,"~OL~FR\252C1%s %s:~RS~FW %s\n",name, pohl(user,"\253G1","\253G2"), colour_com_strip2(inpstr,0));
write_room_except(NULL,text,user);
wrtype=WR_WITHCOL;
sprintf(text,"~OL~FR\252C1%s %s:~RS~FW %s\n",name, pohl(user,"\253G1","\253G2"), inpstr);
write_room_except(NULL,text,user);
wrtype=0;
record_gossip(text);
}


/*** Do a gossip emote ***/
void gemote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nic kecat.\n");  return;
      }

if (user->igngossip) {
      write_user(user,"Nemozes kecat, pokial sam tie kecy ignorujes!\n"); return;
      }
      
if (word_count<2) {
      write_user(user,"O com chces kecat?\n");  return;
      }

if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"Kecanie spoza mrezi zalara vyzaduje maximum energie!\n");
	return;
	}

user->commused[4]++; 
if (user->vis) name=user->name; else name=invisname(user);

if (inpstr[0]=='@') sprintf(text,"~OL~FR\252C1%s~RS~FW%s\n",name,inpstr+1);
else sprintf(text,"~OL~FR\252C1%s~RS~FW %s\n",name,inpstr);
write_user(user,text);
wrtype=WR_NOCOL;
if (inpstr[0]=='@') sprintf(text,"~OL~FR\252C1%s~RS~FW%s\n",name, colour_com_strip2(inpstr+1,0));
else sprintf(text,"~OL~FR\252C1%s~RS~FW %s\n",name, colour_com_strip2(inpstr,0));
write_room_except(NULL,text,user);
wrtype=WR_WITHCOL;
if (inpstr[0]=='@') sprintf(text,"~OL~FR\252C1%s~RS~FW%s\n",name, inpstr+1);
else sprintf(text,"~OL~FR\252C1%s~RS~FW %s\n",name, inpstr);
write_room_except(NULL,text,user);
wrtype=0;
record_gossip(text);

}

/*** Quest (for glory:) ***/
void quest_command(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char *name;
int kolko;
char text2[200];
int questy_za_den;
int i;

questy_za_den=1;

if (word_count>1 && !strcmp(word[1],"top"))
  {
  if (word_count==3)
   {
    kolko=atoi(word[2]);
   }
  else kolko=10;

  if (kolko<1 || kolko>99)  {
			  sprintf(text,"Pouzi: .quest top [n] (pre [n] max. 99)\n");
			  write_user(user,text);
			  return;
			  }
  winners(user,DB_QUEST,kolko);
  return;
  }

 if (word_count>1 && !strcmp(word[1],"oaberbyamd")) { 
	quest.lastquest=0;   /* Safra, spakky, ty PODRAZAK! :>> */
	write_user(user,"Okay, bude mozne spravit dalsi quest! ;)\n");
	return;
	}

     if(quest.lastquest==questy_za_den) {
     	sprintf(text2,"~OL~FWPosledny ~RSquest uz dnes bol o ~OL~FW%-5s~RS~FW, leader ~OL~FW%s~RS~FW, ", quest.lq_time, quest.lq_leader);
     	if (strlen(quest.lq_winner)>1) sprintf(text,"vitaz: ~OL~FY%s~RS~FW\n", quest.lq_winner);
     		else strcpy(text,"bez vyhlasenia vitaza.\n");
	strcat(text2,text);	
     	}
     else { if (quest.lastquest<questy_za_den && quest.lastquest!=0) {
         	sprintf(text2,"Quest uz dnes bol o ~OL~FW%-5s~RS~FW, leader ~OL~FW%s~RS~FW, ", quest.lq_time, quest.lq_leader);
     	 	if (strlen(quest.lq_winner)>1) sprintf(text,"vitaz: ~OL~FY%s~RS~FW\n", quest.lq_winner);
     	 	 	else strcpy(text,"bez vyhlasenia vitaza.\n");
	 	strcat(text2,text);
	 	sprintf(text,"Dnes este mozno spravit %d quest\n",questy_za_den-quest.lastquest);
	 	strcat(text2,text);
	 	}
     	  else	      
          strcpy(text2,"Dnes quest este nebol.\n");
          }

if ((user->level>=QLEADER || user->quest>1) && user->quest!=4)
   {
   if (word_count<2)
     {
     write_user(user,"Quest options: ~OL.quest open [tema]         - otvori quest\n");
     write_user(user,"               ~OL.quest close <winner|none> - vyhlasi vitaza\n~RS");
     write_user(user,"               ~OL.quest ban <user>          - zakaze ucast\n~RS");
     if (user->level>=QLEADER) write_user(user,"               ~OL.quest grant <uzivatel>    - umozni uzivatelovi spravit quest\n~RS");
     write_user(user,"               .quest top [n]             - tabulka najlepsich\n");
     write_user(user,"               .quest who                 - kto je prihlaseny do questu\n");
     write_user(user,"               .quest r[eview] [pattern]  - review hlasok [s maskou]\n\n");
     if (!quest.queston) write_user(user,text2);
     return;
     }
   if (!strcmp(word[1],"grant") && user->level>=QLEADER) {
   	if (word_count<3) {
   		write_user(user,"Pouzi: .quest grant <uzivatel> (funguje ako prepinac)\n");
   		return;
   		}
   	if (!(u=get_user(word[2]))) {
   		write_user(user,notloggedon);
   		return;
   		}
   	if (u==user) {
   		write_user(user,"Udelovat grant na quest samemu sebe je dvadsiaty druhy priznak sialenstva!\n");
   		return;
   		}
   	if (u->quest==1) {
   		write_user(user,"Uzivatel je prave v queste, nemozes dat grant.\n");
   		return;
   		}
   	if (u->quest==3) {
   		write_user(user,"Uzivatelovi bol odobrany grant na quest.\n");
   		u->quest=0;
   		return;
   		}
   	if (quest.queston) {
   		write_user(user,"Quest uz prebieha!\n");
   		return;
   		}
   	u->quest=3;
   	sprintf(text,"%s ma odteraz moznost otvorit a viest quest.\n", u->name);
   	write_user(user,text);
   	sprintf(text,"~OL%s ti %s moznost otvorit a viest quest!\n", user->name, pohl(user,"dal","dala"));
   	write_user(u,text);
   	return;
   	}
   if (!strcmp(word[1],"open")) /* otvaranie questu */
     {
     if (quest.queston) /* bezi.. */
        {
         write_user(user,"~FRSORRY:~FW Quest prave prebieha!\n");
         return;
        }
     if (quest.lastquest==questy_za_den)      /* dnes uz bol */
        {
         write_user(user,text2);
         return;
        }
     sprintf(quest.questfilename,"%s/quest-%d.%d.%d-%02d%02d.txt",QUEST_DIR, tmday, tmonth+1, tyear, thour, tmin);
     
     if (word_count>2)
        {
         inpstr=remove_first(inpstr);
         sprintf(text,"~OL~FG%s %s quest na temu: ~FY%s!\n~OL~FGPripojit ku questu sa mozes prikazom '~FW.quest join~FG'...\n",user->name,pohl(user,"otvoril","otvorila"),inpstr);
         write_room_except(NULL,text,user);
         sprintf(text,"~OL~FW%s si quest na temu: ~FY%s!~RS\n",pohl(user,"Otvoril","Otvorila"),inpstr);
         write_user(user,text);
         sprintf(text,"~FG~OL*** Dnesny quest vedie ~FY%s~FG na temu: ~FY%s~FG ***\n",user->name,inpstr);
         record_quest(text);
         quest_logger(text);
         strcpy(quest.lq_leader, user->name);
        }
     else
        {
         sprintf(text,"~OL~FG%s %s quest! Pripojit ku sa mozes prikazom '~FW.quest join~FG'\n",user->name,pohl(user,"otvoril","otvorila"));
         write_room_except(NULL,text,user);
         sprintf(text,"~OL%s si quest!~RS\n",pohl(user,"Otvoril","Otvorila"));
         write_user(user,text);
         sprintf(text,"~FG~OL*** Dnesny quest vedie ~FY%s~FG ***\n",user->name);
         record_quest(text);
         quest_logger(text);         
         strcpy(quest.lq_leader, user->name);
        }
     sprintf(quest.lq_time,"%02d:%02d", thour, tmin);        
     quest.lq_winner[0]='\0';
     quest.queston=1;
     user->quest=2; /* questleader */
     for(i=0; i<REVIEW_LINES;++i) revquestbuff[i][0]='\0';
     revquestline=0;
     return;
     }
   if (!quest.queston)
     {
     write_user(user,"~FRPREPAC:~FW Momentalne neprebieha nijaky quest...\n");
     return;
     }

   if (!strcmp(word[1],"ban") && user->quest==2) { /* vykicknutie magora :) */
        if (word_count<3) {
          write_user(user,"Pouzi: .quest ban <uzivatel>\n");
          return;
         }
   	if (!(u=get_user(word[2]))) {
   		write_user(user,notloggedon);
   		return;
   		}
   	if (u==user) {
   		write_user(user,"Snaha vyhodit sameho seba z questu je 23. priznakom sialenstva!\n");
   		return;
   		}
   	if (u->quest==0) {
   		write_user(user,"Uzivatel nie je v queste, nemozes ho vyhodit!\n");
   		return;
   		}
   	u->quest=4;
   	sprintf(text,"~OL~FG%s %s z questu!\n", u->name,pohl(u,"bol vyhosteny","bola vyhostena"));
   	write_user(user,text);
        sprintf(text,"~OL~FG%s %s %s z questu!\n",user->name,pohl(user,"vyhostil","vyhostila"),sklonuj(u,4));
        write_room_except(NULL,text,user);
        quest_logger(text);
   	sprintf(text,"~OL~FG%s ti %s pokracovat v queste!\n", user->name, pohl(user,"zakazal","zakazala"));
   	write_user(u,text);
   	return;
   	}

   if (!strcmp(word[1],"close")) /* zatvaranie questu */
     {
     if (user->quest!=2 && !(user->level>=GOD && !strcmp(word[3],"force")))
        {
        write_user(user,"~FRPREPAC:~FW Ty nie si questleader!\n");
        return;
        }
     if (word_count<3)
        {
         write_user(user,"~FRPREPAC:~FW Nemozes uzavriet quest bez vyhlasenia vitaza!\n");
         return;
        }
     quest.lastquest+=1;   /* Sem, alebo LEN pri vyhlaseni vytaza?! */
     if (!strcmp(word[2],"none")) /* Odmietam niekoho vyhlasit za vitaza */
        {
         sprintf(text,"~OL~FG%s %s quest ~FYBEZ VYHLASENIA~FG vitaza!\n",user->name,pohl(user,"uzavrel","uzavrela"));
         write_room_except(NULL,text,user);
         quest_logger(text);        
         sprintf(text,"~OL%s si quest (bez vitaza) ....\n",pohl(user,"Uzavrel","Uzavrela"));
         write_user(user,text);
         quest.queston=0;         
         user->quest=0;
         return;
        }
     if ((u=get_user(word[2]))==NULL) 
        {
         write_user(user,"~FRPREPAC:~FW Taky uzivatel nie je prihlaseny!\n");
         return;
        }
     if (!u->quest || u->quest==4)
        {
         write_user(user,"~FRPREPAC:~FW Taky uzivatel nie je v queste!\n");
         return;
        }
     if (u==user)
        {
        write_user(user,"~OLAle no tak! To snad nemyslis vazne! :-))~RS\n");
        return;
        }
     sprintf(text,"~OL~FG%s %s quest a vitazom je ~FY%s~FG! Gratulujeme!\n",user->name,pohl(user,"uzavrel","uzavrela"),u->name);
     write_room_except(NULL,text,user);
     quest_logger(text);
     write_user(u,"~FG~OL~LIGratulujeme! ~RS~OL~FWVitazom dnesneho questu si prave TY!\n");
     sprintf(text,"~OL%s si quest (vitaz: %s) ....\n",pohl(user,"Uzavrel","Uzavrela"), u->name);
     write_user(user,text);
     add_point(u,DB_QUEST,1,0);
     quest.queston=0;
     user->quest=0;
     strcpy(quest.lq_winner,u->name);
     return;
     }
   }

if (!quest.queston)
   {
   write_user(user,"Momentalne neprebieha nijaky quest...\n\n");
   write_user(user, text2);
   return;
   }

if (word_count==2 && !strcmp(word[1],"who"))
  {   
   write_user(user,"~FG~OL+-+-+-+-+-+-+-+-+-+-+-+-+\n~OL~FG|~FY V queste su zapojeni: ~FG|\n~OL~FG+-+-+-+-+-+-+-+-+-+-+-+-+\n");
   for(u=user_first;u!=NULL;u=u->next)
      {
      if (u->login || u->type==CLONE_TYPE || u->room==NULL) continue;
      if (u->quest && u->quest!=3 && u->quest!=4)
            {
            sprintf(text,"~OL~FG|~FR ~FW%-15s       ~FG|\n",u->name);
            if (u->quest==2) text[10]='>';
            write_user(user,text);
            }
      }
   write_user(user,"~FG~OL+-+-+-+-+-+-+-+-+-+-+-+-+~RS~FW\n");
   return;
  }

if (user->muzzled) {
      write_user(user,"Si umlcany, ziadny quest nebude...\n");  return;
      }


if (user->quest==4) {
      write_user(user,"Nemas povolene zapajat sa do questu!\n");
      return;
      }

if (word_count==2 && !strcmp(word[1],"join")) 
   {
   if (user->quest && user->quest!=3 && user->quest!=4)
      {
       sprintf(text,"Co vymyslas, ved uz si %s do questu!\n",pohl(user,"zapojeny","zapojena"));
       write_user(user,text);
       return;
      }
   sprintf(text,"~OLOK: %s si sa ku questu!\n",pohl(user,"pripojil","pripojila"));
   write_user(user,text);
   user->quest=1;
   sprintf(text,"~FG~OL%s~RS~FW sa %s ku questu...\n",user->name,pohl(user,"pripojil","pripojila"));
   write_room_except(NULL,text,user);
   quest_logger(text);
   return;
   }
 
if (!user->quest || user->quest==3)
   {
   write_user(user,"Nie si v queste. Zapojis sa prikazom '~OL.quest join~RS'.\n");
   return;
   }

if (word_count==2 && !strcmp(word[1],"stop")) 
   {
    if (user->quest==2)
      {
      write_user(user,"~FRSORRY:~FW Nemozes sa odhlasit z questu ktory vedies!!!\n");
      return;
      }
    sprintf(text,"OK: %s si sa z questu!\n",pohl(user,"Odhlasil","Odhlasila"));
    write_user(user,text);
    user->quest=0;
    sprintf(text,"~FG~OL%s~RS~FW sa %s z questu...\n",user->name,pohl(user,"odhlasil","odhlasila"));
    write_room_except(NULL,text,user);
    quest_logger(text);
    return;
   }

if (word_count<2) {
      write_user(user,"Pouzi: .quest <text | join | stop | who | r[eview] [pattern] | top [n]>\n");  return;
      }

if (!strncmp(word[1],"review",strlen(word[1])))
   {
   inpstr=remove_first(inpstr);
   revquest(user,inpstr);
   return;
   }

if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing) && (user->quest!=2)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s hanit quest!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->quest!=2) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }

if (user->quest==2) sprintf(text,"~OL~FGQuestujes:~FW %s\n",inpstr);
	else sprintf(text,"~OL~FGQuestujes:~RS~FW %s\n",inpstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname2(user);
if (user->quest==2) sprintf(text,"~OL~FG%s questuje:~FW %s\n",name,inpstr);
	else sprintf(text,"~OL~FG%s questuje:~RS~FW %s\n",name,inpstr);
write_room_except(NULL,text,user);
quest_logger(text);
record_quest(text);
}

void quest_logger(char *retazec)      /* better than nuke ... :> */
{
FILE *fp;

if ((fp=ropen(quest.questfilename,"a"))==NULL) return; /*APPROVED*/
fprintf(fp,"%s", retazec);
fclose(fp);
return;
}

/* uberac power pointov */
void decrease_pp(user, value, dynamic)
UR_OBJECT user;
int value;
int dynamic;
{

/* Vynimky, mozno doplnit aj konkretne mena, napr. niektoryx wizof */
if (user->level >= KIN) return;
/* vypinac a zapinac: */
if (!pp_zapnute) { user->pp=200; return; }

if (dynamic && pp_zapnute==2)
 {
  value=num_of_users*dynamic/100;
 }

if (user->pp-value>=0) user->pp-=value;
	else user->pp=0;
	
if (user->statline==CHARMODE) show_statline(user);

}

/*Spakky - LOGTIME. Prikaz prida/uberie login-time (v hodinach danemu userovi)*/

void logtime(user)
UR_OBJECT user;
{
UR_OBJECT u;
int hodiny;
FILE *fp;

if ((word_count<4) || ((strcmp(word[2],"+")) && (strcmp(word[2],"-")) && (strcmp(word[2],"vrat")))
 || (!(hodiny=atoi(word[3])))) {
        if (user->level>=GOD) write_user(user,"Pouzi: .logtime <uzivatel> <+|-|vrat> <hodiny>\n");
	else write_user(user,"Pouzi: .logtime <uzivatel> <+|-> <hodiny>\n");
	return;
	}
if (!(u=get_user(word[1]))) {
	write_user(user, notloggedon);
	return;
	}
if (!strcmp(word[2],"vrat")) {
        if (hodiny>(u->nontimejails*ZALAR_TLT_CONSUMPTION)/3600) {
  	  sprintf(text,"~OL~FR%s zalar tolko hodin nezhltol!\n~OL~FRHodiny zhltnute necasovanym zalarom: ~FW%d\n",sklonuj(u, 3),(u->nontimejails*ZALAR_TLT_CONSUMPTION)/3600);
	  write_user(user,text);
	  return;
	 }
	u->total_login=u->total_login+(hodiny*3600);
	u->nontimejails-=(hodiny*3600)/ZALAR_TLT_CONSUMPTION;
	sprintf(text,"~OL~FGVratil si %s %d hodin TLT z casu zhltnuteho zalarom!\n", sklonuj(u, 3), hodiny);
	write_user(user,text);
	return;
	}

if (!strcmp(word[2],"+")) {
	u->total_login=u->total_login+(hodiny*3600);
	sprintf(text,"~OL~FGZvysil si TLT %s o %d hodin!\n", sklonuj(u, 2), hodiny);
	write_user(user,text);
	}
	 
if (!strcmp(word[2],"-")) {
	u->total_login=u->total_login-(hodiny*3600);
	sprintf(text,"~OL~FGZnizil si TLT %s o %d hodin!\n", sklonuj(u, 2), hodiny);
	write_user(user,text);
	}
	
if ((fp=ropen(TLT_LOG,"a"))!=NULL) { /*APPROVED*/
	if (!strcmp(word[2],"+")) fprintf(fp,"%-12s: + %-3dh -> %s\n", user->name, hodiny, u->name);
	if (!strcmp(word[2],"-")) fprintf(fp,"%-12s: - %-3dh -> %s\n", user->name, hodiny, u->name);
	fclose(fp);
	}
}

char *check_shortcut(UR_OBJECT user)
{
static char ret[520];
char name[21];
UR_OBJECT u;
unsigned int namelen,cnt=0,len;

strncpy(name,word[1],20);
name[20]='\0';
name[0]=toupper(name[0]);
if (!strncasecmp(user->lasttellto,name,strlen(name)) && strlen(name)<strlen(user->lasttellto)
&& (!(u=get_user_exact(word[1])))) {
  strcpy(word[1],user->lasttellto);
  ret[0]='\0';
  return ret;
 }
strcpy(ret,"Pozor! Zadane meno je skratkou viacerych uzivatelov: ");
len=strlen(ret);
namelen=strlen(name);
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login || u->type!=USER_TYPE || u==user) continue;
  if (!strncasecmp(u->name,name,namelen) && strlen(u->name)>=namelen)  {
    if (strlen(u->name)==namelen) {
      ret[0]='\0';
      return ret;
     }
    if (len+3+strlen(u->name)>79) {
      strcat(ret,",\n");
      strcat(ret,u->name);
      len=strlen(u->name);
     }
    else {
      if (cnt>0) strcat(ret,", ");
      else len-=2;
      strcat(ret,u->name);
      len+=strlen(u->name)+2;
     }
    cnt++;
   }
  if (strlen(ret)>500) break;
 }
if (cnt<2) ret[0]='\0';
else strcat(ret,"\n");
return ret;
}

/*** Tell another user something ***/
/* Zmena spakky - ak je miesto usera bodka, tellne sa to tomu kto mi 
   naposledy telloval. Akesi 'reply' Tiez spolupracuje s .reply commandom */

void tell(user,inpstr, reply) /*TELLL*/
UR_OBJECT user;
char *inpstr;
int reply;
{
UR_OBJECT u;
char type[20],*name,uname[USER_NAME_LEN+10],uname3[USER_NAME_LEN+10];
int notiuser,notivictim;

if (user->muzzled) {
  if (user->lang) write_user(user,"You are muzzled.\n");
  else write_user(user,"Si umlcany.\n");
  return;
 }

if (user->ignall) {
 sprintf(text,"Momentalne vsetkych ignorujes..\n");
 write_user(user,text);
 return;
}

if (((reply==1) && word_count<2) || (word[1][0]=='>')) {
  if (!user->lasttellfrom[0]) sprintf(text,"Zatial ti nikto nic nepovedal.\n");
  else {
   if (user->lasttg==-1) sprintf(text,"Posledne ti hovoril(a): ~OL%s~RS\n",user->lasttellfrom);
   else if (!user->lasttg) sprintf(text,"Posledne ti hovorila: ~OL%s~RS\n","\253Iz");
   else sprintf(text,"Posledne ti hovoril: ~OL%s~RS\n","\253Im");
  }
  write_user(user, text);
  return;
 }
if ((reply==3) && (word_count<2 || !user->lasttellto[0])) {
  if (!user->lasttellto[0])
   sprintf(text,"Zatial si nikomu nic nehovoril%s.\n",pohl(user,"","a"));
  else {
    sprintf(text,"Posledny komu si nieco hovoril%s bol(a): ~OL%s~RS\n",pohl(user,"","a"),user->lasttellto);
   }
  write_user(user, text);
  return;
 }
if (reply==2 || !strcmp(word[1],",")) {
  if (word_count<2) {
    if (!user->call[0]) {
      write_user(user,"Este nemas nastaveny quick-call. (pouzi .call <uzivatel> pre nastavenie)\n");
      return;
     }
    sprintf(text,"Quick-call mas nastaveny na uzivatela: %s\n",user->call);
    write_user(user,text);
    return;
   }
  if (!user->call[0]) {
    write_user(user,"Este nemas nastaveny quick-call! (pouzi: .call <uzivatel> pre nastavenie)\n");
    return;
   }
  strcpy(word[1],user->call);
 }
if ((!reply && word_count<3) || (reply && word_count<2)) {
  write_user(user,"\253T?\n");
  return;
 }
if ((!strcmp(word[1],":")) || (reply==1)) {
  if (!user->lasttellfrom[0]) {
    write_user(user,"Este ti nikto nic nepovedal, nemozes pouzit reply.\n");
    return;
   }
  if(!(u=get_user(user->lasttellfrom))) {
    write_user(user, notloggedon);
    return;
   }
 }
else if (reply==3 || (user->lasttellto[0] && !strcmp(word[1],"."))) {
  if (!(u=get_user(user->lasttellto))) {
    write_user(user,notloggedon);
    return;
   }
 }
else {
  if (user->shortcutwarning==1) {
    strcpy(text,check_shortcut(user));
    if (text[0]) {
      write_user(user,text);
      return;
     }
   } 
  if (!(u=get_user(word[1]))) {
    write_user(user,notloggedon);
    return;
   }
 }
strcpy(uname,u->name);
strcpy(uname3,sklonuj(u,3)); 
if (reply==1) {
 if (user->lasttg==0) { strcpy(uname,"\253Iz"); strcpy(uname3,"\253UN"); }
 if (user->lasttg==1) { strcpy(uname,"\253Im"); strcpy(uname3,"\253Un"); }
}
if (u==user) {
  write_user(user,"Samomluva je prvy priznak sialenstva.\n");
  return;
 }
if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
  if (user->room->sndproof) {
   sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s ta nemoze pocut.\n",uname);
   write_user(user,text);
   return;
  }
  if (u->room->sndproof) {
   sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze ta pocut.\n",uname);
   write_user(user,text);
   return;
  }
 }
if ((user->jailed) && (user->pp<MAXPP)) {
 write_user(user,"V zalari aj rozpravanie len z plneho hrdla (s plnymi pp-ckami)!\n");
 return;
}

if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
 sprintf(text,"%s ta ignoruje.\n",uname);
 write_user(user,text);
 return;
}
if (check_ignore_user(u,user)) {  /* ignorovanie single usera */
 sprintf(text,"Ved %s ignorujes.\n",sklonuj(u,4));
 write_user(user,text);
 return;
}

if (((user->ignportal) || (u->ignportal)) && user->room->group!=u->room->group) {
 if (user->level<WIZ) {
  if (user->ignportal) write_user(user,"Prepac, mas nastavene ignorovanie tellov cez ostrovy.\n");
  else if (u->ignportal) {
   sprintf(text,"Prepac, %s sa nachadza na inom ostrove a ignoruje telly medzi ostrovmi.\n", uname);
   write_user(user,text);
  }
  return;
 }
}

if (u->igntell) {
  if (user->level<WIZ || u->level>=user->level) {
    sprintf(text,"%s ignoruje telly.\n",uname);
    write_user(user,text);
    return;
   }
  else if (user->level>WIZ || u->level<user->level) {
    sprintf(text,"%s by Ta normalne %s ignorovat.\n",uname,pohl(u,"mal","mala"));
    write_user(user,text);
   }
 }

user->commused[1]++;
if (check_notify_user(user,u)) notiuser=1; else notiuser=0;
if (check_notify_user(u,user)) notivictim=1; else notivictim=0;

if (u->afk) {
  if (u->afk_mesg[0]) {
    if (!u->ignafktell && !strcmp(u->afk_mesg,"auto-afk"))
     sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\nSprava bola zobrazena a pridana do %s tell-bufferu.\n",uname,u->afk_mesg,pohl(u,"jeho","jej"));
    else
     sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\nSprava bola pridana do %s tell-bufferu.\n",uname,u->afk_mesg,pohl(u,"jeho","jej"));
   }
  else
   sprintf(text,"%s je prave mimo klavesnice. Sprava bola pridana do %s tell-bufferu.\n",uname,pohl(u,"jeho","jej"));
  write_user(user,text);
  if (!reply) inpstr=remove_first(inpstr);  /* je afk -> record do buffera */
  strcpy(type,pohl(user,"\253Tt","\253TT"));
  if (user->lang) sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname,inpstr);
             else sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname3,inpstr);
  record_tell(user,text);
  if (!u->newtell) {
    if (u->lang) sprintf(text,"~OL\253AK %02d:%02d\n",thour,tmin);
    else sprintf(text,"~OL~FWZatial co si od %02d:%02d %s AFK, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"bol","bola"));
    user->tellpos=user->revline;
    record_tell(u,text);
   }
  if (u->igncoltell) colour_com_strip(inpstr);
  strcpy(type,pohl(user,"\253Ty","\253TY"));
  if (user->vis) name=user->name; else name=invisname(user);
  if (user->vis || u->level<WIZ)
   sprintf(text,"~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,name,type,inpstr);
  else                         /* V) no sklonovanie .. no problem */
   sprintf(text,"~OL~FW\252C4[Invis] ~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,user->name,type,inpstr);
  strcpy(u->lasttellfrom,user->name);
  if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
  if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name);
  record_tell(u,text);
  if (!u->ignafktell && !strcmp(u->afk_mesg,"auto-afk")) write_user(u,text);
  u->newtell++;
  return;
 }
if (u->ignall) {
  if (u->malloc_start!=NULL) {
    if (!reply) inpstr=remove_first(inpstr);
    strcpy(type,pohl(user,"\253Tt","\253TT"));
    if (user->lang)  
     sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname,inpstr);
    else
     sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname3,inpstr);
    record_tell(user,text);
    if (!u->newtell) {
      if (u->lang) sprintf(text,"~OL\253ED %02d:%02d~RS\n",thour,tmin);
      else sprintf(text,"~OLZatial co si od %02d:%02d nieco %s, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"pisal","pisala"));
      user->tellpos=user->revline;
      record_tell(u,text);
     }
    if (u->igncoltell) colour_com_strip(inpstr);
    strcpy(type,pohl(user,"\253Ty","\253TY"));
    if (user->vis) name=user->name; else name=invisname(user);
    if (user->vis || u->level<WIZ)
     sprintf(text,"~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,name,type,inpstr);
    else
     sprintf(text,"~OL~FW\252C4[Invis] ~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,user->name,type,inpstr);
    record_tell(u,text);
    strcpy(u->lasttellfrom,user->name);
    if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
    if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name);
    u->newtell++;
    sprintf(text,"%s prave pise v editore. Odkaz ulozeny do %s tell-bufferu...\n",uname,pohl(u,"jeho","jej"));
   }
  else if (u->filepos) /* Ak cosi cita, ide to do revtell bufferu */
   {
    if (!reply) inpstr=remove_first(inpstr);
    strcpy(type,pohl(user,"\253Tt","\253TT"));
    if (user->lang) sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname,inpstr);
               else sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname3,inpstr);
    record_tell(user,text);
    if (!u->newtell)
     {
      if (u->lang) sprintf(text,"~OL\253RE %02d:%02d~RS\n",thour,tmin);
      else
       sprintf(text,"~OLZatial co si od %02d:%02d nieco %s, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"cital","citala"));
      user->tellpos=user->revline;
      record_tell(u,text);
     }
    if (u->igncoltell) colour_com_strip(inpstr);
    strcpy(type,pohl(user,"\253Ty","\253TY"));
    if (user->vis) name=user->name; else name=invisname(user);
    if (user->vis || u->level<WIZ)
     sprintf(text,"~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,name,type,inpstr);
    else
     sprintf(text,"~OL~FW\252C4[Invis] ~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,user->name,type,inpstr);
    record_tell(u,text);
    strcpy(u->lasttellfrom,user->name);
    if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
    if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name);
    u->newtell++;
    sprintf(text,"%s prave cita nejaky text. Odkaz ulozeny do %s tell-bufferu...\n",uname,pohl(u,"jeho","jej"));
   }
  else
   sprintf(text,"%s prave ignoruje vsetky hlasky.\n",uname);
  write_user(user,text);
  return;
 }

if (!reply) inpstr=remove_first(inpstr);

strcpy(type,pohl(user,"\253Tt","\253TT"));
if (user->lang) sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname,inpstr);
           else sprintf(text,"~OL~FW\252C4%s ~OL~FW\252C%d%s:~RS~FW %s\n",type,notiuser?6:3,uname3,inpstr);
write_user(user,text);
record_tell(user,text);
/*if (u->room->group!=user->room->group) decrease_pp(user,TELDEC,NODYNAMIC);*/
if (user->vis) name=user->name; else name=invisname(user);

if (u->igncoltell) colour_com_strip(inpstr);
strcpy(type,pohl(user,"\253Ty","\253TY"));
if (user->vis || u->level<WIZ) 
 sprintf(text,"~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,name,type,inpstr);
else
 sprintf(text,"~OL~FW\252C4[Invis] ~OL~FW\252C%d%s ~OL~FW\252C4%s:~RS~FW %s\n",notivictim?6:3,user->name,type,inpstr);
write_user(u,text);
record_tell(u,text);
strcpy(u->lasttellfrom,user->name);
if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name);
}

/*** tellall ZMENA Spakky - tellne VSETKYM dany text... ***/
void tellall(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char type[10],*name;

if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo tellallovat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(KIN,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nikomu nic hovorit.\n");
      return;
      }

if (user==user_first && user->next==NULL) {
	write_user(user,"Si tu sam! Nemozes nikomu nic povedat!\n");
	return;
	}

if (word_count<2) {
      write_user(user,"Co chces vsetkym povedat?\n");  return;
      }

/*if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,pohl(user,"povedal","povedala"));*/
/* else */ strcpy(type,pohl(user,"\253Ty","\253TY"));
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~OL~FW\252C3%s si vsetkym ludom:~RS~FW %s\n",pohl(user,"Povedal","Povedala"),inpstr);
write_user(user,text);
sprintf(text,"~OL~FW\252C3%s ~OL~FW\252C4%s:~RS~FW %s\n",name,type,inpstr);

for(u=user_first;u!=NULL;u=u->next) {
	if (u->login || u->room==NULL || u->afk || u->ignall || u->malloc_start ||
	    u->filepos || u->ignfun || u->room->group!=user->room->group ||
	    u->igntell || check_ignore_user(user,u) || u==user
	    || u->type!=USER_TYPE || u->dead) continue;
	write_user(u,text);
	record_tell(u,text);
	}
}


/*** executes a command ***/
void exec_command(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT luser;

if (word_count<3) {
      write_user(user,"Pouzi: exec <uzivatel> <.prikaz> [text]\n");  return;
      }
if (!(luser=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (luser->level>=user->level || luser==user) {
      write_user(user,"Prepac, na tohoto uzivatela to nepojde.\n");
      return;
      }
inpstr=remove_first(inpstr);
sprintf(text,"~FBVykonavas prikaz za %s: ~FG%s~FW\n",sklonuj(luser,4),inpstr);
write_user(user,text);
/* sprintf(text,"~FRNieco silnejsie ako ty ta donutilo pouzit prikaz: %s~RS\n",word[2]);
write_user(luser,text); */   /* Zmena Spakky - Radsej ked o nicom nevie ;) */
word_count=wordfind(inpstr);
exec_com(luser,inpstr);
police_freeze(user,1);
}

/*** To. pouzitie: .to <uzivatel> <text>, output: Buko (to User): Ako sa mas? ***/
void to_user(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char *name;

if (user->muzzled) {
      sprintf(text,"Si %s, nemozes nikomu nic hovorit.\n",pohl(user,"umlcany","umlcana"));
      write_user(user,text);
      return;
      }
if (word_count<3) {
      write_user(user,"Pouzi: .to <uzivatel> <text>\n");  return;
      }
if (!(u=get_user_in_room(word[1],user))) {
      write_user(user,"Nikto taky nie je s tebou v miestnosti\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo rozpravat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }
if (u==user) {
      write_user(user,"Hovorit sam k sebe je strnasty priznak sialenstva.\n");
      return;
      }
if (strcmp(u->room->name,user->room->name)) {
      write_user(user,"Uzivatel je v inej miestnosti ako ty.\n");
      return;
      }
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }

user->commused[2]++;
inpstr=remove_first(inpstr);
sprintf(text,"~FT\252C2%s (~RS~FW%s~FT):~RS %s\n",pohl(user,"\253P3","\253P4"),sklonuj(u,3),inpstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
if (u->vis) sprintf(text,"~FT\252C2%s (~RS~FW%s~FT):~RS %s\n",name,sklonuj(u,3),inpstr);
       else sprintf(text,"~FT\252C2%s (~RS~FW%s~FT):~RS %s\n",name,pohl(u,"\253Un","\253UN"),inpstr);
write_room_except(user->room,text,user);
record(user->room,text);
}

/*** Emote something ***/
void emote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes emotovat.\n");  return;
      }
if (word_count<2 /*&& inpstr[1]<33*/) {
      write_user(user,"Co chces emotovat?\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo emotovat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }
if (strstr(inpstr,"sepkal") || strstr(inpstr,"povedal")) {
	write_user(user,"Toto nemozes emotovat.\n");
	return;
	}
user->commused[2]++;
if (user->vis) name=user->name; else name=invisname(user);
if (inpstr[0]==';') sprintf(text,"%s%s\n",name,inpstr+1);
else sprintf(text,"%s %s\n",name,inpstr);
write_room(user->room,text);
record(user->room,text);
}


/*** Do a shout emote ***/
void semote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;
int i;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes semotovat.\n");  return;
      }
if (word_count<2 /*&& inpstr[1]<33*/) {
      write_user(user,"Co chces zakricat?\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo rozpravat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      } 
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      } 

if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"Namahat hrdlo v zalari vyzaduje maximum energie!\n");
	return;
        }
    
if (user->pp < SEMOTECAN) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, SEMOTECAN); 
	write_user(user,text); 
	return; 
	} 

if (strstr(inpstr,"sepkal") || strstr(inpstr,"povedal")) {
	write_user(user,"Toto nemozes emotovat.\n");
	return;
	}

user->commused[3]++; 
if (user->vis) name=user->name; else name=invisname(user);
if (!user->vis && user->room->group==1) {
  sprintf(text,"~OL~FW[Invis]~OL~FY\252C0 %s %s..\n",user->name,pohl(user,"\253S1","\253S2"));
  write_level(WIZ,1,text,NULL);
 }
if (inpstr[0]=='#') sprintf(text,"~OL~FY\252C0%s~RS~FW%s\n",name,inpstr+1);
else sprintf(text,"~OL~FY\252C0%s~RS~FW %s\n",name,inpstr);
write_user(user,text);

if (user->room->group!=1) {
  write_room_except(user->room,text,user);     /* do roomy...      */
  /* sprintf(texthb,"%s%s",user->room->label,text); */
  record_portalisshout(user->room->label,text);
  for(i=0;i<MAX_LINKS;i++) {               /* a vsetkych okolo */
    if (user->room->link[i]!=NULL)
     write_room_except(user->room->link[i],text,user);
   }
  return;
 }
                                              
write_room_except(NULL,text,user);
record_shout(text); 
if (user->jailed) decrease_pp(user,MAXPP,NODYNAMIC);
	else decrease_pp(user, SEMOTEDEC, SEMOTEDYN);

}


/*** Do a private emote ***/
void pemote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;
int notiuser,notivictim;
UR_OBJECT u;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes s nikym komunikovat.\n");  return;
      }
if (word_count<3) {
      write_user(user,"Pouzi: .pemote <uzivatel> <text>\n");  return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Emotovanie samemu sebe je druhy priznak sialenstva.\n");
      return;
      }
/*if (u->afk) {
      if (u->afk_mesg[0])
            sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\n",u->name,u->afk_mesg);
      else sprintf(text,"%s je prave mimo klavesnice.\n",u->name);
      write_user(user,text);
      return;
      }
     
if (u->ignall) {
      if (u->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",u->name);
      else if (u->filepos) sprintf(text,"%s prave cita nejaky text.\n",u->name);
       else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
      write_user(user,text);  return;
      } 
*/      
if (u->igntell) {
  if (user->level<WIZ || u->level>=user->level) {
    sprintf(text,"%s ignoruje telly a pemote.\n",u->name);
    write_user(user,text);
    return;
   }
  else if (user->level>WIZ || u->level<user->level) {
    sprintf(text,"%s by ta normalne %s ignorovat.\n",u->name,pohl(u,"mal","mala"));
    write_user(user,text);
   }
 }

if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
if (check_ignore_user(u,user)) {  /* ignorovanie single usera */
      sprintf(text,"Ved %s ignorujes.\n",sklonuj(u,4));
      write_user(user,text);
      return;
    }

if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
	if (user->room->sndproof) {
		sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s ta nemoze pocut.\n",u->name);
		write_user(user,text);
		return;
		}
	if (u->room->sndproof) {
		sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze ta pocut.\n",u->name);
		write_user(user,text);
		return;
		}      
      }
      
if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"V zalari LEN z plneho hrdla (s plnymi pp-ckami ;)!\n");
	return;
	}

if (check_notify_user(user,u)) notiuser=1; else notiuser=0;
if (check_notify_user(u,user)) notivictim=1; else notivictim=0;

inpstr=remove_first(inpstr);
if (u->afk) {
  if (u->afk_mesg[0]) {
    if (!u->ignafktell && !strcmp(u->afk_mesg,"auto-afk"))
     sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\nSprava bola zobrazena a pridana do %s tell-bufferu.\n",u->name,u->afk_mesg,pohl(u,"jeho","jej"));
    else
     sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\nSprava bola pridana do %s tell-bufferu.\n",u->name,u->afk_mesg,pohl(u,"jeho","jej"));
   }
  else
   sprintf(text,"%s je prave mimo klavesnice. Sprava bola pridana do %s tell-bufferu.\n",u->name,pohl(u,"jeho","jej"));
  write_user(user,text);
  if (user->lang)  
   sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,u->name,user->name,inpstr);
  else
   sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,sklonuj(u,3),user->name,inpstr);
  record_tell(user,text);
  if (!u->newtell) {
    if (u->lang) sprintf(text,"~OL\253AK %02d:%02d\n",thour,tmin);
    else sprintf(text,"~OL~FWOd %02d:%02d %s AFK, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"si bol","si bola"));
    user->tellpos=user->revline;
    record_tell(u,text);
   }
  if (u->igncoltell) colour_com_strip(inpstr);
  if (user->vis) name=user->name; else name=invisname(user);
  if (user->vis || u->level<WIZ)
   sprintf(text,"~OL~FW\252C4>> ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
  else
   sprintf(text,"~OL~FW\252C4>> [Invis] ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
  if (u->igncoltell) colour_com_strip(inpstr);
  record_tell(u,text);
  if (!u->ignafktell && !strcmp(u->afk_mesg,"auto-afk")) write_user(u,text);
/*  strcpy(u->lasttellfrom,user->name);
  if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
  if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name); */
  u->newtell++;
  return;
 }
if (u->ignall) {
  if (u->malloc_start!=NULL) {
    if (user->lang)  
     sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,u->name,user->name,inpstr);
    else
     sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,sklonuj(u,3),user->name,inpstr);
    record_tell(user,text);
    if (!u->newtell) {
      if (u->lang) sprintf(text,"~OL\253ED %02d:%02d~RS\n",thour,tmin);
      else sprintf(text,"~OLZatial co si od %02d:%02d nieco %s, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"pisal","pisala"));
      user->tellpos=user->revline;
      record_tell(u,text);
     }
    if (u->igncoltell) colour_com_strip(inpstr);
    if (user->vis) name=user->name; else name=invisname(user);
    if (user->vis || u->level<WIZ)
     sprintf(text,"~OL~FW\252C4>> ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
    else
     sprintf(text,"~OL~FW\252C4>> [Invis] ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
    if (u->igncoltell) colour_com_strip(inpstr);
    record_tell(u,text);
/*    strcpy(u->lasttellfrom,user->name);
    if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
    if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name); */
    u->newtell++;
    sprintf(text,"%s prave pise v editore. Odkaz ulozeny do %s tell-bufferu...\n",u->name,pohl(u,"jeho","jej"));
   }
  else if (u->filepos) /* Ak cosi cita, ide to do revtell bufferu */
   {
/*    if (!reply) inpstr=remove_first(inpstr); */
    if (user->lang)  
     sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,u->name,user->name,inpstr);
    else
     sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,sklonuj(u,3),user->name,inpstr);
    record_tell(user,text);
    if (!u->newtell)
     {
      if (u->lang) sprintf(text,"~OL\253RE %02d:%02d~RS\n",thour,tmin);
      else
       sprintf(text,"~OLZatial co si od %02d:%02d nieco %s, ludia ti povedali toto:~RS\n",thour,tmin,pohl(u,"cital","citala"));
      user->tellpos=user->revline;
      record_tell(u,text);
     }
    if (u->igncoltell) colour_com_strip(inpstr);
    if (user->vis) name=user->name; else name=invisname(user);
    if (user->vis || u->level<WIZ)
     sprintf(text,"~OL~FW\252C4>> ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
    else
     sprintf(text,"~OL~FW\252C4>> [Invis] ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
    record_tell(u,text);
/*    strcpy(u->lasttellfrom,user->name);
    if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
    if (!(reply==1 && user->lasttg>-1)) strcpy(user->lasttellto,u->name); */
    u->newtell++;
    sprintf(text,"%s prave cita nejaky text. Odkaz ulozeny do %s tell-bufferu...\n",u->name,pohl(u,"jeho","jej"));
   }
  else
   sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
  write_user(user,text);
  return;
 }


user->commused[1]++;
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~OL~FW\252C4(~OL~FW\252C%d%s~OL~FW\252C4) >> ~OL~FW\252C3%s~RS~FW %s\n",notiuser?6:3,sklonuj(u,3),user->name,inpstr);
write_user(user,text);
record_tell(user,text);
if (u->igncoltell) colour_com_strip(inpstr);
sprintf(text,"~OL~FW\252C4>> ~OL~FW\252C%d%s~RS~FW %s\n",notivictim?6:3,name,inpstr);
write_user(u,text);
record_tell(u,text);
}

/***!!! COOLTALK - ZMENA - robi vselijake blbosti.... ;-) ***/
/************************************************************/

void cooltalk(user,inpstr)  /* cooltalk alebo coolsay */
UR_OBJECT user;
char *inpstr;
{
char *name, vysledok[MAX_LAME+1],temp[MAX_LAME+1];
int cislo;

if (user->muzzled) {
      write_user(user,"Si umlcany - nemozes pouzivat cooltalk.\n");  return;
      }
if (word_count<3 || !is_number(word[1])) {
      write_user(user,"Pouzi: ctalk <typ> <sprava>\n");  return;
      }
      
cislo=atoi(word[1]);

if (cislo<0 || cislo>MAX_CISLO) {
      sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu 1-%d.\n",MAX_CISLO);
      write_user(user,text);      return;
      }
if (user->vis) name=user->name; else name=invisname(user);
inpstr=remove_first(inpstr);
if (strlen(inpstr)>COOLTALK_LEN) {
      sprintf(text,"Prepac, maximalna dlzka pre cooltalk je %d znakov.\n",COOLTALK_LEN);
      write_user(user,text);
      return;
      }

if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo cooltalkovat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

/* Tento switch vola jednotlive funkcie, ktore modifikuju inpstr.
   Rozsah volanych funkcii je 1 az MAX_CISLO */
bzero(vysledok,sizeof(vysledok));
switch(cislo) {
	  case 0: sstrncpy(vysledok,inpstr, MAX_LAME-1); break;
	  case 1: sstrncpy(vysledok,lamerize(inpstr), MAX_LAME-1); break;
	  case 2: sstrncpy(vysledok,revert(inpstr), MAX_LAME-1); break;
	  case 3: sstrncpy(vysledok,lame_color(inpstr,1), MAX_LAME-1); break;
	  case 4: sstrncpy(vysledok,lame_color(inpstr,2), MAX_LAME-1); break;
	  case 5: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break; 
	  case 6: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  case 7: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break;
	  case 8: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  default: write_user(user,"Bug v cooltalk()!?\n"); return;
	  }

if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && cislo==7) {
  write_user(user,noswearing);
  return;
 }

if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo cooltalkovat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

user->commused[2]++;
sprintf(text,"~FT\252C2%s~RS~FW: %s\n",pohl(user,"\253P3","\253P4"),vysledok);
write_user(user,text);
sprintf(text,"~FT\252C2%s %s~RS~FW: %s\n",name, pohl(user,"\253P1","\253P2"), vysledok);
write_room_except(user->room,text,user);
record(user->room,text);
}

/* ZMENA cooltopic */
void cooltopic(user,inpstr)  /* meni topic */
UR_OBJECT user;
char *inpstr;
{
char *lamerize(char buf[]);       /* nadefinovacie fcii, ktore vracaju char */
char *lame_color(char buf[],int typ);
char *revert(char vstup[]);
char *name, vysledok[MAX_LAME+1],temp[MAX_LAME+1];
int cislo,duplicate=0,usertopics=0;
RM_OBJECT rm;

if (user->muzzled) {
      write_user(user,"Si umlcany - nemozes pouzivat cooltopic.\n");  return;
      }
if (word_count<3 || !is_number(word[1])) {
      write_user(user,"Pouzitie: .ctopic <typ> <tema>\n");  return;
      }

if (user->pp < TOPICCAN) {    /*PP*/
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, TOPICCAN);
	write_user(user,text);
	return;
	}

if (strlen(inpstr)>70) {
	write_user(user,"~FRPrepac, prilis dlhy text!\n");
	return;
	}
	
cislo=atoi(word[1]);
if (cislo<0 || cislo>MAX_CISLO) {
      sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu 1-%d.\n",MAX_CISLO);
      write_user(user,text);      return;
      }

if (cislo && strstr(inpstr,"~")) {
	write_user(user,"~FRPrepac, v cooltopicu typu inom nez 0 nemozes pouzit znak '~'!\n");
	return;
	}

if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit skaredy cooltopic!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

if (user->vis) name=user->name; else name=invisname(user);
inpstr=remove_first(inpstr);
/* Tento switch vola jednotlive funkcie, ktore modifikuju inpstr.
   Rozsah volanych funkcii je 1 az MAX_CISLO */
   
switch(cislo) {
	  case 0: sstrncpy(vysledok,inpstr,MAX_LAME-1); break;
	  case 1: sstrncpy(vysledok,lamerize(inpstr), MAX_LAME-1); break;
	  case 2: sstrncpy(vysledok,revert(inpstr), MAX_LAME-1); break;
	  case 3: sstrncpy(vysledok,lame_color(inpstr,1), MAX_LAME-1); break;
	  case 4: sstrncpy(vysledok,lame_color(inpstr,2), MAX_LAME-1); break;
	  case 5: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break; 
	  case 6: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  case 7: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break;
	  case 8: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  default: write_user(user,"Bug v cooltalk()!?\n"); return;
	  }
	  
if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && cislo==7) {
  write_user(user,noswearing);
  return;
 }

if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit skaredy cooltopic!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

/*sprintf(text,"~OL~FBYou are now setting cooltopic #%d.~RS~FW\n",cislo);
write_user(user,text);*/
if (strlen(inpstr)>TOPIC_LEN || strlen(vysledok)>COOLBUF_LEN) {
   /* ale dlzka user->room->topic[] je TOPIC_LEN+COOLBUF_LEN - je to lamerske ale je to tak! ;)  */
      sprintf(text,"~FRPrepac, maximalna dlzka pre cooltopic je %d znakov.\n",TOPIC_LEN);
      write_user(user,text);
      return;
      }
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"%s ~RS~FW~OL[%s]",vysledok,name);
for(rm=room_first;rm!=NULL;rm=rm->next) {
  if (rm!=user->room && !strcmp(rm->topic,text)) duplicate++;
  if (!strcmp(rm->topicmaker,user->name)) usertopics++;
 }
if (usertopics>2) { 
  sprintf(text,"Uz si nastavil%s 3 temy, najprv nejaku zmaz (.topic cancel).\n",pohl(user,"","a"));
  write_user(user,text);
  return;
 }
sstrncpy(user->room->topic,text,COOLBUF_LEN);
strcpy(user->room->topicmaker,user->name);
sprintf(text,"Tema nastavena na: %s\n",vysledok);
write_user(user,text);
sprintf(text,"~FR%s~FW %s TEMU na: %s\n",name, pohl(user,"nastavil","nastavila"), vysledok);
write_room_except(user->room,text,user);
for(rm=room_first;rm!=NULL;rm=rm->next) if (rm!=user->room && !strcmp(rm->topic,user->room->topic)) duplicate++;
if (duplicate) decrease_pp(user,TOPICDEC*(duplicate+1),NODYNAMIC);
else decrease_pp(user,TOPICDEC,NODYNAMIC);
}

/* ZMENA cdesc */
void cdesc(user,inpstr)  /* meni description */
UR_OBJECT user;
char *inpstr;
{
char *lamerize(char buf[]);       /* nadefinovacie fcii, ktore vracaju char */
char *lame_color(char buf[],int typ);
char *revert(char vstup[]);
char /* *name, */vysledok[MAX_LAME+1],temp[MAX_LAME+1];
int cislo;

if (word_count<3 || !is_number(word[1])) {
      write_user(user,"Pouzitie: .cdesc <typ> <popis>\n");  return;
      }
    
if (user->muzzled) { /* Zmena S. - toto uz bolo treba :) */ 
        sprintf(text,"Si %s, ziadna komunikacia cez .cdesc!\n",pohl(user,"umlcany","umlcana"));
	write_user(user,text); 
	return; 
	}    

if (strlen(inpstr)>80) {
	write_user(user,"Prepac, prilis dlhy text!\n");
	return;
	}
	 
cislo=atoi(word[1]);
if (cislo<0 || cislo>MAX_CISLO) {
      sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu 1-%d.\n",MAX_CISLO);
      write_user(user,text);      return;
      }

if (cislo && strstr(inpstr,"~")) {
	write_user(user,"~FRPrepac, v cooldescu typu roznom od 0 nemozes pouzit znak '~'!\n");
	return;
	}

/* UMBAKARNA */
/* if (inpstr[strlen(inpstr)-1]=='~') inpstr[strlen(inpstr)-1]='.'; */

if (!cislo && (strstr(inpstr,"~LB") || strstr(inpstr,"~LI"))) {
	write_user(user,"Bohuzial, nemozes pouzivat blikanie a pipanie.\n");
	return;
	}

/* if (user->vis) name=user->name; else name=invisname(user); */
inpstr=remove_first(inpstr);

if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit skaredy cooldesc!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

/* Tento switch vola jednotlive funkcie, ktore modifikuju inpstr.
   Rozsah volanych funkcii je 1 az MAX_CISLO */
switch(cislo) {
	  case 0: sstrncpy(vysledok,inpstr,MAX_LAME-1); break;
	  case 1: sstrncpy(vysledok,lamerize(inpstr), MAX_LAME-1); break;
	  case 2: sstrncpy(vysledok,revert(inpstr), MAX_LAME-1); break;
	  case 3: sstrncpy(vysledok,lame_color(inpstr,1), MAX_LAME-1); break;
	  case 4: sstrncpy(vysledok,lame_color(inpstr,2), MAX_LAME-1); break;
	  case 5: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break; 
	  case 6: sstrncpy(temp,lamerize(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  case 7: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,1), MAX_LAME-1);
	          break;
	  case 8: sstrncpy(temp,revert(inpstr), MAX_LAME-1);
	          sstrncpy(vysledok,lame_color(temp,2), MAX_LAME-1);
	          break;
	  default: write_user(user,"Bug v cooltalk()!?\n"); return;
	  }

if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && cislo==7) {
  write_user(user,noswearing);
  return;
 }
if ((user->level<MOZENADAVAT) && (contains_swearing(vysledok,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit skaredy cooldesc!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }

if (cislo && (strlen(inpstr)>USER_DESC_LEN || strlen(vysledok)>DESC_CHAR_LEN)) {
      sprintf(text,"Prepac, maximalna dlzka pre cooldesc je %d znakov.\n",USER_DESC_LEN);
      write_user(user,text);
      return;
      }

/* strcat(vysledok,"~RS~BK"); */
if ((strlen(vysledok)+strlen(user->name)-(colour_com_count(vysledok)))+1>39) {
      write_user(user,"Prilis dlhy popis.\n");  return;
      }      /* (S) Xaxa toto tu bolo treba!!! */
      
sstrncpy(user->desc,vysledok, DESC_CHAR_LEN); /* POZOR na toto!!! */
sprintf(text,"Popis nastaveny na: %s %s\n",user->name, vysledok);
write_user(user,text);
}

/*                           F U N K C I E                               
 **********
 **********  The Lame Filter
 **********  Slovak version 1.3 by Buko <bukovcan@alpha.euba.sk>
 **********/
char *lamerize(buf)
char buf[MAX_LAME+1];
{
	register int i, found=0,  count=0;
	unsigned int j;
	static char out[MAX_LAME+1];
        char samohlasky[]={'a','e','u','o','y','i'};
	time_t t;

		srand((unsigned) time(&t));
		bzero(out,sizeof(out));
		    for(j=0; j<strlen(buf); j++) {
		    if( buf[j]=='v') {
                          found=0;
                          for(i=0;i<6;i++) if (buf[j+1]==samohlasky[i] || buf[j+1]==toupper(samohlasky[i])) found=1;
                          if (!found) out[count++]='f';
                          else if ( (rand()%3==1) ) out[count++]='w';
                               else out[count++]='v';
		    }
		    else if( buf[j]=='V') {
                           found=0;
                           for(i=0;i<6;i++) if (buf[j+1]==samohlasky[i] || buf[j+1]==toupper(samohlasky[i])) found=1;
                           if (!found) out[count++]='F';
                           else if ( (rand()%3==1) ) out[count++]='W';
                                else out[count++]='V';
                    }
		    else if( buf[j]=='i' || buf[j]=='I') {
                          found=0;
                          for(i=0;i<4;i++) if (buf[j+1]==samohlasky[i] || buf[j+1]==toupper(samohlasky[i])) found=1;
                          if (found) out[count++]='j';
	                  else if(rand()%5>=3) out[count++]='1';
                               else out[count++]='i';
		    }
		    else if( buf[j]=='o' || buf[j]=='O') {
		    	if( rand()%3==1 ) {
		    		out[count++]='('; out[count++]=')';
			} else out[count++]='0';
		    }
                    else if( buf[j]==' ' && buf[j+1]=='a' && buf[j+2]==' '){
		    out[count++]=' '; out[count++]='&'; out[count++]=' '; j+=2;
		    }
                    else if( buf[j]=='C' && (buf[j+1]=='h' || buf[j+1]=='H') && (rand()%2==1)){
		    out[count++]='X'; j+=1;
		    }
                    else if( buf[j]=='c' && buf[j+1]=='h' && (rand()%2==1) ){
                    out[count++]='x'; j+=1;
		    }
                    else if( buf[j]=='k' && buf[j+1]=='s') {
                    out[count++]='x'; j+=1;
		    }
                    else if( buf[j]=='k' && buf[j+1]=='v') {
                    out[count++]='q'; j+=1;
		    }
		    else if( buf[j]=='l' && (rand()%3==1)) {
				out[count++]='|';
		    }
  		    else if(buf[j]=='s'&&(buf[j+1]==' '||ispunct(buf[j+1]))){
				out[count++]='z'; out[count++]='z';
		    }
		    else if( (buf[j]=='e' || buf[j]=='E') &&(rand()%5 >= 3) ){
				out[count++]='3';
		    }
                    else if ((buf[j]=='s' || buf[j]=='S') && (rand()%3==1) ){
                                out[count++]='$';
                    }
		    else if( rand()%6==1 ) out[count++]=toupper(buf[j]);
		    else out[count++]=buf[j];
		}

	out[count]='\0'; /*DZR*/
	return out;
}

/******** Color lame filter ***********/
char *lame_color(buf, typ)
char buf[MAX_LAME+1];
int typ;
{
	register int j, i, count=0;
	static char out[MAX_LAME+1];
	char colors[]={'R','G','B','W','Y','M','T','K'};
	/* pozn. 'K' je cierna - standardne sa nepouziva, preto iba rand()%7 */

		bzero(out,sizeof(out));
		/* srand((unsigned) getpid());     Uvidime SRANDu .... */
		i=strlen(buf);
		for(j=0; j<i; j++) {
		  /* typ 1 meni kazde pismeno inou farbou */
		  if (typ==1) {  if (buf[j]=='\n') {out[count++]='\n'; break; }
				 out[count++]='~';
				 if (rand()%2==1) { out[count++]='O'; out[count++]='L'; out[count++]='~'; }
				 out[count++]='F';
				 out[count++]=colors[rand()%7];
				 out[count++]=buf[j];
				 if (buf[j]!='\n') {out[count++]='~'; out[count++]='R'; out[count++]='S';}
			      }
		  /* typ 2 meni kazde slovo inou farbou */
		  if (typ==2) {  if (buf[j]=='\n') {out[count++]='\n'; break; }
				 if ((buf[j]==' ' && buf[j+1]!=' ') || j==0) { out[count++]='~'; out[count++]='R'; out[count++]='S';
				 out[count++]='~';
				 if (rand()%2==1) { out[count++]='O'; out[count++]='L'; out[count++]='~'; }
				 out[count++]='F';
				 out[count++]=colors[rand()%7];
				 }
				 out[count++]=buf[j];
				 if (j==i-1) {out[count++]='~'; out[count++]='R'; out[count++]='S';}
			      }
		}

	out[count]='\0'; /*DZR*/
	return out;
}
/********************************************************************/

/*** Reverts a string ***/
char *revert(char vstup[MAX_LAME+1])
{
int dlzka,i;
static char out[MAX_LAME+1];

bzero(out,sizeof(out));
dlzka=strlen(vstup)-1;
for (i=0; i<=dlzka; i++) {
    out[i]=vstup[dlzka-i];
    }
out[i]='\0';
return out;
}
/**************** KONIEC COOLTALK ******************************************/
  
/******* Kraviny (c) by Rider by Buko by Spartakus! ;-)))))  
   Toto je funkcny implementat kravin! Treba ho zaradit medzi ostatne  
   talkovacie rutiny.  
   Navrhujem dat (terajsim) SAGEom alebo co to bude v buducnosti.. knight?  
   (proste level number 3). Alebo novemu levelu - 4.  
   cow(user) - povie krawinu  ;) */  
  
void cow(user)  
UR_OBJECT user;
{
char *name;
#define POCET 21      /* Pocet slov v aky,kto,robi,komu */
#define POCET_AKO 11  /* Pocet slov v ako */


  char temp[81],flag1[81],flag2[81];
  char veta[81];
  char *aky[]={"drz","opit","mudr","krvav","stupidn","pekn","teroristick",
	       "uchylack","slovensk","15-rocn","ruzov","inteligentn",
               "plechov","smradlav","3x prevaren","pekeln","nadrzan",
               "nasran","zapichnut","ozrat","nebezpecn"};

  char *kto[]={"stary ujo1","doktor1","sluzba2","babka2","automat1","Buko1",
               "prezident1","slecna2","martan1","komando3","Rambo1","stonozka2",
               "farar1","slimak1","kurie oko3","sliepka2","velke prasa3",
               "jalovica2","plesnivy chlieb1","veduci1","Rider1"};

  char *ako[]={" tajne "," silno "," pomaly "," podvedome "," takmer ",
               " ne"," sikovne "," pekelne "," naisto "," uchylne "," drasticky "};

  char *robi[]={"pitval1","nakopal1","hack2","odbach2","kus2","reset2",
                "pobozkal1","otravil1","zaskrtil1","prichytil1","zbil1",
                "oklamal1","klonoval1","rozdupal1","oziaril1","vytvoril1",
                "mal1","zmaril1","zachoval1","odcudzil1","rozohnal1"};

  char *komu[]={"eho bacu","ych ludi", "u ufonku","e stado klokanov",
                "y telefon","e zrkadlo","u Slovenku","u ekonomku","y trabant",
                "y pocitac","eho vtaka","eho politika","y parlament",
                "y radiator","eho maratonskeho bezca","eho revizora",
                "u rusku babusku","u skupinku turistov","ych vedcov",
                "eho ucitela","u tarantulu"};

  int n,m;
  
  if(user->muzzled) {
  	write_user(user,"Si umlcany - nemozes posielat kraviny!\n"); return;
  	}
 
  for(m=0;m!=tsec;++m) { n=rand()%20; } /* Zamiesanie poradia */

  n=rand()%POCET;
  strcpy(veta,aky[n]);
  n=rand()%POCET;
  strcpy(temp,kto[n]);
  sprintf(flag1,"%s",&temp[strlen(temp)-1]);
  if (!strcmp(flag1,"1")) strcat(veta,"y");
  if (!strcmp(flag1,"2")) strcat(veta,"a");
  if (!strcmp(flag1,"3")) strcat(veta,"e");
  strcat(veta," "); 
  temp[strlen(temp)-1]=0;
  strcat(veta,temp);
  n=rand()%POCET_AKO;
  strcpy(temp,ako[n]);
  strcat(veta,temp);
  n=rand()%POCET;
  strcpy(temp,robi[n]);
  sprintf(flag2,"%s",&temp[strlen(temp)-1]);
  if (!strcmp(flag1,"1")) { 
                            if (!strcmp(flag2,"1")) temp[strlen(temp)-1]=0;
                            if (!strcmp(flag2,"2")) { temp[strlen(temp)-1]=0;
                                                      strcat(temp,"ol"); } 
                          } 
  if (!strcmp(flag1,"2")) { if (!strcmp(flag2,"1")) { temp[strlen(temp)-1]=0;
                                                      strcat(temp,"a"); }
                            if (!strcmp(flag2,"2")) { temp[strlen(temp)-1]=0;
                                                      strcat(temp,"la"); } 
                          }
  if (!strcmp(flag1,"3")) { if (!strcmp(flag2,"1")) { temp[strlen(temp)-1]=0;
                                                      strcat(temp,"o"); }
                            if (!strcmp(flag2,"2")) { temp[strlen(temp)-1]=0;
                                                      strcat(temp,"lo"); } 
                          }
  strcat(veta,temp);
  n=rand()%POCET;
  strcpy(temp,aky[n]);
  strcat(veta," "); strcat(veta,temp);
  n=rand()%POCET;
  strcpy(temp,komu[n]);
  strcat(veta,temp);
/* Konecna uprava */
  strcat(veta,"."); strcpy(temp,veta); 
  temp[0]=toupper(temp[0]);
  strcpy(veta,temp);
  
  sprintf(text, "~FT\252C2%s si~RS~FW: %s\n",pohl(user,"Vypotil","Vypotila"),veta);
  write_user(user,text);
  if (user->vis) name=user->name; else name=invisname(user);
  sprintf(text,"~FT\252C2%s %s~RS~FW: %s\n",name,pohl(user,"vypotil","vypotila"),veta);
  write_room_except(user->room,text,user);
  record(user->room,text);
}

/* POET 1.0 - free lyrics generator (r) RIDER software for Atlantis talker */

void poet(user, rym, kolko)
int rym, kolko;
UR_OBJECT user;

#define pocet_typof 10 /* kolko je typov koncoviek??? <file1..n>   */
#define maxrymof 7    /* maximalny pocet versov v jednom type + 1 */
#define dlzkrymu 80   /* maximalna dlzka jedneho versa v znakoch  */

/* Nasleduju definicie jednotlivych typov rymov. Prve cislo je pocet poloziek */
#define file1 {"5","Ja som z toho puk","Tuhy som jak druk","Tak uz ani muk","Pevnych par mam ruk","Si sta ruze puk"}
#define file2 {"5","Poviem ti par viet","Ty si ako kvet","Jak sa kruti svet","Hladam odpoved","Idem na spoved"}
#define file3 {"4","Tak znie moja gitara","Clovek toho natara","Ty si ale ostara","Zabijeme postara"}
#define file4 {"5","Casto sa to stava","Je to marna slava","Spytajte sa Slava","S radostou ti mava","Tak uz sa to stava"}
#define file5 {"4","Ked to clovek uvazi","Vsetko tuto je f pazi","Hoc argument zavazi","Rad sa prejdem po plazi"}
#define file6 {"6","Raz to bolo tak","Kracam spet sta rak","Ty si ale vtak","Brucho mam sta vak","Raz skocim pod vlak","Nechajme to tak"}
#define file7 {"4","Ludom chyba laska","Vravi plavovlaska","Pekna sedmikraska","Zivot je len maska"}
#define file8 {"5","Tak nevahaj a pod","Uz caka nasa lod","Po nebi ma vod","Len nikam nechod","Daco po mne hod"}
#define file9 {"5","Uz mam toho dost","Tebe pre radost","Vylejem si zlost","Ty si ale kost","Len ma nenazlost"}
#define file10 {"5","Takto znie ponuka","Dajme tam aj Buka","Hoc' ma boli ruka","Pustime ho dnuka","Zivot to je suka"}
   {

   char lyric[pocet_typof][maxrymof][dlzkrymu]={file1,file2,file3,file4,file5,file6,file7,file8,file9,file10};
   /* file1...filen treba doplnit pri doplneni v define!!! */

   char versebuff[5][80];
   int pocet,currentype,first,second,loop;
   time_t t;      /* quoli generatoru rand */


   srand((unsigned) time(&t));   /* generator rand, nex sa to neopakuje */

while(kolko-- > 0)
 {
 sprintf(text,"\n");
 write_user(user,text);
 
 for (loop=0; loop<=3;)
  {
   currentype=rand()%pocet_typof;
   pocet = ( *lyric[currentype][0]-'0');
	first=1+rand()%pocet;
	 do
	second=1+rand()%pocet;
	 while (second == first);
   strcpy(versebuff[0+loop++],lyric[currentype][first]);
   strcpy(versebuff[0+loop++],lyric[currentype][second]);
  }
   switch(rym)
   {
   case 1:
   sprintf(text,"%s\n%s\n%s\n%s\n",versebuff[0],versebuff[1],versebuff[2],versebuff[3]);
   write_user(user,text);
   break;
   case 2:
   sprintf(text,"%s\n%s\n%s\n%s\n",versebuff[0],versebuff[2],versebuff[3],versebuff[1]);
   write_user(user,text);
   break;
   case 3:
   sprintf(text,"%s\n%s\n%s\n%s\n",versebuff[0],versebuff[2],versebuff[1],versebuff[3]);
   write_user(user,text);
   
   break;
   default: write_user(user,"Chybicka se vloudila ;-))))\n"); break;
   }
 }
}

/* poetizuj - tuto sa to zlameri a vypise ;-) */

void poetizuj(user)
UR_OBJECT user;
{
int rym, kolko;
UR_OBJECT u;

if (word_count>4 || word_count<2) {
      write_user(user,"Pouzi: .poet <rym> <pocet sloh> [user]\n Rym je 1: zdruzeny (AABB) 2: obkrocny (ABBA) 3: striedavy (ABAB)\n");  return;
      }

rym=atoi(word[1]);
kolko=atoi(word[2]);
   if (rym < 1 || rym > 3)
	{
	sprintf(text,"poet: rym je 1: zdruzeny (AABB) 2: obkrocny (ABBA) 3: striedavy (ABAB)\n");
	write_user(user,text);
	return;
	}
   else if (kolko > MAXSLOHY  || kolko < 1)
	{
	sprintf(text,"poet: pocet sluoh moze byt 1 az %d\n",MAXSLOHY);
	write_user(user,text);
	return;
	}      
u=user;
if (word_count==4)  { if (!(u=get_user(word[3]))) {
                       if (u==user) { write_user(user,"Posielat basnicky sam sebe je sestnasty znak sialenstva! ;-).\n"); return; }
                     write_user(user,notloggedon);
                     write_user(user,"Pouzi: .poet <rym> <pocet sloh> [user]\n Rym je 1: zdruzeny (AABB) 2: obkrocny (ABBA) 3: striedavy (ABAB)\n");
                     return;
                     }
                     
                     if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
	if (user->room->sndproof) {
		sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s Ta nemoze pocut.\n",u->name);
		write_user(user,text);
		return;
		}
	if (u->room->sndproof) {
		sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze Ta pocut.\n",u->name);
		write_user(user,text);
		return;
		}      
          }
      if (check_ignore_user(user,u)) {
  	sprintf(text,"%s ta ignoruje.\n",u->name);
	write_user(user,text);
	return;
       }
      if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"V zalari LEN z plneho hrdla (s plnymi pp-ckami ;)!\n");
	return;
	}
                     sprintf(text,"Poslal%s si ",pohl(user,"","a"));
		     strcat(text,pohl(u,"mu","jej"));
		     strcat(text," nejaku peknu basnicku ;)\n");
                     write_user(user,text);
   }
   else write_user(user,"~OLToto je basnicka iba pre Teba:\n");
   

poet(u,rym,kolko);
if (word_count==4) { sprintf(text,"\n~OLTuto basnicku ti poslal%s ~FR%s~FW~RS.\n",pohl(user,"","a"),user->name);
                     write_user(u,text);
                     }
return;
}

/* End of poet!!!!! ;-) */

/*** Fortune cookies - ZMENA - pridane! Vypise nejaku nahodne zvolenu "mudrost" */

void fortune_cookies(user)
UR_OBJECT user;
{
FILE *fp;
char filename[80],line[201];
int n,ktory,all;

if (word_count>2) {
      write_user(user,"Pouzi: .fortune [<all>]\n");  return;
      }

all=0;
if ((word_count==2) && (!strcmp(word[1],"all"))) {
	all=1;
	}
	else if (word_count>=2) { write_user(user,"Pouzi: .fortune [<all>]\n");  return;
	}

ktory=(random()%COUNT+1);
sprintf(filename,FORTUNE_FILE);
if (!(fp=ropen(filename,"r"))) write_user(user,"Chyba: Nenasiel som potrebny subor!\n"); /*APPROVED*/
else {
	
      for(n=0;n<=ktory;n++) {
	    if (!feof(fp)) fgets(line,200,fp);
	    }
      if (all) { sprintf(text,"~OL%s mudruje: ~RS%s",user->name,line);
		 write_room_except(user->room, text, user);
		 sprintf(text,"~OLMudrujes: ~RS%s",line);
		 write_user(user, text);
	       }
      else     { sprintf(text,"~OLPamataj: ~RS%s",line);
		 write_user(user, text);
	       }
       fclose(fp);
      }
}

/* .hint - tip, hadze pomocky a uzitocne hlasky k menej znamym
    prikazom/parametrom v Atlantise */
void hint(user, ihned)
UR_OBJECT user;
int ihned;
{
FILE *fp;
char filename[80],line[501];
int count,ktory,n;

if ((word_count>2) && (!ihned)) {
      write_user(user,"Pouzi: .hint\n");  return;
      }
sprintf(filename,HINT_FILE);
if (!(fp=ropen(filename,"r"))) { write_user(user,"Chyba: Nenasiel som potrebny subor s tipmi!\n"); return; } /*APPROVED*/

count=0;
while(!feof(fp)) {                   /* pocitadlo riadkov.. */
		fgets(line,500,fp);
		count++;
		}
rewind(fp);
ktory=(random()%count+1);
for(n=1;n!=ktory+1;n++) {
	    fgets(line,500,fp);
	    }
 fclose(fp);
sprintf(text,"~FTTIP: %s si, ze:\n~FW%s",pohl(user,"Vedel","Vedela"), line);
write_user(user, text);
if (user->level>CIT && note_vote(user)) {
  sprintf(text,"~OL~FWPrebieha hlasovanie, do ktoreho si sa este nezapojil%s. (pouzi prikaz '.vote')\n",pohl(user,"","a"));
  write_user(user,text);
 }
}


/*** Echo something to screen ***/
void echo(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int swears=0;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes echovat.\n");  return;
      }
if (word_count<2) {
      write_user(user,"Pouzi: .echo text\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (swears=contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo echovat!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
      write_level(WIZ,1,text,user);            
      return;
     }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
     }
	
if (detect_user(inpstr,user)) { write_user(user,"Tento text nemozes echovat.\n"); return; }

if (!user->room->sndproof && swears) {
  sprintf(text,"%s echuje (~FT%s~FW): %s\n",user->name, user->room->name, inpstr);
  write_level(WIZ,1,text,NULL);
 }
sprintf(text,"%s\n",inpstr);
write_room(user->room,text);
record(user->room,text);
}

/*** ZMENA Spakky - shoutecho (.secho) Ako Echo, ale shutne. Cize ako Shout,
     ale echuje ;-))) Poxopili ? ;-) ***/
void secho(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i;

if (user->muzzled) {
	write_user(user,"Si umlcany, nemozes sa ozyvat!!!\n");return;
	}
if (word_count<2) {
	write_user(user,"Pouzi: .secho text\n"); return;
	}
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo shout-echovat!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);            
      return;
     }	
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
     }	
if (user->level==GOD && !strcmp(word[1],"NULL")) {
  inpstr=remove_first(inpstr);
  write_room(NULL,inpstr);
  write_room(NULL,"\n");
  return;
 }
if (detect_user(inpstr,user)) { write_user(user,"Nemozes hlasno echovat tento text.\n"); return; }
 
if (user->pp < SECHOCAN) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, SECHOCAN); 
	write_user(user,text); 
	return; 
	} 
 
if(user->room->group!=1) sprintf(text,"%s shout-echuje (~FT%s~FW): %s\n",user->name,user->room->name,inpstr);
else sprintf(text,"%s shout-echuje: %s\n",user->name,inpstr);
if (!user->room->sndproof) write_level(WIZ,1,text,NULL);
sprintf(text,"%s\n",inpstr);
write_user(user,text);

if (user->room->group!=1) {
   write_room_except(user->room,text,user); /* do roomy...      */
   /* sprintf(texthb,"%s%s",user->room->label,text); */
   record_portalisshout(user->room->label,text);
   for(i=0;i<MAX_LINKS;i++) {               /* a vsetkych okolo */
       if (user->room->link[i]!=NULL)
          write_room_except(user->room->link[i],text,user);
       }
   return;
   }

write_room_except(NULL,text,user);
record_shout(text);
decrease_pp(user,SECHODEC,SECHODYN); 
}		

/*** Spakky - pecho = private echo ***/
/* doverne ozyvanie? to je co za novotvar? :) */
void pecho(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
/* char *name; */
UR_OBJECT u;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes sa ozyvat ani doverne.\n");  return;
      }
if (word_count<3) {
      write_user(user,"Pouzi: .pecho <uzivatel> <text>\n");  return;
      }
word[1][0]=toupper(word[1][0]);
if (!strcmp(word[1],user->name)) {
      write_user(user,"Doverne ozyvanie sa samemu sebe je patnasty priznak sialenstva.\n");
      return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
      
if (!strcmp(u->name,"Poseidon") || !strcmp(u->name,"Smsbot")) {
	write_user(user,"Prepac, tomuto uzivatelovi nemozes doverne echovat.\n");
	return;
	}

if (u->afk) {
      if (u->afk_mesg[0])
            sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\n",u->name,u->afk_mesg);
      else sprintf(text,"%s je prave mimo klavesnice.\n",u->name);
      write_user(user,text);
      return;
      }
if (u->ignall) {
      if (u->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",u->name);
        else if (u->filepos) sprintf(text,"%s prave cita nejaky text.\n",u->name);      
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
      write_user(user,text);  return;
      } 
      
if (u->igntell) {
   if (user->level<WIZ || u->level>=user->level) {
      sprintf(text,"%s ignoruje telly a ostatne sukromne hlasky.\n",u->name);
      write_user(user,text);
      return;
      } else if (user->level>WIZ || u->level<user->level) { sprintf(text,"%s by ta normalne %s ignorovat.\n",u->name, pohl(u,"mal","mala"));
               write_user(user,text); }
          }
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
if (check_ignore_user(u,user)) {  /* ignorovanie single usera */
      sprintf(text,"Ved %s ignorujes.\n",sklonuj(u,4));
      write_user(user,text);
      return;
    }

if (u->room->group!=user->room->group) {
      sprintf(text,"%s je od teba prilis %s na doverne echo...\n",u->name,pohl(u,"vzdialeny","vzdialena"));
      write_user(user,text);
      return;
      }
      
if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
	if (user->room->sndproof) {
		sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s ta nemoze pocut.\n",u->name);
		write_user(user,text);
		return;
		}
	if (u->room->sndproof) {
		sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze ta pocut.\n",u->name);
		write_user(user,text);
		return;
		}      
      }

if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"V zalari LEN z plneho hrdla (s plnymi pp-ckami ;)!\n");
	return;
	}
if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s skaredo private-echovat!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);            
      return;
     }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
     }

/* if (user->vis) name=user->name; else name=invisname(user); */
inpstr=remove_first(inpstr);
if (detect_user(inpstr,user)) { write_user(user,"Nemozes doverne echovat tento text.\n"); return; }
sprintf(text,"~OL(echo-pre %s)~RS %s\n",sklonuj(u,4),inpstr);
write_user(user,text);
sprintf(text,"~OL(echo-pre %s)~RS %s\n",sklonuj(u,4),inpstr);
record_tell(user,text);
if (u->level>=KIN && u->level>=user->level)
 sprintf(text,"[Pecho od %s] %s\n",sklonuj(user,2),inpstr);
else 
 sprintf(text,"%s\n",inpstr);
write_user(u,text);
record_tell(u,text);
}

/* ZMENA: zisti ci sa v danom stringu nenachadza meno usera */

int detect_user(char *inpstr,UR_OBJECT except)
{
UR_OBJECT user;  
char coco[20]; 
char tempstr[ARR_SIZE+5]; 
  
sstrncpy(tempstr, inpstr, ARR_SIZE);

colour_com_strip(tempstr);  
strtolower(tempstr);
if (strstr(tempstr,"ti povedal")) return 1;
if (strstr(tempstr,"tell")) return 1;
if (strstr(tempstr,"heslo")) return 1;
if (strstr(tempstr,"passw")) return 1;
if (strstr(tempstr,"zamkn")) return 1;
if (strstr(tempstr,"odomk")) return 1;
if (strstr(tempstr,"posta")) return 1;
if (strstr(tempstr,"postu")) return 1;
if (strstr(tempstr,"mail")) return 1;
if (strstr(tempstr,"panic")) return 1;
if (strstr(tempstr,"boot")) return 1;
if (strstr(tempstr,"shut")) return 1;
if (strstr(tempstr,"notify")) return 1;
if (strstr(tempstr,"sign o") || strstr(tempstr,"cyber")
                           || strstr(tempstr,"system")) return 1;
/* staci ked tam nebude "tells you:" a Poseidon bude spokojny.. ;) */

for (user=user_first;user!=NULL;user=user->next) {  
  if (user->login || user->room==NULL || !user->zaradeny
  || user->type!=USER_TYPE || user==except) continue;
  strcpy(coco,user->name);  
  strtolower(coco);
  if (strstr(tempstr,coco)) return 1;  
  } 
    
return 0;  
}

int get_memory_usage(void)
{
   FILE *meminfofile;
   char znak,filename[100];
   int memory;
   unsigned int tmp;

   sprintf(filename,"/proc/%d/stat",getpid());
   if ((meminfofile=fopen(filename,"r"))==NULL)
      return -1;
   fscanf(meminfofile,"%u %s %c %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %d %u %u %u %u %u %u %u %u %u %u %u %u",&tmp,filename,&znak,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&memory,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp);
   fclose(meminfofile);
   return memory;
}

int get_resmem_usage(void)
{
   FILE *meminfofile;
   char znak,filename[100];
   int memory;
   unsigned int tmp;

   sprintf(filename,"/proc/%d/stat",getpid());
   if ((meminfofile=fopen(filename,"r"))==NULL)
      return -1;
   fscanf(meminfofile,"%u %s %c %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %d %u %u %u %u %u %u %u %u %u %u %u",&tmp,filename,&znak,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&memory,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp,&tmp);
   fclose(meminfofile);
   return memory*getpagesize();
}

void go(user)
UR_OBJECT user;
{
RM_OBJECT rm,strm;
UR_OBJECT u;
int i,linked,vec;
char *name,travelby[50];

if (user->vis) name=user->name; else name=invisname(user);	
if (word_count<2 && user->level<CIT) {
  write_user(user,langselect(user,"Pouzi: .go <miestnost>\n","Use: .go <roomname>"));  return;
 }
else
 if(word_count<2) {
   if (user->room->group==4) {
     write_user(user,"Odtialto sa nemozes transportovat.\n");
     return; 
    }
   if (user->room->group==2) rm=get_room("ostrov",NULL);
   else rm=get_room("namestie",NULL);
   if (rm==user->room) { 
     if (user->room->group==2) write_user(user,"Uz si na ostrove, nerob paniku.\n");
     else write_user(user,"Uz si na namesti, nerob paniku.\n");
     return; 
    }
   if (!strcmp(user->room->name,"plachetnica")
   || !strcmp(user->room->name,"letun") || !strcmp(user->room->name,"tunel")) {
     write_user(user,"Lutujem, z tohoto miesta sa nemozes transportovat.\n");
     return;
    }
   if (rm->group!=user->room->group) { write_user(user,"Lutujem, si mimo dosahu transportu!\n"); return; }
   if (user->jailed) { write_user(user,"Odtialto sa nedostanes!\n"); return; }
   if (user->glue) {
     sprintf(text,"Si tu %s zuvackou k zemi, nemozes sa ani pohnut!\n",pohl(user,"prilepeny","prilepena"));
     write_user(user,text);
     return;
    }
   wrtype=WR_IOPHR;
   user->commused[8]++;
   if (user->room->group==2) sprintf(text,"~FT%s sa %s na ostrov ...\n", name, pohl(user,"odtransportoval","odtransportovala"));
   else sprintf(text,"~FT%s sa %s na namestie ...\n", name, pohl(user,"odtransportoval","odtransportovala"));      		
   if (user->vis) write_room_except(user->room,text,user);
   strm=user->room;
   move_user(user,rm,3);
   if (user->room->group==2) write_user(user,"Transport na ostrov...\n");
   else write_user(user,"Transport na namestie...\n");
   sprintf(text,"<- %s sa sem %s.\n",name,pohl(user,"transportoval","transportovala"));
   if (user->vis) write_room_except(rm,text,user);
    
   for(u=user_first;u!=NULL;u=u->next)  {
     if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
     if (!strcmp(u->follow,user->name)) {
       if (u->room==strm) {
         sprintf(text,"Nasledujes %s %s.\n",sklonuj(user,4),rm->into);
         write_user(u,text);
         move_user(u,rm,3);
         sprintf(text,"<- %s sa sem %s.\n",u->name,pohl(u,"transportoval","transportovala"));
         if (u->vis) write_room_except(rm,text,u);
        }
      }
    }
   wrtype=0;
   return;  
  }

if ((rm=get_linked_room(word[1],user->room))==NULL) {
  if ((rm=get_room(word[1],user))==NULL) {
    write_user(user,langselect(user,nosuchroom,nosuchroom_en));
    return;
   }
 }

if (user->glue) {
      sprintf(text,"Si tu %s zuvackou k zemi, nemozes sa ani pohnut!\n",pohl(user,"prilepeny","prilepena"));
      write_user(user,text);
      return;
      	}

/* ()STROV */
/*if (!strcmp(rm->name,SHIPPING_SHIP) && (strcmp(user->rihand,"*") || strcmp(user->lehand,"*")))
	{
         write_user(user,"Pasovat predmety na lod je ZAKAZANE!\n");
         return;
	}*/
linked=0;
for(i=0;i<MAX_LINKS;++i) if (user->room->link[i]==rm) linked=1;

travelby[0]='\0';
for(i=0;i<MPVM;i++)
 if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->function==5 && user->room->dur[i]>9999) {
   u=get_user_by_sock(user->room->dur[i]%10000);
   if (u!=NULL && u==user)
    sprintf(travelby," na %s",predmet[user->room->predmet[i]]->inytiv);
  }
if ((linked || rm==user->room) && (rm->group==4 || user->room->group==4)) { /* Atlantis Wars walking system (V)*/
  if (!strcmp(rm->name,"diera")) {
    sprintf(text,"~FB%s skocil%s %s odkial vychadza ziara..\n",name,pohl(user,"","a"),rm->into);
    write_room_except(user->room,text,user);
    sprintf(text,"~OLSkocil%s si %s odkial vychadza ziara..\n",pohl(user,"","a"),rm->into);
    write_user(user,text);
    brutalis_gate--;
    if (brutalis_gate==1000) {
      sprintf(text,"~FBZiara vyhasla a ostala po nej len hlboka jama..\n");
      write_room_except(user->room,text,user);
      unlink_room(user->room,rm);
      brutalis_gate=0;
     }
    user->room=rm;
    return;
   }

  if (rm==user->room) {
    if (user->into!=NULL) { 
      if (user->from==NULL) {
        user->from=user->into;
        user->into=NULL;
        user->way=-user->way;
        sprintf(text,"~FGRozmyslel%s si si to a vracias sa%s s5.\n",pohl(user,"","a"),travelby);
        write_user(user,text);
        sprintf(text,"~FG%s si to rozmyslel%s to a vracia sa%s s5.\n",user->name,pohl(user,"","a"),travelby);
        write_room_except(user->room,text,user);
        return;
       }
      else {
        user->into=NULL;
        sprintf(text,"~FGIdes%s rovno %s.\n",travelby,user->room->into);
        write_user(user,text);
        sprintf(text,"~FG%s ide%s rovno %s.\n",user->name,travelby,user->room->into);
        write_room_except(user->room,text,user);
        return;
       }
     }
    else { 
      if (sk(user)) sprintf(text,"~FGUz si %s!\n",rm->where);
      else sprintf(text,"~FGYou are already in the %s!\n",rm->name_en);
      write_user(user,text);
      return;
     }
   }
  if (user->into==NULL) {
    if (user->from==NULL) {
      user->into=rm;
      vec=get_vehicle(user);
      if (vec>-1 && predmet[vec]->uoutphr!=NULL) {
        sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->uoutphr,user,NULL,rm,0));
        write_user(user,text);
       }
      else {
        sprintf(text,"~FGVydal%s si sa%s smerom %s.\n",pohl(user,"","a"),travelby,rm->into);
        write_user(user,text);
       }
      if (vec>-1 && predmet[vec]->routphr!=NULL) {
        sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->routphr,user,NULL,rm,10));
        write_room_except(user->room,text,user);
       }
      else {
        sprintf(text,"~FG%s sa vydal%s%s smerom %s.\n",user->name,pohl(user,"","a"),travelby,rm->into);
        write_room_except(user->room,text,user);
       }
      return;
     }
    else if (user->from==rm) {
      user->into=rm;
      user->way=-user->way;
      user->from=NULL;
      sprintf(text,"~FGVracias sa%s %s.\n",travelby,rm->into);
      write_user(user,text);
      sprintf(text,"~FG%s sa vracia%s %s.\n",user->name,travelby,rm->into);
      write_room_except(user->room,text,user);
      return;
     }
    else {
      user->into=rm;
      sprintf(text,"~FGNamieril%s si si to%s smerom %s.\n",pohl(user,"","a"),travelby,rm->into);
      write_user(user,text);
      sprintf(text,"~FG%s si to%s namieril%s smerom %s.\n",user->name,travelby,pohl(user,"","a"),rm->into);
      write_room_except(user->room,text,user);
      return;
     }
   }
  if (user->into!=NULL) {
    if (user->into==rm) {
       sprintf(text,"~FGVed uz ides %s.\n",user->into->into);
       write_user(user,text);
       return;
      }
    if (user->from==NULL) {
      user->from=user->into;
      user->into=rm;
      user->way=-user->way;
      sprintf(text,"~FGOtocil%s si sa a ides%s smerom %s.\n",pohl(user,"","a"),travelby,rm->into);
      write_user(user,text);
      sprintf(text,"~FG%s sa otocil%s a ide%s smerom %s.\n",user->name,pohl(user,"","a"),travelby,rm->into);
      write_room_except(user->room,text,user);
      return;
     }
    else {
      user->into=rm;
      if (user->from==rm) { user->way=-user->way; user->from=NULL; }
      sprintf(text,"~FGRozmyslel%s si si to a ides%s smerom %s.\n",pohl(user,"","a"),travelby,rm->into);
      write_user(user,text);
      sprintf(text,"~FG%s si to rozmyslel%s a ide%s smerom %s.\n",user->name,pohl(user,"","a"),travelby,rm->into);
      write_room_except(user->room,text,user);
      return;
     }
   }
 }

if (rm==user->room) {
      if (sk(user)) sprintf(text,"Uz si %s!\n",rm->where);
      else sprintf(text,"You are already in the %s!\n",rm->name_en);
      write_user(user,text);
      return;
      }

/* See if link from current room */
if (linked) {
  if (!strcmp(rm->name,"hrob")) {
    sprintf(text,"~FB%s skocil%s %s odkial vychadza ziara..\n",name,pohl(user,"","a"),rm->into);
    write_room_except(user->room,text,user);
    sprintf(text,"~OLSkocil%s si %s odkial vychadza ziara..\n",pohl(user,"","a"),rm->into);
    write_user(user,text);
    portalis_gate--;
    if (portalis_gate==1000) {
      sprintf(text,"~FBZiara vyhasla a ostal po nej len prazdny hrob..\n");
      write_room_except(user->room,text,user);
      portalis_gate=0;
     }
    unlink_room(user->room,rm);
    user->room=rm;
    return;
   }
    strm=user->room;
    move_user(user,rm,0);
    for(u=user_first;u!=NULL;u=u->next)  {
      if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
      if (!strcmp(u->follow,user->name)) {  /*smolka-niekto nas followuje!;)*/
        if (u->room==strm) {
/*	  if (rm->group==4) {
	   } */
	  sprintf(text,"Nasledujes %s %s.\n",sklonuj(user,4),rm->into);
	  write_user(u,text);
	  move_user(u,rm,0);
	 }
       }
     }
    return; 
 }

      
if (user->lieta) {
	if (user->room->group==4) {
		write_user(user,"Tu nemozes lietat.\n");
		return;
		}
	if (rm->group==4 || !strcmp(rm->name,"hrob") || !strcmp(rm->name,"diera")) {
		write_user(user,"Tam sa neda zaletiet.\n");
		return;
		}
	if (user->jailed) {
		write_user(user,"Bohuzial, v zalari mas pristrihnute kridelka! :-)\n");
		return;
		}
	if (!strcmp(user->room->name,"tunel")) {
		write_user(user,"Z tunela nevyletis, je tu strasna tma.\n");
		return;
		}
	if (!strcmp(rm->name,"zalar"))
	 { 
	  if (!has_room_access(user,rm)) {
		sprintf(text,"Nemozes vletiet do zalara ked nie si pozvan%s na navstevu!\n",pohl(user,"y","a"));
        	write_user(user,text);
		return;
		}	
	  if (visit<0) {
		write_user(user,"Bohuzial, momentalne nie su navstevne hodiny.\n");
		return;
		}	
	 }
	if ((rm->access==PRIVATE || rm->access==FIXED_PRIVATE) && 
	   (user->invite_room!=rm)) {
	   	write_user(user,"Nemozes zaletiet do PRIVATE miestnosti, poziadaj o pristup pomocou .letmein!\n");
	   	return;
	   	}	  
	if (!strcmp(rm->name,"tunel")) {
		write_user(user,"Nemozes vletiet do tunela, je tam strasna tma.\n");
		return;
		}
	if (rm->access==GOD_PRIVATE) {
		write_user(user,"Nemozes zaletiet do BOZSKEJ miestnosti!\n");
		return;
		}
        if (rm->group!=user->room->group) { /* ak user->pp<MAXPP -> zruti sa */
        	if (user->pp<MAXPP) {
        		sprintf(text,"~OLTvoja energia nestacila na prelet ponad more ... %s si sa do mora, kde\n~OLTvoje telo zozrali drave morske zivocichy ...\n",pohl(user,"zrutil","zrutila"));
        		write_user(user,text);
                        sprintf(text,"~FRpad do mora");
                        logout_user(user,text);
                        return;
        		}
        	setpp(user,0);
	        for(i=0;i<HANDS;i++)
	         {
	          if (user->predmet[i]>-1)
		   {
	            if (predmet[user->predmet[i]]->type!=3 && predmet[user->predmet[i]]->type!=4)
	             {
	              user->carry-=predmet[user->predmet[i]]->weight;
	              user->predmet[i]=-1;
    	    	      user->dur[i]=0;
	             }
		   }
	         }
        	write_user(user,"~OLPodarilo sa ti preletiet ponad more, avsak tento let ta uplne vycerpal!\n");
        	}        
	strm=user->room;
	move_user(user,rm,4);
	return;
	}
            
if (user->level<WIZ)
 {
  if (user->room->group==1 && !strcmp(rm->name,"zalar"))
   {
    if (!has_room_access(user,rm))
     {
      sprintf(text,"Nemozes ist do zalara ked nie si pozvan%s na navstevu!\n",pohl(user,"y","a"));
      write_user(user,text); 
      return; 
     }
    if (visit<0)
     {
      write_user(user,"Bohuzial, momentalne nie su navstevne hodiny.\n");
      return;
     }	
    move_user(user,rm,6);
    return;
   }
  else 
   {
    if (sk(user)) sprintf(text,"%s sa odtialto nedostanes, pozri sa na mapu.\n",rm->into);
    else sprintf(text,"The %s is not adjoined to here.\n",rm->name_en);
    text[0]=toupper(text[0]);
    write_user(user,text);
    return;
   }
 }      
            
strm=user->room;
move_user(user,rm,1);
if ((play.on==1) && (!strcmp(rm->name,"amfiteater"))) write_user(user,"~OL~FRPsssst! Prebieha divadelne predstavenie, nerusit!\n");

for (u=user_first;u!=NULL;u=u->next) {
      	if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
	if (!strcmp(u->follow,user->name)) {
		if(u->room==strm) {
			if (u->level>=WIZ) {
			sprintf(text,"Nasledujes %s %s\n",sklonuj(user, 4),rm->into);
			write_user(u,text);
			move_user(u,rm,1);
			}
			else {
			sprintf(text,"Prepac, nemozes nasledovat %s %s!\n",sklonuj(user, 4),rm->into);
			write_user(u,text);
			}
		}
	}        
   }
}

int get_vehicle(UR_OBJECT user)
{
int i;
UR_OBJECT u;

for(i=0;i<MPVM;i++)
 if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->function==5 && user->room->dur[i]>9999) {
   u=get_user_by_sock(user->room->dur[i]%10000);
   if (u!=NULL && u==user) return user->room->predmet[i];
  }
return -1;
}

int move_vehicle(UR_OBJECT user,RM_OBJECT newroom,int quiet)
{
int vec;

vec=get_vehicle(user);
if (vec>-1) {
  if (is_free_in_room(newroom)==-1) return 0;
  wrtype=WR_IOPHR;
  if (quiet==0) {
    if (predmet[vec]->rinphr!=NULL) {
      sprintf(text,"<- %s\n",parse_phrase(predmet[vec]->rinphr,user,NULL,newroom,0));
      write_room(newroom,text);
     }
    if (user->room!=NULL && user->room->group!=4 && predmet[vec]->routphr!=NULL) {
      sprintf(text,"-> %s\n",parse_phrase(predmet[vec]->routphr,user,NULL,newroom,10));
      write_room_except(user->room,text,user);
     }
    if (user->room!=NULL && user->room->group!=4 && predmet[vec]->uoutphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->uoutphr,user,NULL,newroom,10));
      write_user(user,text);
     }
   }
  put_in_room(newroom,vec,user->socket+10000);
  remove_from_room(user->room,vec,1);
  wrtype=0;
  return 1;
 }
return -1;
}

/*** Called by go() and move() ***/
void move_user(user,rm,teleport)
UR_OBJECT user;
RM_OBJECT rm;
int teleport;
{
RM_OBJECT old_room;
int result;

old_room=user->room;
/* ZMENA tu to zistuje */
if ((teleport<2 || (teleport==4)) && (rm->access==GOD_PRIVATE)) {
      write_user(user,"Prepac - GOD chce mat sukromie - tam sa nedostanes.\n");
      return;
      }
if ((teleport<2 || (teleport==4)) && !has_room_access(user,rm)) {
      write_user(user,"Ta miestnost je prave PRIVATNA, nemozes vstupit.\n");
      return;
      }
                   
/* Reset invite room if in it */
if (user->invite_room==rm) user->invite_room=NULL;

if (teleport==0 || teleport==1) {
  result=move_vehicle(user,rm,0);
  if (result==0) {
    sprintf(text,"~FG%s nie je miesto, na %s sa tam nedostanes, musis zosadnut.\n",rm->where,predmet[get_vehicle(user)]->inytiv);
    text[3]=toupper(text[3]);
    write_user(user,text);
    return;
   }
  else if (result==1) goto SKIP;
 }

if (!user->vis) {
  wrtype=WR_IOPHR;
  write_room(rm,invisenter(user));
  write_room_except(user->room,invisleave(user),user);
  wrtype=0;
  goto SKIP;
 }
if (teleport==3) goto SKIP;

/* ()STROV */
/*if (teleport==0 && !strcmp(rm->name,SHIPPING_SHIP) && (strcmp(user->rihand,"*") || strcmp(user->lehand,"*")))
    {
    write_user(user,"Stoj! Nevies, ze pasovat predmety na lod sa nesmie!?\n");
    return;
    }*/

if (teleport==1) {
      sprintf(text,"~FT%s %s z magickeho trblietaveho oblaku ~OL~FB(Puff!)\n",user->name,pohl(user,"vystupil","vystupila"));
      if (user->vis) write_room(rm,text);
      sprintf(text,"~OL~FB(Puff!) ~RS~FT%s %s v trblietavom oblaku!\n",user->name,pohl(user,"zamrmlal kuzlo a zmizol","zamrmlala kuzlo a zmizla"));
      if (user->vis) write_room_except(old_room,text,user);
      goto SKIP;
      }
      
if (teleport==4) {
      sprintf(text,"%s sem %s na kridlach!\n",user->name,pohl(user,"priletel","priletela"));
      if (user->vis) write_room(rm,text);      
      sprintf(text,"%s s pomocou kridel %s %s!\n",user->name,pohl(user,"odletel","odletela"), rm->into);
      if (user->vis) write_room_except(old_room,text,user);
      goto SKIP;
      }

if (teleport==5) { /* FLYER */
      sprintf(text,"%s %s z letuna...\n",user->name,pohl(user,"vystupil","vystupila"));
      if (user->vis) write_room(rm,text);
      goto SKIP;
      }
                  
if (teleport==2) {
      write_user(user,"~OL~FTHromovy hlas vyslovil teleportovacie kuzlo a Tvoje telo zacalo miznut!\n");
      sprintf(text,"~FT%s sa vynara z trblietaveho oblaku ... ~OL~FB(Puff!)\n",user->name);
      if (user->vis) write_room(rm,text);
      sprintf(text,"~FTPocujes teleportovacie kuzlo, a %s zrazu mizne v oblaku ... ~OL~FB(Puff!)\n",user->name);
      if (user->vis) write_room_except(old_room,text,user);            
      goto SKIP;
      }
if (teleport==6) {
      sprintf(text,"%s %s na navstevu.\n",user->name,pohl(user,"prisiel","prisla"));
      if (user->vis) write_room(rm,text);
      sprintf(text,"-> %s odisiel na navstevu do zalara.\n",user->name);
      if (user->vis) write_room_except(old_room,text,user);
      goto SKIP;
      }
wrtype=WR_IOPHR;
user->commused[8]++;
sprintf(text,"<- %s %s.\n",user->name,user->in_phrase);
if (!rm->sndproof && user->vis) write_room(rm,text);
sprintf(text,"-> %s %s.\n",user->name,expand_outphr(user,rm));
if (!user->room->sndproof && user->vis) write_room_except(user->room,text,user);
wrtype=0;

SKIP:
if (teleport!=0 && teleport!=1) move_vehicle(user,rm,1);

/*if ((rm->group==2 && old_room->group!=2) || !strcmp(rm->name,"plachetnica")) {
	strcpy(user->rihand,"*");
	strcpy(user->lehand,"*");
	}*/
if (teleport!=5 && old_room==get_room("letun",NULL)) eject(user);
if (!strcmp(rm->name,"zalar") && visit<1) visit=180;
user->room=rm;
look(user);
reset_access(old_room);
if (user->statline==CHARMODE) show_statline(user);
}

/*** Switch ignoring all on and off ***/
void toggle_ignall(user)
UR_OBJECT user;
{
if (!user->ignall) {
      write_user(user,"Odteraz vsetkych ignorujes.\n");
      sprintf(text,"%s vas odteraz vsetkych ignoruje.\n",user->name);
      write_room_except(user->room,text,user);
      user->ignall=1;
      return;
      }
sprintf(text,"%s si ignorovat, zase budes vsetko pocut.\n", pohl(user,"Prestal","Prestala"));
write_user(user,text);
sprintf(text,"%s vas %s ignorovat.\n",user->name, pohl(user,"prestal","prestala"));
write_room_except(user->room,text,user);
user->ignall=0;
}

/*** check_ignore_user() - zisti, ci nas target ignoruje
 vrati 0 ak nie, 1 ak ano */

int check_ignore_user(user,target)
UR_OBJECT user,target;
{
NOTIFY bunka;

if (user->level>=KIN && target->level<GOD) return 0;
 /* zakotvene pre vsetkyx KING a vyssie + god moze ignorovat kinga */
for (bunka=target->ignorelist; bunka!=NULL; bunka=bunka->next) {  
  if (!strcasecmp(bunka->meno,user->name)) return 1;
 }
return 0;
}

/*** Ignore user -> kopletne skopirovane notify - cil je to vsetko DYNAMICKY ;)) (V) ***/
void ignore_user(user)
UR_OBJECT user;
{
UR_OBJECT u;
int n,/* neni, */cnt;
/* char filename[200]; */
char tmptxt[200];
/* FILE *fp; */
int je_nahlaseny=0, existuje=0;
NOTIFY bunka;
/* neni=0; */

if (word_count<3) {

      if (!user->ignorelist) {
      		write_user(user,"V tomto momente nikoho neignorujes.\n"); 
      		write_user(user,"Pouzi: .ignore user <meno>\n");
      		return; 
      		}
      write_user(user,"Momentalne ignorujes tychto uzivatelov:\n");
      strcpy(text,"~OL~FR+--------------+");
      cnt=0;
      for (bunka=user->ignorelist; (bunka!=NULL && cnt<5); bunka=bunka->next) {
      	cnt++;
      	if (cnt>1) strcat(text,"--------------+");
      	}
      strcat(text,"\n");
      write_user(user,text);
      cnt=0;
      strcpy(text,"");
      for(bunka=user->ignorelist; bunka!=NULL; bunka=bunka->next) { 
        cnt++;
        sprintf(tmptxt,"~OL~FR| ~RS~FW%-12s ", bunka->meno);
        strcat(text,tmptxt);
        if (cnt==5) {
          strcat(text,"~OL~FR|\n~OL~FR+--------------+--------------+--------------+--------------+--------------+\n");
       	  write_user(user,text);
      	  cnt=0;
      	  strcpy(text,"");
         }
       }
      if (cnt) strcat(text,"~OL~FR|\n~OL~FR");
      for (n=0; n<cnt; n++) strcat(text,"+--------------");      	
      if (cnt) strcat(text,"+\n");
      write_user(user, text);
      write_user(user,"Ak chces niekoho prestat/zacat ignorovat, napis .ignore user <meno>\n");
      return;      
     }
if (!(u=get_user_exact(word[2]))) je_nahlaseny=0;
	else je_nahlaseny=1;
if (je_nahlaseny && u==user) {
	write_user(user,"Ignoracia sameho seba je dvadsiatym priznakom sialenstva!\n");
	return;
	}
		
/*
sprintf(filename,"%s/%s.D",USERFILES,word[2]);
if (!(fp=ropen(filename,"r"))) {
	existuje=0;	
      	}
      	else {
	  existuje=1;
	  fclose(fp);
	 }
*/
existuje=0;
strcpy(tmptxt,db_user_name(word[2])); /* MySQL */
if (tmptxt[0]) { existuje=1; strcpy(word[2],tmptxt); }

if (!je_nahlaseny && !existuje) {
	if ((u=get_user(word[2]))) je_nahlaseny=1;	
	}

if (!(delete_notify(&(user->ignorelist), word[2]))) {
	if (!existuje) {
		write_user(user, nosuchuser);
		return;
		}
	add_notify(&(user->ignorelist), word[2]);

        sprintf(query,"replace into `ignuser` (`userid`,`victim`) values('%d','%s');",user->id,word[2]);
        mysql_kvery(query);
	
	if (je_nahlaseny)
	 sprintf(text,"%s si ignorovat ~OL~FR%s~RS.\n",pohl(user,"Zacal","Zacala"),sklonuj(u,4));
	else
	 sprintf(text,"%s si do svojho ignore-listu meno: ~OL~FR%s~RS.\n",pohl(user,"Pridal","Pridala"),word[2]);
	write_user(user,text);
	if (je_nahlaseny) {
		sprintf(text,"~OL~FR%s~RS~FW ta %s ignorovat!\n",user->name,pohl(user,"zacal","zacala")); /*SEMSEM*/
		if ((!u->ignall) && (!check_ignore_user(user,u))) write_user(u,text);	
		}
	return;
	}   

sprintf(query,"delete from `ignuser` where `userid`='%d' and `victim`='%s';",user->id,word[2]);
mysql_kvery(query);

if (je_nahlaseny)
 sprintf(text,"%s si ignorovat ~OL~FR%s~RS.\n",pohl(user,"Prestal","Prestala"),sklonuj(u,4));
else
 sprintf(text,"%s si zo svojho ignore-listu meno: ~OL~FR%s~RS.\n",pohl(user,"Vyradil","Vyradila"),word[2]);
write_user(user,text);
if (je_nahlaseny) {
	sprintf(text,"~OL~FR%s~RS ta prestal%s ignorovat.\n",user->name,pohl(user,"","a"));
	if ((!u->ignall) && (!check_ignore_user(user,u))) write_user(u,text);
	}
return;
}


/*** NOTIFY - kompletne prerobene - cil je to vsetko DYNAMICKY (S) ***/
void notify_user(user)
UR_OBJECT user;
{
UR_OBJECT u;
int n/* ,neni */,cnt;
/* char filename[200]; */
char tmptxt[200];
/* FILE *fp; */
int je_nahlaseny=0, existuje=0;
NOTIFY bunka;
/* neni=0; */

if ((word_count==2) && (!strcmp(word[1],"who"))) {
	who_from_notify_is_online(user);
	return;
	}

/* new style notify, Q}--->-->---!!! */

if (word_count<2) {

      if (!user->notifylist) {
      		write_user(user,"Nemas nikoho v notify-zozname.\n"); 
      		write_user(user,"Pouzi: .notify <meno>\n");
      		return; 
      		}
      write_user(user,"V notify-zozname mas tychto uzivatelov:\n");
      strcpy(text,"~OL~FB+--------------+");
      cnt=0;
      for (bunka=user->notifylist; (bunka!=NULL && cnt<5); bunka=bunka->next) {
      	cnt++;
      	if (cnt>1) strcat(text,"--------------+");
      	}
      strcat(text,"\n");
      write_user(user,text);
      cnt=0;
      strcpy(text,"");
      for(bunka=user->notifylist; bunka!=NULL; bunka=bunka->next) { 
      cnt++;     
      if ((u=get_user_exact(bunka->meno))!=NULL && (user->level>=u->level || u->vis)) {
      		sprintf(tmptxt,"~OL~FB|~FR*~FY%-12s ", bunka->meno);
         	strcat(text,tmptxt);
         	}
         	else {        
         	sprintf(tmptxt,"~OL~FB| ~RS~FW%-12s ", bunka->meno);
         	strcat(text,tmptxt);
         	}
         
        if (cnt==5) {
	      	strcat(text,"~OL~FB|\n~OL~FB+--------------+--------------+--------------+--------------+--------------+\n");
	      	write_user(user,text);
	      	cnt=0;
	      	strcpy(text,"");
      	}
      }
      if (cnt) strcat(text,"~OL~FB|\n~OL~FB");
      for (n=0; n<cnt; n++) {
      	strcat(text,"+--------------");      	
      	}
      if (cnt) strcat(text,"+\n");
      write_user(user, text);
      write_user(user,"Ak chces niekoho pridat/vyradit z tohoto zoznamu, napis .notify <meno>\n");
      sprintf(text,"Momentalne mas %d %s v notify zozname.\n",user->notify_num,skloncislo(user->notify_num,"cloveka","ludi","ludi"));
      write_user(user,text);
      return;      
      }


if (!(u=get_user_exact(word[1]))) je_nahlaseny=0;
	else je_nahlaseny=1;
if (je_nahlaseny && u==user) {
	write_user(user,"Nemozes zaradit do zoznamu seba!\n");
	return;
	}
/*		
sprintf(filename,"%s/%s.D",USERFILES,word[1]);
if (!(fp=ropen(filename,"r"))) {
	existuje=0;	
      	}
      	else {
	      	existuje=1;
		fclose(fp);
		}
*/
existuje=0;
strcpy(tmptxt,db_user_name(word[1])); /* MySQL */
if (tmptxt[0]) { existuje=1; strcpy(word[1],tmptxt); }
		
if (!je_nahlaseny && !existuje) {
	if ((u=get_user(word[1]))) je_nahlaseny=1;	
	}

/* Skusime vymazat. Ak sa podari, pohoda (mozem teda zmazat aj toho kto
   tu uz nema konto). Ak sa nepodari zmazat, znamena to ze ho nemam v
   zozname, teda ho xcem pridat ... A teda az tu ohlasim xybu ked user
   nema konto, lebo takeho nemozem pridat ...*/
   
if (!(delete_notify(&(user->notifylist), word[1]))) {
	if (!existuje) {
		write_user(user, nosuchuser);
		return;
		}
	add_notify(&(user->notifylist), word[1]);
	
        sprintf(query,"replace into `notify` (`userid`,`victim`) values('%d','%s');",user->id,word[1]);
        mysql_kvery(query);
        
	sprintf(text,"%s si do svojho notify-listu meno: ~OL%s~RS.\n",pohl(user,"Pridal","Pridala"),word[1]);
	write_user(user,text);
        user->notify_num++;
	if (je_nahlaseny) {
	  sprintf(text,"~OL%s~RS si ta %s do svojho notify-listu!\n",user->name,pohl(user,"pridal","pridala")); /*SEMSEM*/
	  if ((!u->ignall) && (!check_ignore_user(user,u))) write_user(u,text);	
	 }
	return;
	}   

/* ak to doslo sem, snad sa podarilo vymazat usera -> teda to oznamime .. */

sprintf(query,"delete from `notify` where `userid`='%d' and `victim`='%s';",user->id,word[1]);
mysql_kvery(query);

sprintf(text,"%s si zo svojho notify-listu meno: ~OL%s~RS.\n",pohl(user,"Vyradil","Vyradila"),word[1]);
write_user(user,text);
if (user->notify_num>0) user->notify_num--;
if (je_nahlaseny) {
	sprintf(text,"~OL%s~RS ta %s zo svojho notify listu.\n",user->name,pohl(user,"vyradil","vyradila"));
	if ((!u->ignall) && (!check_ignore_user(user,u))) write_user(u,text);
	}

/* Kedze notify-list je dynamicky a teda je "nekonecny", hlaska o naplneni
   notify listu je zbytocna :> */

/* hmm, uz sa nasiel exot co si zaradil komplet vsetkych userov atlantisu ? (V) */

return;
}

/* (S) vypise kto_z_notify_je_prihlaseny :))) */

void who_from_notify_is_online(user)
UR_OBJECT user;
{
UR_OBJECT u;
int pocet,h;
char temp[500];
NOTIFY bunka;

pocet=0;
for (bunka=user->notifylist; bunka!=NULL; bunka=bunka->next) {
	if ((u=get_user_exact(bunka->meno))!=NULL && (user->level>=u->level || u->vis)) {
		pocet++;
		if (pocet==1) {
			write_user(user,"~FY-=-o-=-o-=-o-=-o- ~OL~FRPrihlaseni uzivatelia z tvojho notify-listu ~RS~FY-o-=-o-=-o-=-o-=-\n\n");
			}
		sprintf(temp,"~FT%s ~FW%s~RS~FW",u->name,u->desc);
		h=(40+(colour_com_count(temp)));
		sprintf(text,"     %-*.*s ~RS~OL~FW%3d/%-3d min. ~FR[%3.3s] ~RS~FG(%s)", h, h, temp, (int)(time(0)-u->last_login)/60,(int)(time(0)-u->last_input)/60,u->room->name, u->cloak?level_name[u->cloak]:level_name[u->level]);
		if (u->afk) strcat(text," ~OL~FWAFK");
		strcat(text,"\n");
		write_user(user,text);
		}
	}
if (pocet) write_user(user,"\n~FY-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-o-=-\n");
if (!pocet) {
	write_user(user,"Nikto z tvojho notify-listu nieje prihlaseny!\n");
	return;
	}
return;
}


/*** check_notify_user() - zisti, ci sa target nachadza v
     notify_liste -> 0 ak nie, 1 ak ano */

/* ZMENA - dynamicke */

int check_notify_user(user,target) /* user==zdroj target==cielovy user */
UR_OBJECT user,target;
{
NOTIFY bunka;

for (bunka=user->notifylist; bunka!=NULL; bunka=bunka->next) {
  if (!strcasecmp(bunka->meno,target->name)) return 1;
 }  
return 0;
}


/* A CIL NOVY TOGGLE PROMPT */
/*** Switch prompt on and off 
    Zmena Spakky - podporuje viac typov promptov 
    Zmena Buko - prehladny vypis dostupnych promptov + pridane 
    Zmena Rider - uplne prepisane na volitelny prompt [cha, ze on and off!] :)
    ***/ 
 
void toggle_prompt(user, inpstr) /* PROMMMPT */
UR_OBJECT user; 
char *inpstr; 
{ 

if (strlen(inpstr) > PROMPT_LEN-10) {
    write_user(user,"Prompt je prilis dlhy!\n"); 
    return;
   }

if (word_count==1) { 
        write_user(user,"Pouzitie: .prompt <string| show | 0..7>\n");
	write_user(user,"\nPouzitelne premenne:\n"); 
	write_user(user,"none - vypne prompt\n"); 
	write_user(user,"show - vypise aktualne nastavenie\n"); 
	write_user(user,"$N - Name - Meno uzivatela\n"); 
	write_user(user,"$R - Room - Miestost\n"); 
	write_user(user,"$T - Time - Cas (hodina a minuta)\n"); 
	write_user(user,"$S - Session time - Dlzka prihlasenia (hodin a minut)\n");
	write_user(user,"$A - Alarm - Zvysny cas do spustenia alarmu\n");
	write_user(user,"$F - Follow - Osoba, ktoru nasledujes\n"); 
	write_user(user,"$L - Last tell - Osoba, ktora ti posledne tellovala\n"); 
	write_user(user,"$P - Power Points - Pocet tvojich Power Pointov\n"); 
        write_user(user,"$M - Mana - Magicka energia\n");
        write_user(user,"$E - Novy riadok (max 1 ks, vhodne pouzit na konci promptu)\n");
        write_user(user,"V promptoch sa mozu pouziva standardne farebne kody NUTS\n");
        write_user(user,"Priklad: '.prompt ~~FT~~OL$N v Atlantide uz $S> ' vyzera asi takto:\n");
        write_user(user,"~FT~OLMojemeno v Atlantide uz 01:32> \n");
	write_user(user,"Mozno vyuzit aj vopred definovane prompty (.prompt 1 az 7)\n");
        if (user->prompt) {
	   sprintf(text,"\nAktualne nastavenie: %s~RS~BK~FW\nMomentalny prompt: %s\n",colour_code_show(user->prompt_string),user->prompt_string);
	   write_user(user,text); 
          }
	return; 
	}

if ((word_count==2) && (atoi(word[1])!=0))
        {
	switch (atoi(word[1]))
	  {
 	    case 1:	strcpy(user->prompt_string,"~FT>~FW ");
 	    		write_user(user,"Prompt nastaveny na: ~FT>~FW \n");
 	    		break;

 	    case 2:     strcpy(user->prompt_string,"~FT~OL$N>~FW ");
			write_user(user,"Prompt nastaveny na: ~OL~FTMeno>~FW \n"); 
			break;

	    case 3:	strcpy(user->prompt_string,"~OL~FT$N~FW:~FR$R~RS~FT>~FW ");
	    		write_user(user,"Prompt nastaveny na: ~OL~FTMeno~FW:~FRMeno_miestnosti~RS~FT>~FW\n"); 
			break;
			
	    case 4:	strcpy(user->prompt_string,"~OL~FT$N ~FG$T ~FR$R~RS~FT>~FW ");
			write_user(user,"Prompt nastaveny na: ~OL~FTMeno ~FGHod:min ~FRMeno_miestnosti~RS~FT>~FW\n"); 
			break;
			
	    case 5:	strcpy(user->prompt_string,"~FW-~OL~FY$S~RS~FW-~OL~FY$R~RS~FW-> ");
			write_user(user,"Prompt nastaveny na: ~FW-~OL~FYHod_prihlas.:Min.prihlas.~RS~FW-~OL~FYMeno_miestnosti~RS~FW->\n"); 
			break;
			
	    case 6:	strcpy(user->prompt_string,"~OL~FG| ~RS~FTF: ~OL$F ~RS~FTT: ~OL$L ~FG|>~RS~FW ");
	 		write_user(user,"Prompt nastaveny na: ~OL~FG| ~RS~FTF: ~OLSledovany_uzivatel~RS~FTT: ~OLPosledny_sepot ~FG|>~RS~FW\n"); 
			break;
			
	    case 7:	strcpy(user->prompt_string,"~OL~FT$N~FW:~FR$R~RS/~OL~FM$P~RS~FT>~FW ");
			write_user(user,"Prompt nastaveny na: ~OL~FTMeno~FW:~FRMeno_miestnosti~RS/~OL~FMEnergia~RS~FT>~FW\n"); 
			break;

	    default:	write_user(user,"~FRPredvoleny prompt musi byt z intervalu <1,7>!\n");
	                return;
	                break;
	  }
	user->prompt=1;  
        return;
        }

if (!strcmp(word[1],"none") || !strcmp(word[1],"0")) { 
      write_user(user,"Prompt ~FRVYPNUTY~RS\n"); 
      user->prompt=0;
      return; 
      }
if (!strcmp(word[1],"show")) { 
      sprintf(text,"Tvoj prompt je: %s~RS~FW~BK\n",colour_code_show(user->prompt_string)); 
      write_user(user,text); 
      return; 
     }
strcpy(user->prompt_string,inpstr);
sprintf(text,"Prompt nastaveny na: '%s~RS~FW~BK'\n", user->prompt_string); 
user->prompt=1;
write_user(user,text); 
} 

 
/*** Set user description ***/
void set_desc(user,inpstr)
UR_OBJECT user;
char *inpstr;
{

if (word_count<2) {
      sprintf(text,"Aktualne nastavenie: %s\nTvoj momentalny desc: %s %s\n",colour_code_show(user->desc),user->name,user->desc);
      write_user(user,text);
      return;
      }
if (strstr(word[1],"(CLONE)") || strstr(inpstr,"nema nastaveny")) {
      write_user(user,"Nemozes mat takyto desc.\n");  return;
      }
 
if (user->muzzled) { /* Zmena S. - toto uz bolo treba :) */ 
	write_user(user,"Si umlcany, ziadna komunikacia cez .desc!\n"); 
	return; 
	} 
 
/* ZMENA -  Spakky - Toto je zmena v prikaze set_desc(). Nieje dana pevna dlzka
   desc, ale zalezi od dlzky mena. Spolu nesmu mat viac ako 39 znakov.
   To zabezpeci ze vypis .who nebude vselijaky "pokriveny". */

if (((strlen(inpstr)+strlen(user->name)-(colour_com_count(inpstr)))+1>39) || (strlen(inpstr)>80)) {
      write_user(user,"Prilis dlhy popis.\n");  return;
      }
      
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit si skaredy desc ;-)!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }
strcpy(user->desc,inpstr);
sprintf(text,"Tvoj popis bol nastaveny: %s %s\n", user->name, user->desc);
write_user(user,text);
}

int check_iophrase(char *fraza)
{
char temp[PHRASE_LEN+1];

sstrncpy(temp, fraza, PHRASE_LEN);
strtolower(temp);

if (strstr(temp,"povedal") || strstr(temp,"zakrical")) return 1;

return 0;
}



/*** Set in and out phrases ***/
void set_iophrase(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
if (strlen(inpstr)>PHRASE_LEN) {
      write_user(user,"Prilis dlha fraza.\n");  return;
      }
      
if (check_iophrase(inpstr)) {
	write_user(user,"Nemozes mat takuto frazu.\n"); return;
	}

  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit si skaredu in/out phrase!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }         
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }         
if (com_num==INPHRASE) {
      if (word_count<2) {
            sprintf(text,"Nastavi spravu pri prichode do miestnosti. Momentalne sa zobrazi:\n<- %s %s.\n",user->name,user->in_phrase);
            write_user(user,text);
            return;
            }
      strcpy(user->in_phrase,inpstr);
      write_user(user,"Vstupna fraza bola nastavena.\n");
      return;
      }
if (word_count<2) {
      sprintf(text,"Nastavi spravu pri odchode miestnosti. Mozes pouzit premenne:\n $0 - do miestnosti (napr: na namestie), $1 - v miestnosti (na namesti),\n $2 - miestnost (namestie)\nPriklad: '.outphr uz je $1 :)' zobrazi: -> Xyz uz je na namesti :).\nMomentalne sa zobrazi:\n-> %s %s.\n",user->name,expand_outphr(user,NULL));
      write_user(user,text);
      return;
      }
strcpy(user->out_phrase,inpstr);
write_user(user,"Vystupna fraza bola nastavena.\n");
}

char *expand_outphr(UR_OBJECT user,RM_OBJECT destination)
{
 static char retphr[100],*tmp;
 unsigned int i,cnt;
 
 if (destination==NULL) destination=user->room;
 if (destination==NULL) destination=get_room("namestie",NULL);
 i=0;cnt=0;
 while (i<strlen(user->out_phrase)-1) {
   if (user->out_phrase[i]=='$' && (user->out_phrase[i+1]=='0'
   || user->out_phrase[i+1]=='1' || user->out_phrase[i+1]=='2')) { 
     tmp=user->out_phrase;
     if (i>0) { 
       strncpy(retphr,user->out_phrase,i);
       retphr[i]='\0';
       if (user->out_phrase[i-1]!=' ') strcat(retphr," ");
      }
     else strcpy(retphr,"");
     switch (user->out_phrase[i+1]) {
       case '0':strcat(retphr,destination->into); break;
       case '1':strcat(retphr,destination->where); break;
       case '2':strcat(retphr,destination->name);
      }
     if (strlen(user->out_phrase)>i+2) {
       if (user->out_phrase[i+2]!=' ') strcat(retphr," ");
       tmp=tmp+(i+2);
       strcat(retphr,tmp);
      }
     cnt=1;
     break;
    }
   i++;
  }
 if (cnt==0) { 
   strcpy(retphr,user->out_phrase);
   strcat(retphr," ");
   strcat(retphr,destination->into);
  }
 return retphr;
}

/*** Privatizacia ***/
void set_room_access(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;
int cnt;

rm=user->room;
if (word_count<2) rm=user->room;
else {
      if (user->level<gatecrash_level) {
            write_user(user,"Nemas dostatocne vysoky level.\n");
            return;
	    }
      if ((rm=get_room(word[1],user))==NULL) {
            write_user(user,nosuchroom);  return;
            }
      }
if (user->vis) name=user->name; else name=invisname(user);
if ((rm->access>PRIVATE) && (com_num!=GODPRIV)) {
      if (rm==user->room)
            write_user(user,"Pristup do miestnosti je fixovany. Ak mozes, odfixuj ho.\n");
      else write_user(user,"Pristup do tej miestnosti je fixovany. Ak mozes, odfixuj ho.\n");
      return;
      }
if (com_num==PUBCOM && rm->access==PUBLIC) {
      if (rm==user->room)
            write_user(user,"Tato miestnost uz je verejna.\n");
      else write_user(user,"Ta miestnost uz je verejna.\n");
      return;
      }
if (user->vis) name=user->name; else name=invisname(user);
/* ZMENA - God nastavuje private a nikto mu tam nevlezie, ani iny GOD! */
if ((com_num==GODPRIV) && (rm->access!=GOD_PRIVATE)) {
      if (rm!=user->room) {
      	write_user(user,"Nemozes na dialku nastavovat BOZSKU miestnost.\n");
      	return;
      	}
      sprintf(text,"Nastavil si miestnost na ~OLBOZSKU~RS.\n");
      write_user(user,text);
      write_room_except(rm,"Miestnost bola nastavena na \"BOZSKU\" - maximalne sukromie.\n",user);
      rm->access=GOD_PRIVATE;
      return;
      }
      
if (com_num==SNDPROOF) {
	if (rm->sndproof) {
		write_user(user,"Zvukotesnost miestnosti ~FRZRUSENA.\n");
		sprintf(text,"~FT%s ~FR%s ~FTzvukotesnost miestnosti.\n",user->name,pohl(user,"zrusil","zrusila"));
		write_room_except(rm,text,user);
		rm->sndproof=0;
		return;
		}
	write_user(user,"Tato miestnost je odteraz ~FGzvukotesna.\n");
	sprintf(text,"~FT%s zvukovo %s miestnost.\n",user->name,pohl(user,"utesnil","utesnila"));
	write_room_except(rm,text,user);
	rm->sndproof=1;
	return;	
	}      
      
if (com_num==PRIVCOM) {
      if (rm->access==PRIVATE || rm->access==GOD_PRIVATE) {
            if (rm==user->room)
                  write_user(user,"Miestnost, v ktorej si je uz PRIVATNA.\n");
            else write_user(user,"Ta miestnost uz je PRIVATNA.\n");
            return;
            }
      cnt=0;
      for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
      if (cnt<min_private_users && user->level<ignore_mp_level) {
	    sprintf(text,"V miestnosti musia byt aspon %d uzivatelia/klony!\n",min_private_users);
            write_user(user,text);
            return;
            }
      if (user->pp<PRIVPUBCAN) {
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp,PRIVPUBCAN);
	write_user(user,text);
	return;
       }
      decrease_pp(user,PRIVPUBDEC,NODYNAMIC);
      
      write_user(user,"Miestnost nastavena na ~FRPRIVATNA.~RS\n");
      if (rm==user->room) {
            sprintf(text,"%s %s miestnost na ~FRPRIVATNU.~RS\n",name, pohl(user,"nastavil","nastavila"));
            write_room_except(rm,text,user);
            }
      else write_room(rm,"Miestnost bola nastavena na ~FRPRIVATNU.~RS\n");
      rm->access=PRIVATE;
      return;
      }
/* ZMENA - vracia spet na public */
/* z GODPRIV moze vratit len ten god co je v tej miestnosti
   pomocou prikazu .god                                     */
if (com_num!=GODPRIV && rm->access==GOD_PRIVATE) {
      write_user(user,"Prepac - miestnost bude dovtedy BOZSKA, dokial bude dotycny GOD v nej.\n");
      return;
      }
if (com_num==GODPRIV && rm->access==GOD_PRIVATE) {
      if (rm!=user->room) {
      	write_user(user,"Ak chces zrusit BOZSKY pristup, musis byt v tej miestnosti!\n");
      	return;
      	}
      sprintf(text,"Zrusil si ~OLBOZSKY~RS pristup.\n");
      write_user(user,text);
      }
if (user->pp<PRIVPUBCAN) {
  sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp,PRIVPUBCAN);
  write_user(user,text);
  return;
 }
decrease_pp(user,PRIVPUBDEC,NODYNAMIC);

write_user(user,"Miestnost bola nastavena na ~FGVEREJNU.\n");
if (rm==user->room) {
      sprintf(text,"%s %s miestnost na ~FGVEREJNU.\n",name, pohl(user,"nastavil","nastavila"));
      write_room_except(rm,text,user);
      }
else write_room(rm,"Miestnost bola nastavena na ~FGVEREJNU.\n");
rm->access=PUBLIC;
rm->lemmein[0]='\0';
rm->countdown=0;

/* Reset any invites into the room & clear review buffer */
for(u=user_first;u!=NULL;u=u->next) {
      if (u->invite_room==rm) u->invite_room=NULL;
      }
clear_revbuff(rm);
}


/*** Ask to be let into a private room ***/
void letmein(user)
UR_OBJECT user;
{
RM_OBJECT rm;

if (word_count<2) {
      write_user(user,"Kam chces byt vpusteny?\n");  return;
      }
if ((rm=get_room(word[1],user))==NULL) {
      write_user(user,nosuchroom);  return;
      }
if (rm==user->room) {
      sprintf(text,"Ale ved uz si v miestnosti %s!\n",rm->name);
      write_user(user,text);
      return;
      }
/*for(i=0;i<MAX_LINKS;++i)
      if (user->room->link[i]==rm) goto GOT_IT;
sprintf(text,"Do miestnosti %s sa bude dat dostat jedine na kridlach!\n",rm->name);
write_user(user,text);
return;

GOT_IT:
*/
if (!(rm->access & PRIVATE) && rm->access!=GOD_PRIVATE) {
      sprintf(text,"Miestnost %s je verejna.\n",rm->name);
      write_user(user,text);
      return;
      }
/* mala ZMENA */
if (!strcmp(rm->lemmein,user->name)) { /* tha need for antiflood (V) */
  sprintf(text,"Uz si sa pytal%s ci ta pustia %s. Ak budu chciet, pustia ta.\n",pohl(user,"","a"),rm->into);
  write_user(user,text);
  return;
 }
strcpy(rm->lemmein,user->name);
rm->countdown=5; /* po 5 min. sa to resetne, takze moze otravovat znova (V) */
sprintf(text,"Slusne si sa opytal%s, ci ta pustia %s.\n",pohl(user,"","a"),rm->into);
write_user(user,text);
sprintf(text,"%s sa opytal%s pustia %s\n",user->name,pohl(user," tych vedla, ci ho","a tych vedla, ci ju"),rm->into);
write_room_except(user->room,text,user);
sprintf(text,"%s sa vas pyta, ci %s pozvete medzi seba.\n",user->name,pohl(user,"ho","ju"));
write_room(rm,text);
}

/*** Invite a user into a private room ***/
void invite(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;

if (word_count<2) {
      write_user(user,"Koho pozvat?\n");  return;
      }
rm=user->room;
if (!(rm->access & PRIVATE)) {
      write_user(user,"Tato miestnost je verejna.\n");
      return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Pozyvanie sameho seba niekam je treti priznak sialenstva.\n");
      return;
      }
if (u->room==rm) {
      sprintf(text,"%s tu uz je!\n",u->name);
      write_user(user,text);
      return;
      }
if (user->jailed && visit<0) {
      sprintf(text,"Bohuzial, momentalne nie su navstevne hodiny.\n");
      write_user(user,text);
      return;
      }
if (u->invite_room==rm) {
      sprintf(text,"%s sem uz %s.\n",u->name, pohl(u,"bol pozvany","bola pozvana"));
      write_user(user,text);
      return;
      }
sprintf(text,"%s si sem %s.\n",pohl(user,"Pozval","Pozvala"),sklonuj(u,4));
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"%s sem %s %s.\n",name,pohl(user,"pozval","pozvala"),sklonuj(u,4));
write_room_except(user->room,text,user);
sprintf(text,"%s Ta %s %s.\n",name,pohl(user,"pozval","pozvala"),rm->into);
write_user(u,text);
u->invite_room=rm;
}

/*** Set the room topic ***/
void set_topic(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int duplicate=0,usertopics=0;
RM_OBJECT rm;
char *name;

rm=user->room;
if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nastavovat temu.\n");  return;
      }
if (word_count<2) {
      if (!strlen(user->room->topic)) {
            write_user(user,"Momentalne nie je nastavena ziadna tema.\n");  return;
            }
      sprintf(text,"Aktualna tema je: %s\n",user->room->topic);
      write_user(user,text);
      if (!strcmp(user->room->topicmaker,user->name))
        write_user(user,"Temu mozes zmazat prikazom '.topic cancel'.\n");
      if (user->level>=KIN && user->room->topicmaker[0]) {
        sprintf(text,"~FTTemu nastavil(a): %s\n",user->room->topicmaker);
        write_user(user,text);
       }
      return;
      }
if (!strcmp(word[1],"cancel") || !strcmp(word[1],"zmaz")) {
  if (strcmp(user->room->topicmaker,user->name)) {
    sprintf(text,"Zmazat mozes len temu ktoru si nastavil%s.\n",pohl(user," sam","a sama"));
    write_user(user,text);
    return;
   }
  user->room->topic[0]='\0';
  user->room->topicmaker[0]='\0';
  sprintf(text,"Zrusil%s si topic %s.\n",pohl(user,"","a"),user->room->where);
  write_user(user,text);
  if (user->vis) name=user->name; else name=invisname(user);
  sprintf(text,"~FR%s~FW zrusil%s temu.\n",name,pohl(user,"","a"));
  write_room_except(user->room,text,user);
  return;
 }
if (user->pp < TOPICCAN) {    /*PP*/
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, TOPICCAN);
	write_user(user,text);
	return;
	}
if (strlen(inpstr)>TOPIC_LEN) {
      write_user(user,"Prilis dlha tema.\n");  return;
      }
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s nastavit skaredy topic v roome %s]~RS\n",user->name,pohl(user,"pokusil","pokusila"), user->room->name);
      writesys(WIZ,1,text,user);
      return;
      }         
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }         
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"%s ~RS~FW~OL[%s]",inpstr,name);
for(rm=room_first;rm!=NULL;rm=rm->next) {
  if (rm!=user->room && !strcmp(rm->topic,text)) duplicate++;
  if (!strcmp(rm->topicmaker,user->name)) usertopics++;
 }
if (usertopics>2) { 
  sprintf(text,"Uz si nastavil%s 3 temy, najprv nejaku zmaz (.topic cancel).\n",pohl(user,"","a"));
  write_user(user,text);
  return;
 }
strcpy(user->room->topic,text);
strcpy(user->room->topicmaker,user->name);
sprintf(text,"Tema nastavena na: %s\n",inpstr);
write_user(user,text);
sprintf(text,"~FR%s~FW %s temu na: %s\n",name, pohl(user,"nastavil","nastavila"), inpstr);
write_room_except(user->room,text,user);
if (duplicate) decrease_pp(user,TOPICDEC*(duplicate+1),NODYNAMIC);
else decrease_pp(user,TOPICDEC,NODYNAMIC);
}


/*** Wizard moves a user to another room ***/
void move(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;

if (word_count<2) {
      write_user(user,"Pouzi: .move <uzivatel> [miestnost]\n");  return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (word_count<3) rm=user->room;
else {
      if ((rm=get_room(word[2],user))==NULL) {
            write_user(user,nosuchroom);  return;
            }
      }
if (user==u) {
      write_user(user,"Snaha pohnut sameho seba tymto sposobom je stvrty priznak sialenstva.\n");  return;
      }
/* ZMENA moznost move aj rovnakych levelov!!!??? */
if (u->level>=user->level) {
      write_user(user,"Nemozes hybat s uzivatelmi, ktori maju rovnaky alebo vyssi level ako ty sam.\n");
      return;
      }
if (rm==u->room) {
      sprintf(text,"Ale %s predsa uz je v %s!\n",u->name,rm->name);
      write_user(user,text);
      return;
      };

if (u->room->group!=rm->group && rm!=user->room) { /* Koli ziskom z osobnej lodnej dopravy:> */
      write_user(user,"Je mi luto, ale aj tvoje magicke schopnosti maju iste hranice...\n");
      return;
    }

if ((u->room->access==GOD_PRIVATE) && (user->room!=u->room)) {
	sprintf(text,"%s je v BOZSKEJ miestnosti, nemozes s %s hybat pokial tam niesi.\n",u->name,pohl(u,"nim","nou"));
	write_user(user,text);
	return;	
	}

if ((rm->access==PRIVATE || rm->access==FIXED_PRIVATE) && (user->room!=rm)) {
	write_user(user,"Nemozes premiestnovat do PRIVATNEJ miestnosti, pokial v nej nie si.\n");
	return;
	}
	
if ((rm->access==GOD_PRIVATE) && (rm!=user->room)) {
	write_user(user,"Nemozes premiestnovat uzivatela do BOZSKEJ miestnosti pokial v nej nie si!\n");
	return;
	}
 
write_user(user,"~OL~FTPodarilo sa ti vyvolat kuzlo TELEPORT!\n");
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FT%s sa snazi vyvolat kuzlo TELEPORT ...\n",name);
write_room_except(user->room,text,user);
move_user(u,rm,2);
prompt(u);
}

/*** Broadcast an important message ***/
void bcast(UR_OBJECT user,char *inpstr,int done_editing)
{
char *c;
char *whichone;

if (done_editing) {
  query[0]='\0';
  c=user->malloc_start;
  while(c!=user->malloc_end) {
    if (*c=='\n') strcat(query,"\n~OL~FW");
    else chrcat(query,*c);
    ++c;
   }
  query[strlen(query)-7]='\0';
  whichone=query;
 }
else {
  if (user->muzzled) {
    write_user(user,"Si umlcany, nemozes pouzivat broadcast.\n");
    return;
   }
  if (word_count<2) {
    write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~FY Pisanie dolezitej spravy ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
    user->misc_op=11;
    editor(user,NULL);
    return;
   }
  else whichone=inpstr;
 }

force_listen=1;
if (user->vis)
      sprintf(text,"~LB\n~OL~FK--~RS~FW-=~OL~FW=(*~FY Dolezita sprava od %s ~OL~FW*)=~RS~FW=-~OL~FK--\n~OL~FW%s~RS~FW\n\n",sklonuj(user, 2),whichone);
else sprintf(text,"~LB\n~OL~FK--~RS~FW-=~OL~FW=(*~FY DOLEZITA SPRAVA ~OL~FW*)=~RS~FW=-~OL~FK--\n~OL~FW%s~RS~FW\n\n",whichone);
write_room(NULL,text);
if (user->vis)
 sprintf(text,"~OL~FK--~RS~FW-=~OL~FW=(* ~FW%s~FY  Dolezita sprava od %s ~OL~FW*)=~RS~FW=-~OL~FK--\n~OL~FW%s~RS~FW\n\n",long_date(3),sklonuj(user, 2),whichone);
else 
 sprintf(text,"~OL~FK--~RS~FW-=~OL~FW=(* ~FW%s~FY  DOLEZITA SPRAVA ~OL~FW*)=~RS~FW=-~OL~FK--\n~OL~FW%s~RS~FW\n\n",long_date(3),whichone);
record_bcast(text);

force_listen=0;
}

void show_timeouts(UR_OBJECT user)
{
UR_OBJECT u;
FILE *fp;
char filename[90];
int vyskyt[101],cnt,i;

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nepodarilo sa otvorit docasny subor pre zakladne .who!\n");
	return;
	}

fprintf(fp,"%s",title("~OL~FWTIMEOUTS","~OL~FR"));
cnt=0;
for (i=0;i<101;i++) vyskyt[i]=0;

for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE) continue;
  if (u->login) continue;
  if (u->hangups<1) continue;
  if (u->hangups>99) vyskyt[100]++;
  else vyskyt[u->hangups]++;
 }

cnt=0;
for (i=100;i>0;i--) if (vyskyt[i]>0) {
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->type==CLONE_TYPE) continue;
    if (u->login) continue;
    
    if ((i==100 && u->hangups>99) || u->hangups==i) {
      sprintf(text,"~RS%-12s:~OL%3d ~RS~FG%-22.22s ",u->name,u->hangups,u->site);
      if (cnt%2==1) strcat(text,"\n");
      fprintf(fp,"%s",text);
     }
   }
 }
fprintf(fp,"\n");
if (cnt%2==0) fprintf(fp,"\n");

fprintf(fp,"%s",title("~OL~FWTIMEOUTZ","~OL~FR"));
cnt=0;
for (i=0;i<101;i++) vyskyt[i]=0;

for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE) continue;
  if (u->login) continue;
  if (u->hangupz<1) continue;
  if (u->hangupz>99) vyskyt[100]++;
  else vyskyt[u->hangupz]++;
 }

cnt=0;
for (i=100;i>0;i--) if (vyskyt[i]>0) {
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->type==CLONE_TYPE) continue;
    if (u->login) continue;
    
    if ((i==100 && u->hangupz>99) || u->hangupz==i) {
      sprintf(text,"~FW~OL%-12s:~FR%3d ~RS~FG%-22.22s ",u->name,u->hangupz,u->site);
      if (cnt%2==1) strcat(text,"\n");
      fprintf(fp,"%s",text);
     }
   }
 }
if (cnt%2==0) fprintf(fp,"\n");

fclose(fp);
if (user->pagewho) {
  switch(more(user,user->socket,filename)) {
    case 0: write_user(user,"Chyba pri citani zoznamu timeoutov!\n");  break;
    case 1: user->misc_op=2;
   }
 }
else showfile(user,filename);
}

void customexamine(UR_OBJECT user,char *username,int testall)
{
FILE *fp;
char filename[81];
char line[1001];
int oflajn=0,reqlines,new_mail=0,err=0,repos=0,vec=-1;
UR_OBJECT u/* ,u2 */;
int align,type,pos,wizvar,disablelineifvarempty,nasiel=0;
unsigned int width,len,profile=100,i,ii;
char temp[500],var[500],yes[50],no[50],z;
int mins,idle=0,blik,cols,timelen,days2,hours2,mins2;
int holding[BODY];
char *str;



 if (word_count<2) {
   u=user;
   /* u2=u; */
  }
 else {
   if (user->level>=KIN && !strncmp(word[2],"repos",5)) {
     repos=1;
     nasiel=0;
    }
   else {
     vec=expand_predmet(word[1]);
     if ((u=get_user_exact(word[1]))!=NULL) {  
       nasiel=1;
       /* u2=u; */
      }
    }
   if (!nasiel) {
     if ((u=create_user())==NULL) {  
       sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);  
       write_user(user,text);  
       write_syslog("CHYBA: Nemozno vytvorit docasny user object v examine().\n",0);  
       return;  
      }
     sstrncpy(u->name,word[1],12);  
     if (db_load_user_details(u,repos)) {
       nasiel=1; 
       oflajn=1;
       /* u2=NULL; */
      }
     else {             
       destruct_user(u);  
       destructed=0;
      }             
    }     
   if (!nasiel && !repos) {
     if ((u=get_user(word[1]))!=NULL) { 
       nasiel=1;                      
       /* u2=u; */
      }      	
    }
      
   if (!nasiel) { 
     /* skusime predmet */
     if (!repos && vec>-1) {
       sprintf(filename,"predmety/%s",predmet[vec]->name);  
       if (!(fp=ropen(filename,"r"))) {  /*APPROVED*/
         if (!testall) write_user(user,"K tomuto predmetu niet opisu!\n");
	 return; 
       	}			
       else {  
         fgets(line,440,fp);  
         while(!feof(fp)) {  
           write_user(user,line);  
           fgets(line,440,fp);  
          }  
         fclose(fp);  
        }   	
       return;
      }	
     if (!testall) write_user(user,nosuchuser);
     return;
    }
  } /* koniec toho else :) */		                  

/*
if (word_count>1) {
  if ((u=get_user(word[1]))==NULL) {
    if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
      if (!testall) write_user(user,text);
      return;
     }
    sstrncpy(u->name,word[1],12);
    if (!load_user_details(u)) {
      destruct_user(u);
      destructed=0;
      if (!testall) write_user(user,"Taky uzivatel neexistuje.\n");
      return;
     }
    oflajn=1;
   }
  else oflajn=0;
 }
else {
  u=user;
  oflajn=0;
 }
*/
 if (word_count>2) {
   reqlines=atoi(word[2]);
   if (reqlines<0) profile=0;
   if (reqlines>100) profile=100;
   if (!strncmp(word[2],"no",2)) profile=0;
  }
 else profile=100;

 sprintf(query,"select max(UNIX_TIMESTAMP(time)) from mailbox where userid='%d'",u->id);
 new_mail=query_to_int(query);

 if (!oflajn) {
   mins=(int)(time(0) - u->last_login)/60;
   idle=(int)(time(0) - u->last_input)/60;
   for(i=0;i<BODY;i++) holding[i]=-1;
   cols=0;
   for(i=0;i<BODY;i++) if (u->predmet[i]>-1) { holding[cols]=u->predmet[i]; cols++; }
  }
 else 
   mins=u->last_login_len/60;
 timelen=(int)(time(0) - u->last_login);  
 days2=timelen/86400;
 hours2=(timelen%86400)/3600;
 mins2=(timelen%3600)/60;

/* profajl=NULL;
 if (profile) {
   sprintf(query,"select `profile` from `profiles` where `userid`='%d';",u->id);
   if ((result=mysql_result(query))) {
     if ((row=mysql_fetch_row(result)) && row[0]) {
       if ((profajl=(char *)malloc((strlen(row[0])+3)*sizeof(char)))!=NULL) {
         strcpy(profajl,row[0]);
        }
      }
    }
   mysql_free_result(result);
  }
*/
 if (profile)
  sprintf(query,"select `examines`.`examine`,`profiles`.`profile` from `examines`,`profiles` where `examines`.`username`='%s' and `profiles`.`userid`='%d'",username,u->id);
 else
  sprintf(query,"select `examine` from `examines` where `username`='%s'",username);
 if (!(result=mysql_result(query))) {
   sprintf(text,"Chyba pri citani examine skinu.\n");
   write_user(user,text);
   if (oflajn) {
     destruct_user(u);
     destructed=0;
    }
   return;
  }
 if (!(row=mysql_fetch_row(result))) {
   mysql_free_result(result);
   err=3;
   if (profile) {
     sprintf(query,"select `examine`,'' from `examines` where `username`='%s'",username);
     if (!(result=mysql_result(query))) {
       sprintf(text,"Chyba pri citani examine skinu.\n");
       write_user(user,text);
       err=1;
      }
     if (err==3 && !(row=mysql_fetch_row(result))) {
       mysql_free_result(result);
       err=2;
      }
     else err=0;
    }
   if (err>1) {
     if (!strcmp(username,user->name))
       write_user(user,"Nemas vytvoreny examine skin.\n");
     else {
       sprintf(text,"Examine skin '%s' nie je definovany.\n",username);
       if (!testall) write_user(user,text);
      }
    }
   if (err) {
     if (oflajn) {
       destruct_user(u);
       destructed=0;
      }
     return;
    }
  }

 sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
 if (!(fp=ropen(filename,testall?"a":"w"))) { /*APPROVED*/
   if (!testall) write_user(user,"Chyba! Nepodarilo sa otvorit docasny subor!\n");
   mysql_free_result(result);
   if (oflajn) {
     destruct_user(u);
     destructed=0;
    }
   return;
  }

 str=row[0];
 while (*str) {
   if (*str=='\r') { ++str; continue; }
   if (!testall && strlen(str)>6 && !strncmp(str,"PROFIL",6)) {
     if (profile) {
       if (row[1] && row[1][0]) {
         if (profile<100) {
           i=0;len=0;
           while (row[1][i]) {
             if (row[1][i]=='\n') len++;
             i++;
             if (len==profile) break;
            }
           if (row[1][i]) {
             row[1][i]='\0';
             fputs(row[1],fp);
             fputs("[...]\n",fp);
            }
           else fputs(row[1],fp);
          }
         else fputs(row[1],fp);
        }
       else fputs("Ziadny profil.\n",fp);
      }
     while (*str && *str!='\n') ++str;
     if (*str) ++str;
     continue;
    }
   wizvar=0;
   disablelineifvarempty=0;
   if ((oflajn && *str=='+') || (!oflajn && *str=='-')) {
     while (*str && *str!='\n') ++str;
     if (*str) ++str;
     continue;
    }
   if (*str=='+' || *str=='-') ++str;
   if (!*str) continue;
   pos=0;
   texthb[0]='\0';
   while (*str && pos<1000) {
     if (*str=='\r') { ++str; continue; }
     if (*str=='$') { 
       ++str;
       var[0]='\0';
       z=0;
       type=0;
       width=0;
       blik=0;
       disablelineifvarempty=0;
       if (*str=='!') { disablelineifvarempty=1; ++str; }
       if (*str=='-') { align=0; ++str; }
       else align=1;
       if (*str=='0') { z=48; ++str; }
       else if (*str=='*') { ++str; z=*str; ++str; }
       if (isdigit(*str)) { width=*str-48; ++str; }
       if (isdigit(*str)) { width=width*10+*str-48; ++str; }
       switch (*str) {
         case '$': if (z==0) z=' '; sprintf(var,"\250%c",z); break;
         case 'n': strcpy(var,u->name); break;
         case 'N': 
         case 'F': 
           type=-1;
           yes[0]='\0';
           if (*(str+1)==':') {
             *str+=2;
             len=0;
             while(*str!=':' && len<30) {
               yes[len]=*str;
               len++;
               ++str;
              }
             yes[len]='\0';
            }
           if (*str=='F') sprintf(var,"%s%s %s",u->name,yes,colour_com_strip2(u->desc,0));
           else sprintf(var,"%s%s %s",u->name,yes,u->desc);
         break;
         case 'D': type=-1; strcpy(var,u->desc); break;
         case 'q': if (!oflajn || u->logoutmsg[0]=='-') break; type=-1; strcpy(var,u->logoutmsg); break;
         case 'U': strcpy(var,u->homepage); break;
         case 'c': sprintf(var,"%s",zobraz_datum((time_t *)&u->last_login, 2)); break;
         case 'k': sprintf(var,"%s",zobraz_datum((time_t *)&(u->last_login),1)); break;
         case 'I': if (oflajn) break; sprintf(var,"%d",idle); break;
         case 'R': strcpy(var,u->room->name); break;
         case 'r': strcpy(var,u->room->label); break;
         case 'L': strcpy(var,u->cloak?level_name[u->cloak]:level_name[u->level]); break;
         case 'l': sprintf(var,"%d",u->cloak?u->cloak:u->level); break;
         case 'B': if (u->level>=KIN) blik=1; strcpy(var,u->cloak?level_name[u->cloak]:level_name[u->level]); type=-1; break;
         case 'b': if (u->level>=KIN) blik=1; sprintf(var,"%d",u->cloak?u->cloak:u->level); type=-1; break;
         case 'S': sprintf(var,"%d",mins); break;
         case '[': if (oflajn) sprintf(var,"%d",days2); break;
         case '|': if (oflajn) sprintf(var,"%d",hours2); break;
         case ']': if (oflajn) sprintf(var,"%d",mins2); break;
         case 'a': if (!oflajn) sprintf(var,"%d",u->ap); break;
         case 'h': sprintf(var,"%d",mins/60); break;
         case 'H': sprintf(var,"%d",u->health); break;
         case 'm': sprintf(var,"%d",mins%60); break;
         case 'P': sprintf(var,"%d",u->pp); break;
         case 'T': sprintf(var,"%d",(int)u->total_login/86400); break;
         case 't': sprintf(var,"%d",(int)((u->total_login%86400)/3600)); break;
         case 'u': sprintf(var,"%d",(int)(((u->total_login%86400)%3600)/60)); break;
         case 'M': sprintf(var,"%d",u->mana); break;
         case 'v': if (u->age<1) strcpy(var,"-"); else sprintf(var,"%d",u->age); break;
         case '(': if (!oflajn && holding[0]>-1) sprintf(var,"%s",predmet[holding[0]]->name); break;
         case ')': if (!oflajn && holding[1]>-1) sprintf(var,"%s",predmet[holding[1]]->name); break;
         case '{': if (!oflajn && holding[2]>-1) sprintf(var,"%s",predmet[holding[2]]->name); break;
         case '}': if (!oflajn && holding[3]>-1) sprintf(var,"%s",predmet[holding[3]]->name); break;
         case 'Z': if (!oflajn && holding[HANDS]>-1) sprintf(var,"%s",predmet[holding[HANDS]]->name); break;
         case 'z': if (!oflajn && holding[HANDS+1]>-1) sprintf(var,"%s",predmet[holding[HANDS+1]]->name); break;
         case 'X': if (!oflajn && holding[HANDS+2]>-1) sprintf(var,"%s",predmet[holding[HANDS+2]]->name); break;
         case '#': if (user->level<KIN || oflajn) break;
                   sprintf(var,"%d",u->socket); break;
         case '@': if (user->level>=KIN) strcpy(var,u->email); wizvar=1; break;
         case '=': if (user->level>=KIN) {
                     if (oflajn) strcpy(var,u->last_site); 
                     else strcpy(var,u->site);
                    }
                   wizvar=1;
                   break;
         case '%': if (user->level>=KIN) strcpy(var,u->real_ident); wizvar=1; break;
         case '^': if (user->level>=KIN) strcpy(var,u->requestemail); wizvar=1; break;
         case '?': if (oflajn) break;
                   if (u->afk) strcpy(var,"A");
                   else if (u->ignall) {
                     if (u->malloc_start!=NULL) strcpy(var,"E");
                     else if (u->filepos) strcpy(var,"R");
                    }
                   if (!var[0]) strcpy(var," ");
         break;
         case 'K': strcpy(var,u->afk_mesg); break;
         case 'A': if (oflajn) break; type=1; break;
         case 'G': type=2; break;
         case 'V': if (oflajn) break; type=3; break;
         case 'i': type=4; break;
         case 'E': if (oflajn) break; type=6; break;
         case 'C': if (oflajn) break; type=7; break;
         case 'o': type=5; break;
         case 'p': type=8; break;
         default:;
        }
       if (type>0) {
         if (*str) ++str;
         len=0;
         while(*str && *str!=':' && *str!='\n' && len<30) {
           if (*str=='\r') { ++str; continue; }
           yes[len]=*str;
           len++;
           ++str;
          }
         if (*str==':') ++str;
/*         else --str; */
         if (len>29) len=0;
         yes[len]='\0';
         if ((type==1 && u->afk) || (type==2 && u->sex) || (type==3 && u->vis)
         || (type==4 && ((width==1 && u->ignall) || (width==2 && u->ignshout)
                     || (width==3 && u->igntell) || (width==4 && u->ignsys)
                     || (width==5 && u->igngossip) || (width==6 && u->ignfun)
                     || (width==7 && u->igngames) || (width==8 && u->ignportal)))
         || (type==5 && new_mail>u->read_mail)
         || (u->ignall && ((type==6 && u->malloc_start!=NULL) || (type==7 && u->filepos)))
         || (type==8 && u->room!=NULL && (u->room->access & PRIVATE))) {
           strcpy(var,yes);
           for(ii=0;ii<len;ii++) if (*str && *str!='\n') ++str;
           width=len;
          }
         else {
           width=len;
           if (!*str || *str=='\n') {
             strcpy(var,"");
            }
           else {
             for(ii=0;ii<len && *str && *str!='\n';ii++) {
               if (*str=='\r') { ii--; ++str; continue; }
               no[ii]=*str;
               ++str;
              }
/*             --str; */
             no[ii]='\0';
             strcpy(var,no);
            }
          }
        }
       else if (*str) ++str;
       if (var[0]=='\0' && disablelineifvarempty==1) disablelineifvarempty=2;
       if (width==0) {
         strcpy(temp,var);
        }
       else {
         cols=colour_com_count(var);
         if (type==-1) type=1; else type=0;
         if (type==1 && strlen(var)+cols>width) {
           while (strlen(var)-colour_com_count(var)>width) {
             var[strlen(var)-1]='\0';
            }
          }
         cols=colour_com_count(var);
         if (align==0) sprintf(temp,"%s%-*s%s",blik?"~LI":"",width+type*cols,var,blik?"~RS":"");
         else sprintf(temp,"%s%*s%s",blik?"~LI":"",width+type*cols,var,blik?"~RS":"");
         temp[width+blik*6+cols]='\0';
        }
       texthb[pos]='\0';
       if (z>0) {
         len=0;
         while (len<strlen(temp)) {
           if (temp[len]==' ') temp[len]=z;
           len++;
          }
        }
       pos+=strlen(temp);
       if (pos<1000) strcat(texthb,temp);
       if (disablelineifvarempty==2) {
         while (*str && *str!='\n') ++str;
         if (*str) ++str;
/*         pos=0;
         texthb[0]='\0'; */
         break;
        }
      }
     else {
       texthb[pos]=*str;
       pos++;
       texthb[pos]='\0';
       if (*str=='\n') {
         str++;
         break;
        }
       if (*str) ++str;
      }
    }
   strcat(texthb,"~RS~FW~BK");
   if (strchr(texthb,'\250') && (!wizvar || user->level>=KIN) && disablelineifvarempty<2) {
     pos=0;
     while (texthb[pos]!='\250') { line[pos]=texthb[pos]; pos++; }
     z=texthb[pos+1];
     len=pos;
     width=strlen(texthb)-colour_com_count(texthb)-3;
     if (width<80) for(i=0;i<80-width;i++) { line[len]=z; len++; }
     pos+=2;
     while (texthb[pos]!='\0') { line[len]=texthb[pos]; len++; pos++; }
     line[len]='\0';
     fputs(line,fp);
    }
   else {
     if ((!wizvar || user->level>=KIN) && disablelineifvarempty<2) fputs(texthb,fp);
/*     else {
       while (*str && *str!='\n') ++str;
       if (*str) ++str;
      }*/
    }
  }
 fclose(fp);

 mysql_free_result(result);
 
 if (!oflajn && u->level>=WIZ && u!=user && !u->ignall && user->level<=u->level) {
   sprintf(text,"~OL%s si ta prezera...\n",user->name);
   write_user(u,text);
  }
 if (oflajn) {
   destruct_user(u);
   destructed=0;
  }
 if (!testall) {
   showfile(user,filename);
  }
}


void testwho(UR_OBJECT user)
{
FILE /* *infp, */ *fp;
char filename[81]; /* ,filename2[81]; */
/* char line[511]; */
char query[255];
MYSQL_RES *result;
MYSQL_ROW row;
/* int stage=0; */
UR_OBJECT u;

 sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
 if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
   write_user(user,"Chyba! Nepodarilo sa vytvorit docasny subor!\n");
   sprintf(text,"~OL~FW~BRVarovanie:~RS ~OLNepodarilo sa vytvorit docasny subor '%s'!\n",filename);
   write_level(KIN,1,text,NULL);
   return;
  }
 for(u=user_first;u!=NULL;u=u->next) {
   if (u->type==CLONE_TYPE || u->login) continue;
   if (!u->vis && u->level>user->level) continue;
   if (u->who_type>0) continue;
   sprintf(query,"select `body` from `who` where `username`='%s'",u->name);
   if((result=mysql_result(query))) {
     row=mysql_fetch_row(result);
     fputs(parse_who_line(u,row[0],0,0,user->level),fp);
   }
   /* sprintf(filename2,"whos/%s.who",u->name);
   if (!(infp=ropen(filename2,"r"))) continue;
   stage=0;
   fgets(line,500,infp);
   while(!(feof(infp)) && stage==0) {
     if (line[0]=='\n') stage++;
     fgets(line,500,infp);
    }
   fclose(infp);
   fputs(parse_who_line(u,line,0,0,user->level),fp);*/
  }
 fclose(fp);
 if (user->pagewho) {
   switch(more(user,user->socket,filename)) {
     case 0: write_user(user,"Chyba pri citani who!\n");  break;
     case 1: user->misc_op=2;
    }
  }
 else showfile(user,filename);
}

void testwhoall(UR_OBJECT user)
{
FILE /* *infp, */ *fp;
char filename[81]; /* ,filename2[81]; */
/* char line[511]; */
char query[255];
MYSQL_RES *result;
MYSQL_ROW row;

 sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
 if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
   write_user(user,"Chyba! Nepodarilo sa vytvorit docasny subor!\n");
   sprintf(text,"~OL~FW~BRVarovanie:~RS ~OLNepodarilo sa vytvorit docasny subor '%s'!\n",filename);
   write_level(KIN,1,text,NULL);
   return;
  }
 sprintf(query,"select `username`,`body` from `who`");
 if ((result=mysql_result(query))) {
   while((row=mysql_fetch_row(result))) {
     if (row[0][0] != '+') { /* nie je to preddefinovany skin */
       sprintf(text,"%s:\n",row[0]);
       fputs(text,fp);
       fputs(parse_who_line(user,row[1],0,0,user->level),fp);
      }
    }
  } else {
   write_user(user,"Chyba! Nepodarilo sa nacitat uzivatelske who skiny z databazy!\n");
   fclose(fp);
   return;
  }
 fclose(fp);
 if (user->pagewho) {
   switch(more(user,user->socket,filename)) {
     case 0: write_user(user,"Chyba pri citani who!\n");  break;
     case 1: user->misc_op=2;
    }
  }
 else showfile(user,filename);
}

char *parse_who_line(UR_OBJECT u,char *line,int wizzes,int users,int userlevel)
{
int align,type,pos;
unsigned int i,ii,width,len;
char temp[500],var[500],yes[32],no[32],z;
int secs, daysuptime, hoursuptime,idle=0,mins=0,blik,cols;

 if (u!=NULL) {
   mins=(int)(time(0) - u->last_login)/60;
   idle=(int)(time(0) - u->last_input)/60;
  }
   i=0;
   parsed[0]='\0';
   if(strchr(line,'$')) {
     pos=0;
     while (i<strlen(line) && pos<1000) {
       if (line[i]=='$') {
         i++;
         var[0]='\0';
         z=0;
         type=0;
         width=0;
         blik=0;
         if (line[i]=='-') { align=0; i++; }
         else align=1;
         if (line[i]=='0') { z=48; i++; }
         else if (line[i]=='*') { i++; z=line[i]; i++; }
         if (isdigit(line[i])) { width=line[i]-48; i++; }
         if (isdigit(line[i])) { width=width*10+line[i]-48; i++; }
         if (u==NULL) switch (line[i]) {
           case 'U':
             if (width==0) width=2;
             secs=(int)(time(0)-boot_time);
             daysuptime=secs/86400;
             sprintf(var,"%d",daysuptime);
           break;
           case 'u':
             if (width==0) width=2;
             secs=(int)(time(0)-boot_time);
             hoursuptime=(secs%86400)/3600;             
             sprintf(var,"%d",hoursuptime);
           break;
           case 'S': if (width==0) width=3; sprintf(var,"%d",userlevel); break;
           case 'x': if (width==0) width=3; sprintf(var,"%d",max_users_was); break;
           case '#': if (width==0) width=3; sprintf(var,"%d",users); break;
           case 'w': if (width==0) width=2; sprintf(var,"%d",wizzes); break;
           case 'c': if (width==0) width=5; sprintf(var,"%2d:%02d",thour,tmin); break;
           case 'm': if (width==0) width=2; sprintf(var,"%d",tmonth+1); break;
           case 'r': if (width==0) width=4; sprintf(var,"%d",tyear); break;
           case 'd': if (width==0) width=2; sprintf(var,"%d",tmday); break;
           case 't': if (width==0) width=3; strcpy(var,day[twday]); break;
           case 'M': if (width==0) width=3; strcpy(var,month[tmonth]); break;
           case 'D': if (width==0) width=10; sprintf(var,"%d.%d.%4d",tmday,tmonth+1,tyear); break;
          }
         else switch (line[i]) {
           case 'n': if (width==0) width=12; strcpy(var,u->name); break;
           case 'N': 
           case 'F': 
             type=-1; if (width==0) width=40;
             yes[0]='\0';
             if (line[i+1]==':') {
               i+=2;
               len=0;
               while(line[i]!=':' && len<30) {
                 yes[len]=line[i];
                 len++;
                 i++;
                }
               yes[len]='\0';
              }
             if (line[i]=='F') sprintf(var,"%s%s %s",u->name,yes,colour_com_strip2(u->desc,0));
             else sprintf(var,"%s%s %s",u->name,yes,u->desc);
           break;
           case 'D': type=-1; if (width==0) width=25; strcpy(var,u->desc); break;
           case 'd': if (width==0) width=25; strcpy(var,colour_com_strip2(u->desc,0)); break;
           case 'I': if (width==0) width=3; sprintf(var,"%d",idle); break;
           case 'R': if (width==0) width=4; if (u->room->group==4) strcpy(var,BRUTALIS); else strcpy(var,u->room->name); break;
           case 'r': if (width==0) width=3; if (u->room->group==4) strcpy(var,"bru"); else strcpy(var,u->room->label); break;
           case 'L': if (width==0) width=7; strcpy(var,u->cloak?level_name[u->cloak]:level_name[u->level]); break;
           case 'l': if (width==0) width=1; sprintf(var,"%d",u->cloak?u->cloak:u->level); break;
           case 'B': if (u->level>=KIN) blik=1; if (width==0) width=7; strcpy(var,u->cloak?level_name[u->cloak]:level_name[u->level]); break;
           case 'b': if (u->level>=KIN) blik=1; if (width==0) width=1; sprintf(var,"%d",u->cloak?u->cloak:u->level); break;
           case 'S': if (width==0) width=4; sprintf(var,"%d",mins); break;
           case 'h': if (width==0) width=2; sprintf(var,"%d",mins/60); break;
           case 'H': if (width==0) width=3; sprintf(var,"%d",u->health); break;
           case 'a': if (width==0) width=3; sprintf(var,"%d",u->ap); break;
           case 'm': if (width==0) width=2; sprintf(var,"%d",mins%60); break;
           case 'P': if (width==0) width=3; sprintf(var,"%d",u->pp); break;
           case 'T': if (width==0) width=3; sprintf(var,"%d",(int)u->total_login/86400); break;
           case 't': if (width==0) width=2; sprintf(var,"%d",(int)((u->total_login%86400)/3600)); break;
           case 'M': if (width==0) width=3; sprintf(var,"%d",u->mana); break;
           case '#': if (width==0) width=3; sprintf(var,"%d",users); break;
           case 's': if (userlevel<KIN) break; if (width==0) width=3; sprintf(var,"%d",u->socket); break;
           case '@': if (userlevel<KIN) break; if (width==0) width=20; strcpy(var,u->email); break;
           case '=': if (userlevel<KIN) break; if (width==0) width=20; strcpy(var,u->site); break;
           case '%': if (userlevel<KIN) break; if (width==0) width=20; strcpy(var,u->real_ident); break;
           case 'W': if (width==0) width=2; sprintf(var,"%d",u->who_type); break;
           case '?': if (width==0) width=1;
                     if (u->afk) strcpy(var,"A");
                     else if (u->ignall) {
                       if (u->malloc_start!=NULL) strcpy(var,"E");
                       else if (u->filepos) strcpy(var,"R");
                      }
                     if (!var[0]) strcpy(var," ");
           break;
           case 'A': type=1; break;
           case 'G': type=2; break;
           case 'V': type=3; break;
           case 'i': type=4; break;
           case '>': type=5; break;
           case 'E': type=6; break;
           case 'C': type=7; break;
           case 'p': type=8; break;
          }
         if (type>0) {
           i++;
           len=0;
           while(line[i]!=':' && line[i]!='\n' && len<30) {
             yes[len]=line[i];
             len++;
             i++;
            }
           if (line[i]!='\n') i++;
           if (len==30) len=0;
           yes[len]='\0';
           if ((type==1 && u->afk) || (type==2 && u->sex) || (type==3 && u->vis)
           || (type==4 && ((width==1 && u->ignall) || (width==2 && u->ignshout)
                       || (width==3 && u->igntell) || (width==4 && u->ignsys)
                       || (width==5 && u->igngossip) || (width==6 && u->ignfun)
                       || (width==7 && u->igngames) || (width==8 && u->ignportal)))
           || (type==5 && wizzes==-1)
           || (u->ignall && ((type==6 && u->malloc_start!=NULL) || (type==7 && u->filepos)))
           || (type==8 && u->room!=NULL && (u->room->access & PRIVATE))) {
             strcpy(var,yes);
             for(ii=0;ii<len;ii++) if (line[i]!='\n') i++;
             width=len;
            }
           else {
             width=len;
             if (line[i]=='\n') {
               strcpy(var,"");
              }
             else {
               for(ii=0;ii<len && line[i]!='\n';ii++) {
                 no[ii]=line[i];
                 i++;
                }
               no[len]='\0';
               strcpy(var,no);
              }
            }
          }
         else i++;
         cols=colour_com_count(var);
         if (type==-1) type=1; else type=0;
         if (type==1 && strlen(var)+cols>width) {
           while (strlen(var)-colour_com_count(var)>width) {
             var[strlen(var)-1]='\0';
            }
          }
         cols=colour_com_count(var);
         if (align==0) sprintf(temp,"%s%-*s",blik?"~LI":"",width+type*cols,var);
         else sprintf(temp,"%s%*s",blik?"~LI":"",width+type*cols,var);
         temp[width+blik*3+cols]='\0';
         if (blik) strcat(temp,"~RS");
         parsed[pos]='\0';
         if (z>0) {
           len=0;
           while (len<strlen(temp)) {
             if (temp[len]==' ') temp[len]=z;
             len++;
            }
          }
         pos+=strlen(temp);
         if (pos<1000) strcat(parsed,temp);
        }
       else {
         parsed[pos]=line[i];
         i++;
         pos++;
         parsed[pos]='\0';
        }
      }
    }
   else strcpy(parsed,line);
   strcat(parsed,"~RS~FW~BK");
   return parsed;
}

/* DEFINOVATELNE WHO !! (V) */
void newwho(UR_OBJECT user,char *username)
{
FILE *fp;
char filename[81];
int invis=0;
int wizzes=0,users=0,userno=0,match=0;
UR_OBJECT u;

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
   who_from_notify_is_online(user);
   return;
  }
*/
 sprintf(query,"select `head`,`body`,`legs` from `who` where `username`='%s'",username);
 if (!(result=mysql_result(query))) {
   sprintf(text,"Chyba pri citani who skinu.\n");
   write_user(user,text);
   return;
  }
 if (!(row=mysql_fetch_row(result))) {
   if (!strcmp(username,user->name))
     write_user(user,"Nemas vytvoreny skin pre who.\n");
    else {
      sprintf(text,"Who skin '%s' nie je definovany.\n",username);
      write_user(user,text);
     }
   mysql_free_result(result);
   return;
  }
/* sprintf(filename2,"whos/%s.who",username);
 if (!(infp=ropen(filename2,"r"))) {
   if (!strcmp(username,user->name))
     write_user(user,"Nemas vytvoreny skin pre who.\n");
   else {
     sprintf(text,"Who skin nie je definovany.\n");
     write_user(user,text);
    }
   return;
  }
 */
 sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
 if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
   write_user(user,"Chyba! Nepodarilo sa otvorit docasny subor!\n");
   mysql_free_result(result);
   return;
  }
 fputs(parse_who_line(NULL,row[0],wizzes,users,user->level),fp);

 for(u=user_first;u!=NULL;u=u->next) {
   if (u->type==CLONE_TYPE || u->login) continue;
   if (!u->vis) {
     if (u->level<KIN || u->level<=user->level) users++;
     if (u->level>user->level) continue;
     invis++;
    }
   else {
     if (u->level>=KIN) wizzes++;
     users++;
    }
   userno++;
   if (u==user) 
     parse_who_line(u,row[1],-1,userno,user->level);
   else
     parse_who_line(u,row[1],0,userno,user->level);
   if ((word_count>1) && (!user->login) && !search_success(user,u,parsed,word_count)) continue;
   match++;
   fputs(parsed,fp);
  }

 fputs(parse_who_line(NULL,row[2],wizzes,users,match),fp);
 fclose(fp);
 mysql_free_result(result);
 
 if (user->pagewho) {
   switch(more(user,user->socket,filename)) {
     case 0: write_user(user,"Chyba pri citani who!\n");  break;
     case 1: user->misc_op=2;
    }
  }
 else showfile(user,filename);		
}

/*** Show who is on - ZMENA Spakky, prerobeny dizajn. ***/
void who(user,people) /*WHOSTART*/
UR_OBJECT user;
int people;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle,logins,i;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char rname[ROOM_NAME_LEN+1],idlestr[6],sockstr[3];
FILE *fp;
char filename[90];

/*if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/
if (people && (word_count==2) && (!strcmp(word[1],"t"))) {
  show_timeouts(user);
  return;
 }
total=0;invis=0;/* remote=0; */logins=0;

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nepodarilo sa otvorit docasny subor pre zakladne .who!\n");
	return;
	}

if (people) { 
  sprintf(text,"\n~OL~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ~FMPrihlaseni ludia~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  fprintf(fp,"%s",text);
  fprintf(fp,"~FTMeno            :     Level Link IgnVse Vidi Neci Minu Adresa \r\n\n");
 }
else {
  fprintf(fp,"  ~RS_________________ ~OL~FY_____ ____ __   _____ __ __ ____ __ ____ ~RS~FW_________________\n");
  fprintf(fp,"['                / ~OL~FY||_||  ||  ||   ||_|| ||\\||  ||  || ||__ ~RS~FW\\                `]\n");
  fprintf(fp,"~OL(~RS======~OL~FW)~RS__________\\ ~FY|| ||  ||  ||__ || || || ||  ||  || __|| ~RS~FW/__________~OL(~RS======~OL)\n");
  fprintf(fp," ~RS~FW@~OL~FW]\\\\[~RS~FW@_~OL~FW>~RS~FBL~OL~FBevel~RS~OL~FW<__>~RS~FBM~OL~FBeno ~RS~FBa d~OL~FBescription~RS~OL~FW<____________________>~RS~FBM~OL~FBiestnost~RS~OL~FW<_____~RS~FW@~OL~FW]\\\\[~RS~FW@\n");
  fprintf(fp,"~OL  ]\\\\[                                                                    ]\\\\[ ~RS\n");
 }
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE) continue;
      
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
                   
      if (u->login) {
            if (!people) continue;
            sprintf(text,"~FY[Prihl. staz %d] :        -  %4d     -    - %4d    -  %-25.25s\n",4 - u->login,u->socket,idle,u->site);
            fprintf(fp,"%s",text);
            logins++;
            continue;
            }
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
      if (people) {
            if (u->afk) strcpy(idlestr," AFK");
            else sprintf(idlestr,"%4d",idle);
            sprintf(sockstr,"%2d",u->socket);
            sprintf(text,"~OL~FR%-15s : ~RS~FG %7s  %4s   %s  %s %3s %4d  ~OL~FY%-25.25s\n",u->name,level_name[u->level],sockstr,noyes(u->ignall),noyes(u->vis),idlestr,mins,u->site);
            fprintf(fp,"%s",text);
            continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

      if (u->afk) sprintf(line," ~RS~FR%s ~FT%s~RS~FW",u->name,u->desc);
	else sprintf(line," ~OL~FR%s ~FT%s~RS~FW",u->name,u->desc);
	
    
      if (!u->vis) line[0]='*';
      if (u==user) line[0]='>'; /* */
      strcpy(rname,u->room->name);
      
      i=0; 
      while (rname[i]!='\0') {
      	if (rname[i]=='_') rname[i]=' ';
      	i++;
      	}

      
      /* Count number of colour coms to be taken account of when formatting */
      cnt=colour_com_count(line);
      if (u->level >= WIZ) sprintf(text,"~OL~FW  ]\\\\[  %-7s ~OL~FY",u->cloak?level_name[u->cloak]:level_name[u->level]);
        	else sprintf(text,"~OL~FW  ]\\\\[  ~RS%-7s ~OL~FY",u->cloak?level_name[u->cloak]:level_name[u->level]);
      fprintf(fp,"%s",text);      
      sprintf(text,"%-*s ~RS~FW%-16s ~OL]\\\\[\n", 40+cnt,line,rname);
      fprintf(fp,"%s",text);
      }
if (people) sprintf(text,"\n~OL~FR=-=-=-=- ~RS~FTJe tu ~OL~FG%03d ~RS~FTviditelnych, ~OL~FG%03d ~RS~FTneviditelnych ludi, spolu ~OL~FY%03d ~RS~FTludi~OL~FR =-=-=-=-=\n",num_of_users-invis,invis,total);
else sprintf(text,"~OL .]\\\\[.                                                                  .]\\\\[.\n~RS/------\\ ~BW~FK(prikazom ~OL~FY.set who ~RS~FK~BWsi mozes zvolit iny typ vypisu .who, skus!)~BK ~FW/------\\\n=========================~OL== Celkove: ~FY%03d ~FWprihlasenych ==~RS========================\n",total);

fprintf(fp,"%s",text);
/*=-=-=-=-=- There are 01 visible, 01 invisible users, total 23 users =-=-=-=-=-=*/
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani BASIC .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
		}
	else showfile(user,filename);
		
}

/* Alternativny vypis .who */
void who_alt1(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote,idle,logins */,i;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char rname[ROOM_NAME_LEN+1];
char stav[20];
FILE *fp;
char filename[90];

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	fclose(fp);
	deltempfile(filename);
	}
if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor v NATURE .who!\n");
	return;
	}

total=0;invis=0;/* remote=0;logins=0; */
fprintf(fp,"            ~FT,,                                                   ||\n");
fprintf(fp,"           ~FY(~OL~FWoo~RS~FY)                                                 ~RS~FY(~OL~FWoo~RS~FY)\n");
fprintf(fp,"         ~FYooO~OL~FT()~RS~FYOoo            ~OL~FGPrihlaseni uzivatelia~RS            ~FYooO~OL~FT()~RS~FYOoo\n");
sprintf(text,"~RS~FG=~OL=<~FW+~FG>=~RS~FG=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=~OL=<~FW+~FG>=~RS~FG=\n");
fprintf(fp,"%s",text);
fprintf(fp,"          ~OL~FYMENO/POPIS                          MIESTNOST       UROVEN   MIN STAV\n\n");
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      /* idle=(int)(time(0) - u->last_input)/60; */
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

      if (u->afk) sprintf(line," ~FR%s %s~RS~FW",u->name,u->desc);
	else  sprintf(line," ~FT%s ~FW%s~RS~FW",u->name,u->desc);
	
	
      if (!u->vis) line[0]='*';
      if (u==user) line[0]='>'; 
      strcpy(rname,u->room->name);
      
      i=0; 
      while (rname[i]!='\0') {
      	if (rname[i]=='_') rname[i]=' ';
      	i++;
      	}
    
      
      /* Count number of colour coms to be taken account of when formatting */
      cnt=colour_com_count(line);
      sprintf(text,"%-*s ~RS~FW| ~FT%-16s ~FW| ", 40+cnt,line,rname);
      fprintf(fp,"%s",text);
      if (u->level >= WIZ) sprintf(text,"~OL~FR%-7s ",u->cloak?level_name[u->cloak]:level_name[u->level]);
        	else sprintf(text,"~FT%-7s ",u->cloak?level_name[u->cloak]:level_name[u->level]);
      fprintf(fp,"%s",text);      
    /*  if (idle>14) { sprintf(text, "~RS~FM: %3d/",mins); 
      		  	fprintf(fp,"%s",text);
		  	sprintf(text, "~OL~FM%d\n",idle); 
		  }  */
		  
            /* else */   sprintf(text,"~RS~FW| ~FT%-4d ",mins);
      fprintf(fp,"%s",text);      
    strcpy(stav,"~OL~FT");
    if (u->afk) strcat(stav,"A");
      else if (u->malloc_start!=NULL) strcat(stav,"E");
        else if (u->filepos) strcat(stav,"R");
          else if (u->lab) strcat(stav,"Y"); 
       
          	
    		if (u->hang_stage!=-1) strcat(stav,"H");
    			else {
	    		if (u->game==2) strcat(stav,"P");
	    		if (u->game==3) strcat(stav,"G");
	    		if (u->game==4) strcat(stav,"L");
	    		}
    		if (u->doom_energy) strcat(stav,"D");
                else if (u->reversi_cislotahu) strcat(stav,"V");
		if (u->dhrac>0) strcat(stav,"Q");
		else if (u->miny_tab) strcat(stav,"M");
		if (u->chrac>0) strcat(stav,"C");
		
		               
   strcat(stav,"~RS\n");
   fprintf(fp,"%s",stav);
      }
sprintf(text,"\n~FG=~OL-> ~FWuzivatelov: %-3d ~FG<-~RS~FG=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=~OL=<~FW+~FG>=~RS~FG=\n",total);
fprintf(fp,"%s",text);
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani NATURE .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
	}
	else showfile(user,filename);

}




/* Dalsi alternativny vypis .who, tentokrat trosku avantgardny :>>>>>> */
void who_alt2(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle/* ,logins */;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char stav[20];
FILE *fp;
char filename[90];


/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre CYBERIA .who!\n");
	return;
	}
		
total=0;invis=0;/* remote=0;logins=0; */
fprintf(fp,"~OL~FY                    + + +         ~RS~FTCURRENT USERS LOGGED IN\n");
sprintf(text,"~OL~FM  _______   ________~FY| | |~FM_____________________________________________   ____\n");
fprintf(fp,"%s",text);
fprintf(fp,"~OL~FM /");
fprintf(fp,"~OL~FWMin/Idl~FM\\ /");
fprintf(fp,"~OL~FWMiesto PP~FY\\|/ ~FWMeno              Popis                 LEVEL~FM\\ /");
fprintf(fp,"~OL~FWStat~FM\\ \n");
fprintf(fp,"~OL~FM|~RS~FM---------~OL|~RS~FM---------- ~OL~FY|~RS~FM -----------------------------------------------~OL|~RS~FM------~OL|\n");

for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

sprintf(text,"~OL~FM| ~RS~FT%4d/%2d ~OL~FM| ~FR[%3.3s] ",mins,idle, u->room->name);
fprintf(fp,"%s",text);
if (u->pp>149) sprintf(text,"~OL~FT%3d ~FM| ",u->pp);
  else if ((u->pp>99) && (u->pp<150)) sprintf(text,"~RS~FT%3d ~OL~FM| ",u->pp);
    else if ((u->pp>49) && (u->pp<100)) sprintf(text,"~OL~FB%3d ~FM| ",u->pp);
       else sprintf(text,"~RS~FB%3d ~OL~FM| ",u->pp);
fprintf(fp,"%s",text);       

      if (u->afk) sprintf(line," ~RS~FR%s %s~RS~FW",u->name,u->desc);
	else  sprintf(line," ~RS~FT%s ~FW%s~RS~FW",u->name,u->desc);  
	
    
      if (!u->vis) line[0]='*';
      if (u==user) line[0]='>'; 
      
      /* Count number of colour coms to be taken account of when formatting */
      cnt=colour_com_count(line);
      sprintf(text,"%-*s ~RS", 40+cnt,line);
      fprintf(fp,"%s",text);
      
      if (u->level >= WIZ) sprintf(text,"~OL~FG(%3.3s) ~FM| ",u->cloak?level_name[u->cloak]:level_name[u->level]);
        	else sprintf(text,"~RS~FG(%3.3s) ~OL~FM| ",u->cloak?level_name[u->cloak]:level_name[u->level]);
      fprintf(fp,"%s",text);      

    strcpy(stav,"~OL~FY");
    if (u->afk) strcat(stav,"A");
        if (u->malloc_start!=NULL) strcat(stav,"E");
        else if (u->filepos) strcat(stav,"R");
          	
    		if (u->hang_stage!=-1) strcat(stav,"H");
    		if (u->lab) strcat(stav,"Y");
    			
	    		if (u->game==2) strcat(stav,"P");
	    		if (u->game==3) strcat(stav,"G");
	    		if (u->game==4) strcat(stav,"L");
            		if (u->doom_energy) strcat(stav,"D");
	    		if (u->reversi_cislotahu) strcat(stav,"V");
			if (u->dhrac>0) strcat(stav,"Q");
			if (u->miny_tab) strcat(stav,"M");
			if (u->chrac>0) strcat(stav,"C");

    	    
   sprintf(text,"%10.10s ~OL~FM|~RS\n",stav);
   fprintf(fp,"%s",text);
      }

sprintf(text,"~OL~FM \\_______/ \\_________/ \\______________________________________________/ \\____/\n");
fprintf(fp,"%s",text);
sprintf(text,"~OL~FM                     \\_~OL~FTTotal of ~FY%d ~FTusers.~FM_/ \n",total);
fprintf(fp,"%s",text);
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani CYBERIA .who!!!\n");  break;
	                  case 1: user->misc_op=2;
	              }
	          }
	          else showfile(user, filename);
}



/* Dalsi alternativny vypis .who, tentokrat karneval :) */
void who_alt3(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle/* ,logins */,i;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char rname[ROOM_NAME_LEN+1];
char filename[90];
FILE *fp;


/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre CARNEVAL .who!\n");
	return;
	}
	
total=0;invis=0;/* remote=0;logins=0; */
fprintf(fp,"~OL~FR,*.,.*'``'*.,.*'``'*.,.*'``'*.,~FM+~FYLOGGED IN USERS~FM+~FR,.*'``'*.,.*'``'*.,.*'``'*.,.*,\n");
fprintf(fp,"~OL~FR| ~FYLEVEL Meno a charakteristika  ~FR=============== |      ~FYMiesto     ~FR|~FYSex Min/Idl~FR|\n");
fprintf(fp,"~OL~FR|~RS~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=~OL|~RS~FR=-=-=-=-=-=-=-=-=~OL|~RS~FR=-=-=-=-=-=~OL|\n");


for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

      sprintf(text,"~OL~FR| ~RS");
      if (u->level>=WIZ) strcat(text,"~OL");
      sprintf(line,"~FG(%3.3s)",u->cloak?level_name[u->cloak]:level_name[u->level]);
      strcat(text,line);
      fprintf(fp,"%s",text);
      /* Count number of colour coms to be taken account of when formatting */
      
            if (u->afk) sprintf(line," ~RS~FW%s %s~RS~FW",u->name,u->desc);
	else  sprintf(line," ~OL~FW%s ~RS~FW%s~RS~FW",u->name,u->desc);
      if (!u->vis) line[0]='*';
      if (u==user) line[0]='>'; 
      
      cnt=colour_com_count(line);
           
      sprintf(text,"%-*s ~RS", 40+cnt,line);
      fprintf(fp,"%s",text);

      strcpy(rname,u->room->name);
      i=0; 
      while (rname[i]!='\0') {
      	if (rname[i]=='_') rname[i]=' ';
      	i++;
      	}
    if (strlen(rname)==16)  sprintf(text,"~OL~FR| ~FW%-15s~FR|",rname);
	else  sprintf(text,"~OL~FR| ~FW%-15s ~FR|",rname);
      fprintf(fp,"%s",text);

      if (!u->sex) fprintf(fp," ~OL~FMZ ");
      	else fprintf(fp," ~OL~FBM ");
      	
sprintf(text,"~RS~FW%4d/%-2d ~OL~FR|~RS\n",mins,idle);
fprintf(fp,"%s",text);
	}

sprintf(text,"~OL~FR|_____.,.*'``'* ~FMTotal users: ~FY%03d ~FR*'``'*.,.______|_________________|___________|~RS\n",total);
fprintf(fp,"%s",text);

fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani CARNEVAL .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
		}
		else showfile(user, filename);
}

/* Dalsi alternativny vypis .who, tentokrat "ocean" */
void who_alt4(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/*,remote */,idle/* ,logins */;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char stav[20];
char filename[90];
FILE *fp;

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre OCEAN .who!\n");
	return;
	}
		
total=0;invis=0;/* remote=0;logins=0; */

fprintf(fp,"  /~OL\\,~RS/~OL\\                      ~FYPrihlaseni uzivatelia                         ~FG.-.-\n");
fprintf(fp,"~FB~OL.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-.~RS~FG_.~FY|~FG-\n");
fprintf(fp,"~FB~OL) ~FTM~RS~FTin/~OLI~RS~FTdl ~OL~FB| ~FTM~RS~FTeno    ~OLD~RS~FTescription                      ~OLL~RS~FTev ~OL~FB| ~FTR~RS~FToom  ~OL~FB| ~FTM~RS~FT/~OLZ ~FB| ~FTS~RS~FTtat ~OL~FB(\n");
fprintf(fp,"~FB~OL(--~RS~FB-------~OL+--~RS~FB----~OL----~RS~FB-----------~OL----------------------~RS~FB---~OL+--~RS~FB----~OL-+--~RS~FB---~OL+--~RS~FB----~OL)\n");
  

for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

sprintf(text,"~OL~FB| ~FW%4d~FT/~RS~FW%2d ~OL~FB|~FW",mins,idle);
fprintf(fp,"%s",text);

if (u->afk) sprintf(line," ~RS~FT%s %s~RS~FW",u->name,u->desc);
else  sprintf(line," ~FT%s ~RS~FT%s~RS~FW",u->name,u->desc);

	

if (!u->vis) line[0]='*';
if (u==user) line[0]='>'; 
      
/* Count number of colour coms to be taken account of when formatting */
cnt=colour_com_count(line);
sprintf(text,"%-*s ", 40+cnt,line);
fprintf(fp,"%s",text);

if (u->level >= WIZ) sprintf(text,"~OL~FB[~FY~LI%3.3s~RS~OL~FB]| ",u->cloak?level_name[u->cloak]:level_name[u->level]);
else sprintf(text,"~OL~FB[~FY%3.3s~FB]| ",u->cloak?level_name[u->cloak]:level_name[u->level]);
fprintf(fp,"%s",text);

if (u->sex==1) sprintf(text,"~OL~FB[~FY%3.3s~FB] | [~FYM~FB] | ", u->room->name);
else if (u->sex==0) sprintf(text,"~OL~FB[~FY%3.3s~FB] | [~FRZ~FB] | ", u->room->name);
else sprintf(text,"~OL~FB[~FY%3.3s~FB] | [~FY?~FB] | ", u->room->name);
fprintf(fp,"%s",text);


    strcpy(stav,"~OL~FW");
    if (u->afk) strcat(stav,"A");
        if (u->malloc_start!=NULL) strcat(stav,"E");
        else if (u->filepos) strcat(stav,"R");
          	
    		if (u->hang_stage!=-1) strcat(stav,"H");
    		if (u->lab) strcat(stav,"Y");
    			
	    		if (u->game==2) strcat(stav,"P");
	    		if (u->game==3) strcat(stav,"G");
	    		if (u->game==4) strcat(stav,"L");
            		if (u->doom_energy) strcat(stav,"D");
	    		if (u->reversi_cislotahu) strcat(stav,"V");
			if (u->dhrac>0) strcat(stav,"Q");
			if (u->miny_tab) strcat(stav,"M");
			if (u->chrac>0) strcat(stav,"C");
    	    
   sprintf(text,"%10.10s ~FB|~RS\n",stav);
   fprintf(fp,"%s",text);
      }

sprintf(text,"~OL~FB(_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-[~RS~FBspolu~OL:~RS~FW%03d~OL~FB]-._.-._.-._)\n",total);
fprintf(fp,"%s",text);
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani OCEAN .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
	}
	else showfile(user, filename);
}

/* Dalsi alternativny vypis .who, no a tentokrat je to TITANIC!!!!!!! :>>> */
void who_alt5(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle/* ,logins */;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
FILE *fp;
char filename[90];

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
total=0;invis=0;/* remote=0;logins=0; */

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	fclose(fp);
	deltempfile(filename);
	}
	
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre TITANIC .who!\n");
	return;
	}

fprintf(fp,"    ~RS~FW|\n");
fprintf(fp,"    ~RS~FW|\n");
fprintf(fp,"    ~OL~FW*                     ~FK###             ###             ###             ###\n");
fprintf(fp,"    ~RS~FW|                     ~FY###             ###             ###             ###\n");
fprintf(fp,"    ~RS~FW|      ~OL~FB__             ~RS~FY###             ###             ###             ###\n");
fprintf(fp,"    ~RS~FW|     ~OL~FB|~RS~FBO ~OL~FB|            ~RS~FY###             ###             ###             ###\n");
fprintf(fp,"    ~RS~FW|     ~OL~FB)--+------------------------------------------------------------------\n");
fprintf(fp,"    ~RS~FW|    ~OL~FB(~RS~FB|~OL~FB[]~RS~FB::~OL~FYP~RS~FYassenger~FB:~OL~FB[]~RS~FB::[]::~OL~FB[]~RS~FB::~OL~FB[]~RS~FB::[]::~OL~FB[]~RS~FB::~OL~FYC~RS~FYlass~FB:::~OL~FYR~RS~FYoom~FB:~OL~FB[]~RS~FB::~OL~FYM~RS~FYin~OL~FY/I~RS~FYdle~FB:~OL~FB[]~RS~FB::~OL~FB[\n");
fprintf(fp," ~OL~FB___~RS~FW|~OL~FB___/-------~RS~FB=========~OL~FB-------------------------~RS~FB=====~OL~FB---~RS~FB====~OL~FB-----~RS~FB===~OL~FB-~RS~FB====~OL~FB-----\n");
  
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }

      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

if (total==1) fprintf(fp,"~OL~FB/  ~FY%03d.~OL~FB",total);
	else if (total==2) fprintf(fp," ~RS~FB\\ ~OL~FY%03d.~OL~FB",total);
	 	else fprintf(fp,"  ~RS~FB|~OL~FY%03d.~OL~FB",total);

strcpy(line,"");
if (u->afk) sprintf(line,":~RS~FW%s %s~RS~FW",u->name,u->desc);
	else  sprintf(line,":~OL~FW%s~RS~FW %s~RS~FW",u->name,u->desc);



if (!u->vis) line[0]='*';
if (u==user) line[0]='>'; 
      
/* Count number of colour coms to be taken account of when formatting */
cnt=colour_com_count(line);
sprintf(text,"%-*s",40+cnt,line);
fprintf(fp,"%s",text);
fprintf(fp," ~OL~FB:~RS~FB:");

if (u->level >= WIZ) fprintf(fp,"~FT=~OL~FY%3.3s~RS~FT=~FB:~OL~FB: ",u->cloak?level_name[u->cloak]:level_name[u->level]);
else fprintf(fp," ~OL~FY%3.3s ~RS~FB:~OL~FB: ",u->cloak?level_name[u->cloak]:level_name[u->level]);

fprintf(fp,"~RS~FT%5.5s ~OL~FB:~RS~FB:",u->room->name);


fprintf(fp,"~OL~FW%4d/~RS~FW%-2d~FB:~OL~FB:~OL~FY ",mins,idle);

    if (u->afk) fprintf(fp,"A");
        if (u->malloc_start!=NULL) fprintf(fp,"E");
        else if (u->filepos) fprintf(fp,"R");
          	
    		if (u->hang_stage!=-1) fprintf(fp,"H");
    		if (u->lab) fprintf(fp,"Y");
    			
	    		if (u->game==2) fprintf(fp,"P");
	    		if (u->game==3) fprintf(fp,"G");
	    		if (u->game==4) fprintf(fp,"L");
            		if (u->doom_energy) fprintf(fp,"D");
	    	        else if (u->reversi_cislotahu) fprintf(fp,"V");
			if (u->dhrac>0) fprintf(fp,"Q");
			else if (u->miny_tab) fprintf(fp,"M");
			if (u->chrac>0) fprintf(fp,"C");
	    		
   fprintf(fp,"\n");
      }

fprintf(fp,"~RS~FB-.~OL_.~RS~FB-~OL.~RS~FB_.-.~OL_.~RS~FB-._.~OL-~RS~FB._.-~OL._~RS~FB.-._~OL.-.~RS~FB_.-~OL._~RS~FB.-~OL._.~RS~FB-.~OL_~RS~FB.-~OL._.~RS~FB-~OL._~RS~FB.-._.~OL-._~RS~FB.~OL->~FKR.M.S. TITANIC~FB<-~RS~FB._\n");
 fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani TITANIC .who!!!\n");  break;
	                  case 1: user->misc_op=2;
	                  }
	           }
	       else showfile(user, filename);
}

/* Dalsi alternativny vypis .who, tentoraz SOP-ka :-) (mieesto v euroope,
   maaame s eS oO Pee!!! :> */
   
void who_alt6(user) /* Volcano */
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle/* ,logins */;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
FILE *fp;
char filename[90];

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
total=0;invis=0;/* remote=0;logins=0; */

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre VOLCANO .who!\n");
	return;
	}

fprintf(fp,"                                    ~OL~FK\\\\||//\n");
fprintf(fp,"                                     ~OL~FR\\||/\n");
fprintf(fp,"                                     ~RS~FG/~OL~FR**~RS~FG\\\n");
fprintf(fp,"           ~RS~FG/^\\ /\\         /^\\/^\\ /\\ / \\ /\\/\\            /^\\           /\\\n");
fprintf(fp,"~RS~FG__/^\\__/^\\/   \\  \\_/^\\___/   /  \\  \\   \\    \\__/^\\__/^\\/ / \\/^\\__/^\\_/\\ \\/^\\___/\n"); 
fprintf(fp,"~RS~FG|  ~OL~FT#    N A M E  &  D E S C R I P T I O N      ~RS~FG\\  ~OL~FTLEVEL  ROOM  MIN/IDLE IGNORE ~RS~FG\\\n");
fprintf(fp,"~RS~FG\\~FT--~OL~FT-~RS~FT----~OL~FT---------------------------------~RS~FT------~FG|~FT--~OL~FT-----~RS~FT--~OL~FT----~RS~FT--~OL~FT--------~RS~FT-~OL~FT------~RS~FT-~FG|\n");

  
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }

      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

if (total%4==3) fprintf(fp,"~RS~FG| ~OL~FW%03d.",total);
if (total%4==2) fprintf(fp,"~RS~FG/ ~OL~FW%03d.",total);
if (total%4==1) fprintf(fp,"~RS~FG| ~OL~FW%03d.",total);
if (total%4==0) fprintf(fp,"~RS~FG\\ ~OL~FW%03d.",total);

strcpy(line,"");
if (u->afk) sprintf(line," ~RS~FW%s %s~RS~FW",u->name,u->desc);
	else  sprintf(line," ~RS~FW%s~FT %s~RS~FW",u->name,u->desc);
	

if (!u->vis) line[0]='*';
if (u==user) line[0]='>'; 
      
/* Count number of colour coms to be taken account of when formatting */
cnt=colour_com_count(line);
sprintf(text,"%-*s",40+cnt,line);
fprintf(fp,"%s",text);      
if (total%4==3) fprintf(fp," ~RS~FG\\");
if (total%4==2) fprintf(fp," ~RS~FG|");
if (total%4==1) fprintf(fp," ~RS~FG/");
if (total%4==0) fprintf(fp," ~RS~FG|");

if (u->level >= WIZ) fprintf(fp," ~OL~LI~FY%-7s~RS~FT ",u->cloak?level_name[u->cloak]:level_name[u->level]);
else fprintf(fp," ~OL~FY%-7s~RS~FT ",u->cloak?level_name[u->cloak]:level_name[u->level]);

fprintf(fp,"[%3.3s] ",u->room->name);

fprintf(fp,"~OL~FW%4d/~RS~FW%-2d  ",mins,idle);

if (u->ignall) fprintf(fp,"~OL~FRA");
	else fprintf(fp,"~RS~FR-");

if (u->ignshout) fprintf(fp,"~OL~FRS");
	else fprintf(fp,"~RS~FR-");
 	
if (u->igntell) fprintf(fp,"~OL~FRT");
	else fprintf(fp,"~RS~FR-");

if (u->ignsys) fprintf(fp,"~OL~FRY");
	else fprintf(fp,"~RS~FR-");

if (u->ignfun) fprintf(fp,"~OL~FRF");
	else fprintf(fp,"~RS~FR-");

if (u->igngossip) fprintf(fp,"~OL~FRG");
	else fprintf(fp,"~RS~FR-");

if (total%4==3) fprintf(fp," ~RS~FG\\");
if (total%4==2) fprintf(fp," ~RS~FG|");
if (total%4==1) fprintf(fp," ~RS~FG/");
if (total%4==0) fprintf(fp," ~RS~FG|");
	    		
   fprintf(fp,"\n");
      }

fprintf(fp,"~RS~FG\\----------------------------------------------|-------------------------------/~RS~FW\n");
 fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani VOLCANO .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
		}
	else showfile(user, filename);

}


/* Dalsi alternativny vypis .who, tentokrat "HELL" (S) */
void who_alt7(user)
UR_OBJECT user;
{
UR_OBJECT u;
int total,mins,idle,i, poc1, dayst, hourst, wizzes;
int secs, daysuptime, hoursuptime;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char filename[90];
FILE *fp;

secs=(int)(time(0)-boot_time);
daysuptime=secs/86400;
hoursuptime=(secs%86400)/3600;
wizzes=0;
/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre HELL .who!\n");
	return;
	}
	
total=0;
fprintf(fp,"  ~FR\\__ /\\_ ============================================================= _/\\__/\n");
fprintf(fp,"   ~FR\\\\`-_---  !@$*(&*)&(#!@@%%)*(&)(!@)&*&!@&*(%%^!@^%%$)(&!#%%#&^%%!@%%&!  ---_-`//\n");
fprintf(fp,"   ~FR/`- !Q@#$~OLW~RS~FR&!$~OLe~RS~FR$~OLl~RS~FR$#$~OLc~RS~FR!)(~OLo~RS~FR$&~OLm~RS~FR  ~OLe  ~RS~FR!@#%%(~OLt o~RS~FR  @#~OLH~RS~FR_(*~OLE~RS~FR(_*@~OLL~RS~FR%%#6~OLL~RS~FR@#####*)&!@%% -'\\\n");
fprintf(fp,"  ~FR|      !Q!@*($&)(!)(&@$&^!%%@^&!(^   !@(#%%#!(%%_!_*(%%!@#*)(%%&()!@)#*%%&      |\n");
fprintf(fp," ~FR\\\\!@@$~OLn~RS~FR%%&*~OLa~RS~FR*)~OLm~RS~FR%%~OLe~RS~FR%%*~OLlev~RS~FR^&~OLmin~RS~FR(~OLid~RS~FR&~OLl~RS~FR^%%!@)!~OLi~RS~FR(~OLg~RS~FR$~OLn~RS~FR*!~OLstat~RS~FR(~OLr~RS~FR#~OLo~RS~FR(~OLo~RS~FR!@~OLm~RS~FR#(~OLp~RS~FR^~OLp~RS~FR!@~OLtot~RS~FR_~OLlogn~RS~FR!~OLw~RS~FR$//\n");
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis && u->level>user->level) continue;

      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

if (total%4==0) strcpy(text," ~FR-/.");
if (total%4==1) strcpy(text," ~FR|  ");
if (total%4==2) strcpy(text," ~FR/-.");
if (total%4==3) strcpy(text," ~FR | ");
if (u->sex==1) strcat(text,"m");
	else if (u->sex==0) strcat(text,"z");
		else strcat(text,"?");
if (u->vis) strcat(text," ~OL"); else strcat(text,"*~OL");
strcat(text,u->name); strcat(text,"~RS~FR");
for (i=strlen(u->name); i<12; ++i)
	strcat(text,"_");
strcat(text," ");
sprintf(line,"%3.3s",u->cloak?level_name[u->cloak]:level_name[u->level]);
line[1]=tolower(line[1]);
if (u->level>=WIZ) {
	wizzes++;
	strcat(text,"~OL");
	}
strcat(text,line);
sprintf(line," ~RS~FR[~OL%4d~RS~FR/~OL%-2d~RS~FR] ",mins,idle);
strcat(text,line);
if (total%2==0) strcat(text,"\\~OL|~RS~FR/ ");
	else strcat(text,"/~OL|~RS~FR\\ ");
if (u->ignall) strcat(text,"~OLA~RS~FR");
	else strcat(text,"_");
if (u->ignshout) strcat(text,"~OLS~RS~FR");
	else strcat(text,"_");
if (u->igntell) strcat(text,"~OLT~RS~FR");
	else strcat(text,"_");
if (u->ignfun) strcat(text,"~OLF~RS~FR");
	else strcat(text,"_");
if (u->ignsys) strcat(text,"~OLY~RS~FR");
	else strcat(text,"_");
if (u->igngossip) strcat(text,"~OLG~RS~FR");
	else strcat(text,"_");
strcat(text,"->~OL");
poc1=0;
    if (u->afk) { strcat(text,"A"); poc1++; }
        if (u->malloc_start!=NULL) { strcat(text,"E"); poc1++; }
        else if (u->filepos) { strcat(text,"R"); poc1++; }          	
    		if (u->hang_stage!=-1) { strcat(text,"H"); poc1++; }
    		if (u->lab) { strcat(text,"Y"); poc1++;}    			
	    		if (u->game==2) { strcat(text,"P"); poc1++; }
	    		if (u->game==3) { strcat(text,"G"); poc1++; }
	    		if (u->game==4) { strcat(text,"L"); poc1++; }
            		if (u->doom_energy) { strcat(text,"D"); poc1++; }
            		else if (u->reversi_cislotahu) {strcat(text,"V"); poc1++; }
			if (u->dhrac>0) {strcat(text,"Q"); poc1++; }
			else if (u->miny_tab) {strcat(text,"M"); poc1++; }
			if (u->chrac>0) {strcat(text,"C"); poc1++; } 

if (poc1==0) strcat(text,"~RS~FR----<-:~OL");
if (poc1==1) strcat(text,"~RS~FR---<-:~OL");
if (poc1==2) strcat(text,"~RS~FR--<-:~OL");
if (poc1==3) strcat(text,"~RS~FR-<-:~OL");
if (poc1==4) strcat(text,"~RS~FR<-:~OL");
sprintf(line,"%4.4s~RS~FR:->.~OL%03d~RS~FR.",u->room->name,u->pp);
strcat(text,line);
if (total%4==0) strcat(text,"|");
if (total%4==1) strcat(text,"\\");
if (total%4==2) strcat(text,"|");
if (total%4==3) strcat(text,"/");
dayst=u->total_login/86400;
hourst=(u->total_login%86400)/3600;
sprintf(line,"~OL%3d~RS~FRd~OL%2d~RS~FRh%2d.",dayst,hourst,u->who_type);
strcat(text,line);
if (total%4==0) strcat(text," \\-\n");
if (total%4==1) strcat(text,"  |\n");
if (total%4==2) strcat(text,".-/\n");
if (total%4==3) strcat(text," |\n");
fprintf(fp,"%s",text);
      }
sprintf(text," ~FR\\.:Total # of users: ~OL%03d ~RS~FR- Record is: ~OL%03d ~RS~FR- Uptime is: ~OL%02d~RS~FRd~OL%02d~RS~FRh - Wizzes:~OL%02d~RS~FR::./\n",total,max_users_was,daysuptime,hoursuptime,wizzes);
fprintf(fp,"%s",text);
fprintf(fp,"~FR/~OL|~RS~FR\\/====)!*#)&%%*&^&%%@#!(!*()%%^^!%%@$)&*^!#%%&*^@#%%^)*(!&@@#%%^$!&!)(@$^!#%%====\\/~OL|~RS~FR\\\n");
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani HELL .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
	}
	else showfile(user, filename);
}

/* Tento alternativny who wypis ani nie je alternativny :> (R) */
void who_alt8(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt,total,invis,mins/* ,remote */,idle,i;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char rname[ROOM_NAME_LEN+1],portstr[5];
char filename[90];
FILE *fp;

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/
sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre OLD .who!\n");
	return;
	}

total=0;invis=0;/* remote=0; */

sprintf(text,"\n~OL~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ~FMCurrent users ~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
fprintf(fp,"%s",text);
fprintf(fp,"~OL Name and description                      Room               Level     Min/Idle\n\n");

for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      
      if (u->port==port[0]) strcpy(portstr,"MAIN");
      	else strcpy(portstr," WIZ");
           
      ++total;
      if (!u->vis) {
            ++invis;
            if (u->level>user->level) continue;
            }
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

      if (u->afk) sprintf(line," ~FR%s ~FM%s~RS~FW",u->name,u->desc);
	else sprintf(line," ~OL~FR%s ~FM%s~RS~FW",u->name,u->desc);

	

      if (!u->vis) line[0]='*';
      if (u==user) line[0]='>'; /* */
      strcpy(rname,u->room->name);
      
      i=0; 
      while (rname[i]!='\0') {
      	if (rname[i]=='_') rname[i]=' ';
      	i++;
      	}

      
      cnt=colour_com_count(line);
      sprintf(text,"%-*s ~RS~FM: ~FT%-16s ~FM: ", 40+cnt,line,rname);
      fprintf(fp,"%s",text);
      if (u->level >= WIZ) sprintf(text,"~OL~FM%-7s ",u->cloak?level_name[u->cloak]:level_name[u->level]);
        	else sprintf(text,"~RS~FM%-7s ",u->cloak?level_name[u->cloak]:level_name[u->level]);
      fprintf(fp,"%s",text);      
      if (idle>14) { sprintf(text, "~RS~FM: %3d/",mins); /* 15 m. idle ? Juj! */
      		  	fprintf(fp,"%s",text);
		  	sprintf(text, "~OL~FM%d\n",idle); 
		  } /* Preto tak divne, lebo /~OL mi neslo ;))) */
		  
           else sprintf(text,"~RS~FM: %3d/%d\n",mins,idle);
      fprintf(fp,"%s",text);      
      }
sprintf(text,"\n~OL~FR=-=-=-=-= ~RS~FTThere are ~OL~FG%-3d ~RS~FTvisible, ~OL~FG%-3d ~RS~FTinvisible users, total ~OL~FY%-3d ~RS~FTusers~OL~FR -=-=-=-=-=\n",num_of_users-invis,invis,total);
fprintf(fp,"%s",text);
fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani OLD .who!!!\n");  break;
	                  case 1: user->misc_op=2;
				}
	}
	else showfile(user, filename);
}


void who_alt9(user) /* Winamp! It REALLY WHIPS a lamer's ass! :-) */
UR_OBJECT user;
{
UR_OBJECT u;
int total,mins,idle;
char line[USER_NAME_LEN+(COOLBUF_LEN*2)];
char pomocna[100];
FILE *fp;
char filename[90];

/* if ((word_count==2) && (!strcmp(word[1],"notify"))) {
	who_from_notify_is_online(user);
	return;
	}
*/	
total=0; 

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	fclose(fp);
	deltempfile(filename);
	}
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
	write_user(user,"Prepac, nemozno otvorit docasny subor pre WINAMP .who!\n");
	return;
	}

fprintf(fp,"           ~OL~FK_________________________________________________________\n");
fprintf(fp,"          ~OL~FK| ~FY================== ~FWAtlantis Playlist ~FY=============== ~FW^^~FYX~FK|\n");
fprintf(fp,"          ~OL~FK|.--------------------------------------------------------|\n");
 
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60;
      ++total;
      if (!u->vis) if (u->level>user->level) continue;
            
      if ((word_count>1) && (!user->login) && !search_success(user,u,"",word_count)) continue;

	sprintf(pomocna,"%s %s", u->name, u->desc);
	colour_com_strip(pomocna);
	strcpy(line,"");
	if (u->afk) sprintf(line,"          ~OL~FK|| ~RS~FG%03d. %-39.39s %4d/%-3d~FW| ~OL~FK|\n", total, pomocna, mins, idle);
		else if (u==user) sprintf(line,"          ~OL~FK||~OL~FG~BB %03d. %-39.39s %4d/%-3d~BK~RS~FW| ~OL~FK|\n", total, pomocna, mins, idle);
			else sprintf(line,"          ~OL~FK|| ~OL~FG%03d. %-39.39s %4d/%-3d~RS~FW| ~OL~FK|\n", total, pomocna, mins, idle);
	fputs(line,fp);
     }

fprintf(fp,"          ~OL~FK||~RS~FW------------------------------------------------------'-~OL~FK|\n");
fprintf(fp,"          ~RS~FW|| ~BW~FK Add ~BK ~BW Rem ~BK ~BW Sel ~BK ~BW Msc ~BK~FW___~OL~FY%03d/%03d~RS~FW________________|    ~OL~FT^~FK|\n", total, max_users_was);
fprintf(fp,"          ~RS~FW|| ~OL~FK^^^^^^^^^^^^^^^^^^^^^^^~BB~OL~FY|<~BK ~BB>~BK ~BB||~BK ~BB[]~BK ~BB>|~BK ~BB^~BK ~RS~FW_______~OL~FG:~RS~FW__|   ~FT//~OL~FK|\n");
fprintf(fp,"           ~RS~FW`---------------------------------------~OL~FK-----------------'\n");

fclose(fp);

if (user->pagewho) {
	switch(more(user,user->socket,filename)) {
	                  case 0: write_user(user,"Chyba pri citani WINAMP .who!!!\n");  break;
	                  case 1: user->misc_op=2;
	                  }
		}
	else showfile(user, filename);

}

int search_success(UR_OBJECT user,UR_OBJECT u,char *lookat,int words)
{
char pomocna[4096];
int is_in_not;
 
 is_in_not=check_notify_user(user,u);
 sprintf(pomocna,"%s %s %s %s [%s] %s [%s] [%s] %s %s %s",u->name, u->desc, u->room->name, u->cloak?level_name[u->cloak]:level_name[u->level], (u->sex?"M":"Z"), ((u->room->group==2)?"portalis":"atlantis"), (u->akl?"AKLIENT":""), ((u->statline==CHARMODE)?"STATLINE":""),u->igngossip?"":"gossip",is_in_not?"notify":"",colour_com_strip2(lookat,0));
 if (!strstr(pomocna,word[1])) return 0;
 if (words==3 && !strstr(pomocna,word[2])) return 0;
 return 1;
}

/* Spakky: who from  
 vypise userov zo zadanej site/alebo userov z rovnakej site ako ma dany user  
 */  
void who_from(user)
UR_OBJECT user;  
{  
char odkial[101];
UR_OBJECT u;
UR_OBJECT uu;

if (word_count<2) {  
	write_user(user,"Pouzi: .wfrom <uzivatel> | <adresa> | samesite\n");  
	return;  
	}  
if (!strncmp(word[1],"same",4)) {
  int num,total=0;
  for(u=user_first;u!=NULL;u=u->next) u->temp=0;
  for(u=user_first;u!=NULL;u=u->next) {  
    if (u->type!=USER_TYPE || u->login || u->temp==-1) continue;
    num=0;
    sprintf(text,"~OL%s~RS: %s",u->site,u->name); 
    for(uu=u->next;uu!=NULL;uu=uu->next) {
      if (uu->type!=USER_TYPE || uu->login || uu->temp==-1) continue;
      if (!strcmp(u->site,uu->site)) {
        u->temp++;
        uu->temp=-1;
        strcat(text,", ");
        strcat(text,uu->name);
        num++;
       }
     }
    if (num>0) {
      strcat(text,"\n");
      write_user(user,text);
      total++;
     }
   }
  if (total==0) write_user(user,"~FTKazdy uzivatel ide z unikatnej domeny.. divne.\n");
  return;
 }

sprintf(odkial,"%s",word[1]);	  
if ((uu=get_user(word[1]))==NULL) {  
	sprintf(text,"~FTZ domeny ~OL%s~RS~FT su prihlaseni nasledovni uzivatelia:\n",odkial);
	write_user(user,text);  
	for(u=user_first;u!=NULL;u=u->next) {  
		if (u->type!=USER_TYPE || u->login) continue;  
		if (strstr(u->site, odkial)) {  
			sprintf(text,"~OL~FY%-12s ~RS~FR[~OL~FR%s~RS~FR]\n",u->name, u->site);
			write_user(user,text);  
			}  
		}  
	}  
  else {  
  	sprintf(text,"~FTNasledovni uzivatelia idu z rovnakej domeny ako ~OL%s~RS\n",uu->name);
  	write_user(user,text);  	
  	for(u=user_first;u!=NULL;u=u->next) {  
   		if (u->type!=USER_TYPE || u->login) continue;  
   		if (!(strcmp(u->site, uu->site)) && u!=uu) {  
   			sprintf(text,"~OL~FY%s\n",u->name);  
   			write_user(user,text);  
   			}  
		}   		  
	}  
}  
  
/*Spakky - insult user*/  
void insult_user(user)  
UR_OBJECT user;  
{  
UR_OBJECT u;  
char veta[85], *name;  
  
if (word_count<2) {  
	write_user(user,"Pouzi: .insult <uzivatel>\n");  
	return;  
	}  
if (user->muzzled) {  
	write_user(user,"Si umlcany, nemozes ani urazat! :)\n");  
	return;  
	}	  
if ((u=get_user(word[1]))==NULL) {  
	write_user(user, notloggedon);  
	return;  
	}  
if (u==user) {
	write_user(user,"Urazat sam seba je piaty priznak sialenstva!\n");  
	return;  
	}	  
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
  
switch ((rand()%12)+1) {
	case 1: sprintf(veta,"Niektori piju z fontany poznania, akurat %s stale iba klokta!\n",u->name); break;  
	case 2: sprintf(veta,"%s zabije jeden a pol dna sledovanim '24 hodin vo svete'!\n",u->name); break;
	case 3: sprintf(veta,"%s ziari ako Aliaska v Decembri!\n",u->name); break;  
	case 4: sprintf(veta,"%s je %s pre prirodnu deselekciu!\n",pohl(u,"hlavnym kandidatom","hlavnou kandidatkou"),u->name); break;  
	case 5: sprintf(veta,"Pocas evolucie %s %s predkov v dozornej komisii!\n",pohl(u,"mal","mala"),u->name); break;
	case 6: sprintf(veta,"%s %s z vlastneho rodokmena!\n",u->name, pohl(u,"vypadol","vypadla")); break;  
	case 7: sprintf(veta,"%s ma dva mozgy: jeden sa kdesi stratil a ten druhy ho hlada!\n",u->name);break;  
	case 8: sprintf(veta,"%s je tak %s, az vytvara ozvenu!\n",u->name,pohl(u,"duty","duta"));break;  
	case 9: sprintf(veta,"Ak by hlupost nadnasala, %s by sa %s ako Zeppelin!\n",u->name,pohl(u,"vznasal","vznasala"));break;
	case 10:sprintf(veta,"Ak vedla teba stoji %s dost blizko, pocujes dunenie oceanu!\n",u->name);break; 
	case 11:sprintf(veta,"Ak by %s %s by robit vyhybkara na Mliecnej drahe!\n",u->name,pohl(u,"bol taky vysoky ako blby, mohol","bola taka vysoka ako hlupa, mohla"));
	default: sprintf(veta,"Dlabat na to...\n");
	}  
sprintf(text,"~FTInzultujes~FW: %s~RS",veta);  
write_user(user,text);  
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FT~BK%s inzultuje~FW: %s~RS",name,veta);
write_room_except(user->room,text,user);  
}  
  
  
/*Spakky: change user name*/  
void chname(user)
UR_OBJECT user;
{
UR_OBJECT u;
char newpwd[20],meno[14],tempname[USER_NAME_LEN+5];
int newpass,oflajn,cnt=0;
unsigned int i;

if (word_count<3) {
	write_user(user,"Pouzi: .chname <stare_meno> <nove_meno>\n");
	return;
	}

if (strlen(word[2])>USER_NAME_LEN || strlen(word[1])>USER_NAME_LEN) {
	write_user(user,"Prilis dlhe meno!\n");
	return;
	}

      if (strlen(word[2])<2) {
            write_user(user,"Prilis kratke meno.\n");
            return;
            }
            
      if (contains_swearing2(word[2])) {
      	    write_user(user,"Meno nemoze obsahovat nadavky!\n");
      	    return;      	       	           	   
       	    }
       	
/*      if (spravny_predmet(word[2])) {      	    
      	    write_user(user,"Toto je nazov predmetu v Atlantide, nemoze byt pouzity ako meno.\n");
      	    return;
      	    } 
*/
      for (i=0;i<strlen(word[2]);++i) {
            if (!isalpha(word[2][i])) {
                  write_user(user,"V mene su povolene len pismena.\n");
                  return;
                  }
            }
      sstrncpy(tempname,word[2],USER_NAME_LEN);
      tempname[0]=tolower(tempname[0]);	    
      cnt=0;
      for (i=0;i<strlen(tempname);i++)
        if (tolower(tempname[i])!=tempname[i]) cnt++;
      if (cnt>1) {
        write_user(user,"Meno moze obsahovat okrem prveho len jedno velke pismeno.\n");
        return;
       } 
      word[2][0]=toupper(word[2][0]);
      if (jdb_info(DB_BAN,word[2])!=NULL) {
          sprintf(text,"Tento nick bol vybanovany!\n");
          return;
	  }
      if (user_banned(word[2])) {
      	sprintf(text,"Na meno '%s' nemozes premenovat.\n",word[2]);
      	write_user(user,text);
      	return;
       }            

/*if (strlen(word[3])>12) {
	write_user(user,"Prilis dlhe heslo!\n");
	return;
	}
*/
word[1][0]=toupper(word[1][0]);

newpass=100+rand()%900;
sprintf(newpwd,"4%3d",newpass);

if (!strcasecmp(word[1],word[2])) {
  if ((u=get_user(word[1]))!=NULL) {
    sprintf(u->name,"%s",word[2]);
    sprintf(text,"~OL~FG%s~RS~FG Ti zmenil%s meno na ~FT~OL%s~RS~FG.\n~OL~FWTvoje heslo bolo nastavene na \"~FR%s~FW\", BEZODKLADNE si ho zmen (prikaz .passwd).\n",user->name,pohl(user,"","a"),u->name,newpwd);
    write_user(u,text);
    strcpy(u->pass,(char *)md5_crypt(expand_password(newpwd),u->name));
    sprintf(text,"Meno zmenene na: ~FT~OL%s~RS~FG. Jeho nove heslo je \"%s\".\n",u->name,newpwd);
    write_user(user,text);
    save_user_details(u,1);
/*    sprintf(oldfile,"%s/%s.M",MAILFILES,word[1]);
    sprintf(newfile,"%s/%s.M",MAILFILES,word[2]);
    rename(oldfile,newfile);
    sprintf(oldfile,"%s/%s.N",NOTESDIR,word[1]);
    sprintf(newfile,"%s/%s.N",NOTESDIR,word[2]);
    rename(oldfile,newfile);
    sprintf(oldfile,"whos/%s.who",word[1]);
    sprintf(newfile,"whos/%s.who",word[2]);
    rename(oldfile,newfile);
    sprintf(oldfile,"xamines/%s.ex",word[1]);
    sprintf(newfile,"xamines/%s.ex",word[2]);
    rename(oldfile,newfile);*/
    return;
   }
 }

/*sprintf(oldfile,"%s/%s.D",USERFILES,word[1]);
sprintf(newfile,"%s/%s.D",USERFILES,word[2]);

if (!(fp=ropen(oldfile,"r"))) {
	sprintf(text,"Uzivatel %s neexistuje!\n",word[1]);
	write_user(user,text);
	return;
	}
  else {  fclose(fp);  }
*/

if (!db_user_exists(word[1])) { /* MySQL */
  sprintf(text,"Uzivatel %s neexistuje!\n",word[1]);
  write_user(user,text);
  return;
 }
else strcpy(word[1],db_user_name(word[1]));
/*
if ((fp=ropen(newfile,"r"))) {

	sprintf(text,"Uzivatel %s uz existuje!\n",word[2]);
	write_user(user,text);
	 fclose(fp);
	return;
	}
*/

if (db_user_exists(word[2])) { /* MySQL */
  sprintf(text,"Uzivatel %s uz existuje!\n",word[2]);
  write_user(user,text);
  return;
 }
 
if ((u=get_user(word[1]))==NULL) {
  if ((u=create_user())==NULL) {
    sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
    write_user(user,text);
    write_syslog("CHYBA: Nemozno vytvorit docasny user object v chname().\n",0);
    return;
   }
  sstrncpy(u->name,word[1],12);
  if (!load_user_details(u)) {
    destruct_user(u);
    destructed=0;
    return;
   }
  oflajn=1;
 }
else oflajn=0;

if (user==u) {
  write_user(user,"Seba ?? Na to zabudni.\n");
  return;
 }
if (user->level<=u->level) {
  write_user(user,"Tak toto asi nepuojde.\n");
  sprintf(text,"WARNING! %s sa pokusil premenovat %s na %s!\n",user->name,u->name,word[2]);
  write_syslog(text,1);
  if (oflajn==0 && u->level>=GOD) {
    sprintf(text,"POZOR! %s sa ta pokusil premenovat na: %s\n",user->name,word[2]);
    write_user(u,text);
   }
  return;
 }

if (!strcmp(u->prevname,"x")) {
  sprintf(text,"Uzivatel %s uz bol premenovany aj vrateny. Uz ho nemozes premenovat.\n",word[1]);
  write_user(user,text);
  if (oflajn==1) {
    destruct_user(u);
    destructed=0;
   }
  return;
 }
if (u->prevname[0] && strcmp(u->prevname,word[2])) {
  sprintf(text,"Uzivatel %s uz bol premenovany. Mozes mu iba vratit povodne meno: %s.\n",word[1],u->prevname);
  write_user(user,text);
  if (oflajn==1) {
    destruct_user(u);
    destructed=0;
   }
  return;
 }

if (oflajn==1) {
  destruct_user(u);
  destructed=0;
 }

/*if ((temp=jdb_info(JDB_JAIL,word[1]))!=NULL) {
  sscanf(temp,"%s", meno);
  temp=remove_first(temp);
  endstring(temp);
  jdb_vyrad(JDB_JAIL,word[1]);
  jdb_zarad(JDB_JAIL,word[2],meno,temp);
 }
*/
/*
sprintf(oldfile,"%s/%s.M",MAILFILES,word[1]);
sprintf(newfile,"%s/%s.M",MAILFILES,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.P",USERFILES,word[1]);
sprintf(newfile,"%s/%s.P",USERFILES,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.notify",USERFILES,word[1]);
sprintf(newfile,"%s/%s.notify",USERFILES,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.MC",MACRODIR,word[1]);
sprintf(newfile,"%s/%s.MC",MACRODIR,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.N",NOTESDIR,word[1]);
sprintf(newfile,"%s/%s.N",NOTESDIR,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.com",PRIKAZDIR,word[1]);
sprintf(newfile,"%s/%s.com",PRIKAZDIR,word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"whos/%s.who",word[1]);
sprintf(newfile,"whos/%s.who",word[2]);
rename(oldfile,newfile);
sprintf(oldfile,"xamines/%s.ex",word[1]);
sprintf(newfile,"xamines/%s.ex",word[2]);
rename(oldfile,newfile);
*/

rename_user_on_list(word[1],word[2]);

/* sprintf(filename,"%s","misc/users.ren");
if(!(fp=ropen(filename,"a"))) {
	write_user(user,"Nemozem zapisat zmenu do users.ren!!!\n");
	return;
	}
fprintf(fp,"%s --> %s\n",word[1],word[2]);
 fclose(fp);
*/

sprintf(text,"%s %s uzivatela %s na %s! Nove heslo: \"%s\".\n",user->name, pohl(user,"premenoval","premenovala"),word[1],word[2],newpwd);
write_syslog(text,1);

if ((u=get_user(word[1]))!=NULL) {
        if (!strcmp(u->prevname,word[2])) strcpy(u->prevname,"x");
        else strcpy(u->prevname,word[1]);
	sprintf(u->name,"%s",word[2]);
	sprintf(text,"~OL~FG%s~RS~FG Ta premenoval%s! Odteraz sa volas ~FT~OL%s~RS~FG.\n~OL~FWTvoje heslo bolo nastavene na \"%s\", BEZODKLADNE si ho zmen (prikaz .passwd).\n",user->name,pohl(user,"","a"),u->name,newpwd);
	write_user(u,text);
	strcpy(u->pass,(char *)md5_crypt(expand_password(newpwd),u->name));
	sprintf(text,"~OL~FW~LIInformacia: ~RS~FGUzivatel ~OL~FG%s ~RS~FGbol premenovany! Jeho nove meno je: ~OL~FT%s~RS~FG.\n",word[1],word[2]);
	write_room_except(NULL,text,u);
	sprintf(text,"Jeho nove heslo je \"%s\".\n",newpwd);
	write_user(user,text);
        save_user_details(u,1);
	}
       else
	{
/*	 sprintf(text,"~OL~FW~LIInformacia: ~RS~FGUzivatel ~OL~FG%s ~RS~FGbol premenovany! Jeho nove meno je: ~OL~FT%s~RS~FG.\n",word[1],word[2]);
	 write_room(NULL,text); */
	 write_user(user,"~OL~FRPOZOR!~FW Uzivatel nieje nahlaseny. Upozorni ho na nove meno a heslo!\n");
 	 sprintf(text,"Jeho nove heslo je \"%s\". Ak ma autoforward zapnuty, heslo mu pride mailom.\n",newpwd);
	 write_user(user,text);

	 if ((u=create_user())==NULL) {
          sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
          write_user(user,text);
          write_syslog("CHYBA: Nemozno vytvorit docasny user object v chname().\n",0);
          return;
         }
	 sstrncpy(u->name,word[1],12);
	 if (!load_user_details(u)) {
          destruct_user(u);
	  destructed=0;
          return;
	 }
 	 strcpy(u->name,word[2]);
	 strcpy(u->pass,(char *)md5_crypt(expand_password(newpwd),u->name));
	 u->socket=-2;
	 strcpy(u->site,u->last_site);
         if (!strcmp(u->prevname,word[2])) strcpy(u->prevname,"x");
         else strcpy(u->prevname,word[1]);
	 save_user_details(u,0);
	 strcpy(meno,u->name);
	 destruct_user(u);
	 destructed=0;
         sprintf(text,"%s Ta premenoval! Tvoje nove meno je: %s, heslo: %s\n",user->name,meno,newpwd);
         send_mail(user,meno,text);
        }
/* pred tym ak nebol user online, heslo sa nezmenilo, takze ostalo zakryptovane
   pod starym nickom takze sa user uz nenalogoval..
   iba ak mu niekto zmenil rucne heslo.. (V) */
}

void sclerotic(user)
UR_OBJECT user;
{
UR_OBJECT u;
char samo[7],spolu[18],heslo[7];
char emailadr[102],name[USER_NAME_LEN+2];

strtolower(word[2]);
if (word_count<2) {
	write_user(user,"Vygeneruje nove heslo a posle uzivatelovi na jeho nastavenu email adresu.\nPouzi: .sklerotic <uzivatel>\n");
	return;
	}
if ((u=get_user_exact(word[1]))!=NULL) {
  write_user(user,"Tento uzivatel nie je skleroticky pretoze je prave nalogovany.\n");
  return;
 }
if ((u=create_user())==NULL) {
  sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
  write_user(user,text);
  write_syslog("CHYBA: Nemozno vytvorit docasny user object v sclerotic().\n",0);
  return;
 }
sstrncpy(u->name,word[1],12);
if (!load_user_details(u)) {
  destruct_user(u);
  destructed=0;
  write_user(user,"Taky sklerotik neexistuje.\n");
  return;
 }
if (user->level<=u->level) {
  destruct_user(u);
  destructed=0;
  write_user(user,"Spravcom sa skleroza netoleruje!\n");
  return;
 }

sstrncpy(emailadr,u->email,100);
if (strpbrk(emailadr," ;$/+*[]\\|<>") || (!strstr(emailadr,"."))
|| (!strstr(emailadr,"@")) || (!isalpha(emailadr[strlen(emailadr)-1]))
|| (strlen(emailadr)<7)) {
  write_user(user,"Asi to nepojde, uzivatel ma krepo nastavenu email adresu!\n");
  return;
 }

sprintf(text,"%s %s sklerotikovi %s nove heslo.\n",user->name,pohl(user,"poslal","poslala"),word[1]);
write_syslog(text,1);

sprintf(text,"%s si sklerotikovi %s nove heslo, pride mu emailom.\n",pohl(user,"Poslal","Poslala"),word[1]);
write_user(user,text);

strcpy(samo, "aeiouy");
strcpy(spolu,"bcdfghjklmnprstvz");		

heslo[0]=spolu[(rand()%17)];
heslo[1]=samo[(rand()%6)];
heslo[2]=spolu[(rand()%17)];
heslo[3]=samo[(rand()%6)];
heslo[4]=spolu[(rand()%17)];
heslo[5]=samo[(rand()%6)];
heslo[6]='\0';

strcpy(u->pass,(char *)md5_crypt(expand_password(heslo),u->name));
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
strcpy(name,u->name);
destruct_user(u);
destructed=0;

strcpy(user->subject,"Zmena hesla na talkeri Atlantis");
sprintf(text,"Ahoj %s,\n\nPoziadal(a) si, alebo niekto v Tvojom mene poziadal o zmenu Tvojho hesla\nna talkeri Atlantis (ak doslo k zneuzitiu, ospravedlnujeme sa).\n\nTvoje nove heslo je: %s\n\n  S pozdravom Tvoj Atlantis.",name,heslo);
send_email(user,emailadr,text);
}

/* ZMENA Spakky - novy prikaz - RANKS, zobrazi dostupne levely, a pri nix
   pocet prikazov (v zatvore je celkovy pocet prikazov pristupnych pre dany level).  
   Vypise aj celkovy pocet prikazov. */
void ranks(user)
UR_OBJECT user;
{
int com, slave=0, citizen=0, soldier=0, warrior=0, sage=0, priest=0, wizard=0, king=0, god=0;
com=0;
while (command[com][0]!='*') {
	switch (com_level[com]) {
		case NEW: slave++; break;
		case CIT: citizen++; break;
		case SOL: soldier++; break;
		case WAR: warrior++; break;
		case SAG: sage++; break;
		case PRI: priest++; break;
		case WIZ: wizard++; break;
		case KIN: king++; break;
		case GOD: god++; break;
		}
	com++;
	}

/** [=]=[=]=[=]=[=]=[=]=[=]=[ Levely obyvatelov Atlantidy: ]=[=]=[=]=[=]=[=]=[=]=[=]\n\n");
                                                         prikazy
                                                        nove spolu total login
    Level 0: OTROK   - to je bezpravny jedinec           %-2d (%-3d) %d \n", slave, slave, SLAVE_TLT);
    Level 1: OBCAN   - pravoplatny obcan Atlantidy       %-2d (%-3d)\n", citizen, slave+citizen); 
**/       


write_user(user,"~RS~FT[=]=[=]=[=]=[=]=[=]=[=]=[ ~OL~FYLevely obyvatelov Atlantidy: ~RS~FT]=[=]=[=]=[=]=[=]=[=]=[=]\n\n");
write_user(user,"                                                          ~OL~FGPRIKAZY      potrebny\n");
write_user(user,"                                                       ~OL~FGnove   celkove  cas (dni)\n");

sprintf(text," ~FTLevel ~OL~FB0: OTROK    ~RS~FW- ~OLto je bezpravny jedinec            ~OL~FB%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", slave, slave, tlt4level[0]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FM1: OBCAN    ~RS~FW- ~OLpravoplatny obcan Atlantidy        ~OL~FM%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", citizen, slave+citizen, tlt4level[1]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FM2: VOJAK    ~RS~FW- ~OLvojak, majuci nasu doveru          ~OL~FM%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", soldier, slave+citizen+soldier, tlt4level[2]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FM3: BOJOVNIK ~RS~FW- ~OLsilny a odvazny vodca bojovnikov   ~OL~FM%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", warrior, slave+citizen+soldier+warrior, tlt4level[3]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FM4: MUDRC    ~RS~FW- ~OLinteligencia Atlantidy             ~OL~FM%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", sage, slave+citizen+soldier+warrior+sage, tlt4level[4]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FM5: KNAZ     ~RS~FW- ~OLvazena a vplyvna osobnost          ~OL~FM%-2d     (%-3d)      ~OL~FR%d~RS~FW\n", priest, slave+citizen+soldier+warrior+sage+priest, tlt4level[5]);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FY6: MAG      ~RS~FW- ~OLcarodejnik ovladajuci vyssiu magiu ~OL~FY%-2d     (%-3d)    ~RS~FR(.faq)~RS~FW\n", wizard, slave+citizen+soldier+warrior+sage+priest+wizard);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FY7: KRAL     ~RS~FW- ~OLjeden z vladcov, prava ruka Bohov  ~OL~FY%-2d     (%-3d)    ~RS~FR(.faq)~RS~FW\n", king, slave+citizen+soldier+warrior+sage+priest+wizard+king);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FY8: BOH      ~RS~FW- ~OLbozski stvoritelia Atlantidy       ~OL~FY%-2d     (%-3d)    ~RS~FR(.faq)~RS~FW\n", god, slave+citizen+soldier+warrior+sage+priest+wizard+king+god);
write_user(user,text);

sprintf(text," ~FTLevel ~OL~FY9: SASO     ~RS~FW- ~OLdvorny zabavac Atlantidy :-)~RS~FW\n");
write_user(user,text);

sprintf(text,"\n~RS~FT[=]=[=]=[=]=[=]=[=]=[=]=[=]=[  ~OL~FTSpolu %-3d prikazov  ~RS~FT]=[=]=[=]=[=]=[=]=[=]=[=]=[=]\n",slave+citizen+soldier+warrior+sage+priest+wizard+king+god);
write_user(user,text);			
}	

/* ZMENA Sign command from Spakky-vykresli text do tabulecky. Na oznamovanie
   dolezitych sprav. Usage: .sign sprava
   Max. 76 znakov aby to vyslo na obrazovku. Cute! */
void sign(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
char *name;
char hv[90];
unsigned int i;


sprintf(hv,"                                                                                \n");
if (user->muzzled) {
      write_user(user,"Si umlcany - nemozes pisat tabulky ...\n");  return;
      }
      
if (word_count<2) {
      write_user(user,"Pouzi: .sign <sprava>\n");  return;
      }
if (strlen(inpstr)-(colour_com_count(inpstr))>76)  {
      write_user(user,"Sprava moze mat max. 76 znakov.\n"); return;
      }
if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s ukazat skaredu tabulku! ;]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
      }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
      }
for(i=0;i!=((strlen(inpstr)-(colour_com_count(inpstr)))+4);++i) {
	if (i%2==0) hv[i]='=';
		else hv[i]='-';
	}
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text, "~FR%s ~FW%s tabulku:\n", name,pohl(user,"napisal","napisala"));
write_room(user->room,text);
sprintf(text, "~FY%s", hv);
write_room(user->room,text);
sprintf(text,"~FY|~FW %s ~RS~FY|\n",inpstr);
write_room(user->room,text);
sprintf(text, "~FY%s", hv);
write_room(user->room,text);
sprintf(text, "~FT\252C2%s %s tabulku: ~RS~FW%s\n", name,pohl(user,"napisal","napisala"),inpstr);
record(user->room,text);
}

/*** HELPY - zmena (S), mozne skracovanie prikazov, a uz sa berie LEN
  z pomocfiles a nie helpfiles (sorry, nemal to kto prekladat
  a boli s tym len problemy ;-((((( )
  Safra, o preklade nechcem nic ANI POCUT!!! (R) :>> *******/
  
void help(user,flag)
UR_OBJECT user;
int flag;
{
int ret,i,len;
/* FILE *fp; */
char filename[120],name[51];
int found=0;

if (word_count<2 && !flag) {
      sprintf(filename,"%s/prvapomoc",POMOCFILES);  /* Widaut .sk ekstenzn! */
      if (!(ret=more(user,user->socket,filename))) {
            write_user(user,"Prva pomoc nie je pristupna.\n");
            return;
            }       
      if (ret==1) user->misc_op=2;
      return;
   }     

if (flag) strcpy(word[1],"obrazky");

if (strlen(word[1])>50) {
	write_user(user,"Nespravna specifikacia helpu.\n");
	return;
	}

if
	(
	 !strcasecmp(word[1],"c") ||
	 !strcasecmp(word[1],"commands") ||
	 !strcasecmp(word[1],"p") ||
	 !strcasecmp(word[1],"prikazy")
	)
	{
		help_commands(user,word[1][0]=='p'?0:1);
		return;
	}

if
	(
	 !strcasecmp(word[1],"credits") ||
	 !strcasecmp(word[1],"kredit")
	)
	{
		help_credits(user);
		return;
	}

/*
 if (!strcasecmp(word[1],"commands")) {  help_commands(user,1);  return;  }
 if (!strcasecmp(word[1],"c"))        {  help_commands(user,1);  return;  }
 if (!strcasecmp(word[1],"credits"))  {  help_credits(user);  return;  }
 if (!strcasecmp(word[1],"prikazy")) {  help_commands(user,0);  return;  }
 if (!strcasecmp(word[1],"p"))       {  help_commands(user,0);  return;  }
 if (!strcasecmp(word[1],"kredit"))  {  help_credits(user);  return;  }
*/

if
	 (
	  !strcasecmp(word[1],"font") ||
	  !strcasecmp(word[1],"fonty") ||
	  !strcasecmp(word[1],"fonts")
	 )
	 {
		 help_fonts(user);
		 return;
	 }

strncpy(name,word[1],50);
name[50]='\0';
i=0; com_num=-1;
len=strlen(word[1]);
while(command[i][0]!='*') {
  if (!strncasecmp(command[i],word[1],len)) {
    strcpy(name,command[i]);
    com_num=i;
    found=1;
    break;
   } /* Ow jes, it ez e koment, boj! Bad nau widaut ekstenzn (.sk), kos idis e litle big wejsd! */
  ++i;
 }
if (!found) {
  i=0;
  while (command_sk[i][0]!='*') {
    if (!strncasecmp(word[1],command_sk[i],len)) {
      com_num=sk_to_en_com[i];
      strcpy(name,command[com_num]);
      found=1;
      break;
     }
    i++;
   }
 }

if (found && com_level[com_num]>user->level) {
  sprintf(text,"~RS~FWPrikaz ~OL%s~RS ma level ~OL%s~RS a vyssie.\n",command[com_num],level_name[com_level[com_num]]);
  write_user(user,text);
  return;
 }

/* sprintf (query,"select `help` from `help` where `name` like ('%s%%') and `level`<='%d' order by `name`,`lang%s`,`level` desc;",name,user->level,user->lang==0?"":" desc");

 if (!(result=mysql_result(query)))
  { write_user(user,"Chyba: Nastal problem s citanim helpu.\n"); return; }
 if ((row=mysql_fetch_row(result))) {
   sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
   if (!(fp=ropen(filename,"w"))) {
     mysql_free_result(result);
     write_user(user,"Chyba: Nastal problem s citanim helpu.\n");
     return;
    }
   if (!(row[0])) {
     mysql_free_result(result);
     write_user(user,"Chyba: Nastal problem s citanim helpu.\n");
     return;
    }
   fputs(row[0],fp);
   mysql_free_result(result);
   if (found) {  
     sprintf(text,"~RS~FWTento prikaz ma level ~OL%s ~RS~FWa vyssie.\n\n",level_name[com_level[com_num]]);
     fputs(text,fp);
    }          
   fclose(fp);
  }
 else {
   mysql_free_result(result);
   write_user(user,"Pomoc k takemu prikazu sa nenasla.\n");
   return;
  }
 if (!(ret=more(user,user->socket,filename))) write_user(user,"Pomoc k takemu prikazu sa nenasla.\n");
 if (ret==1) user->misc_op=2;
*/
 sprintf (query,"select help from help where name like ('%s%%') and level<='%d' order by name,lang%s,level desc;",name,user->level,user->lang==0?"":" desc");
 if (!(ret=sqlmore(user,user->socket,query))) write_user(user,"Pomoc k takemu prikazu sa nenasla.\n");
 if (ret==1) user->misc_op=222;
 if (found && !user->misc_op) {  
   sprintf(text,"~RS~FWTento prikaz ma level ~OL%s ~RS~FWa vyssie.\n\n",level_name[com_level[com_num]]);
   write_user(user,text);
  }          
}

/* Vypise zoznam fontov z databazy */
void help_fonts(user)
	UR_OBJECT user;
{
	FILE *fp;
	char filename[81];
	char query[]="SELECT `fontid`, `fontname` FROM `fonts` WHERE `enabled`='Y' ORDER BY `fontid`";
	char querylongestname[]="SELECT MAX(LENGTH(`fontname`)) as `longestname` FROM `fonts` WHERE `enabled`='Y'";
	char querylongestid[]="SELECT LENGTH(MAX(`fontid`)) as `longestid` FROM `fonts` WHERE `enabled`='Y'";
	MYSQL_RES *result;
	MYSQL_ROW row;

	sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);

	if (!(fp=ropen(filename,"w"))) {
		write_user(user,"Nepodarilo sa vytvorit docasny subor!\n");
		return;
	}

	/* Zistime dlzku najdlhsieho nazvu fontu - kvoli zarovnavaniu stlpcov */
	if (!(result=mysql_result(querylongestname))) {
		write_user(user,"Nepodarilo sa nacitat zoznam fontov!\n");
		return;
	}

	row=mysql_fetch_row(result);
	int longest_fontname=atoi(row[0]);
	mysql_free_result(result);

	/* Zistime dlzku najdlhsieho id fontu - kvoli zarovnavaniu stlpcov */
	if (!(result=mysql_result(querylongestid))) {
		write_user(user,"Nepodarilo sa nacitat zoznam fontov!\n");
		return;
	}

	row=mysql_fetch_row(result);
	int longest_fontid=atoi(row[0]);
	mysql_free_result(result);

	/* Nacitame zoznam fontov z databazy a zapiseme do array */
	if (!(result=mysql_result(query))) {
		write_user(user,"Nepodarilo sa nacitat zoznam fontov!\n");
		return;
	}

	int rows=mysql_num_rows(result);
	char **fontids;
	if ((fontids=malloc(sizeof(char*)*rows))==NULL) {
		write_user(user,"Prepacte poruchu, zavada nie je vo vasom prijimaci.\n");
		write_level(KIN,1,"~OL~FRVAROVANIE: ~FWNepodarilo sa alokovat pamat pre zoznam fontov!\n",NULL);
		return;
	}
	char **fontnames;
	if ((fontnames=malloc(sizeof(char*)*rows))==NULL) {
		write_user(user,"Prepacte poruchu, zavada nie je vo vasom prijimaci.\n");
		write_level(KIN,1,"~OL~FRVAROVANIE: ~FWNepodarilo sa alokovat pamat pre zoznam fontov!\n",NULL);
		free(fontids);
		return;
	}

	int i=0;

	while((row=mysql_fetch_row(result))) {
		if ((fontids[i]=malloc(sizeof(char)*(strlen(row[0])+1)))==NULL) {
			write_user(user,"Prepacte poruchu, zavada nie je vo vasom prijimaci.\n");
			write_level(KIN,1,"~OL~FRVAROVANIE: ~FWNepodarilo sa alokovat pamat pre zoznam fontov!\n",NULL);
			free(fontids);
			free(fontnames);
			return;
		}
		if ((fontnames[i]=malloc(sizeof(char)*(strlen(row[1])+1)))==NULL) {
			write_user(user,"Prepacte poruchu, zavada nie je vo vasom prijimaci.\n");
			write_level(KIN,1,"~OL~FRVAROVANIE: ~FWNepodarilo sa alokovat pamat pre zoznam fontov!\n",NULL);
			free(fontids);
			free(fontnames);
			return;
		}
		strcpy(fontids[i],row[0]);
		strcpy(fontnames[i],row[1]);
		i++;
	}

	mysql_free_result(result);

	/* Kolko stlpcov a riadkov bude mat vystup */
	int columns=(int)(80/(longest_fontid+longest_fontname+2));
	if (columns==0) columns=1;
	if (columns>1) if (columns*(longest_fontid+longest_fontname+2)+(columns-1)*2 > 80) columns--;
	int fonts_per_column=(int)(rows/columns);
	if (fonts_per_column*columns!=rows) fonts_per_column++;

	i=0;
	int curid=0;

	/* Zapise vsetko do suboru */
	while (i<fonts_per_column) {
		for (int j=0; j<columns; j++) {
			/* fonty zapisujeme pod seba, t. j. prvy stlpec 1, 2, 3, ... n, druhy stlpec n+1, n+2, n+3 ... n+n atd*/
			curid=i+j*fonts_per_column;
			if (curid>=rows) break;
			fprintf(fp,"%*s. %-*s",longest_fontid,fontids[curid],longest_fontname,fontnames[curid]);
			if (j<(columns-1)) fprintf(fp,"  ");
		}
		fprintf(fp,"\n");
		i++;
	}

	fclose(fp);

	free(fontnames);
	free(fontids);

	/* zobrazime subor userkovi */
	if (user->pagewho) {
		switch(more(user,user->socket,filename)) {
			case 0: write_user(user,"Chyba pri citani zoznamu fontov!\n");  break;
			case 1: user->misc_op=2;
		}
	} else {
		showfile(user,filename);
	}
	return;
}

/* Cita spravy, ktore uzivatelia pisu do suborov... ZMENA */
void read_notices(user)
UR_OBJECT user;
{
char filename[150];

if (user->level>=GOD && word_count>1) {    /* tajny stuff... */
  sprintf(filename,"%s",word[1]);
  switch(more(user,user->socket,filename)) {
    case 0: write_user(user,"Subor nenajdeny\n"); break;
    case 1: user->misc_op=2;
   }
  return;
 }
sprintf(filename,"%s",NOTICEBOARD);
switch(more(user,user->socket,filename)) {
  case 0: write_user(user,"Nemozem sa dostat k suboru NOTICEBOARD.\n"); break;
  case 1: user->misc_op=2;
 }
}

/* spakky - commands vypise prikazy vsetky, podobne ako v mude. */
void commands(user,engl)
UR_OBJECT user;
int engl;
{
int com,cnt;
char temp[30];
char pom[30];
int lev;
char *comm;

sprintf(text,"\n~OL~FM=-=-=-=-=-=-=-=-=-=-= ~RS~FTPrikazy pristupne pre level: ~OL~FY%-7s ~FM=-=-=-=-=-=-=-=-=-=-=\n\n",level_name[user->level]);
write_user(user,text);
      com=0;  cnt=0;  text[0]='\0';
      while(1) {
            if (engl) {
              comm=command[com];
              lev=com_level[com];
              strcpy(pom,command[com]);
             }
            else {
              comm=command_sk[com];
              lev=com_level[sk_to_en_com[com]];
              strcpy(pom,command[sk_to_en_com[com]]);
             }
            if (comm[0]=='*') break;
            if (lev>user->level) {  com++;  continue;  }            
            if (user->room!=NULL && user->room->group!=4 
            && (!strcmp(pom,"hide") || !strcmp(pom,"buy") || !strcmp(pom,"sell") || !strcmp(pom,"gold")))
             { com++; continue; }
            if (lev==user->level) {           
            	sprintf(pom,"~OL~FR%s",comm); }
            else sprintf(pom,"~RS~FW%s",comm);
            if (!strcmp(comm,"to")) strcat(pom," /");
            if (!strcmp(comm,"tell")) strcat(pom," >");
            if (!strcmp(comm,"pemote")) strcat(pom," <");
            if (!strcmp(comm,"echo")) strcat(pom," -");
            if (!strcmp(comm,"shout")) strcat(pom," !");
            if (!strcmp(comm,"gossip")) strcat(pom," @");            
            if (!strcmp(comm,"reply")) strcat(pom," :");
            if (!strcmp(comm,"emote")) strcat(pom," ;");
            if (!strcmp(comm,"semote")) strcat(pom," #");
            if (!strcmp(comm,"call")) strcat(pom," ,");
            if (!strcmp(comm,"quest")) strcat(pom," $");
            sprintf(temp,"%-16s ", pom);
            strcat(text,temp);
            if (cnt==6) {
                  strcat(text,"\n");
                  write_user(user,text);
                  text[0]='\0';  cnt=-1;
                  }
            com++;
            cnt++;
            }
      if (cnt) {
            strcat(text,"\n");  write_user(user,text);
            }
      
write_user(user,"\nNapis ~OL.pomoc <meno prikazu>~RS na ziskanie viac informacii o prikaze.\n");
sprintf(text,"Prikazy pisane ~OL~FRcervenou~RS su prikazy levelu ~OL~FY%s~RS~FW.\n",level_name[user->level]);
write_user(user,text);       	
write_user(user,"Skus este: ~OL.faq~RS pre casto kladene otazky, a ~OL.rules~RS pre zakonnik Atlantidy.\n");
}

/*** Show the command available ***/
void help_commands(user,engl)
UR_OBJECT user;
int engl;
{
int com,cnt;
char temp[40];
char pom[45];
int lev,lvl;
char *comm;

if (!strncmp(word[1],"z",1)) { commands(user,0); return; }
sprintf(text,"\n~RS~FM=<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-= ~RS~FTPrikazy dostupne pre level: ~OL~FW%-7s ~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>=\n",level_name[user->level]);
write_user(user,text);
                      
for(lev=NEW;lev<=user->level;++lev) {
      if (lev==WIZ) continue;
      sprintf(text,"~FR[~OL%s~RS~FR]\n",level_name[lev]);
      write_user(user,text);
      com=0;  cnt=0;  text[0]='\0';
      while(1) {
            if (engl) {
              comm=command[com];
              lvl=com_level[com];
              strcpy(pom,command[com]);
             }
            else {
              comm=command_sk[com];
              lvl=com_level[sk_to_en_com[com]];
              strcpy(pom,command[sk_to_en_com[com]]);
             }
            if (comm[0]=='*') break;
            if (lvl!=lev) {  com++;  continue;  }
            if (user->room!=NULL && user->room->group!=4 
            && (!strcmp(pom,"hide") || !strcmp(pom,"buy") || !strcmp(pom,"sell") || !strcmp(pom,"gold")))
             { com++; continue; }
            strcpy(pom,comm);
            /* (S) sem spravime vypis skratiek k prikazom */ /* okej (V) */
            if (!strcmp(comm,"to")) strcat(pom," /");
            if (!strcmp(comm,"tell")) strcat(pom," >");
            if (!strcmp(comm,"pemote")) strcat(pom," <");
            if (!strcmp(comm,"echo")) strcat(pom," -");
            if (!strcmp(comm,"shout")) strcat(pom," !");
            if (!strcmp(comm,"gossip")) strcat(pom," @");            
            if (!strcmp(comm,"reply")) strcat(pom," :");
            if (!strcmp(comm,"emote")) strcat(pom," ;");
            if (!strcmp(comm,"semote")) strcat(pom," #");
            if (!strcmp(comm,"call")) strcat(pom," ,");
            if (!strcmp(comm,"quest")) strcat(pom," $");
            
            sprintf(temp,"%-11s",pom);
            strcat(text,temp);
            if (cnt==6) {
                  strcat(text,"\n");
                  write_user(user,text);
                  text[0]='\0';  cnt=-1;
                  }
            com++;
            cnt++;
            }
      if (cnt) {
            strcat(text,"\n");  write_user(user,text);
            }
      }
write_user(user,"~FM=<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>-=-<~OL~FY=-=~RS~FM>=\n");
write_user(user,"Prikazy sa musia zacinat '.' a mozu byt skratene. (Stary vypis: .prik zoznam)\nNapis '~FG.pomoc <meno prikazu>~FW' ak chces ziskat viac informacii o prikaze.\n");
}

void help_credits(user)
UR_OBJECT user;
{
sprintf(text,"\n*** The Credits ***\n\nNUTS version %s,\n%s,\nCopyright (C) Neil Robertson 1996.\n\n",VERSION,WORK);
write_user(user,text);

write_user(user,"~BM             ~BB             ~BT             ~BG             ~BY             ~BR             ~RS~BK~FW\n");
write_user(user,"NUTS stands for Neils Unix Talk Server, a program which started out as a\nuniversity project in autumn 1992 and has progressed from thereon. In no\nparticular order thanks go to the following people who helped me develop or\n");
write_user(user,"debug this code in one way or another over the years:\n   ~FTDarren Seryck, Steve Guest, Dave Temple, Satish Bedi, Tim Bernhardt,\n   ~FTKien Tran, Jesse Walton, Pak Chan, Scott MacKenzie and Bryan McPhail.\n");
write_user(user,"Also thanks must go to anyone else who has emailed me with ideas and/or bug\nreports and all the people who have used NUTS over the intervening years.\n");
write_user(user,"I know I've said this before but this time I really mean it - this is the final\nversion of NUTS 3. In a few years NUTS 4 may spring forth but in the meantime\nthat, as they say, is that. :)\n\n");
write_user(user,"If you wish to email me my address is '~FGneil@ogham.demon.co.uk~RS' and should\nremain so for the forseeable future.\n\nNeil Robertson - November 1996.\n");
write_user(user,"~BM             ~BB             ~BT             ~BG             ~BY             ~BR             ~RS~BK~FW\n\n");
}

/*** See review of conversation ***/
void review(UR_OBJECT user, char *inpstr)
{
RM_OBJECT rm=user->room;
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count<2) rm=user->room;
else {
  if (user->level>=WIZ) {
    if ((rm=get_room(word[1],user))==NULL) {
      rm=user->room;
     }
    else {
      inpstr=remove_first(inpstr);
      word_count--;
     }
    if (user->level<GOD && rm!=user->room && (rm->access==GOD_PRIVATE || (rm->access & PRIVATE))) {
      write_user(user,"Ta miestnost je prave PRIVATNA, nemozes si pozriet posledne hlasky.\n");
      return;
     }
   }
 }
if (rm!=user->room && rm->sndproof==1) {
  write_user(user,"Prepac, nemozes si pozerat hlasky vo zvukotesnej miestnosti.\n");
  return;
 }

if (word_count>1) {
  if (!strcmp(word[1],"new")) {
    kolko=rm->revline-user->lastrevi;
    if (kolko<0) kolko=kolko+REVIEW_LINES;
    if (kolko==0) {
      write_user(user,"Od tvojho posledneho reviewu sa v miesntosti nic nehovorilo.\n");
      return;
     }
   }
  else kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (rm->revbuff[(rm->revline+i)%REVIEW_LINES]!=NULL) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(rm->revline+i)%REVIEW_LINES;
  if (rm->revbuff[line]!=NULL) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FWPosledne hlasky v miestnosti %s",rm->name);
      write_user(user,title(text,"~FG"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(rm->revbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom,rm->revbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr)) write_user(user,user->timeinrevt?rm->revbuff[line]:rm->revbuff[line]+6);
        free(pom); 
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVIEW()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"V miestnosti sa este nic nehovorilo...\n");
else write_user(user,"~FG.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
user->lastrevi=rm->revline;
}

void revshout(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0,ok,ii;
char *pom,rum[10];
RM_OBJECT rm;

if (!strncmp(word[1],"atl",3) && user->room!=NULL && user->room->group!=1) {
  clear_words();
  word_count=wordfind(inpstr);
  inpstr=remove_first(inpstr);
 }
else 
if (user->room->group!=1) {

if (word_count>1) {
  if (!strcmp(word[1],"new")) {
    kolko=revporshline-user->lastrevs;
    if (kolko<0) kolko=kolko+REVIEW_LINES;
    if (kolko==0) {
      write_user(user,"Od tvojho posledneho revshoutu nikto nic nekrical.\n");
      return;
     }
   }
  else kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i) {
  if (revporshbuff[(revporshline+i)%REVIEW_LINES][0]) {
    strcpy(rum,"   ");
    rum[0]=revporshbuff[(revporshline+i)%REVIEW_LINES][0];
    rum[1]=revporshbuff[(revporshline+i)%REVIEW_LINES][1];
    rum[2]=revporshbuff[(revporshline+i)%REVIEW_LINES][2];
    ok=0;
    for(rm=room_first;rm!=NULL;rm=rm->next) 
     if (!strcmp(rm->label,rum)) {
       if (user->room==rm) ok=1;
       else {
	 for(ii=0;ii<MAX_LINKS;ii++) 
          if (rm->link[ii]!=NULL && user->room==rm->link[ii]) ok=1;
	}
      }
    if (ok==0) total++;
   }
 }
cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revporshline+i)%REVIEW_LINES;
  if (revporshbuff[line][0]) {
    strcpy(rum,"   ");
    rum[0]=revporshbuff[line][0];
    rum[1]=revporshbuff[line][1];
    rum[2]=revporshbuff[line][2];
    ok=0;
    for(rm=room_first;rm!=NULL;rm=rm->next) 
     if (!strcmp(rm->label,rum)) {
       if (user->room==rm) ok=1;
       else {
	 for(ii=0;ii<MAX_LINKS;ii++) 
          if (rm->link[ii]!=NULL && user->room==rm->link[ii]) ok=1;
	}
      }
    if (ok==0) continue;
    
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FY\252C0Posledne vykriky %s",user->room->where);
      write_user(user,title(text,"~FY"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revporshbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom,revporshbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr)) {
          write_user(user,(user->timeinrevt)?revporshbuff[line]+3:revporshbuff[line]+9);
         }
        free(pom);	   
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVIEW()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Davno tu nikto nekrical.\n");
else write_user(user,"~RS~FY.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
user->lastrevs=revporshline;
return;
}

if (word_count>1) {
  if (!strcmp(word[1],"new")) {
    kolko=revshoutline-user->lastrevs;
    if (kolko<0) kolko=kolko+REVIEW_LINES;
    if (kolko==0) {
      write_user(user,"Od tvojho posledneho revshoutu nikto nic nekrical.\n");
      return;
     }
   }
  else kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (revshoutbuff[(revshoutline+i)%REVIEW_LINES][0]) total++;
cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revshoutline+i)%REVIEW_LINES;
  if (revshoutbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FY\252C0Posledne vykriky");
      write_user(user,title(text,"~FY"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revshoutbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom, revshoutbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr)) write_user(user,user->timeinrevt?revshoutbuff[line]:revshoutbuff[line]+6);
        free(pom);	   
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVSHOUT()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Nikto este nic nekrical.\n");
else write_user(user,"~RS~FY.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
user->lastrevs=revshoutline;
}

/* Revgossip - review of gossips buffer! ***/
void revgossip(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count==2) {
  kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (revgossipbuff[(revgossipline+i)%REVIEW_LINES][0]) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revgossipline+i)%REVIEW_LINES;
  if (revgossipbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FWPosledne klebety a taraniny");
      write_user(user,title(text,"~FR"));
     }
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revgossipbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom, revgossipbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        if (!strlen(inpstr) || strstr(pom,inpstr))write_user(user,user->timeinrevt?revgossipbuff[line]:revgossipbuff[line]+6);
        free(pom);
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVGOSSIP()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Nikomu sa este nechcelo klebetit a tarat...\n");
else write_user(user,"~FR.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
}


/* Revquest - review of quests records ***/
void revquest(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count==2) {
  kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (revquestbuff[(revquestline+i)%REVIEW_LINES][0]) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revquestline+i)%REVIEW_LINES;
  if (revquestbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FWCo sa dosial udialo v queste:");
      write_user(user,title(text,"~FG"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revquestbuff[line])*sizeof(char))+1);
      if (pom!=NULL) {
        strcpy(pom, revquestbuff[line]);
        colour_com_strip(pom);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr))write_user(user,revquestbuff[line]);
        free(pom);
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVGUES()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Zatial sa nic v queste neudialo!\n");
else write_user(user,"~FR.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
}


/*** Revwizshout - review of wizshout buffer! ;-) ***/
void revwizshout(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count==2) {
  kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (revwizshoutbuff[(revwizshoutline+i)%REVIEW_LINES][0]) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revwizshoutline+i)%REVIEW_LINES;
  if (revwizshoutbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~FWPosledne zvolania levelu %s",level_name[user->level]);
      write_user(user,title(text,"~FM"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revwizshoutbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom, revwizshoutbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        /* *** */
  /*	   if (user->level==WIZ && (strstr(pom,"kingom") || strstr(revwizshoutbuff[line],"godom"))) continue; */
        if (user->level==KIN && strstr(revwizshoutbuff[line],"\253Wg") ) continue;
        else if (!strlen(inpstr) || strstr(pom,inpstr))  write_user(user,revwizshoutbuff[line]);
        free(pom);
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVWIZSH()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Strazcovia Atlantisu sa este o nicom nebavili.\n");
else write_user(user,"~OL~FM.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.~RS\n");
}

void revbcast(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count==2) {
  kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVB_LINES<kolko) kolko=REVB_LINES;
 }
else kolko=REVB_LINES;

for(i=0;i<REVB_LINES;++i)
 if (revbcastbuff[(revbcastline+i)%REVB_LINES][0]) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVB_LINES;++i) {
  line=(revbcastline+i)%REVB_LINES;
  if (revbcastbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FWDolezite spravy:");
      write_user(user,title(text,"~FG"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revbcastbuff[line])*sizeof(char))+1);
      if (pom!=NULL) {
        strcpy(pom, revbcastbuff[line]);
        colour_com_strip(pom);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr))write_user(user,revbcastbuff[line]);
        free(pom);
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVBCAST()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Zatial neboli ziadne dolezite spravy.\n");
else write_user(user,"~FG.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
}

/*** Spakky: .last - zobrazenie last logoutov  .last***/
void last(user)
UR_OBJECT user;
{
int i,line,cnt;
char *pom;

cnt=0;
for(i=0;i<LASTLOG_LINES;++i) {
      line=(lastlogline+i)%LASTLOG_LINES;
      if (lastlog[line][0]) {
	    cnt++;
	    if (cnt==1 && word_count==1) {
	      sprintf(text,"~FY~OLPosledni odhlaseni");
	      write_user(user,title(text,"~FY")); 
	     } 
            pom=(char *) malloc ((strlen(lastlog[line])*sizeof(char))+1);
            if (pom!=NULL) {
              strcpy(pom,lastlog[line]);
              colour_com_strip(pom);
   	      strtolower(pom);
  	      if (word_count==1 || strstr(pom,word[1])) write_user(user,lastlog[line]); 
              free(pom);	   
             }
            else {
              sprintf(text,"~OL~FROUT OF MEMORY IN LAST()\n");
              write_level(KIN,1,text,NULL); 
              colour_com_strip(text);
              write_syslog(text,1);
             }
	  } 
      } 
if (!cnt) write_user(user,"Zatial sa nikto neodhlasil.\n"); 
else if (word_count==1) write_user(user,"~FY.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-\n");
} 

void revsos(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,line,cnt,kolko,total=0;
char *pom;

if (word_count==2) {
  kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
 }
else kolko=REVIEW_LINES;

for(i=0;i<REVIEW_LINES;++i)
 if (revsosbuff[(revsosline+i)%REVIEW_LINES][0]) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIEW_LINES;++i) {
  line=(revsosline+i)%REVIEW_LINES;
  if (revsosbuff[line][0]) {
    cnt++;
    if (cnt==1) {
      sprintf(text,"~OL~FWSave Our Souls:");
      write_user(user,title(text,"~FR"));
     }
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(revsosbuff[line])*sizeof(char))+1);
      if (pom!=NULL) {
        strcpy(pom, revsosbuff[line]);
        colour_com_strip(pom);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr))write_user(user,revsosbuff[line]);
        free(pom);
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVSOS()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) write_user(user,"Sos buffer je prazdny.\n");
else write_user(user,"~FR.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
}

/*** Show some user stats 
     (S) Zmena - iny design (tabulecka ;)
     (S) dalsia zmena, vacsia a obsaznejsia tabula :>>     
      ***/
         
void status(user)
UR_OBJECT user;
{
UR_OBJECT u;
char ir[ROOM_NAME_LEN+1],pred1[100],pred2[100];
int days,hours,mins,minst,uj,i,ap;
int opil=0,lamer=0,debil=0;

if (word_count<2 || user->level<WIZ) {
      u=user;
      sprintf(text,"\n~FGStav uzivatela:~FW %s %s\n\n",user->name, user->desc);
      write_user(user,text);
      }
else {
      if (!(u=get_user(word[1]))) {
            write_user(user,notloggedon);  return;
            }
      if (u->level>user->level) {
            write_user(user,"Nemozes si pozerat stav uzivatela s vyssim levelom ako mas sam.\n");
            return;
            }
      sprintf(text,"\n~FGStav uzivatela:~FW %s %s\n\n",u->name, u->desc);
      write_user(user,text);
      }
if (u->invite_room==NULL) strcpy(ir,"<nowhere>");
else strcpy(ir,u->invite_room->name);

uj=0;
strcpy(pred1,"ziadne");
pred2[0]='\0';
for (i=0;i<HANDS;i++)
 if (u->predmet[i]>-1)
  {
   if (uj==0) sprintf(pred1,"%-12s ",predmet[u->predmet[i]]->name);
   if (uj==1) sprintf(pred2,"%-12s ",predmet[u->predmet[i]]->name);
   if (uj==2) strcat(pred1,predmet[u->predmet[i]]->name);
   if (uj==3) strcat(pred2,predmet[u->predmet[i]]->name);
   uj++;
  }
if (u->jailed) uj=1;
else uj=0;
days=u->total_login/86400;
hours=(u->total_login%86400)/3600;
minst=(u->total_login%3600)/60;
mins=(int)(time(0) - u->last_login)/60;
if (u->lieta) i=1; else i=0;
ap=u->ap;
if (u->affpermanent>-1) {
  if (predmet[u->affpermanent]->altfunct & 128) opil=1;
  if (predmet[u->affpermanent]->altfunct & 256) lamer=1;
  if (predmet[u->affpermanent]->altfunct & 512) debil=1;
 }
if (u->affected>-1) {
  if (predmet[u->affected]->altfunct & 128) opil=1;
  if (predmet[u->affected]->altfunct & 256) lamer=1;
  if (predmet[u->affected]->altfunct & 512) debil=1;
 }  
write_user(user,"~OL~FB===~BR~FY IGNORUJE ~BK~FB=============================~BR~FY DALSIE INFO ~BK~FB=========================\n");
write_user(user,"~OL~FB| ~RS~FGAll  Shout  Tell  Sys  Fun  Gossip ~OL~FB|| ~RS~FGUmlcany V base NewMail CharEcho Farby  ~OL~FB|\n");
sprintf(text,"~OL~FB| ~FW%-3s   %-3s    %-3s  %-3s  %-3s   %-3s   ~FB||   ~FW%-3s     %-3s    %-3s     %-3s     %-3s   ~FB|\n", noyes(u->ignall), noyes(u->ignshout), noyes(u->igntell), noyes(u->ignsys), noyes(u->ignfun), noyes(u->igngossip), noyes(u->muzzled!=0), noyes(uj), noyes(has_unread_mail(u)), offon[u->charmode_echo], offon[u->colour!=0]); 
write_user(user,text);
write_user(user,"~OL~FB===~BR~FY PRIVATE INFO ~BK~FB=====================~BR~FY SYSTEM INFO ~BK~FB=============================\n");
sprintf(text,"~OL~FB| ~RS~FGTERM [~OL~FW%-3.3d~FG:~FW%-3.3d~RS~FG]~FB|| ~RS~FGLevel : ~OL~FW%-7s ~OL~FB|| ~RS~FGLinka      : ~OL~FW%-27.27s ~FB|\n",u->lines, u->colms, u->cloak?level_name[u->cloak]:level_name[u->level],u->site);
write_user(user,text);
sprintf(text,"~OL~FB| ~RS~FGTyp .who : ~OL~FW%-2d ~FB|| ~RS~FGPohl. : ~OL~FW%-1.1s       ~FB|| ~RS~FGPosledne z : ~OL~FW%-27.27s ~FB|\n", u->who_type,pohl(u,"M","F"),u->last_site);
write_user(user,text);
sprintf(text,"~OL~FB| ~RS~FGAlarm    : ~OL~FW%-2d ~FB|| ~RS~FGPP/MA : ~OL~FW%3d/%-4d~FB|| ~RS~FGCelkovy cas: ~OL~FW%3d dni, %2d hodin, %2d min.  ~FB|\n",(u->alarm/60),u->pp, u->mana, days, hours, minst);
write_user(user,text);
sprintf(text,"~OL~FB| ~RS~FGAutoFWD  : ~OL~FW%-3s~FB|| ~RS~FGArena : ~OL~FW%5d b ~FB|| ~RS~FGE-mail     : ~OL~FW%-27.27s ~FB|\n", noyes(u->autofwd), ap, u->email);
write_user(user,text);
sprintf(text,"~OL~FB| ~RS~FGWrapLines: ~OL~FW%-3s~FB|| ~RS~FGOnline: ~OL~FW%-3d min ~FB|| ~RS~FGWebstranka : ~OL~FW%-27.27s ~FB|\n", noyes(u->wrap), mins, u->homepage);
write_user(user,text);
write_user(user,"~OL~FB===~BR~FY PREDMETY ~BK~FB=================~BR~FY POUZITE PREDMETY ~BK~FB================================\n");
sprintf(text,"~OL~FB|~OL~FW %-25s~FB|| ~RS~FGZuv. Opit%s  Lamer. Debil. Lep. Prehan. Lsd. Lieta~OL~FB|\n",pred1,pohl(u,"y","a"));
write_user(user,text);
sprintf(text,"~OL~FB|~OL~FW %-25s~FB|| ~OL~FW%-3s   %-3s    %-3s    %-3s   %-3s   %-3s    %-3s   %-3s ~FB|\n",pred2, noyes(u->zuje), noyes(opil), noyes(lamer), noyes(debil), noyes(u->glue), noyes(u->prehana), noyes(user->lsd), noyes(i));
write_user(user,text);
write_user(user,"~OL~FB===~BR~FY VSTUP A VYSTUP ~BK~FB========================~BR~FY UCAST V HRACH ~BK~FB======================\n");
sprintf(text,"~OL~FB|~RS~FGVst fraza : ~OL~FW%-25.25s ~FB||~RS~FGPisk. Geo Lod. Hang. Lab.  DOOM  Quest~OL~FB|\n", u->in_phrase);
write_user(user,text);
sprintf(text,"~OL~FB|~RS~FGVys fraza : ~OL~FW%-25.25s ~FB|| ~FW%-3s  %-3s %-3s   %-3s  %-3s    %-3s   %-3s ~FB|\n",u->out_phrase, noyes(u->game==2), noyes(u->game==3), noyes(u->game==4), noyes(u->hang_stage!=-1), noyes(u->lab!=0), noyes(u->doom_energy>0), noyes((u->quest!=0)&&(u->quest!=4)));
write_user(user,text);
write_user(user,"~OL~FB================================================================================\n");
/* sprintf(text,"remote_com: %d", user->remote_com);
write_user(user,text); */
}


/*** Read your mail ***/
void rmail(user)
UR_OBJECT user;
{
int ret,tot;
char uname[20];

if (!is_number(word[1]) && strncmp(word[1],"new",strlen(word[1]))
&& strncmp(word[1],"last",strlen(word[1]))) {
  strcpy(uname,db_user_name(word[1]));
  if (uname[0]) {
    strcpy(word[1],uname);
   }
  mymail(user,1);
  return;
 }

tot=0;
if (word[1][0]!='l') {
  if (word[1][0]!='n')
   write_user(user,"\n~RS~FM====+~OL~FM+~RS~FM+======~OL~FB/atlantis-mail/ ~OL~FYTVOJA POSTOVA SCHRANKA ~OL~FB/atlantis-mail/~RS~FM======+~OL~FM+~RS~FM+====\n\n");
  else
   write_user(user,"~RS~FM====+~OL~FM+~RS~FM+========~OL~FB/atlantis-mail/ ~OL~FYN O V A  P O S T A ~OL~FB/atlantis-mail/~RS~FM========+~OL~FM+~RS~FM+====\n\n");
 }   

tot=0;  
user->rjoke_from=1;   
user->browsing=2; 
user->messnum=0;

if (word_count==2) {
  if (word[1][0]=='n') {
    user->browsing=10;
    sprintf(query,"select count(msgid) from mailbox where userid='%d' and time<FROM_UNIXTIME('%d')",user->id,(int)user->read_mail);
    user->rjoke_from=query_to_int(query);
    user->rjoke_from++;
   }
  else {
    user->rjoke_from=atoi(word[1]);
    if (user->rjoke_from<=0) user->rjoke_from=1;
   }
 }
if (word_count==3 && word[1][0]=='l') {
        tot=mail_from(user,0);
	user->rjoke_from=tot-atoi(word[2])+1;
	if (user->rjoke_from<=0) user->rjoke_from=1;
	if (user->rjoke_from>tot) user->rjoke_from=tot;
        sprintf(text,"~RS~FM====+~OL~FM+~RS~FM+=======~OL~FB/atlantis-mail/ ~OL~FYPOSLEDNE SPRAVY (%2d) ~OL~FB/atlantis-mail/~RS~FM=======+~OL~FM+~RS~FM+====\n\n",tot-user->rjoke_from+1);
	write_user(user,text);
	}
user->messnum=user->rjoke_from;
ret=mailmore(user);
if (ret==1) user->misc_op=223;
}


/*** Send mail message ***/
void smail(user,inpstr,done_editing)
UR_OBJECT user;
char *inpstr;
int done_editing;
{
UR_OBJECT u;
char emailadr[101],uname[USER_NAME_LEN+2];
int igns=0,uid=0,bigquota=0,mailboxsize=0;

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nikomu posielat postu.\n");  return;
      }
if (done_editing) {
      if (!strcmp(user->mail_to,"atlantis@hotweb.sk")) {  /* milo xcel aby som */
        user->mail_to[0]='\0';                /* toto osetril qoli SMSkam. (V) */
        return;
       }
      send_mail(user,user->mail_to,user->malloc_start);
      send_copies(user,user->malloc_start);
      user->mail_to[0]='\0';
      return;
      }
if (word_count<2) {
      write_user(user,"Komu chces poslat postu?\n");  return;
      }

if (strlen(word[1])>99) {
	write_user(user,"Prilis dlha adresa!\n");
	return;
	}
sstrncpy(emailadr, word[1],99);
if (strstr(emailadr,"@")) { /* Je to E-MAIL sprava. */
		if (strpbrk(emailadr," ;$/+*[]\\<|>")) {
		write_user(user,"Nespravne zadana email adresa!\n");
		return;
		}
	if ((!strstr(emailadr,".")) || (!strstr(emailadr,"@")) || (!isalpha(emailadr[strlen(emailadr)-1])) || (strlen(emailadr)<7)) {
		write_user(user,"Nespravne zadana email adresa!\n");
		return;
  	        }
	sprintf(text,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie E-mailu na adresu ~OL~FR%s ~OL~FW*)=~RS~FW=-~OL~FK--\n\n",word[1]);
	write_user(user,text);

	sstrncpy(user->mail_to,word[1], WORD_LEN);
	if (word_count>2) {
		inpstr=remove_first(inpstr);
		if (strlen(inpstr)>120) {
			write_user(user,"Prilis dlhy subject!\n");			
			return;
			}
		sstrncpy(user->subject, inpstr, 149);
		}
	else strcpy(user->subject,"(ziadny subject)");
	if (strlen(user->subject)<2) strcpy(user->subject,"(ziadny subject)");
	user->misc_op=4;	
	editor(user,NULL);  	
  	return;  	        	
	}

word[1][0]=toupper(word[1][0]);

/* LOGOVANIE PRISTUPOV Z KLIENTA */
if (!strcmp(word[1],"Klient") && !strcmp(word[2],"Pripojenie")) {
	aklient_log(user->name);
	user->akl=1;
	return;
	}
/*********************************/	
	
	
/* See if user exists */
u=NULL;
if (!(u=get_user_exact(word[1]))) {
  /* sprintf(filename,"%s/%s.D",USERFILES,word[1]);
  if (!(fp=ropen(filename,"r"))) { */
  strcpy(uname,db_user_name(word[1]));
  if (!uname[0]) { /* MySQL */
    if (!(u=get_user(word[1]))) {
      write_user(user,nosuchuser);
      return;
     }                
   }
  else strcpy(word[1],uname);
 }
            
if (u!=NULL) {
  uid=u->id;
  strcpy(word[1],u->name);
  if (check_ignore_user(user,u)) { 
    sprintf(text,"%s ta ignoruje.\n",u->name);
    write_user(user,text);
    return;
   }
 }
else {
  sprintf(query,"select `victim` from `ignuser` where `userid`='%d' and `victim`='%s';",user->id,uname);
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result))) igns=1;
    mysql_free_result(result);
   }
  if (igns==1) {
    sprintf(text,"%s ta ignoruje.\n",uname);
    write_user(user,text);
    return;
   }
  uid=db_userid(word[1]);
 }

if (strlen(word[1])>12) {
  write_user(user,"Prilis dlhe meno adresata.\n");
  return;
 }
sprintf(query,"select sum(length(message)) from mailbox where userid='%d'",uid);
mailboxsize=query_to_int(query);
bigquota=db_user_switch(uid,2); /* no quota userlist */
if (mailboxsize>MAIL_LIMIT*(1+bigquota*20)) {
  sprintf(text,"Prepac, %s ma plnu schranku, nemoze momentalne prijat ziadnu postu.\n",word[1]);
  write_user(user,text);
  return;
 }

if (word_count>2) {
      /* One line mail */
      strcat(inpstr,"\n");
      send_mail(user,word[1],remove_first(inpstr));
      send_copies(user,remove_first(inpstr));
      return;
      }
sprintf(text,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie spravy uzivatelovi ~OL~FR%s ~OL~FW*)=~RS~FW=-~OL~FK--\n\n",word[1]);
write_user(user,text);
user->misc_op=4;
sstrncpy(user->mail_to,word[1], WORD_LEN);
editor(user,NULL);
}



/*** Enter user profile ***/
void enter_profile(user,done_editing)
UR_OBJECT user;
int done_editing;
{
/* FILE *fp; */
char *c;
/* char filename[80]; */

if (!done_editing) {
      write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Editacia profilu ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      sprintf(text,"~OL~FRProfil si mozes editovat aj cez web na stranke ~FW%s/editor\n~OL~FRa to ovela jednoduhsie a s moznostou pouzivania farieb.\n\n",TALKER_WEB_SITE);
      write_user(user,text);
      user->misc_op=5;
      editor(user,NULL);
      return;
      }

sprintf(query,"replace into `profiles` (`userid`,`profile`) values ('%d','",user->id);      
c=user->malloc_start;
while(c!=user->malloc_end) {
  if (*c=='\'') strcat(query,"\\'");
  else if (*c=='"') strcat(query,"\\\"");
  else if (*c=='\\') strcat(query,"\\\\");
  else chrcat(query,*c);
  ++c;
 }
strcat(query,"');");
if (mysql_kvery(query))
 write_user(user,"Profil bol ulozeny.\n");
else 
 write_user(user,"Nastala chyba pri zapise profilu.\n");

/*sprintf(filename,"%s/%s.P",USERFILES,user->name);
if (!(fp=ropen(filename,"w"))) {
      sprintf(text,"%s: Nemozno ulozit tvoj profil.\n",syserror);
      write_user(user,text);
      sprintf("CHYBA: Nemozno otvorit subor %s pre zapis v enter_profile().\n",filename);
      write_syslog(text,0);
      return;
      }
      
c=user->malloc_start;
while(c!=user->malloc_end) putc(*c++,fp);
 fclose(fp);
chmod(filename,S_IRUSR+S_IWUSR+S_IRGRP+S_IWGRP+S_IROTH+S_IWOTH);
*/
}

/*** Examine a user - zmeneny design (B) + uprava (S) ***/  
void examine(user)  
UR_OBJECT user;  
{  
UR_OBJECT u,u2;  
FILE *fp;  
char filename[80],line[451], temp[400], hp[HOMEPAGE_LEN+20];  
int new_mail,days,hours,mins,timelen,days2,hours2,mins2,idle;  
int nasiel=0;
int vec,i,pos,len=0,profile=0,repos=0;
char vecd[4][20],ruum[ROOM_NAME_LEN+1];
u2=NULL;         

 if (user->examine<0 || 2<user->examine) user->examine=1;

 if (word_count>2) {
   profile=atoi(word[2]);
   if (profile<0) profile=0;
   if (profile>100) profile=100;
   if (!strncmp(word[2],"no",2)) profile=0;
  }
 else profile=100;


 if (word_count<2) {
   u=user;
   u2=u;
  }
 else {
   if (user->level>=KIN && !strncmp(word[2],"repos",5)) {
     repos=1;
     nasiel=0;
    }
   else {
     vec=expand_predmet(word[1]);
     if ((u=get_user_exact(word[1]))!=NULL) {  
       nasiel=1;
       u2=u;
      }
    }
   if (!nasiel) {
     if ((u=create_user())==NULL) {  
       sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);  
       write_user(user,text);  
       write_syslog("CHYBA: Nemozno vytvorit docasny user object v examine().\n",0);  
       return;  
      }
     sstrncpy(u->name,word[1],12);  
     if (db_load_user_details(u,repos)) {
       nasiel=1; 
       u2=NULL; 
      }
     else {             
       destruct_user(u);  
       destructed=0;
      }             
    }     
   if (!nasiel && !repos) {
     if ((u=get_user(word[1]))!=NULL) { 
       nasiel=1;                      
       u2=u;      	
      }      	
    }
      
   if (!nasiel) { 
     /* skusime predmet */
     if (!repos && vec>-1) {
       sprintf(filename,"predmety/%s",predmet[vec]->name);  
       if (!(fp=ropen(filename,"r"))) {  /*APPROVED*/
         write_user(user,"K tomuto predmetu niet opisu!\n");
	 return; 
       	}			
       else {  
         fgets(line,440,fp);  
         while(!feof(fp)) {  
           write_user(user,line);  
           fgets(line,440,fp);  
          }  
         fclose(fp);  
        }   	
       return;
      }	
     write_user(user,nosuchuser); return;
    }
  } /* koniec toho else :) */		                  
 
 user->commused[6]++;

sstrncpy(hp,u->homepage,HOMEPAGE_LEN); /* overflow bug.. ;) */
sprintf(text,"\n~RS~FB-=(~OL~FW %s~RS %s ~RS~FB)=",u->name,u->desc);
strcpy(temp,text); colour_com_strip(temp); timelen=73-strlen(temp);
while((timelen--)>0) strcat(text,"-");
strcat(text,"=(~OL~FY*~RS~FB)=--\n\n");  
write_user(user,text);  
if (profile) {
  sprintf(query,"select `profile` from `profiles` where `userid`='%d';",u->id);
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result)) && row[0] && row[0][0]) {
      if (profile<100) {
        i=0;len=0;
        while (row[0][i]) {
          if (row[0][i]=='\n') len++;
          i++;
          if (len==profile) break;
         }
        if (row[0][i]) {
          row[0][i]='\0';
          write_user(user,row[0]);
          write_user(user,"[...]\n");
         }
        else write_user(user,row[0]);
       }
      else write_user(user,row[0]);
     }
    else write_user(user,"Ziadny profil.\n");
    mysql_free_result(result);
   }
 }
/*
	sprintf(filename,"%s/%s.P",USERFILES,u->name);  
	if (!(fp=ropen(filename,"r"))) 
		write_user(user,"Ziadny profil.\n");
	else {  
		
	      fgets(line,440,fp);  
	      while(!feof(fp)) {
	            write_user(user,line);
	            fgets(line,440,fp);
	            }
	       fclose(fp);
	      }
*/

sprintf(query,"select max(UNIX_TIMESTAMP(time)) from mailbox where userid='%d'",u->id);
new_mail=query_to_int(query);
  
days=u->total_login/86400;  
hours=(u->total_login%86400)/3600;  
mins=(u->total_login%3600)/60;  
timelen=(int)(time(0) - u->last_login);  
days2=timelen/86400;  
hours2=(timelen%86400)/3600;  
mins2=(timelen%3600)/60;  
  
if (u2==NULL) {    /*Nie je nahlaseny*/
      /* Zisti roomu, kde bol naposledy: */
      /* get_goto_room(u); */
      if (user->room==NULL) user->room=room_first;
      
      sprintf(temp,"%s\n",zobraz_datum((time_t *)&(u->last_login),1));  
      temp[strlen(temp)-1]=0;
      
if (user->examine==2) {
      write_user(user,"~RS~FG_______________________________________________________________________________\n");
      if (new_mail>u->read_mail) sprintf(text,"~RS~FT%s ~OL%s~RS~FT was last logged on ~OL~FY%s  ~OL~FW~LINEW MAIL\n",u->cloak?level_name[u->cloak]:level_name[u->level],u->name,temp);
	else sprintf(text,"~RS~FT%s ~OL%s~RS~FT was last logged on ~OL~FY%s\n",u->cloak?level_name[u->cloak]:level_name[u->level],u->name,temp);
      write_user(user,text);
      sprintf(text,"~RS~FG%-3s was logged~OL~FW%3d ~RS~FGdays ~OL~FW%2d ~RS~FGhours ~OL~FW%2d~RS~FG minutes ago ~OL~FR(was on for ~FT%d~FR hrs ~FT%d~FR mins)\n",pohl(u, "He","She"), days2, hours2, mins2, u->last_login_len/3600,(u->last_login_len%3600)/60);
      write_user(user,text);   
      sprintf(text,"~RS~FGTotal login:  ~OL~FW%3d~RS~FG days ~OL~FW%2d~RS~FG hours ~OL~FW%2d~RS~FG minutes.    Was in room: ~OL~FW%.20s\n",days, hours, mins,langselect(user,u->room->name,u->room->name_en));
      write_user(user,text);
      texthb[0]='\0';
      if (u->age>0) sprintf(texthb," ~RS~FGAge:~OL~FW~FW%3d ~RS~FGy",u->age);
      sprintf(text,"~RS~FGHomepage : ~OL~FW%-36.36s ~RS~FGPP/MA: ~OL~FW%3d~RS~FG/~OL~FW%-3d %s\n", hp, u->pp,u->mana,texthb);
      write_user(user,text);
      if (u->logoutmsg[0] && u->logoutmsg[0]!='-') {
        sprintf(text,"~RS~FGLogoutMsg: ~OL~FW%s\n",colour_com_strip2(u->logoutmsg,0));
        write_user(user,text);
       }
      if (user->level>=WIZ) { 
        sprintf(text,"~RS~FGEmail    : ~OL~FW%-32.32s~RS~FG     Site: ~OL~FW%-25.25s\n",u->email,u->last_site);
        write_user(user,text);
       }
    }

if (user->examine==1) {    
	sprintf(text,"\n~FB.--[~OL~FW%-7s~RS~FB]-._ ~OL~FYnaposledy: ~FW%-35s~RS~FB_.-%s%s-.\n",u->cloak?level_name[u->cloak]:level_name[u->level],temp,(u->muzzled?"[~OL~FRUML~RS~FB]":"-----"),(u->jailed?"[~FR~OLVAZEN~RS~FB]":"-------"));
	write_user(user,text);
	sprintf(text,"~FB-------------._________________________/___________/\\_________.----------------'\n");
	write_user(user,text);
        if (u->age>0) sprintf(texthb,"Vek:~FW%3d ~RS%s ",u->age,pohl(u,"~FTM","~FMZ"));
        else sprintf(texthb,"Pohl: ~RS~FW[%s~RS~FW] ",pohl(u,"~FTM","~FMZ"));
	sprintf(text,"~FY~OLCelkovy cas ~RS~FB|~FW~OL %3d~RS dni~OL %2d~RS hodin ~OL%2d ~RSminut~FB| ~OL~FY%s~FB| ~OL~FYMiestnost: ~FW%-15s\n",days, hours, mins,texthb,u->room->name);
	write_user(user,text);
	sprintf(text,"~FB\\~OL~FY  Predmety ~RS~FB|~OL~FW             ~RS~FB|~OL~FW            ~RS~FB|~OL~FY Mail:~RS~FW [~LI~OL~FR%c~RS~FW] ~FB|~OL~FY Ignoruje :~OL~FW ",(new_mail>u->read_mail?'N':' '));
	if (u->igntell)  strcat(text,"TE ");
	if (u->ignshout) strcat(text,"SH ");
        if (u->ignsys) strcat(text,"SY ");
        if (u->ignfun) strcat(text,"FU ");
	if (u->igngossip) strcat(text,"GO");
        strcat(text,"\n");
	write_user(user,text);
	sprintf(text,"~OL~FB )__________/-------------^------------^------------\\__________________________/\n");
	write_user(user,text);
	sprintf(text,"~OL~FB/ ~FYHomepage ~FB|~FW  %.65s \n",hp);
	write_user(user,text);
        if (u->logoutmsg[0] && u->logoutmsg[0]!='-') {
          sprintf(text,"~OL~FB|~FYOdhlasenie~FB|~FW  %s\n",colour_com_strip2(u->logoutmsg,0));
          write_user(user,text);
         }
	if (user->level>=WIZ)  {
	  sprintf(text,"~OL~FB|   ~FYE-mail ~FB|  ~FW%-30.30s  ~RS[~OL%.30s~RS] \n",u->email,u->last_site);
	  write_user(user,text);
        }
	sprintf(text,"~OL~FB------------\\_______________________________________________________________~RS~FB__/ \n");
	write_user(user,text);
	sprintf(text,"~FB\\_____ ~OL~FYPosledne pred ~FW%3d ~FYdnami, ~FW%2d ~FYhod. a ~OL~FW%2d~FY min. ~FY(na ~FW%3d~FY hod. a ~FW%2d~FY min.)~RS~FB ____) \n", days2, hours2, mins2, u->last_login_len/3600,(u->last_login_len%3600)/60);
	write_user(user,text);
       }
      destruct_user(u);  
      destructed=0;
      return;  
      }   
    
/* je prave prihlaseny */  
if (u->room->group==4 && (u->team!=user->team || u->team==0)) strcpy(ruum,BRUTALIS);
else strcpy(ruum,langselect(user,u->room->name,u->room->name_en));
if ((!u->ignall) && (u->level >= user->level) && (u!=user)) { /* Kridlo sa stazoval(o) ze to vypisuje nizsim levelom ;) */
			    /*   sprintf(text,"~OL%s is examining you!\n~RS",user->name);
			       write_user(u,text); */
			       }
/* Zisti tzv. arena pointy */

sprintf(temp,"%s\n",zobraz_datum((time_t *)&u->last_login, 2));
idle=(int)(time(0) - u->last_input)/60;
temp[strlen(temp)-1]=0;
hours2=timelen/3600;

if (user->examine==2) {

	write_user(user,"~RS~FG_______________________________________________________________________________\n");
	if (new_mail>u->read_mail) sprintf(text,"~RS~FT%s ~OL%s~RS~FT IS LOGGED ON since ~OL~FY%s ~OL~FW~LINEW MAIL\n",u->cloak?level_name[u->cloak]:level_name[u->level],u->name,temp);
		else sprintf(text,"~RS~FT%s ~OL%s~RS~FT IS LOGGED ON since ~OL~FY%s\n",u->cloak?level_name[u->cloak]:level_name[u->level],u->name,temp);
	write_user(user,text);
	sprintf(text,"~RS~FG%-3s is on for:~OL~FW%3d~RS~FG hours ~OL~FW%2d ~RS~FGminutes.            Idle for ~OL~FW%d~RS~FG minutes.\n",pohl(u,"He","She"),hours2,mins2,idle);
	write_user(user,text);
	sprintf(text,"~RS~FGTotal login:  ~OL~FW%3d~RS~FG days ~OL~FW%2d~RS~FG hours ~OL~FW%2d~RS~FG minutes.    Is in room: ~OL~FW%.20s\n",days, hours, mins, ruum);
	write_user(user,text);
        texthb[0]='\0';
        if (u->age>0) sprintf(texthb," ~RS~FGAge:~OL~FW~FW%3d ~RS~FGy",u->age);
	sprintf(text,"~RS~FGHomepage: ~OL~FW%-36.36s ~RS~FGPP/MA: ~OL~FW%3d~RS~FG/~OL~FW%-3d %s\n", hp, u->pp,u->mana,texthb);
	write_user(user,text);
     }

if (user->examine==1) {
	sprintf(text,"\n~FB.--[~OL~FW%-7s~RS~FB]-._ ~OL~FYPrihlasen%s od: ~FW%-5s ~RS~FB_%s%s_.-%s%s-.\n",u->cloak?level_name[u->cloak]:level_name[u->level],pohl(u,"y","a"),temp,(u->malloc_start!=NULL?"[~FR~OLNieco pise~RS~FB]":"__ _________"),(u->filepos?"[~FR~OLNieco cita~RS~FB]":"__ _________"),(u->muzzled?"[~OL~FRUML~RS~FB]":"-----"),(u->jailed?"[~FR~OLVAZEN~RS~FB]":"-------"));
	write_user(user,text);
	sprintf(text,"~FB-------------._________________________/___________/\\_________.----------------'\n");
	write_user(user,text);
        if (u->age>0) sprintf(texthb,"Vek:~FW%3d ~RS%s ",u->age,pohl(u,"~FTM","~FMZ"));
        else sprintf(texthb,"Pohl: ~RS~FW[%s~RS~FW] ",pohl(u,"~FTM","~FMZ"));
	sprintf(text,"~FY~OLCelkovy cas ~RS~FB|~FW~OL %3d~RS dni~OL %2d~RS hodin ~OL%2d ~RSminut~FB| ~OL~FY%s~FB| ~OL~FYMiestnost: ~FW%-16s\n",days, hours, mins, texthb,ruum);
	write_user(user,text);
	for (i=0;i<HANDS;i++) vecd[i][0]='\0';
	pos=0;
	for (i=0;i<HANDS;i++)
	 if (u->predmet[i]>-1) {
	   strcpy(vecd[pos],predmet[u->predmet[i]]->name);
	   pos++;
	  }
	sprintf(text,"~FB\\~OL~FY  Predmety ~RS~FB|~OL~FW %-12.12s~RS~FB|~OL~FW%-12.12s~RS~FB|~OL~FY Mail: ~RS~FW[~LI~OL~FR%c~RS~FW] ~FB|~OL~FY Ignoruje :~OL~FW ",vecd[0],vecd[1],(new_mail>u->read_mail?'N':' '));
	if (u->igntell)  strcat(text,"TE ");
	if (u->ignsys) strcat(text,"SY ");
	if (u->ignshout) strcat(text,"SH ");
	if (u->ignfun) strcat(text,"FU ");
	if (u->igngossip) strcat(text,"GO");
	strcat(text,"\n");
	write_user(user,text);
	if (pos>2) {
	  sprintf(text,"~FB >          ~RS~FB|~OL~FW %-12.12s~RS~FB|~OL~FW%-12.12s~RS~FB|           ~FB\\\n",vecd[2],vecd[3]);
	  write_user(user,text);
	 }
        pos=0;
        for(i=HANDS;i<BODY;i++) if (u->predmet[i]>-1) pos=1;
        if (pos==1) {
	  sprintf(text,"~FB< ~OL~FYOblecenie ~RS~FB|~OL~FW %-12.12s~RS~FB|~OL~FW%-12.12s~RS~FB|~OL~FW%-12.12s~RS~FB|\n",u->predmet[HANDS]>-1?predmet[u->predmet[HANDS]]->name:"",u->predmet[HANDS+1]>-1?predmet[u->predmet[HANDS+1]]->name:"",u->predmet[HANDS+2]>-1?predmet[u->predmet[HANDS+2]]->name:"");
  	  write_user(user,text);
         }
	sprintf(text,"~OL~FB )__________/-------------^------------^------------\\__________________________/\n");
	write_user(user,text);
	sprintf(text,"~OL~FB/ ~FYHomepage ~FB|~FW  %.65s \n",hp);
	write_user(user,text);
	if (user->level>=WIZ)  {
	sprintf(text,"    ~FY~OLE-mail ~FB|  ~FW%-30.30s  ~RS[~OL%.30s~RS] \n",u->email,u->site);
	write_user(user,text);}
	sprintf(text,"~OL~FB------------\\_______________________________________________________________~RS~FB__/ \n");
	write_user(user,text);
	sprintf(text,"~FB\\_____ ~OL~FYDlzka prihlasenia: ~FW%4d~FY hod. a ~FW %2d~FY min. ~FY(Necinnost: ~FW%2d~FY min. )~RS~FB _________) \n", hours2, mins2, idle);
	write_user(user,text);
      }
      
if (user->examine==2) {
  text[0]='\0';
  strcpy(line,"");
  for (i=0;i<HANDS;i++)
   if (u->predmet[i]>-1) {
     strcat(line,predmet[u->predmet[i]]->name);
     strcat(line," ");
    }
  if (line[0]!='\0') {
    sprintf(text,"~RS~FGHolding :~OL~FW %s\n",line);
    write_user(user,text);
   }
  strcpy(line,"");
  for (i=HANDS;i<BODY;i++)
   if (u->predmet[i]>-1) {
     strcat(line,predmet[u->predmet[i]]->name);
     strcat(line," ");
    }
  if (line[0]!='\0') {
    sprintf(text,"~RS~FGWearing :~OL~FW %s\n",line);
    write_user(user,text);
   }

  if (user->level>=WIZ) { 
    sprintf(text,"~RS~FGEmail   : ~OL~FW%-32.32s~RS~FG     Site: ~OL~FW%-25.25s\n",u->email,u->site);
    write_user(user,text);
   }
  if (u->malloc_start!=NULL) {
    sprintf(text,"~OL~FWUzivatel pise v editore.");
    write_user(user,text);
   }
  if (u->filepos) {
    sprintf(text,"~OL~FWUzivatel cita nejaky text.");
    write_user(user,text);
   }	
  write_user(user,"\n");  
 }

if (u->level>=WIZ && u!=user && !u->ignall && user->level<=u->level) {
	sprintf(text,"~OL%s si ta prezera...\n",user->name);
	write_user(u,text);
	}                        /* (S) Toto xcel PSIcko!!!!!!! */
}  

/*** .anal -> vsetko nfo o luzerovi pokope ***/  
void analyze(user)  
UR_OBJECT user;  
{  
UR_OBJECT u;  
char temp[400],hp[HOMEPAGE_LEN+20];  
int /* new_mail, */days,hours,mins,timelen,minstot,hours2,mins2;
int idles,idlem,idlet,percent,avgidle;

if (word_count<2) u=user;
else
 if (!(u=get_user(word[1]))) {
   write_user(user,notloggedon);
   return;
  }

sstrncpy(hp,u->homepage,HOMEPAGE_LEN);

sprintf(query,"select max(UNIX_TIMESTAMP(time)) from mailbox where userid='%d'",u->id);
/* new_mail=query_to_int(query); */

days=u->total_login/86400;  
hours=(u->total_login%86400)/3600;  
mins=(u->total_login%3600)/60;  
timelen=(int)(time(0) - u->last_login);  
minstot=timelen/60;  
hours2=(timelen)/3600;  
mins2=(timelen%3600)/60;  
avgidle=timelen/u->cps;
  
sprintf(temp,"%s\n",zobraz_datum((time_t *)&u->last_login, 2));
idles=(int)(time(0) - u->last_input);
idlem=(int)(time(0) - u->last_input)/60;
idlet=u->idle+idlem;
temp[strlen(temp)-1]=0;
if (minstot) percent=(int)(idlet*100)/minstot;
else percent=0;

sprintf(text,"~FG[ %s ~OL~FW%s~RS~FG ] ~OL~FW%s~RS:%d  ~OL~FW%s~RS~FW (%d)\n",level_name[u->level],u->name,u->site,u->site_port,u->ipcka,u->socket);
/* if (new_mail>u->read_mail) strcat(text," Mail: ~OL~FWNEW"); else strcat(text," Mail: OLD"); */
/* if (u->vis) strcat(text,"~FGInvis: ~OL~FWN\n"); else strcat(text,"~FGInvis: ~FW~OLY\n"); */
write_user(user,text);  
/* sprintf(text,"~RS~FGOn4: ~OL~FW%d~RS~FGh ~OL~FW%d~RS~FGm (~OL~FW%d~RS~FGm) since ~OL%s~RS~FG. TLT: ~OL~FW%d~RS~FGd ~OL~FW%d~RS~FGh ~OL~FW%d~RS~FGm. PP/MA: ~OL~FW%3d~RS~FG/~OL~FW%-3d~RS~FG Room: ~OL~FW%s\n",hours2,mins2,minstot,temp,days,hours,mins,u->pp,u->mana,u->room->label); */
sprintf(text,"~RS~FGOn4: ~OL~FW%d~RS~FGh ~OL~FW%d~RS~FGm (~OL~FW%d~RS~FGm) since ~OL%s~RS~FG.  TLT: ~OL~FW%d~RS~FGd ~OL~FW%d~RS~FGh ~OL~FW%d~RS~FGm.  Room: ~OL~FW%s\n",hours2,mins2,minstot,temp,days,hours,mins,u->room->name);
write_user(user,text);
sprintf(text,"~FGIdle: ~OL~FW%d~RS~FG sec  ~OL~FW%d~RS~FG/~OL~FW%d~RS~FG/~OL~FW%d~RS~FG (~OL~FW%d~RS~FG %%) Average: ~OL~FW%d~RS~FG sec (~OL~FW%d~RS~FG min)",idles,idlem,idlet,minstot,percent,avgidle,avgidle/60);
if (u->malloc_start!=NULL) strcat(text," Action: ~OL~FWEditor\n");
else if (u->filepos) strcat(text," Action: ~OL~FWReading\n");
else if (u->afk) strcat(text," Action: ~OL~FWAFK\n");
else strcat(text,"\n");
write_user(user,text);
/* sprintf(text,"~FGEmail: ~FW%s~FG HP: ~FW%s\n",u->email,hp); */
sprintf(text,"~FGEmail: ~FW%s\n",u->email);
write_user(user,text);
strcpy(word[1],u->name);
word_count=2;
view_history(user);
}  

/*** Show talker rooms ***
     .rmst = rooms(user) => teraz sa vola .rooms!!
    ZMENA - Spakky - vypisuje prehladnejsie a zarovnava */

void rooms(user)
UR_OBJECT user;
{
RM_OBJECT rm;
UR_OBJECT u;
char access[20],*pom;
char nadpis[COOLBUF_LEN+2];
int cnt,i, ppredm=0,zwjs/* ,rast */;
FILE *fp;
char filename[81];

sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
  fclose(fp);
  deltempfile(filename);
 }
if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
  write_user(user,"Chyba: Nastal problem so zapisovanim docasneho suboru.\n");
  return;
 }

fprintf(fp,"\n~OL~FB=--=--=--=--=--=--=--=--=- ~OL~FYInformacie o miestnostiach ~OL~FB-=--=--=--=--=--=--=--=--=\n~FTNazov miestnosti ~OL~FB|~RS~FT Pris. Predm Uziv Sprav  Tema\n~OL~FB=--=--=--=--=--=-|-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=\n");
for(rm=room_first;rm!=NULL;rm=rm->next) { 
      if (rm->invisible && user->level<WIZ) continue;
      if (rm->group!=user->room->group && user->level<WIZ) continue; 
      if (user->level>=WIZ) if (strlen(word[1])==1 && atoi(word[1])!=rm->group) continue;
      if (user->level<KIN && !strcmp(rm->name,"tunel")) continue;
      strcpy(access,langselect(user," ~FGVER"," ~FGPUB"));
      if (rm->access & PRIVATE) strcpy(access," ~FRPRIV");
      if (rm->access==GOD_PRIVATE) strcpy(access,"  ~OL~FYGOD");
      if (rm->access & FIXED) access[0]='*';
      cnt=0;
      for(u=user_first;u!=NULL;u=u->next)
           if (u->type!=CLONE_TYPE && !u->login && u->room==rm) ++cnt;
      ppredm=0;
      zwjs=0;
      /* rast=0; */
      for(i=0;i<MPVM;++i) if (rm->predmet[i]>-1) {
        if (predmet[rm->predmet[i]]->type<3) zwjs++;
	/* else if (predmet_type[rm->predmet[i]]==3 || predmet_type[rm->predmet[i]]==4) rast++; */
	else ppredm++;
       }
           
        if (rm->topic[0]=='\0') strcpy(nadpis,langselect(user,"<bez temy>","<no topic>"));
	else
         {
          strcpy(nadpis,rm->topic);
          force_language(nadpis,user->lang,1);
	  while (strlen(colour_com_strip2(nadpis,0))>38) nadpis[strlen(nadpis)-1]='\0';
	  if (nadpis[strlen(nadpis)-1]=='\253') nadpis[strlen(nadpis)-1]='\0';
	  if (nadpis[strlen(nadpis)-2]=='\253') nadpis[strlen(nadpis)-2]='\0';
	  if (nadpis[strlen(nadpis)-3]=='\253') nadpis[strlen(nadpis)-3]='\0';
         }

        sprintf(text,"%-16s ~OL~FB|~RS~FW %8s~RS ~FW%2d ~FY%2d~FW %3d  %3d  %s\n",langselect(user,rm->name,rm->name_en),access,ppredm,zwjs,cnt,rm->mesg_cnt,nadpis);

      pom=(char *) malloc ((strlen(text)*sizeof(char))+1);
      if (pom!=NULL) {
        strcpy(pom,text);
        colour_com_strip(pom);
        if (word_count>1 && strlen(word[1])>1 && !strstr(pom,word[1])) continue;
        if (word_count==3 && !strstr(pom,word[2])) continue;
        fputs(text,fp);
        free(pom);	   
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN ROOMS()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
fprintf(fp,"~OL~FB=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-\n");
fclose(fp);
switch(more(user,user->socket,filename)) {
  case 0: write_user(user,"Chyba pri citani rooms!\n");  break;
  case 1: user->misc_op=2;
 }	
}

/* (S) Zmena - interaktivne ... */

void change_pass(user)
UR_OBJECT user;
{
UR_OBJECT u;

if (word_count>1) {
	if (user->level<KIN) {
		write_user(user,"Pouzi: .passwd\n");
		return;
		}
	else {
		word[1][0]=toupper(word[1][0]);
		if (!strcmp(word[1],user->name)) {
			write_user(user,"Pouzi: .passwd\n");
			return;
			}
		if ((u=get_user_exact(word[1]))!=NULL) {
			if ((u->level>=user->level) && (user->level<9)) {
				write_user(user,"Heslo mozes menit len mensiemu levelu ako mas sam.\n");
				return;
			  }
			strcpy(user->chanp, u->name);
			user->misc_op=13;
			user->has_ignall=user->ignall;
			user->ignall=1;
			echo_off(user);
			sprintf(text,"~FTNove heslo pre %s: ",sklonuj(u,4));
			write_user(user,text);
			return;
			}
		if ((u=create_user())==NULL) {
			sprintf(text,"%s: Nemozno vytvorit docasnu uzivatelsku strukturu!\n",syserror);
			write_user(user,text);
			write_syslog("CHYBA: Nemozno vytvorit docasnu uzivatelsku strukturu v change_pass().\n",0);
			return;
			}
		sstrncpy(u->name,word[1],12);
		if (!load_user_details(u)) {
			write_user(user,nosuchuser);
			destruct_user(u);
			destructed=0;
			return;
			}
		if ((u->level>=user->level) && (user->level<9)) {
				write_user(user,"Heslo mozes menit len mensiemu levelu ako mas sam.\n");
				destruct_user(u);
				destructed=0;
				return;
			  }
		strcpy(user->chanp,u->name);
		sprintf(text,"~FTNove heslo pre %s: ",sklonuj(u,4));
		destruct_user(u);
		destructed=0;
		user->misc_op=13;
		user->has_ignall=user->ignall;
		user->ignall=1;
		echo_off(user);
		write_user(user,text);
		return;
		}
	}
if (word_count>2) {
	write_user(user,"Pouzi: .passwd");
	return;
	}
user->misc_op=12;
user->has_ignall=user->ignall;
user->ignall=1;
user->chanp[0]='\0';
echo_off(user);
write_user(user,"~FTStare (aktualne) heslo: ");
}


/* (S) Funkcia potrebna pre zmenu hesla (sposobom B na ucet 501 ;-) */
int zmenheslo(UR_OBJECT user,int wizpass)
{
UR_OBJECT u;
char temp[20];

 if (strcmp(word[0],user->newpass)) {
   write_user(user,"\n~OL~FRNespravne potvrdenie!\n");
   if (wizpass) {
     user->misc_op=31;
     if (user->socket>1000) echo_off(user);
     write_user(user,"\n~FTNove bezpecnostne heslo: ");
    }
   else {
     user->misc_op=13;
     if (user->socket>1000) echo_off(user);
     write_user(user,"\n~FTNove heslo: ");
    }
   return 1;
  }
 echo_on(user);
 user->misc_op=0;
 user->ignall=user->has_ignall;
 if (user->chanp[0]!='\0') {
   if ((u=get_user_exact(user->chanp))!=NULL) {
     strcpy(u->pass,(char *)md5_crypt(expand_password(word[0]),u->name));
     sprintf(text,"\n~OL~FYHeslo pre %s bolo zmenene.\n",sklonuj(u, 4));
     write_user(user,text);
     sprintf(text,"~OL~FY%s ti %s heslo!\n",user->name, pohl(user,"zmenil","zmenila"));
     write_user(u,text);
     sprintf(text,"%s %s heslo uzivatelovi: %s.\n",user->name,pohl(user,"zmenil", "zmenila"), u->name);
     write_syslog(text,1);
    }
   else {
     if ((u=create_user())==NULL) {
       sprintf(text,"%s: Nemozno vytvorit docasnu uzivatelsku strukturu.\n",syserror);
       write_user(user,text);
       write_syslog("CHYBA: Nemozno vytvorit docasnu uzivatelsku strukturu v misc_op case 13\n",0);
      }
     else {
       strcpy(u->name,user->chanp);
       if (!load_user_details(u)) {
         write_user(user,nosuchuser);
         destruct_user(u);
         destructed=0;
        } 
       else {
         strcpy(u->pass,(char *)md5_crypt(expand_password(word[0]),u->name));
         save_user_details(u,0);
         sprintf(text,"\n~FYHeslo pre %s bolo zmenene.\n",sklonuj(u, 4));
         write_user(user,text);
         sprintf(text,"%s %s heslo uzivatelovi: %s.\n",user->name,pohl(user,"zmenil","zmenila"),u->name);
         write_syslog(text,1);
         destruct_user(u);
         destructed=0;
        }
      }
    }
   prompt(user);
   return 1;
  }
 else {
   if (wizpass) {
     sprintf(temp,"Secure%s",user->name);
     strcpy(user->wizpass,(char *)md5_crypt(expand_password(word[0]),temp));
     save_user_details(user,0);
     sprintf(text,"\n~OL~FYHeslo bolo zmenene.\n");
    }
   else {   
     strcpy(user->pass,(char *)md5_crypt(expand_password(word[0]),user->name));
     save_user_details(user,0);
     sprintf(text,"\n~OL~FYHeslo bolo zmenene.\n");
    }
   write_user(user,text);
   prompt(user);
   return 1;
  }
} 

/*** Quit a user - replaces standard "disconnect_user" calling (R) ***/
void quit_user(user,inpstr)
UR_OBJECT user;
char *inpstr;
{

strcpy(user->logoutmsg,"-");
if (!strcmp(user->room->name,"skala_samovrahov")) {
	sprintf(text,"~FT%s sa %s zo skaly ...\n",user->name,pohl(user,"rozbehol a skocil","rozbehla a skocila"));
	write_room_except(user->room,text,user);
        if (word_count<2) sprintf(text,"~FRskok zo skaly");
        else sprintf(text,"~FT%.52s",inpstr);
	logout_user(user,text);
        return;
	}
if (word_count<2)
   disconnect_user(user,1,NULL);
else if (((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing))
      || (contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT)))
       disconnect_user(user,1,NULL);
else {
   sprintf(text,"~FT%.52s",inpstr);
   logout_user(user,text);
  } 
}

/*** Kill a user ** Mensia ZMENA - vypise, kto ta killne - da sa upravit*/
void kill_user(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT victim;
/* RM_OBJECT rm; */
char *name;

if (word_count<2) {
      write_user(user,"Pouzi: .kill <uzivatel> [text]\n");  return;
      }
if (!(victim=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (user==victim) {
      write_user(user,"Snaha spachat takymto sposobom samovrazdu je siesty priznak sialenstva.\n");
      return;
      }
if (victim->level>=user->level) {
      write_user(user,"Bohuzial nemozes zabit nikoho s rovnakym alebo vyssim levelom ako mas sam.\n");
      sprintf(text,"~FR%s sa ta pokusal zabit! Daj si nanho bacha.~RS\n",user->name);
      write_user(victim,text);
      return;
      }
if (word_count>2) inpstr=remove_first(inpstr);
if (word_count>2) sprintf(text,"%s KILLED %s: %s.\n",user->name,victim->name,inpstr);
else sprintf(text,"%s KILLED %s.\n",user->name,victim->name);
write_syslog(text,2);
if (user->vis) name=user->name; else name=invisname(user);
/* rm=victim->room; */
srand(time(0));
switch (rand()%3) /* Altrenativny KILL - kto ma fantaziu, moze pridat :> */
  {
       case 0:  write_user(user,"~FR~OLVytahujes z vrecka stary indiansky tomahawk zdedeny po babicke...\n");
		sprintf(text,"~FR%s %s z vrecka stary indiansky tomahawk zdedeny po babicke...~FW\n",name,pohl(user,"vytiahol","vytiahla"));
		write_room_except(NULL,text,user);
		write_user(victim,"~FR~OLStary hrdzavy tomahawk ti oddelil hlavu od tela!\n");
		if (word_count>2)
		 sprintf(text,"~FR~OL%s~RS~FR %s presnu ranu tomahawkom medzi oci: %s\n",victim->name,pohl(victim,"dostal","dostala"),inpstr);
		else
		 sprintf(text,"~FR~OL%s~RS~FR %s presnu ranu tomahawkom medzi oci.\n",victim->name,pohl(victim,"dostal","dostala"));
		write_room_except(NULL,text,victim);
		break;

       case 1:  write_user(user,"~FR~OLVytahujes spoza plasta staru knihu magie zdedenu po deduskovi...\n");
		/* sprintf(text,"~FR~OL%s vytahuje knihu smrtiacich kuziel!~RS\n",name); */
		/* write_room_except(NULL,text,user); */
		sprintf(text,"~FR%s vytahuje spoza plasta staru knihu magie zdedenu po deduskovi...~FW\n",name);
		write_room_except(NULL,text,user);
		write_user(victim,"~FR~OLKuzlo metelescu-blescu ta roztrhalo na kusky!\n");
		if (word_count>2)
 		 sprintf(text,"~FRKuzlo metelescu-blescu roztrhalo ~OL%s~RS~FR na kusky: %s\n",sklonuj(victim,4),inpstr);
		else
 		 sprintf(text,"~FRKuzlo metelescu-blescu roztrhalo ~OL%s~RS~FR na kusky.\n",sklonuj(victim,4));
		write_room_except(NULL,text,victim);
		break;

       case 2:  write_user(user,"~FR~OLOtvaras oci a v ich hlbke nemozno citat nic prijemne...\n");
		sprintf(text,"~FR%s %s: 'Prach si a v prach sa obratis!'~RS\n",name,pohl(user,"vyhlasil","vyhlasila"));
		write_room_except(NULL,text,user);
		write_user(victim,"~FR~OLSmrtiaci pohlad ta premenil na dymiacu kopku popola!\n");
		/* sprintf(text,"~FR%s %s jeden zo svojich vrazednych pohladov...~FW\n",name,pohl(user,"vrhol","vrhla")); */
		/* write_room(user->room,text); */
		if (word_count>2)
		 sprintf(text,"~FR~OL%s~RS~FR sa %s na dymiacu kopku popola: %s\n",victim->name,pohl(victim,"premenil","premenila"),inpstr);
		else
		 sprintf(text,"~FR~OL%s~RS~FR sa %s na dymiacu kopku popola.\n",victim->name,pohl(victim,"premenil","premenila"));
		write_room_except(NULL,text,victim);
		break;

  }

if (word_count>2) {  /* Uprava: vypise PRECO ho dotycna osoba klofla :> (R) */
  sprintf(text,"~FR~OLPosledne co si %s boli slova: %s\n",pohl(victim,"zacul","zacula"),inpstr);
  write_user(victim,text);
 }

if (word_count>2)
 sprintf(text,"~FR%s: %s",pohl(victim,"Zabity","Zabita"),inpstr);
else
 sprintf(text,"~FR%s", pohl(victim,"Zabity","Zabita"));
victim->killed++;       
logout_user(victim,text);
inyjefuc=1;
police_freeze(user,1);
}

/*** ZMENA - .fight - zurive bitky mozu zacat! ;) [s:zd] */

void fight_user(user)
UR_OBJECT user;
{
UR_OBJECT victim;
char *name;
int user_chance, victim_chance, points, v_arene=0,i,ppcan,ppdec,winuser;

if (user->room->group==4) {
  fight_brutalis(user);
  return;
 }
if (word_count<2) {
      write_user(user,"Pouzi: .fight <uzivatel>\n");
      return;
      }
if (!(victim=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
                                     
if (user==victim) {
      write_user(user,"Snaha zbit sam seba je devatnastym priznakom sialenstva!\n");
      return;
      }
if (strcmp(victim->room->name,user->room->name)) {
      write_user(user,"Suboj sa moze odohravat len v rovnakej miestnosti.\n");
      return;
      }

if (victim->afk) {        /* ked je afk tak preco ho mlatit? */
      sprintf(text,"%s je prave mimo klavesnice.\n",victim->name);
      write_user(user,text);
      return;
      }

if (victim->ignall) {     /* ak pise v editore - ziadne fighty */
      if (victim->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",victim->name);
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",victim->name);
      write_user(user,text);
      return;
      }

/* Kde sa nesmie fajtovat ? */

if (user->room->group==2) {
	write_user(user,"Lutujem, ale na Portalise vladne pokoj a mier...\n");
	return;
	}      

/* if (user->level==SOL && strcmp(user->room->name,"arena")) { */
/*	write_user(user,"Ved si vojak, nemozes mlatit slusnych ludi. Ak sa chces mlatit, chod do areny.\n"); */
if (strcmp(user->room->name,"arena")) {
	write_user(user,"Na bitky je vyhradena arena.\n");
	return;
	}      

if (!strcmp(user->room->name,"zalar")) {
	write_user(user,"Ale, ale! Este aj v zalari?!\n");
	return;
	}
if (!strcmp(user->room->name,"namestie")) {
	write_user(user,"Na namesti nie! Na suboj si najdite nejake odlahlejsie miesto.\n");
	return;
	}
if (!strcmp(user->room->name,"afroditin_chram")) {
	write_user(user,"Afroditin chram je miesto lasky a pokoja, ziadne bitky!\n");
	return;
	}
if (!strcmp(user->room->name,"amfiteater")) {
	write_user(user,"Nemozes sa bit v amfiteatri!\n");
	return;
	}
if (!strcmp(user->room->name,"svatyna")) {
	write_user(user,"Svatyna je miesto pre meditaciu a nie pre bitky!\n");
	return;
	}

if (!strcmp(user->room->name,"plachetnica")) {
	write_user(user,"Ale no tak, namornik!\n");
	return;
	}	

if (!strcmp(user->room->name, FLYER_ROOM) && flyer.pozicia) {
	write_user(user,"Pocas letu to nie je velmi dobry napad!\n");
	return;
	}
if (!strcmp(user->room->name,"arena")) {
	ppcan=FIGHTCANARENA;
	ppdec=FIGHTDECARENA;
	} else {
	ppcan=FIGHTCAN;
	ppdec=FIGHTDEC;
	}
if (user->pp < ppcan) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, ppcan); 
	write_user(user,text); 
	return; 
	} 

if ((is_in_hands(victim,expand_predmet("stit"))>-1) && (strcmp(user->room->name,"arena"))) {
	sprintf(text,"%s je %s teraz bojovat.\n",victim->name,pohl(victim,"chraneny stitom, nieje rozumne s nim","chranena stitom, nieje rozumne s nou"));
	write_user(user,text);
	return;
	}
 
decrease_pp(user, ppdec, NODYNAMIC); 
decrease_pp(victim, ppdec/2, NODYNAMIC); 
if (victim->pp<1) setpp(victim,1);
   
if (user->vis) name=user->name; else name=invisname(user);

/* Tu sa im prideluju sance: */
/*
switch(user->level) {
              case 0:
              case 1: user_chance=0; break;
              case 2: user_chance=random()%51+20; break;
              case 3: user_chance=random()%51+30; break;
              case 4: user_chance=random()%51+40; break;
              case 5: user_chance=random()%51+50; break;
              case 6: user_chance=random()%51+60; break;
              case 7: user_chance=random()%51+70; break;
              case 8: user_chance=random()%3+98;  break;
              default: write_user(user,"S tym si daj pokoj!\n"); return;
              }
switch(victim->level) {
              case 0: victim_chance=random()%31;    break;
              case 1: victim_chance=random()%11+30; break;
              case 2: victim_chance=random()%31+30; break;
              case 3: victim_chance=random()%31+40; break;
              case 4: victim_chance=random()%31+50; break;
              case 5: victim_chance=random()%31+60; break;
              case 6: victim_chance=random()%31+70; break;
              case 7: victim_chance=random()%31+80; break;
              case 8: victim_chance=random()%3+98;  break;
              default: write_user(user,"S tym si daj pokoj!\n"); return;
	      }
*/	      
if (!strcmp(user->room->name,"arena")) v_arene=1;

if (v_arene) {
  if (user->level>=GOD) user_chance=random()%101+500;
  else user_chance=random()%101;
  if (victim->level>=GOD) victim_chance=random()%101+500;
  else victim_chance=random()%101;
 }
else {
  if (user->level>=KIN) user_chance=random()%101+200+(user->level-KIN)*500;
  else user_chance=random()%101+user->level*10;
  if (victim->level>=KIN) victim_chance=random()%101+200+(user->level-KIN)*500;
  else victim_chance=random()%91+10+user->level*20;
 }

/* Zvysenie sance ak ma user mec */	 
if (is_in_hands(user,expand_predmet("mec"))>-1) user_chance+=30;
if (is_in_hands(victim,expand_predmet("mec"))>-1) victim_chance+=30;

/* Znizuje sance ak ma user amulet ;>>> */
if (is_in_hands(user,expand_predmet("amulet"))>-1) user_chance-=20;
if (is_in_hands(victim,expand_predmet("amulet"))>-1) victim_chance-=20;

/* stit u obete znizuje sance utocnika */
if (is_in_hands(victim,expand_predmet("stit"))>-1) user_chance-=20;

if (user->pp>100) user_chance+=((user->pp-100)/10); /* + 0 az 10 podla PPciek */
if (victim->pp>100) victim_chance+=((victim->pp-100)/10);

sprintf(text,"~OL~FR%s ~RS~FWsa chce bit s ~OL~FR%s~RS~FW. ~OL~FYMiestnostou sa ozyvaju udery ich boja...\n",name, victim->vis?sklonuj(victim,7):pohl(victim,"Neznamym","Neznamou"));
write_room_except2users(user->room,text,user,victim);
sprintf(text,"~OL~FR%s ~RS~FWsa chce s tebou bit. ~OL~FYMiestnostou sa ozyvaju udery vasho boja...\n",name);
write_user(victim,text);
sprintf(text,"Chces sa bit s ~OL~FR%s~RS~FW. ~OL~FYMiestnostou sa ozyvaju udery vasho boja...\n",sklonuj(victim,7));
write_user(user,text);

hesh(user->room); /* zvieratka nemaju rady boje.. hes! :) */

points=0;
for(i=0;i<HANDS;i++) if (user->predmet[i]>-1)
 if ((user->predmet[i]==expand_predmet("mec") || user->predmet[i]==expand_predmet("stit")) && points==0) {
   user->dur[i]--;
   if (user->dur[i]<1) {
     if (user->predmet[i]==expand_predmet("mec"))
       write_user(user,"Mec sa pri boji zlomil.\n");
     if (user->predmet[i]==expand_predmet("stit"))
       write_user(user,"Stit sa ti rozpadol pocas boja.\n");
     user->carry-=predmet[user->predmet[i]]->weight;
     user->predmet[i]=-1;
     points=1;
    }    
  }

/* Buko: Arena stuff */
winuser=user_chance-victim_chance;
if (winuser>10) winuser=1;
else if (winuser<-10) winuser=2;
else winuser=0;
if ( v_arene && (winuser==1)) add_point(user,DB_ARENA,1,0);
if ( v_arene && (winuser==2)) add_point(victim,DB_ARENA,1,0);

if (winuser==0) {
    write_room(user->room,"~OLPo dlhom boji obaja klesli vycerpani na zem - nieto vitaza ani porazeneho!\n");
    }	
if (winuser==2) {
    sprintf(text,"%s %s zo suboja ako vitaz a pripisuje si bod!\n",victim->name,pohl(victim,"vysiel","vysla"));
    write_room_except(user->room,text,victim);
    sprintf(text,"~LB~FR%s si!!! Nabuduce si dobre rozmysli, s kym pojdes do suboja.\n",pohl(user,"Prehral","Prehrala"));
    write_user(user,text);
    sprintf(text,"~LB~FR%s si sa!!! Vsetci pritomni ti gratuluju k hrdinskemu vykonu.\n",pohl(victim,"Ubranil","Ubranila"));
    write_user(victim,text);
    sprintf(text,"~FR%s %s",pohl(user,"ubity","ubita"),sklonuj(victim,7));
    if (!v_arene) logout_user(user,text);
    else disconnect_user(user,3,NULL); 
    }
if (winuser==1) {
    sprintf(text,"%s %s zo suboja ako vitaz a pripisuje si bod!\n",name,pohl(user,"vysiel","vysla"));
    write_room_except(user->room,text,user);
    sprintf(text,"~FR%s si!!! ", pohl(user,"Vyhral","Vyhrala"));
    write_user(user,text);    
    sprintf(text,"~LB~FR%s sancu proti tvojej uzasnej bojovej technike.\n",pohl(victim,"Nemal","Nemala"));
    write_user(user,text);
    sprintf(text,"~LB~FR%s si sa!!! ",pohl(victim,"Neubranil","Neubranila"));
    write_user(victim,text);        
    sprintf(text,"~FRBudes musiet opustit Atlantidu ako %s ...\n",pohl(victim,"porazeny","porazena"));
    write_user(victim,text);
    sprintf(text,"~FR%s %s",pohl(victim,"ubity","ubita"),sklonuj(user,7));
    if (!v_arene) logout_user(victim,text);
    	else disconnect_user(victim,3,NULL); 
    inyjefuc=1;    
    }
/*write_room(user->room,"~FM~OLBol to suboj hodny obyvatelov Atlantidy.\n");*/
/*           ^^^^^^^^^^ BUKO BUG!!!! :>>> */
}

/*** Kick - vykickne usera z roomy */
void kick(user)
UR_OBJECT user;
{
UR_OBJECT victim;
RM_OBJECT rm;
char *name;
/* int i=0,rcount; */
int vec,kde,dur;

if (word_count<2) {
      write_user(user,"Pouzi: .kick <uzivatel>\n");
      return;
      }
if (user->vis) name=user->name; else name=invisname(user);
vec=expand_predmet(word[1]);
kde=is_in_room(user->room,vec);
victim=get_user(word[1]);
if (vec>-1 && predmet[vec]->type<3) {
  if (victim==NULL || (victim!=NULL && kde>-1)) {
    if (kde==-1) {
      sprintf(text,"Ziadn%s %s sa tu momentalne nenachadza.\n",zwjpohl(vec,"y","a","e",""),predmet[vec]->name);
      write_user(user,text);
      return;
     }
    /* ideme nakopat zvjeratko */
    if (predmet[vec]->weight>MAXCARRY*2) { /* privelke zviera na kopanie ;) */
      sprintf(text,"~FYPriblizil%s si sa ku %s ale %s do hlavy ze vidis same ***.\n",pohl(user,"","a"),predmet[vec]->dativ,zwjpohl(vec,"ten ta tak kopol","ta ta tak kopla","to ta tak koplo","tie ta tak kopli"));
      write_user(user,text);
      wrtype=WR_ZVERY;
      sprintf(text,"~FY%s tak kop%s %s do hlavy ze vidi same ***.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,4));
      text[3]=toupper(text[3]);
      write_room_except(user->room,text,user);
      wrtype=0;
      user->stars=40;
      return;
     }
    rm=random_room(user->room,0);
    if (rm==NULL) {
      if (user->room==get_room("plachetnica",NULL) || user->room==get_room("letun",NULL))
       {
        wrtype=WR_ZVERY;
        user->room->predmet[kde]=-1;
        user->room->dur[kde]=0;
        sprintf(text,"~FYSchmat%s si %s a hodil%s si %s cez palubu.\n",pohl(user,"ol","la"),predmet[vec]->akuzativ,pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
        write_user(user,text);
        sprintf(text,"~FY%s schmat%s %s a hodil%s %s cez palubu.\n",name,pohl(user,"ol","la"),predmet[vec]->akuzativ,pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
        write_room_except(user->room,text,user);
	wrtype=0;
        return;
       }
      sprintf(text,"Kopat do zvieratiek.. ze sa nehanbis!\n");
      write_user(user,text);
      return;
     }
    user->room->predmet[kde]=-1;
    dur=user->room->dur[kde];
    user->room->dur[kde]=0;
    sprintf(text,"~FYVykop%s si odtialto %s.\n",pohl(user,"ol","la"),predmet[vec]->akuzativ);
    write_user(user,text);
    wrtype=WR_ZVERY;
    sprintf(text,"~FY%s odtialto vykop%s %s.\n",name,pohl(user,"ol","la"),predmet[vec]->akuzativ);
    write_room_except(user->room,text,user);
    if (is_free_in_room(rm)>-1)
     {
      if (((vec!=32 && user->socket+10000==dur) || dur<10000)) put_in_room(rm,vec,default_dur(vec));
      else put_in_room(rm,vec,dur);
      sprintf(text,"~FYNiekto sem kopol %s.\n",predmet[vec]->akuzativ);
      write_room(rm,text);
     }
    wrtype=0;
    return; 
   }
 }
if (victim==NULL) {
  write_user(user,notloggedon);
  return;
 }                                     
if (user==victim) {
      write_user(user,"Snaha kopnut sam seba je dvadsiatym prvym priznakom sialenstva!\n");
      return;
      }
if (strcmp(victim->room->name,user->room->name)) {
      sprintf(text,"%s nie je v tejto miestnosti.\n",victim->name);
      write_user(user,text);
      return;
      }
if (victim->ignall) {     /* ak pise v editore - ziadne fighty */
      if (victim->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",victim->name);
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",victim->name);
      write_user(user,text);
      return;
      }

/* no-kick areas: */
      
if (!strcmp(user->room->name,"zalar")) {
	write_user(user,"Nemozes vykopavat zo zalara!\n");
	return;
	}
if (!strcmp(user->room->name,"namestie")) {
	write_user(user,"Na namesti nie!\n");
	return;
	}
if (user->room->group==2 && strcmp(user->room->name,"skala_samovrahov")) {
	write_user(user,"Na Portalise ziadne nasilie!\n");
	return;
	}
if (!strcmp(user->room->name,FLYER_ROOM)) {
	write_user(user,"Vykopnut z letuna?!? To by bolo surove...\n");
	return;
	}
if (!strcmp(user->room->name,SHIPPING_SHIP)) {
	write_user(user,"Z lode nemozes nikoho vykopnut.\n");
	return;
	}

if (!strcmp(user->room->name,"amfiteater")) {
	write_user(user,"Nemozes z amfiteatru!\n");
	return;
	}
if (!strcmp(user->room->name,"svatyna")) {
	write_user(user,"Svatyna je miesto pre meditaciu!\n");
	return;
	}

if (check_ignore_user(user,victim)) {  /* ignorovanie single usera */
 sprintf(text,"%s ta ignoruje.\n",victim->name);
 write_user(user,text);
 return;
}

if (user->pp < KICKCAN) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, FIGHTCAN); 
	write_user(user,text); 
	return; 
	} 
if (user->level<victim->level) {
    write_user(user,"Uzivatel ma vyssi level ako ty!\n");
    return;
    }	

if (victim->glue) {
	sprintf(text,"Nech skusas ako skusas, nejde to! %s je %s zuvackou!\n",victim->name,pohl(victim,"prilepeny","prilepena"));
	write_user(user,text);
	return;
	}
 
/* Tu sa im prideluju sance: */

/* kukni dalsie roomy: */

if (!strcmp(user->room->name,"skala_samovrahov")) {
  if (((int)(time(0) - victim->last_input)/60)<12) {
    sprintf(text,"%s je %s sa ti uhnut.\n",victim->name,pohl(victim,"culy jak vevericka, stihol","cula jak vevericka, stihla"));
    write_user(user,text);    
    decrease_pp(user, KICKDEC/2, NODYNAMIC); 
    return;
   }
  sprintf(text,"~OLNenapadne si sa priblizil%s k %s a strcil%s si ",pohl(user,"","a"),sklonuj(victim,3),pohl(user,"","a"));
  strcat(text,pohl(victim,"ho zo skaly.\n","ju zo skaly.\n"));
  write_user(user,text);
  sprintf(text,"\n~OL~FR%s sa k tebe nenapadne priblizil%s a strcil%s ta zo skaly.. padas..\n",name,pohl(user,"","a"),pohl(user,"","a"));
  write_user(victim,text);    
  sprintf(text,"~FT%s sa priblizil%s k %s a strcil%s ",name,pohl(user,"","a"),sklonuj(victim,3),pohl(user,"","a"));
  strcat(text,pohl(victim,"ho zo skaly.\n","ju zo skaly.\n"));
  write_room_except2users(user->room,text,user,victim);
  decrease_pp(user, KICKDEC, NODYNAMIC); 
  victim->dead=10;
  return;
 }

   
rm=random_room(user->room,0);
if (rm==NULL) {
  if (user->level>=KIN) rm=get_room("namestie",NULL);
  else {
    sprintf(text,"Nemas %s kam vykopnut.\n",pohl(victim,"ho","ju"));
    write_user(user,text);
    return;
   }
 }
/*
rcount=0;
for(i=0;i<MAX_LINKS;i++) {
        if (user->room->link[i]!=NULL) rcount++;
 	else break;
	}
if (!rcount) {
	rm=get_room("namestie",NULL);
	}
else {
	i=random()%rcount;
	rm=user->room->link[i];
	}
*/	
decrease_pp(user, KICKDEC, NODYNAMIC); 
move_user(victim,rm,3);
sprintf(text,"\n~OL%s %s %s z miestnosti!\n",name,pohl(user,"vykopol","vykopla"),sklonuj(victim, 4));
write_room_except(user->room,text,user);
sprintf(text,"\n~OL%s si %s %s.\n", pohl(user,"Vykopol","Vykopla"),sklonuj(victim, 4),rm->into);
write_user(user,text);    
sprintf(text,"\n~OL%s ta %s %s.\n", name, pohl(user,"vykopol","vykopla"),user->room->from);
write_user(victim,text);    

if (victim->affected==7) {
	victim->affected=-1;
        victim->affecttime=0;
	sprintf(text,"%s\n",pohl(victim,"Dostal si taku ranu, ze si v momente vytriezvel!","Dostala si taku ranu, ze si v momente vytriezvela!"));
	write_user(victim,text);
	}

}

void autopromote(user,promote)
UR_OBJECT user;
int promote;
{
/* char text2[80],*temp; */

/*if (promote && (temp=jdb_info(JDB_DEMOTE, user->name))!=NULL) {
  sscanf(temp,"%s", text2);
  if (strcmp(user->name, text2)) return;
 }
Zbytocne zatazovalo..  checkuje sa raz za minutu,
promotuje/demotuje sa max do +/- 1 hodiny okolo potrebneho TLT (V)
*/
if (promote) {
  user->level++;
  sprintf(text,"\n~FG~OLPrave si %s na level: ~RS~OL%s!\n",pohl(user,"dosiahol potrebny cas a bol si povyseny","dosiahla potrebny cas a bola si povysena"), level_name[user->level]);
  write_user(user,text);
  sprintf(text,"~FG~OL%s %s na level: ~RS~OL%s.\n",user->name, pohl(user,"bol povyseny","bola povysena"), level_name[user->level]);
  write_room_except(NULL,text,user);
  sprintf(text,"%s autoPROMOTED to level %s.\n",user->name,level_name[user->level]);
  level_log(text);
  write_syslog(text,1); 
 }
else {
  user->level--;
  sprintf(text,"\n~FR~OLPrave si %s na level: ~RS~OL%s!\n",pohl(user,"bol pre nedostatok celkoveho casu degradovany","bola pre nedostatok celkoveho casu degradovana"), level_name[user->level]);
  write_user(user,text);
  sprintf(text,"~FR~OL%s %s na level: ~RS~OL%s ~FR(nedostatok casu).\n",user->name, pohl(user,"bol degradovany","bola degradovana"), level_name[user->level]);
  write_room_except(NULL,text,user);
  sprintf(text,"%s autoDEMOTED to level %s.\n",user->name,level_name[user->level]);
  level_log(text);
  write_syslog(text,1); 
 }

}

/*** Promote a user *** dalsia ZMENA - vypisuje, kto ta promotol */
void promote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char text2[80],*name, *temp,meno[14];
int days;
int ph,lev;

ph=0;
if (word_count<2) {
      write_user(user,"Pouzi: .promote <uzivatel> [dovod] [sure]\n");  return;
      }
      if (user->vis) name=user->name; else name=invisname(user);
/* See if user is on atm */
if ((u=get_user(word[1]))!=NULL) {

      if ((u->level>=user->level) && (strcmp(user->name,"X"))) {
            write_user(user,"Nemozes povysit uzivatela s rovnakym alebo vyssim levelom ako mas sam.\n");
            return;
            }
/* we allow to promote to wizz levels */
    if ((u->level>=PRI) && (strcmp(user->name,"X")) && user->level>=GOD) {
/*    	if (strcmp((char *)md5_crypt(word[2],"horedaj"),PROMOTE_PASSWD))
    	 {
    	write_user(user,"Prepac - toto nieje dostatocny dovod na taky vysoky level!\n");
    	return;
    	}*/
    ph=1;
    }
/*
    if (((u->level+1)>=user->level) && (strcmp(user->name,"X"))) {
    	write_user(user,"Nemozes promotovat na vyssiu alebo rovnaku uroven ako mas sam.\n");
    	return;
    	}
*/
               
days=u->total_login/86400;

if ((u->level==CIT && days<2)  ||
   (u->level==SOL && days<5)  ||
   (u->level==WAR && days<10) ||
   (u->level==SAG && days<18) ||
   (u->level>=PRI)) {
	   if (user->level<GOD) {
	   	vwrite_user(user,"Povysit %s mozes, az ked bude mat dostatocny total-login!\n", sklonuj(u,2));
	   	return;
	   	}
	   if (word_count<3) {
		   write_user(user,"Uzivatel este nema dostatok casu; uved dovod promotu!\n");
		   return;
		   }
	   }
	   
   if ((temp=jdb_info(DB_DEMOTE,u->name))!=NULL) {
        strcpy(text2,jdb_wizz(DB_DEMOTE,u->name));
   	if (strcmp(user->name, text2) && strcmp(user->name,"X")) {
   		if (user->level<GOD || strcmp(word[2],"sure")) {   	
	   		sprintf(text,"%s demotol %s, dovod: ~OL%s~RS.\nLen %s moze promotnut %s naspat.\n", sklonuj(u, 4), text2, temp, text2, sklonuj(u,4));
	   		write_user(user,text);
	   		return;
	   		}
	   	}
   	jdb_vyrad(DB_DEMOTE,u->id);
   	}
      	         	    
      u->level++;
      if (u->level==6) u->level++;
      sprintf(text,"~FG~OL%s si %s na level: ~RS~OL%s.\n",pohl(user,"Povysil","Povysila"), sklonuj(u, 4),level_name[u->level]);
      write_user(user,text);
      sprintf(text,"~FG~OL%s ta %s na level: ~RS~OL%s!\n",name,pohl(user,"povysil","povysila"), level_name[u->level]);
      write_user(u,text);
      sprintf(text,"~FG~OL%s %s na level: ~RS~OL%s.\n",u->name, pohl(u,"bol povyseny","bola povysena"), level_name[u->level]);
      write_room_except(NULL,text,u);
   
   if (word_count<3) {
      sprintf(text,"%s PROMOTED %s to level %s.\n",user->name,u->name,level_name[u->level]);
      level_log(text);
      write_syslog(text,2); 
      }
   else {
      inpstr=remove_first(inpstr);
      sprintf(text,"%s PROMOTED %s to level %s: %s\n",user->name,u->name,level_name[u->level], inpstr);
      if (ph) sprintf(text,"%s PROMOTED %s to %s: *******\n", user->name, u->name, level_name[u->level]);
      level_log(text);
      write_syslog(text,2);
     }      
    police_freeze(user,1);
    return;
   }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v promote().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if ((u->level>=user->level) && (strcmp(user->name,"X"))) {
      write_user(user,"Nemozes povysit na vyssiu alebo rovnaku uroven ako mas sam.\n");
      destruct_user(u);
      destructed=0;
      return;
      }

    if ((u->level>=PRI) && (strcmp(user->name,"X")) && user->level>=GOD) {
    	if (strcmp((char *)md5_crypt(word[2],"horedaj"),PROMOTE_PASSWD))    	
    	 {
    	write_user(user,"Prepac - toto nieje dostatocny dovod na taky vysoky level!\n");
    	destruct_user(u);
    	destructed=0;
       	return;
    	}
    ph=1;
    }

    if (((u->level+1)>=user->level) && (strcmp(user->name,"X"))) {
    	write_user(user,"Nemozes promotovat na vyssiu uroven ako mas sam.\n");
    	destruct_user(u);
    	destructed=0;
    	return;
    	}
    	
days=u->total_login/86400;

/*
if (((u->level==CIT) && (days<2) && (word_count<3))  ||
   ((u->level==SOL) && (days<5) && (word_count<3))  ||
   ((u->level==WAR) && (days<10) && (word_count<3)) ||
   ((u->level==SAG) && (days<18) && (word_count<3)) ||
   ((u->level>=PRI) && (word_count<3)))  {
   destruct_user(u);
   destructed=0;
   write_user(user,"Uzivatel este nema dostatok casu; uved dovod promotu!\n");
   return;
   }
*/

if ((u->level==CIT && days<2)  ||
   (u->level==SOL && days<5)  ||
   (u->level==WAR && days<10) ||
   (u->level==SAG && days<18) ||
   (u->level>=PRI)) {
	   if (user->level<GOD) {
	   	vwrite_user(user,"Povysit %s mozes, az ked bude mat dostatocny total-login!\n", sklonuj(u,2));
	   	destruct_user(u);
	   	destructed=0;
	   	return;
	   	}
	   if (word_count<3) {
		   write_user(user,"Uzivatel este nema dostatok casu; uved dovod promotu!\n");
		   destruct_user(u);
		   destructed=0;
		   return;
		   }
	   }


   if ((temp=jdb_info(DB_DEMOTE,u->name))!=NULL) {
        strcpy(text2,jdb_wizz(DB_DEMOTE,u->name));
   	if (strcmp(user->name, text2) && strcmp(user->name,"X")) {
   		if (user->level<GOD || strcmp(word[2],"sure")) {
	   		sprintf(text,"%s degradoval(a) %s, dovod: ~OL%s~RS.\nLen %s moze promotnut %s naspat.\n", sklonuj(u, 4), text2, temp, text2, sklonuj(u,4));
	   		write_user(user,text);
	   		destruct_user(u);
	   		destructed=0;
	   		return;
	   		}
	   	}
    	 jdb_vyrad(DB_DEMOTE, u->id);
   	}
    
u->level++;
if (u->level==6) u->level++;
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
strcpy(meno,u->name);
lev=u->level;
sprintf(text,"%s si %s na level: ~OL%s.\n",pohl(user,"Povysil","Povysila"), sklonuj(u, 4),level_name[u->level]);
write_user(user,text);
destruct_user(u);
destructed=0;
sprintf(text2,"~FG~OL%s ta %s na level: ~OL%s.~RS~BK~FW\n",name,pohl(user,"povysil","povysila"), level_name[lev]);
send_mail(user,meno,text2);

   if (word_count<3) {
      sprintf(text,"%s PROMOTED %s to level %s.\n",user->name,word[1],level_name[lev]);
      level_log(text);
      write_syslog(text,2); 
      }
   else {
      sprintf(text,"%s PROMOTED %s to level %s: ",user->name,word[1],level_name[lev]);
      inpstr=remove_first(inpstr);
      sprintf(text2,"%s\n",inpstr);
      if (ph) strcpy(text2,"*******\n");
      strcat(text,text2);      
      level_log(text);
      write_syslog(text,2);    
   }
  police_freeze(user,1);
}


/*** Demote a user ** ZMENA - to iste ako promote */
void demote(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char text2[80],*name,meno[14];
int lev,uid=0;

if (word_count<3) {
      write_user(user,"Pouzi: .demote <uzivatel> dovod\n");  return;
      }
if (user->vis) name=user->name; else name=invisname(user);
/* See if user is on atm */
if ((u=get_user(word[1]))!=NULL) {
      if (u->level==NEW) {
            write_user(user,"Uzivatela s levelom OTROK nemozes demotnut.\n");
            return;
            }
      if (u->level>=user->level) {
            write_user(user,"Nemozes znizit level uzivatelovi s rovnakym alebo vyssim levelom ako mas sam.\n");
            return;
            }
      u->level--;
      sprintf(text,"~FR~OLDegradoval si %s na level ~RS~OL%s.\n",sklonuj(u, 4),level_name[u->level]);
      inpstr=remove_first(inpstr);
        
      write_user(user,text);

      sprintf(text,"~FR~OL%s ta degradoval na uroven: ~RS~OL%s! ~FRDovod: ~RS~OL%s\n",name,level_name[u->level],inpstr);
      write_user(u,text);
      
      sprintf(text,"~FR~OL%s %s na level: ~RS~OL%s~FR, dovod: ~RS~OL%s\n",u->name, pohl(u,"bol degradovany","bola degradovana"), level_name[u->level],inpstr);
      write_room_except(NULL,text,u);

      sprintf(text,"%s DEMOTED %s to level %s: %s\n",user->name,u->name,level_name[u->level],inpstr);
      level_log(text);
      write_syslog(text,2);
      jdb_zarad(DB_DEMOTE,u->id,user->id,inpstr);
      police_freeze(user,1);
      return;
      }
/* User not logged on */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v demote().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->level==NEW) {
      write_user(user,"Nemozes znizit uroven OTROKovi.\n");
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->level>=user->level) {
      write_user(user,"Nemozes znizit level uzivatelovi s levelom vyssim alebo rovnakym ako mas sam.\n");
      destruct_user(u);
      destructed=0;
      return;
      }
u->level--;
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
sprintf(text,"Degradoval si %s na: ~OL%s.\n",sklonuj(u, 4),level_name[u->level]);
write_user(user,text);
strcpy(meno,u->name);
lev=u->level;
uid=u->id;
destruct_user(u);
destructed=0;
sprintf(text2,"~FR~OL%s Ta degradoval%s na level: ~RS~OL%s~RS~BK~FW.\n",name,pohl(user,"","a"),level_name[lev]);
send_mail(user,meno,text2);
sprintf(text,"%s DEMOTED %s to level %s: ",user->name,word[1],level_name[lev]);
inpstr=remove_first(inpstr);
sprintf(text2,"%s\n",inpstr);
strcat(text,text2);
level_log(text);
write_syslog(text,2);
jdb_zarad(DB_DEMOTE,uid,user->id,inpstr);
police_freeze(user,1);
}


/*** List banned sites or users ***/
void listbans(user)
UR_OBJECT user;
{
int i,what=0;
char filename[80];

if (!strncmp(word[1],"us",2)) {
  what=DB_BAN;
  write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Vyblokovani uzivatelia ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
  write_user(user,"~OL~FW Uzivatel     Vyblokoval   Dovod vyblokovania\n~OL~FB------------------------------------------------------------------------------\n");
 }
if (!strncmp(word[1],"de",2)) {
  what=DB_DEMOTE;
  write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Degradovani uzivatelia ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
  write_user(user,"~OL~FW Uzivatel     Degradoval   Dovod degradovania\n~RS~FG------------------------------------------------------------------------------\n");
 }
if (!strncmp(word[1],"jai",3)) {
  what=DB_JAIL;
  write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Uzivatelia v zalari ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
  write_user(user,"~OL~FW Uzivatel     Zazalaroval  Dovod zalarovania\n~OL~FR------------------------------------------------------------------------------\n");
 }

if (what>0) {
  sprintf(query,"select `users`.`username`,`outlaws`.`wizzid`,`outlaws`.`reason` from `outlaws`,`users` where `outlaws`.`type`=%d and `users`.`id`=`outlaws`.`userid`",what);
  if ((result=mysql_result(query))) {
    while ((row=mysql_fetch_row(result))) {
      sprintf(text," %-12s %10s %s\n",row[0],row[1],row[2]);
      write_user(user,text);
     }
    mysql_free_result(result);
   }
  return;
 }

if (!strncmp(word[1],"ren",3)) {
  write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Premenovani uzivatelia ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
  sprintf(query,"select `username`,`prevname` from `users` where length(`prevname`)>0");
  write_user(user,"~OL~FWNove meno    Stare meno  (x => uz sa neda premenovat)\n~OL~FR------------------------------------------------------------------------------\n");
  if ((result=mysql_result(query))) {
    while ((row=mysql_fetch_row(result))) {
      sprintf(text," %-12s %-12s\n",row[0],row[1]);
      write_user(user,text);
     }
    mysql_free_result(result);
   }
  return;
 }      
      
if (!strncmp(word[1],"si",2)) {
      write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Vyblokovane adresy ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      sprintf(filename,"misc/%s",SITEBAN);
      switch(more(user,user->socket,filename)) {
            case 0:
            write_user(user,"Ziadne adresy neboli najdene.\n\n");
            return;

            case 1: user->misc_op=2;
            }
      return;
      }
if (!strncmp(word[1],"mul",3)) {
      write_user(user,title("~OLRequesty na pouzite adresy","~FR"));
      sprintf(filename,"%s",MULTI_REQUEST);
      switch(more(user,user->socket,filename)) {
            case 0:
            write_user(user,"Ziadne requesty na pouzite adresy neboli najdene.\n\n");
            return;
            case 1: user->misc_op=2;
            }
      return;
      }
if (!strncmp(word[1],"sw",2)) {
      write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Cenzurovane slova ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      i=0;
      while(swear_words[i][0]!='*') {
            write_user(user,swear_words[i]);
            write_user(user,"\n");
            ++i;
            }
      if (!i) write_user(user,"Niesu tu ziadne nadavky.\n");
      if (ban_swearing) write_user(user,"(Blokovanie nadavok je zapnuta, ziadny prikaz obsahujuci nadavky sa nevykona.)\n");
      else write_user(user,"\n(Blokovanie nadavok je vypnute, nadavky sa iba vybodkuju).\n\n");
      return;
      }
write_user(user,"Pouzi: .listbans sites/users/jails/demotes/swears/renames/multirequest\n");
}


/*** Ban a site/domain or user (S) zmena ***/
void ban(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
char *usage="Pouzi: .ban <adresa/meno> <dovod (len pri uzivatelovi)>\n";

if (word_count<2) {
      write_user(user,usage);  return;
      }
      
if (strstr(word[1],"."))  {  ban_site(user);  return;  }
if (word_count<3) {
	write_user(user,usage); return;
	}
ban_user(user, inpstr);  return;
}

void ban_site(user)
UR_OBJECT user;
{
FILE *fp;
char filename[80],host[81],site[102];

gethostname(host,80);
if (!strcmp(word[1],host)) {
      write_user(user,"Nemozes vyblokovat stroj, na ktorom bezi talker.\n");
      return;
      }
sprintf(filename,"misc/%s",SITEBAN);

/* See if ban already set for given site */
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	
      fscanf(fp,"%s",site);
      while(!feof(fp)) {
            if (!strcmp(site,word[1])) {
                  write_user(user,"Tato adresa/domena uz je vyblokovana.\n");
                   fclose(fp);  return;
                  }
            fscanf(fp,"%s",site);
            }
       fclose(fp);
      }

/* Write new ban to file */
if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
      sprintf(text,"%s: Nemozno otvorit subor pre doplnanie.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno otvorit subor pre doplnanie v ban_site().\n",0);
      return;
      }
      
fprintf(fp,"%s\n",word[1]);
fclose(fp);
write_user(user,"Adresa bola vyblokovana.\n");
sprintf(text,"%s BANNED domain %s.\n",user->name,word[1]);
write_syslog(text,1);
write_syslog(text,2);
police_freeze(user,2);
}

void ban_site_for_newuser(user)
UR_OBJECT user;
{
FILE *fp;
char filename[80],site[102];

sprintf(filename,"misc/%s",NEWUSER_SITEBAN);
if (word_count<2) {
	write_user(user,"Pouzi: .newban <adresa>\nMomentalne vyblokovane adresy:\n");
	showfile(user,filename);
	return;
	}
	
if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/
	
      fscanf(fp,"%s",site);
      while(!feof(fp)) {
            if (!strcmp(site,word[1])) {
                  write_user(user,"Tato adresa/domena uz je vyblokovana pre zakladanie novych kont.\n");
                   fclose(fp);  return;
                  }
            fscanf(fp,"%s",site);
            }
       fclose(fp);
      }

if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
      sprintf(text,"%s: Nemozno otvorit subor pre doplnanie.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno otvorit subor pre doplnanie v ban_site_for_newuser().\n",0);
      return;
      }
      
fprintf(fp,"%s\n",word[1]);
fclose(fp);
write_user(user,"Adresa bola vyblokovana pre zakladanie novych kont.\n");
sprintf(text,"%s BANNED domain %s for new-users.\n",user->name,word[1]);
write_syslog(text,1);
write_syslog(text,2);
}



void ban_user(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
/* FILE *fp; */
/* char filename2[80],p[45]; */
/* int a,b,c,d; */
int level,uid;

word[1][0]=toupper(word[1][0]);
if (!strcmp(user->name,word[1])) {
      write_user(user,"Snaha vyblokovat sameho seba je siedmy priznak sialenstva.\n");
      return;
      }

/* See if already on */
if ((u=get_user(word[1]))!=NULL) {
      if (u->level>=user->level) {
            write_user(user,"Nemozes vyblokovat uzivatela s rovnakym/vyssim levelom ako mas sam.\n");
            return;
            }
      }
else {
      /* User not on so load up his data */
/*      sprintf(filename2,"%s/%s.D",USERFILES,word[1]); */
/*      if (!(fp=ropen(filename2,"r"))) { */
      level=db_load_user_level(word[1]);
      if (level<0) {
        write_user(user,nosuchuser);
        return;
       }
      if (level>=user->level) {
        write_user(user,"Nemozes vyblokovat uzivatelas rovnakym/vyssim levelom ako mas sam.\n");
        return;
       }
     }
uid=db_userid(word[1]);
if (uid<0) {
  write_user(user,"Ohh dear, drastic error occured!\n");
  return;
 }
inpstr=remove_first(inpstr);
if (!jdb_zarad(DB_BAN,uid,user->id,inpstr)) {
  write_user(user,"Tento uzivatel uz je vyblokovany.\n");
  return;
 }
	
write_user(user,"Uzivatel bol vyblokovany.\n");
sprintf(text,"%s BANNED user %s: %s\n",user->name, word[1], inpstr);
write_syslog(text,1);
write_syslog(text,2);
if (u!=NULL) {
/*      sprintf(text,"\n\07~FG%s ti %s konto! Dovod: ~OL%s~RS~FG\n~OL~FWKontaktuj spravcov mailom (%s@%s), alebo sa obrat priamo\n~OL~FW", user->name, pohl(user,"zablokoval", "zablokovala"), inpstr, sklonuj(user,2), user->name, TALKER_EMAIL_ALIAS, TALKER_EMAIL_HOST); */
      sprintf(text,"\n\07~FG%s ti %s konto! Dovod: ~OL%s~RS~FG\n", user->name, pohl(user,"zablokoval", "zablokovala"), inpstr);
      write_user(u,text);
      disconnect_user(u,1,NULL);
      inyjefuc=1;
      }
police_freeze(user,1);
}

/*** unban a site (or domain) or user (S) zmena ***/
void unban(user)
UR_OBJECT user;
{
char *usage="Pouzi: .unban <adresa/nick>\n";

if (word_count<2) {
      write_user(user,usage);  return;
      }
if (strstr(word[1],".")) {  unban_site(user);  return;  }
unban_user(user);  return; 
}

void unban_site(user)
UR_OBJECT user;
{
FILE *infp,*outfp;
char filename[80],site[105];
int found,cnt;

sprintf(filename,"misc/%s",SITEBAN);
if (!(infp=ropen(filename,"r"))) { /*APPROVED*/
      write_user(user,"Tato adresa/domena nieje vyblokovana.\n");
      return;
      }
      
if (!(outfp=ropen("tempfile","w"))) { /*APPROVED*/
      sprintf(text,"%s: Couldn't open tempfile.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno otvorit docasny subor na zapis v unban_site().\n",0);
       fclose(infp);
      return;
      }
      
found=0;   cnt=0;
fscanf(infp,"%s",site);
while(!feof(infp)) {
      if (strcmp(word[1],site)) {
            fprintf(outfp,"%s\n",site);  cnt++;
            }
      else found=1;
      fscanf(infp,"%s",site);
      }
 fclose(infp);
 fclose(outfp);
if (!found) {
      write_user(user,"Tato adresa/domena nieje vyblokovana.\n");
      unlink("tempfile");
      return;
      }
if (!cnt) {
      unlink(filename);  unlink("tempfile");
      }
else rename("tempfile",filename);
write_user(user,"Blokovanie adresy bolo zrusene.\n");
sprintf(text,"%s UNBANNED site %s.\n",user->name,word[1]);
write_syslog(text,1);
write_syslog(text,2);
}


void unban_site_for_newuser(user)
UR_OBJECT user;
{
FILE *infp,*outfp;
char filename[80],site[105];
int found,cnt;

sprintf(filename,"misc/%s", NEWUSER_SITEBAN);
if (word_count<2) {
	write_user(user,"Pouzi: .newunban <adresa>\nMomentalne vyblokovane adresy:\n");
	showfile(user,filename);
	return;
	}

if (!(infp=ropen(filename,"r"))) { /*APPROVED*/
      write_user(user,"Tato adresa/domena nieje vyblokovana pre zakladanie novych kont.\n");
      return;
      }
      
if (!(outfp=ropen("tempfile","w"))) { /*APPROVED*/
      sprintf(text,"%s: Couldn't open tempfile.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno otvorit docasny subor na zapis v unban_site_for_newuser().\n",0);
       fclose(infp);
      return;
      }
      
found=0;   cnt=0;
fscanf(infp,"%s",site);
while(!feof(infp)) {
      if (strcmp(word[1],site)) {
            fprintf(outfp,"%s\n",site);  cnt++;
            }
      else found=1;
      fscanf(infp,"%s",site);
      }
 fclose(infp);
 fclose(outfp);
if (!found) {
      write_user(user,"Tato adresa/domena nieje vyblokovana pre zakladanie novych kont.\n");
      unlink("tempfile");
      return;
      }
if (!cnt) {
      unlink(filename);  unlink("tempfile");
      }
else rename("tempfile",filename);
write_user(user,"Blokovanie adresy pre zakladanie novych kont bolo zrusene.\n");
sprintf(text,"%s UNBANNED site %s for new-users.\n",user->name,word[1]);
write_syslog(text,1);
write_syslog(text,2);
}


void unban_user(user)
UR_OBJECT user;
{
char *info;
char kto[14];
int uid=0;

word[1][0]=toupper(word[1][0]);

info=jdb_info(DB_BAN, word[1]);

if (info==NULL) { 
	write_user(user,"Tento uzivatel nieje vyblokovany!\n");	
	return;
	}
strcpy(kto,jdb_wizz(DB_BAN,word[1]));

if (strncmp(info,"Pravdepodobne",13) && strcmp(kto, user->name)) {
	sprintf(text,"Prepac, len %s moze odblokovat tohoto uzivatela.\nDovod BANu bol: ~OL%s~RS\n", kto, info);
	write_user(user, text);
	return;
	}

uid=db_userid(word[1]);
if (!jdb_vyrad(DB_BAN,uid)) {
	write_user(user,"Tento uzivatel nieje vyblokovany!\n");
	return;
	}
	
write_user(user,"Uzivatelovi bol znovu povoleny pristup.\n");
sprintf(text,"%s UNBANNED user %s.\n",user->name,word[1]);
write_syslog(text,1);
write_syslog(text,2);
}

void hide(UR_OBJECT user)
{
int hide;
 if (user->room->group!=4) {
   write_user(user,"Prikaz hide funguje iba na ostrove brutalis.\n");
   return;
  }
 hide=1;
 if (!strncmp(word[2],"can",3) || !strncmp(word[2],"sto",3)) hide=0;
 if (!hide) {
   if (user->hidden==0) {
     sprintf(text,"~FTVed nie si skryt%s, nerob paniku.\n",pohl(user,"y","a"));
     write_user(user,text);
    }
   else if (user->hidden==1) {
     sprintf(text,"~FTVyliez%s si z ukrytu.\n",pohl(user,"ol","la"));
     write_user(user,text);
     sprintf(text,"~FT%s vyliez%s z ukrytu.\n",user->name,pohl(user,"ol","la"));
     write_room_except(user->room,text,user);
     user->hidden=0;
    }
   else if (user->hidden>1) {
     sprintf(text,"~FTPrestal%s si sa maskovat.\n",pohl(user,"","a"));
     write_user(user,text);
     user->hidden=0;
    }
   return;
  }
 else {
   if (user->hidden==1) {
     sprintf(text,"~FTUz si skryt%s, ak chces vyjst z ukrytu, pouzi: .hide stop.\n",pohl(user,"y","a"));
     write_user(user,text);
    }
   else if (user->hidden>1) {
     sprintf(text,"~FTVed uz sa maskujes.\n");
     write_user(user,text);
    }
   else if (user->hidden==0) {
     sprintf(text,"~FTZacal%s si sa maskovat.\n",pohl(user,"","a"));
     write_user(user,text);
     user->hidden=5;
    }
   return;
  }
}

/*** Set user visible or invisible ***/
void visibility(user,vis)
UR_OBJECT user;
int vis;
{

if (vis) {
      if (user->vis) {
            sprintf(text,"Uz si viditeln%s, nerob paniku.\n",pohl(user,"y","a"));
            write_user(user,text);
            return;
            }
      write_user(user,"~FB~OLRecitujes melodicke zaklinadlo a tvoje telo sa materializuje.\n");
      sprintf(text,"~FB~OLPocujes recitovat melodicke zaklinadlo a %s sa materializuje pred tebou!\n",user->name);
      write_room_except(user->room,text,user);
      user->vis=1;
      return;
      }
if (!user->vis) {
      sprintf(text,"Uz si neviditeln%s, nerob paniku.\n",pohl(user,"y","a"));
      write_user(user,text);
      return;
      }
sprintf(text,"~FB~OLRecitujes melodicke zaklinadlo a tvoje telo mizne - si %s!\n",pohl(user,"neviditelny","neviditelna"));
write_user(user,text);
sprintf(text,"~FB~OL%s recituje melodicke zaklinadlo a mizne v modrej hmle!\n",user->name);
write_room_except(user->room,text,user);
user->vis=0;
}

/*** Site a user ***/
void site(user)
UR_OBJECT user;
{
UR_OBJECT u;

if (word_count<2) {
      write_user(user,"Pouzi: .site <uzivatel>\n");  return;
      }
/* User currently logged in */
if ((u=get_user(word[1]))!=NULL) {
      sprintf(text,"%s je %s z adresy %s:%d [%s]\n",u->name,pohl(u,"prihlaseny","prihlasena"), u->site,u->site_port, u->ipcka);
      write_user(user,text);
      return;
      }
/* User not logged in */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nepodarilo sa vytvorit docasny user object v site().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
sprintf(text,"%s %s z adresy %s.\n",word[1],pohl(u,"bol naposledny prihlaseny","bola naposledy prihlasena"), u->last_site);
write_user(user,text);
destruct_user(u);
destructed=0;
}

/*** Wake up some sleepy herbert ***/
void wake(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char *name;

if (word_count<2) {
      if (user->level<KIN) write_user(user,"Pouzi: .wake <uzivatel> [<sprava>]\n");
      else write_user(user,"Pouzi: .wake <uzivatel> [force] [<sprava>]\n");
      return;
      }
if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes nikoho budit.\n");  return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Snaha zobudit sameho seba je osmy priznak sialenstva.\n");
      return;
      }
/*if (user->level<KIN && u->room->group!=user->room->group) {
      sprintf(text,"%s je od teba prilis %s na budenie!\n",u->name,pohl(u,"vzdialeny","vzdialena"));
      write_user(user,text);
      return;
    }*/
if (u->afk && strcmp(u->afk_mesg,"auto-afk")) {
      write_user(user,"Uzivatel je mimo klavesnice - nemozes ho zobudit.\n");  return;
      }  
if (u->ignall && (user->level<KIN || strcmp(word[2],"force"))) {
      if (u->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",u->name);
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
      write_user(user,text);
      if (user->level>=KIN)
       write_user(user,"Ak mas na to zavazny dovod, pouzi .wake <user> force <sprava>\n");
      return;
      }
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
if (!strcmp(u->room->name,"svatyna")) {
	write_user(user,"Uzivatel medituje v svatyni a nezela si byt ruseny!\n");
	return;
	}
if (!strcmp(user->room->name,"svatyna")) {
	write_user(user,"Prave meditujes vo svatyni a nemas zaujem nikoho budit.\n");
	return;
	}
 
if (user->pp < WAKECAN) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, WAKECAN); 
	write_user(user,text); 
	return; 
	} 
  
if (user->vis) name=user->name; else name=invisname(user);  
if (!strcmp(word[2],"force"))
 { write_user(u,"\n"); inpstr=remove_first(inpstr); }
if (!u->ignfun && !u->ignall) {
        sprintf(query,"select `body` from `files` where `filename`='wake_up.pic';");
        if ((result=mysql_result(query))) {
          if ((row=mysql_fetch_row(result)) && row[0]) {
            write_user(u,row[0]);
           }
          else write_user(user,"Nenasiel som budiaci obrazok.\n");
          mysql_free_result(result);
         }
        else write_user(user,"Nenasiel som budiaci obrazok.\n");
	/* \07 sposobi pipnutie - co takto ich tam dat 5??? ;) */  
	if (word_count>2) {
	                  inpstr=remove_first(inpstr);
	                  sprintf(text,"~LB~LB~LB\n*** %s krici: ~LI~OL%s ~RS***\n",name,inpstr);
	                  write_user(u,text);
	                  }
	else {
	      sprintf(text,"~LB~LB~LB\n*** %s krici: ~OL~LIZOBUD SA!!!~RS ***\n\n",name);
	      write_user(u,text);
	     }
	}
	
   else {
   	sprintf(text,"~LB~LB~LB~RS* ~OL~LIVSTAVAJ ~RS* ~OL~LIVSTAVAJ ~RS* ~OL%s krici: ZOBUD SA!!! ~RS* ~OL~LIVSTAVAJ ~RS* ~OL~LIVSTAVAJ ~RS*\n",name);
   	writecent(u,text);
   	if (word_count>2) {
   		inpstr=remove_first(inpstr);
   		sprintf(text,"~OL- ~RS%s ~OL-\n",inpstr);
   		writecent(u,text);
   		}
   	}
	
sprintf(text,"%s budicek...\n",pohl(u,"Dostal","Dostala"));  
write_user(user,text);  
 
decrease_pp(user,WAKEDEC,NODYNAMIC);
 
}  

/*** ZMENA: Hug - hug me tight! ;) - to bolo "Hold Me Tight" ty truba :> ***/
                                         /* --^^^^^^^^^^^^^^^^^^^^^^^ :>>>*/
void send_hug(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char *name;

if (word_count<2) {
      write_user(user,"Pouzi: .hug <uzivatel> [text]\n");  return;
      }
if (user->muzzled) {
      write_user(user,"Si umlcany a nemozes ani nikoho objimat.\n"); return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Skusat objimat sam seba je dvanasty priznak sialenstva.\n");
      return;
      }

/*if (u->room->group!=user->room->group) {
      sprintf(text,"%s je od teba prilis %s na objatia...\n",u->name,pohl(u,"vzdialeny","vzdialena"));
      write_user(user,text);
      return;
    }
*/
      
if (u->afk) {
      write_user(user,"Uzivatel je mimo klavesnice, cize ta nemoze pocut.\n");  return;
      }
if (u->ignall) {
      if (u->malloc_start!=NULL)
            sprintf(text,"%s prave nieco pise v editore.\n",u->name);
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
      write_user(user,text);
      return;
      }
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
if (!strcmp(u->room->name,"svatyna")) {
	write_user(user,"Uzivatel medituje v svatyni a nezela si byt ruseny!\n");
	return;
	}

if (!strcmp(user->room->name,"svatyna")) {
	write_user(user,"Prave meditujes vo svatyni, nemas zaujem s nikym sa objimat!\n");
	return;
	}
  
if (user->pp < HUGCAN) {    /*PP*/ 
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, HUGCAN); 
	write_user(user,text); 
	return; 
	} 

if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
	if (user->room->sndproof) {
		sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s Ta nemoze pocut.\n",u->name);
		write_user(user,text);
		return;
		}
	if (u->room->sndproof) {
		sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze Ta pocut.\n",u->name);
		write_user(user,text);
		return;
		}      
      }
      
if ((user->jailed) && (user->pp<MAXPP)) {
	write_user(user,"V zalari LEN cez mreze (s plnymi pp-ckami ;)!\n");
	return;
	}

if (u->affpermanent>-1 && !strcmp(predmet[u->affpermanent]->name,"sliepka"))
 u->affpermanent=-1; /* odsliepkaj */
if (user->vis) name=user->name; else name=invisname(user);
if (word_count>2) inpstr=remove_first(inpstr);

if (!u->ignfun) {
  sprintf(query,"select `body` from `files` where `filename`='hug.pic';");
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result)) && row[0]) {
      write_user(u,row[0]);
     }
    else write_user(user,"Nenasiel som objimaci obrazok.\n");
    mysql_free_result(result);
   }
  else write_user(user,"Nenasiel som objimaci obrazok.\n");
  if (word_count>2)
   sprintf(text,"\n~FG*** ~OL~FGPozdravuje Ta %s: ~FW%s ~RS~FG***\n",name,inpstr);
  else 
   sprintf(text,"\n~FG*** ~OL~FGPozdravuje Ta %s! :-)~RS~FG ***\n",name);
  write_user(u,text);
 }
else {
  sprintf(text,"~OL~FM*~RS~FM--~OL~FM* ~FYH U G ~FM*~RS~FM--~OL~FM* ~FTPozdravuje Ta %s! ~FM*~RS~FM--~OL~FM* ~FYH U G ~FM*~RS~FM--~OL~FM*\n",name);
  writecent(u,text);
  if (word_count>2) {
    sprintf(text,"~OL~FM*** ~FW%s ~RS~OL~FM***\n",inpstr);
    writecent(u,text);
   }
 }
if (word_count>2) {
  sprintf(text,"~OL~FTObjal%s si ~FW%s~FT a zasepkal%s:~RS~FW %s\n",pohl(user,"","a"),sklonuj(u,4),pohl(user,"","a"),inpstr);
  write_user(user,text);

  sprintf(text,"~OL~FW%s~FT~OL ta %s:~RS~FW %s\n",name,pohl(user,"objal a zasepkal","objala a zasepkala"),inpstr);
  record_tell(u,text);
  sprintf(text,"~OL~FT%s si ~OL~FW%s~OL~FT a %s:~RS~FW %s\n",pohl(user,"Objal","Objala"),sklonuj(u,4),pohl(user,"zasepkal","zasepkala"),inpstr);
  record_tell(user,text);
 }
else {
  sprintf(text,"~OL~FTObjal%s si ~FW%s~FT...~RS~FW\n",pohl(user,"","a"),sklonuj(u,4));
  write_user(user,text);

  sprintf(text,"~OL~FW%s~FT ta %s...~RS\n",name,pohl(user,"objal","objala"));
  record_tell(u,text);
  sprintf(text,"~OL~FT%s si ~FW%s~FT...~RS\n",pohl(user,"Objal","Objala"),sklonuj(u,4));
  record_tell(user,text);
}

if (!user->afro) decrease_pp(user,HUGDEC,NODYNAMIC); 
 
}

/*** ZMENA: bomb Thiz rulez! ;) ***/

void send_bomb(user)
UR_OBJECT user;
{
UR_OBJECT u;
char /* *name, */filename[80];

if (word_count<2) {
      write_user(user,"Pouzi: .bomb <uzivatel>\n");  return;
      }
if (user->muzzled) {
      write_user(user,"Si umlcany, nebudes bombovat!\n"); return;
      }
if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Skusat bombovat sam seba je sedemnasty priznak sialenstva.\n");
      return;
      }
if (u->level>user->level) {
      write_user(user,"Nemozes bombovat vyssi level.\n");
      return;
      }            
if (u->afk) {
      write_user(user,"Uzivatel je mimo klavesnice - nic nebude.\n");  return;
      }
/* tuto to netreba... ;)  
if (u->ignall) {  
      if (u->malloc_start!=NULL)  
	    sprintf(text,"%s prave nieco pise v editore.\n",u->name);  
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);  
      write_user(user,text);  
      return;  
      }  
*/  

if (((user->room->sndproof) || (u->room->sndproof)) && (user->room!=u->room)) {
	if (user->room->sndproof) {
		sprintf(text,"Nachadzas sa vo zvukotesnej miestnosti, %s ta nemoze pocut.\n",u->name);
		write_user(user,text);
		return;
		}
	if (u->room->sndproof) {
		sprintf(text,"%s sa nachadza vo zvukotesnej miestnosti, nemoze ta pocut.\n",u->name);
		write_user(user,text);
		return;
		}      
      }

  
/* if (user->vis) name=user->name; else name=invisname(user); */
sprintf(filename,"misc/bomb");
            switch(more(u,u->socket,filename)) {
                  case 0: write_user(user,"Nenasiel som bombu.\n");  return;
                  case 1: u->misc_op=2;
                  }
sprintf(text,"\n~FG*** ~OL~FR~LIUZI SI TO!~RS~FG ***\n\n");
write_user(u,text);
write_user(user,"~OLBuuuuuuuuuuuuuuuuuuuuuum. Teraz pockame kym sa odhlasi...\n");
}

/* KISS - posiela pusinku... ZMENA!!!
	Kompletne prerobeny ... (S)
*/

void send_kiss(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
RM_OBJECT from_rm, partner_rm;
char *name;
int obrazok=1;
int prenesie=1;

if (word_count<2) {
      write_user(user,"Pouzi: .kiss <uzivatel> [sprava]\n");
      return;
      }
           
/* 1. cast - kedy sa nemoze VOBEC kissnut */      
if (user->muzzled) {
      write_user(user,"Si umlcany a nemozes ani nikoho bozkavat.\n");
      return;
      }

if (!(u=get_user(word[1]))) {
      write_user(user,notloggedon);
      return;
      }
      
from_rm=user->room;
partner_rm=u->room;
      
if (u==user) {
      write_user(user,"Skusat bozkavat sam seba je trinasty priznak sialenstva.\n");
      return;
      }
      
if (u->afk) {
      write_user(user,"Uzivatel je mimo klavesnice, cize sa s nim nemozes bozkavat. :)\n");
      return;
      }
      
if (u->ignall) {
      if (u->malloc_start!=NULL)
	    sprintf(text,"%s prave nieco pise v editore.\n",u->name);
      else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
      write_user(user,text);
      return;
      }
      
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }

if (u->room->group!=user->room->group) {
      sprintf(text,"%s je od teba prilis %s na bozky...\n",u->name,pohl(u,"vzdialeny","vzdialena"));
      write_user(user,text);
      return;
    }

if (strcmp(from_rm->name,partner_rm->name)) {
	sprintf(text,"%s nieje v tejto miestnosti!\n",u->name);
	write_user(user,text);
	return;
	}

if (!strcmp(u->room->name,"svatyna")) {
	write_user(user,"Uzivatel medituje v svatyni a nezela si byt ruseny!\n");
	return;
	}
	
if (user->pp < KISSCAN) {    /*PP*/
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, KISSCAN);
	write_user(user,text);
	return;
	}

/* 2. cast - kedy neprenesie do Afrochramu */

if (from_rm->access==GOD_PRIVATE) prenesie=0;
if (from_rm->access==PRIVATE || from_rm->access==FIXED_PRIVATE) prenesie=0;
if (!strcmp(from_rm->name,"zalar") || !strcmp(from_rm->name,"letun") || !strcmp(from_rm->name,"plachetnica")) prenesie=0;
if (u->ignfun) prenesie=0;
if ((user->level+3)<u->level) prenesie=0;

/* 3. cast - kedy neposle obrazok */

if ((u->ignfun) && (strcmp(partner_rm->name,"afroditin_chram"))) obrazok=0;

/* 4. cast - samotne vykonanie vsetkeho! */

/* kiss v amfiku = tixy! :-) */
if (user->vis) name=user->name; else name=invisname(user);
if ((!strcmp(u->room->name, "amfiteater")) && (play.on==1)) {
	sprintf(text,"~OL~FR%s Ta %s!\n", name, pohl(user,"pobozkal","pobozkala"));
	write_user(u,text);
	if (u->affpermanent==19) u->affpermanent=-1; /* ozabauj */
	write_user(user,"~OL~FR*Cmuk!*\n");
	return;
	}

if (obrazok) {
  sprintf(query,"select `body` from `files` where `filename`='kiss';");
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result)) && row[0]) {
      write_user(u,row[0]);
     }
    else write_user(user,"Nenasiel som bozkavaci obrazok.\n");
    mysql_free_result(result);
   }
  else write_user(user,"Nenasiel som bozkavaci obrazok.\n");
 }

if (strcmp(from_rm->name,"afroditin_chram") && strcmp(from_rm->name,PORTALIS_KISS) && prenesie)  {
	if (user->room->group==1) {
		move_user(user,get_room("afroditin_chram",NULL),3);
	   	move_user(u,get_room("afroditin_chram",NULL),3);
		}
	else if (user->room->group==2) {
	   	move_user(user,get_room(PORTALIS_KISS,NULL),3);
		move_user(u,get_room(PORTALIS_KISS,NULL),3);
		}
	}

if (word_count>2) {
                  inpstr=remove_first(inpstr);
                  sprintf(text,"~FR*~OLCMUK!~RS~FR* ~OL~FR%s si %s: ~OL~FW%s ~RS~FR*~OLCMUK!~RS~FR*\n", pohl(user,"Pobozkal","Pobozkala"), sklonuj(u, 4),inpstr);
                  write_user(user,text);
                  sprintf(text,"~FR*CMUK!* ~OL~FR%s ta bozkava: ~FW%s ~RS~FR*CMUK!*\n",name,inpstr);
                  write_user(u,text);
                  sprintf(text,"~FR%s~FW ~OLta %s: ~RS%s\n",name,pohl(user,"pobozkal a zasepkal","pobozkala a zasepkala"),inpstr);
                  record_tell(u,text);
		  sprintf(text,"~OL%s si ~RS~FR%s~OL~FW a %s si: ~RS%s\n",pohl(user,"Pobozkal","Pobozkala"),sklonuj(u,4),pohl(user,"zasepkal","zasepkala"),inpstr);
                  record_tell(user,text);
	          }
	else {
              sprintf(text,"~FR*~OLCMUK!~RS~FR* ~OL~FR%s si %s! ~FR*~OLCMUK!~RS~FR*\n", pohl(user,"Pobozkal","Pobozkala"), sklonuj(u, 4));
              write_user(user,text);
	      sprintf(text,"~FR*CMUK!* ~OL~FR%s sa s Tebou bozkava!~RS~FR *CMUK!*\n",name);
	      write_user(u,text);
	      sprintf(text,"~FR%s~FW~OL ta %s...~RS\n",name,pohl(user,"pobozkal","pobozkala"));
	      record_tell(u,text);
              sprintf(text,"~OL%s si ~RS~FR%s~OL~FW...~RS\n",pohl(user,"Pobozkal","Pobozkala"),sklonuj(u,4));
     	      record_tell(user,text);
	     }		  	
	     
sprintf(text,"~OL~FR%s %s %s! ~RS~FR. o O (~OL*CMUK!*~RS~FR)\n",name,pohl(user,"pobozkal","pobozkala"), sklonuj(u, 4));
write_room_except2users(user->room,text,u,user);
if (!user->afro) decrease_pp(user,KISSDEC,NODYNAMIC);
if (u->affpermanent>-1 && !strcmp(predmet[u->affpermanent]->name,"zaba"))
 u->affpermanent=-1; /* odzabuj */
}

/* kidnap - unesie usera - ZMENA Spakky, robi to iste, ako ked ti niekto
   povie "muvni ma k sebe" (pozn. Buko ;-)
   UNOS!!! Toto je prikaz ktory sluzi na "unesenie" daneho lusera do
   miestnosti kde som ja. Je to jednoduhsie ako pozerat kde je dany luzer
   a nasledne ho movnut. */

void kidnap(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT odkial, kam;
char *name;
if (word_count<2) {
      write_user(user,"Pouzi: .kidnap <uzivatel>\n");  return;
      }
if (!(u=get_user(word[1]))) {      
      write_user (user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Nemozes uniest sam seba!\n");
      return;
      }
if (u->level>user->level) {
      write_user(user,"Nemozes uniest vyssi level ako mas sam!\n");	      
      return;
      }
if (!u->level) {
	write_user(user,"Nemozes unasat novacika!\n");
	return;
	}
if (u->afk) {
      write_user(user,"Uzivatel je mimo klavesnice, nemozes ho uniest!\n");  return;
      }
if (u->malloc_start) {
	write_user(user,"Uzivatel prave nieco pise v editore, nemozes ho uniest!\n");
	return;
	}      
if (u->filepos) {
	write_user(user,"Uzivatel prave cita nejaky text, nemozes ho uniest!\n");
	return;
	}
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }
if (u->glue) {
	sprintf(text,"%s je %s k zemi zuvackou!\n",u->name,pohl(u,"prilepeny","prilepena"));
	write_user(user,text);
	return;
	}

if (user->pp < KIDCAN) {    /*PP*/
	sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, KIDCAN);
	write_user(user,text);
	return;
	}
odkial=u->room;
kam=user->room;
if (odkial->access==GOD_PRIVATE) {
      write_user(user,"Nemozes uniest niekoho kto je v BOZSKEJ miestnosti!\n");
      return;
      }
      
if (odkial->access==PRIVATE || odkial->access==FIXED_PRIVATE) {
	write_user(user,"Nemozes uniest niekoho kto je v PRIVATNEJ miestnosti!\n");
	return;
	}
      
if ((!strcmp(u->room->name,"amfiteater")) && (play.on==1)) {
	write_user(user,"Uzivatel pozera divadlo, nemozes ho uniest!\n");
        return;
        }

if (user->room->group!=u->room->group) {
	sprintf(text,"Z unasania nic nebude, %s je od teba prilis %s!\n",u->name,pohl(u,"vzdialeny","vzdialena"));
	write_user(user,text);
	return;
	}

if (user->room==u->room) {
	sprintf(text,"Ved %s uz je s tebou %s!\n",u->name,user->room->where);
	write_user(user,text);
	return;
	}

if (kam->access==GOD_PRIVATE) {
      write_user(user,"Toto je BOZSKA miesnost - nik sem nesmie!\n");
      return;
      }          /* Ak som v GODPRIVATE a unesiem luzera, tak to nejde! */      
                 /* ALE mohlo by to ist ... */  
if (!(strcmp(odkial->name,"zalar"))) {
	write_user(user,"Pomahanie pri uteku z vazenia sa tresta!\n");
	return; }
if (!strcmp(odkial->name,"svatyna")) {
	write_user(user,"Uzivatel medituje v svatyni a nezela si byt ruseny!\n");
	return;
	}

if (!(strcmp(kam->name,"arena"))) {
	write_user(user,"Nemozes unasat do areny, do areny treba ist dobrovolne.\n");
	return; }
if (!(strcmp(kam->name,"zalar"))) {
      write_user(user,"Ak chces niekoho pozvat na navstevu, pouzi prikaz invite.\n");
      return;
      }
if (!(strcmp(odkial->name,"tunel"))) {
	write_user(user,"Takto z tunela nikoho nedostanes, musi si najst vychod.\n");
	return; }
if (!(strcmp(kam->name,"tunel"))) {
	write_user(user,"Takto do sem nikoho nedostanes, musi si najst vchod.\n");
	return; }
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"\n~OL~FG%s~RS~FG ta %s k sebe!~RS\n\n",name,pohl(user,"uniesol","uniesla"));
write_user(u,text);
sprintf(text,"~OL~FT%s~RS~FT %s ~OL~FT%s!~RS\n",u->name,pohl(u,"bol uneseny","bola unesena"), sklonuj(user,7));
write_room(odkial,text);
sprintf(text,"~OL~FT%s~RS~FT %s ~OL~FT%s!~RS\n",name,pohl(user,"uniesol","uniesla"), sklonuj(u, 4));
write_room(kam,text);
move_user(u,user->room,3); /* 3 bolo najlepsie */

decrease_pp(user,KIDDEC,NODYNAMIC);
}


void join(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT odkial, kam;
/* char *name; */
int i,cnt=0;

if (user->room->group==4) {
  if (user->team<0 || user->team>4) user->team=0;
  if (word_count==2) {
    for(i=1;i<5;i++) if (!strncmp(teamname[i],word[1],4)) {
      sprintf(text,"Pripojil%s si sa do teamu: %s%s\n",pohl(user,"","a"),teamcolor[i],teamname[i]);
      write_user(user,text);
      user->team=i;
      return;
     }
   }
  for(i=1;i<5;i++) if (teamname[i][0]) {
    cnt++;
    if (cnt==1) write_user(user,"Pouzi: .join <team>  Mozes sa pripojit do nasledujucich teamov:\n");
    sprintf(text," %s%s\n",teamcolor[i],teamname[i]);
    write_user(user,text);
   }
  if (cnt==0) {
    sprintf(text,"Momentalne vladne anarchia, neexistuju ziadne organizovane skupiny.\n");
    write_user(user,text);
    return;
   }
  if (user->team>0) {
    sprintf(text,"Momentalne patris do teamu %s%s~RS~FW.\n",teamcolor[user->team],teamname[user->team]);
    write_user(user,text);
   }
  return;
 }

if (word_count<2) {
      write_user(user,"Pouzi: .join <uzivatel>\n");  return;
      }
if (!(u=get_user(word[1]))) {
      write_user (user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Nemozes sa premiestnit sam k sebe!\n");
      return;
      }
      
if (user->glue) {
	sprintf(text,"Si %s tajomnou silou (zeby obycajnou zuvackou?:) k zemi!\n",pohl(user,"prilepeny","prilepena"));
	write_user(user,text);
	return;
	}

/* Toto je diskutabilne .. ked nas ignoruje, asi nexce aby sme sa k nemu
   premiestnovali ;) */
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }

odkial=user->room;
kam=u->room;
if (kam->access==GOD_PRIVATE && kam!=user->invite_room) {
      write_user(user,"Nemozes sa premiestnit do BOZSKEJ miestnosti.\n");
      return;
      }

if ((kam->access==PRIVATE || kam->access==FIXED_PRIVATE) && kam!=user->invite_room) {
	sprintf(text,"Nemozes sa takto dostat do private miestnosti, skus .letmein %s\n",kam->name);
	write_user(user,text);
	return;
	}

if (!strcmp(kam->name,"nebesa")) {
	write_user(user,"Nemozes sa takto premiestnit na nebesa.\n");
	return;
	}

if (user->room->group!=u->room->group && user->level<WIZ) {
	write_user(user,"Lutujem, na taku dialku to nepojde.\n");
	return;
	}

if (user->room==u->room) {
	sprintf(text,"Neblbni, ved %s je v tejto miestnosti!\n",u->name);
	write_user(user,text);
	return;
	}

if (!(strcmp(odkial->name,"zalar"))) {
	write_user(user,"Nie, nie; v zalari to asi nepojde!\n");
	return; }

if (!(strcmp(odkial->name,"tunel"))) {
	write_user(user,"Takto sa z tunela nedostanes, je tu tma, musis si najst cestu von.\n");
	return; }

if (!(strcmp(kam->name,"tunel"))) {
	write_user(user,"Takto sa do tunela nedostanes, musis najst vchod.\n");
	return; }

if (!(strcmp(kam->name,"zalar"))) {
	sprintf(text,"Ak chces do zalara, vazen ta musi pozvat na navstevu (.invite)\n");
	write_user(user,text);
	return; }

/* if (user->vis) name=user->name; else name=invisname(user); */
sprintf(text,"%s sem %s niekoho navstivit.\n",user->name,pohl(user,"prisiel","prisla"));
if (user->vis) write_room(kam,text);
sprintf(text,"%s odchadza niekoho navstivit %s.\n",user->name,kam->into);
if (user->vis) write_room(odkial,text);
move_user(user,u->room,3);
}

/* follow - follows a user to his/her room. ZMENA Spakky */

void follow(user, unfol) /* unfol: 0=follow, 1=unfollow */
UR_OBJECT user;
int unfol;
{
UR_OBJECT u;
int oflajn=0;

if (word_count==3 && user->level==GOD && !strcmp(word[2],"sms")) {
  if (!strcmp(user->mobile,"-")) {
      sprintf(text,"Nemas nastavene cislo mobilu! Bez toho sa agent nema ako s tebou dorozumiet!\n");
      write_user(user,text);
      return;
     }
  if ((u=get_user(word[1]))==NULL) {
    if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
      write_user(user,text);
      return;
     }
    sstrncpy(u->name,word[1],12);
    if (!load_user_details(u)) {
      destruct_user(u);
      destructed=0;
      write_user(user,"Taky luzer neexistuje.\n");
      return;
     }
    oflajn=1;
   }
  else oflajn=0;
  
  if (unfol) {
    if (!strcmp(u->smsfollow,"-")) {
      sprintf(text,"Na %s nie je nasadeny ziadny agent!\n",sklonuj(u,4));
      write_user(user,text);
      if (oflajn==1) {
        destruct_user(u);
        destructed=0;
       }
      return;
     }
    if (strcmp(u->smsfollow,user->mobile)) {
      sprintf(text,"Na %s nasadil tajneho agenta niekto iny!\n",sklonuj(u,4));
      write_user(user,text);
      if (oflajn==1) {
        destruct_user(u);
        destructed=0;
       }
      return;
     }
    else {
      sprintf(text,"Odvolal%s si agenta zo sledovania %s.\n",pohl(user,"","a"),sklonuj(u,2));
      write_user(user,text);
      strcpy(u->smsfollow,"-");
     }
   }
  else {
    if (strcmp(u->smsfollow,user->mobile) && strcmp(u->smsfollow,"-")) {
      sprintf(text,"Na %s uz nasadil tajneho agenta niekto iny!\n",sklonuj(u,4));
      write_user(user,text);
      if (oflajn==1) {
        destruct_user(u);
        destructed=0;
       }
      return;
     }
    if (!strcmp(u->smsfollow,user->mobile)) {
      sprintf(text,"Na %s si uz nasadil%s jedneho agenta!\n",sklonuj(u,4),pohl(user,"","a"));
      write_user(user,text);
      if (oflajn==1) {
        destruct_user(u);
        destructed=0;
       }
      return;
     }
    else {
      sprintf(text,"Nasadil%s si agenta na sledovanie %s.\n",pohl(user,"","a"),sklonuj(u,2));
      write_user(user,text);
      strcpy(u->smsfollow,user->mobile);
     }
   }
  if (oflajn==1) {
    u->socket=-2;
    strcpy(u->site,u->last_site);
    save_user_details(u,0);
    destruct_user(u);
    destructed=0;
   }
  return;
 }

if (unfol) {
  if (user->follow[0]) {
    sprintf(text,"%s si nasledovat: %s\n", pohl(user,"Prestal","Prestala"), user->follow);
    write_user(user,text);
    user->follow[0]='\0';
    return;
   }
  else write_user(user,"Ved nikoho nenasledujes..\n");
  return;
 }

if (word_count<2) {
      write_user(user,"Pouzi: .follow <uzivatel>\n");  return;
      }
if (!(u=get_user(word[1]))) {
      write_user (user,notloggedon);  return;
      }
if (u==user) {
      write_user(user,"Nemozes nasledovat sam seba!\n");
      return;
      }
if (check_ignore_user(user,u)) {  /* ignorovanie single usera */
      sprintf(text,"%s ta ignoruje.\n",u->name);
      write_user(user,text);
      return;
    }

if (user->room!=u->room) {
	sprintf(text,"%s nieje v tejto miestnosti!\n", u->name);
	write_user (user,text);
	return;
	}
strcpy(user->follow, u->name);
sprintf(text, "%s si nasledovat %s.\n", pohl(user,"Zacal","Zacala"), sklonuj(u, 4));
write_user(user,text);
sprintf(text, "%s ta %s prenasledovat!\n", user->name, pohl(user,"zacal","zacala"));
write_user(u, text);
}

/* JAIL - sup s nim do basy! ZMENA!!!
   + dalsia zmena - mozes jailovat/unjailovat, aj ked use nie je logged
   + casovany jail - po uplynuti urc. doby automaticky odjailuje
   */

void send_to_jail(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
UR_OBJECT u;
char *name;
int cas=0;
char *temp, meno[14], tempcas[80];

if (word_count<3) {  
      write_user(user,"Pouzi: .jail <uzivatel> [<cas_v_minutach!> | <cancel [sure]> | <dovod>]\n");  return;
      }
if (user->muzzled) {
      write_user(user,"Si umlcany a nemozes nikoho uvaznit.\n"); return;
      }
if (user->jailed) {
      write_user(user,"Sam si vo vazeni, nemozes nikoho uvaznit!\n"); return;
      }

if (user->vis) name=user->name; else name=invisname(user);

if ((u=get_user(word[1]))!=NULL) {
	
if ((word_count==3 || word_count==4) && (!strncmp(word[2],"can",3))) {
  if (u->jailed) {
    if (u->level<=user->level) {
      if ((temp=jdb_info(DB_JAIL,u->name))!=NULL) {
        strcpy(meno,jdb_wizz(DB_JAIL,u->name));
	if (strcmp(meno, user->name)) { /* nemoze odzalarovat */
	  if (user->level<GOD || strcmp(word[3],"sure")) {
	    sprintf(text,"Iba %s moze prepustit %s. Dovod zalara: ~OL%s\n", meno, sklonuj(u, 4), temp);
	    write_user(user,text);
	    return;
	   }
	 }
	if (!jdb_vyrad(DB_JAIL, u->id)) {
	  write_user(user,"Huh?!\n"); /* :-)) */
	 }
       }
      u->jailed=0;
      sprintf(text,"~FTPrepustil si %s na slobodu.\n",sklonuj(u,4));
      write_user(user,text);
      move_user(u,get_room("namestie",NULL),3);
      sprintf(text,"\n~OL~FT%s sa nad tebou %s ta z temneho zalara na slobodu!\n", name, pohl(user,"zlutoval, a prepustil","zlutovala, a prepustila"));
      write_user(u,text);
      sprintf(text,"~FT%s %s %s zo zalara!\n", name, pohl(user,"prepustil","prepustila"), sklonuj(u,4));
      write_room_except2users(NULL,text,user,u);
      sprintf(text,"%s UNJAILED %s.\n",user->name,u->name);
      write_syslog(text,2);
      return;
     }
    else { write_user(user,"Uzivatel ma vyssi level ako ty - nemozes ho prepustit zo zalara.\n"); return; }
   }
  else { write_user(user,"Tento uzivatel nie je v zalari.\n"); return; }
 }
      
if (u->level>=user->level) {
      write_user(user,"Nemozes uvaznit uzivatela s rovnakym alebo vyssim levelom ako mas ty sam.\n");
      return;
      }

cas=atoi(word[2]);
inpstr=remove_first(inpstr);
if (cas) inpstr=remove_first(inpstr);
if (cas<0) {
      write_user(user,"Ale no tak!\n");
      return;
      }
if (cas>119) {
      write_user(user,"Pozor! Cas zalara sa zadava v minutach!\nTo su (vyse) 2 hodiny, nie je to trochu privela ?\n");
      return;
      }

if (u->jailed) {
  sprintf(text,"%s uz je vo vazeni.\n",u->name);
  write_user(user,text);
  return;
 }

if (!cas) jdb_zarad(DB_JAIL,u->id,user->id,inpstr);

/*
  temp=jdb_info(JDB_JAIL, u->name);
  sscanf(temp,"%s", meno);
  temp=remove_first(temp);
  endstring(temp);
  sprintf(text,"%s uz je vo vazeni (%s: ~OL%s~RS)\n", u->name, meno, temp);
  write_user(user,text);
  return;
 }*/
	    
/***********/
  sprintf(query,"select `body` from `files` where `filename`='jail.pic';");
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result)) && row[0]) {
      write_user(u,row[0]);
     }
    mysql_free_result(result);
   }

 u->jailed=cas;
 if (cas) {
   if (cas==1) sprintf(tempcas," na ~FW%d~FR minutu",cas);
   else if (1<cas && cas<5) sprintf(tempcas," na ~FW%d~FR minuty",cas);
   else sprintf(tempcas," na ~FW%d~FR minut",cas);
   u->jailed=cas*60;
  }
 else { strcpy(tempcas,""); u->jailed=-1; }
 
 if (!cas || word_count>3) {
   sprintf(text,"~OL~FR%s %s straze, ktore ta odviedli do zalara%s!\n~OL~FRDovod: ~FW%s\n",name,pohl(user,"zavolal","zavolala"),tempcas,inpstr);
   write_user(u,text);
   sprintf(text,"~OL~FR%s si straze, ktore odviedli %s do zalara%s!\n~OL~FRDovod: ~FW%s\n",pohl(user,"Zavolal","Zavolala"),sklonuj(u,4),tempcas,inpstr);
   write_user(user,text);
   sprintf(text,"~OL~FR%s %s straze, ktore odviedli %s do zalara%s!\n~OL~FRDovod: ~FW%s\n",name,pohl(user,"zavolal","zavolala"),sklonuj(u,4),tempcas,inpstr);
   write_room_except2users(NULL,text,user,u);
   sprintf(text,"%s JAILED %s 4 %d min: %s\n",user->name,u->name,cas,inpstr);
   write_syslog(text,2);
  }
 else {
   sprintf(text,"~OL~FR%s %s straze, ktore ta odviedli do zalara%s!\n",name,pohl(user,"zavolal","zavolala"),tempcas);
   write_user(u,text);
   sprintf(text,"~OL~FR%s si straze, ktore odviedli %s do zalara%s!\n",pohl(user,"Zavolal","Zavolala"),sklonuj(u,4),tempcas);
   write_user(user,text);
   sprintf(text,"~OL~FR%s %s straze, ktore odviedli %s do zalara%s!\n",name,pohl(user,"zavolal","zavolala"),sklonuj(u,4),tempcas);
   write_room_except2users(NULL,text,user,u);
   sprintf(text,"%s JAILED %s 4 %d min.\n",user->name,u->name,cas);
   write_syslog(text,2);
  }
 move_user(u,get_room("zalar",NULL),3);
 write_user(u,"\n~OL~FRAk pochopis preco si tu, pouzi prikaz .excuse na ospravedlnenie.\n\n");
 police_freeze(user,1);
 return;
}

/* User not logged on */
/* write_user(user,"User not logged...\n"); */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozne vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v send_to_jail().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->level>=user->level) {
      write_user(user,"Nemozes uvaznit uzivatela s rovnakym alebo vyssim levelom ako mas ty sam.\n");
      destruct_user(u);
      destructed=0;
      return;
      }
if ((word_count==3 || word_count==4) && (!strncmp(word[2],"can",3))) {
	if (u->jailed) {
	 if (u->level<=user->level) {
	    if ((temp=jdb_info(DB_JAIL, u->name))!=NULL) {
                strcpy(meno,jdb_wizz(DB_JAIL,u->name));
	 	if (strcmp(meno, user->name)) { /* nemoze odzalarovat */	 			 			
	 		if (user->level<GOD || strcmp(word[3],"sure")) {
		 		sprintf(text,"Iba %s moze prepustit %s. Dovod zalara: ~OL%s\n", meno, sklonuj(u, 4), temp);
		 		write_user(user,text);
		 		destruct_user(u);
		 		destructed=0;
		 		return;
		 		}
		 	}		 	
	 	if (!jdb_vyrad(DB_JAIL, u->id)) {
			write_user(user,"Huh?!\n"); /* :-)) */
	 		}	 		
	 	}
	 u->jailed=0;
	 save_user_details(u,0);
	 sprintf(text,"~OL~FTPrepustil si %s na slobodu.\n",pohl(u,"ho","ju"));
	 write_user(user,text);
         sprintf(text,"%s UNJAILED %s.\n",user->name,u->name);
         write_syslog(text,2);
	 strcpy(meno,u->name);
	 destruct_user(u);
	 destructed=0;
	 sprintf(text,"~OL%s zo zalara!\n",pohl(u,"Bol si prepusteny","Bola si prepustena"));
	 send_mail(user,meno,text);
	 return;
	 }  else { write_user(user,"Uzivatel ma vyssi level ako ty - nemozes ho prepustit zo zalara.\n");
	 	destruct_user(u); destructed=0; return; }
	} else { write_user(user,"Tento uzivatel nie je v zalari.\n"); 
		destruct_user(u); destructed=0; return; }
      }

cas=atoi(word[2]);
inpstr=remove_first(inpstr);
if (cas) inpstr=remove_first(inpstr);
if (cas<0) {
      write_user(user,"Ale no tak!\n");
      destruct_user(u);
      destructed=0;
      return;
      }
if (cas>119) {
      write_user(user,"Pozor! Cas zalara sa zadava v minutach!\nTo su (vyse) 2 hodiny, nie je to trochu privela ?\n");
      destruct_user(u);
      destructed=0;
      return;
      }

if (u->jailed) {
  sprintf(text,"%s uz je vo vazeni.\n",u->name);
  write_user(user,text);
  destruct_user(u);
  destructed=0;
  return;
 }

if (!cas) jdb_zarad(DB_JAIL,u->id,user->id,inpstr);

 u->jailed=cas;
 if (cas) {
   if (cas==1) sprintf(tempcas," na ~FW%d~FR minutu",cas);
   else if (1<cas && cas<5) sprintf(tempcas," na ~FW%d~FR minuty",cas);
   else sprintf(tempcas," na ~FW%d~FR minut",cas);
   u->jailed=cas*60;
  }
 else { strcpy(tempcas,""); u->jailed=-1; }

 if (!cas || word_count>3) 
  sprintf(text,"%s JAILED %s 4 %d min: %s\n",user->name,u->name,cas,inpstr);
 else
  sprintf(text,"%s JAILED %s 4 %d min.\n",user->name,u->name,cas);
 write_syslog(text,2);

 strcpy(meno,u->name); 
 u->room=get_room("zalar",NULL);
 u->socket=-2;
 strcpy(u->site,u->last_site);
 save_user_details(u,0);

 if (!cas || word_count>3) {
   sprintf(text,"~OL~FR%s bol poslan%s do zalara%s!\n~OL~FRDovod: ~FW%s\n",u->name,pohl(u,"y","a"),tempcas,inpstr);
   write_user(user,text);
   sprintf(text,"~OL~FR%s do zalara%s!\n~OL~FRDovod: ~FW%s\n",pohl(u,"Bol si uvrhnuty","Bola si uvrhnuta"),tempcas,inpstr);
   send_mail(user,meno,text);
  }
 else {
   sprintf(text,"~OL~FR%s bol poslan%s do zalara%s!\n",u->name,pohl(u,"y","a"),tempcas);
   write_user(user,text);
   sprintf(text,"~OL~FR%s do zalara%s!\n",pohl(u,"Bol si uvrhnuty","Bola si uvrhnuta"),tempcas);
   send_mail(user,meno,text);
  }
 destruct_user(u);
 destructed=0;
 police_freeze(user,1);
}
/* koniec send_to_jail() */


/*** Shout something to other wizes and gods. If the level isnt given it
      defaults to WIZ level. ***/
void wizshout(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int lev;
/* char komu[30]; */

if (user->muzzled) {
      write_user(user,"Si umlcany, nemozes wizshoutovat.\n");  return;
      }
if (word_count<2) {
      write_user(user,"Pouzi: .wizshout [<superuser level>] <sprava>\n");
      return;
      }
strtoupper(word[1]);
lev=(get_level(word[1]));
if (lev==-1) { /* if (strstr(word[1],"WIZ")) lev=WIZ; */
               if (strstr(word[1],"KIN")) lev=KIN;
               if (strstr(word[1],"GOD")) lev=GOD;
             }  
if (lev==-1) lev=KIN;
else {
      if (lev<WIZ || word_count<3) {
            write_user(user,"Pouzi: .wizshout [<superuser level>] <sprava>\n");
            return;
            }
      if (lev>user->level) {
            write_user(user,"Nemozes wizkricat vyssim levelom ako mas sam.\n");
            return;
            }
/*      if (lev==WIZ) strcpy(komu,"wizardom"); */
/*      if (lev==KIN) strcpy(komu,"kingom"); */
/*      if (lev==GOD) strcpy(komu,"godom"); */
      inpstr=remove_first(inpstr);
      if (lev==KIN)
       sprintf(text,"~OL\253WK:~RS %s\n",inpstr);
      else
       sprintf(text,"~OL\253WG:~RS %s\n",inpstr);
      write_user(user,text);
      if (lev==KIN)
       sprintf(text,"~OL%s \253Wk:~RS %s\n",user->name,inpstr);
      else
       sprintf(text,"~OL%s \253Wg:~RS %s\n",user->name,inpstr);
      write_level(lev,1,text,user); 
      record_wizshout(text); 
      user->commused[10]++;
      return; 
      }
sprintf(text,"~OL\253WK:~RS %s\n",inpstr); 
write_user(user,text); 
sprintf(text,"~OL%s \253Wk:~RS %s\n",user->name,inpstr); 
write_level(WIZ,1,text,user); 
record_wizshout(text); 
user->commused[10]++;
} 
 
/*** S.O.S. - zoophaly user prosi o pomoc... ZMENA - pridane */ 
void send_sos(user, inpstr) 
UR_OBJECT user; 
char *inpstr;
{ 
UR_OBJECT u;
/* FILE *fp; */ 

 if (user->level<KIN) {
   if (word_count<2) { 
     write_user(user,"Pouzi: .sos <sprava>\n~OLNEZNEUZIVAJ TENTO PRIKAZ!\n");
     return; 
    } 
   if (user->muzzled) { 
     write_user(user,"Si umlcany - nemozes vysielat SOS.\n");
     return; 
    } 
   sprintf(text,"~OL~FRPOZOR: %s ti posiela SOS:~RS~FW %s\n",user->name, inpstr); 
   write_level(WIZ,1,text,NULL); 
   sprintf(text,"~OL~FR%s SOSuje:~RS~FW %s\n",user->name, inpstr); 
   record_sos(text);
   write_user(user,"~OLVyslal si SOS signal vsetkym wizardom.\n"); 
   return;
  }
 
 if (word_count<3) { 
   write_user(user,"Pouzi: .sos [<user> | NULL] <sprava>\n");
   return; 
  } 
 if (user->muzzled) { 
   write_user(user,"Si umlcany - nemozes vysielat SOS.\n");
   return; 
  }
 if (!strcasecmp(word[1],"null")) {
   inpstr=remove_first(inpstr);
   sprintf(text,"~OL~FR%s SOSuje:~RS~FW %s\n",user->name, inpstr); 
   write_level(WIZ,1,text,user); 
   record_sos(text);
   sprintf(text,"~OL~FRSOSujes:~RS~FW %s\n",inpstr); 
   write_user(user,text); 
  }
 else {
   if ((u=get_user(word[1]))!=NULL) { 
     inpstr=remove_first(inpstr);
     sprintf(text,"~OL~FR%s SOSuje %s:~RS~FW %s\n",user->name,sklonuj(u,3),inpstr);
     write_level(WIZ,1,text,user); 
     record_sos(text);
     sprintf(text,"~OL~FRSOSujes %s:~RS~FW %s\n",sklonuj(u,3),inpstr); 
     write_user(user,text);
     sprintf(text,"~OL~FR%s ti SOSuje:~RS~FW %s\n",user->name,inpstr); 
     write_user(u,text);
    }
   else write_user(user,notloggedon);
  }
} 


/* ZMENA - Prikaz NOTICE: Complaint(1), suggestion(2), excuse(3)!!!  */ 
void write_noticeboard(user, what, done_editing)
UR_OBJECT user;
int what;
int done_editing;
{
FILE *fp;
FILE *fp2;
char *ptr;
int cnt;
static int copak;
char typ;
char pomocna[50];
time_t akt_cas;

if (what<1 || what>3) return;

if (user->muzzled && what!=3) {
      write_user(user,"Si umlcany - nemozes pouzivat tento prikaz.\n");  return;
      }
      
if (!done_editing) {
      	if (what==1) write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie STAZNOSTI ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      	else if (what==2) write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie NAVRHU ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      		else if (what==3) write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Pisanie OSPRAVEDLNENIA ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
        user->misc_op=16;
        editor(user,NULL);
        copak=what;
        return; 
        }        
else {
	ptr=user->malloc_start; 
      }
    
/* vypishe */
if (copak==1) sprintf(text,"~OL~FRStaznost od %s:\n", sklonuj(user,2));
    else if (copak==2) sprintf(text,"~OL~FRNavrh od %s:\n", sklonuj(user,2));
      	else if (copak==3) sprintf(text,"~OL~FROspravedlnenie od %s:\n", sklonuj(user,2));
      	
if (copak==2) {
	write_level(GOD, 1, text, NULL);
	write_level(GOD, 1, ptr, NULL);
	}
	else {
	write_level(WIZ, 1, text, NULL);
	write_level(WIZ, 1, ptr, NULL);
	}


if (!(fp=ropen(NOTICEBOARD,"a"))) return; /*APPROVED*/

/* je tam, or not */
if ((fp2=ropen(NOTICE_DIGEST,"r"))==NULL) {
	if ((fp2=ropen(NOTICE_DIGEST,"w"))==NULL) { fclose (fp); return; }
	time(&akt_cas);
	fprintf(fp2,"From: Atlantis Talker <%s@%s>\n",TALKER_EMAIL_ALIAS,TALKER_EMAIL_HOST);
	fprintf(fp2,"To: %s@%s\n",WIZZES_EMAIL_ALIAS,TALKER_EMAIL_HOST);
	sprintf(text,"Subject: Notice digest (%s)\n\n", zobraz_datum(&akt_cas,4));
	fprintf(fp2,"%s",text);
	fclose(fp2);
	}
else fclose(fp2);

if (!(fp2=ropen(NOTICE_DIGEST,"a"))) return;

if (copak==1) typ='C';
else if (copak==2) typ='S';
	else if (copak==3) typ='E';
		else typ='-';
		
sprintf(pomocna,"%c %02d/%02d/%02d - %s", typ, tmday, tmonth+1, tyear, user->name);

sprintf(text,"~OL~FR[====== ~FW%s ~FR=========================", pomocna);
text[46]='\0';
/* for (cnt=0; cnt<(25-strlen(pomocna)); cnt++) strcat(text,"="); */
strcat(text,"============================================]\n");

fprintf(fp, "%s", text);
colour_com_strip(text);
fprintf(fp2,"%s", text);

cnt=0;
while(*ptr!='\0') {
      putc(*ptr,fp);
      putc(*ptr,fp2);
      if (*ptr=='\n') cnt=0; else ++cnt;
      if (cnt==80) { cnt=0; }
      ++ptr;
     }
     
fclose(fp);
fclose(fp2);

if (copak==1) write_user(user,"Tvoja staznost bola zaregistrovana.\n");
else if (copak==2) write_user(user,"Tvoj navrh bol zaregistrovany.\n");
	else if (copak==3) write_user(user,"Tvoje ospravedlnenie bolo zaregistrovane.\n");
}


/*** Muzzle (zapcha usta) an annoying user so he cant speak, emote, echo,
  write, smail or bcast. Muzzles have levels from WIZ to GOD so for instance
  a wiz cannot remove a muzzle set by a god.  
  Zmena S. - pokus s akciami ***/  
 
void muzzle(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cas;
char meno[14];

if (word_count<2) {
      write_user(user,"Pouzi: .muzzle <uzivatel> [minuty]\n");  return; 
      } 
if ((u=get_user(word[1]))!=NULL) { 
      if (u==user) { 
	    write_user(user,"Snaha umlcat sameho seba je deviaty priznak sialenstva.\n"); 
	    return; 
	    } 
      if (u->level>=user->level) { 
	    write_user(user,"Nemozes umlcat rovnaky alebo vyssi level ako mas sam.\n"); 
	    return; 
	    } 
      if (u->muzzled>=user->level) { 
	    sprintf(text,"%s uz je %s.\n",u->name, pohl(u,"umlcany","umlcana")); 
	    write_user(user,text);  return; 
	    } 
      cas=atoi(word[2]);

      if (cas>0) { 
      	    sprintf(text,"~FR~OL%s na %d minut!\n",pohl(u,"Bol si umlcany","Bola si umlcana"), cas); 
      	    write_user(u,text); 
      	    sprintf(text,"~FR~OL%s ma odteraz nahubok levelu: ~RS~OL%s ~FRna ~RS~OL%d ~FRminut.\n",u->name, level_name[user->level],cas);
      	    write_user(user,text); 
      	    u->muzzletime=cas*60; 
	    u->muzzled=user->level;
      	    return;      	 
      	    }    
      sprintf(text,"~FR~OL%s ma odteraz nahubok levelu: ~RS~OL%s.\n",u->name,level_name[user->level]);
      write_user(user,text);
      sprintf(text,"~FR~OL%s!\n",pohl(u,"Bol si umlcany","Bola si umlcana"));
      write_user(u,text);
      u->muzzled=user->level;
      u->muzzletime=cas*60;
      sprintf(text,"%s MUZZLED %s.\n",user->name,u->name);
      write_syslog(text,2);
      police_freeze(user,1);
      return;
      }
/* User not logged on */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v muzzle().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->level>=user->level) {
      write_user(user,"Nemozes umlcat uzivatela s rovnakym/vyssim levelom ako mas sam.\n");
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->muzzled>=user->level) {
      sprintf(text,"%s uz ma nahubok.\n",u->name);
      write_user(user,text);
      destruct_user(u);
      destructed=0;
      return;
      }
u->socket=-2;
u->muzzled=user->level;
strcpy(u->site,u->last_site);
save_user_details(u,0);
sprintf(text,"~FR~OL%s ma odteraz nahubok levelu: ~RS~OL%s.\n",u->name,level_name[user->level]);
write_user(user,text);
sprintf(text,"~FR~OL%s!~RS~FW\n",pohl(u,"Bol si umlcany","Bola si umlcana"));
strcpy(meno,u->name);
destruct_user(u);
destructed=0;
send_mail(user,meno, text);
sprintf(text,"%s MUZZLED %s.\n",user->name,meno);
write_syslog(text,2);
police_freeze(user,1);
}

/*** Umuzzle the bastard now he's apologised 
     an grovelled (ponizil sa) enough via email ***/
     
void unmuzzle(user)
UR_OBJECT user;
{
UR_OBJECT u;
char meno[14];

if (word_count<2) {
      write_user(user,"Pouzi: .unmuzzle <uzivatel>\n");  return;
      }
if ((u=get_user(word[1]))!=NULL) {
      if (u==user) {
            write_user(user,"Snaha odstranit umlcanie seba sameho je desiaty priznak sialenstva.\n");
            return;
            }
      if (!u->muzzled) {
            sprintf(text,"%s nema nahubok.\n",u->name);
            write_user(user,text);
            return;
            }
      if (u->muzzled>user->level) {
            sprintf(text,"%s ma nahubok levelu %s, nemas dost sily na jeho odstranenie.\n",u->name,level_name[u->muzzled]);
            write_user(user,text);  return;
            }
      sprintf(text,"~FG~OLOdstranil si nahubok uzivatela %s.\n",u->name);
      write_user(user,text);
      sprintf(text,"~FG~OL%s uzivatelom %s! Dakuj %s laskavost!\n",pohl(u,"Bol si odmlcany","Bola si odmlcana"), user->name, pohl(user,"mu za jeho","jej za jej"));
      write_user(u,text);
      sprintf(text,"%s UNMUZZLED %s.\n",user->name,u->name);
      write_syslog(text,2);
      u->muzzled=0;
      u->muzzletime=0;
      return;
      }
/* User not logged on */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v unmuzzle().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if (!u->muzzled) {
        sprintf(text,"%s nema nahubok.\n",u->name);
        write_user(user,text);
        destruct_user(u);
        destructed=0;
        return;	
	}     
if (u->muzzled>user->level) {
      sprintf(text,"%s ma nahubok levelu %s, nemas dost sily na jeho odstranenie.\n",u->name,level_name[u->muzzled]);
      write_user(user,text);
      destruct_user(u);
      destructed=0;
      return;
      }
u->socket=-2;
u->muzzled=0;
u->muzzletime=0;
strcpy(u->site,u->last_site);
save_user_details(u,0);
sprintf(text,"~FG~OLOdmlcal si uzivatela %s.\n",u->name);
write_user(user,text);
sprintf(text,"~FG~OL%s.\n", pohl(u,"Bol si odmlcany","Bola si odmlcana"));
strcpy(meno,u->name);
destruct_user(u);
destructed=0;
send_mail(user,meno,text);
sprintf(text,"%s UNMUZZLED %s.\n",user->name,meno);
write_syslog(text,2);
}

void minlogin(user)
UR_OBJECT user;
{
UR_OBJECT u,next;
char *usage="Pouzi: .minlogin NONE/<user level>\n";
char levstr[10],*name;
int lev,cnt;

if (word_count<2) {
  if (minlogin_level==-1) strcpy(levstr,"NONE");
  else strcpy(levstr,level_name[minlogin_level]);
  write_user(user,usage);
  sprintf(text,"Minlogin je prave nastaveny na: ~OL%s.\n",levstr);
  write_user(user,text);
  return;
 }
strtoupper(word[1]);
if ((lev=get_level(word[1]))==-1) {
      if (strcmp(word[1],"NONE")) {
            write_user(user,usage);  return;
            }
      lev=-1;
      strcpy(levstr,"NONE");
      }
else strcpy(levstr,level_name[lev]);
if (lev>user->level) {
      write_user(user,"Nemozes nastavit minlogin na vyssi level ako mas sam.\n");
      return;
      }
if (minlogin_level==lev) {
      write_user(user,"Uz je nastaveny na tuto hodnotu.\n");  return;
      }
minlogin_level=lev;
sprintf(text,"Minlogin level nastaveny na: ~OL%s.\n",levstr);
write_user(user,text);
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"%s nastavil minlogin level na: ~OL%s.\n",name,levstr);
write_level(KIN,1,text,user);
sprintf(text,"%s set the minlogin level to %s.\n",user->name,levstr);
write_syslog(text,1);

/* Now boot off anyone below that level */
cnt=0;
u=user_first;
while(u) {
      next=u->next;
      if (!u->login && u->type!=CLONE_TYPE && u->level<lev) {
            write_user(u,"\n~OL~FYZ technickych pricin bol docasne nastaveny minimalny level.\n");
	    disconnect_user(u,1,NULL);
	    inyjefuc=1;
            ++cnt;
            }
      u=next;
      }
sprintf(text,"Celkove bolo odpojenych %d uzivatelov.\n",cnt);
destructed=0; /*MORLOR ??? */
write_user(user,text);
}

/*** Show talker system parameters etc ***/
void system_details(user, typ)
UR_OBJECT user;
int typ;
{
RM_OBJECT rm;
UR_OBJECT u;
char bstr[80],minlogin[10];
char *ca[]={ "NONE  ","IGNORE","REBOOT" };
int days,hours,mins,secs;
int rms,num_clones,mem,size,memo,memr;

if (user->level==GOD && !strcmp(word[1],"backup")) {
  if (backuptime>0) {
    write_user(user,"Ukludni sa, zaloha bola urobena pred par minutami.\n");
    return;
   }
  if (!strcmp(word[2],"full")) {
    write_user(user,"Dumping database..\n");
    do_db_backup(1);
   }
  else {
    write_user(user,"Dumping usertable..\n");
    do_db_backup(0);
   }
  backuptime=10;
  return;
 }
 
if (typ) {
	sprintf(text,"\n*** NUTS ver. %s (Atlantis version: %s)  ***\n\n",VERSION,ATLANTIS);
	write_user(user,text);
	}
	
/* Get some values */
strcpy(bstr,zobraz_datum((time_t *)&boot_time, 1));
secs=(int)(time(0)-boot_time);
days=secs/86400;
hours=(secs%86400)/3600;
mins=(secs%3600)/60;
secs=secs%60;
num_clones=0;
mem=0;

size=sizeof(struct user_struct);
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE) num_clones++;
      mem+=size;
      }

memr=get_resmem_usage();
memo=get_memory_usage();

if (!typ) {
  
  sprintf(text,"Uptime ~OL%d~RSd ~OL%d~RSh, ~OL%d~RS ludi, ~OL%d~RS rekord, nachamranych ~OL%'d~RS bajtov.\nRozoznanych ~OL%d~RS adries, Hits ~OL%d~RS, Pos ~OL%d~RS, Timeouts ~OL%lu~RS + ~OL%lu~RS.\nVyuzita fyzicka pamat: ~OL%'d~RS bajtov.\n",days,hours,num_of_users,max_users_was,memo,resc_resolved,resc_cached, rescn,tajmauts,tajmautz,memr);
  write_user(user,text);
  return;
 }

rms=0;
size=sizeof(struct room_struct);
for(rm=room_first;rm!=NULL;rm=rm->next) {
      ++rms;  mem+=size;
      }

if (minlogin_level==-1) strcpy(minlogin,"NONE");
else strcpy(minlogin,level_name[minlogin_level]);

/* Show header parameters */
sprintf(text,"~FTID Procesu   : ~FG%d\n~FTSpusteny dna : ~FG%s\n~FTProces bezi  : ~FG%d dni, %d hodin, %d minut a %d sekund\n~FTVyuzita RAM  : ~FG%'d bajtov\n~FTPorty (M/W)  : ~FG%d, %d\n~FTCesta pre docasne subory: ~FG%s\n\n",getpid(),bstr,days,hours,mins,secs,memr,port[0],port[1],TMPFOLDER);
write_user(user,text);

/* Show others */
sprintf(text,"Max uzivatelov         : %-5d        Momentalny pocet uziv. : %d\n",max_users,num_of_users);
write_user(user,text);
sprintf(text,"Max write timeouts     : %4d/%-4d    Momentalny pocet klon. : %d\n",max_timeouts,max_timeoutz,num_clones);
write_user(user,text);
sprintf(text,"Min. pristupovy level  : %-4s         Max. login necinost    : %d secs.\n",minlogin,login_idle_time);
write_user(user,text);
sprintf(text,"Max. uzivat. necinnost : %-4d secs.   Volanie heartbeatu     : %d\n",user_idle_time,heartbeat);
write_user(user,text);
sprintf(text,"Min. uroven pre wizport: %-7s      Bezbarierovy level     : %s\n",level_name[wizport_level],level_name[gatecrash_level]);
write_user(user,text);
sprintf(text,"Max. uzivat. necinnost : %-7s      Min. ludi pre sukromie : %d\n",level_name[time_out_maxlevel],min_private_users);
write_user(user,text);
sprintf(text,"Zivotnost sprav        : %-2d dni       Cas kontroly sprav     : %02d:%02d\n",mesg_life,mesg_check_hour,mesg_check_min);
write_user(user,text);
sprintf(text,"Sietova necinnost      : %-4d sek.    Pocet miestnosti       : %d\n",net_idle_time,rms);
write_user(user,text);
sprintf(text,"Zobrazenie hesiel      : %s          Blokovanie nadavok     : %s\n",noyes(password_echo),noyes(ban_swearing));
write_user(user,text);
sprintf(text,"Max. AFK necinnost     : %s          Velke pismena v menach : %s\n",noyes(time_out_afks),noyes(allow_caps_in_name));
write_user(user,text);
sprintf(text,"Standardny typ promptu : %s          Stand. nastav. farieb  : %s\n",offon[prompt_def],offon[colour_def]);
write_user(user,text);
sprintf(text,"Standardne charecho    : %s          Systemovy log          : %s\n",offon[charecho_def],offon[system_logging]);
write_user(user,text);
sprintf(text,"Vynimocny stav sposobi : %s       Objekty spolu alokuju  : %d\n",ca[crash_action],mem);
write_user(user,text);
sprintf(text,"Velkost uziv. strukt.  : %-6lu       Nachamrana pamat: ~OL%'d~RS bajtov\n", sizeof(struct user_struct),memo);

write_user(user,text);
}

/*** Set the character mode echo on or off. This is only for users logging in
     via a character mode client, those using a line mode client (eg unix
     telnet) will see no effect. ***/
void toggle_charecho(UR_OBJECT user,char *inpstr)
{
if (!strcmp(word[1],"text:")) {
  inpstr=remove_first(inpstr);
  if (inpstr[0]=='.') {
    sprintf(text,"%s\n",inpstr);
    write_user(user,text);
   }
  return;
 }
if (!user->charmode_echo) {
      write_user(user,"Odozva z klientov so znakovym modom ~FGZAPNUTA.\n");
      user->charmode_echo=1;
      }
else {
      write_user(user,"Odozva z klientov so znakovym modom ~FRVYPNUTA.\n");
      user->charmode_echo=0;
      }
if (user->room==NULL) prompt(user);
}


/*** Free a hung socket ***/
void clearline(user)
UR_OBJECT user;
{
UR_OBJECT u;
int sock;

if (word_count<2 || !is_number(word[1])) {
      write_user(user,"Pouzi: .clearline <linka>\n");  return;
      }
sock=atoi(word[1]);

/* Find line amongst users */
for(u=user_first;u!=NULL;u=u->next)
      if (u->type!=CLONE_TYPE && u->socket==sock) goto FOUND;
write_user(user,"Tento socket nieje prave aktivny.\n");
return;

FOUND:
if (!u->login) {
      write_user(user,"Nemozes zrusit socket prave prihlaseneho uzivatela.\n");
      return;
      }
disconnect_user(u, 3,NULL);
inyjefuc=1;
sprintf(text,"%s vymazal(a) linku %d.\n",user->name,sock);
write_syslog(text,1);
sprintf(text,"Socket %d zruseny.\n",sock);
write_user(user,text);
/*destructed=0;*/ /*MORLOR ??*/
no_prompt=0;
}


/*** Change whether a rooms access is fixed or not ***/
void change_room_fix(user,fix)
UR_OBJECT user;
int fix;
{
RM_OBJECT rm;
char *name;

if (word_count<2) rm=user->room;
else {
      if ((rm=get_room(word[1],user))==NULL) {
            write_user(user,nosuchroom);  return;
            }
      }
if (rm->access==GOD_PRIVATE) {
            write_user(user,"Nic nezmenis na tomto nastaveni.\n");
            return;
            }
if (user->vis) name=user->name; else name=invisname(user);
if (fix) {
      if (rm->access & FIXED) {
            if (rm==user->room)
                  write_user(user,"Pristup do miestnosti uz je fixovany.\n");
            else write_user(user,"Pristup do tej miestnosti uz je fixovany.\n");
            return;
            }
      sprintf(text,"Pristup do miestnosti %s je odteraz ~FRFIXOVANY.\n",rm->name);
      write_user(user,text);
      if (user->room==rm) {
            sprintf(text,"%s ~FR%s~RS pristup do tejto miestnosti.\n",name, pohl(user,"ZAFIXOVAL","ZAFIXOVALA"));
            write_room_except(rm,text,user);
            }
      else {
            sprintf(text,"Pristup do tejto miestnosti bol ~FRFIXOVANY.\n");
            write_room(rm,text);
            }
      sprintf(text,"%s ZAFIXOVAL pristup do miestnosti %s.\n",user->name,rm->name);
      write_syslog(text,1);
      rm->access+=2;
      return;
      }
if (!(rm->access & FIXED)) {
      if (rm==user->room)
            write_user(user,"Pristup do tejto miestnosti este nebol fixovany.\n");
      else write_user(user,"Pristup do tej miestnosti este nebol fixovany.\n");
      return;
      }
sprintf(text,"Pristup do miestnosti %s je ~FGODFIXOVANY.\n",rm->name);
write_user(user,text);
if (user->room==rm) {
      sprintf(text,"%s ~FG%s~RS pristup do tejto miestnosti.\n",name,pohl(user,"ODFIXOVAL","ODFIXOVALA"));
      write_room_except(rm,text,user);
      }
else {
      sprintf(text,"Pristup do tejto miestnosti bol ~FGODFIXOVANY.\n");
      write_room(rm,text);
      }
sprintf(text,"%s ODFIXOL pristup do miestnosti %s.\n",user->name,rm->name);
write_syslog(text,1);
rm->access-=2;
reset_access(rm);
}

/*** View the system log ***/
void viewlog(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
FILE *fp, *fp2;
char *emp="\nSystemovy log je prazdny.\n";
int lines;
char line[200], fn[50];

if (word_count==1) {
      write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Systemovy dennik ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      switch(more(user,user->socket,SYSLOG)) {
            case 0: write_user(user,emp);  return;
            case 1: user->misc_op=2;
            }
      return;
      }

else {
      if (!(fp=ropen(SYSLOG,"r"))) { /*APPROVED*/
      	write_user(user,emp);
      	return;
      	}
      sprintf(fn,"log/%s.SYSLTMP", user->name);
      if (!(fp2=ropen(fn,"w"))) { /*APPROVED*/
      	write_user(user,"Neda sa otvorit docasny subor vo viewlog().\n");
      	fclose(fp);
      	return;
      	}
      write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Systemovy dennik - vyhladavanie ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");     
      lines=0;
      fgets(line,199,fp);
      while(!feof(fp)) {
      	if (strstr(line, inpstr)) {
      		lines=1;
      		fprintf(fp2,"%s", line);
      		}
      	fgets(line,199,fp);
      	}
      fclose(fp);
      fclose(fp2);
      if (!lines) {
      	sprintf(text,"Vyskyt retazca \"~OL%s~RS\" nebol najdeny.\n", inpstr);
      	write_user(user,text);      
      	}
      else {
      switch(more(user,user->socket,fn)) {
            case 0: write_user(user,emp);  return;
            case 1: user->misc_op=2;
            }      
      	}
      return;
      }
      
}


/*** A newbie is requesting an account. Get his email address off him so we
     can validate who he is before we promote him and let him loose as a
     proper user.
     account_request, ZMENA Spakky - Novy sposob requestovania, pozri tiez
     misc_op 9 a 10, proceduru posielanie(), polozku user->email a script
     posli.
***/
void account_request(user)
UR_OBJECT user;
{
UR_OBJECT u;

char zadal[105];

if (user->level>NEW) {
      write_user(user,"Tento prikaz sluzi iba pre novych uzivatelov, mozno ho pouzit iba raz.\n");
      return;
      }      
if ((word_count<2) && ((user->accreq) || (strcmp(user->email,"Nema ziadny email")))) {
	write_user(user,"~OL~FRUz si podal(a) ziadost o povysenie.~RS Mail s heslom bol odoslany na Tvoju adresu:\n");
	sprintf(text,"<~OL%s~RS>. Ak uz mas heslo, napis ~OL.request <heslo>~RS.\nV pripade problemov sa obrat na ", user->email);
	write_user(user,text);
	sprintf(text,"spravcov (GODov) na adresu ~OL%s@%s~RS,\nalebo kontaktuj GODa priamo na ",TALKER_EMAIL_ALIAS,TALKER_EMAIL_HOST);
	write_user(user,text);
	write_user(user,"talkeri.\n");
	return;
	}
			
if (word_count<2) {
	write_user(user,"~OL~FRDakujeme, ze si sa rozhodol(la) stat sa obcanom Atlantisu. ~RS~FWAby si mohol(la) byt\n");
	write_user(user,"povyseny(a), potrebujeme vediet tvoju presnu E-mail adresu, na ktoru ti obratom\n");
	write_user(user,"posleme request-heslo. Potom uz staci zadat prikaz ~OL.request <heslo>~RS a ak je\n");
	write_user(user,"heslo spravne, budes automaticky povyseny(a). Pozor, ak sa pomylis v zadavani\n");
	write_user(user,"adresy, heslo sa nedoruci! Preto bud opatrny(a).\nPriklad E-mail adresy: peter@domena.sk\n\nTvoja E-mail adresa: ");
	user->has_ignall=user->ignall;
	user->ignall=1; /*Nexceme aby nas niegdo obtazoval ;)) */
	user->misc_op=9;
	no_prompt=1;
	sprintf(text,"~OL~FM%s~RS~FG vstupuje do requestu ...\n",user->name);
	wrtype=WR_NEWBIE;
	writesys(KIN, 1, text, NULL);
	wrtype=0;
	return;
	}	
      
/* Luser zadar Promote-Password. Ak zadal spravne heslo, promotne ho to */
strcpy(zadal,word[1]);
strtolower(zadal);

if (user->request[0]) {
  if (!strcmp(user->request,zadal)) {
    sprintf(text,"~OL~FYSPRAVNE HESLO! ~FMGratulujeme, prave si sa %s PLNOPRAVNYM obcanom Atlantisu.\n",pohl(user,"stal","stala"));
    write_user(user,text);
    u=user;
    u->level++;
    sprintf(text,"~FG~OL%s na level: ~RS~OL%s!\n",pohl(u,"Bol si povyseny","Bola si povysena"), level_name[u->level]);
    write_user(u,text);
    sprintf(text,"~OL~FG%s~FR %s na level ~FY%s!\n",user->name,pohl(u,"bol povyseny","bola povysena"),level_name[u->level]);
    write_room_except(user->room,text,user);
    sprintf(text,"%s %s na level %s.\n",u->name,pohl(u,"bol AUTOMATICKY povyseny","bola AUTOMATICKY povysena"), level_name[u->level]);
    writesys(KIN,1,text,NULL);
    write_syslog(text,1);
    save_user_details(user,1);
   }
  else {
    sprintf(text,"~OL~FRNespravne request-heslo!~RS~FG Heslo sa dozvies z emailu ktory sme ti poslali\n~FGpri podavani requestu. Ak ti mail neprisiel alebo mas iny problem, kontaktuj\n~FGspravcov na adrese '~OL~FW%s@%s~RS~FG'.\n",TALKER_EMAIL_ALIAS,TALKER_EMAIL_HOST);
    write_user(user,text);
    return;
   }			
 }
else {
  write_user(user,"~OL~FREste nemas podany request! Zadaj prikaz ~OL~FW.request~FR bez parametru.\n");
  return;
 }
}

void otravuj_usera_s_mailinglistom(user)
UR_OBJECT user;
{
write_user(user,"\nEste by sme ti chceli ponuknut moznost prihlasit sa do mailinglistu Atlantisu.\nV mailingliste (e-mailova konferencia) sa ohlasuju novinky a zmeny v Atlantide,\n");
write_user(user,"diskutuje sa o nich, a priestor sa nechava aj rieseniu aktualnych problemov\nsuvisiacich s talkerom Atlantis. Blizsie informacie o konferencii (ako sa\n");
write_user(user,"prihlasit/odhlasit, zakladne info) si mozes zobrazit prikazom ~OL\".pomoc mlist\"~RS~FW\n");
write_user(user,"\nChces sa teraz automaticky prihlasit do mailinglistu? (odporucame) (A/n) ");
user->has_ignall=user->ignall;
user->ignall=1;
no_prompt=1;
user->misc_op=15;
return;
}

/*** Posielanie ML requestu ... ***/
void send_mailinglist_request(user)
UR_OBJECT user;
{
FILE *fp;
char filename[80];

sprintf(filename,"%s/%s.MLIST",MAILSPOOL,user->name);
if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
  write_syslog("Nemozem otvorit subor pre odoslanie posty vo send_mailinglist_request()\n",0);
  return;
  }
    
fprintf(fp,"From: %s <%s>\n",user->name, user->email);
fprintf(fp,"To: majordomo@rak.isternet.sk\n");
fprintf(fp,"X-mailer: Atlantis Talker ver. %s\n",ATLANTIS);
fprintf(fp,"Subject: prihlasenie\n");
fprintf(fp,"\n");
fprintf(fp,"subscribe atlantis\n");
fclose(fp);
send_forward_email("majordomo@rak.isternet.sk",filename);

write_user(user,"\nZiadost o zaradenie do mailinglistu bola odoslana. O chvilu ti pride e-mail\ns uvodnymi informaciami o mailingliste. Svoje prispevky do diskusie posielaj na\nadresu ~OLatlantis@rak.isternet.sk~RS~FW.\n\n");
return;
}

/*** Clear the review buffer ***/
void revclr(user)
UR_OBJECT user;
{
char *name;

clear_revbuff(user->room);
write_user(user,"Buffer poslednych hlasok bol zmazany.\n");
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"%s %s spatny prehlad textu.\n",name, pohl(user,"zmazal","zmazala"));
write_room_except(user->room,text,user);
}


/*** Clone a user in another room ***/
void create_clone(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;
int cnt;

/* Check room */
if (word_count<2) rm=user->room;
else {
      if ((rm=get_room(word[1],user))==NULL) {
            write_user(user,nosuchroom);  return;
            }
      }
/* If room is private then nocando */
if (!has_room_access(user,rm)) {
      write_user(user,"Ta miestnost je PRIVATNA, nemozes v nej vytvorit klona.\n");
      return;
      }
/* Count clones and see if user already has a copy there , no point having
   2 in the same room */
cnt=0;
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE && u->owner==user) {
            if (u->room==rm) {
                  sprintf(text,"Uz mas klona v miestnosti %s.\n",rm->name);
                  write_user(user,text);
                  return;
                  }
            if (++cnt==max_clones) {
                  write_user(user,"Uz mas vytvoreny maximalny pocet klonov.\n");
                  return;
                  }
            }
      }
/* Create clone */
if ((u=create_user())==NULL) {
      sprintf(text,"%s: Nemozno vytvorit kopiu.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: nemozno vytvorit kopiu uzivatela v clone().\n",0);
      return;
      }
u->type=CLONE_TYPE;
u->socket=user->socket;
u->room=rm;
u->owner=user;
strcpy(u->name,user->name);
strcpy(u->desc,"(CLONE)");

if (rm==user->room)
      write_user(user,"~FB~OLSepkas zvlastne kuzlo a nahle sa objavuje tvoj dvojnik - klon teba sameho!\n");
else {
      sprintf(text,"~FB~OLSepkas zvlastne kuzlo a nahle sa v %s objavuje tvoj dvojnik - klon teba sameho!\n",rm->name);
      write_user(user,text);
      }
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FB~OL%s sepka zvlastne prastare kuzlo...\n",name);
write_room_except(user->room,text,user);
sprintf(text,"~FB~OLPocujes zvlastny sykavy zvuk a citis chvenie vzduchu...\n%s si %s klona v tejto miestnosti!\n",user->name,pohl(user,"vytvoril","vytvorila"));
write_room_except(rm,text,user);
}


/*** Destroy user clone ***/
void destroy_clone(user)
UR_OBJECT user;
{
UR_OBJECT u,u2;
RM_OBJECT rm;
char *name;

/* Check room and user */
if (word_count<2) rm=user->room;
else {
      if ((rm=get_room(word[1],user))==NULL) {
            write_user(user,nosuchroom);  return;
            }
      }
if (word_count>2) {
      if ((u2=get_user(word[2]))==NULL) {
            write_user(user,notloggedon);  return;
            }
      if (u2->level>=user->level) {
            write_user(user,"Nemozes zrusit klona rovnakeho alebo vyssieho levelu ako mas sam.\n");
            return;
            }
      }
else u2=user;
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE && u->room==rm && u->owner==u2) {
            destruct_user(u);
            reset_access(rm);
            sprintf(text,"~FM~OL%s si ostre kuzlo a tvoj klon bol zniceny.\n",pohl(user,"Zasepkal","Zasepkala"));
            write_user(user,text);
            if (user->vis) name=user->name; else name=invisname(user);
            sprintf(text,"~FM~OL%s sepka ostre kuzlo...\n",name);
            write_room_except(user->room,text,user);
            sprintf(text,"~FM~OL%s %s svojho klona ostrym kuzlom.\nKlon sa zacne trblietavo jagat a chviet, pomaly sa rozplynie a zanikne.\n",u2->name,pohl(u2,"zasiahol","zasiahla"));
            write_room(rm,text);
            if (u2!=user) {
                  sprintf(text,"~OLSYSTEM: ~FR%s %s tvojho klona v miestnosti %s!\n",user->name,rm->name,pohl(user,"znicil","znicila"));
                  write_user(u2,text);
                  }
            destructed=0; /*MORLOR ??? */
            return;
            }
      }
if (u2==user) sprintf(text,"Nemas klona v miestnosti %s.\n",rm->name);
else sprintf(text,"%s nema klona v miestnosti %s.\n",u2->name,rm->name);
write_user(user,text);
}


/*** Show users own clones ***/
void myclones(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt;

cnt=0;
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=CLONE_TYPE || u->owner!=user) continue;
      if (++cnt==1)
            write_user(user,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Miestnosti, v ktorych mas klonov ~OL~FW*)=~RS~FW=-~OL~FK--\n\n");
      sprintf(text,"  %s\n",u->room->name);
      write_user(user,text);
      }
if (!cnt) write_user(user,"Nemas ziadnych klonov.\n");
else {
      sprintf(text,"\nPocet Tvojich klonov: %d\n\n",cnt);
      write_user(user,text);
      }
}


/*** Show all clones on the system ***/
void allclones(user)
UR_OBJECT user;
{
UR_OBJECT u;
int cnt;

cnt=0;
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type!=CLONE_TYPE) continue;
      if (++cnt==1) {
            sprintf(text,"\n~OL~FK--~RS~FW-=~OL~FW=(*~RS~FW Zoznam klonov %s ~OL~FW*)=~RS~FW=-~OL~FK--\n\n",long_date(1));
            write_user(user,text);
            }
      sprintf(text,"%-15s : %s\n",u->name,u->room->name);
      write_user(user,text);
      }
if (!cnt) write_user(user,"Niesu aktivne ziadne klony.\n");
else {
      sprintf(text,"\nCelkovy pocet klonov: %d.\n\n",cnt);
      write_user(user,text);
      }
}


/*** User swaps places with his own clone. All we do is swap the rooms the
      objects are in. ***/
void clone_switch(user)
UR_OBJECT user;
{
UR_OBJECT u;
RM_OBJECT rm;

if (word_count<2) {
      write_user(user,"Pouzi: .switch <miestnost s klonom>\n");  return;
      }
if ((rm=get_room(word[1],user))==NULL) {
      write_user(user,nosuchroom);  return;
      }
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) {
            write_user(user,"\n~FB~OLPrezivas zvlastny pocit - opustas svoje telo...\n");
            u->room=user->room;
            user->room=rm;
            sprintf(text,"~OL%s %s svojho klona v tejto miestnosti!\n",u->name,pohl(u,"ozivil","ozivila"));
            write_room_except(user->room,text,user);
            sprintf(text,"~OL%s sa %s na klona!\n",u->name,pohl(u,"zmenil","zmenila"));
            write_room_except(u->room,text,u);
            look(user);
            return;
            }
      }
write_user(user,"V tej miestnosti nemas klona.\n");
}


/*** Make a clone speak ***/
void clone_say(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
RM_OBJECT rm;
UR_OBJECT u;

if (user->muzzled) {
      write_user(user,"Si umlcany, a ani tvoj klon nemoze rozpravat.\n");
      return;
      }
if (word_count<3) {
      write_user(user,"Pouzi: .csay <miestnost s klonom> <sprava>\n");
      return;
      }
if ((rm=get_room(word[1],user))==NULL) {
      write_user(user,nosuchroom);  return;
      }
for(u=user_first;u!=NULL;u=u->next) {
/*      sprintf(text,"Type: %d Room: %s Owner: %s\n",u->type,u->room->name,u->owner->name);
      write_room(NULL,text);
      sprintf(text,"Inpstr: %s\n",inpstr);
      write_room(NULL,text); */
      if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) {
            say(u,remove_first(inpstr),1);  return;
            }
      }
write_user(user,"V tej miestnosti nemas klona.\n");
}


/*** Set what a clone will hear, either all speach , just bad language
      or nothing. ***/
void clone_hear(user)
UR_OBJECT user;
{
RM_OBJECT rm;
UR_OBJECT u;

if (word_count<3
    || (strcmp(word[2],"all")
          && strcmp(word[2],"swears")
          && strcmp(word[2],"nothing"))) {
      write_user(user,"Pouzi: .chear <miestnost s klonom> all/swears/nothing\n");
      return;
      }
if ((rm=get_room(word[1],user))==NULL) {
      write_user(user,nosuchroom);  return;
      }
for(u=user_first;u!=NULL;u=u->next) {
      if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) break;
      }
if (u==NULL) {
      write_user(user,"V tej miestnosti nemas klona.\n");
      return;
      }
if (!strcmp(word[2],"all")) {
      u->clone_hear=CLONE_HEAR_ALL;
      write_user(user,"Kloni budu odteraz pocut vsetko.\n");
      return;
      }
if (!strcmp(word[2],"swears")) {
      u->clone_hear=CLONE_HEAR_SWEARS;
      write_user(user,"Kloni budu odteraz pocut len nadavky.\n");
      return;
      }
u->clone_hear=CLONE_HEAR_NOTHING;
write_user(user,"Kloni nebudu pocut nic.\n");
}


/*** Do AFK ***/
void afk(UR_OBJECT user,char *inpstr)
{

if (strlen(inpstr)) {
  if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
    write_user(user,noswearing);  
    sprintf(text,"~OL~FR~LB[CENZURA: %s sa %s pisat skaredo v hlaske afk!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
    writesys(WIZ,1,text,user);
    return;
   }
 }
user->miscoptime=thour*3600+tmin*60+tsec;
if (word_count>1) {
      if (!strcmp(word[1],"lock")) {
            inpstr=remove_first(inpstr);
            if (strlen(inpstr)>AFK_MESG_LEN) {
                  write_user(user,"AFK-sprava je prilis dlha.\n");  return;
                  }
            write_user(user,"Tvoje konto je uzamknute, odomknes ho svojim heslom.\n");
            if (inpstr[0]) {
		  sstrncpy(user->afk_mesg,inpstr, AFK_MESG_LEN);
                  sprintf(text,"AFK-sprava bola nastavena: ~OL%s~RS\n", inpstr);
                  write_user(user,text);
                  }
            user->afk=2;
           /* sprintf(text,"~OL%s si od klavesnice o %02d:%02d\n",pohl(user,"Odisiel","Odisla"),thour,tmin);
            record_tell(user,text);*/
            echo_off(user);
            }
      else {
            if (strlen(inpstr)>AFK_MESG_LEN) {
                  write_user(user,"AFK-sprava je prilis dlha!\n");  return;
                  }
            write_user(user,"Si v AFK mode. Stlac ENTER pre navrat do talkera.\n");
            if (inpstr[0]) {
		  sstrncpy(user->afk_mesg,inpstr, AFK_MESG_LEN);
                  sprintf(text,"AFK-sprava bola nastavena: ~OL%s~RS\n", inpstr);
                  if (!user->ignall || user->malloc_start==NULL)
		   write_user(user,text);
                  }
            user->afk=1;
           /* sprintf(text,"~OL%s si od klavesnice o %02d:%02d\n",pohl(user,"Odisiel","Odisla"), thour,tmin);
            record_tell(user,text);*/

            }
      }
else {
      write_user(user,"Si v AFK mode. Stlac ENTER pre navrat do talkera.\n");
      user->afk=1;
   /*   sprintf(text,"~OL%s si od klavesnice o %02d:%02d\n",pohl(user,"Odisiel","Odisla"), thour,tmin);
      record_tell(user,text); */
      }
if (user->vis) {
      if (user->afk_mesg[0])
            sprintf(text,"%s %s od klavesnice: %s\n", user->name, pohl(user,"odisiel","odisla"), user->afk_mesg);
      else sprintf(text,"%s %s od klavesnice.\n",user->name, pohl(user,"odisiel","odisla"));
      write_room_except(user->room,text,user);
      }
}


/*** Toggle user colour on and off ***/
void toggle_colour(user)
UR_OBJECT user;
{
int col;

/*
if (word_count<2) {
	write_user(user,"Pouzi: .colour <on | off | fei | special | test>\n");
	write_user(user,"       ~OL.colour on~RS      - zapne farby\n");
	write_user(user,"       ~OL.colour off~RS     - vypne farby\n");
	write_user(user,"       ~OL.colour fei~RS     - mod pre terminaly na FEI STU (bez blikania)\n");
	write_user(user,"       ~OL.colour special~RS - mod pre X-window\n");
	write_user(user,"       ~OL.colour test~RS    - test zobrazovania farieb\n\n");
	}
*/

/* A hidden "feature" , notalot of practical use but lets see if any users
   stumble across it :) */
if (user->command_mode && user->ignall && user->charmode_echo) {
      for(col=1;col<NUM_COLS;++col) {
            sprintf(text,"%s: ~%sNUTS 3 VIDEO TEST~RS\n",colcom[col],colcom[col]);
            write_user(user,text);

            }
      write_user(user,"~OL~BK~FYWOW - prave sa ti podarilo aktivovat super skrytu tajnu funkciu tohto talkeru!\nOkamzite kontaktuj najblizsieho GODa! Tajne heslo je \"sitnalta\"!!! ;-)\n");
      return;
      }
      
if (!strcmp(word[1],"fei")) {
	write_user(user,"Farby nastavene pre terminaly nepodporujuce blikanie.\n");
	user->colour=2;
	return;
	}      

if (!strcmp(word[1],"special")) {
	write_user(user,"Nastaveny mod pre X-window.\n");
	user->colour=3;
	return;
	}

if (!strcmp(word[1],"test")) {
	write_user(user,"\n~RS~FWTmavy text:\n~RS~FR Cervena   ~FGZelena   ~FYHneda   ~FBModra   ~FMFialova   ~FTBelasa   ~FWBiela\n");
	write_user(user,"~RS~FWSvetly text:\n~RS~OL~FR Cervena   ~FGZelena   ~FYZlta    ~FBModra   ~FMFialova   ~FTBelasa   ~FWBiela   ~FKCierna\n");          
	write_user(user,"~RS~FWPozadie:\n~RS~FW~BR Cervena ~BK ~FK~BG Zelena ~BK ~FW~BY Hneda ~BK ~BB Modra ~BK ~BM Fialova ~BK ~FK~BT Belasa ~BK ~BW~FK Biela ~RS~BK~FW\n");
	write_user(user,"~RS~FWOstatne:\n~LI Blikanie ~RS~RV Reverz ~RS ~UL Podciarknute~RS\n\n");
	return;
	}
      
if (!strcmp(word[1],"off")) {
      if (!user->colour) {
      	write_user(user,"Farby uz mas vypnute!\n");
      	return;
      	}
      write_user(user,"Farby su ~FRVYPNUTE.\n");
      user->colour=0;
      return;
      }

if (!strcmp(word[1],"on")) {
      user->colour=1;
      write_user(user,"Farby su ~FGZAPNUTE~FW, standardny rezim.\n");
      return;
      }
      
write_user(user,"Pouzi: .colour <on | off | fei | special | test>\n");
write_user(user,"       ~OL.colour on~RS      - zapne farby\n");
write_user(user,"       ~OL.colour off~RS     - vypne farby\n");
write_user(user,"       ~OL.colour fei~RS     - mod pre terminaly na FEI STU (bez blikania)\n");
write_user(user,"       ~OL.colour special~RS - mod pre X-window\n");
write_user(user,"       ~OL.colour test~RS    - test zobrazovania farieb\n\n");

}


void toggle_ignshout(user)
UR_OBJECT user;
{
if (user->ignshout) {
      sprintf(text,"%s si ignorovat shouty a semote.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignshout=0;
      return;
      }
write_user(user,"Odteraz ignorujes shout a semote.\n");
user->ignshout=1;
}

void toggle_igngossip(user)
UR_OBJECT user;
{
if (user->igngossip) {
      sprintf(text,"%s si ignorovat taraniny a prazdne reci..\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->igngossip=0;
      return;
      }
write_user(user,"Odteraz ignorujes taraniny a prazdne reci..\n");
user->igngossip=1;
}


void toggle_io(user)
UR_OBJECT user;
{
if (user->ignio) {
      sprintf(text,"%s si ignorovat prichody/odchody do/z miestnosti.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignio=0;
      return;
      }
write_user(user,"Odteraz ignorujes prichody/odchody do/z miestnosti.\n");
user->ignio=1;
}

void toggle_zvery(user)
UR_OBJECT user;
{
if (user->ignzvery) {
      sprintf(text,"%s si ignorovat prichody/odchody zvierat do/z miestnosti.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignzvery=0;
      return;
      }
write_user(user,"Odteraz ignorujes prichody/odchody zvierat do/z miestnosti.\n");
user->ignzvery=1;
}

void toggle_coltell(user)
UR_OBJECT user;
{
if (user->igncoltell) {
      sprintf(text,"%s si ignorovat fareby v telloch, pemotoch a gossipe.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->igncoltell=0;
      return;
      }
write_user(user,"Odteraz ignorujes farby v telloch, pemotoch a gossipe.\n");
user->igncoltell=1;
}

void toggle_ignlook(user)
UR_OBJECT user;
{
if (user->ignlook) {
      sprintf(text,"%s si ignorovat vypis .look po prihlaseni\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignlook=0;
      return;
      }
write_user(user,"Odteraz ignorujes vypis .look po prihlaseni.\n");
user->ignlook=1;
}


void toggle_igntell(user)
UR_OBJECT user;
{
if (user->igntell) {
      sprintf(text,"%s si ignorovat tell, pemote a tbanner.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->igntell=0;
      return;
      }
write_user(user,"Odteraz ignorujes kazdy tell, pemote a tbanner.\n");
user->igntell=1;
}

void toggle_ignbeep(user)
UR_OBJECT user;
{
if (user->ignbeep) {
      sprintf(text,"%s si ignorovat pipanie.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignbeep=0;
      return;
      }
write_user(user,"Odteraz ignorujes pipanie.\n");
user->ignbeep=1;
}

void toggle_ignfun(user)
UR_OBJECT user;
{
if (user->ignfun) {
      sprintf(text,"%s si ignorovat budicky, objatia a bozky\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignfun=0;
      return;
      }
write_user(user,"Odteraz ignorujes kazdy budicek, objatie a bozk.\n");
user->ignfun=1;
}

void toggle_ignportal(user)
UR_OBJECT user;
{
if (user->ignportal) {
      sprintf(text,"%s si ignorovat sepot z ineho ostrova.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignportal=0;
      return;
      }
write_user(user,"Odteraz ignorujes sepot z ineho ostrova.\n");
user->ignportal=1;
}


void toggle_ignsys(user)
UR_OBJECT user;
{
if (user->ignsys) {
      sprintf(text,"%s si ignorovat systemove hlasky.\n",pohl(user,"Prestal","Prestala"));
      write_user(user,text);
      user->ignsys=0;
      return;
      }
write_user(user,"Odteraz ignorujes systemove hlasky (LOGIN/LOGOUT apod.)\n");
user->ignsys=1;
}



/* Samovrazedne sklony... mensia ZMENA */
void suicide(user)
UR_OBJECT user;
{
int a,b;
if (word_count<2) {
      write_user(user,"Pouzi: .suicide <tvoje heslo>\n");  return;
      }
if (strcmp((char *)md5_crypt(expand_password(word[1]),user->name),user->pass)) {
      write_user(user,"Nespravne heslo.\n");  return;
      }
write_user(user,"\n\07~FR~OL~LI*** UPOZORNENIE - Sposobi to vymazanie tvojho konta na tomto talkeri! ***\n\n");
if (user->total_login>172800) {
  a=rand()%30+20;
  b=rand()%30+20;
  user->kontrola=a*b;      
  write_user(user,"Ak to naozaj chces, musis sa podrobit malemu matematickemu testu,\nna zistenie Tvojej sposobilosti urobit taketo zavazne rozhodnutie.\n");
  sprintf(text,"1.) ~OL%d * %d = ?\n",a,b);
  write_user(user,text);
  user->exams=5;
 }
else {
  a=rand()%50;
  b=rand()%50;
  user->kontrola=a+b;      
  sprintf(text,"Ak to naozaj chces, napis kolko je ~OL%d + %d!\n",a,b);
  write_user(user,text);
  user->exams=1;
 }
user->misc_op=6;
no_prompt=1;
}

/*GGG*/
void wipe_user(meno)
char meno[13];
{
FILE *infp, *outfp;
char filename[80], filename2[80];
char userheslo[7],usermeno[13], bogus[70];


sprintf(filename,"%s",ZOZNAM_USEROV);
if (!(infp=ropen(filename,"r"))) {return;} /*APPROVED*/

sprintf(filename2,"misc/users.new.temp");
if (!(outfp=ropen(filename2,"w"))) {  fclose(infp); return;} /*APPROVED*/

fscanf(infp,"%s %s %s", userheslo, usermeno, bogus);
while(!feof(infp)) {	
	if (!strcmp(usermeno,meno)) { }
		else {
		fprintf(outfp,"%-6s %-12s %s\n",userheslo,usermeno, bogus);
		}			
fscanf(infp,"%s %s %s", userheslo, usermeno, bogus);
}
 fclose(infp);
 fclose(outfp);
rename("misc/users.new.temp", "misc/users.new");
}

void rename_user_on_list(meno, novemeno)
char meno[13], novemeno[13];
{
FILE *infp, *outfp;
char filename[80], filename2[80];
char userheslo[7],usermeno[13], bogus[70];

sprintf(filename,"%s",ZOZNAM_USEROV);
if (!(infp=ropen(filename,"r"))) {return;} /*APPROVED*/

sprintf(filename2,"misc/users.new.temp");
if (!(outfp=ropen(filename2,"w"))) {  fclose(infp); return; } /*APPROVED*/

fscanf(infp,"%s %s %s",userheslo, usermeno, bogus);
while(!feof(infp)) {	
	if (!strcmp(usermeno,meno)) { 
			fprintf(outfp,"%-6s %-12s %s\n",userheslo,novemeno, bogus);		
			}
		else {
		fprintf(outfp,"%-6s %-12s %s\n",userheslo,usermeno, bogus);		
		}			
fscanf(infp,"%s %s %s", userheslo, usermeno, bogus);
}
 fclose(infp);
 fclose(outfp);
rename("misc/users.new.temp", "misc/users.new");
}

void rebirth(UR_OBJECT user)
{
UR_OBJECT u;

if (word_count<2 || strlen(word[1])>12) {
	write_user(user,"Pouzi: .rebirth <uzivatel>\n");
	return;
	}
word[1][0]=toupper(word[1][0]);

if ((u=get_user_exact(word[1]))!=NULL) {
	vwrite_user(user,"Uzivatel (%s) musi byt odhlaseny pocas reinkarnacie!\n", u->name); 
	return;
	}
	
repository(word[1],2);
write_user(user,"Reinkarnacia prebehla. Uspesnost skontroluj cez .examine\n");
sprintf(text,"%s RESURRECTED %s.\n",user->name,word[1]);
write_syslog(text,2);

}

/* UNUSED */
/*
void do_repository(char *subor, int typ) 
{
FILE *fp;
char repos[100];

sprintf(repos,"%s/%s", REPOSITORY, subor);
if (typ==0) {
	if ((fp=ropen(repos,"r"))!=NULL) {
		fclose(fp);
		unlink(subor);
		}
	else rename(subor,repos);	
	return;
	}
if (typ==1) {
	unlink(subor);
	return;
	}
if (typ==2) {
	rename(repos,subor);
	return;
	}
}
*/

void repository(char *name, int typ)
{
/* 0 - presun do repository, ak tam este nieje ... ak je, tak delene */
/* 1 - tvrdy delete */
/* 2 - obnova z repository */
UR_OBJECT u=NULL;

if (typ==1) db_deluser(name);
else {
  if ((u=create_user())==NULL) {
    write_syslog("CHYBA: Nepodarilo sa vytvorit docasny user object v repository().\n",0);
    return;
   }
  sstrncpy(u->name,name,12);
  if (typ==0 && load_user_details(u)) {
    save_user_details(u,2);
    db_deluser(u->name);
   }
  else if (typ==2) {
    db_load_user_details(u,1);
    db_save_user_details(u,0);
   }
 }
destruct_user(u);
destructed=0;

/*
sprintf(filename,"%s/%s.D",USERFILES,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.M",MAILFILES,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.P",USERFILES,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.notify",USERFILES,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.MC",MACRODIR,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.com",PRIKAZDIR,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.S", REMOTEDIR,name);
do_repository(filename, typ);

sprintf(filename,"%s/%s.N", NOTESDIR, name);
do_repository(filename, typ);

sprintf(filename,"whos/%s.who",name);
do_repository(filename,typ);

sprintf(filename,"xamines/%s.ex",name);
do_repository(filename,typ);
*/
}

/*** Delete a user ***/
void delete_user(user,this_user)
UR_OBJECT user;
int this_user;
{
UR_OBJECT u;
char name[USER_NAME_LEN+1],filename[81];
FILE *fp;
time_t tim;
int uid=0;

if (this_user) {
      /* User structure gets destructed in disconnect_user(), need to keep a
         copy of the name */
      strcpy(name,user->name);
      if (user->total_login/86400>=2) {
        time (&tim);
        sprintf(filename,"%s/suicides",DATAFILES);
        if ((fp=ropen(filename,"a"))) {
          sprintf(text,"%10s %s %s %s\n",zobraz_datum(&tim,4),level_name[user->level],user->name,user->desc);
          fputs(text,fp);
          fclose(fp);
         }
       }
      sprintf(query,"select `body` from `files` where `filename`='suicide.pic';");
      if ((result=mysql_result(query))) {
        if ((row=mysql_fetch_row(result)) && row[0]) {
          write_user(user,row[0]);
         }
        else write_user(user,"\n~FR~LI~OLTVOJE KONTO BOLO VYMAZANE!\n");
        mysql_free_result(result);
       }
      else write_user(user,"\n~FR~LI~OLTVOJE KONTO BOLO VYMAZANE!\n");
      sprintf(text,"~OL~LI%s %s samovrazdu! Uctime si minutou ticha %s pamiatku.\n",user->name,pohl(user,"spachal","spachala"),pohl(user,"jeho","jej"));
      write_room_except(user->room,text,user);
      sprintf(text,"%s %s SUICIDED with TLT %d hours.\n",level_name[user->level],name,(int)user->total_login/3600);
      write_syslog(text,1);
      level_log(text);
      jdb_vyrad(DB_JAIL,user->id);
      disconnect_user(user,1,NULL);
      repository(name,0); /* sjusajd is pejnless */
      return;
      }
if (word_count<2) {
      write_user(user,"Pouzi: .delete <uzivatel>\n");  return;
      }
word[1][0]=toupper(word[1][0]);
if (!strcmp(word[1],user->name)) {
      write_user(user,"Snaha vymazat sameho seba je jedenasty priznak sialenstva.\n");
      return;
      }
if (get_user(word[1])!=NULL) {
      /* Safety measure just in case. Will have to .kill them first */
      write_user(user,"Nemozes vymazat prave prihlaseneho uzivatela. Najskor ho .kill-ni\n");
      return;
      }
if ((u=create_user())==NULL) {
      sprintf(text,"%s: nemozno vytvorit docasny user object.\n",syserror);
      write_user(user,text);
      write_syslog("CHYBA: Nemozno vytvorit docasny user object v delete_user().\n",0);
      return;
      }
sstrncpy(u->name,word[1], 12);
if (!load_user_details(u)) {
      write_user(user,nosuchuser);
      destruct_user(u);
      destructed=0;
      return;
      }
if (u->level>=user->level) {
      write_user(user,"Nemozes len tak zmazat uzivatela s rovnakym/vyssim levelom ako mas sam.\n");
      destruct_user(u);
      destructed=0;
      return;
      }
uid=u->id;
destruct_user(u);
destructed=0;

repository(word[1],0); /* sjusajd */
wipe_user(word[1]);
jdb_vyrad(DB_JAIL,uid);
jdb_vyrad(DB_BAN,uid);
sprintf(text,"~LB~FR~OL~LIUzivatel %s bol zmazany!\n",word[1]);
write_user(user,text);
sprintf(text,"%s DELETED %s.\n",user->name,word[1]);
level_log(text);
write_syslog(text,1);
police_freeze(user,3);
}

/*** Shutdown talker interface func. Countdown time is entered in seconds so
      we can specify less than a minute till reboot. ***/
void shutdown_com(user)
UR_OBJECT user;
{
if (rs_which==1) {
      write_user(user,"Shutdown countdown je aktivny, musis ho najprv deaktivovat.\n");
      return;
      }
if (!strcmp(word[1],"cancel")) {
      if (!rs_countdown || rs_which!=0) {
            write_user(user,"Odpocitavanie do zhodenia systemu nieje aktivne.\n");
            return;
            }
      if (rs_countdown && !rs_which && rs_user==NULL) {
            write_user(user,"Niekto iny prave nastavuje odpocitavanie shutdown.\n");
            return;
            }
      write_room(NULL,"~OLSYSTEM:~RS~FG Odpocitavanie do padu systemu bolo zrusene.\n");
      sprintf(text,"%s zrusil system odpocitavanie shutdownu.\n",user->name);
      write_syslog(text,1);
      rs_countdown=0;
      rs_announce=0;
      rs_which=-1;
      rs_user=NULL;
      return;
      }
if (word_count>1 && !is_number(word[1])) {
      write_user(user,"Pouzi: .shutdown [<pocet sekund>/cancel]\n");  return;
      }
if (rs_countdown && !rs_which) {
      write_user(user,"Shutdown countdown je aktivny, musis ho najprv deaktivovat\n");
      return;
      }
if (word_count<2) {
      rs_countdown=0;
      rs_announce=0;
      rs_which=-1;
      rs_user=NULL;
      }
else {
      rs_countdown=atoi(word[1]);
      rs_which=0;
      }
write_user(user,"\n~LB~FR~OL~LI*** VAROVANIE - Toto zhodi system ATLANTIS! ***\n\nUrcite to chces (y/n)? ");
user->misc_op=1;
no_prompt=1;
}

/*** Reboot talker interface func. ***/
void reboot_com(user)
UR_OBJECT user;
{
if (!rs_which) {
      write_user(user,"Odpocitavanie restartu je prave aktivne, musis ho najprv deaktivovat.\n");
      return;
      }
if (!strcmp(word[1],"cancel")) {
      if (!rs_countdown) {
            write_user(user,"Odpocitavanie restartu nieje aktivne.\n");
            return;
            }
      if (rs_countdown && rs_user==NULL) {
            write_user(user,"Niekto iny prave nastavuje restart casovac.\n");
            return;
            }
      write_room(NULL,"~OLSYSTEM:~RS~FG Restart Atlantisu zruseny.\n");
      sprintf(text,"%s zrusil odpocitavanie restartu.\n",user->name);
      write_syslog(text,1);
      rs_countdown=0;
      rs_announce=0;
      rs_which=-1;
      rs_user=NULL;
      return;
      }
if (word_count>1 && !is_number(word[1])) {
      write_user(user,"Pouzi: .reboot [<pocet sekund>/cancel]\n");  return;
      }
if (rs_countdown) {
      write_user(user,"Odpocitavanie restartu je aktivne, musis ho najprv deaktivovat.\n");
      return;
      }
if (word_count<2) {
      rs_countdown=0;
      rs_announce=0;
      rs_which=-1;
      rs_user=NULL;
      }
else {
      rs_countdown=atoi(word[1]);
      rs_which=1;
      }
write_user(user,"\n~LB~FY~OL~LI*** WAROVANIE - Toto restartne system Atlantis! ***\n\nUrcite to chces (y/n)? ");
user->misc_op=7;
no_prompt=1;
}

/*** Show recorded tells and pemotes ***/
void revtell(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,cnt,line,kolko,total=0;
char *pom;

if (word_count>1) {
  if (!strcmp(word[1],"new")) {
    kolko=user->revline-user->lastrevt;
    if (kolko<0) kolko=kolko+REVTELL_LINES;
    if (kolko==0) {
      write_user(user,"Od posledneho revtellu ti neprisli ani si neposlal ziadne telly.\n");
      return;
     }
   }
  else kolko=atoi(word[1]);
  if (kolko) inpstr=remove_first(inpstr);
  if (kolko<1 || REVTELL_LINES<kolko) kolko=REVTELL_LINES;
 }
else kolko=REVTELL_LINES;

for(i=0;i<REVTELL_LINES;++i)
 if (user->revbuff[(user->revline+i)%REVTELL_LINES]!=NULL) total++;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVTELL_LINES;++i) {
  line=(user->revline+i)%REVTELL_LINES;
  if (user->revbuff[line]!=NULL) {
    cnt++;
    if (cnt==1) write_user(user,title("~FWTvoj tell buffer","~OL~FB"));
    /* *** */
    if (total-kolko<cnt) {
      pom=(char *) malloc ((strlen(user->revbuff[line])*sizeof(char))+100);
      if (pom!=NULL) {
        strcpy(pom, user->revbuff[line]);
        colour_com_strip(pom);
        force_language(pom,user->lang,1);
        strtolower(pom);
        /* *** */
        if (!strlen(inpstr) || strstr(pom,inpstr)) write_user(user,user->revbuff[line]);
        free(pom); 
       }
      else {
        sprintf(text,"~OL~FROUT OF MEMORY IN REVTELL()\n");
        write_level(KIN,1,text,NULL); 
        colour_com_strip(text);
        write_syslog(text,1);
       }
     }
   }
 }
if (!cnt) { if (user->lang) write_user(user,"No tells.\n");
            else write_user(user,"Este ti nikto nic nepovedal.\n");
          }  
else write_user(user,"~OL~FB.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
user->lastrevt=user->revline;
}

/*** Show recorded tells and pemotes ***/
void revirc_command(user,inpstr)
UR_OBJECT user;
char *inpstr;
{
int i,cnt,line;
char *pom;

cnt=0;
strtolower(inpstr);
for(i=0;i<REVIRC_LINES;++i) {
      line=(user->revircline+i)%REVIRC_LINES;
      if (user->revirc[line]!=NULL) {
            cnt++;
	    if (cnt==1) write_user(user,title("~FWTvoj IRC  buffer","~OL~FB"));
	   /* *** */
	   pom=(char *) malloc ((strlen(user->revirc[line])*sizeof(char))+1);
           if (pom!=NULL) {
  	     strcpy(pom, user->revirc[line]);
             colour_com_strip(pom);
	     strtolower(pom);
             /* *** */
	      if (!strlen(inpstr) || strstr(pom,inpstr)) write_user(user,user->revirc[line]);
	      free(pom);
             }
            else {
              sprintf(text,"~OL~FROUT OF MEMORY IN REVIRC()\n");
              write_level(KIN,1,text,NULL); 
              colour_com_strip(text);
              write_syslog(text,1);
             }
	   }
      }
if (!cnt) { if (user->lang) write_user(user,"No IRC session yet.\n");
            else write_user(user,"Este si nemal ziadnu IRC session.\n");
          }  
else write_user(user,"~OL~FB.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-\n");
}


/*SPAKKY - funkcia na vytvorenie nah. mena temp.suboru*/
char *get_temp_file()
{
static char tempname[80];

sprintf(tempname,"temp_%d.%d",rand()%500,rand()%500);
return tempname;
}


/****************** 
	Spakky. PODLA POHLAVIA VRATI SPRAVNY TVAR SLOVA
        0 = FEMALE
        1 = PROUD TO BE A MALE :>  
        Rider: Uplne prerobene, lebo to nefungovalo a bolo to
               zbytocne komplikovane :>
*******************/

char *pohl(user,muz,zena)
UR_OBJECT user;
char *muz, *zena;
{
if (user->sex) return (muz);
    else return(zena);
}

char *skloncislo(pocet,squirrel,squirrle,squirrelov)
int pocet;
char *squirrel,*squirrle,*squirrelov;
{
if (pocet==1) return (squirrel);
else if (1<pocet && pocet<5) return(squirrle);
else return (squirrelov);
}

/* ()STROV -----> */
void make_travel()
{
RM_OBJECT home,isle,ship;

if ((home=get_room(SHIPPING_HOME,NULL))==NULL) return;
if ((isle=get_room(SHIPPING_ISLAND,NULL))==NULL) return;
if ((ship=get_room(SHIPPING_SHIP,NULL))==NULL) return;
/* to boli testy ci vobec roomy existuju :) */

switch (ship_timer)
  {
   case 1: link_room(ship,home);       /* Lod v pristave */
           write_room(ship,"~FT~OLLod zakotvila pri mole v Atlantise!\n");
	   write_room(home,"~FT~OLDo pristavu prave vplavala plachetnica!\n");
           ship_timer=2;
           ship->group=1;
           break;

   case 2: unlink_room(ship,home);    /* Lod na mori */
           write_room(ship,"~FT~OLLod zdvihla kotvy a vydala sa na cestu...\n");
	   write_room(home,"~FT~OLLod zdvihla kotvy a pomaly odplavala...\n");
           ship_timer=3;
           ship->group=3;
           break;

   case 3: link_room(ship,isle);      /* Lod pri ostrove */
           write_room(ship,"~FT~OLLod priplavala k ostrovu Portalis!\n");
	   write_room(isle,"~FT~OLDo pristavu prave vplavala plachetnica!\n");
           ship_timer=4;
           ship->group=2;
           break;

   case 4: unlink_room(ship,isle);      /* Lod na mori */
           write_room(ship,"~FT~OLLod zdvihla kotvy a vydala sa na cestu...\n");
	   write_room(isle,"~FT~OLLod zdvihla kotvy a odplavala k Atlantide!\n");
           ship_timer=1;
           ship->group=3;
           break;
  }
}

void link_room_oneway(RM_OBJECT room,RM_OBJECT rm)
{
int i;

 for(i=0;i<MAX_LINKS;++i)
  if (room->link[i]==rm) return; /* aby sa nenalinkovala 1 ruma 2 qrat */
 
 for(i=0;i<MAX_LINKS;++i)
   if (room->link[i]==NULL) {
     room->link[i]=rm;
     if (i<MAX_LINKS-1) room->link[i+1]=NULL;
     break;
    }
}

void link_room(room,rm)
RM_OBJECT room;
RM_OBJECT rm;
{
 int i;
 
 if (rm==room) return;

 for(i=0;i<MAX_LINKS;++i)
  if (room->link[i]==rm) return; /* aby sa nenalinkovala 1 ruma 2 qrat */
 
 for(i=0;i<MAX_LINKS;++i)
     if (room->link[i]==NULL)
        {
         room->link[i]=rm;
         if (i<MAX_LINKS-1) room->link[i+1]=NULL;
         break;
        }

 for(i=0;i<MAX_LINKS;++i)
     if (rm->link[i]==NULL)
        {
         rm->link[i]=room;
         if (i<MAX_LINKS-1) rm->link[i+1]=NULL;
         break;
        }
}

void unlink_room(room,rm)
RM_OBJECT room;
RM_OBJECT rm;
{
 int i,x;

 if (room==rm) return;
/* if (rm==NULL)
  {
   for(i=0;i<MAX_LINKS;++i)
    room->link[i]=NULL;
   return;
  }*/
 
 for(i=0;i<MAX_LINKS;++i)
     if (room->link[i]==rm)
        {
         for(x=i;x<MAX_LINKS-1;++x)
            room->link[x]=room->link[x+1];
         room->link[x]=NULL;
        }

 for(i=0;i<MAX_LINKS;++i)
     if (rm->link[i]==room)
        {
         for(x=i;x<MAX_LINKS-1;++x)
            rm->link[x]=rm->link[x+1];
         rm->link[x]=NULL;
        }
}

/* <---- ()STROV */


void flyer_fly() /* FLYER */
{
RM_OBJECT letun,letisko;
UR_OBJECT user, pilot;
int cest=0;

if (flyer.pozicia==0) return; /* este sa nelieta :> */
if ((letun=get_room(FLYER_ROOM,NULL))==NULL) return; /* pre istotu... */
if ((letisko=get_room(FLYER_HOME,NULL))==NULL) return;

if (!(pilot=get_user_exact(flyer.pilot))) { /* pre istotu :> */
		flyer.pozicia=0;
		write_room(letisko,"~OL~FGDalsi letun sa vratil zo svojej cesty. Nechces nastupit?\n");
		link_room(letisko,letun);
		letun->group=2;
		return;
		}
		
switch (flyer.pozicia)
 {
  case 1: write_room(letun,"~OL~FGKrystal na letuni zacal ziarit a energeticke plachty sa rozvinuli!\n");
	  write_room(letisko,"~OL~FGLetun rozvinul energeticke plachty a vzniesol sa!\n");
  	  unlink_room(letun,letisko);
  	  letun->group=3;
          flyer.vykon=0;
          flyer.mind=10;
          flyer.vyska=0;
          flyer.palivo=300;
          flyer.vzdialenost=0;
          flyer.pozicia=2;
          flyer.timeout=0;
          break;

  case 2: if (flyer.vyska > 0)
            {
             flyer.pozicia=3;
	     write_room(letun,"~OL~FGLetun sa zatriasol a pomaly sa zdvihol zo zeme!\n");
	     write_room(letisko,"~OL~FGLetun sa pootocil a zmizol v dialke smerom na Atlantis...\n");
            }
          else
            {
             flyer.timeout++;
             if (flyer.timeout>FLYER_TIMEOUT)
                {
		write_user(pilot,"~OL~FGKrystal pohasol a plachty sa sklopili... Asi sa mas este vela co ucit...\n");
		write_room(letisko,"~OL~FGEnergeticke plachty na letuni pohasli a letun opat zosadol na zem...\n");
		sprintf(texthb,"~OL~FG%s sa %s riadenia...\n",pilot->name,pohl(pilot,"vzdal","vzdala"));
		write_room(letun,texthb);
		flyer.pozicia=0;
		link_room(letun,letisko);
		letun->group=2;
                return;
                }
            }
          break;

  case 3: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>50) {
              write_room(letun,"~OL~FGVzdusny pristav zmizol za chvostom letuna, bliziaceho sa k pobreziu...\n");
              flyer.pozicia=4;
              }
          break;
 
  case 4: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>100) {
              write_room(letun,"~OL~FGLetun sa prave ocitol nad sirym oceanom - kurz Atlantis!\n");
              flyer.pozicia=5;
              }
          break;

  case 5: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>150) {
              write_room(letun,"~OL~FGVsade naokolo iba nekonecny ocean a maly letun s posadkou uprostred...\n");
              flyer.pozicia=6;
              }
          break; 

  case 6: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>200) {
              write_room(letun,"~OL~FGPobrezie Atlantisu sa stale priblizuje... Letun je uz blizko!\n");
              flyer.pozicia=7;
              }
          break;          

  case 7: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>250) {
              write_room(letun,"~OL~FGLetun preletel nad pobrezim Atlantidy... Teraz najst miesto na pristanie...\n");
              flyer.pozicia=8;
              }
          break;

  case 8: if (check_crash_flyer()) return;
          if (flyer.vzdialenost>300) {
              write_room(letun,"~OL~FGPod letunom je namestie! Bajecne miesto na pristatie!\n");
              flyer.pozicia=9;
              }
          break;

  case 9: if (flyer.vyska==0) {
		write_room(get_room(FLYER_LAND,NULL),"~FBNad namestim zastavil letun z Portalisu...\n");
                write_room(letun,"~OL~FGVyborne! Letun jemne pristal na namesti Atlantidy!\n");
                cest=0;
		for (user=user_first;user!=NULL;user=user->next) {
		   if (user->login || user->room==NULL) continue;
		   if (user->room==letun) { 
		   	move_user(user,get_room(FLYER_LAND,NULL),5);
		   	if (strcmp(user->name,flyer.pilot)) cest++;
		   	}		   
		   }
		add_point(pilot,DB_LETUN,cest,flyer.palivo);
		write_room(get_room(FLYER_LAND,NULL),"~FBLetun zdvihol kotvy a zmizol v dialke...\n");
		flyer.pozicia=0;
		write_room(letisko,"~OL~FGDalsi letun sa vratil zo svojej cesty. Nechces nastupit?\n");
		link_room(letisko,letun);
		letun->group=2;
		return;
              }
              
          if (flyer.vzdialenost>350) {
              write_room(letun,"~OL~FGNeskoro... Letun sa opat ocitol nad pobreznou ciarou...\n");
              flyer.pozicia=10;
              }
          break;          

  case 10: if (check_crash_flyer()) return;
           if (flyer.vzdialenost>400) {
               write_room(letun,"~OL~FGLetun sa obratil a skusa opat naletiet na namestie v Atlantide...\n");
               flyer.pozicia=7;
               flyer.vzdialenost=210;
               }
           break;
 }

if ((rand()%3==1) && (flyer.mind > 0)) flyer.mind--;

flyer.palivo-=flyer.vykon;
if (flyer.palivo<0) flyer.palivo=0;
if (flyer.palivo==0) flyer.vykon=0;
flyer.vyska=(flyer.vyska+20*flyer.vykon/(20-flyer.mind)-10-(10-flyer.mind)/5);
if (flyer.vyska<0) flyer.vyska=0;
flyer.vzdialenost+=(flyer.vyska/10);

sprintf(texthb,"~FG[Sustredenie: ~OL%d~RS~FG Vykon: ~OL%d~RS~FG Vyska: ~OL%d~RS~FG Palivo: ~OL%d~RS~FG]\n",flyer.mind,flyer.vykon,flyer.vyska,flyer.palivo);
write_user(pilot,texthb);
}

int check_crash_flyer() /* FLYER - caplo nam to ci nie? :> */
{
UR_OBJECT user;
RM_OBJECT letun,letisko;
int cest=0;

if (flyer.vyska) return 0;

letisko=get_room(FLYER_HOME,NULL);
letun=get_room(FLYER_ROOM,NULL);

sprintf(texthb,"~FGCo sa to deje? %s straca kontrolu nad letunom!!!\n",flyer.pilot);
write_room(letun,texthb);

if (((flyer.pozicia > 4) && (flyer.pozicia < 8)) || (flyer.pozicia > 9)) { /* pad do mora */
	  sprintf(texthb,"~FG~OLSo silnym narazom letun dopadol na morsku hladinu a v jedinom okamihu sa roz-\n~FG~OLtriestil na tisicky kusov. Posledne co si pamatas je pocit tvojho tela rozdr-\n~FG~OLveneho strasnou silou narazu na kasu...\n");
	  write_room(letun,texthb);
  }
else { /* pad na zem */
  sprintf(texthb,"~FG~OLOzval sa priserny rachot, ako energeticke plachty letunu zavadili o zem, cely\n~FG~OLletun sa zachvel a z rachotom sa rozbil o zem. Posledne, co tvoje zmysly zare-\n~FG~OLgistrovali, bola obrovska explozia eletioveho krystalu...\n");
  write_room(letun,texthb);
  }



flyer.pozicia=0; /* toto je TU dolezite, inak sa to zacykli! */
for (user=user_first;user!=NULL;user=user->next) {
   if (user->login || user->room==NULL) continue;
   if (user->room==letun) {
       /*disconnect_user(user,3);*/
       if (strcmp(user->name,flyer.pilot)) cest--;
       user->dead=3; /*letun*/    
       }
    }
    
if (!(user=get_user_exact(flyer.pilot))) {	
	}
	else add_point(user,DB_LETUN,cest,0);
   
write_room(letisko,"~OL~FGPrave priletel dalsi letun! Nechces nastupit?\n");
link_room(letun,letisko);
letun->group=2;
return 1;
}

void check_messages(user)
UR_OBJECT user;
{
RM_OBJECT rm;
FILE *infp;
char filename[80],line[301];
int board_cnt, bad_cnt, tmp;

board_cnt=0;
bad_cnt=0;

for(rm=room_first;rm!=NULL;rm=rm->next) {
      tmp=rm->mesg_cnt;
      rm->mesg_cnt=0;
      sprintf(filename,"%s/%s.B",DATAFILES,rm->name);
      if (!(infp=ropen(filename,"r"))) continue; /*APPROVED*/
      
      board_cnt++;

      fgets(line,300,infp);
      while(!feof(infp)) {            
            if (strstr(line,"~OL~FM=-=-")) rm->mesg_cnt++;
            fgets(line,300,infp);
            }

      fclose(infp);
      if (rm->mesg_cnt!=tmp) bad_cnt++;            
      }
      
sprintf(texthb,"%d board files checked, %d had an incorrect message count.\n",board_cnt,bad_cnt);
write_user(user,texthb);      
}



/**************************** EVENT FUNCTIONS ******************************/

void do_events()
{
/* if (logcommands) log_commands("HBS","",0); */
set_date_time();

if (!hb_can) {
	if (logcommands) log_commands("HBE-CANNOT","",0);
	/* reset_alarm(); */
	return;
	}
check_reboot_shutdown();
check_idle_and_timeout();
udalosti();
check_cimazacat();
check_predstavenie();
check_security();
doom_load_users();
if (tsec%10==0) check_ident_reply();
if (tsec%4==0) check_web_commands();
flyer_fly(); /* FLYER */
if (doplnovanie_predmetov) {
  if ((ryxlost_doplnovania+(((100-num_of_users)*predmety_dynamic)/100))<=predmety_time) { 
    dopln_predmety();
    predmety_time=0;
   }
  else predmety_time+=2;
 }
test_lynx_done();
if (pollerror>0) {
  sprintf(text,"Poll(): Error condition (%d ks)\n",pollerror);
  write_syslog(text,1);
  pollerror=0;
 }
do_webusers();
 
if (every_min_num!=tmin) {
	every_min();
	every_min_num=tmin;
	if (tmin%5==0) every_5min();
	}

if (burkamin!=tmin && tsec==30) {
	burkamin=tmin;
	if ((tmin+3)%5==0) 
	do_burka();
	}
		 
if (daily_num!=tmday) {
	daily();
	daily_num=tmday;
	}
/* reset_alarm(); */
/* if (logcommands) log_commands("HBE","",0); */
return;
}

void every_5min()
{
/* if (logcommands) log_commands("E5M","",0); */
zapis_statistiku(); 
make_travel();
if (ryxlost_doplnovania==0) ryxlost_doplnovania=60;
misc_stuff(1);
/* remote_antiidle(); */
}


void every_min()
{
RM_OBJECT rm;
/* if (logcommands) log_commands("EM","",0); */
check_web_board();
who_for_web();
obnov_statline_userof();
do_tunel(NULL);
do_brutalis(NULL);
alter_maxtimeouts(0);
if (backuptime>0) backuptime--;
if (mesg_check_hour==thour && mesg_check_min+5==tmin && backuptime==0) {
  if (twday==0) do_db_backup(1); 
  else do_db_backup(0);
 }
if (mesg_check_hour==thour && mesg_check_min==tmin) {
  check_messages_night(0);
  if (logcommands) log_commands("WIPING EXPIRED MESSAGES","",0);
/*  sprintf(query,"",(int)time(0)); */
 }
for(rm=room_first;rm!=NULL;rm=rm->next) if (rm->countdown>0) {
  rm->countdown--;
  if (rm->countdown==0) rm->lemmein[0]='\0';
 }
if ((tmin+1)%5==0) misc_stuff(0);
}

void daily()
{
char email[255];
if (logcommands) log_commands("DAILY","",0);
zober_predpoved(NULL,0);
resc_save();
save_topic();
sprintf(email,"%s@%s",WIZZES_EMAIL_ALIAS,TALKER_EMAIL_HOST);
send_noticeboard_digest(email);
quest.lastquest=0;
}

/************************************************************/

void check_web_commands()
{
int cnt=0,id=0,uid=0,re=0,parse=0;
UR_OBJECT u;
RM_OBJECT rm;

 sprintf(query,"select `id`,`action`,`val` from `exec`");
 if ((result=mysql_result(query))) {
   while ((row=mysql_fetch_row(result))) {
     cnt++;
     if (!row[0]) continue;
     id=atoi(row[0]);
     switch (id) {
       case 1:
        if (!row[1]) continue;
        for (u=user_first;u!=NULL;u=u->next)
         if (u->level==0 && u->room!=NULL && !u->login 
         && u->type==USER_TYPE && !strcmp(row[1],u->name)) {
           sprintf(text,"~FMGratulujeme, prave si sa stal%s PLNOPRAVNYM obcanom Atlantisu.\n",pohl(u,"","a"));
           write_user(u,text);
           u->level++;
           sprintf(text,"~FG~OL%s na level: ~RS~OL%s!\n",pohl(u,"Bol si povyseny","Bola si povysena"), level_name[u->level]);
           write_user(u,text);
           sprintf(text,"~OL~FG%s~FR %s na level ~FY%s!\n",u->name,pohl(u,"bol povyseny","bola povysena"),level_name[u->level]);
           write_room_except(u->room,text,u);
           sprintf(text,"%s %s na level %s.\n",u->name,pohl(u,"bol AUTOMATICKY povyseny","bola AUTOMATICKY povysena"), level_name[u->level]);
           writesys(KIN,1,text,NULL);
           write_syslog(text,1);
           save_user_details(u,1);
          }
       break;
       case 2:
        if (!row[1]) continue;
        uid=atoi(row[1]);
        for (u=user_first;u!=NULL;u=u->next)
          if (u->id==uid) {
           write_user(u,"~LB~FT~OL** ~LIPRISLA TI NOVA POSTA (E-mailom)!~RS~OL~FT (pouzi: ~FY.rmail new~FT) **\n");
          }
       case 3:
        if (!row[1]) continue;
        rm=get_room(row[1],NULL);
        if (rm!=NULL) {
           rm->mesg_cnt++;
           sprintf(text,"%s napisal/a cez web spravu na nastenku (#%d).\n",row[2]?row[2]:"",rm->mesg_cnt);
           write_room(rm,text);
           re=1;
          }
       break;
       case 4:
         u=get_user_exact(row[1]);
	 if (u!=NULL) parse=1;
       break;
      }
    }
   mysql_free_result(result);
   if (cnt) {
     sprintf(query,"delete from `exec`");
     mysql_kvery(query);
    }
   if (re && rm!=NULL) {
     sprintf(query,"select count(`msgid`) from `board` where `room`='%s' and `deleted`=0",rm->name);
     rm->mesg_cnt=query_to_int(query);
    }
   if (parse) load_and_parse_predmets(u);
  }
}

void do_brutalis(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;
RM_OBJECT gate;
int cnt=0;
 
 if (portalis_gate==-666) return;
 if (user!=NULL) {
   if (!strcmp(user->room->name,"hrob")) {
     rm=get_room("krater",NULL);
     gate=get_room("hrob",NULL);
     user->room=rm;
     sprintf(text,"~FBZ utrob kratera zasvietilo svetlo a %s tu prask%s na zem.\n",user->name,pohl(user,"ol","la"));
     write_room_except(rm,text,user);
     sprintf(text,"~OLPrask%s si na zem a spamatavas sa.. Vsade je tma..\n",pohl(user,"ol","la"));
     write_user(user,text);
    }
   else {
     rm=get_room("cintorin",NULL);
     gate=get_room("diera",NULL);
     user->room=rm;
     sprintf(text,"~FB%s sa vyhrabal%s z hrobu..\n",user->name,pohl(user,"","a"));
     write_room_except(rm,text,user);
     sprintf(text,"~OLZrazu si sa ocit%s na cintorine..\n",pohl(user,"ol","la"));
     write_user(user,text);
     loose_predmets(user);
    }
  }
 else {
   rm=get_room("cintorin",NULL);
   gate=get_room("hrob",NULL);
   for(u=user_first;u!=NULL;u=u->next) {
     if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
     if (u->ap>20 && u->room==rm) ++cnt;
    }
   if (cnt>1) {
     if (portalis_gate<1000) {
       if (portalis_gate>4) {
         write_room(rm,"\n~HY~LI     \\  \\      ~RS   Z tmavej oblohy do jedneho z hrobov s ohlusujucim rachotom\n~HY~LI      \\  |     ~RS   udrel blesk..\n~HY~LI       | \\     ~RS   Na mieste kam udrel ostala v zemi velka diera z ktorej\n~HY~LI        \\ |    ~RS   vychadza oslepujuca ziara..\n");
         portalis_gate=1000+cnt;
         link_room_oneway(rm,gate);
        }
       else {
         portalis_gate+=cnt-1;
        }
      }
    }
   else {
     if (portalis_gate>0)  portalis_gate--;
     if (portalis_gate==1000)  portalis_gate=0;
    }
   if (portalis_gate>1000 && cnt>portalis_gate-1000) portalis_gate=1000+cnt;
   
   cnt=0;
   rm=get_room("krater",NULL);
   gate=get_room("diera",NULL);
   for(u=user_first;u!=NULL;u=u->next) {
     if (u->login || u->room==NULL || u->type!=USER_TYPE) continue;
     if (u->ap>20 && u->room==rm) ++cnt;
    }
   if (cnt>0) {
     if (brutalis_gate<1000) {
       if (brutalis_gate>2) {
         write_room(rm,"\n~HK    /\\ ~HW_~HK_~HW/\\     ~RS  Ozval sa ohlusujuci vybuch a v krateri sa objavila diera\n  /\\ ~DR_...~HW\\_|     ~RS z ktorej vychadza oslnujuce svetlo..\n");
         brutalis_gate=1000+cnt;
         link_room_oneway(rm,gate);
        }
       else {
         brutalis_gate+=cnt;
        }
      }
    }
   else {
     if (brutalis_gate>0) brutalis_gate--;
     if (brutalis_gate==1000) brutalis_gate=0;
    }
   if (brutalis_gate>1000 && cnt>brutalis_gate-1000) brutalis_gate=1000+cnt;
  }
}
  
void do_tunel(room)
RM_OBJECT room;
{
RM_OBJECT rm,tunel;
UR_OBJECT u;
int ran;

if (tunelik>1) tunelik--;
if (tunelik<0) tunelik++;
if (tunelik==1 || room!=NULL)
 { /* otvor tunel */
  tunel=get_room("tunel",NULL);
  if (room==NULL) {
    ran=(int)(rand()%3);
    if (ran==0) rm=get_room("vrchol",NULL);
    else if (ran==1) rm=get_room("pohorie",NULL);
    else rm=get_room("husty_les",NULL);
    write_room(rm,"~FBCast pody sa zosypala do nejakej diery v zemi.\n");
   }
  else rm=room;
  link_room(tunel,rm);
  ran=(int)(rand()%2);
  if (ran==0) rm=get_room("krcma",NULL);
  else rm=get_room("trhovisko",NULL);
  link_room(tunel,rm);
  write_room(rm,"~FBNieco zavrzgalo a naraz sa prepadli padacie dvere do nejakej pivnice.\n");
  write_room(tunel,"~HYDo tunela zacali prenikat svetelne luce..\n");
  tunel->sndproof=0;
  tunelik=-5-(int)(rand()%11); /* zavre sa za 5-15 min. */
 }
if (tunelik==0)
 { /* zatvor tunel */
  tunel=get_room("tunel",NULL);
  rm=get_room("vrchol",NULL);
  unlink_room(rm,tunel);
  rm=get_room("pohorie",NULL);
  unlink_room(rm,tunel);
  rm=get_room("husty_les",NULL);
  unlink_room(rm,tunel);
  rm=get_room("trhovisko",NULL);
  unlink_room(rm,tunel);
  rm=get_room("krcma",NULL);
  unlink_room(rm,tunel);
  write_room(tunel,"~FRPocujes dunenie.. Odrazu sa nad tebou zosypal strop..\n");
  for (u=user_first;u!=NULL;u=u->next)
   if (!u->login && u->room==tunel && u->type!=CLONE_TYPE && u->zaradeny)
    u->dead=9;
  tunel->sndproof=1;
  tunelik=120+(int)(rand()%121); /* otvori sa znova za 2 - 4 hodiny. */
 }
}

void do_burka()
{
RM_OBJECT ship;
UR_OBJECT user;
int i,kolko,rast;
if ((ship=get_room(SHIPPING_SHIP,NULL))==NULL) return;
 if (ship_timer%2==1)
  {
   sprintf(text,"~FBOkolo plachetnice sa prehnala burka a velke vlny sa preliali cez palubu.\n");
   write_room(ship,text);
   for(i=0;i<MPVM;i++)
    { ship->predmet[i]=-1; ship->dur[i]=0; }
   for (user=user_first;user!=NULL;user=user->next)
    {
     if (user->type!=USER_TYPE || !user->zaradeny
      || user->login || user->room!=ship) continue;
     kolko=0;
     rast=0;
     for(i=0;i<HANDS;i++)
      {
       if (user->predmet[i]>-1)
        {
         if (predmet[user->predmet[i]]->type==3 || predmet[user->predmet[i]]->type==4) rast++; /* rastlinky nevypadnu */
         else
          {
           user->carry-=predmet[user->predmet[i]]->weight;
           user->predmet[i]=-1;
           user->dur[i]=0;
           kolko++;
          }
	}
      }
     if (kolko>0 && rast==0)
      {
       sprintf(text,"Stih%s si sa chytit stoziara ale voda splachla vsetky tvoje predmety do mora.\n",pohl(user,"ol","la"));
       write_user(user,text);
      }
     if (kolko>0 && rast>0)
      {
       sprintf(text,"Stih%s si sa chytit stoziara ale niektore tvoje predmety splachla voda do mora.\n",pohl(user,"ol","la"));
       write_user(user,text);
      }
    }
  }
}

void send_noticeboard_digest(char* komu)
{
	FILE *fp;
	if((fp=ropen(NOTICE_DIGEST,"r"))) {
		fclose(fp);
		send_forward_email(komu,NOTICE_DIGEST);
	}
}

void obnov_statline_userof()
{
UR_OBJECT u;

for (u=user_first;u!=NULL;u=u->next) {
	if (!u->login && u->statline==CHARMODE) show_statline(u);
	}
}

void save_topic()
{
RM_OBJECT rm1;

for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
  sprintf(query,"update `rooms` set `topic`='%s' where `name_sk`='%s'",dbf_string(rm1->topic),rm1->name);
  mysql_kvery(query);
 }
}

void check_web_board()
{
 char filename[80], meno[20], rmeno[ROOM_NAME_LEN+1];
 RM_OBJECT rm;
 FILE *fp;
 
 sprintf(filename,"%s/%s",DATAFILES,WEB_BOARD_DAT);
 if ((fp=ropen(filename,"r"))==NULL) { /*APPROVED*/
            return;
            }
      
fscanf(fp,"%s %s",meno,rmeno);
	while(!feof(fp)) {
		    if ((rm=get_room(rmeno,NULL))!=NULL) {
			    sprintf(texthb,"%s napisal(a) cez web spravu na nastenku.\n",meno);
			    write_room(rm,texthb);
			    rm->mesg_cnt++;
			    }
	            fscanf(fp,"%s %s",meno,rmeno);
	            }
	    
 fclose(fp);
unlink(filename);
}

void zapis_statistiku()
{
FILE *fp, *temp;
int i, nasiel;
char match[40], string1[40];
char filename1[40], filename2[40];

sprintf(match,"%d.%d.%d",tmday,tmonth+1,tyear);
sprintf(filename1,"misc/statistika");
sprintf(filename2,"misc/tempstat.tmp");

if (!(fp=ropen(filename1,"r"))) { /*APPROVED*/
	/* nieje subor */
	nasiel=0;
	fp=ropen(filename1,"w"); /*APPROVED*/
	fprintf(fp,"%s %d\n",match,all_logins);
	all_logins=0;
	 fclose(fp);			
	return;
	}

if (!(temp=ropen(filename2,"w"))) { /*APPROVED*/
	 fclose(fp);
	return;
	}

i=0;
fscanf(fp,"%s %d",string1,&i);
nasiel=0;
while (!feof(fp)) {
	if (!strcmp(string1,match)) {
		i+=all_logins;
		fprintf(temp,"%s %d\n", string1, i);
		nasiel=1;
		all_logins=0;		
		}
	else fprintf(temp,"%s %d\n", string1, i);
	fscanf(fp,"%s %d",string1,&i);
	}
	
if (!nasiel) {
	fprintf(temp,"%s %d\n",match,all_logins);
	all_logins=0;
	}
	
 fclose(temp);
 fclose(fp);
rename("misc/tempstat.tmp","misc/statistika");
return;
}

void dopln_predmety()
{
RM_OBJECT rm,into;
UR_OBJECT u;
int pocet, i, r, nah, y,cnt=0,where=0;

if (pocet_predmetov==0) return;

while (1) {
  predmety_position++;
  if (predmety_position>=pocet_predmetov) predmety_position=0;
  i=predmety_position;
  if (cnt>=pocet_predmetov) return;
  cnt++;
 
  if (predmet[i]==NULL) return;

    pocet=0;		
    for(rm=room_first;rm!=NULL;rm=rm->next) {			
      for(y=0; y<MPVM; y++) if (rm->predmet[y]==i) pocet++;
     }		
    for(u=user_first;u!=NULL;u=u->next) {
      if (u==NULL || u->room==NULL || !u->zaradeny || u->login) continue;
      for(y=0; y<BODY; y++) if (u->predmet[y]==i) ++pocet;
     }			
    if (pocet<(predmet[i]->amount)) {
      where=predmet[i]->spawnarea;
      if (predmet[i]->spawn!=NULL) {
        into=predmet[i]->spawn;
	if (where==0 || (where==3 && into->group==4)
	 || (where==1 && into->group==1) || (where==2 && into->group==3)) {
          put_in_room(into,i,default_dur(i));
          continue;
	 }
       }
      if (where==0) {
        if (predmet[i]->type<10) where=2;
        else where=1;
       }
      r=0;
      if (where==2) while (strcmp(bylina_do_miestnosti[r],"*")) ++r;
      else if (where==3) while (strcmp(predmet_na_brutalis[r],"*")) ++r;
      else while (strcmp(predmet_do_miestnosti[r],"*")) ++r;
      nah=rand()%r;
      if (where==2) { if (!(rm=get_room(bylina_do_miestnosti[nah],NULL))) continue; }
      else  if (where==3) { if (!(rm=get_room(predmet_na_brutalis[nah],NULL))) continue; }
      else { if (!(rm=get_room(predmet_do_miestnosti[nah],NULL))) continue; }
      if (is_free_in_room(rm)==-1) continue;
      put_in_room(rm,i,default_dur(i));
      return;
     }
 }
}

RM_OBJECT random_room(RM_OBJECT rm,int ajprivat)
{
RM_OBJECT r2;
int i2,cnt,tar;
int target[MAX_LINKS];
cnt=0;
for(i2=0;i2<MAX_LINKS;++i2)
 {
  if (rm->link[i2]==NULL) continue;
  if (!ajprivat && (rm->link[i2]->access!=PUBLIC && rm->link[i2]->access!=FIXED_PUBLIC)) continue; 
  target[cnt]=i2;
  cnt++;
 }
if (cnt==0) return NULL;
tar=(int) (rand()%cnt);
r2=(rm->link[target[tar]]);
return r2;
}

void check_security()
{
if (newusers>2) minlogin_level=1; /* OPRAVIT */
newusers=0;
}

void check_cimazacat()
{
static int bolo2=0;
int upozorni;
int predhodina, predminuta;

upozorni=2;
predhodina=play.hodina;
if (play.minuta<upozorni) { predminuta=59-(upozorni-play.minuta-1);
			    if (play.hodina==0) predhodina=23;
			    else predhodina=play.hodina-1;
			    }
	else  predminuta=play.minuta-upozorni;
	       
/* (S) X minut pred predstavenim upozornime ... */
if ((thour==predhodina) && (tmin==predminuta)) {
	if (bolo2)  return; 
         }   
      else { bolo2=0; return; }

bolo2=1;
if (play.on==0) {
        wrtype=WR_NOEDITOR;
      	write_room(NULL,"~OL~FW~LB--Pozor! O 2 minuty zacina v amfiteatri divadelne predstavenie!\n");	
      	sprintf(texthb,"~OL~FW--Nazov predstavenia: ~FT%s ~FW(blizsie informacie - prikaz .play)\n",play.nazov);
      	write_room(NULL,texthb);
        wrtype=0;
       }

}

void check_predstavenie()
{
static int bolo=0;

      if (play.on!=2 && (((play.hodina==thour) && (play.minuta==tmin)) || (play.on))) {
            if (bolo && !play.on) return;
            }
      else {  bolo=0; return;  }
     
bolo=1;
     
if (play.on)
	{ if (!play.time--) play_it(); } /* prehrava to... prehrava to... etc...     */
   else
   { play_on(); }
	        
}

/* Spakky-UDALOSTI!! Toto je prikaz takpovediac na "ozivenie" talkera - kazda 
   miestnost ma svoj zoznam viet s danou pravdepodobnostou, z ktorych jedna sa, 
   ak nastane pravdepodobnostna udalost ktora sa riadi poissonovym rozdelenim;)
   Napriklad: Fuka tu silny vietor...
              V dialke pocut sumenie mora ...
              Vysoke vlny obmyvaju pevninu ...

   Kazda miestnost ma svoj subor ulozeny v DATAFILES a ma meno ako miestnost
   a priponu .udl
   Prvy riadok v subore je vzdy cislo, ktore udava POCET riadkov (viet) v 
   danom subore. Hned od dalsieho riadku zacinaju vety ...
***/
void udalosti()
{
FILE *fp;
RM_OBJECT rm;
UR_OBJECT u;
char filename[80],line[150]; /* hrtbt */
int kolko,m;
int pocet;

if (visit==2)
 {
  kolko=0;
  rm=get_room("zalar",NULL);
  for (u=user_first;u!=NULL;u=u->next)
   if (!u->login && u->room==rm && u->type!=CLONE_TYPE && u->zaradeny && !u->jailed && u->level<KIN)
    {
     write_user(u,"~OL~FRNavstevne hodiny prave skoncili. Straze ta odprevadili von zo zalara.\n");
     kolko++;
     u->room=room_first;
     sprintf(text,"<- %s %s.\n",u->name,u->in_phrase);
     if (u->vis) write_room_except(u->room,text,u);
    }
  if (kolko) write_room(rm,"Navstevne hodiny prave skoncili. Straze odprevadili navstevu von zo zalara.\n");
  visit=-300; /* navstevy znova najskor za 5 min. */
 }
if (visit<0) visit+=2;
if (visit>0) visit-=2;

if (((rand()%1200)+1)==60) {         /* TU SA ROZHODUJE AKO CASTO SA TO BUDE VOLAT */
				   /* napr. ..%60)+1)==30 znamena ze priblizne raz za 2 minuty 
				      (ak je hardbeat 2) */
for (rm=room_first;rm!=NULL;rm=rm->next) {
	sprintf(filename,"%s/%s.udl",DATAFILES,rm->name);
	if (!(fp=ropen(filename,"r"))) continue; /*APPROVED*/
	
	fscanf(fp,"%d",&pocet);
	if (!pocet) { 
		fclose(fp);
		continue;
		}
	fgets(line,149,fp); 		
	kolko=(rand()%(pocet))+1;	
	for(m=0; m!=kolko; m++) { 
		if (feof(fp)) 
			{
			fclose(fp);
			return;
			}
		fgets(line,149,fp); 
		}
	write_room(rm,line);
	 fclose(fp);
	}
}

/* Spakky - Nova funkcia - "poradca" pre novyx LUZEROF */

if((rand()%55)==15) {  /* Tu sa rozhodne ako casto bude newbie bombardovany
			    nasimi dobre mienenymi radami ;)) */
for (u=user_first;u!=NULL;u=u->next) {
	if (!u->login && u->level==NEW && u->room!=NULL && !u->ignall && u->type!=CLONE_TYPE && u->zaradeny) {
		write_user(u,"~OLAko novemu uzivatelovi ti odporucame:~RS\n");
		if (!u->colour) write_user(u,"   -> Skusit zapnut farby prikazom .col on (pri problemoch pouzi .col off)\n");
	/*	if (u->sex==2) write_user (u,"   -> Nastavit si pohlavie prikazom ~OL.set gender muz~RS alebo ~OL.set gender zena~RS\n"); */
		if (!strcmp(u->email,"Nema ziadny email")) write_user(u,"   -> Zaregistrovat sa ako obcan (CITIZEN) prikazom ~OL.request~RS\n");
		if (strstr(u->desc,"nema nastaveny")) vwrite_user(u,"   -> Nastavit si nejaku popisku - ~OL.desc~RS (zobrazuje sa pri .who, apod)\n");
                /*      a profil prikazom ~OL.profile~RS (zobrazi sa ostatnym pri .examine %s)\n", u->name); */
		if (strcmp(u->email,"Nema ziadny email")) {write_user(u,"   -> Precitat si mail ktory sme ti poslali po podani requestu, a spravit\n");
							   write_user(u,"      to, co sa v nom pise (t.j. prikaz ~OL.request <specialne heslo>~RS).\n");
							   write_user(u,"   -> Najst si tu mnoho dobrych priatelov ;-)\n");  							    }									    		
		}
	}
  }
}

void reset_alarm()
{
/* signal(SIGALRM,do_events); */
/* signal(SIGALRM,do_nothing); */
signal(SIGALRM,SIG_IGN);
/* alarm(heartbeat); */
}

void do_nothing()
{
/* toto nespravi vobec nic :> */
return;
}

/*** See if timed reboot or shutdown is underway ***/
void check_reboot_shutdown()
{
int secs;
char *w[]={ "~FRPad systemu","~FYRestart systemu" };

if (rs_user==NULL) return;
rs_countdown-=heartbeat;
if (rs_countdown<=0) talker_shutdown(rs_user,NULL,rs_which);

/* Print countdown message every minute unless we have less than 1 minute
   to go when we print every 10 secs */
secs=(int)(time(0)-rs_announce);
if (rs_countdown>=60 && secs>=60 && rs_countdown>=300) {
      sprintf(texthb,"~OLSYSTEM: %s o %d minut a %d sekund.\n",w[rs_which],rs_countdown/60,rs_countdown%60);
      write_room(NULL,texthb);
      rs_announce=time(0);
      }
if (rs_countdown<60 && secs>=10) {
      sprintf(texthb,"~OLSYSTEM: %s o %d sekund!!!\n",w[rs_which],rs_countdown);
      write_room(NULL,texthb);
      rs_announce=time(0);
      }
}

/*** login_time_out is the length of time someone can idle at login,
     user_idle_time is the length of time they can idle once logged in.
     Also ups users total login time. ***/
void check_idle_and_timeout()
{
UR_OBJECT user,next,u;
int tm, idle,mins;
char filename[80];
char inpstr[20];
int bazina,i;

/* Use while loop here instead of for loop for when user structure gets
   destructed, we may lose ->next link and crash the program */
   
sayfloodnum++;
user=user_first;
while(user) {
      next=user->next;
      if (user->type!=USER_TYPE) {  user=next;  continue;  }
      if (!user->zaradeny) { user=next; continue; }

      	    /* REM0TE */
            switch (user->remote_login)
     	    	{
     	    	case 3: user->remote_login=0; prihlas_irc(user); break;
	        case 2: user->remote_login=0; write2sock(NULL,user->remote_login_socket,user->remote_passwd,0);  break;
	        case 1: user->remote_login=2; write2sock(NULL,user->remote_login_socket,user->remote_name,0); break;
/*	        default: */
	    }


if (!user->login) {

  if (user->pp < MAXPP) {
	user->pp+=INCPP;
	if (is_funct_in_hand(user,4096)>-1) {
		user->pp+=INCPP; /*Dvojnasobne ryxlo*/
		if (user->pp%2==1) user->pp++;			
		}	
	if (user->statline==CHARMODE && (user->pp%2==0)) show_statline(user);
	}
  if (user->pp > MAXPP) user->pp=MAXPP;

  if (user->mana < SPELLK*user->level && tsec%3==0) {
    user->mana+=INCPP;
    if (is_funct_in_hand(user,131072)>-1) {
      user->mana+=3*INCPP;
      if (user->mana%2==1) user->mana++;
     }
   }
  if (user->mana > SPELLK*user->level) user->mana=SPELLK*user->level; /*SPELL*/
  }

if (tsec%6==0 && user->room!=NULL && (!strcmp(user->room->name,"hrob") || !strcmp(user->room->name,"diera"))) {
  do_brutalis(user);
 }

if (user->reveal>0) {
  user->reveal-=1;
  if (user->reveal==0) {
    i=0;
    for(u=user_first;u!=NULL;u=u->next)
     if (u->room==user->room && u->hidden==1 && (u->team==0 || u->team!=user->team)) i=1;
    if (i>0) {
      sprintf(text,"~FTSpozoroval%s si, ze sa tu niekto ukryva.\n",pohl(user,"","a"));
      write_user(user,text);
     }
   }
 }
if (user->hidden>1) {
  user->hidden-=1;
  if (user->hidden==1) {
    sprintf(text,"~FTTeraz si zamaskovan%s, ked vojde nepriatel do miestnosti,\n~FTbude mu chvilu trvat kym zisti ze si tu skryt%s.\n",pohl(user,"y","a"),pohl(user,"y","a"));
    write_user(user,text);
    sprintf(text,"~FT%s sa ukryl%s.\n",user->name,pohl(user,"","a"));
    write_room_except(user->room,text,user);
    for(u=user_first;u!=NULL;u=u->next) if (u->room==user->room) u->reveal=0;
   }
 }

  
if (user->hangupz>666) user->hangups=666;
if (user->hangupz>90) user->hangupz=90;
/* should work better :> */

if (user->saturate>3 && user->saturate<20) {
	user->saturate=(user->saturate*15);
	if (user->saturate>130) user->saturate=130;
	}
	else if (user->saturate>30) user->saturate-=10;
		else user->saturate=0;
		
/* say flood check */

if (sayfloodnum>=10) {
	sayfloodnum=0;
	tm=(user->total_login%86400)/3600;
	if (user->level<SOL && tm<10) {
		if (user->sayflood>=5) {
			user->sayflood=0;
			user->saturate=130;
			}			
		}
	user->sayflood=0;
	}

if (tsec==0 && !user->login && user->level>0 && user->level<KIN) {
 tm=user->total_login/86400;
 i=(user->total_login%86400)/3600;
 if (user->level>CIT && i==23 && (tlt4level[user->level]-tm)==1) autopromote(user,0);
 if (user->level<PRI && i==0 && !user->ignautopromote
 && (tm-tlt4level[user->level+1])==0) autopromote(user,1);
}

idle=(int)(time(0) - user->last_input)/60;
mins=(int)(time(0) - user->last_login)/60;
if (tsec<2 && mins==3 && strcmp(user->smsfollow,"-")) {
  sprintf(text,"%d:%02d %s logged in from %s, is on for 3 minutes, idle: %d min.",thour,tmin,user->name,user->site,idle);
  send_sms(user->smsfollow,text,0);
 }


if (tsec<2 && tmin%20==0 && user->wizactivity>0) user->wizactivity-=1;

/* if (tsec<2 && tmin%12==0 && user->smsssent>0) user->smsssent-=1; */
if (user->smswait) {
  user->smswait=0;
  sms(user,2);
 }
 
if (user->jailed>0) 
 { 
  user->jailed-=2;
  if (user->jailed<2) {
    user->jailed=0;
    move_user(user,get_room("namestie",NULL),3);
    write_user(user,"~OL~FRTvoj trest vyprsal!~OL\n");		
    sprintf(texthb,"~FT%s %s zo zalara - trest vyprsal!.\n",user->name,pohl(user,"bol prepusteny","bola prepustena"));
    write_room_except(NULL,texthb,user);
   }
 }	 

if (user->muzzletime>0) {
  user->muzzletime-=2;
  user->totalmuzzletime+=2;
  if (user->muzzletime<2) {
    user->muzzletime=0;
    user->muzzled=0;
    if (user->muzzle_t>-1) {
      put_in_hands(user,user->muzzle_t,predmet[user->muzzle_t]->dur);
      user->muzzle_t=-1;
     }
    write_user(user,"~OL~FRCas nahubku vyprsal!\n");
   }
 }

if ((user->flood>3) && (user->level<GOD)) {
  user->muzzled=1;
  user->muzzletime=60;
  write_user(user,"~OL~FRPrepac, floodujes! Minutu si ani neceknes...\n");
 }

user->flood=0;

/* Vyprsanie neactionovych predmetov */
if (user->affected>-1 && user->affecttime>0) {
  if (user->affecttime>250 && predmet[user->affected]->altfunct & 128) {
    if (rand()%2==0) {
      sprintf(text,"~FTZacalo ta napinat.. a uz to tu omietas zvyskami nestravenej potravy.\n");
      write_user(user,text);
      sprintf(text,"~FT%s zacalo napinat.. a uz to tu aj omietol zvyskami potravy.\n",sklonuj(user,4));
      write_room_except(user->room,text,user);
     }
    else {  
      sprintf(text,"~FTZaludok ta prestal posluchat.. a o chvilu uz vidis jeho obsah na zemi.\n");
      write_user(user,text);
      sprintf(text,"~FT%s prestal posluchat zaludok a o chvilu uz vidno jeho obsah na zemi.\n",sklonuj(user,4));
      write_room_except(user->room,text,user);
     }
    user->affecttime=user->affecttime/2; 
   }
  user->affecttime-=2;
  if (user->affecttime==1 || user->affecttime==2) {
    if (predmet[user->affected]->ustop!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[user->affected]->ustop,user,NULL,NULL,0));
      write_user(user,text);
     }
    user->affecttime=0;
    user->affected=-1;
   }
 }
if (user->zuje_t>-1 && user->zuje) {
  user->zuje-=2;
  if (user->zuje==1 || user->zuje==2) {
    if (predmet[user->zuje_t]->ustop!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[user->zuje_t]->ustop,user,NULL,NULL,1));
      write_user(user,text);
     }
    user->zuje=0;
    user->zuje_t=0;
   }
 }
if (user->glue) {
  user->glue-=2;
  if (user->glue==1 || user->glue==2) {
    user->glue=0;
    sprintf(texthb,"Pomaly sa odliepas a citis sa byt opat %s, yeah!\n",pohl(user,"slobodny","slobodna"));
    write_user(user,texthb);	
   }
 }

if (user->stars) {
  user->stars-=2;
  if (user->stars<3) {
    user->stars=0;
    sprintf(texthb,"Spamatal%s si sa a prestal%s si vidiet same ***.\n",pohl(user,"","a"),pohl(user,"","a"));
    write_user(user,texthb);	
   }
 }

if (user->room!=NULL && (is_funct_in_room(user->room,4)>-1 || is_funct_in_hand(user,4)>-1))
 user->lsd=-4;
if (user->lsd) {
  if (user->lsd==2 || user->lsd==1) {
    user->lsd=0;
    sprintf(texthb,"Ucinok drogy sa pomaly straca ...\n");
    write_user(user,texthb);
   }
  if (user->lsd>0) user->lsd-=2;
  if (user->lsd<0) user->lsd+=2;
  if (user->lsd>300) {
    sprintf(texthb,"Tvoj organizmus zjavne nevydrzal ucinky silnej drogy..\n");
    write_user(user,texthb);
    user->dead=7;
   }
 }
/*if ((user->tiktak) && (user->tiktak%10==0)) {
	write_user(user,"Tik ... Tak ... Tik ... Tak ... (hmmm, co to asi tak moze byt?)\n");
	}*/
if (user->afro) {
  user->afro-=2;
  if (user->afro==0)
   write_user(user,"Zacinas citit, ze sila afrodiziaka za pomaly vytraca ...\n");
 }

if (user->viscount<0) user->viscount+=2;
if (user->viscount>0) user->viscount-=2;

if (user->viscount==-14) {
	write_user(user,"Co sa to deje?! Zda sa, ze ti zmizli nohy!!!\n");	
	}
if (user->viscount==-8) {
	write_user(user,"Neuveritelne! Uz si nevidis vlastne nohy, dokonca ani ruky!\n");
	}
if (user->viscount==-2) {
	sprintf(texthb,"Puff! Pokusas si nahmatat aspon tvar, ale ta tu uz nieje ... ~OL~FTSI %s!!!\n",pohl(user,"NEVIDITELNY","NEVIDITELNA"));	
	write_user(user,texthb); /* ^ hmh, napoj ti odpazi aj hlavu ? ;) */
	sprintf(texthb,"~OL~FTPuff! ~RS~FW%s sa %s\n",user->name,pohl(user,"napil napoja neviditelnosti, a ... ZMIZOL!","sa napila napoja neviditelnosti, a ... ZMIZLA!"));
	write_room_except(user->room,texthb,user);
	user->viscount=120;
	user->vis=0;
	}
if (user->viscount==10) {
	write_user(user,"Zda sa, ze ucinok napoja neviditelnosti konci - objavila sa ti hlava!\n");
	}
	
if (user->viscount==2) {
	user->vis=1;
	user->viscount=0;
	sprintf(texthb,"~OL~FGFfup! ~RS~FWTvoje telo sa postupne od hlavy po paty znovu zmaterializovalo!\n");
	write_user(user,texthb);
	sprintf(texthb,"%s sa postupne materializuje ... prestava byt %s!\n",user->name,pohl(user,"neviditelny","neviditelna"));
	write_room_except(user->room,texthb,user);
	}

if ((user->prehana) && (user->prehana_t)) user->prehana_t-=2;
if ((user->prehana) && (user->prehana_t<=0)) {
  RM_OBJECT rm; int rcount, i;	
  user->prehana_t=10;
  user->prehana_t2-=1;
  if (user->prehana_t2<=0) {
    user->prehana=0;
    user->prehana_t=0;
    user->prehana_t2=0;		
   }
  else {	
    rcount=0;
    for(i=0;i<MAX_LINKS;i++) {
      if (user->room->link[i]!=NULL) rcount++;
      else break;
     }
    if (!rcount) {
      rm=get_room("namestie",NULL);
     }	
    else {
      i=random()%rcount;
      rm=user->room->link[i];
      if ((rm->access!=PRIVATE) && (rm->access!=GOD_PRIVATE) && (strcmp(user->room->name,"zalar"))) {
        sprintf(texthb,"%s sa pod vplyvom prehanadla rychlo %s %s\n",user->name,pohl(user,"odtackal","odtackala"),rm->into);
        write_room_except(user->room,texthb,user);
        move_user(user,rm,3);	
       }
     }
   }
 }
if (idle>user->killmeafter && user->level!=POS && !user->dead) {
 /* po 120 min to vyhodi kazdeho okrem POSa */
      write_user(user,"\n~OL~BRAstalavista, baby\n");
      /*disconnect_user(user,1);*/
      user->dead=2; /*timeout*/ 
      user=next; continue;
      }
else {
  if ((user->room!=NULL) && (!user->dead)) {
    if (strcmp(user->room->name,"zalar") && idle<5 && !user->afk) {
      user->total_login+=heartbeat;
     }
    else 
      if (idle>2 && (!user->afk || (user->afk && !strcmp(user->afk_mesg,"auto-afk"))))
        user->idletime+=heartbeat;
   }
 }

if (!user->login && !user->dead && !strcmp(user->room->name,"zalar") && (user->jailed)) {
  if (user->total_login>3600) {
   user->total_login-=ZALAR_TLT_CONSUMPTION*heartbeat; /* 666 */
   if (user->jailed==-1) user->nontimejails+=heartbeat;
   else user->totaljailtime+=heartbeat;
  }
  if (user->total_login<0) {
   user->total_login+=ZALAR_TLT_CONSUMPTION*heartbeat;
   if (user->jailed==-1) user->nontimejails-=heartbeat;
   else user->totaljailtime-=heartbeat;
  }
 }

if (--user->autosave<=0) {
	if (!user->login && user->room!=NULL) save_user_details(user,1);
	user->autosave=AUTOSAVE;
	}

if ((user->room!=NULL) && (user->alarm>0)) {
	user->alarm-=heartbeat;
	if (user->alarm==0) {
	    user->alarm=-1;
	    sprintf(filename,"%s/alarmfile",DATAFILES);
            /* ja nefim uz ci nam to niekto kradne alebo co,
               ale uz asi 5 krat sa stratil alarmfile..
               jednoducho zmizol a musel som ho tam znova kopirovat.. (V) */
            switch(more(user,user->socket,filename)) {
                  case 0: write_user(user,"~LB~LB~LB~LBNenasiel som sice alarm obrazok, ale na tento cas si si alarm nastavil!!!~LB~LB~LB~LB\n");  break;
                  case 1: user->misc_op=2;
			}	
		}
	}

     if (user->room!=NULL) if ((!strcmp(user->room->name,"amfiteater")) && (play.on==1)) {
	      	            user->last_input=time(0);
      		            }

      tm=(int)(time(0) - user->last_input);

      bazina=(user_idle_time-tm);
      if (bazina<1) bazina=1;
      if (!user->login && user->room!=NULL) {
	      if ((!strcmp(user->room->name,"bazina_smutku")) && (tm>=60) && (rand()%bazina==0) && !user->dead) {
      		    write_user(user,"\n~FR~OLTvoje nohy sa zrazu zaborili do niecoho makkeho...\n~FR~OLObrovska sila Ta stiahla do hlbin... Fuj, to blato chuti hnusne!\n");
      		    sprintf(texthb,"~FB~OLBazina si vyziadala dalsiu obet - tentokrat to bol jeden z nas...\n");
	            write_room_except(user->room,texthb,user);
	      	    user->dead=5;
	      	    user=next;
	      	    continue;
	      	    }
          }   
	
      if (user->level>time_out_maxlevel) {
        if (!user->afk && !user->login && tm>=(user->goafkafter*60)) {
	  sprintf(inpstr,"auto-afk");
	  word_count=2;
	  afk(user,inpstr);
	  user->warned=0;
         }
	user=next;
	continue;
       }
      if (user->login && tm>=login_idle_time) {
            if (!user->dead) write_user(user,"\n\n*** Cas vyprsal! ***\n\n");
	    /*disconnect_user(user,1);*/
	    user->dead=4; /*timeout-login*/
	    user=next;
            continue;
            }

      if (tm>1500 && (!user->dead)
       && !user->login && user->room->group==2) 
        do_attack(user);

      if (user->warned) {
            if (tm<user_idle_time-60) {  user->warned=0; user=next; continue; }
            if ((tm>=user_idle_time) && (!user->dead)) {
            	  if (user->afk && time_out_afks && tm<(user_idle_time*3)) {
            	  	user=next; continue;
            	  	}
                  write_user(user,"\n\n~LB~FR~OL~LI*** CAS VYPRSAL, DOVIDENIA! ***\n\n");
		  /*disconnect_user(user,1);*/
		  user->dead=2; /*timeout*/
                  user=next;
                  continue;
                  }
            }
      if ((!user->afk || (user->afk && time_out_afks))         
          && !user->login
          && !user->warned
          && tm>=(user->goafkafter*60-60)) {
            if (user->afk && tm>=(user_idle_time*3)-60) {
            	user->warned=1;
                sprintf(texthb,"\n~LB~FY~OL~LI*** POZOR! Ak do minuty nic nenapises, budes automaticky %s! ***\n", pohl(user,"odpojeny","odpojena"));
                write_user(user,texthb);            	
            	user=next;
            	continue;
            	}
            if (!user->afk && tm>=(user->goafkafter*60)) {
	            sprintf(inpstr,"auto-afk");
	            word_count=2;
	            afk(user,inpstr);
	            user->warned=0;
	            }
            }
      user=next;
      }
}

void check_messages_night(int vculeky)
{
RM_OBJECT rm;
int total=0,deleted=0;


if (vculeky) printf("Kontrolujem nastenky...");

sprintf(query,"update `board` set `deleted`=1 where `room`<>'trhovisko' and UNIX_TIMESTAMP(`time`)<%d",(int)time(0)-864000);
mysql_kvery(query);
deleted=mysql_affected_rows(&mysql);

for(rm=room_first;rm!=NULL;rm=rm->next) {
  sprintf(query,"select count(msgid) from board where room='%s' and deleted=0",rm->name);
  rm->mesg_cnt=query_to_int(query);
  total+=rm->mesg_cnt;
 }

sprintf(text,"  %d skontrolovanych, naslo sa %d prestarnutych sprav.\n",total+deleted,deleted);
if (vculeky) printf("%s",text);
write_syslog(text,1);
}

/***************************************************************************
***         Tuto su funkcie Amfiteatru (az do odvolania ;-))))     (R) *****
***************************************************************************/  
void play_on()            /* otvori file, ak je oki zapne prehravanie */
{
RM_OBJECT room;
room=get_room("amfiteater",NULL);

if ((play.file = ropen(play.name, "r")) == NULL)
   {
   write_room(room,"~FY~OLZ technickych dovodov sa predstavenie, zial, nekona...\n~FY~OLZasa ktosi stratil scenar! A toto mi robia furt! No nezabili by ste ich?!~RS~FW\n");
   if ((play.time=play_nxt())==ZERO) play.on=2;    /* nenaslo subor - nastavi casovac na dalsiu hru */
   return;
   }

clear_revbuff(room);
write_room(room,"\n~FBSvetla potemneli a miestnost sa zahalila do tajomneho sera...\n~FB~LI~OLPredstavenie prave ZACINA!~LB~RS~FW\n");
play.time=play.on=1;        /* nastavi flag play.on a cas na dalsiu slucku */
}

void vypis_predstavenia(riadok)
char riadok[MAXSTRING+1];
{
UR_OBJECT u;
RM_OBJECT rooma;

rooma=get_room("amfiteater",NULL);
record(rooma,riadok);
for (u=user_first;u!=NULL;u=u->next) {
	if (u->login || u->room==NULL || !u->zaradeny) continue;
	if (!strcmp(u->room->name,"amfiteater")) {
	    write_user(u,riadok); }
	}
return;

}

void play_it()                             /* Hlavna rutinka - prehravac */
{
char string[MAXSTRING+1]="", command[MAXSTRING+1]="", lcommand[MAXSTRING+1]="";
int pozicia, count;

  while (fgets(string,MAXSTRING,play.file)) /* fgets pri eof ci err vracia NULL */
  {
  string[strlen(string)-1]='\0';           /* fgets tam dava aj \n  -> odpazit */
  if (string[0]=='#') continue;
  if (string[0]!='<')
     {
     sprintf(texthb,"~OL~FT%s\n",string);    
     vypis_predstavenia(texthb);
     }
  else
    {
     command[0]='\0';
     for (pozicia=1; string[pozicia] != '>' && string[pozicia]!='\0'; pozicia++)
	 {
	 chrcat(command,string[pozicia]);              /* prikaz */
	 }
     strcat(command,"\0");
     count=0;
     for (pozicia++; string[pozicia]!='\0'; pozicia++) /* string */
	 {
	 string[count++]=string[pozicia];
	 }
     string[count]='\0';
     strcpy(lcommand,command);        /* vytvori sa "lowercommand" */
     strtolower(lcommand);            /* a tu sa "tolowerne"       */

    if (!strcmp(lcommand,"koniec"))
	{
	play_end();
	return;
	}
    else if(atoi(command))
	{
	play.time=atoi(command);
	return;
	}
    else if(!strcmp(lcommand,"title"))
	{
	sprintf(texthb,"~OL~FY%s\n",string);
	vypis_predstavenia(texthb);
	}
    else
	{
	if (strrchr(string,'?'))
	   {
	   sprintf(texthb,"~FG~OL%s ~FWsa pyta: ~RS~FW%s\n",command, string);
	   vypis_predstavenia(texthb);
	   }
	else if (strrchr(string,'!'))
	   {
	   sprintf(texthb,"~FG~OL%s ~FWkrici: ~RS~FW%s\n",command, string);
	   vypis_predstavenia(texthb);
	   }
	else
	   {
	   sprintf(texthb,"~FG~OL%s ~FWvravi: ~RS~FW%s\n",command, string);
	   vypis_predstavenia(texthb);
	   }
	}
    }
  play.time=1;
  }
play_end();
}


void play_end()  /* zaraxne fajl, ukonci vypis a nastavi dalsi cas */
{
RM_OBJECT room;
room=get_room("amfiteater",NULL);

play.on=0;
if ((play.time=play_nxt())==ZERO) play.on=2;
 fclose(play.file);
write_room(room,"~OL~LI~FTT H E   E N D~RS\n");
}


char *chrcat(char *dest, char src)   /* ako strcpy ale so znakom ;-) */
{
int pozicia;
pozicia=strlen(dest);
*(dest+pozicia++)=src;
*(dest+pozicia)='\0';
return(dest);
}

long play_nxt(void) /* (S) uprava */
{
   char filename[MAXSTRING+1], ourtime[MAXSTRING+1], nazov[80];
   long curmin,ourmin;
   int count,poz;
   /* int cilhod, cilmin; */
   int cnt2,cnt3;
   FILE *program;

/* Najskor zistime, kolko mame hodin (resp. minut) */
   play.on=0;
   /* cilhod=thour; */ /* Aky cas je CIL :> */
                 /* po Nitriansky: VCULEKY! :> */
   /* cilmin=tmin; */
   curmin=thour*60+tmin;      
   if ((program = ropen(KOLOS_PROGRAM,"r")) == NULL) return(ZERO); /*APPROVED*/
   
   count=0; /*fcil posluzi ako flag */
   while (fgets(ourtime,MAXSTRING,program)!=NULL)  /* divoky string alebo eof */
    {
    cnt2=0;
    cnt3=0;
    if (strlen(ourtime) < 5) {  fclose(program); return (ZERO); } /* debilneho string */
    strcpy(nazov,""); strcpy (filename,"");
    for (poz=6; ourtime[poz]!='\n'; poz++) {
	if (!cnt2) { filename[poz-6]=ourtime[poz];
		     if (ourtime[poz]==' ') { filename[poz-6]='\0'; cnt2=1; }
			}
	if (!cnt3) { nazov[poz-6]=ourtime[poz+35];
		     if (ourtime[poz+35]=='\n') { nazov[poz-6]='\0'; cnt3=1; }
		     }
	}
    ourtime[5]='\0'; /*Cas predstavenia vo prehladavanom riadku*/
    
    if ((ourmin=asctomin(ourtime,0))==ZERO) { fclose (program); return (ZERO);} /* vadny format */
    if (!count) {ourmin=asctomin(ourtime,1); strcpy(play.name,filename); strcpy(play.nazov,nazov); count=1; }
    if (curmin<ourmin)
       {
        fclose(program);
       ourmin=asctomin(ourtime,1);
       strcpy(play.name,filename);
       strcpy(play.nazov,nazov);
       return 1;
       }
    }
    fclose(program);
   return 1;
}

long asctomin(char string[], int upd) /* konverzia "HH:MM" na minuty (S) uprava*/
   {
   char buffer[10];
   long minutes;

   if (strlen(string)!=5 || string[2]!=':') return(ZERO); /* blby format */
   buffer[2]='\0';
   buffer[0]=string[0]; buffer[1]=string[1];
   if (upd) play.hodina=atoi(buffer);
   minutes=60*atoi(buffer);
   buffer[0]=string[3]; buffer[1]=string[4];
   if (upd) play.minuta=atoi(buffer);
   minutes+=atoi(buffer);
   return(minutes);
   }

void wizzes(user)
UR_OBJECT user;
{
UR_OBJECT u;
int pocet=0;
int cnt;
char lajna[500];

for(u=user_first;u!=NULL;u=u->next) {
	if ((u->level>=KIN) && (u->level<=GOD) && u->vis && !u->login) {
		++pocet;
		if (pocet==1) {
			write_user(user,"~FGPrihlaseni su nasledovni strazcovia Atlantidy:\n\n");
			}			
	        sprintf(lajna,"  ~FT%s ~FW%s",u->name,u->desc);
		cnt=colour_com_count(lajna);
		if (u->afk)
		 sprintf(text,"  %-*.*s  ~OL~FWAFK\n", 40+cnt, 40+cnt, lajna); 	
		else
		 sprintf(text,"  %-*.*s  ~RS~FWIdle ~OL~FW%d ~RSmin.\n", 40+cnt, 40+cnt, lajna,(int)(time(0)-u->last_input)/60); 	
		write_user(user,text);
		}
	}
	if (!pocet) {
		write_user(user,"Bohuzial, nie je pritomny ziadny strazca Atlantidy. Skus sa so svojim problemom\n");
		write_user(user,"obratit na cloveka s levelom KNAZ (.who KNAZ), alebo pockaj kym sa prihlasi\n");
		write_user(user,"niekto zo strazcov.\n");
		}
	else write_user(user,"\n~FGNa tychto ludi sa mozes obratit so svojimi problemami.\n");
}

void zazuvackuj(inpstr, ok)
char *inpstr;
int ok;
{
unsigned int i;
char *newimpstr;

if ((newimpstr=(char *)malloc(4000*sizeof(char)))==NULL) {
	*inpstr='\0';
	return;
	}
*newimpstr='\0';

for (i=ok; i<word_count; i++) {	
	strcat(newimpstr,word[i]);
	strcat(newimpstr," *MLASK!* ");
	}
strcat(newimpstr," . o O (PUK!)");
sstrncpy(inpstr,newimpstr,ARR_SIZE-1);

free((void *)newimpstr);
}

/*void zachlastaj(inpstr, ok)
char *inpstr;
int ok;
{
int i, nah, j;
char *newimpstr;

if ((newimpstr=(char *)malloc(5000*sizeof(char)))==NULL) {
	*inpstr='\0';
	return;
	}
*newimpstr='\0';

for (i=ok; i<word_count; i++) {
	if (i>1) {
		for (j=0; j<strlen(word[i]); j++) {
			nah=rand()%8;
			if (nah==3) word[i][j]='_';
			if (nah==5) word[i][j]='*';
			}
		}
	strcat(newimpstr,word[i]);
	nah=rand()%20;
	switch(nah) {
	        case 1:
		case 2:
		case 3: strcat(newimpstr," *Grg!* "); break;
		case 4:
		case 5:
		case 6: strcat(newimpstr," *Hik!* "); break;
		case 7: strcat(newimpstr," *[Nejde o tooo!]* "); break;
		case 8: strcat(newimpstr," *[h-h-hnevas sa n-na mna ?]* "); break;		
		case 9:
		case 10:
		case 11: strcat (newimpstr," ..hhh.. "); break;				
		default: strcat (newimpstr," "); break;
		}
	}
sstrncpy(inpstr, newimpstr, ARR_SIZE-1);
free((void *)newimpstr);
}
*/
void zachlastaj2(inpstr, ok)
char *inpstr;
int ok;
{
unsigned int i, nah, j;
char *newimpstr;

if ((newimpstr=(char *)malloc(5000*sizeof(char)))==NULL) {
	*inpstr='\0';
	return;
	}
*newimpstr='\0';

for (i=ok; i<word_count; i++) {
	if (i>1) {
		for (j=0; j<strlen(word[i]); j++) {
			nah=rand()%8;
			if (nah==3) word[i][j]='%';
			if (nah==5) word[i][j]='*';
			if (nah==7) word[i][j]='#';
			}
		}
	strcat(newimpstr,word[i]);
	nah=rand()%20;
	switch(nah) {
	        case 1:
		case 2:
		case 3: strcat(newimpstr," *Grrrrrrrrg!* "); break;
		case 4: strcat(newimpstr," *Goooaaaargrh!* "); break;
		case 5:
		case 6: strcat(newimpstr," *Hiiik!* "); break;
		case 7: strcat(newimpstr," *[Neeejde o tooo!]* "); break;
		case 8: strcat(newimpstr," *[h-h-h-hnevaas sa n-n-nn-na m-mm-na ?]* "); break;
		case 9: strcat(newimpstr," *[b-bbodzgaaaaaaj!]* "); break;
		case 10:
		case 11: strcat (newimpstr," ..hhhaaah.. "); break;				
		case 12: strcat (newimpstr," *Bleeeeeah!* "); break;		
		case 13: strcat (newimpstr," [!@#$%^&*] "); break;
		default: strcat (newimpstr," "); break;
		}
	}
sstrncpy(inpstr,newimpstr, ARR_SIZE-1);
free((void *)newimpstr);
}

void zalamerizuj(inpstr, ok)
char *inpstr;
int ok;
{
int nah, nah2,wordct=0;

wordct=word_count-ok;
if (wordct<1) wordct=1;
nah=(rand()%wordct)+ok;
nah2=(rand()%6);

if (ok>1) strcpy(texthb,"Huh! ");
else texthb[0]='\0';

if ((ok) && (nah==0)) nah=1;
switch(nah2) {
	case 0: sprintf(text,"%sCo znamena slovo '%s' ?",texthb,word[nah]); break; 
	case 1: sprintf(text,"%sVie mi niekto vysvetlit co znamena '%s' ?",texthb,word[nah]); break; 
	case 2: sprintf(text,"%sCo je to '%s' ?",texthb,word[nah]); break; 
	case 3: sprintf(text,"%sProsim povedzte mi, co to znamena '%s'!",texthb,word[nah]); break; 
	case 4: sprintf(text,"%sCo znamena '%s' ?!",texthb,word[nah]); break; 
	case 5: sprintf(text,"%sJa som lamer, neviem co znamena slovo '%s'!",texthb,word[nah]); break;
	default: sprintf(text,"%sPoviete mi uz konecne, co je to '%s' ?!",texthb,word[nah]); break; 
	}

sstrncpy(inpstr,text,ARR_SIZE-1);
}

void debilneho(inpstr, ok)
char *inpstr;
int ok;
{
unsigned int i;
char *newimpstr;

if ((newimpstr=(char *)malloc(4000*sizeof(char)))==NULL) {
	*inpstr='\0';
	return;
	}
*newimpstr='\0';

for (i=ok; i<word_count; i++) {
	if (i%3!=0) strcat(newimpstr,word[i]);
		else strcat(newimpstr,"debilneho");
	strcat(newimpstr," ");
	}
sstrncpy(inpstr,newimpstr, ARR_SIZE-1);
free((void *)newimpstr);
}

/* M@CRO>> ... taaaakze, MAKROIDNE FUNKCIE!!! */
 
void list_user_macros(UR_OBJECT user,int global) {
MACRO macro;
FILE *fp;
char filename[81];

 sprintf(filename,"%s%s%s",TMPFOLDER,user->name,TMPSUFFIX);
 if ((fp=ropen(filename,"r"))!=NULL) { /*APPROVED*/	
   fclose(fp);
   deltempfile(filename);
  }
 if ((fp=ropen(filename,"w"))==NULL) { /*APPROVED*/
   write_user(user,"Chyba: Nastal problem so zapisovanim docasneho suboru.\n");
   return;
  }
 if (global) {
   fprintf(fp,"\n~OL~FTZoznam globalnych makier:\n~RS~FW");
   for (macro = defaultmacrolist; macro!=NULL; macro = macro -> next) {
     sprintf(text,"%s=%s~RS~FW~BK\n",macro->name,macro->value);
     fputs(text,fp);	
    }
   if (user->level>=GOD) fprintf(fp,"Globalne makra (misc/macro) mozes obnovit prikazom: ~OL~FW.macro reload\n");
  }
 else {
   fprintf(fp,"\n~OL~FTMas vytvorene tieto makra:\n~RS~FW");
   for (macro = user->macrolist; macro!=NULL; macro = macro -> next) {
     sprintf(text,"%s=%s~RS~FW~BK\n",macro->name,macro->value);
     fputs(text,fp);	
    }
   fprintf(fp,"\nVytvorenie makra: ~OL~FR.macro nazov=prikaz  ~RS~FWMazanie: ~OL~FR.macro nazov=\n");
   fprintf(fp,"Viac informacii cez prikaz ~OL.pomoc macro~RS  Momentalne mas %d %s.\n",user->macro_num,skloncislo(user->macro_num,"makro","makra","makier"));
   if (user->level>=GOD) fprintf(fp,"Globalne makra (misc/macro) mozes obnovit prikazom: ~OL~FW.macro reload\n");
  }
 fclose(fp);
switch(more(user,user->socket,filename)) {
  case 0: write_user(user,"Chyba pri citani makier!\n");  break;
  case 1: user->misc_op=2;
 }	
} /* list_user_macros */

void show_user_macros(UR_OBJECT user, UR_OBJECT u) {
	MACRO macro;
	sprintf(text,"~OL~FT%s ma tieto makra:\n~RS~FW",u->name);
	write_user(user,text);
	for (macro = u->macrolist; macro!=NULL; macro = macro -> next) {
		sprintf(text,"%s=%s\n",macro->name,macro->value);
		write_user(user,text);	
	}
 } /* show_user_macros */


void free_macrolist(MACRO *list) { 
	MACRO macro,next;

	macro = *list;

	while ( macro ) {
		next=macro->next;
		free_macro(macro);
		macro=next;
	} 

	*list=NULL;

} /* free_macrolist */

void free_macro(MACRO macro) {
	free(macro->name);
	free(macro->value);
	free(macro);
} /* free_macro */
/*
int save_macros(MACRO *list,char *filename) { 
	FILE *fp;	
	MACRO macro;

	if (! ( fp=ropen( filename,"w" ) ) ) {
		sprintf(text,"Nemozem otvorit subor <%255s> pre zapis",filename);
		write_syslog(text,1);
		return FAIL;
	}
	
	for (macro = *list; macro != NULL; macro= macro->next) {
		sprintf(text,"%s=%s\n",macro->name,macro->value);
		fputs(text,fp);
	}

	 fclose(fp);
	return SUCCESS;
} */

void macro(MACRO *list,char *input,UR_OBJECT user,int is_action) {
        
        UR_OBJECT u;
	char *equals,*value,*name;
	MACRO macro;

	/* If just .macro by itself, list the macros */

	if ((!input[0])&&(is_action==0)) {
		list_user_macros(user,0);
		return;
	}

if (!strcmp(word[1],"forward")) {
  fmail(user,2);
  return;
 }

if ((!strcmp(word[1],"reload")) && (user->level>=GOD)) {
  global_macros(user);
  return;
 }

if (!strcmp(word[1],"global") || !strcmp(word[1],"fixed")) {
  list_user_macros(user,1);
  return;
 }
 
if ((!strcmp(word[1],"user")) && (user->level>=KIN)) 
       {
       if (word_count<3)
          {
           write_user(user,"Pouzi: .macro user <uzivatel>\n");
           return;
          }
       if ((u=get_user(word[2]))==NULL)
          {
           write_user(user,"Nikto s takym menom nie je momentalne prihlaseny.\n");
           return;
          }
        else
          {
           show_user_macros(user,u);
           return;
          }
       }

	equals=strchr(input,'=');

        if (!equals && word_count==2) {        	
        	macro=ismacro(&user->macrolist,NULL,word[1]);
        	if (macro!=NULL) {
        		vwrite_user(user,"%s=%s\n", macro->name, macro->value);
        		}
        	else vwrite_user(user,"Makro s nazvom ~OL%s~RS neexistuje.\n", word[1]);
        	return;
        	}
	if (!equals) {
		write_user(user,"Medzi meno makra a prikazmi musis vlozit rovnitko (=)\n");
		return;
	}
	*equals='\0'; /* end the string at the equals sign */

	name=input; /* macro name is the part before the equals sign */
	value=equals+1; /* macro value starts after the equals sign */

	if (!value[0]) {		/* if macro has no value portion, delete macro */ 
		delete_macro(user, list,name);		
		return;
	}
	if ((macro = findmacro(list,NULL,name))!=NULL) {
		update_macro(user, macro,value);
		return;
	}
	add_macro(list,name,value);
        sprintf(query,"replace into `macros` (`userid`,`name`,`value`) values ('%d','",user->id);
        strcat(query,dbf_string(name)); strcat(query,"','");
        strcat(query,dbf_string(value));strcat(query,"');");
        mysql_kvery(query);
	write_user(user,"Makro bolo pridane.\n");
        user->macro_num++;
} /* macro */

void delete_macro(UR_OBJECT user, MACRO *list,char *name) {

	MACRO macro,prev;
	prev = NULL; /* no previous macro on the list */

	for (macro = *list; macro != NULL; macro = macro -> next) {

		if (! strcmp(macro->name,name) )  {
			/* if we don't have a previous macro,change head of list */
			if (prev) 
				prev -> next = macro -> next;
			else
				*list = macro->next;
			free_macro(macro);
                        sprintf(query,"delete from `macros` where `userid`='%d' and `name`='%s';",user->id,dbf_string(name));
                        mysql_kvery(query);
			write_user(user,"Makro bolo vymazane.\n");
                        if (user->macro_num>0) user->macro_num--;
			return;
		}
		prev=macro;     
	} /* for */ 

	/* if we exit the loop without deleting finding anything, we didn't
	   have anything to delete anyway :-) */
write_user(user,"Makro s takym nazvom neexistuje!\n");
} /* delete_macro */

int update_macro(UR_OBJECT user, MACRO macro,char *value) {
	char *newvalue;
	newvalue=(char *) malloc(strlen(value)+1);
	if (newvalue) {
		strcpy(newvalue,value);
		free(macro->value);
		macro->value=newvalue;
                sprintf(query,"replace into `macros` (`userid`,`name`,`value`) values ('%d','",user->id);
                strcat(query,dbf_string(macro->name)); strcat(query,"','");
                strcat(query,dbf_string(macro->value));strcat(query,"');");
                mysql_kvery(query);
		write_user(user,"Makro bolo zmenene.\n");
		return SUCCESS;
	}

	write_syslog("Couldn't allocate memory in function update_macro",1);
	return FAIL;	
} /* update macro */
  
void add_macro(MACRO *list,char *name,char *value) {
	MACRO macro,prev;
	int num,star;
	char *replace,replace_type;

	if ( !( macro = allocate_macro(name,value) ) ) {
		write_syslog("CHYBA: Nedostatok pamate vo funkcii add_macro",1);
		return;
	}

	strcpy(macro->name,name);
	strcpy(macro->value,value);

	/* find macro whose name alphabetically preceeds "name" */
	prev = findprevmacro(list,name);

	if (prev == NULL ) { /* no precursor, add to start of list */
		macro->next = *list;
		*list = macro;
	} else {
		macro->next=prev->next; /* point to next entry in list */
		prev->next = macro;		/* insert macro into list */
	}	
	/* Store the starting place of a $* expansion, for speed */ 

	star = 0;

	while ((replace=strchr(value,'$'))!=NULL){
		replace_type = replace[1];
                if (star==0 && (replace_type=='n' || replace_type=='g' || replace_type=='d' || replace_type=='a')) star=1;
		if (isdigit(replace_type)){
			num = todigit(replace_type); 
			if (num > star)
				star = num;
		}

		if (replace[1])
			value=replace+2;
		else
			break; /* Hit end of string, we're done */

	} /*while*/

	star++; /* we start with first *unused* word */
	macro->star = star;
	
	macro->is_running = FALSE; /* starts out *not* running :-) */
} /* add macro */

MACRO allocate_macro(char *macroname,char *macrovalue) {
	MACRO macro;
	char *name,*value;

	/* try to allocate the space we need */

	name=(char *) malloc(strlen(macroname)+1);
	value=(char *) malloc(strlen(macrovalue)+1);
	macro = (struct macro_struct *) malloc(sizeof(struct macro_struct));

	/* if we can't, free it all, and exit */

	if (! (name && value && macro ) ){
	
		if (!name) free(name);
		if (!value) free(value);
		if (!macro) free(macro);

		return NULL;
	}

	/* assign good defaults to our macro */
	macro->next = NULL;
	macro->name = name;
	macro->value = value;
	macro->star = 0;
	return macro;
} /* allocate macro */
	
int macroexpand(MACRO *list,MACRO *deflist,char *expansion, UR_OBJECT user, int irc) {
MACRO macro;
UR_OBJECT u;
unsigned int i, def;
char *replacement,*string, replace_type;
int overcheck=ARR_SIZE, size=0; /* Hnuusny overflow!!! :-( */

	expansion[0]='\0'; /* initialize properly */
        
	def=0; 
	if (!(macro=ismacro(list,deflist,word[0]))) {
		if (!(macro=ismacro(list,NULL,"default"))
		 || (user->command_mode || strchr("./;!@$<>-#:,`",word[0][0])))
		  { return FALSE; }
		else def=1;
	/* return FALSE; */
	}		
	
	if (macro->is_running)
		return FALSE; /* we expand macros only once */
	
	string=macro->value;
	
	while (*string) {
	
		replacement=strchr(string,'$');
		if (!replacement) {
			size=strlen(string);
			if (size>overcheck) break;
			strcat(expansion,string);
			overcheck-=size;
			break;
		} 

		if (replacement-string > overcheck) break;
		strncat(expansion,string,replacement-string);
		overcheck-=(replacement-string);
		/* copy macro text up to the point of the replacment */

		replace_type=replacement[1]; /* first char after $ is type */

		if (isdigit(replace_type)) { /* if $[0-9] */
			size=strlen(word[(int)todigit(replace_type)]);
			if (size>overcheck) break;
			strcat(expansion,word[(int)todigit(replace_type)]);
			overcheck-=size;
			}
		if (replace_type=='n' || replace_type=='g' || replace_type=='d' || replace_type=='a') {
                        if (!word[1][0]) {
                          write_user(user,"Musis zadat parameter!\n");
                          expansion[0]='\0';
                          return TRUE;
                         }
                        if ((u=get_user(word[1]))==NULL) {
                          write_user(user,notloggedon);
                          expansion[0]='\0';
                          return TRUE;
                         }
                        switch (replace_type) {
                          case 'n': size=strlen(u->name); break;
                          case 'g': size=strlen(sklonuj(u,2)); break;
                          case 'd': size=strlen(sklonuj(u,3)); break;
                          case 'a': size=strlen(sklonuj(u,4)); break;
                         }
			if (size>overcheck) break;
                        switch (replace_type) {
                          case 'n': strcat(expansion,u->name); break;
                          case 'g': strcat(expansion,sklonuj(u,2)); break;
                          case 'd': strcat(expansion,sklonuj(u,3)); break;
                          case 'a': strcat(expansion,sklonuj(u,4)); break;
                         }
			overcheck-=size;
			}
                if (replace_type == '$') {
                	if (1>overcheck) break;
                	strcat(expansion,"$");
                	overcheck--;                	
                	}
		if (replace_type == '*') {     /* if $* */
		        size=0;			
			for (i=macro->star-def; i<word_count; i++) {
			  size=strlen(word[i])+1;
			  if (size>overcheck) break;
			  strcat(expansion,word[i]);
			  strcat(expansion," ");
			  overcheck-=size;
                         }
			if (size>overcheck) break; /* while */
			/* if (expansion[0]) expansion[strlen(expansion)-1]='\0'; */ /* remove final space */
		}


		if ( !replacement[1] ) /* if we're at end of string, get out */
			break;
		string = replacement+2;

	} /* while */
	macro -> is_running = TRUE;
	parse(user,expansion,irc); /* re-parse the expanded string */
	macro -> is_running = FALSE;
	return TRUE;
} /* macroexpand */

char todigit(char c) {
	 return (c-'0');
}

MACRO ismacro(MACRO *list,MACRO *deflist,char *command) {
	MACRO macro;

        if (command==NULL) {
          sprintf(text,"command je NULL in ismacro()!\n");
          if (logcommands) log_commands("BULLSHIT",text,1);
          return NULL;
         }
	for ( macro= *list; macro != NULL; macro=macro->next ) {

          if (macro->name==NULL) {
            sprintf(text,"macro->name je NULL in ismacro()!\n");
            if (logcommands) log_commands("BULLSHIT",text,1);
            return NULL;
           }
          
/* Zmena: Only Exact Hit (R)
		if ( !strncmp( macro->name,command,strlen(command) ) )  */
		if (!strcmp( macro->name,command) ) 
			return macro;
	}
	if (deflist!=NULL) for ( macro= *deflist; macro != NULL; macro=macro->next ) {
	 if ( !strcmp( macro->name,command) ) 
	 return macro;
	}
	return NULL;
} /* ismacro */		

/* like "ismacro", but look for an exact match */
MACRO findmacro(MACRO *list,MACRO *deflist,char *command) {
	MACRO macro;

	for ( macro= *list; macro != NULL; macro=macro->next ) {
		if ( !strcmp( macro->name,command) )
			return macro;
	}
	if (deflist!=NULL) for ( macro= *deflist; macro != NULL; macro=macro->next ) {
		if ( !strcmp( macro->name,command) )
			return macro;
	}
	return NULL;
}
/* this function returns the macro whose name alphabetically preceeds
   "name", or NULL if no such macro is found */
MACRO findprevmacro(MACRO *list, char *name) {
	MACRO current,prev;

	prev=NULL;

	for (current = *list; current != NULL; current = current -> next) {
		if ( strcmp(current->name,name) > 0 ) /* alphabetically preceeds */
			 return prev;
		prev = current; /* store previous entry */
	}

	return prev;
}

void parse(UR_OBJECT user,char *string, int irc) {
	char *separator, *start;
	char expansion[ARR_SIZE*3];    /* FIX ASAP */
	int done=FALSE;

	start=string; /* store starting position of string */
	expansion[0]='\0';	/* clear expansion string */
	
	
	while (!done) {
	if (user->com_counter<=0) /* Zmena (R) - counter commandov :) */
	   {
	    write_user(user,"~FR~OL~LIDosiahnuty maximalny pocet prikazov!\n");
	    break;
	   }
		separator = strchr(string, INPUT_SEP); 
		if(separator){
			if (separator[1] == INPUT_SEP){  
				 string = separator+2; /* skip past both separators */
				 continue;
				}
			else {
				separator[0]='\0'; /* turn it into the end of string */
				}
				
		 } /* if */
                user->com_counter--;
		clear_words();
		word_count=wordfind(start);

		/* if we have macros, re-parse the string */
		if ( strlen(word[0]) > 0 ) { /* if we really have a command */

/*			if (!macroexpand(&(user->macrolist),&(defaultmacrolist),expansion,user, irc)) */
			if ((!irc && !macroexpand(&(user->macrolist),&(defaultmacrolist),expansion,user, irc)) 
			 || (irc && !macroexpand(&(user->macrolist),NULL,expansion,user, irc)) )
			 {
				/* convert any multiple separators to single for execution */
				fix_separators(start,expansion);
			
			/*	M@CRO zmena -> nehlada word[0], ale inpstr (R)
			        if ( user->command_mode || strchr("./;!@$<>-#:,`",word[0][0])) */
			        
			        if (irc) {
			        	irc_prikaz(user, expansion);
			        	}
			        else {
					if ( user->command_mode || strchr("./;!@$<>-#:,`",start[0]))
						{
						 dump_all_users_data();
						 reset_all_users_xml();
						 exec_com(user,expansion);
						 dump_all_users_data();
						 if ((destructed && !inyjefuc) || (user->misc_op)) { done=TRUE; return; }
						 else if (strstr(expansion,".do")) { user->com_counter=0; done=TRUE; return; }
						}
					else
					{
						dump_all_users_data();
						reset_all_users_xml();
						say(user,expansion,0);
						dump_all_users_data();
					}
				}

			}

		   }

		if (separator) { 		/* we have more */
			start = separator+1; /* point past separator character */
			string=start;
		        }
		 else {
			done=TRUE;
		      }

	} /*while*/
} /* parse */

void fix_separators(char *input,char *output) {
int pos=0,des=0;
 
 while(*(input+pos)) {
   if (*(input+pos)==INPUT_SEP && *(input+pos+1)==INPUT_SEP) pos++;
   *(output+des)=*(input+pos);
/*   sprintf(text,"%4d-%c  ",des,*(output+des)); */
/*   write_room(testroom,text); */
   pos++;
   des++;
  }
 *(output+des)='\0';
} /* fix_separators */

int quotacheck(char *username) /* ze ci ma danemu userovi quotu checkovat */
{
FILE *fp;
char menko[20];

if (!(fp=ropen(NO_QUOTA,"r"))) { /*APPROVED*/
	return 1;
	}
username[0]=toupper(username[0]);	
fscanf(fp,"%s", menko);
while (!feof(fp)) {
	menko[0]=toupper(menko[0]);
	if (!strcmp(menko, username)) {
		fclose(fp);
		return 0;
		}
	fscanf(fp,"%s", menko);	
	}
fclose(fp);
return 1;	
}

int showfile(UR_OBJECT user, char filename[90])
{
FILE *fp;
char linaj[501];

if (!(fp=ropen(filename,"r"))) return 0; /* APPROVED */

fgets(linaj, 500, fp);
while (!feof(fp)) {
	write_user(user, linaj);
	fgets(linaj, 500, fp);
	}
fclose(fp);
if (strlen(filename) >= strlen(TMPSUFFIX)) {
	if (!strcmp(strchr(filename,'\0')-strlen(TMPSUFFIX),TMPSUFFIX)) deltempfile(filename);
}
return 1;
}

int showrow(UR_OBJECT user)
{
int pos=0,i=0;

query[0]='\0';
while (row[0][pos]!='\0') {
  query[i]=row[0][pos];
  if (query[i]=='\n' || i>400) {
    query[i+1]='\0';
    write_user(user,query);
    i=0;
   }
  else i++;
  pos++;
 }
return 1;
}


/* Rider ja Ta proste uz len koli tomuto obdivujem ... */
/* Mas za co, hehe  :>> (skromnost je moja silna stranka, ze?) */

char *sklonuj(UR_OBJECT juzer, int pad)
{
static char dopadnute[USER_NAME_LEN+10];
char meno[USER_NAME_LEN+10];
int pohl;

if (pad==2 && juzer->sname[0][0]) return juzer->sname[0];
else if (pad==3 && juzer->sname[1][0]) return juzer->sname[1];
else if (pad==4 && juzer->sname[2][0]) return juzer->sname[2];
strcpy(meno, juzer->name);
pohl=juzer->sex;

strcpy(dopadnute,meno);

if (pohl) switch (pad) { /* M */
  case 1: break;
  case 2: if (lastchar(meno,"ter")) strcpy(dopadnute,striend(meno,"ter","tra"));
	  else if (lastchar(meno,"ay")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"ey")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"oy")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"y")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"ie")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"i")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","a"));
	  else if (lastchar(meno,"ok")) strcpy(dopadnute,striend(meno,"ok","ku"));
	  else if (lastchar(meno,"oj")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","u"));
	  else if (lastchar(meno,"ec")) strcpy(dopadnute,striend(meno,"ec","ca"));
/*	  else if (lastchar(meno,"es")) strcpy(dopadnute,striend(meno,"es","a")); */ /*!!! sa */
	  else if (lastchar(meno,"et")) strcat(dopadnute,"u");
	  else if (lastchar(meno,"kus")) strcpy(dopadnute,striend(meno,"kus","ka"));
	  else if (lastchar(meno,"seus")) strcpy(dopadnute,striend(meno,"seus","sea"));
	  else strcat(dopadnute,"a");
	  break;
  case 3: if (lastchar(meno,"ter")) strcpy(dopadnute,striend(meno,"ter","trovi"));
	  else if (lastchar(meno,"ay")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"ey")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"oy")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"y")) strcat(dopadnute,"mu");
	  else if (lastchar(meno,"ie")) strcat(dopadnute,"mu");
	  else if (lastchar(meno,"i")) strcat(dopadnute,"mu");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","ovi"));
	  else if (lastchar(meno,"ok")) strcpy(dopadnute,striend(meno,"ok","ku"));
	  else if (lastchar(meno,"oj")) strcat(dopadnute,"u");
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","ovi"));
	  else if (lastchar(meno,"ec")) strcpy(dopadnute,striend(meno,"ec","covi"));
/*	  else if (lastchar(meno,"es")) strcpy(dopadnute,striend(meno,"es","ovi")); */ /* !!! */
	  else if (lastchar(meno,"et")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"kus")) strcpy(dopadnute,striend(meno,"kus","kovi"));
	  else if (lastchar(meno,"seus")) strcpy(dopadnute,striend(meno,"seus","seovi"));
	  else strcat(dopadnute,"ovi");
	  break;
  case 4: if (lastchar(meno,"ter")) strcpy(dopadnute,striend(meno,"ter","tra"));
	  else if (lastchar(meno,"ay")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"ey")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"oy")) strcat(dopadnute,"a");
	  else if (lastchar(meno,"y")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"ie")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"i")) strcat(dopadnute,"ho");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","a"));
	  else if (lastchar(meno,"ok")) break;
	  else if (lastchar(meno,"oj")) break;
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","u"));
	  else if (lastchar(meno,"ec")) strcpy(dopadnute,striend(meno,"ec","ca"));
/*	  else if (lastchar(meno,"es")) strcpy(dopadnute,striend(meno,"es","a")); */ /* !!! sa */
	  else if (lastchar(meno,"et")) break;
	  else if (lastchar(meno,"kus")) strcpy(dopadnute,striend(meno,"kus","ka"));
	  else if (lastchar(meno,"seus")) strcpy(dopadnute,striend(meno,"seus","sea"));
	  else strcat(dopadnute,"a");
	  break;
  case 6: if (lastchar(meno,"ter")) strcpy(dopadnute,striend(meno,"ter","trovi"));
	  else if (lastchar(meno,"ay")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"ey")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"oy")) strcat(dopadnute,"ovi");
	  else if (lastchar(meno,"y")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"ie")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"i")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","ovi"));
	  else if (lastchar(meno,"ok")) strcpy(dopadnute,striend(meno,"ok","ku"));
	  else if (lastchar(meno,"oj")) strcat(dopadnute,"i");
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","ovi"));
	  else if (lastchar(meno,"ec")) strcpy(dopadnute,striend(meno,"ec","covi"));
/*	  else if (lastchar(meno,"es")) strcpy(dopadnute,striend(meno,"es","ovi")); */ /*!!! sovi */
	  else if (lastchar(meno,"et")) strcat(dopadnute,"e");
	  else if (lastchar(meno,"kus")) strcpy(dopadnute,striend(meno,"kus","kovi"));
	  else if (lastchar(meno,"seus")) strcpy(dopadnute,striend(meno,"seus","seovi"));
	  else strcat(dopadnute,"ovi");
	  break;
  case 7: if (lastchar(meno,"ter")) strcpy(dopadnute,striend(meno,"ter","trom"));
	  else if (lastchar(meno,"ay")) strcat(dopadnute,"om");
	  else if (lastchar(meno,"ey")) strcat(dopadnute,"om");
	  else if (lastchar(meno,"oy")) strcat(dopadnute,"om");
	  else if (lastchar(meno,"y")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"ie")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"i")) strcat(dopadnute,"m");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","om"));
	  else if (lastchar(meno,"ok")) strcpy(dopadnute,striend(meno,"ok","kom"));
	  else if (lastchar(meno,"oj")) strcat(dopadnute,"om");
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","om"));
	  else if (lastchar(meno,"ec")) strcpy(dopadnute,striend(meno,"ec","com"));
/*	  else if (lastchar(meno,"es")) strcpy(dopadnute,striend(meno,"es","om")); */ /*!!! som*/
	  else if (lastchar(meno,"et")) strcat(dopadnute,"om");
	  else if (lastchar(meno,"kus")) strcpy(dopadnute,striend(meno,"kus","kom"));
	  else if (lastchar(meno,"seus")) strcpy(dopadnute,striend(meno,"seus","seom"));
	  else strcat(dopadnute,"om");
	  break;
 }
else switch (pad) /* F */
 {
  case 1: break;
  case 2: if (lastchar(meno,"za")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ca")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ja")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ea")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ia")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"oa")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ua")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","y"));
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","a"));
	  break;
  case 4: if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","u"));
	  break;
  case 3: if (lastchar(meno,"za")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ca")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ja")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ea")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ia")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"oa")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ua")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"ao")) strcpy(dopadnute,striend(meno,"o","e"));
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","u"));
	  break;
  case 6: if (lastchar(meno,"za")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ca")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ja")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ea")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ia")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"oa")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"ua")) strcpy(dopadnute,striend(meno,"a","i"));
	  else if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","e"));
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","u"));
	  break;
  case 7: if (lastchar(meno,"a")) strcpy(dopadnute,striend(meno,"a","ou"));
	  else if (lastchar(meno,"ao")) strcat(dopadnute,"u");
	  else if (lastchar(meno,"o")) strcpy(dopadnute,striend(meno,"o","om"));
	  break;
 }
return dopadnute;
}

int je_samohlaska(char znak)
{
 if (znak=='a' || znak=='e' || znak=='i' || znak=='y' || znak=='o' || znak=='u') 
  return 1;
return 0;
}


void free_notifylist(NOTIFY *list) { 
	NOTIFY bunka, next;

	bunka = *list;
	while (bunka) {
		next=bunka->next;
		free((void *) bunka);
		bunka=next;
	} 
	*list=NULL;
} 

/*
int save_notifylist(NOTIFY *list, char *filename) { 
	FILE *fp;	
	NOTIFY bunka;

	if (!(fp=ropen(filename,"w"))) {
		sprintf(text,"Nemozem otvorit subor <%255s> na zapis (save_notifylist())",filename);
		write_syslog(text,1);
		return FAIL;
	}
	
	for (bunka=*list; bunka!=NULL; bunka=bunka->next) {
		sprintf(text,"%s\n",bunka->meno);
		fputs(text,fp);
	}
	fclose(fp);
	return SUCCESS;
} 
*/

int load_notifylist(NOTIFY *list,int id,int noti)
{
int pocet=0;

  sprintf(query,"select %s from %s%s%s where `userid`='%d' %s;",(noti<2)?"`victim`":"`cmd`",noti==1?"`notify`":"",noti==0?"`ignuser`":"",noti==2?"`comban`":"",id,(noti<2)?"order by `victim` desc":"");
  if ((result=mysql_result(query))) {
    while ((row=mysql_fetch_row(result))) {
      if (row[0]) {
        pocet++;
        add_notify(list,row[0]);
       }
     }
    mysql_free_result(result);
   }
 return pocet;
}

int delete_notify(NOTIFY *list, char *meno) {
	NOTIFY bunka, prev;
	prev=NULL;

	for (bunka=*list; bunka!=NULL; bunka=bunka->next) {
		if (!strcasecmp(bunka->meno,meno) )  {
			if (prev) 
				prev->next=bunka->next;
			else
				*list=bunka->next;
			free(bunka);		
			return 1;
		}
		prev=bunka;     
	} 
return 0;
} 

void add_notify(NOTIFY *list, char *name) {
	NOTIFY bunka;
	
	if (!(bunka = (struct notify_struct *) malloc(sizeof(struct notify_struct)))) {
		write_syslog("CHYBA: Nedostatok pamate vo funkcii add_notify",1);
		return;
	}

	strcpy(bunka->meno,name);

	bunka->next = *list;
	*list = bunka;
} 

void zrus_pager_haldu(UR_OBJECT user) {
PAGER pom;

while (user->pager!=NULL) {
	pom=user->pager;
	user->pager=user->pager->next;
	free((void *)pom);
	}
}

char *expand_password(char *pass)
{
static char vysledok[20];

sprintf(vysledok,"&%-10s*2=h", pass); /* NEMENIT!!!!!!!!!!!!!!!!!!! */
return vysledok;
}

int check_passwd_simplex(char *passwd)
{
FILE *fp;
char heslo[50];
char *tmp;

if (strlen(passwd)<6) return 1;

if (!(fp=ropen(DICT,"r"))) return 0; /*APPROVED*/

strtolower(passwd);
fscanf(fp, "%s", heslo);
strtolower(heslo);
while (!feof(fp)) {
	if (strstr(passwd, heslo)) {
		fclose(fp);
		return 1;
		}
	fscanf(fp, "%s", heslo);
	strtolower(heslo);
	}
fclose(fp);

tmp=passwd;

while(*tmp!='\0') if (!isalpha(*tmp++)) return 0;

return 1;
}

int skontroluj()
{
FILE *fp;
char slovo[50];

if (!(fp=ropen("misc/kontrola","r"))) return 0; /*APPROVED*/
fscanf(fp,"%s",slovo);
fclose(fp);
if (strlen(slovo)>1) return 1;
return 0;
}

void aklient_log(char *meno)
{
FILE *fp, *tempfp;
char tempfilename[80];
char juzer[15];
char verzia[20];
char ver[110];
int pristupy;
int je;

sprintf(tempfilename,"log/aklient_log_users.tmp");
if (!(fp=ropen(AKLIENT_LOG_USERS,"r"))) { /*APPROVED*/
	if (!(fp=ropen(AKLIENT_LOG_USERS,"w"))) return;
	fprintf(fp,"%s 1\n", meno);
	fclose(fp);	
	}
else {
	if (!(tempfp=ropen(tempfilename,"w"))) { /*APPROVED*/
		fclose(fp);
		return;
		}
	je=0;	
	fscanf(fp,"%s %d", juzer, &pristupy);
	while(!feof(fp)) {
		if (!strcmp(juzer, meno)) { 
			pristupy++;
			je=1;
			}
		fprintf(tempfp, "%s %d\n", juzer, pristupy);
		fscanf(fp,"%s %d", juzer, &pristupy);
		}	
	if (!je) fprintf(tempfp, "%s 1\n", meno);
	fclose(fp);
	fclose(tempfp);
	rename(tempfilename, AKLIENT_LOG_USERS);
	}

strcpy(ver,word[6]);
/* MILOOOOO! :> */
if (!strcmp(word[6],"v")) strcpy(ver,word[7]);
if (!strcmp(word[6],"2000") && strcmp(word[7],"official")) sprintf(ver,"2000b%s", word[8]);
if (!strcmp(word[6],"2000") && !strcmp(word[7],"official")) strcpy(ver,"2000");
/****************/

/* LOGNUTIE VERZIE */
sprintf(tempfilename,"log/aklient_log_ver.tmp");
if (!(fp=ropen(AKLIENT_LOG_VER,"r"))) { /*APPROVED*/
	if (!(fp=ropen(AKLIENT_LOG_VER,"w"))) return;
	fprintf(fp,"%s 1\n", ver);
	fclose(fp);
	}
else {
	if (!(tempfp=ropen(tempfilename,"w"))) { /*APPROVED*/
		fclose(fp);
		return;
		}
	je=0;
	fscanf(fp,"%s %d", verzia, &pristupy);
	while(!feof(fp)) {
		if (!strcmp(verzia, ver)) {
		pristupy++;
		je=1;
		}
		fprintf(tempfp, "%s %d\n", verzia, pristupy);
		fscanf(fp,"%s %d", verzia, &pristupy);
		}	
	if (!je) fprintf(tempfp,"%s 1\n", ver);
	fclose(fp);
	fclose(tempfp);
	rename(tempfilename, AKLIENT_LOG_VER);
	}

return;
}

/* Zoznam userov hod do suboru ... */
void who_for_web()
{
UR_OBJECT u;
int mins,idle;
char pohl;

FILE *fp;

if (disable_web) return;
if ((fp=ropen(WHO_FOR_WEB,"w"))==NULL) { /*APPROVED*/	 
	return;
	}

for(u=user_first;u!=NULL;u=u->next) { 
      if (u->type!=USER_TYPE || u->login || u->room==NULL) continue;
      mins=(int)(time(0) - u->last_login)/60;
      idle=(int)(time(0) - u->last_input)/60; if (!u->vis) {
            continue;
            }
            
      if (u->sex==0) pohl='Z';
      	else if (u->sex==1) pohl='M';
      		else pohl='?';
      		
      fprintf(fp,"%s %s %s %d %d %c\n", u->name, u->room->name, level_name[u->level], mins, idle, pohl);
      fprintf(fp,"%s\n", u->desc);
    }
    
fclose(fp);
}

char *zobraz_datum(time_t *raw, int typ)
{
struct tm *tm_struct;
static char vystup[50];

vystup[0]='\0';
tm_struct=localtime(raw);

if (typ==1) sprintf(vystup,"%d. %s %d, %02d:%02d (%s)", tm_struct->tm_mday, month[tm_struct->tm_mon], 1900+tm_struct->tm_year, tm_struct->tm_hour, tm_struct->tm_min, day[tm_struct->tm_wday]);
if (typ==2) sprintf(vystup,"%02d:%02d", tm_struct->tm_hour, tm_struct->tm_min);
if (typ==3) sprintf(vystup,"%d.%d.%d, %02d:%02d (%s)", tm_struct->tm_mday, tm_struct->tm_mon+1, 1900+tm_struct->tm_year, tm_struct->tm_hour, tm_struct->tm_min, day[tm_struct->tm_wday]);
if (typ==4) sprintf(vystup,"%d.%d.%d", tm_struct->tm_mday, tm_struct->tm_mon+1, 1900+tm_struct->tm_year);
if (typ==5) sprintf(vystup,"%d.%d.%d %02d:%02d", tm_struct->tm_mday, tm_struct->tm_mon+1, 1900+tm_struct->tm_year, tm_struct->tm_hour, tm_struct->tm_min);
return vystup;
}


int jdb_zarad(int co, int uid, int wizzid, char *dovod)
{

 if (uid<1) return 0;
 sprintf(query,"replace into `outlaws` (`userid`,`wizzid`,`type`,`reason`) values ('%d','%d','%d','%s')",uid,wizzid,co,dovod);
 mysql_kvery(query);
 if (mysql_affected_rows(&mysql)==1) return 1;
 else return 0;

}

int jdb_vyrad(int co, int uid)
{
 if (uid<0) return 0;
 sprintf(query,"delete from `outlaws` where `userid`='%d' and type='%d'",uid,co);
 mysql_kvery(query);
 if (mysql_affected_rows(&mysql)==1) return 1;
 else return 0;

}

char *jdb_info(int co, char *username)
{
int uid=0;

uid=db_userid(username);
if (uid<1) return NULL;

sprintf(query,"select `reason` from `outlaws` where `userid`='%d' and type='%d'",uid,co);
if ((result=mysql_result(query))) {
  if ((row=mysql_fetch_row(result))) { 
    strncpy(texthb,row[0],sizeof(texthb)-1);
    mysql_free_result(result);
    return texthb;
   }
  mysql_free_result(result);
 }
return NULL;
}

char *jdb_wizz(int co, char *username)
{
int uid=0;
static char uname[USER_NAME_LEN+2];

uname[0]='\0';
uid=db_userid(username);
if (uid<1) return uname;

sprintf(query,"select `wizzid` from `outlaws` where `userid`='%d' and type='%d'",uid,co);
if ((result=mysql_result(query))) {
  if ((row=mysql_fetch_row(result)) && row[0]) { 
    uid=atoi(row[0]);
   }
  mysql_free_result(result);
 }
if (uid>0) strcpy(uname,db_username_by_id(uid));
return uname;
}

long filesize(char *path)
{
 struct stat statbuf;
 if (!stat(path,&statbuf)) return statbuf.st_size;
 else return -1;
}   

FILE *ropen (const char *path, const char *mode)
{
 struct statfs diskstat;
 if (strpbrk(mode,"aw+")) { /* kontroluj iba zapis */
    if (statfs(FILESYSTEM, &diskstat)) return NULL;
    if (diskstat.f_bavail<LOW_DISK_LIMIT) return NULL;
 }
 return (fopen(path, mode));
}

void send_sms(char *tonum,char *str,int gate)
{
char comd[500],account[100],mesg[500];
time_t tim;
FILE *fp;
unsigned int pos,max;

 if (!strcmp(tonum,"-")) return;
 strcpy(account,expand_gate(NULL,1,tonum));
 if (!account[0]) return;
 max=get_gate_maxchars(gate);
 strncpy(mesg,str,max);
 mesg[max]='\0';
 sprintf(comd,"smssend %s \"%s\" >/dev/null",account,mesg);
 pos=0;
 while (pos<strlen(comd)) {
   if(comd[pos]=='\n') comd[pos]=' '; 
   pos++;
  }
 if ((fp=ropen("log/sentsms","a"))) { /*APPROVED*/
   time (&tim);
   fprintf(fp,"%s %-12s %s\n",zobraz_datum(&tim,5),"<SYSTEM>",tonum);
/*   fprintf(fp,"%s\n",comd); */
   fclose(fp);
  }

 switch(double_fork()) {
   case -1 :  return;
   case  0 : system(comd);
	      _exit(1);
  }
}

void sms(UR_OBJECT user,int done_editing)
{
FILE *fp;
char cmd[1000],*c,filename[81],account[100];
time_t tim;
int /* ch, */oflajn=0,pos,i;
UR_OBJECT u;
/* char *help="~FTPouzi: ~FW.sms <cislo v medzinarodnom tvare>  ~FTalebo: ~FW.sms <uzivatel>\n~FTSprava sa pise v editore, po kazdom riadku sa zobrazuje zvysny pocet znakov.\n~FTAk chces zmenit branu cez ktoru sa budu posielat SMS spravy, pouzi: ~FW.sms brana\n"; */
char *help="~FTPouzi: ~FW.sms <cislo v medzinarodnom tvare>  ~FTalebo: ~FW.sms <uzivatel>\n~FTSprava sa pise v editore, po kazdom riadku sa zobrazuje zvysny pocet znakov.\n";

if (done_editing==1) {
  strcpy(account,expand_gate(user,1,user->mail_to));
  sprintf(cmd,"smssend %s '<%s> ",account,user->name);
  pos=strlen(cmd);
  c=user->malloc_start;
  while(c!=user->malloc_end) { 
    if (*c=='\n') cmd[pos]=' ';
    else if (*c=='\'' || *c=='"') cmd[pos]='`';
    else cmd[pos]=*c;
    /* ch= *c++; */
    pos++;
   }
  cmd[pos]='\0';
  sprintf(texthb,"' >mailspool/%s.sms",user->name);
  strcat(cmd,texthb);
  user->smswait=1;
  sprintf(text,"~FTPosielam SMS spravu..\n");
  write_user(user,text);
  user->smsssent++;
  sprintf(text,"~FTDnes si uz poslal%s %d SMS %s. (denny limit je %d %s)\n",pohl(user,"","a"),user->smsssent,skloncislo(user->smsssent,"spravu","spravy","sprav"),max_sms,skloncislo(max_sms,"sprava","spravy","sprav"));
  if (max_sms>0) write_user(user,text);
  if ((fp=ropen("log/sentsms","a"))) { /*APPROVED*/
    time (&tim);
    fprintf(fp,"%s %-12s %s\n",zobraz_datum(&tim,5),user->name,user->mail_to);
    fclose(fp);
   }
  user->mail_to[0]='\0';
  user->smschars=0;

  switch(double_fork()) {
    case -1 : sprintf(text,"~OL~FWSYSTEM: ~FRChyba v trojzubci pri posielani sms (user %s)!\n", user->name);
    	      writesys(KIN,1,text,NULL);
              return;
    case  0 : system(cmd);
	      _exit(1);
    }
  return;
 }
if (done_editing==2) {
  pos=0;
  sprintf(filename,"mailspool/%s.sms",user->name);
  if ((fp=ropen(filename,"r"))) { /*APPROVED*/
    fgets(cmd,500,fp);
    i=0;
    while(!feof(fp)) {
      if (i==0) write_user(user,"\n");
      /* if (strstr(cmd,"Copyright(c) Ze KiLleR")) pos=10; */
      if (pos==0) {
        sprintf(text,"~OL%s",cmd);
        write_user(user,text);
       }
      else pos--;
      fgets(cmd,500,fp);
      i++;
     }
    fclose(fp);
    unlink(filename);
   }
  return;
 }

/*if (!(strcmp(word[1],"gate")) || !(strcmp(word[1],"brana"))) {
  if (word_count==2) {
    if (user->smsgate<0) user->smsgate=0;
    strcpy(texthb,expand_gate(user,0,""));
    if (texthb[0]) {
      show_gates(user);
      sprintf(text,"~FTMas nastavenu SMS branu: ~FW~OL%s~RS.\n",texthb);
      write_user(user,text);
     }
    else write_user(user,"~FTZ technickych pricin nie su k dispozicii ziadne brany.\n");
    return;
   }
  i=get_gate_no(word[2]);
  if (i==-1) {
    write_user(user,"~FTTakato brana nie je k dispozicii.\n");
    show_gates(user);
    return;
   }
  user->smsgate=i;
  sprintf(text,"~FTTvoje SMS spravy budu odteraz odosielane cez branu: ~FW~OL%s~RS.\n",word[2]);
  write_user(user,text);
  return;
 }
*/

if (user->smsssent<0) user->smsssent=0;
time(&tim);
if (user->smsday!=tim/86400) {
  user->smsday=tim/86400;
  user->smsssent=0;
 }
if (max_sms>0 && user->smsssent>=max_sms) {
  sprintf(text,"Uz si dosiah%s denny limit %d SMS %s.\n",pohl(user,"ol","la"),max_sms,skloncislo(max_sms,"spravu","spravy","sprav"));
  write_user(user,text);
  return;
 }
if (word_count<2) { 
  write_user(user,help);
  if (user->smsssent>0) {
    sprintf(text,"Dnes si uz poslal%s %d SMS %s.",pohl(user,"","a"),user->smsssent,skloncislo(user->smsssent,"spravu","spravy","sprav"));
    if (max_sms==0) strcat(text,"\n");
    else { 
      sprintf(texthb,"(denny limit je %d %s)\n",max_sms,skloncislo(max_sms,"sprava","spravy","sprav"));
      strcat(text,texthb);
     }
    write_user(user,text);
   }
  return;
 }
if (word_count>2) strcat(word[1],word[2]);

if ((u=get_user(word[1]))==NULL) {
  if ((u=create_user())==NULL) {
    sprintf(text,"%s: nemozno vytvorit docsasny user object.\n",syserror);
    write_user(user,text);
    write_syslog("CHYBA: Nemozno vytvorit docasny user object v sms().\n",0);
    return;
   }
  sstrncpy(u->name,word[1],12);
  if (!load_user_details(u)) {
    destruct_user(u);
    destructed=0;
    u=NULL;
   }
  else {
/*    sprintf(filename,"%s/%s.notify", USERFILES, u->name); */
    u->notifylist=NULL;
    load_notifylist(&(u->notifylist),u->id,1);
/*    sprintf(filename,"%s/%s.ign", USERFILES, u->name); */
    u->ignorelist=NULL;
    load_notifylist(&(u->ignorelist),u->id,0);
    oflajn=1;
   } 
 }
else oflajn=0;

if (u!=NULL) {
  if (!strcmp(u->mobile,"-")) {
    sprintf(text,"%s nema nastavene cislo mobilu.\n",u->name);
    write_user(user,text);
    if (oflajn==1) {
      free_notifylist(&(u->ignorelist));
      free_notifylist(&(u->notifylist));
      destruct_user(u);
      destructed=0;
     }
    return;
   }
  if (u->allowsms==3 || (u->allowsms==2 && check_ignore_user(user,u))
  || (u->allowsms==1 && !check_notify_user(u,user))) {
    sprintf(text,"%s nema pristupne cislo na mobil.\n",u->name);
    write_user(user,text);
    if (oflajn==1) {
      free_notifylist(&(u->ignorelist));
      free_notifylist(&(u->notifylist));
      destruct_user(u);
      destructed=0;
     }
    return;  
   }
  strcpy(user->mail_to,u->mobile);
/*  sprintf(text,"~FWSMS sprava ~OL%s~RS cez branu %s",sklonuj(u,3),expand_gate(user,0,"")); */
  sprintf(text,"~FWSMS sprava ~OL%s~RS",sklonuj(u,3));
  if (oflajn==1) {
    free_notifylist(&(u->ignorelist));
    free_notifylist(&(u->notifylist));
    destruct_user(u);
    destructed=0;
   }
 }
else {
  if (strlen(word[1])!=10 || !is_number(word[1]) || strncmp(word[1],"090",3)) {
    strcpy(filename,word[1]);
    filename[0]='0';
    if (word[1][0]!='+' || !is_number(filename) || strlen(word[1])<10 || strlen(word[1])>15) {
      write_user(user,help);
      write_user(user,"Cislo musi byt v tvare 0903123456 alebo v medzinarodnom tvare +421903123456.\n");
      return;
     }
    strcpy(user->mail_to,word[1]);
   }
  else {
    strcpy(filename,"+42");
    word[1][0]='1';
    strcat(filename,word[1]);
    strcpy(word[1],filename);
    strcpy(user->mail_to,filename);
   }
/*  sprintf(text,"~FWSMS sprava na cislo ~OL%s~RS cez branu %s",user->mail_to,expand_gate(user,0,"")); */
  sprintf(text,"~FWSMS sprava na cislo ~OL%s~RS",user->mail_to);
 }
user->smschars=get_gate_maxchars(user->smsgate);
if (restrict_sms(user->mail_to,user->smsgate)) {
  user->mail_to[0]='\0';
  write_user(user,"Na dane cislo sa cez zvolenu branu neda poslat sprava, skus zmenit branu.\n");
  return;
 }
write_user(user,title(text,"~FG"));
user->misc_op=20;
editor(user,NULL);
return;

}

char *title(char *str,char *col)
{
char text2[ARR_SIZE+1];
static char vtext[200];
int i/* ,cnt */,len,spc,odd;

strcpy(vtext,str);
/* cnt=colour_com_count(vtext); */
len=strlen(colour_com_strip2(vtext,1));
spc=(39-len/2);
odd=(spc+spc+len-78);
sprintf(text2,"%*.*s~RS %s",spc,spc," ",vtext);
for(i=0;i<40 && text2[i]==' ';++i) if (i%2==0) text2[i]='-'; else text2[i]='.';
sprintf(vtext,"%s~RS%s%*.*s",text2,col,spc-odd,spc-odd," ");
for(i=strlen(vtext)-1;vtext[i-1]==' ';--i) if (i%2==len%2) vtext[i]='-'; else vtext[i]='.';
sprintf(text2,"%s%s\n",col,vtext);
strcpy(vtext,text2);
/* if (strlen(vtext)-colour_com_count(vtext)<79) strcat(vtext,".-"); */
return(vtext);
}

void statistic(user)
UR_OBJECT user;
{
UR_OBJECT u;
int oflajn,i,avg,days2,hours2,mins2/* ,vel */;
float po[11],zvy;
FILE *fp;
char filename[81],komand[20],kto[USER_NAME_LEN+5];
char tmp[100],*temp;

oflajn=0;
if (word_count<2)
 {
  u=user;
  sprintf(text,"%s",title("~OL~FGTvoj status","~FG"));
  write_user(user,text);
 }
else 
 {
  if (!(u=get_user(word[1]))) {
    if ((u=create_user())==NULL) {
      write_syslog("ERROR: Unable to create temporary user object in modify().\n",0);
      return;
     }
    strcpy(u->name,word[1]);
    if (!load_user_details(u)) {
      write_user(user,nosuchuser);  
      destruct_user(u);
      destructed=0;
      return;
     }
    oflajn=1;
   }
  sprintf(text,"~OL~FGStatus %s",sklonuj(u,2));
  write_user(user,title(text,"~FG"));
 }
if (oflajn==0 && !strcmp(word[2],"timeouts")) {
  sprintf(text,"~OL~FRTimeouts: ~FW~OL%d~RS/~OL~FR%d\n",u->hangups,u->hangupz);
  write_user(user,text);
  return;
 }
for (i=1;i<11;++i) if (u->commused[i]==-1) u->commused[i]=0;
if (u->commused[0]<1)
 {
  u->commused[0]=1; /* aby sme nedelili nulou ;P (V) */
 }
zvy=100;
for (i=1;i<11;++i) if (i!=9) { /* [9] je pocet disconnectov */
 po[i]=(((float)u->commused[i]*100)/u->commused[0]);
 zvy-=(((float)u->commused[i]*100)/u->commused[0]);
}
sprintf(text,"~FT Examine:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n~FT     Who:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n~FT    Tell:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT) (tell, pemote)\n~FT   Shout:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT) (shout, semote)\n",po[6],u->commused[6],po[5],u->commused[5],po[1],u->commused[1],po[3],u->commused[3]);
write_user(user,text);
sprintf(text,"~FT    Look:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n~FT      Go:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n~FT  Gossip:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n~FT     Say:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT) (say, to, think, sing, emote)\n",po[7],u->commused[7],po[8],u->commused[8],po[4],u->commused[4],po[2],u->commused[2]);
write_user(user,text);
if (u->level>=KIN) { 
  sprintf(text,"~FTWizshout:~RS ~OL%5.1f~RS ~FT%% (~FW%7d~FT)\n",po[10],u->commused[10]);
  write_user(user,text);
 }
sprintf(text,"~FGOstatne:~RS  ~OL%5.1f~RS ~FT%% ",zvy);
if (zvy>=0) write_user(user,text);
sprintf(text,"~FGCelkovo:~RS ~OL%d ~RS~FGprikazov\n",u->commused[0]);
write_user(user,text);

days2=((u->totaljailtime+u->nontimejails)*ZALAR_TLT_CONSUMPTION)/86400;  
hours2=(((u->totaljailtime+u->nontimejails)*ZALAR_TLT_CONSUMPTION)%86400)/3600;  
mins2=(((u->totaljailtime+u->nontimejails)*ZALAR_TLT_CONSUMPTION)%3600)/60;  

sprintf(text,"~FGZalar na urcito/neurcito: ~OL~FW%d~RS~FG/~OL~FW%d~RS~FG min. Zhltnuty TLT: ~OL~FW%d~RS~FG dni ~OL~FW%d~RS~FG hod ~OL~FW%d~RS~FG min\n",u->totaljailtime/60,u->nontimejails/60,days2,hours2,mins2);
write_user(user,text);
sprintf(text,"~FGCas straveny muzzled: ~OL~FW%d~RS~FG min. Pocet killnuti: ~OL~FW%d~RS~FG\n",u->totalmuzzletime/60,u->killed);
write_user(user,text);
if (u->first_login>0)
 {
/* z tohto uz sa vypocita selico..priemerne visenie denne, mesacne .. etc (V) */
  sprintf(text,"~FYPrve prihlasenie: ~FW~OL%s\n",zobraz_datum((time_t *)&u->first_login, 3));
  write_user(user,text);
  if (u->commused[9]<1) u->commused[9]=1; /* pre istotu */
  avg=((u->total_login)/60)/u->commused[9];
  sprintf(text,"~FYPriemerny TLT za prihlasenie: ~FW~OL%d~RS~FY minut. (celkovo ~FW%d~FY prihlaseni)\n",avg,u->commused[9]);
  write_user(user,text);
 } 
else {
  sprintf(text,"~FYPocet prihlaseni od novembra 00: ~FW%d~FW\n",u->commused[9]);
  write_user(user,text);
 }
sprintf(text,"~FYNajdlhsia session: ~FW~OL%d~RS~FY hodin ~FW~OL%d~RS~FY minut.\n",u->longestsession/3600,(u->longestsession%3600)/60);
write_user(user,text);

if (u->shoutswears<0) u->shoutswears=0;
if (u->sayswears<0) u->sayswears=0;
sprintf(text,"~FRPocet vybodkovanych nadavok shout/say: ~FW%d~FR / ~FW%d~FR.\n",u->shoutswears,u->sayswears);
write_user(user,text);

if (u->prevname[0]) {
  if (!strcmp(u->prevname,"x")) {
    sprintf(text,"~FT%s uz bol%s.\n",u->name,pohl(u," premenovany aj vrateny","a premenovana aj vratena"));
    write_user(user,text);
   }
  else {
    sprintf(text,"~FT%s bol%s. Povodne meno: %s.\n",u->name,pohl(u," premenovany","a premenovana"),u->prevname);
    write_user(user,text);
   }
 }

i=0;/* vel=0; */
sprintf(query,"select `cmd`,`banner`,`reason` from `comban` where `userid`='%d';",u->id);
if ((result=mysql_result(query))) {
  while ((row=mysql_fetch_row(result))) {
    if (i==0) {
      sprintf(text,"~OL~FROdobrate prikazy:\n");
      write_user(user,text);
     }
    i++;
    sprintf(text,"~OL~FW%s ~FR(%s:~RS~FW %s~OL~FR)\n",(row[0])?row[0]:"",(row[1])?row[1]:"",(row[2])?row[2]:"");
    write_user(user,text);
   }
  mysql_free_result(result);
 }

if ((temp=jdb_info(DB_JAIL,u->name))!=NULL) {
  strcpy(kto,jdb_wizz(DB_JAIL,u->name));
  sprintf(text,"~OL~FRUzivatela zazalaroval/a: ~FW%s~FR (dovod: ~FW%s~FR)\n",kto,temp);
  write_user(user,text);
 }
temp=jdb_info(DB_BAN,u->name);
if (temp!=NULL) {
  strcpy(kto,jdb_wizz(DB_BAN,u->name));
  sprintf(text,"~OL~FRUzivatela vybanoval/a:   ~FW%s~FR (dovod: ~FW%s~FR)\n",kto,temp);
  write_user(user,text);
 }
temp=jdb_info(DB_DEMOTE,u->name);
if (temp!=NULL) {
  strcpy(kto,jdb_wizz(DB_DEMOTE,u->name));
  sprintf(text,"~OL~FRUzivatela demotol/la:    ~FW%s~FR (dovod: ~FW%s~FR)\n",kto,temp);
  write_user(user,text);
 }
sprintf(filename,"%s",MULTI_REQUEST);
if (!(fp=ropen(filename,"r"))) { } /*APPROVED*/
  else {
  fscanf(fp,"%s %s %s %s",filename,kto,komand,tmp);
  while(!feof(fp)) {
    if ((!strcmp(kto,u->name))) {
      sprintf(text,"~FR%s pokus o request na email usera ~OL%s~RS~FR (%s).\n",filename,komand,tmp);
      write_user(user,text);
     }
    fscanf(fp,"%s %s %s %s",filename,kto,komand,tmp);
   }
  fclose(fp);
 }

write_user(user,"~FG-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-\n");
if (oflajn==1)
 {
/*  u->socket=-2; */
/*  strcpy(u->site,u->last_site); */
/*  save_user_details(u,0); */
  destruct_user(u);
  destructed=0;
 }
}

void eject(UR_OBJECT user)
{
RM_OBJECT letun;
UR_OBJECT u;
if (flyer.pozicia) /* FLYER */
   if (!strcmp(user->name,flyer.pilot))
      {
       letun=get_room(FLYER_ROOM,NULL);
       for(u=user_first; u!=NULL; u=u->next)
          {
           if (u==user || u->login || u->room==NULL || u->type!=USER_TYPE) continue;
           if (u->room==letun) break;
          }
       if (u==NULL)
          {
	   flyer.pozicia=0;
           write_room(get_room(FLYER_HOME,NULL),"~OL~FGPrave priletel dalsi letun! Nechces nastupit?\n");
           link_room(letun,get_room(FLYER_HOME,NULL));
           letun->group=2;
          }
       else 
          {
           sprintf(text,"~OL~FG%s %s letun a ponechal ho bez dozoru!\n",user->name,pohl(user,"opustil","opustila"));
           write_room(letun,text);
           sprintf(text,"~OL~FGNajblizsie ku kormidle %s %s a hned sa %s k riadeniu...\n",pohl(u,"bol","bola"),u->name,pohl(u,"vrhol","vrhla"));
           write_room(letun,text);
           strcpy(flyer.pilot, u->name);
           flyer.gotta_write=0;
           write_user(u,"~OL~FGRiadenie letuna teraz ostalo iba na tebe!\n");
          }
      }
}

void alter_maxtimeouts(int do_what)
{
FILE *fp;
char filename[80];
int value,value2,max;

sprintf(filename,"misc/timeouts.max");
if (do_what==0) {
  if (!(fp=ropen(filename,"r"))) {
    if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
      return;
     }
    fprintf(fp,"%d %d %d\n",max_timeouts,max_timeoutz,max_sms);
    fclose(fp);
    return;
   }			
  fscanf(fp,"%d %d %d",&value,&value2,&max);
  fclose(fp);
  if (value>0 && value<1001) max_timeouts=value;
  if (value2>0 && value2<101) max_timeoutz=value2;
  if (-1<max) max_sms=max;
 }

if (do_what==1) {
  if (!(fp=ropen(filename,"w"))) { /*APPROVED*/
    return;
   }
  fprintf(fp,"%d %d %d\n",max_timeouts,max_timeoutz,max_sms);
  fclose(fp);
  return;
 }
}

char *noyes(int numero)
{
 if (numero==0) return no_or_yes[0];
 return no_or_yes[1];
}

int get_gate_no(char *str)
{
char filename[81],line[100];
FILE *fp;
int i,num;
char name[20];

sprintf(filename,"misc/smsgate");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
  return -1;
 }
fgets(line,99,fp);
i=0;
while(!feof(fp)) {
  sscanf(line,"%d %s",&num,name);
  if (!strcmp(str,name)) {
    fclose(fp);
    return num;
   }
  i++;
  fgets(line,99,fp);
 }
fclose(fp);
return -1;
}

int get_gate_maxchars(int gate_num)
{
char filename[81],line[100];
FILE *fp;
int i,num,max;
char name[20];

sprintf(filename,"misc/smsgate");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
  return -1;
 }
fgets(line,99,fp);
i=0;
while(!feof(fp)) {
  sscanf(line,"%d %s %d",&num,name,&max);
  if (num==gate_num) {
    fclose(fp);
    return max;
   }
  i++;
  fgets(line,99,fp);
 }
fclose(fp);
return -1;
}

char *expand_gate(UR_OBJECT user,int inc_account,char *sendtonum)
{
char filename[81],line[100],uname[30],pass[30],name[20],first[60];
FILE *fp;
int i,num,max,format,gateno=0;
static char ret[20];
char country[10],provider[10],numero[10],tmp1[3],tmp2[3],tmp3[3],wholenum[25];
char *number;

/* if (user!=NULL) gateno=user->smsgate; */
if (user==NULL) gateno=0;
else gateno=smscycle;

ret[0]='\0';
sprintf(filename,"misc/smsgate");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
  return ret;
 }
fgets(line,99,fp);
i=0;
while(!feof(fp)) {
  sscanf(line,"%d %s %d %d %s %s",&num,name,&max,&format,uname,pass);
  if (num==gateno || i==1) {
    if (inc_account) {
      if (uname[0]=='-' || pass[0]=='-') {
        uname[0]='\0';
        pass[0]='\0';
       }
      else {
        strcat(uname," ");
        strcat(pass," ");
       }
      strcpy(wholenum,sendtonum);
      number=wholenum;
      strncpy(country,number+1,3);
      country[3]='\0';
      strncpy(provider,number+4,3);
      provider[3]='\0';
      strncpy(numero,number+7,6);
      numero[6]='\0';
      tmp1[0]='\0';
      tmp2[0]='\0';
      tmp3[0]='\0';
      if (!(format & 1)) strcat(tmp1,"+");
      if (format & 2) strcat(tmp1,"0");
      if (format & 4) strcat(tmp2," ");
      if (format & 8) strcat(tmp2,"0");
      if (format & 16) strcat(tmp3," ");
      if (format & 32) sprintf(ret,"%s %s%s%s%s%s%s",name,uname,pass,tmp2,provider,tmp3,numero);
      else sprintf(ret,"%s %s%s%s%s%s%s%s%s",name,uname,pass,tmp1,country,tmp2,provider,tmp3,numero);
     }
    else strcpy(ret,name);
    if (num==gateno) {
      fclose(fp);
      if (gateno>0) smscycle++;
      return ret;
     }
    if (i==1) strcpy(first,ret);
   }
  i++;
  fgets(line,99,fp);
 }
fclose(fp);
strcpy(ret,first);
smscycle=2;
if (user!=NULL) user->smsgate=0;
return ret;
}

void show_gates(UR_OBJECT user)
{
char filename[81],line[100];
FILE *fp;
int i,num,max,ii,format;
static char name[20],uname[20],pass[20];
char myrestrict[3][15];
char *error="Z technickych pricin nie je k dispozicii ziadna brana.\n";

name[0]='\0';
sprintf(filename,"misc/smsgate");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
  write_user(user,error);
  return;
 }
fgets(line,99,fp);
i=0;
sprintf(text,"~FTK dispozicii su nasledovne brany:\n");
while(!feof(fp)) {
  for(ii=0;ii<3;ii++) myrestrict[ii][0]='\0';
  sscanf(line,"%d %s %d %d %s %s %s %s %s",&num,name,&max,&format,uname,pass,myrestrict[0],myrestrict[1],myrestrict[2]);
  if (myrestrict[0][0]) sprintf(filename,"  %-13s~FT(max. %d znakov) obmedzena na cisla %s %s %s\n",name,max,myrestrict[0],myrestrict[1],myrestrict[2]);
  else sprintf(filename,"  %-13s~FT(max. %d znakov)\n",name,max);
  strcat(text,filename);
  i++;
  fgets(line,99,fp);
 }
fclose(fp);
if (i==0) write_user(user,error);
else write_user(user,text);
}

int restrict_sms(char *str,int gate_num)
{
char filename[81],line[100];
FILE *fp;
int num,max,ok[2],ii,format;
unsigned int i;
char name[20];
char myrestrict[2][15];

sprintf(filename,"misc/smsgate");
if (!(fp=ropen(filename,"r"))) { /*APPROVED*/
  return 1;
 }
fgets(line,99,fp);
while(!feof(fp)) {
  myrestrict[0][0]='\0';
  myrestrict[1][0]='\0';
  sscanf(line,"%d %s %d %d %s %s %s %s",&num,name,&max,&format,name,name,myrestrict[0],myrestrict[1]);
  if (num==gate_num) {
    ok[0]=1;ok[1]=1;max=0;
    for(ii=0;ii<2;ii++) if (myrestrict[ii][0]) {
      max++;
      for(i=0;myrestrict[ii][i]!='*' && i<strlen(myrestrict[ii]) && ok[ii]==1;i++) {
        if (myrestrict[ii][i]!=str[i]) ok[ii]=0;
       }
     }
    fclose(fp);
    num=ok[0]+ok[1];
    if ((max>0 && num>0) || (max==0)) return 0;
    return 1;
   }
  fgets(line,99,fp);
 }
fclose(fp);
return 1;
}

void police_freeze(UR_OBJECT user,int priority)
{
 if (user->level<KIN || user->wizactivity<0) return;
 if (!strcmp(user->wizpass,"-")) return;
 user->wizactivity+=priority;
 if (user->wizactivity>6) {
   write_user(user,"\n\n~OL~FTPOLICE:  ~FWFREEZE! Get your hands behind your head! NOW! DO IT!\n\n");
   write_user(user,"~OL~FRZadaj svoje sluzobne heslo: \n");
   user->has_ignall=user->ignall;
   user->ignall=1;
   user->misc_op=33;
   user->wizactivity=20;
   echo_off(user);
  }
}

/* determine if year is a leap year */
int is_leap(unsigned yr) {
  return yr%400==0 || (yr%4==0 && yr%100!=0);
}
/* convert months to days */
unsigned months_to_days(unsigned mn) {
  return (mn*3057-3007)/100;
}
/* convert years to days */
long years_to_days(unsigned yr) {
  return yr*365L+yr/4-yr/100+yr/400;
}
/* convert a given date (y/m/d) to a scalar */
long ymd_to_scalar(unsigned yr, unsigned mo, unsigned dy) {
long scalar;

scalar=dy+months_to_days(mo);
/* adjust if past February */
if (mo>2) scalar-=is_leap(yr)?1:2;
yr--;
scalar+=years_to_days(yr);
return scalar;
}
/* converts a scalar date to y/m/d */
void scalar_to_ymd(long scalar, unsigned *yr, unsigned *mo, unsigned *dy) {
unsigned n;

/* 146097 == years_to_days(400) */
for (n=(unsigned)((scalar*400L)/146097);years_to_days(n)<scalar;) n++;
*yr=n;
n=(unsigned)(scalar-years_to_days(n-1));
/* adjust if past February */
if (n>59) {                       
  n+=2;
  if (is_leap(*yr)) n-=n>62?1:2;
  }
/* inverse of months_to_days() */
*mo=(n*100+3007)/3057;
*dy=n-months_to_days(*mo);
}
/* determine if the y/m/d given is todays date */
int is_ymd_today(unsigned yr, unsigned mo, unsigned dy) {
  if (((int)yr==(int)tyear) && ((int)mo==(int)tmonth+1) && ((int)dy==(int)tmday)) return 1;
  return 0;
}


void calendar(UR_OBJECT user)
{
int iday,day_1,numdays,j,daj=0,yr,mo;
/* unsigned yr,mo; */
char temp[ARR_SIZE],datumm[10];

if (word_count>3)
 {
  write_user(user,"~FTPouzi: calendar [<mesiac> | <meno>] [<rok>]\n");
  return;
  }
/* full date given */
if (word_count==3)
 {
  yr=atoi(word[2]);
  mo=atoi(word[1]);
  if (yr<100) yr+=2000;
  if ((yr>3000) || (yr<1800) || !mo || (mo>12)) {
  write_user(user,"~FTPouzi: calendar [ <meno> | <mesiac>] [<rok>]\n");
  return; }
 }
/* only month given, so show for this year */
else if (word_count==2) {
  if (strlen(word[1])>2) {
    word[1][0]=toupper(word[1][0]);
    strcpy(datumm,datum_menin(word[1]));
    if (datumm[0]) {
      yr=tyear;
      sscanf(datumm,"%d.%d",&daj,&mo);
      sprintf(datumm,"%d.%d.",daj,mo);
     }
    else {
      write_user(user,"~FTMeno nebolo v kalendari najdene.\n");
      return;
     }
   }
  else {
    yr=tyear;
    mo=atoi(word[1]);
    if (!mo || (mo>12)) {
      write_user(user,"~FTPouzi: calendar [ <meno> | <mesiac>] [<rok>]\n");
      return;
     }
   }
 }
/* todays month and year */
else {
  yr=tyear;
  mo=tmonth+1;
  daj=-1;
  }
/* show calendar */
numdays=cal_days[mo-1];
if (2==mo && is_leap(yr)) ++numdays;
day_1=(int)((ymd_to_scalar(yr,mo,1) - (long)1)%7L);
temp[0]='\n';  text[0]='\n';
sprintf(text,"~OL~FT%s %d~RS",month[mo-1],yr);
write_user(user,title(text,"~OL~FB"));
text[0]='\0';
if (daj>0) {
  sprintf(text,"\n %-24.24s ",meniny(daj,mo-1));
 }
else if (daj==-1) sprintf(text,"\n Dnes ma meniny:          ");
else sprintf(text,"\n                          ");
for (j=0;j<7;) {
   sprintf(temp,"~OL~FY%s~RS",cal_daynames[j]);
   strcat(text,temp);
   if (7!=++j) strcat(text," ");
   }
if (daj>0) 
 sprintf(texthb,"\n  ma meniny ~OL~FY%-6s~RS        ~FB---------------------------\n                          ",datumm);
else if (daj==-1)
 sprintf(texthb,"\n  %-12s            ~FB---------------------------\n                          ",meniny(tmday,tmonth));
else sprintf(texthb,"\n                          ~FB---------------------------\n                          ");
strcat(text,texthb);
for (iday=0;iday<day_1;++iday) strcat(text,"    ");
for (iday=1;iday<=numdays;++iday,++day_1,day_1%=7) {
   if (!day_1 && 1!=iday) strcat(text,"\n                          ");
    if (daj && iday==daj) {
     sprintf(temp," ~OL~FY%2d~RS ",iday);
     strcat(text,temp);
     }
    else if (is_ymd_today(yr,mo,iday)) {
     sprintf(temp," ~OL~FG%2d~RS ",iday);
     strcat(text,temp);
     }
   else {
     if (day_1%7>4) sprintf(temp," ~OL%2d~RS ",iday);
     else sprintf(temp," %2d ",iday);
     strcat(text,temp);
     }
   }
for (;day_1;++day_1,day_1%=7) strcat(text,"      ");
strcat(text,"\n\n~FB-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.\n");
write_user(user,text);
write_user(user,"~FTPouzi: calendar [ <meno> | <mesiac>] [<rok>]\n");
}

char *datum_menin(char *meno)
{
static char date[81];
FILE *fp;
char fname[81],name[81],name2[81];
unsigned int i;
int ok,len;
 
 if (!strcmp(meno,"Petra")) return "29.06";
 date[0]='\0';
 name2[0]='\0';
 sprintf(fname,"%s",MENINY_FILE);
 if (!(fp=ropen(fname,"r"))) { /*APPROVED*/
   sprintf(text,"Nemozem najst subor s meninami!\n");
   write_syslog(text,0);
   return date;
  }
 while (!feof(fp)) {
   fgets(date,80,fp);
   fgets(name,80,fp);
   name[strlen(name)-1]=0;
   if (strchr(name,',')) {
     ok=-1;len=0;
     for(i=0;i<strlen(name);i++) {
       if (ok>-1) { name2[len]=name[i]; len++; }
       else if (name[i]==',') ok=i;
      }
     name[ok]='\0';
    }
   if(!strcmp(name,meno) || !strcmp(name2,meno)) { 
     date[strlen(date)-1]=0;
     fclose(fp);
     return date; 
    }
  }
 rewind(fp);
 while (!feof(fp)) {
   fgets(date,80,fp);
   fgets(name,80,fp);
   if(strstr(name,meno)) { 
     date[strlen(date)-1]=0;
     fclose(fp);
     return date; 
    }
  }
 fclose(fp);
 date[0]=0;
 return date;
}

void show_recent_notifies(UR_OBJECT user)
{
UR_OBJECT u;
int miscoptimeelapsed;
int sessiontime;
int delta;

miscoptimeelapsed=thour*3600+tmin*60+tsec-user->miscoptime;
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE || u->login || u->port==port[1] || !check_notify_user(user,u)) continue;
  sessiontime=(int)(time(0) - u->last_login);
  delta=miscoptimeelapsed-sessiontime;
  if (delta>0) {
    if (sessiontime<60) 
     sprintf(text,"~LB~OL~FG~LI%s~RS~OL~FG sa ti prihlasil%s pred %d sekund%s!\n",u->name,pohl(u,"","a"),sessiontime,skloncislo(sessiontime,"ou","ami","ami"));
    else 
     sprintf(text,"~LB~OL~FG~LI%s~RS~OL~FG sa ti prihlasil%s pred %d minut%s!\n",u->name,pohl(u,"","a"),sessiontime/60,skloncislo(sessiontime/60,"ou","ami","ami"));
    write_user(user,text);
   }
 }
}

int sk(UR_OBJECT user)
{
 if (user->lang==1) return 0;
 return 1;
}

char *langselect(UR_OBJECT user,char *slovak,char *english)
{
if (user->lang==1) return (english);
return(slovak);
}

void misc_stuff(int level)
{
UR_OBJECT u;
int c1=0,c2=0,muzi=0,zeny=0;


for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE || u->login) continue;
  if (u->sex) muzi++;
  else zeny++;
  if (u->room!=NULL && u->room==testroom) {
    if (u->level<=PRI && !u->special) c1++;
    else c2++;
   }
 }

if (level && tmin%10==0) {
  sprintf(query,"insert into `visitors` (`date`,`time`,`males`,`females`) values ('%04d-%02d-%02d','%d','%d','%d')",tyear,tmonth+1,tmday,thour*10+tmin/10,muzi,zeny);
  mysql_kvery(query);
 }

if (c1 && !c2) {
  if (level==0) {
    write_room(testroom,"\n~HYZ dzungle sa ozyvaju strasidelne zvuky.. to budu ludozruti.\n");
    return;
   }
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->type==CLONE_TYPE || u->login) continue;
    if (u->room!=NULL && u->room==testroom) {
      u->dead=15;
      write_user(u,"\n~HRPribehla tlupa ludozrutov, zjedla ti nohy a zvysok si nechali na zajtra!\n");
     }
   }
 }
}

void do_db_backup(int full)
{
char filename[80],store[80];
FILE *fp;

 switch(double_fork()) {
   case -1: return;
   case  0:
     sprintf(filename,"./autobackup");
     if ((fp=fopen(filename,"w"))==NULL) {
       sprintf(text,"Failed opening %s in do_db_backup()\n",filename);
       write_syslog(text,1);
       return;
      }
     if (full) {
       sprintf(store,"f%0dh-%02dd-%02dm",thour,tmday,tmonth);
       fprintf(fp,"mysqldump -h %s -B %s -a -u %s --password=%s >%s/%s.dump\n",DBHOST,DBNAME,DBUSER,DBPASS,BACKUP_DIR,store);
       write_syslog("Dumping database..\n",1);
      }
     else {
       sprintf(store,"u%0dh-%02dd-%02dm",thour,tmday,tmonth);
       fprintf(fp,"mysqldump -h %s -B %s -a --tables users -u %s --password=%s >%s/%s.dump\n",DBHOST,DBNAME,DBUSER,DBPASS,BACKUP_DIR,store);
       write_syslog("Dumping usertable..\n",1);
      }
     fprintf(fp,"rm %s\n",filename);
     fclose(fp);
     chmod(filename,S_IRUSR+S_IWUSR+S_IXUSR);
     system(filename);
     _exit(1);
     return;
  }
}
