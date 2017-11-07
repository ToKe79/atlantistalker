/*--------------------------------------------------------------------------*/
/*                    REMOTE CONNECT - 23-24.12.1999                        */
/*--------------------------------------------------------------------------*/
#include "atl-head.h"
#include "atl-mydb.h"

void test_lynx_done() // vola sa v do_events
{
UR_OBJECT user;
int status;
char filename[256];

for(user=user_first;user!=NULL;user=user->next) {
  if (user->login || user->type!=USER_TYPE || user->room==NULL ||
!user->lynx) continue;
  if (waitpid (user->lynx, &status, WNOHANG) == user->lynx) {
    user->lynx=0;
    sprintf(filename,"mailspool/lynx-%s.tmp",user->name);
    if (user->misc_op) {
      unlink(filename);
      continue;
     }
    write_user(user,"\n~OL~FB-    -  - -- -------< ~FYLynx ukoncil nacitavanie dokumentu!~FB >------- -- -  -    -\n");
    user->browsing=6;
    switch(more(user,user->socket,filename)) {
      case 0: write_user(user,"~FTLynx~FW: Subor nenajdeny!\n"); 
      break;
      case 1: user->misc_op=2;
      break;
     }              
   }
  else {
    sprintf(filename,"mailspool/lynx-%s.tmp",user->name);    
    if (filesize(filename)>200000) {
      kill(-user->lynx,9);
      waitpid(user->lynx, &status, 0);          
      user->lynx=0;
      unlink(filename);
     }
   }
 }
}

void lynx(UR_OBJECT user) // hlavny prikaz
{
int status;
pid_t pid;
FILE *fp;
int len=0;
char *str;

if (word_count!=2) {
   write_user(user,"Pouzitie: .lynx <adresa|stop> (protokol je vzdy http://)\n");
   return;
   }

if (!strcmp(word[1],"stop")) {
   if (user->lynx) {
       kill(-user->lynx,9);
       waitpid(user->lynx, &status, 0);
       sprintf(text,"mailspool/lynx-%s.tmp", user->name);
       if ((fp=ropen(text,"r"))!=NULL) { fclose(fp); unlink(text); }
       write_user(user,"~FTLynx:~FW Stahovanie suboru bolo zrusene...\n");
       user->lynx=0;
       return;
       }
   else {
       write_user(user,"~FTLynx:~FW nespracuvava sa nijaky subor...\n");
       return;
       }
   }

if (user->lynx) {
   write_user(user,"~FTLynx:~FW este stale sa taha subor...\n");
   return;
   }

if (strstr(word[1],":/") || strpbrk(word[1]," ;,[]{}!@$%^*()&|<>")) {
        write_user(user,"Nespravne zadana adresa.\n");
        return;
        }
len=strlen(word[1]);
if (len>4) {
  str=word[1]+len-4;
  if (!strcasecmp(str,".zip") || !strcasecmp(str,".mpg") ||
    !strcasecmp(str,".rar") || !strcasecmp(str,".mp3") ||
    !strcasecmp(str,".arj") || !strcasecmp(str,".tar") ||
    !strcasecmp(str,".avi")
   ) {
      write_user(user,"Pomocu prikazu .lynx sa nedaju stahovat subory!\n");
      return;
     }
 }
switch (pid=fork()) {
    case -1: write_user(user,"~FTLynx:~FW lutujem, chyba pri forku...\n");
             return;
    case  0: setpgid(0,0); /* rodinne meno - vyvrazdime potom vsetko nazraz
:) */
             sprintf(text,"/usr/bin/lynx -restrictions=all -dump http://%s > mailspool/lynx-%s.tmp 2>> mailspool/lynx-%s.tmp",word[1],user->name, user->name);
             system(text);/* deticko */
             _exit(0);
    }
user->lynx=pid;
sprintf(text,"~FTLynx:~FW hlasim sa na adresu '~OL%s~RS'...\n",word[1]);
write_user(user,text);
}

void cmon_auth(UR_OBJECT gdo)
{
char identita[100];
FILE *fp;

switch(double_fork()) {
	case -1: return; break;
	case 0 : sstrncpy(identita,real_auth(gdo),99); /* real_user() */
		 if ((fp=ropen(GOT_AUTH,"a"))!=NULL) {
		 	fprintf(fp,"%s %s\n", gdo->name, identita);
		 	fclose(fp);
		 	if (identita[0]!='<') {
		 		sprintf(text,"Ident reply for %s: %s\n", gdo->name, identita);
			 	write_syslog(text,1);
			 	}
		 	}
		 _exit(1);
	}
} 

void check_ident_reply()
{
FILE *fp;
UR_OBJECT u;
char name[20], ident[100];

if ((fp=ropen(GOT_AUTH,"r"))==NULL) return;

fscanf(fp,"%s %s", name, ident);
while(!feof(fp)) {
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->login) continue;
		if (!strcmp(u->name,name)) {
			sstrncpy(u->real_ident, ident, 99);
			break;
			}
		}	
	fscanf(fp,"%s %s", name, ident);	
	}
fclose(fp);
unlink(GOT_AUTH);
}

/* Toto je nahrada, ale tiez nefunguje a hlavne mrzne. Sux. */
char *real_auth(UR_OBJECT gdo)
{
struct sockaddr_in addr;
struct hostent *he;
struct timeval timeout;
register unsigned long inlocal;
register unsigned long inremote;
register unsigned short local;
register unsigned short remote;
register int fd;
register int fl;
unsigned int dummy;                
int inetnum,s,buflen;
char buf[1000],*sn;
unsigned int i;
fd_set fs;
char meno[500];
static char vysledok[570];

dummy=sizeof(addr);
fd=gdo->socket;
if (getsockname(fd,(struct sockaddr *)&addr,&dummy) == -1) {
	strcpy(vysledok,"<CHYBA1>");
	return vysledok;
	}	
if (addr.sin_family != AF_INET) {
	strcpy(vysledok,"<CHYBA2>");
	return vysledok;              
        }
local = ntohs(addr.sin_port);
inlocal = addr.sin_addr.s_addr;
dummy = sizeof(addr);
if (getpeername(fd,(struct sockaddr *)&addr,&dummy) == -1) {
	strcpy(vysledok,"<CHYBA3>");
	return vysledok;              
	}
remote = ntohs(addr.sin_port);
inremote = addr.sin_addr.s_addr;
                            	            
sn=gdo->site;
while(*sn && (*sn=='.' || isdigit(*sn))) sn++;
if (*sn) {
	if (!(he=gethostbyname(gdo->site))) {
		strcpy(vysledok,"<CHYBA4>");
		return vysledok;
		}
	memcpy((char *)&addr.sin_addr,he->h_addr,(size_t)he->h_length);
	}
else {
	if ((inetnum=inet_addr(gdo->site))==-1) {
		strcpy(vysledok,"<CHYBA5>");
		return vysledok;	
		}
	memcpy((char *)&addr.sin_addr,(char *)&inetnum,(size_t)sizeof(inetnum));
	}
if ((s=socket(AF_INET,SOCK_STREAM,0))==-1) {
	strcpy(vysledok,"<NO_IDENT1>");
	return vysledok;
	}
if (inlocal) {
	/*clearsa(&addr);*/
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = inlocal;
	if (bind(s,(struct sockaddr *)&addr,sizeof(addr)) == -1) {
		close(s);
		strcpy(vysledok,"<CHYBA6>");
		return vysledok;		
		}
	}
if ((fl = fcntl(s,F_GETFL,0)) == -1) {
	close(s);
	strcpy(vysledok,"<CHYBA7>");
	return vysledok;	
	}
if (fcntl(s,F_SETFL,NBLOCK_CMD | fl) == -1) {
	close(s);
	strcpy(vysledok,"<CHYBA8>");
	return vysledok;	
	}
addr.sin_family=AF_INET;
addr.sin_port=htons(113);
addr.sin_addr.s_addr=inremote;
signal(SIGALRM,SIG_IGN);
if (!connect(s,(struct sockaddr *)&addr,sizeof(addr))==-1) {
	reset_alarm();
	close(s);
	strcpy(vysledok,"<NO_IDENT2>");
	return vysledok;
	}
sprintf(buf,"%u,%u\r\n",gdo->site_port,gdo->port);
buflen=strlen(buf);
FD_ZERO(&fs);
FD_SET(s, &fs);
timeout.tv_sec=5;
timeout.tv_usec=0;
if (select(s+1,FD_CAST NULL,FD_CAST&fs,FD_CAST NULL,(struct timeval *)&timeout)==0 || !FD_ISSET(s,&fs)) {
	close(s);
	reset_alarm();
	strcpy(vysledok,"<NO_IDENT4>");
	return vysledok;
	}
if (write(s,buf,buflen)!=buflen) {
	reset_alarm();
	close(s);
	strcpy(vysledok,"<NO_IDENT5>");
	return vysledok;
	}
i=0;
buf[0]=0;
FD_ZERO(&fs);
FD_SET(s,&fs);
timeout.tv_sec=3;
timeout.tv_usec=0;
if (select(s+1,FD_CAST&fs,FD_CAST NULL,FD_CAST NULL,(struct timeval *)&timeout)==0 || !FD_ISSET(s,&fs)) {
	reset_alarm();
	close(s);
	strcpy(vysledok,"<NO_IDENT6>");
	return vysledok;
	}
do {
	FD_ZERO(&fs);
	FD_SET(s,&fs);
	timeout.tv_sec=3;
	timeout.tv_usec=0;
	if (select(s+1,FD_CAST&fs,FD_CAST NULL,FD_CAST NULL,(struct timeval *)&timeout)==0 || !FD_ISSET(s,&fs))
		break;
	if (read(s,buf+i,1)!=1)
		break;
	if (buf[i]!=' ' && buf[i]!='\t' && buf[i]!='\r')
		i++;
	} while(buf[i-1]!='\n' && i<sizeof(buf));
buf[i]='\0';
close(s);
reset_alarm();
if (sscanf(buf,"%*d,%*d:USERID:%*[^:]:%s",meno)!=1) {
	if (sscanf(buf,"%*d , %*d : USERID : %*[^:] : %s",meno)!=1) {
		strcpy(vysledok,"<NO_IDENT7>");
		return vysledok;		
		}
	}
else {
	sprintf(vysledok,"%s@%s", meno, gdo->site);
	return vysledok;
	}
strcpy(vysledok,"<CHYBA>");
return vysledok;	
}


/************** Weather report daily ;) ***************/
/* Spartakus - .weather (predpoved pocasia), berie sa z www.shmu.sk */
void weather(user, inpstr)
UR_OBJECT user;
char *inpstr;
{
char filename[80];

if (word_count<2) {
	  write_user(user,"Pouzi: .weather <zajtra | 2dni | ss | vs | zs | ba | tatry>\n");
	  write_user(user,"       .weather zajtra       - zobrazi predpoved pocasia na zajtra\n");
	  write_user(user,"       .weather 2dni         - predpoved na nasledujuce 2 dni\n");
	  write_user(user,"       .weather ss           - predpoved pre stredne Slovensko\n");
	  write_user(user,"       .weather vs           - predpoved pre vychodne Slovensko\n");
	  write_user(user,"       .weather zs           - predpoved pre zapadne Slovensko\n");
	  write_user(user,"       .weather ba           - predpoved pre Bratislavu\n");	  
	  write_user(user,"       .weather tatry        - predpoved pre Vysoke Tatry\n\n");
	  return;
	}  

if ((!strcmp(inpstr,"update")) && (user->level>=WIZ)) {
	zober_predpoved(user,1);
	return;
	}

filename[0]='\0';
if (!strncmp(word[1],"za",2)) strcpy(filename,WEATHER_TODAY_FILE);
if (!strncmp(word[1],"2",1))  strcpy(filename,WEATHER_TOMORROW_FILE);
if (!strncmp(word[1],"ss",2)) strcpy(filename,WEATHER_STRED_FILE);
if (!strncmp(word[1],"vs",2)) strcpy(filename,WEATHER_VYCHOD_FILE);
if (!strncmp(word[1],"zs",2)) strcpy(filename,WEATHER_ZAPAD_FILE);  
if (!strncmp(word[1],"ba",2)) strcpy(filename,WEATHER_BLAVA_FILE);
if (!strncmp(word[1],"ta",2)) strcpy(filename,WEATHER_TATRY_FILE);

if (!filename[0]) {
	write_user(user,"Pouzi: .weather <zajtra | 2dni | ss | vs | zz | ba | tatry>\n");
	return;
	}
  
switch(more(user,user->socket,filename)) {
	  case 0: write_user(user,"Prepac, predpoved sa nepodarilo najst.\n");  break;
	  case 1: user->misc_op=2;
	  }
}  

void zober_predpoved(user,force)
UR_OBJECT user;
int force;
{

if (force) write_user(user,"Pokusim sa stiahnut predpoved pocasia ...\n");

switch(double_fork()) {
	case -1: return; break;
	case 0 : 
		/* Vsetky fajly ktore xceme stiahnut stiahneme najprv tu ... 
		    www.shmu.sk 193.87.204.9 */
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?2","pocasie/raw2.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?3","pocasie/raw3.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?4","pocasie/raw4.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?5","pocasie/raw5.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?6","pocasie/raw6.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?7","pocasie/raw7.htm");
		webcicni("www.shmu.sk","/predpoved/predpoved.cgi?8","pocasie/raw8.htm");

		/* ... a parsneme */
		parsni_pocasie("pocasie/raw2.htm",WEATHER_TODAY_FILE);
		parsni_pocasie("pocasie/raw3.htm",WEATHER_TOMORROW_FILE);
		parsni_pocasie("pocasie/raw4.htm",WEATHER_STRED_FILE);
		parsni_pocasie("pocasie/raw5.htm",WEATHER_VYCHOD_FILE);
		parsni_pocasie("pocasie/raw6.htm",WEATHER_ZAPAD_FILE);
		parsni_pocasie("pocasie/raw7.htm",WEATHER_BLAVA_FILE);
		parsni_pocasie("pocasie/raw8.htm",WEATHER_TATRY_FILE);			 
		 _exit(1);
		break;
	}

if (force) write_user(user,"Poziadavka bola odoslana ...\n");
}
/*------------------------------------------------------------------------*/

/* WEATHER FUNCTIONS!! */

void webcicni(host, dokument, filename)
char host[50], dokument[80],filename[80];
{ 
int len;
static char rbuf[4]; 
int   zasuva; 
char reqs[80];
FILE *fajl;

strcpy (reqs,"GET ");
strcat (reqs, dokument);
strcat (reqs,"\n");
zasuva = connecthost (host,80);

  if (zasuva < 0)
  { write_syslog("Nepodarilo sa spojit s pocasim! :>\n",1);
    return;
  }
//sleep(5);
  if (write (zasuva, reqs, strlen(reqs)) != strlen(reqs))
  { write_syslog("Write failed: webcinci funkcia",1);
    return;
  }

if (!(fajl=ropen(filename,"w"))) { /*APPROVED*/
	return;
	}

  while ((len = read (zasuva, rbuf, 1)) > 0)
  { 
    fputc(*rbuf,fajl);
  }

  if (len < 0)
  { write_syslog("Chyba v nacitavani stranky (webcicni)",1);  
  }
  close (zasuva);
   fclose(fajl);
  return;
}

void parsni_pocasie(vstup, vystfile)
char vstup[80],vystfile[80];
{
FILE *fajl;
FILE *vystup;
char line[101];
int stav,i;

stav=0;
if (!(fajl=ropen(vstup,"r"))) { /*APPROVED*/
	return;
	}
	
if (!(vystup=ropen(vystfile,"w"))) { /*APPROVED*/
	 fclose(fajl);
	return;
	}

fgets(line,100,fajl);
while (!feof(fajl)) {
	if (strstr(line, "PREDPOVED")) stav=1;
	if (strstr(line, "SHMU")) stav=0;
		if (line[0]=='<') {
			fgets(line,100,fajl);
			continue;
			}
		for (i=0;i<=strlen(line);++i) { 		
			if (line[i]=='<') { line[i]='\n';
					    line[i+1]='\0'; }
			}
	if (stav) fputs(line,vystup);	
	fgets(line,100,fajl);
	}

 fclose(fajl);
 fclose(vystup);
}

int connecthost(serveridlo, port) 
char serveridlo[50];
int port;
{ 
  struct sockaddr_in sin; 
  struct hostent *hp; 
  int     fd;  

if (sigsetjmp(save_state,1)!=0) {
	reset_alarm();
	return -1;
	}
 
  bzero((char *) &sin, sizeof(sin)); 
  sin.sin_port = htons(port); 
  if ((hp = gethostbyname(serveridlo)) == 0) return (-1);  
  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length); 
  sin.sin_family = hp->h_addrtype;  
  fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (fd < 0) return -1;  
  
  #ifndef WIN32
  siginterrupt(SIGALRM,1);                        
  #endif
  signal(SIGALRM, connecthost_timeout);
  alarm(20);
  if (connect(fd,(struct sockaddr *) &sin, sizeof(sin)) < 0) return -1;
  reset_alarm();
  return fd;
}
 
void connecthost_timeout()
{
sprintf(text,"Stiahnutie predpovede pocasia sa timeoutlo.\n");
write_syslog(text,1);
siglongjmp(save_state,1);
} 
/*--------------   End of weather functions   ----------------*/


void note(UR_OBJECT user, char *inpstr)
{
//char filename[80], line[201];
int cnt=0,subs=0,sub=0,num=0;
//FILE *fp;
//struct stat statbuf;
char *hlp="~FTPouzi:~FW .note >strana <poznamka> ~FTalebo ~FW.note <poznamka>  ~FTak chces pridat poznamku\n       .note >strana  ~FTak chces zobrazit poznamky na strane\n       .note move >strana <cislo> >strana ~FTak chces presunut poznamku\n";

if (word_count==1) { 
  write_user(user,title("~OL~FYTvoj poznamkovy blok","~OL~FB"));
  sprintf(query,"select note from notes where userid='%d' and subdir='' order by date;",user->id);
  if ((result=mysql_result(query))) {
    while ((row=mysql_fetch_row(result))) { 
      cnt++;
      sprintf(text,"~OL%2d:~RS %s\n",cnt,row[0]?row[0]:"");
      write_user(user,text);
     }
    mysql_free_result(result);
   }
  sprintf(query,"select subdir from notes where userid='%d' group by subdir;",user->id);
  if ((result=mysql_result(query))) {
    text[0]='\0';
    while ((row=mysql_fetch_row(result))) { 
      if (row[0] && strlen(row[0])==0) continue;
      subs++;
      if (subs==1) write_user(user,title("~OL~FYStrany","~OL~FB"));
      sprintf(texthb," ~OL%-19s",row[0]?row[0]:"");
      strcat(text,texthb);
      if (subs%4==0) strcat(text,"\n");
     }
    if (subs%4!=0) strcat(text,"\n");
    write_user(user,text);
    mysql_free_result(result);
   }
  write_user(user,"~OL~FB.-~FY*~FB-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-~FY*~FB-.\n");
  if (!subs && !cnt) write_user(user,"Nemas ziadny zaznam v poznamkovom bloku.\n");
  write_user(user,hlp);
  return;
 }
else {
  if (!strcmp(word[1],"forward")) {
    fmail(user,1);
    return;
   }
  if (!strcmp(word[1],"move")) {
    if (word_count<4) {
      write_user(user,"~FTPouzi:~FW .note move >strana <cislo> >strana ~FTalebo ~FW.note move <cislo> >strana\n");
      return;
     }
    if (word[2][0]=='>') sub=1;
    else sub=0;
    num=atoi(word[2+sub]);
    if (num<1 || word[3+sub][0]!='>') {
      write_user(user,"~FTPouzi:~FW .note move >strana <cislo> >strana ~FTalebo ~FW.note move <cislo> >strana\n");
      return;
     }
    strcpy(texthb,dbf_string(word[3+sub]));
    strcpy(text,dbf_string(word[2]));
    sprintf(query,"select note from notes where userid='%d' and subdir='%s' order by date;",user->id,sub?text:"");
    if ((result=mysql_result(query))) {
      while ((row=mysql_fetch_row(result))) { 
        if (!row[0]) continue;
        cnt++;
        if (num==cnt) {
          if (strlen(word[3+sub])==1)
            sprintf(query,"update notes set subdir='' where userid='%d' and subdir='%s' and note='%s'",user->id,sub?text:"",dbf_string(row[0]));
          else 
            sprintf(query,"update notes set subdir='%s' where userid='%d' and subdir='%s' and note='%s'",texthb,user->id,sub?text:"",dbf_string(row[0]));
          mysql_kvery(query);
          subs++;
         }
       }
      mysql_free_result(result);
     }
    if (subs) {
      if (sub) word[2][0]=' ';
      word[3+sub][0]=' ';
      strcpy(texthb,dbf_string(word[3+sub]));
      strcpy(text,dbf_string(word[2]));
      if (sub) {
        if (strlen(word[3+sub])==1) 
          sprintf(query,"Poznamka bola presunuta zo strany%s na hlavnu stranu.\n",text);
        else 
          sprintf(query,"Poznamka bola presunuta zo strany%s na stranu%s.\n",text,texthb);
       }
      else sprintf(query,"Poznamka bola presunuta na stranu%s.\n",texthb);
      write_user(user,query);
      return;
     }
    write_user(user,hlp);
    return;
   }
  if (word_count==2) {
    sprintf(query,"select note from notes where userid='%d' and (subdir='%s' or subdir='>%s') order by date;",user->id,dbf_string(word[1]),dbf_string(word[1]));
    if ((result=mysql_result(query))) {
      while ((row=mysql_fetch_row(result))) { 
        cnt++;
        if (cnt==1) {
          if (word[1][0]=='>') word[1][0]=' ';
          sprintf(texthb,"~FYTvoj poznamkovy blok, strana%s~OL%s~RS",word[1][0]==' '?"":" ",word[1]);
          sprintf(text,title(texthb,"~OL~FB"));
          write_user(user,text);
         }
        sprintf(text,"~OL%2d:~RS %s\n",cnt,row[0]?row[0]:"");
        write_user(user,text);
       }
      mysql_free_result(result);
     }
    if (cnt) {
      write_user(user,"~OL~FB.-~FY*~FB-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-~FY*~FB-.\n");
      write_user(user,hlp);
      return;
     }
   }
  if (word[1][0]=='>' && strlen(word[1])>1) {
    if (word_count==2) {
      word[1][0]=' ';
      sprintf(text,"Stranu~OL%s~RS este nemas vytvorenu.\n",word[1]);
      write_user(user,text);
      return;
     }
    sub=1;
    inpstr=remove_first(inpstr);
    strcpy(texthb,dbf_string(word[1]));
   }
  inpstr[512]='\0';
  sprintf(query,"replace into notes (userid,subdir,note,date) values ('%d','%s','%s',FROM_UNIXTIME(%d))",
  user->id,sub?texthb:"",dbf_string(inpstr),(int)time(0));
  mysql_kvery(query);
  word[1][0]=' ';
  if (sub) sprintf(texthb,"na stranu~OL%s~RS",word[1]);
  else sprintf(texthb,"do poznamkoveho bloku");
  sprintf(text,"%s si spravu %s:\n%s\n",pohl(user,"Pridal","Pridala"),texthb,inpstr);
  write_user(user,text);
  return;
 }
}

void dnote(UR_OBJECT user)
{
int od=0,po=0,cnt=0,deleted=0,sub=0;
char *help="Pouzi: .dnote [>strana] [cislo] [<od> <do>]  ~FT ak chces vygumovat spravu/y\n       .dnote all  ~FTak chces vysklbat cely blok\n       .dnote >strana all  ~FTak chces vytrhnut stranu\n";

if (word_count<2) {
  write_user(user,help);
  return;
 }

if (word_count==2 && !strcasecmp(word[1],"all")) {
  sprintf(query,"delete from notes where userid='%d'",user->id);
  mysql_kvery(query);
  sprintf(text,"Vysklbal%s si vsetky strany zo svojho poznamkoveho bloku.\n",pohl(user,"","a"));
  write_user(user,text);
  return;
 }

if (word_count==3 && word[1][0]=='>' && strlen(word[1]) && !strcasecmp(word[2],"all")) {
  sprintf(query,"delete from notes where userid='%d' and subdir='%s'",user->id,dbf_string(word[1]));
  mysql_kvery(query);
  word[1][0]=' ';
  if (mysql_affected_rows(&mysql)) {
    sprintf(text,"Vytrh%s si stranu~OL%s~RS zo svojeho poznamkoveho bloku.\n",pohl(user,"ol","la"),word[1]);
    write_user(user,text);
   }
  else {
    sprintf(text,"Stranu~OL%s~RS nemas v poznamkovom bloku.\n",word[1]);
    write_user(user,text);
   }
  return;
 }

if (word[1][0]=='>') sub=1;
else sub=0;
od=0; po=0;
od=atoi(word[1+sub]);
if (word_count==2+sub) po=od;
else po=atoi(word[2+sub]);
if (od<1 || po<1 || od>po) write_user(user,help);
else {
  strcpy(texthb,dbf_string(word[1]));
  sprintf(query,"select note from notes where userid='%d' and subdir='%s' order by date;",user->id,sub?texthb:"");
  if ((result=mysql_result(query))) {
    while ((row=mysql_fetch_row(result))) { 
      if (!row[0]) continue;
      cnt++;
      if (od<=cnt && cnt<=po) {
        sprintf(query,"delete from notes where userid='%d' and subdir='%s' and note='%s'",user->id,sub?texthb:"",dbf_string(row[0]));
        mysql_kvery(query);
        deleted++;
       }
     }
    mysql_free_result(result);
   }
  if (deleted) {
    word[1][0]=' ';
    if (sub) sprintf(text,"Vygumoval%s si %d sprav%s zo strany%s.\n",pohl(user,"","a"),deleted,skloncislo(deleted,"u","y",""),word[1]);
    else sprintf(text,"Vygumoval%s si %d sprav%s z hlavnej strany poznamkoveho bloku.\n",pohl(user,"","a"),deleted,skloncislo(deleted,"u","y",""));
    write_user(user,text);
    return;
   }
  write_user(user,help);
  return;
 }
}

/*
RN_OBJECT create_remote()
{
RN_OBJECT remote;
if ((remote=(RN_OBJECT)malloc(sizeof(struct remote_struct)))==NULL) {
                        		fprintf(stderr,"BOOT: Memory allocation failure in create_remote()");
                                        boot_exit(1);
                                                }
remote->next=NULL;
if (remote_start==NULL) remote_start=remote; 
else remote_last->next=remote;
remote_last=remote;

return remote;
}

int init_remote_connections(UR_OBJECT user)
{
RN_OBJECT remote,next;
char rn_file_name[80];
FILE *rn;

if (user!=NULL) {
  remote=remote_start;
  while (remote!=NULL) {
    next=remote->next;
    free(remote);
    remote=next;
   }
 }

  remote=NULL; remote_start=NULL; remote_last=NULL;
  sprintf(rn_file_name,"misc/%s",REMOTE_FILE);
  if ((rn=ropen(rn_file_name,"r"))) {
    while (!feof(rn)) {
      remote=create_remote();
      fscanf(rn,"%c %s %d %s",&remote->shortcut,remote->name,&remote->port,remote->desc);
      fgets(text,80,rn);
     }
    fclose(rn);
   }
  else write_syslog("Cannot open remote file.\n",1);

if (user==NULL) printf("Inicializujem vzdialeny pristup...\n");
else write_user(user,"Inicializujem vzdialeny pristup...\n");

if (remote_start!=remote_last) {
  for(remote=remote_start;remote->next!=remote_last;remote=remote->next);
  remote->next=NULL;
  free(remote_last);
  remote_last=remote;
 }
for(remote=remote_start;remote!=NULL;remote=remote->next) {
  sprintf(text,"[%c] %s %d %s\n",remote->shortcut,remote->name,remote->port,remote->desc);
  if (user==NULL) printf(text);
  else write_user(user,text);
 }
return 0;
}
*/

RN_OBJECT create_remote(UR_OBJECT user)
{
RN_OBJECT remote;
if ((remote=(RN_OBJECT)malloc(sizeof(struct remote_struct)))==NULL) {
  if (user==NULL) {
    fprintf(stderr,"BOOT: Memory allocation failure in create_remote()");
    boot_exit(1);
   }
  else write_user(user,"BOOT: Memory allocation failure in create_remote()");
 }
remote->next=NULL;
if (remote_start==NULL) remote_start=remote; 
else remote_last->next=remote;
remote_last=remote;

return remote;
}

int init_remote_connections(UR_OBJECT user)
{
RN_OBJECT remote,next=NULL;
char rn_file_name[80],line[200];
FILE *rn;

sprintf(rn_file_name,"misc/%s",REMOTE_FILE);
if (!(rn=ropen(rn_file_name,"r"))) {
  if (user!=NULL) write_user(user,"Nemozem otvorit subor vzdialenych pripojeni.\n");
  else {
    printf("Nemozem otvorit subor vzdialenych pripojeni.");
    remote=NULL; remote_start=NULL; remote_last=NULL;
   }
  write_syslog("Nemozem otvorit subor vzdialenych pripojeni.\n",1);
  return -1;
 }

if (user==NULL) {
  remote=NULL;
  remote_start=NULL;
  remote_last=NULL;
  
  fgets(line,199,rn);
  while (!feof(rn)) {
    remote=create_remote(user);
    sscanf(line,"%c %s %d %s",&remote->shortcut,remote->name,&remote->port,remote->desc);
    fgets(line,199,rn);
   }
  fclose(rn);
 }
else {
  remote=remote_start;
  fgets(line,199,rn);
  while (!feof(rn)) {
    if (remote==NULL) {
      remote=create_remote(user);
      write_user(user,"- creating remote connection..\n");
     }
    else write_user(user,"- overwriting remote connection..\n");
    sscanf(line,"%c %s %d %s",&remote->shortcut,remote->name,&remote->port,remote->desc);
    next=remote;
    remote=remote->next;
    fgets(line,199,rn);
   }
  fclose(rn);
  next->next=NULL;
  remote_last=next;
  while (remote!=NULL) {
    next=remote->next;
    free(remote);
    remote=next;
    write_user(user,"- eliminating remote connection..\n");
   }
 }
for(remote=remote_start;remote!=NULL;remote=remote->next) {
  sprintf(text,"[%c] %s %d %s\n",remote->shortcut,remote->name,remote->port,remote->desc);
  if (user==NULL) printf(text);
  else write_user(user,text);
 }

if (user==NULL) printf("Inicializujem vzdialeny pristup...\n");
else write_user(user,"Inicializujem vzdialeny pristup...\n");
return 0;
}

int connect_to_site(UR_OBJECT user,RN_OBJECT remote,int slot)
{
struct sockaddr_in con_addr;
char *sn;
int inetnum, i;
FILE *fp;

char serveridlo[100], id[15];
int def_port;

if ((user->remote_socket[slot]=socket(AF_INET,SOCK_STREAM,0))==-1) {
	reset_alarm();
	return 1;
	}
i=0;
if (!strcmp(remote->desc,"IRC")) {
	if ((fp=fopen(IRC_SERVERS,"r"))!=NULL) {
		fscanf(fp,"%s %s %d", id, serveridlo, &def_port);
		while(!feof(fp)) {
			if (!strcmp(user->irc_serv, id)) {
				i=1;
				break;
				}		
			fscanf(fp,"%s %s %d", id, serveridlo, &def_port);                
			}
		fclose(fp);
		}
	if (!i) {
		strcpy(serveridlo,"irc.nextra.sk");
		def_port=6667;
		strcpy(user->irc_serv,"nextra1"); /* Dzast tu bi shur! */
		}
	}
	else {
		sstrncpy(serveridlo, remote->name, 99);
		def_port=remote->port;
		}

sn = serveridlo;
while(*sn && (*sn=='.' || isdigit(*sn))) sn++;

if (*sn) {
  if (!(remote->he=gethostbyname(serveridlo))) {
  	reset_alarm();
  	return 2;
  	}
  memcpy((char *)&con_addr.sin_addr,remote->he->h_addr,(size_t)remote->he->h_length);
  }
    else {
      if((inetnum=inet_addr(serveridlo))==-1) {
        reset_alarm();
      	return 2;
      	}
      memcpy((char *)&con_addr.sin_addr,(char *)&inetnum,(size_t)sizeof(inetnum));
      }

con_addr.sin_port=htons(def_port);
con_addr.sin_family=AF_INET;

//signal(SIGALRM,SIG_IGN); /* bolo vypozn. */
 
if (connect(user->remote_socket[slot],(struct sockaddr *)&con_addr,sizeof(con_addr))==-1) {
 	reset_alarm();
 	return 3;
 	}
reset_alarm(); /* bolo vypozn. */
if ((user->remote_fds[slot]=ADD_FDS(user->remote_socket[slot],POLLIN|POLLPRI))==0) return 4;
user->remote[slot]=remote;
user->actual_remote_socket=user->remote_socket[slot];

sprintf(text,"User %s connected to %s %d using socket %d\n",user->name,serveridlo,def_port,user->remote_socket[slot]);
write_syslog(text,1);
return 0;
}

void remote_close(UR_OBJECT user,int i)
{
  sprintf(text,"User %s disconnected from %s.\n",user->name,user->remote[i]->name);
  write_syslog(text,1);
  if (user->remote_socket[i]==user->ircsocknum) user->ircsocknum=0;
  close(user->remote_socket[i]);
  user->remote_socket[i]=0;
  user->remote_fds[i]=0;
  user->remote[i]=NULL;
  UPDATE_FDS();
}

int exec_remote_com(UR_OBJECT user, char *inpstr)
{
  RN_OBJECT remote;
  int i,j;

  if (word[0][0]=='.')
    {
      if (!strcasecmp("atl",&word[0][1]))
	{
	  if (user->actual_remote_socket!=0)
	    {
	      sprintf(text,"~FT[~OLATLANTIS~RS~FT]\n");
	      write_user(user,text);
	      user->actual_remote_socket=0;
              if (user->statline==CHARMODE) {
          	show_statline(user);	  
	  	text_statline(user,1);
		}	  
	    }
	  else 
	    sprintf(text,"~FT[~OLATLANTIS~RS~FT]\n");
 	  write_user(user,text);
          if (user->statline==CHARMODE) {
          	show_statline(user);	  
	  	text_statline(user,1);
		}	  

	  return 1;
	}
        if (!strncasecmp(&word[0][0],".rem",4) && !strncasecmp("?",&word[1][0],1))
	{

	  remote=NULL;
	  for (j=0;j<MAX_CONNECTIONS;j++)
	    if (user->remote_socket[j]==user->actual_remote_socket) 
	      {
		remote=user->remote[j]; break;
	      }

	  if (remote!=NULL) {
	    if (user->remote_socket[j]==user->ircsocknum) sprintf(text,"~FTAktivne spojenie: ~OLIRC~RS~FT (~OL~FT%s~RS~FT)\n", get_ircserv_name(user->irc_serv));
	    	else sprintf (text,"~FTAktivny talker: ~OL%s ~RS~FT(~OL%s %d~RS~FT)\n",remote->desc,remote->name,remote->port);
	  } else
	    {
	      sprintf(text,"~FTAktivny talker: ~OLATLANTIS\n");
	    }
	  write_user(user,text);
	  return 1;
	}
     
    }

if (!strcmp(word[0],"+") && word_count==1) {
	user->actual_remote_socket=0;
	write_user(user,"~FT[~OLATLANTIS~RS~FT]\n");
	if (user->statline==CHARMODE) {
		show_statline(user);
		text_statline(user,1);
		}
	prompt(user);
	return 1;
	}
  
  /*  if ((word[0][0]=='.' && !strncasecmp("nex",&word[0][1],3)) || word[0][0]=='\\') */
  if ((word[0][0]=='.' && !strncasecmp("rem",&word[0][1],3) && !strncasecmp("nex",&word[1][0],3)) /*|| !strcmp(word[0],"\\")*/)
    {

      if (user->actual_remote_socket==0) j=-1; else
	{

	  for (j=0;j<MAX_CONNECTIONS;j++)
	    if (user->remote_socket[j]==user->actual_remote_socket) break;
	}

      for (i=j+1;i<MAX_CONNECTIONS;i++)  
	if (user->remote_socket[i]!=0) 
	  {
	    user->actual_remote_socket=user->remote_socket[i]; 
	    sprintf(text,"~FT[~OL%s~RS~FT]\n",user->remote[i]->desc/*,user->remote[i]->name,user->remote[i]->port*/);
	    write_user(user,text);
	    if (user->statline==CHARMODE) {
	    	show_statline(user);
	    	text_statline(user,1);
	    	}
	    return 1;
	  }

      if (user->actual_remote_socket==0)
	{
	  sprintf(text,"Nemas ziadne dalsie spojenia.\n");
	}
      else
	{
	  sprintf(text,"~FT[~OLATLANTIS~RS~FT]\n");
	  write_user(user,text);
	  user->actual_remote_socket=0;
	  if (user->statline==CHARMODE) {
	  	show_statline(user);
	  	text_statline(user,1);
	  	}
	  prompt(user);
	}
      return 1;

    }
  for (remote=remote_start;remote!=NULL;remote=remote->next)
    {
      if ((remote->shortcut==word[0][0] && strlen(inpstr)==1) /*|| (word[0][0]=='.' && !strncasecmp(remote->desc,&word[0][1],3))*/) {
	for (i=0;i<MAX_CONNECTIONS;i++)
	  if (user->remote[i]==remote)
	    {
	      if (user->actual_remote_socket==user->remote_socket[i])
		{
		  sprintf(text,"~FT[~OL%s~RS~FT]\n",remote->desc);
		  write_user(user,text);
		}
	      else
		{
		  user->actual_remote_socket=user->remote_socket[i]; 
		  sprintf(text,"~FT[~OL%s~RS~FT]\n",remote->desc/*,remote->name,remote->port*/);
		  write_user(user,text);
		}
              if (user->statline==CHARMODE) {
              	show_statline(user);
              	text_statline(user,1);
		}	      
	      return 1;
	    }
	return 0;
      }
    }
  i=-1;
  for (remote=remote_start;remote!=NULL;remote=remote->next)
    {
      if (remote->shortcut==word[0][0]) {
	for (j=0;j<MAX_CONNECTIONS;j++)
	  if (user->remote[j]==remote) { i=j; break; }
	if (i!=-1) 
	  {
	    strncat(inpstr,"\r\n",2);    /*\n\r*/
	    if (user->remote_socket[i]==user->ircsocknum) {
	    	user->com_counter=MAX_COMMANDS;
	    	parse(user, inpstr+1, 1);
	    	}
	    	else write(user->remote_socket[i],inpstr+1,strlen(inpstr)-1);
	    return 1;
	  } else break;
      }
    }
  
  if (user->actual_remote_socket!=0 && word[0][0]!='+') 
    {
      strncat(inpstr,"\r\n",2); /*\n\r*/
      if (user->actual_remote_socket==user->ircsocknum) {
	user->com_counter=MAX_COMMANDS;      
      	parse(user, inpstr, 1);
      	}
	else write(user->actual_remote_socket,inpstr,strlen(inpstr));
      return 1;
    }
  if (word[0][0]=='+') strcpy(inpstr, inpstr+1);
  return 0;
}

void links(UR_OBJECT user)
{
  UR_OBJECT u;
  int count,j,found;

  found=0;
  for (u=user_first;u!=NULL;u=u->next) {
    if (u->login || u->type!=USER_TYPE) continue;
    for (j=0;j<MAX_CONNECTIONS;j++)
      if (u->remote_socket[j]!=0) 
	{ 
	  write_user(user,"~OL~FB-=-=-=-=-=-=-=-=-=-=-=-=~FY Aktualne spojenia z Atlantisu ~FB=-=-=-=-=-=-=-=-=-=-=-=-=\n");

	  write_user(user,"~OL~FMTalker          Adresa                       Port    Uzivatel   Socket   Skratka~RS\n");

	  found=1; 
	  break; 
	}
    if (found) break;
  }

  if (!found) {
    write_user(user,"~FWNiesu aktivne ziadne remote-spojenia.\n");
  }
  else
    {
      count=0;
      for (u=user_first;u!=NULL;u=u->next) {
	if (u->login || u->type!=USER_TYPE) continue;
	for (j=0;j<MAX_CONNECTIONS;j++)
	  {
	    if (u->remote_socket[j]!=0) { count++;
	    if (u->remote_socket[j]==u->ircsocknum) sprintf(text,"%-15s %-36s %-12s %-9d %c\n",u->remote[j]->desc,get_ircserv_name(u->irc_serv),u->name,u->remote_socket[j],u->remote[j]->shortcut);
	    	else sprintf(text,"%-15s %-28s %-7d %-12s %-9d %c\n",u->remote[j]->desc,u->remote[j]->name,u->remote[j]->port,u->name,u->remote_socket[j],u->remote[j]->shortcut);
	    write_user(user,text);
	    }
	  }
      }
      write_user(user,"~OL~FB-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=~RS\n");
      
      //sprintf(text,"~FGPocet spojeni na ine talkre: %d~RS\n",count);
      //write_user(user,text);		
    }
}

void timeout_rem()
{
  sprintf (text,"Time-out pri connecte po %d sekundach\n",heartbeat);
  write_syslog(text,1);
  siglongjmp (save_state,1);
}

void view_remote(UR_OBJECT user)
{
  
  FILE *fp,*rn;
  char filename[80],filename2[80],shortcut,shortcut2='\0';
  char name[REMOTE_NAME_LEN],desc[REMOTE_DESC_LEN],port[5];
  char tmp[3];
  int slot,j,i,free,error;
  int found=0;
  RN_OBJECT remote;
  
  if (word_count<2) {
  	write_user(user,"Pouzi: .remote <list|next|??|connect|disconnect|talker/skratka> [nick] [heslo]\n");
  	write_user(user,"       .rem list        - ukaze talkery, na ktore sa da napojit cez connect\n");
  	write_user(user,"       .rem connect     - pripojenie na vzdialeny talker\n");
  	write_user(user,"       .rem disconnect  - odpojenie od vzdialeneho talkra\n");
  	write_user(user,"       .rem next        - aktivuje dalsi talker, na ktory mas spojenie.\n");
  	write_user(user,"       .rem ??          - ukaze, ktory talker mas momentalne aktivny.\n");
  	write_user(user,"       .rem crazy       - vyradi CrazyTalker zo zoznamu auto-prihlasovania\n");  	
  	write_user(user,"       .rem crazy janko secret - zaradi CrazyTalker do zoznamu automatickeho\n                                 prihlasovania (nick: janko,  heslo: secret)\n");
        if (user->level>=GOD) 
  	write_user(user,"       .rem reload      - parsne konfiguracny subor remote pripojeni\n");
  	return;
  	}
  if (user->level>=GOD && !strcmp(word[1],"reload")) {
    init_remote_connections(user);
    return;
   }
  if (!strncmp(word[1],"con",3)) {
	  if (sigsetjmp (save_state,1)!=0) {
	    reset_alarm();
	    if (remote_user_active!=NULL) 
	      {
		remote_user_active->remote_socket[remote_slot_active]=0;
		sprintf(text,"~FTTalker ~OL%s %d~RS~FT je nedostupny.\n",remote_active->name,remote_active->port);
		write_user(user,text);
	      }
	    remote_active=NULL;
	    remote_user_active=NULL;
	    remote_slot_active=0;
	    return;
	  } else 
	    {
	      remote_active=NULL;
	      remote_user_active=NULL;
	      remote_slot_active=0;
	    }
	  found=0;
	  if (word_count<3) {
	    for (j=0;j<MAX_CONNECTIONS;j++)
	      {
		if (user->remote_socket[j]!=0)
		  {
		    write_user(user,"~OL~FB=-=-=-=-=-=-=-=-=-=-=-=- ~FYAktivne spojenia na ine talkre ~FB-=-=-=-=-=-=-=-=-=-=-=-\n");
		    write_user(user,"             ~OL~FRNazov talkera           adresa                     port\n");
		    found=1;
		    break;	
		  }
	      }
	    if (!found) { 
	      write_user(user,"V tejto chvili niesi napojeny na ziadny iny talker\nNa pripojenie pouzi prikaz: .remote connect <nazov talkera/skratka>\n");
	    }
	    else 
	      {
		free=MAX_CONNECTIONS;
		for (j=0;j<MAX_CONNECTIONS;j++)
		  {
		    if (user->remote_socket[j]!=0) {
		      free--;
		      sprintf(text,"             ~FT%-23s %-26s %-5d\n",user->remote[j]->desc,user->remote[j]->name,user->remote[j]->port);
		      write_user(user,text);
		    }
		  }
		/*sprintf(text,"~FY~OLPocet moznych spojeni %d, pocet neobsadenych spojeni %d.~RS\n",MAX_CONNECTIONS,free);
		write_user(user,text);*/
		write_user(user,"~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	
	      }
	   // write_user(user,"Pouzi: .remote connect <talker>\n");
	    return;
	  }
	  else
	    {		  
	      for (remote=remote_start;remote!=NULL;remote=remote->next)
		{
		  if (remote->shortcut==word[2][0] || !strncasecmp(remote->desc,word[2],strlen(word[2]))) 
		    {
		      
		      for (i=0;i<MAX_CONNECTIONS;i++) 
			if (user->remote[i]==remote) {
			  sprintf(text,"Uz si %s na ~OL%s~RS.\n",pohl(user,"napojeny","napojena"), remote->desc);
			  write_user(user,text);
			  return; 
			}

		      slot = 0;
		      for (i=0;i<MAX_CONNECTIONS;i++) if (user->remote_socket[i]==0) { slot=i; break; }
				
		      if (slot==0 && i!=0) {
			sprintf(text,"Prepac, uz sa nemozes napojit na dalsi talker.\n");
			write_user(user,text);
			return;
		      }

                      #ifndef WIN32
		      siginterrupt(SIGALRM,1);
		      #endif
		      signal(SIGALRM,timeout_rem);
		      alarm(heartbeat);
		      remote_active=remote; /* aktivny remote server */
		      remote_user_active=user; /* user, pri ktorom sa to cele zalagovalo */
		      remote_slot_active=slot; /* slot, pri ktorom sa to cele zalagovalo */
		      switch ((error=connect_to_site(user,remote,slot)))
			{
			case 1: sprintf(text,"Chyba v pripajani sa na %s %d.\n",remote->name,remote->port); break;
			case 2: sprintf(text,"Nemozem najst ziadany server.\n"); break;
			case 3: sprintf(text,"Couldn't connect to %s port %d\n",remote->name,remote->port);
			  write_syslog(text,1);
        		  sprintf(text,"~FTNa ~OL%s~RS~FT sa nepodarilo pripojit.\n",remote->desc); break;
			case 4: sprintf(text,"Preplnene fds v remote_connect()!\n");
			  write_syslog(text,1);
			  break;
			case 0: 	i=-1;
			  for (j=0;j<MAX_CONNECTIONS;j++)
			    if (remote==user->remote[j]) { i=j; break; } /* i je index socketu s danym talkerom */
	   
			  if (i==-1) { write_user(user,"Nastala chyba v pripajani.\n"); return; }
			  //if (!strcmp(user->remote[i]->name,"fornax.elf.stuba.sk"))  write(user->remote_socket[i],"vt100\n",10);
			  if (!strcmp(user->remote[i]->desc,"IRC")) {
				user->remote_login_socket=user->remote_socket[i];
			  	user->remote_login=3;
			  	user->ircsocknum=user->remote_socket[i];
			  	}
			  else {
                            sprintf(tmp,"%c",user->remote[i]->shortcut);
                            sprintf(query,"select name,pass from remote where userid='%d' and talker='%s';",user->id,dbf_string(tmp));
                            if ((result=mysql_result(query))) {
                              if ((row=mysql_fetch_row(result))) {
                                if (row[0] && row[1]) {
                                  strcpy(user->remote_name,row[0]);
                                  strcpy(user->remote_passwd,row[1]);
                                  strncat(user->remote_name,"\n\r",2);
				  strncat(user->remote_passwd,"\n\r",2);
				  user->remote_login=1; 
				  user->remote_login_socket=user->remote_socket[i];
                                  found=1;
                                 }
                               }
                              else {
                               }
                              mysql_free_result(result);
                             }
			  }
		  
			  if (user->remote_socket[i]==user->ircsocknum) sprintf(text,"~FT[Pripojeny na IRC server ~OL%s~RS~FT]\n", get_ircserv_name(user->irc_serv));
			  	else sprintf(text,"~FT[Pripojeny na ~OL%s %d~RS~FT]\n",user->remote[i]->name,user->remote[i]->port/*,user->remote_socket[i]*/); break;
//			default:
			}
		      if (error!=0) user->remote_socket[slot]=0; //ak nastala chyba
		      //reset_alarm();
		      write_user(user,text);
		      if (user->statline==CHARMODE) {
		      	show_statline(user);
		      	text_statline(user,1);
		      	}
		      break;
		    }
		}
	      if (remote==NULL) {
		sprintf (text,"Nespravny nazov talkera.\n");
		write_user(user,text);
	      }
	    }  		
	return;
	}


  if (!strncmp(word[1],"dis",3)) {
  	if (word_count<3) {
  		write_user(user,"Pouzi: .remote disconnect <talker/skratka>\n");
  		return;
  		}
	  j=0;

	  for (remote=remote_start;remote!=NULL;remote=remote->next)
	    {
	      if (remote->shortcut==word[2][0] || !strncasecmp(remote->desc,word[2],strlen(word[2]))) {
		for (j=0;j<MAX_CONNECTIONS;j++)
		  if (remote==user->remote[j]) {
		    if (user->remote_socket[j]==user->ircsocknum) sprintf(text,"~FT[%s z ~OLIRC~RS~FT]\n", pohl(user,"Odpojeny","Odpojena"));
		    	else sprintf(text,"~FT[%s z ~OL%s %d~RS~FT]\n", pohl(user,"Odpojeny","Odpojena"), user->remote[j]->name,user->remote[j]->port);
		    write_user(user,text);		    
		    close(user->remote_socket[j]);
		    if (user->remote_socket[j]==user->actual_remote_socket) user->actual_remote_socket=0;
		    user->remote_socket[j]=0;
		    user->remote[j]=NULL;
		    if (user->statline==CHARMODE) show_statline(user);
		    return;
		  }
	      }
	    }
	  sprintf(text,"Na taky talker nie si momentalne %s.\n", pohl(user,"pripojeny","pripojena"));
	  write_user(user,text);  		
	return;
  	}
  	
  if (!strcmp(word[1],"list")) {  	
	  write_user(user,"~OL~FB=-=-=-=-=-=-=-=-=-= ~FYMozes sa napojit na nasledovne talkre: ~FB=-=-=-=-=-=-=-=-=-=-\n");
	  write_user(user,"~OL~FRSkratka     Adresa                  Port    Nazov talkera        Autologin\n");
	  
	  sprintf(filename,"misc/%s",REMOTE_FILE);	  
	  if ((fp=ropen(filename,"r"))!=NULL) {
	 	 while (!feof(fp)) {
		  	fscanf(fp,"%c %s %s %s\n",&shortcut, name, port, desc);
                        
                            sprintf(tmp,"%c",shortcut);
                            sprintf(query,"select name from remote where userid='%d' and talker='%s';",user->id,dbf_string(tmp));
                            if ((result=mysql_result(query))) {
                              if ((row=mysql_fetch_row(result))) {
                                if (row[0]) {
 			      	  sprintf(text,"   %c        %-23s %-7s %-20s %-13.13s\n",shortcut,name,port,desc,row[0]);
			      	  write_user(user,text);
                                 }
                               }
                              else {
                                if (!strcmp(desc,"IRC") && user->irc_nick[0]!='\0') sprintf(text,"   %c        %-31s %-20s %-13.13s\n",shortcut2, get_ircserv_name(user->irc_serv), desc, user->irc_defnick);
                                else sprintf(text,"   %c        %-23s %-7s %-20s ---\n",shortcut,name, port, desc);
		      	        write_user(user,text);
                               }
                              mysql_free_result(result);
                             }
                            else {
                              if (!strcmp(desc,"IRC") && user->irc_nick[0]!='\0') sprintf(text,"   %c        %-31s %-20s %-13.13s\n",shortcut2, get_ircserv_name(user->irc_serv), desc, user->irc_defnick);
                              else sprintf(text,"   %c        %-23s %-7s %-20s ---\n",shortcut,name, port, desc);
		      	      write_user(user,text);
                             }

	      }
           fclose(fp);
	   }
  	  
         
          write_user(user,"~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
          return;
          }
      
	
  sprintf(filename2,"misc/%s",REMOTE_FILE);

  /* na zaklade REMOTE_FILE zistime o aky server ide */
  if ((rn=ropen(filename2,"r")))
    {
      found = 0;
      while (!feof(rn)) 
	{
	  fscanf(rn,"%c %s %s %s",&shortcut,name,port,desc);
	  fgets(text,80,rn);
	  if (word[1][0]==shortcut || !strncasecmp(word[1],desc,strlen(word[1]))) { found=1; break; }
	}
      fclose(rn);
      if (!found) 
	{
	  write_user(user,"Talker s takym menom alebo skratkou nieje v zozname povolenych.\n");
	  return; 
	}
    } else  write_syslog("Cannot open remote file in edit_remote_servers().\n",1);

  switch(word_count)
    {
    case 2:
      sprintf(tmp,"%c",shortcut);
      sprintf(query,"delete from remote where userid='%d' and talker='%s';",user->id,dbf_string(tmp));
      if (mysql_kvery(query)) {
        if (mysql_affected_rows(&mysql)) {
	  sprintf(text,"%s si talker ~OL%s~RS zo svojho zoznamu.\n", pohl(user,"Vymazal","Vymazala"), desc);
	  write_user(user,text);
         }
        else {
          sprintf(text,"Talker ~OL%s~RS (~OL%c~RS) nemas v zozname.\n",desc,shortcut);
          write_user(user,text);
         }
       }
      return;
    case 3:
      write_user(user,"Pouzi: .remote <nazov talkera/skratka> <nick> <heslo>\n");
      return;

    case 4: 
      /* name,port,desc mame server, ktory treba pridat */
      if (strlen(word[2])>15 || strlen(word[3])>50) {
	  write_user(user,"Prilis dlhy nick alebo heslo.\n"); 
	  return;
	}
      sprintf(tmp,"%c",shortcut);
      sprintf(query,"replace into remote (userid,talker,name,pass) values ('%d','%s','",user->id,dbf_string(tmp));
      strcat(query,dbf_string(word[2])); strcat(query,"','");
      strcat(query,dbf_string(word[3])); strcat(query,"');");
      if (mysql_kvery(query)) {
        if (mysql_affected_rows(&mysql)==2) {
      	  sprintf(text,"Zaznam bol aktualizovany.\n");
	  write_user(user,text);
         }
        else {
          sprintf(text,"%s si talker ~OL%s~RS do zoznamu.\n",pohl(user,"Pridal","Pridala"), desc);
          write_user(user,text);
         }
       }
      else {
        sprintf(text,"Nastala chyba pri zapise do zoznamu.\n");
        write_user(user,text);
       }
      return;
    default:
      write_user(user,"Pouzi: .remote <nazov talkera/skratka> <nick> <heslo>\n");
      return;
    }            
}

void remote_antiidle()
{
UR_OBJECT user;
int i;

	for (user=user_first;user!=NULL;user=user->next)
	  {
	    if (user->type!=USER_TYPE || user->login || user->afk) continue;
	    for (i=0;i<MAX_CONNECTIONS;i++)
	      if (user->remote_socket[i]!=0 && user->remote_socket[i]!=user->ircsocknum) 
		write(user->remote_socket[i],"\033[0",4);
	    //		write(user->remote_socket[i],"\033[0m",4);
	  }
}



int UPDATE_FDS()
{
  UR_OBJECT u;
  int i;
  size_fds=2;

  for (u=user_first;u!=NULL;u=u->next)
    {
      if (u->type != USER_TYPE || u->socket == 0 || u->socket >= 1000)
	continue;
      fds[size_fds].events=(POLLIN|POLLPRI);
      fds[size_fds].revents=0;
      u->fds=size_fds;
      size_fds++;
       for (i=0;i<MAX_CONNECTIONS;i++)
	if (u->remote_socket[i]!=0) {
	  fds[size_fds].fd=u->remote_socket[i];
	  fds[size_fds].events=(POLLIN|POLLPRI);
	  fds[size_fds].revents=0;
	  u->remote_fds[i]=size_fds;
	  size_fds++;
	  }
    }
  
  return(--size_fds);
}

int ADD_FDS(int socket, int events)
{
  if (size_fds==MAX_POLL-1) return 0;
  else 
    {
    size_fds++;
    fds[size_fds].fd=socket;
    fds[size_fds].events=events;
    fds[size_fds].revents=0;
    return size_fds;
  }
}

extern int spracuj_remote_vstup(UR_OBJECT user, char *inpstr)
{
int x;
char *ptr;
char *povolene[]={
/* NUTS Staff... */
"\033[0m", "\033[1m", "\033[4m", "\033[5m", "\033[7m", "\033[8m", 
"\033[30m","\033[31m","\033[32m","\033[33m",
"\033[34m","\033[35m","\033[36m","\033[37m",
"\033[40m","\033[41m","\033[42m","\033[43m",
"\033[44m","\033[45m","\033[46m","\033[47m",
/* TTT Staff... */
"\033[0;30m","\033[0;31m","\033[0;32m","\033[0;33m",
"\033[0;34m","\033[0;35m","\033[0;36m","\033[0;37m",
/* FORNAX Staff... */
"\033[1;30m","\033[1;31m","\033[1;32m","\033[1;33m",
"\033[1;34m","\033[1;35m","\033[1;36m","\033[1;37m",
"*"};

if ((ptr=strchr(inpstr,(char)255))!=NULL) {
   *ptr='\0';
   }
ptr=inpstr;

while((ptr=strchr(ptr,'\033'))!=NULL)
 {
  for(x=0;povolene[x][0]!='*';x++)
      if (!strncmp(ptr,povolene[x],strlen(povolene[x]))) break;
  if (povolene[x][0]=='*') *ptr=' ';
  ptr++;
 }
if (!strlen(inpstr)) return 1;
else return 0;
}

/**** Toto je uplne uzasna funkcia, ktora je vsak zdegradovana na parsovanie
      telnet prikazov. Po malej uprave je to vsak o dost lepsi strstr, ktory
      pozna wildmask v podobe otaznika. Pravda, istym problebom by mohlo byt
      zistovanie samotneho otazniku, ale who cares?                ********/

char *strteln(char *string, char *substring, unsigned int stringlen)
{
 char *ptr1,*ptr2,*ptr3;
 int len;

 ptr1=string; ptr2=substring;
 if (!(*ptr2) || ptr1==NULL || ptr2==NULL) return NULL;

 while (stringlen)
   {
    ptr3=ptr1;
    len=stringlen;
    while (*ptr2 && len) {
       if (*ptr1==*ptr2 || *ptr2=='?') { ptr2++; ptr1++; len--; }
       else {ptr2=substring; ptr1=ptr3; break;}
      }
    if (!(*ptr2)) return ptr3;
    if (!len) return NULL;
    ptr1++; stringlen--;
   }
 return NULL;
}

extern void parse_telnet_chars(UR_OBJECT user,char *inpstr, size_t len)
{
char *ptr;
char smalbuf[100];

/* nastavenie terminalu */
sprintf(smalbuf,"%c%c%c%c%c%c%c%c%c",IAC,SB,TELOPT_NAWS,0,'?',0,'?',IAC,SE);
if ((ptr=strteln(inpstr,smalbuf,len))!=NULL) {
   user->colms=(unsigned char) *(ptr+4); /* RIDERFLOW */
   if (user->colms<5) user->colms=5; /* pre istotu :) */
   user->lines=(unsigned char) *(ptr+6); /* RIDERFLOW */
   if (user->lines<5) user->lines=5; /* pre istotu :) */
   if (user->statline==CHARMODE) {
	user->statlcount=user->lines;
	user->lines-=2;
	init_statline(user);
	}

   }
/* Are you there? */
sprintf(smalbuf,"%c%c",IAC,AYT);
if (strteln(inpstr,smalbuf,len)!=NULL) {
   write_user(user,"[Atlantis : yes]\n");
   }

/* Start Linemode */
sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_SGA);
if (strteln(inpstr,smalbuf,len)!=NULL) {
   if (user->statline==UNKNOWN) user->statlcan=1;
   else user->statline=NONEMODE;
   }

sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_ECHO);
if (strteln(inpstr,smalbuf,len)!=NULL) {
   if (user->statlcan==1) boot_statline(user);
   }

/* Do status! */
sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_STATUS);
if (strteln(inpstr,smalbuf,len)!=NULL) {
   sprintf(smalbuf,"%c%c%c",IAC,WILL,TELOPT_STATUS);
   write2sock(NULL,user->socket,smalbuf,0);
   }

sprintf(smalbuf,"%c%c%c",IAC,WILL,TELOPT_NEW_ENVIRON);
if (strteln(inpstr,smalbuf,len)!=NULL) {
   user->remtelopt|=2;
   sprintf(smalbuf,"%c%c%c%c%c%c",IAC,SB,TELOPT_NEW_ENVIRON,TELQUAL_SEND,IAC,SE);
   write2sock(NULL,user->socket,smalbuf,0);
   }

sprintf(smalbuf,"%c%c%c%c%c%c%c%c%c%c",IAC,SB,TELOPT_NEW_ENVIRON,'?',NEW_ENV_VAR
,'U','S','E','R',NEW_ENV_VALUE); if ((user->remtelopt==(user->remtelopt|2)) &&
strteln(inpstr,smalbuf,10)!=NULL) {
   sprintf(smalbuf,"%c%c",IAC,SE);
   if ((ptr=strteln(inpstr,smalbuf,len))!=NULL)
//     if ((strteln(inpstr,"@",len)!=NULL) && (ptr>(inpstr+10))) {
     if (ptr>(inpstr+10)) {
        int sajze=0;
        char meno[100];
        ptr=inpstr+10; /* VALUE */
        while(*ptr!=(char)IAC) {
           meno[sajze]=*ptr;
           sajze++; ptr++;
           if (sajze==99) break;
           }
        meno[sajze]=0;
        strcpy(user->remote_ident,meno);
	sprintf(text,"Remote reply for %s: %s\n", user->name,user->remote_ident);
 	write_syslog(text,1);
        if (!user->level && newuser_siteban(user->remote_ident)) {
          sprintf(text,"~OL~FRZ tejto adresy nie je mozne vytvaranie novych kont!\n");
          write_user(user,text);
          user->dead=666;
         }
        }
    }
} 

extern void parse_remote_ident(UR_OBJECT user,char *rinpstr,int len,int wsock) {
 char smalbuf[200],smallerbuf[100];
 int sajze;

 if (*rinpstr!=(char)255) return; /* nech sa nezdrzuje */
 sprintf(smalbuf,"%c%c%c",IAC,DO,TELOPT_NEW_ENVIRON);
 if (strteln(rinpstr,smalbuf,len)!=NULL) { /* xcu vediet, ci to zvladame... */
    user->remtelopt|=1; /* ze to zvlada ... */
    sprintf(smalbuf,"%c%c%c",IAC,WILL,TELOPT_NEW_ENVIRON);
    write(user->remote_socket[wsock],smalbuf,3); /* a potvrdime to! */
    return;
    }
sprintf(smalbuf,"%c%c%c%c%c%c",IAC,SB,TELOPT_NEW_ENVIRON,TELQUAL_SEND,IAC,SE);
 if (strteln(rinpstr,smalbuf,len)!=NULL) { /* semraci, semruju ident! */

    if (user->remtelopt!=(user->remtelopt|1)) return; /* nepoprosili - nedostanu ;-) */
    sprintf(smalbuf,"%c%c%c%c%c%c%c%c%c%c",IAC,SB,TELOPT_NEW_ENVIRON,TELQUAL_IS,
    NEW_ENV_VAR,'U','S','E','R',NEW_ENV_VALUE); sajze=10; /* dlzka smalbuf-u */
//    strcpy((smalbuf+sajze),user->real_ident); sajze+=strlen(user->real_ident);
    sprintf(smallerbuf,"%s[%s]",user->name,user->site);
    strcpy((smalbuf+sajze),smallerbuf); sajze+=strlen(smallerbuf);
    /* V) CHANGE: ked uz skemraju, poslime im 'Username[site]' */
    *(smalbuf+sajze)=IAC;
    *(smalbuf+sajze+1)=SE;
    sajze+=2;
    write(user->remote_socket[wsock],smalbuf,sajze); return; }
}
