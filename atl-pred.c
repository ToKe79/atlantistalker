#include "atl-head.h"
#include "atl-mydb.h"

P_OBJECT create_vec()
{
P_OBJECT vec;

 if ((vec=(P_OBJECT)malloc(sizeof(struct p_struct)))==NULL)
  {
   write_syslog("HUH: Memory allocation failure in create_predmet().\n",0);
   return NULL;
  }
 vec->name[0]='\0'; 
 vec->dativ[0]='\0';
 vec->akuzativ[0]='\0';
 vec->inytiv[0]='\0';
 vec->type=10;
 vec->food[0]='\0';     
 vec->function=0;
 vec->altfunct=0;
 vec->restrikt=0;
 vec->dur=0;
 vec->amount=0;
 vec->weight=0;
 vec->price=0;
 vec->pp=0;
 vec->enter=NULL;
 vec->leave=NULL;
 vec->uinphr=NULL;
 vec->uoutphr=NULL;
 vec->rinphr=NULL;
 vec->routphr=NULL;
 vec->ustart=NULL;
 vec->rstart=NULL;
 vec->ustop=NULL;
 vec->rstop=NULL;
 vec->userphr=NULL;
 vec->roomphr=NULL;
 vec->victimphr=NULL;
 vec->special=NULL;
 vec->error=NULL;
 vec->attack=0;
 vec->firerate=0;
 vec->seconddur=0;
 vec->tajm=0;
 vec->picture=NULL;
 vec->showpict=0;
 vec->ofense=0;
 vec->defense=0;
 vec->udestroy=NULL;
 vec->rdestroy=NULL;
 vec->spawn=NULL;
 vec->spawnarea=0;
 vec->ujoinphr=NULL;
 vec->rjoinphr=NULL;
 vec->searchphr=NULL;
 return vec;
}

void destruct_vec(P_OBJECT vec)
{
 if (vec->enter!=NULL) free ((void *)vec->enter);
 if (vec->leave!=NULL) free ((void *)vec->leave);
 if (vec->uinphr!=NULL) free ((void *)vec->uinphr);
 if (vec->uoutphr!=NULL) free ((void *) vec->uoutphr);
 if (vec->rinphr!=NULL) free ((void *) vec->rinphr);
 if (vec->routphr!=NULL) free ((void *) vec->routphr);
 if (vec->ustart!=NULL) free ((void *) vec->ustart);
 if (vec->rstart!=NULL) free ((void *) vec->rstart);
 if (vec->ustop!=NULL) free ((void *) vec->ustop);
 if (vec->rstop!=NULL) free ((void *) vec->rstop);
 if (vec->userphr!=NULL) free ((void *) vec->userphr);
 if (vec->roomphr!=NULL) free ((void *) vec->roomphr);
 if (vec->victimphr!=NULL) free ((void *) vec->victimphr);
 if (vec->special!=NULL) free ((void *) vec->special);
 if (vec->error!=NULL) free ((void *) vec->error);
 if (vec->udestroy!=NULL) free ((void *) vec->udestroy);
 if (vec->rdestroy!=NULL) free ((void *) vec->rdestroy);
 if (vec->ujoinphr!=NULL) free ((void *) vec->ujoinphr);
 if (vec->rjoinphr!=NULL) free ((void *) vec->rjoinphr);
 if (vec->searchphr!=NULL) free ((void *) vec->searchphr);

 free(vec);
}

CO_OBJECT create_convert()
{
CO_OBJECT con;
int i;

 if ((con=(CO_OBJECT)malloc(sizeof(struct co_struct)))==NULL)
  {
   write_syslog("HUH: Memory allocation failure in create_convert().\n",0);
   return NULL;
  }
 for(i=0;i<4;i++) con->component[i]=-1;
 con->product=-1;
 con->source=0;
 con->destination=2;
 con->setowner=0;
 for(i=0;i<4;i++) con->eliminate[i]=0;
 con->usermsg=NULL; 
 con->roommsg=NULL;
 con->missing=NULL; 
 con->spell[0]='\0';
 con->mana=0;
 con->heal=0;
 return con;
}

void destruct_convert(CO_OBJECT con)
{
 if (con->usermsg!=NULL) free ((void *)con->usermsg);
 if (con->roommsg!=NULL) free ((void *)con->roommsg);
 if (con->missing!=NULL) free ((void *)con->missing);
 free((void *)con);
}

char *fetchstring(char *str)
{
char *s;
 if (strlen(str)==0) return NULL;
 s = (char *) malloc ((strlen(str)+1)*sizeof(char));
 if (s!=NULL) strcpy(s,str);
 return s;
}

void load_and_parse_predmets(UR_OBJECT user)
{
int num,total,i,p;
RM_OBJECT rm;
UR_OBJECT u;
char pname[MAX_PREDMETY][21];
int maxbefore=0,index;

 wrtype=WR_AVOID_PREDMETS;
 if (user==NULL) printf("Parsujem predmety..\n");
 else write_user(user,"\n~HWParsujem predmety .. ");


   i=0;
   while (convert[i]!=NULL) {
     destruct_convert(convert[i]);
     convert[i]=NULL;
     i++;
    }

   for(i=0;i<MAX_PREDMETY;i++) pname[i][0]='\0';
   i=0;
   while (predmet[i]!=NULL) {
     strcpy(pname[i],predmet[i]->name);
     destruct_vec(predmet[i]);
     predmet[i]=NULL;
     i++;
    }
   maxbefore=i;
   for (i=0;i<MAX_PREDMETY;i++) predmet[i]=NULL;


 total=0;
 sprintf(query,"select `name`,`dativ`,`akuzativ`,`inytiv`,`type`,`food`,`function`,`altfunct`,`restrikt`,`dur`,`amount`,`weight`,`price`,`pp`,`enter`,`leave`,`uinphr`,`uoutphr`,`rinphr`,`routphr`,`ustart`,`rstart`,`ustop`,`rstop`,`userphr`,`roomphr`,`victimphr`,`special`,`error`,`attack`,`firerate`,`seconddur`,`tajm`,`picture`,`showpict`,`ofense`,`defense`,`udestroy`,`rdestroy`,`spawn`,`spawnarea`,`ujoin`,`rjoin`,`search` from `entities` where `disabled`=0 order by `name`;");


 if (!(result=mysql_result(query))) return;
 while ((row=mysql_fetch_row(result))) {
   if ((predmet[total]=create_vec())==NULL) {
     sprintf(text,"FATAL ERROR: Problem s alokovanim pamate pre predmet (index %d).\n",total);
     if (user==NULL) {
       perror(text);
       boot_exit(12);
      }
     else write_user(user,text);
     wrtype=0;
     return;
    }
   num=0;
   sstrncpy(predmet[total]->name,row[num++],sizeof(predmet[total]->name)-1);
   sstrncpy(predmet[total]->dativ,row[num++],sizeof(predmet[total]->dativ)-1);
   sstrncpy(predmet[total]->akuzativ,row[num++],sizeof(predmet[total]->akuzativ)-1);
   sstrncpy(predmet[total]->inytiv,row[num++],sizeof(predmet[total]->inytiv)-1);
   predmet[total]->type=atoi(row[num++]);
   sstrncpy(predmet[total]->food,row[num++],sizeof(predmet[total]->food)-1);
   predmet[total]->function=atoi(row[num++]);
   predmet[total]->altfunct=atoi(row[num++]);
   predmet[total]->restrikt=atoi(row[num++]);
   predmet[total]->dur=atoi(row[num++]);
   predmet[total]->amount=atoi(row[num++]);
   predmet[total]->weight=atoi(row[num++]);
   predmet[total]->price=atoi(row[num++]);
   predmet[total]->pp=atoi(row[num++]);
   predmet[total]->enter=fetchstring(row[num++]);
   predmet[total]->leave=fetchstring(row[num++]);
   predmet[total]->uinphr=fetchstring(row[num++]);
   predmet[total]->uoutphr=fetchstring(row[num++]);
   predmet[total]->rinphr=fetchstring(row[num++]);
   predmet[total]->routphr=fetchstring(row[num++]);
   predmet[total]->ustart=fetchstring(row[num++]);
   predmet[total]->rstart=fetchstring(row[num++]);
   predmet[total]->ustop=fetchstring(row[num++]);
   predmet[total]->rstop=fetchstring(row[num++]);
   predmet[total]->userphr=fetchstring(row[num++]);
   predmet[total]->roomphr=fetchstring(row[num++]);
   predmet[total]->victimphr=fetchstring(row[num++]);
   predmet[total]->special=fetchstring(row[num++]);
   predmet[total]->error=fetchstring(row[num++]);

   predmet[total]->attack=atoi(row[num++]);
   predmet[total]->firerate=atoi(row[num++]);
   predmet[total]->seconddur=atoi(row[num++]);
   predmet[total]->tajm=atoi(row[num++]);
   
   predmet[total]->picture=fetchstring(row[num++]);

   predmet[total]->showpict=atoi(row[num++]);
   predmet[total]->ofense=atoi(row[num++]);
   predmet[total]->defense=atoi(row[num++]);

   predmet[total]->udestroy=fetchstring(row[num++]);
   predmet[total]->rdestroy=fetchstring(row[num++]);
   
   if (strlen(row[num])==0) predmet[total]->spawn=NULL;
   else predmet[total]->spawn=get_room(row[num],NULL);
   num++;
   predmet[total]->spawnarea=atoi(row[num]);
   num++;

   predmet[total]->ujoinphr=fetchstring(row[num++]);
   predmet[total]->rjoinphr=fetchstring(row[num++]);
   predmet[total]->searchphr=fetchstring(row[num++]);

   total++;
  }
 mysql_free_result(result);

 /* preindexujeme aby sa nepomenili predmety v roomach a u userov*/
 if (user!=NULL) {

   for(i=0;i<maxbefore;i++) {
     index=-1;
     p=0;
     while (predmet[p]!=NULL) {
       if (!strcmp(pname[i],predmet[p]->name)) { index=p; break; }
       p++;
      }

     for(rm=room_first;rm!=NULL;rm=rm->next) 
       for(p=0;p<MPVM;p++)
         if (rm->predmet[p]==i) {
	   rm->predmet[p]=index+1000;
	   if (index==-1) rm->dur[p]=0;
	  }
	   
     for(u=user_first;u!=NULL;u=u->next)
       for(p=0;p<BODY;p++)
         if (u->predmet[p]==i) {
	   u->predmet[p]=index+1000;
	   if (index==-1) u->dur[p]=0;
	  }
    }
   for(rm=room_first;rm!=NULL;rm=rm->next) 
     for(p=0;p<MPVM;p++)
       if (rm->predmet[p]>666) rm->predmet[p]-=1000;
     
   for(u=user_first;u!=NULL;u=u->next)
     for(p=0;p<BODY;p++)
       if (u->predmet[p]>666) u->predmet[p]-=1000;
  }
 wrtype=0;
 pocet_predmetov=total;
 
 total=0;
 sprintf(query,"select `component1`,`component2`,`component3`,`component4`,`product`,`source`,`destination`,`setowner`,`eliminate`,`usermsg`,`roommsg`,`missing`,`spell`,`mana`,`health` from `convert` where `disabled`=0 order by `id`");

 if ((result=mysql_result(query))) {
   while ((row=mysql_fetch_row(result))) {
     if ((convert[total]=create_convert())==NULL) {
       sprintf(text,"FATAL ERROR: Problem s alokovanim pamate pre konverzie (index %d).\n",total);
       if (user==NULL) {
         perror(text);
         boot_exit(12);
        }
       else write_user(user,text);
       wrtype=0;
       return;
      }
     for(i=0;i<4;i++) convert[total]->component[i]=expand_predmet(row[i]);
     convert[total]->product=expand_predmet(row[4]);
     convert[total]->source=atoi(row[5]);
     convert[total]->destination=atoi(row[6]);
     convert[total]->setowner=atoi(row[7]);
     p=atoi(row[8]);
     for(i=0;i<4;i++) {
       if (p%10==1) convert[total]->eliminate[3-i]=1;
       p/=10;
      }
     for(i=0;i<4;i++) {
       num=0;
       for(p=0;p<4-i;p++) if (convert[total]->component[p]>convert[total]->component[num]) num=p;
       p=convert[total]->component[num];
       convert[total]->component[num]=convert[total]->component[3-i];
       convert[total]->component[3-i]=p;
       p=convert[total]->eliminate[num];
       convert[total]->eliminate[num]=convert[total]->eliminate[3-i];
       convert[total]->eliminate[3-i]=p;
      }
     convert[total]->usermsg=fetchstring(row[9]);
     convert[total]->roommsg=fetchstring(row[10]);
     convert[total]->missing=fetchstring(row[11]);
     sstrncpy(convert[total]->spell,row[12],sizeof(convert[total]->spell)-1);
     convert[total]->mana=atoi(row[13]);
     convert[total]->heal=atoi(row[14]);
  
     if (convert[total]->product==-1) {
//      || (convert[total]->component[0]==-1 && convert[total]->component[1]==-1
//          && convert[total]->component[2]==-1 && convert[total]->component[3]==-1)) {
       destruct_convert(convert[total]);
       convert[total]=NULL;
      }
     else total++;
    }
   mysql_free_result(result);
  }
 pocet_konverzii=total;
 
 if (user==NULL) {
   sprintf(text,"Hotofka.. %d predmetov, %d konverzii.\n",pocet_predmetov,pocet_konverzii);
   printf(text);
  }
 else {
   sprintf(text,"~HG%d~HW ks, ~HG%d~HW konverzii.\n",pocet_predmetov,pocet_konverzii);
   write_user(user,text);
  }
}

void do_attack(user)
UR_OBJECT user;
{
int i,pocet=0,vec,ran;
char *name;
RM_OBJECT rm;

if (user->room==get_room("letun",NULL) || user->room==get_room("plachetnica",NULL)) return;
if (tsec%15==0)
 {
  for (i=0;i<MPVM;++i)
   if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->type<3)
    pocet++;
  if (pocet==0) return;
  ran=(random()%pocet+1);
  pocet=0;
  for (i=0;i<MPVM;++i)
  {
   if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->type<3)
   { 
    pocet++;
    if (ran==pocet) {
      vec=user->room->predmet[i];
      switch (abs(predmet[vec]->attack)) {
        case 1://macka
         sprintf(texthb,"~FT~OL%s~RS~FT ta doskriabal%s.\n",predmet[vec]->name,zwjpohl(vec,"","a","o","i"));
         sprintf(text,"~FY%s doskriabal%s %s.\n",predmet[vec]->name,zwjpohl(vec,"","a","o","i"),sklonuj(user,4));
         break;
        case 2://had
         sprintf(texthb,"~FT~OL%s~RS~FT ta ustip%s do nohy. Zacala sa ti tocit hlava.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
         sprintf(text,"~FY%s ustip%s %s do nohy.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,4));
         break;
        case 3://squirrel
         sprintf(texthb,"~FT~OL%s~RS~FT ti hodil%s orech rovno do hlavy.\n",predmet[vec]->name,zwjpohl(vec,"","a","o","i"));
         sprintf(text,"~FY%s hodil%s %s orech do hlavy.\n",predmet[vec]->name,zwjpohl(vec,"","a","o","i"),sklonuj(user,3));
         break;
        case 4://topier
         sprintf(texthb,"~FT~OL%s~RS~FT sa ti zahryz%s do krku a cicia ti krv.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
         sprintf(text,"~FY%s sa zahryz%s %s do krku a cicia %s krv.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,3),pohl(user,"mu","jej"));
         break;
        case 5://kon
         sprintf(texthb,"~FT~OL%s~RS~FT ti tak kop%s do hlavy ze vidis same ***.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
         sprintf(text,"~FY%s tak kop%s %s do hlavy ze vidi same ***.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,4));
         break;
        case 6:
         sprintf(texthb,"~FT~OL%s~RS~FT ta pohryz%s.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
         sprintf(text,"~FY%s pohryz%s %s.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,4));
         break;
        case 7:
         sprintf(texthb,"~FT~OL%s~RS~FT ta klof%s.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
         sprintf(text,"~FY%s klof%s %s.\n",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"),sklonuj(user,4));
         break;
       }
      texthb[6 ]=toupper(texthb[6]);
      text[3]=toupper(text[3]);
      write_user(user,texthb);
      wrtype=WR_ZVERY;
      write_room_except(user->room,text,user);
      wrtype=0;
      if (predmet[vec]->altfunct & 2048) user->lsd+=120; /* jedi had ;) */
      if (predmet[vec]->altfunct & 64) user->stars+=30;
      pocet=user->pp-80;
      if (pocet>0) setpp(user,pocet);
       else {
        setpp(user,200);
        user->dead=8;
        sprintf(texthb,"~OL~FRUtrpel%s si smrtelne zranenia.\n",pohl(user,"","a"));
        write_user(user,texthb);
        if (user->vis) name=user->name; else name=invisname(user);
        sprintf(texthb,"~FY%s utrpel%s smrtelne zranenia a ostal%s bezvladne lezat na zemi.\n",name,pohl(user,"","a"),pohl(user,"","a"));
        write_room_except(user->room,texthb,user);
       }
      return;
     }
   }
  }
 }

if (tsec%10==0) {
  pocet=0;
  for (i=0;i<MPVM;++i)
   if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->type<3)
    pocet++;  
  if (pocet<2) for(rm=room_first;rm!=NULL;rm=rm->next)
   {
    if (rm==user->room || rm->group!=2) continue;
    for (i=0;i<MPVM;++i)
     if (rm->predmet[i]>-1 && predmet[rm->predmet[i]]->type<3 && rm->dur[i]<10000 && rm->dur[i]>=0) {
       if (is_free_in_room(rm)==-1) return;
       vec=rm->predmet[i];
       /*if (rm->dur[i]>9999) put_in_room(user->room,vec,rm->dur[i]);*/
       /*else*/ put_in_room(user->room,vec,predmet[vec]->dur);
       rm->predmet[i]=-1;
       rm->dur[i]=0;
       sprintf(text,"~FY%s sa naraz %s neni.\n",predmet[vec]->name,zwjpohl(vec,"splasil a uz ho","splasila a uz jej","splasilo a uz ho",""));
       text[3]=toupper(text[3]);
       wrtype=WR_ZVERY;
       write_room(rm,text);
       if (predmet[vec]->enter!=NULL) {
         sprintf(text,"~FYNevedno odkial %s %s.\n",parse_phrase(predmet[vec]->enter,NULL,NULL,NULL,0),predmet[vec]->name);
         write_room(user->room,text);
        }
       wrtype=0;
       return;
      }
   }  
 }
}

void show_pict_on_event(UR_OBJECT user,int event,int vec,int newline)
{
char filename[81],line[450];
FILE *fp;

 if (vec==-1) return;
 if (predmet[vec]->showpict==0 || predmet[vec]->picture==NULL) return;
 if (!(predmet[vec]->showpict & event)) return;
 if (newline) write_user(user,"\n");
 sprintf(filename,"pictures/%s",predmet[vec]->picture);

 sprintf(query,"select `body` from `files` where `filename`='%s';",predmet[vec]->picture);

 if ((predmet[vec]->showpict & 1)) {
   if (more(user,user->socket,filename)==1) { user->misc_op=2; return; }
   if (sqlmore(user,user->socket,query)==1) { user->misc_op=222; return; }
   return;
  }
 if (!(fp=ropen(filename,"r"))) {
   if ((result=mysql_result(query))) {
     if ((row=mysql_fetch_row(result)) && row[0]) {
       write_user(user,row[0]);
      }
     mysql_free_result(result);
    }
   return;
  }
 fgets(line,440,fp);  
 while(!feof(fp)) {
   write_user(user,line);
   fgets(line,440,fp);
  }
 fclose(fp);
}

void pohaluz_predmetom(char *inpstr,int ok,int vec)
{
int i,which=0,pos=0,cnt=0;
char *newimpstr;
char ins[10][21],tmp[WORD_LEN+1];

if (vec==-1) return;
if (predmet[vec]->altfunct & 32) {
  if (predmet[vec]->special==NULL) return;
  pos=strlen(predmet[vec]->special)/2;
  for(i=0;i<strlen(inpstr);i++) { 
    for(cnt=0;cnt<pos;cnt++) { 
      if (*(inpstr+i)==predmet[vec]->special[cnt*2])
       *(inpstr+i)=predmet[vec]->special[cnt*2+1];
     }
   }
  return;
 }
if ((newimpstr=(char *)malloc(4000*sizeof(char)))==NULL) {
	*inpstr='\0';
	return;
	}
*newimpstr='\0';

if (predmet[vec]->special!=NULL) {
  for (i=0;i<10;i++) ins[i][0]='\0';
  for (i=0;i<strlen(predmet[vec]->special) && which<10;i++) {
    if (predmet[vec]->special[i]==';') {
      if (pos<20) ins[which][pos]='\0';
      else ins[which][20]='\0';
      pos=0;
      which++;
     }
    else if (pos<20) {
      ins[which][pos]=predmet[vec]->special[i];
      pos++;
     }
   }
  if (pos<20) ins[which][pos]='\0';
  else ins[which][20]='\0';
  which++;
}

if (which>0) {
  for (i=ok; i<word_count; i++) {
    strcat(newimpstr,word[i]);
    cnt++;
    if (predmet[vec]->firerate>0 && rand()%100+1<predmet[vec]->firerate)
     { strcat(newimpstr," "); strcat(newimpstr,ins[rand()%which]); }
    if (predmet[vec]->firerate<0 && cnt%abs(predmet[vec]->firerate)==0)
     { strcat(newimpstr," "); strcat(newimpstr,ins[rand()%which]); }
    strcat(newimpstr," ");
   }
 }
else {
  for (i=ok; i<word_count; i++) {
    cnt=strlen(word[i]);
/*    for (cnt=0;cnt<pos;cnt++) {
      tmp[cnt]=word[i][(pos-1)-cnt];
     }
*/  
    for(pos=0;pos<cnt;pos++) {
      if(word[i][pos]=='~' && pos<cnt-2) { 
        tmp[cnt-1-pos]=word[i][pos+2];
        tmp[cnt-2-pos]=word[i][pos+1];
        tmp[cnt-3-pos]='~';
        pos+=2;
       }
      else tmp[cnt-1-pos]=word[i][pos];
     }
    tmp[cnt]='\0';
    strcat(newimpstr,tmp);
    strcat(newimpstr," ");
   }
 }
sstrncpy(inpstr,newimpstr, ARR_SIZE-1);
free((void *)newimpstr);
}

void do_funct_stuff(UR_OBJECT user,char *inpstr,int start)
{
int store=-1,start2;
 
 start2=start;
 if (start==2) start=1;
 if (user->affpermanent>-1 && user->affected>-1) {
   store=user->affpermanent;
   user->affpermanent=-1;
  }
 if (is_affected(user,1024)>-1) pohaluz_predmetom(inpstr,start,is_affected(user,1024));    
 else if (is_affected(user,128)>-1) zachlastaj2(inpstr,start);
 else if (is_affected(user,256)>-1) zalamerizuj(inpstr,start2);
 else if (is_affected(user,512)>-1) debilneho(inpstr,start);
 else if (user->zuje) zazuvackuj(inpstr,start);
 if (is_affected(user,32)>-1) pohaluz_predmetom(inpstr,start,is_affected(user,32));    

 if (store>-1) user->affpermanent=store;
}

int is_affected(UR_OBJECT user,int flag)
{
 if (user->affected>-1 && predmet[user->affected]->altfunct & flag) return user->affected;
 if (user->affpermanent>-1 && predmet[user->affpermanent]->altfunct & flag) return user->affpermanent;
return -1;
}

void do_alt_funct(UR_OBJECT user,int vec)
{
int defa;
 if (vec==-1) return;
 if (predmet[vec]->pp!=0) setpp(user,user->pp+predmet[vec]->pp);
 
   defa=predmet[vec]->tajm;
   if (defa==0) defa=90;
   if (predmet[vec]->altfunct & 64) {
     user->stars+=defa;
     return;
    }
   if (predmet[vec]->altfunct & 2048) {
     user->lsd+=defa;
     return;
    }
   if (predmet[vec]->altfunct & 16384) {
     user->afro+=defa;
     return;
    }
   if (predmet[vec]->altfunct & 8192) {
     user->prehana=1;
     user->prehana_t=10;
     user->prehana_t2=10; /* kolkokrat ho to prezenie ;-) */
     return;
    }
   if (predmet[vec]->altfunct & 32768) {
     user->muzzled=1;
     user->muzzletime=120; 
     user->muzzle_t=vec;
     return;
    }
   if (predmet[vec]->altfunct & 65536) {
     user->viscount=-20;
     return;
    }

 if (predmet[vec]->altfunct & 32
 || predmet[vec]->altfunct & 128 || predmet[vec]->altfunct & 256
 || predmet[vec]->altfunct & 512 || predmet[vec]->altfunct & 1024) {
   if (defa==-1) { user->affpermanent=vec; return; }
   if (user->affected!=vec) user->affecttime=0;
   user->affecttime+=defa;
   user->affected=vec;
  }
}

int forbidden(UR_OBJECT user,UR_OBJECT u,int vec)
{
  if (u==NULL) {
    if (predmet[vec]->restrikt & 16 && !(predmet[vec]->restrikt & 1) && user->afro) {
      sprintf(text,"Ale no tak, jedno afrodiziakum ti nestaci ?!\n");
      write_user(user,text);
      return 1;
     }
    if (predmet[vec]->restrikt & 32 && !(predmet[vec]->restrikt & 1) && user->afro) {
      sprintf(text,"Prehanadlo radsej neuzivaj ked si zalepen%s, mohlo by to zle dopadnut.\n",pohl(user,"y","a"));
      write_user(user,text);
      return 1;
     }
    if (predmet[vec]->restrikt & 256 && !(predmet[vec]->restrikt & 1)) {
      if (!user->vis) {
        sprintf(text,"Nema zmysel pouzit %s, ked uz si %s!\n",predmet[vec]->akuzativ,pohl(user,"neviditelny","neviditelna"));
        write_user(user, text);
        return 1;
       }
      if (user->viscount!=0) {
        write_user(user,"Ved uz miznes!\n");
        return 1;
       }
     }
    if (predmet[vec]->restrikt & 16384 && !(predmet[vec]->restrikt & 1) && user->sex) {
      sprintf(text,"Tento predmet mozu pouzit iba zeny.\n");
      write_user(user,text);
      return 1;
     }
    if (predmet[vec]->restrikt & 32768 && !(predmet[vec]->restrikt & 1) && !user->sex) {
      sprintf(text,"Tento predmet mozu pouzit iba muzi.\n");
      write_user(user,text);
      return 1;
     }
    return 0;
   }
  if ((predmet[vec]->restrikt & 4) && !(predmet[vec]->restrikt & 1) && u->level==NEW) {
    sprintf(text,"Nemozes pouzit %s na novom uzivatelovi!\n",predmet[vec]->akuzativ);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 2 && !(predmet[vec]->restrikt & 1) && u->level>=KIN) {
    sprintf(text,"Nemozes pouzit %s na ochrancovi Atlantidy!\n",predmet[vec]->akuzativ);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 64 && !(predmet[vec]->restrikt & 1) && u->ignfun) {
    sprintf(text,"%s ma nastaveny ignore fun!\n",u->name);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 128 && !(predmet[vec]->restrikt & 1) && u->muzzled) {
    sprintf(text,"%s uz je umlcan%s.\n",u->name,pohl(u,"y","a"));
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 256 && !(predmet[vec]->restrikt & 1) && !u->vis) {
    sprintf(text,"%s je neviditeln%s.\n",u->name,pohl(u,"y","a"));
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 8 && !(predmet[vec]->restrikt & 1) && user->level<u->level) {
    sprintf(text,"%s ma vyssi level ako Ty.\n",u->name);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 16 && !(predmet[vec]->restrikt & 1) && u->afro) {
    sprintf(text,"%s je pod vplyvom afrodiziaka.\n",u->name);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 32 && !(predmet[vec]->restrikt & 1) && u->glue) {
    sprintf(text,"%s je prilepen%s.\n",u->name,pohl(u,"y","a"));
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 512 && !(predmet[vec]->restrikt & 1) && u->room!=NULL && u->room->group!=4) {
    sprintf(text,"Tento predmet mozno pouzit iba na ostrove brutalis.\n");
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 1024 && !(predmet[vec]->restrikt & 1) && check_ignore_user(user,u)) {
    sprintf(text,"%s ta ignoruje.\n",u->name);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 2048 && !(predmet[vec]->restrikt & 1) && check_notify_user(u,user)) {
    sprintf(text,"%s ta nema v notify.\n",u->name);
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 4096 && !(predmet[vec]->restrikt & 1) && u->sex) {
    sprintf(text,"Tento predmet mozes pouzit len na zenu.\n");
    write_user(user,text);
    return 1;
   }
  if (predmet[vec]->restrikt & 8192 && !(predmet[vec]->restrikt & 1) && !u->sex) {
    sprintf(text,"Tento predmet mozes pouzit len na muza.\n");
    write_user(user,text);
    return 1;
   }
 return 0;
}

void hurt(UR_OBJECT user,UR_OBJECT u,int vec)
{
int ofense=0,i,wounds=0,protect,condition,bonus=0,equi=0,before;

  ofense=predmet[vec]->ofense;
  wounds=ofense*100;
  for(i=0;i<BODY-1;i++) if (u->predmet[i]>-1) {
    protect=u->predmet[i];
    if (predmet[protect]->defense>-1 && i<HANDS) continue;
    if (predmet[protect]->defense==0) continue;
    condition=(u->dur[i]*50)/predmet[protect]->dur+50;
    wounds-=(wounds*abs(predmet[protect]->defense)*condition)/10000;
    u->dur[i]--;
    if (u->dur[i]==0) {
      if (predmet[protect]->udestroy!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[protect]->udestroy,u,user,NULL,0));
        write_user(u,text);
       }
      if (predmet[protect]->rdestroy!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[protect]->rdestroy,u,user,NULL,10));
        write_room_except(u->room,text,u);
       }
     }
   }
  before=u->health;
  wounds=wounds/100;
  if (wounds>0) u->health-=wounds;
  if (u->health<10) {
    if (user!=NULL && before>9) {
      if (user->team!=u->team || u->team==0) {
        user->kills++;
        add_point(user,DB_BRUTALIS,1,0);
        if (user->kills>0) {
          bonus=((4*(u->kills-user->kills))/(u->kills+user->kills));
         }
        if (user->gold/200<1) equi=5-5*(user->gold/200);
         
        if (user->kills>=0) user->gold+=10+bonus+equi;
       }
      else {
        user->kills--;
        add_point(user,DB_BRUTALIS,-1,0);
        if (user->gold>=8) user->gold-=8;
       }
     }
    if (u->health<1) {
      u->health=0;
      u->deathtype=vec;
      u->dead=14;
      u->deaths++;
      add_point(u,DB_BRUTALIS,-1,0);
     }
    else u->deathtype=vec;
    if (predmet[vec]->uoutphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->uoutphr,u,user,u->room,0));
      write_user(u,text);
     }
    if (predmet[vec]->routphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->routphr,u,user,u->room,10));
      write_room_except(u->room,text,u);
     }
    
   }
}

int convert_predmet(UR_OBJECT user,int spell)
{
int i,foundh[4],foundr[4],pinput[4],cnt=0,p,num,ic=-1;
int hands[HANDS],room[MPVM],hks=0,rks=0,ks=0;

if (spell==1) {
  ic=-1;
  for(i=0;i<pocet_konverzii;i++)
    if (!strcmp(convert[i]->spell,word[1])) ic=i;
  if (ic==-1) return 0;
  

  for(i=0;i<4;i++) {
    pinput[i]=convert[ic]->component[i];
    if (pinput[i]>-1) cnt++;
   }
 }

 for(i=0;i<HANDS;i++) hands[i]=0;
 for(i=0;i<MPVM;i++) room[i]=0;
 for(i=0;i<4;i++) {
   foundh[i]=-1;
   foundr[i]=-1;
  }
 
 if (spell==0) {
   for(i=0;i<4;i++) {
     pinput[i]=-1;
     if (word[1+i][0]) {
       pinput[i]=expand_predmet(word[1+i]);
       if (pinput[i]>-1) cnt++;
      }
    }
   if (cnt==0) return 0;
 
   for(i=0;i<3;i++) {
     num=0;
     for(p=0;p<4-i;p++) if (pinput[p]>pinput[num]) num=p;
     p=pinput[num];
     pinput[num]=pinput[3-i];
     pinput[3-i]=p;
    }
 
   for(i=0;i<pocet_konverzii;i++) {
     if (convert[i]->spell[0]) continue;
     if (convert[i]->component[0]==pinput[0] && convert[i]->component[1]==pinput[1]
      && convert[i]->component[2]==pinput[2] && convert[i]->component[3]==pinput[3])
       { ic=i; break; }
    }
   if (ic==-1) return 0;
  }

 if (convert[ic]->source==0) {
   for(i=4-cnt;i<4;i++) { 
     for(p=0;p<HANDS;p++) if (user->predmet[p]==pinput[i] && hands[p]==0) {
       foundh[i]=p;
       hands[p]=1;
       if (convert[ic]->eliminate[i]) hks++;
       ks++;
       break;
      }
    }
  }
 else if (convert[ic]->source==1) {
   for(i=4-cnt;i<4;i++) { 
     for(p=0;p<MPVM;p++) if (user->room->predmet[p]==pinput[i] && room[p]==0) {
       foundr[i]=p;
       room[p]=1;
       if (convert[ic]->eliminate[i]) rks++;
       ks++;
       break;
      }
    }
  }
 else {
   for(i=4-cnt;i<4;i++) { 
     for(p=0;p<HANDS;p++) if (user->predmet[p]==pinput[i] && hands[p]==0) {
       foundh[i]=p;
       hands[p]=1;
       if (convert[ic]->eliminate[i]) hks++;
       ks++;
       break;
      }
     if (foundh[i]==-1) for(p=0;p<MPVM;p++) if (user->room->predmet[p]==pinput[i] && room[p]==0) {
       foundr[i]=p;
       room[p]=1;
       if (convert[ic]->eliminate[i]) rks++;
       ks++;
       break;
      }
    }
  }
 if (ks!=cnt) {
   if (convert[ic]->missing!=NULL) {
     sprintf(text,"%s\n",parse_phrase(convert[ic]->missing,user,user,user->room,0));
     write_user(user,text);
    }
   else {
     if (ks==0) write_user(user,"Ved nemas ani jeden z predmetov!\n");
     else if (ks==cnt-1) write_user(user,"Chyba ti este jeden predmet!\n");
     else if (ks==cnt-2) write_user(user,"Chybaju ti este dva predmety!\n");
     else if (ks==cnt-3) write_user(user,"Chybaju ti este tri predmety!\n");
    }
   return 1; 
  }
 if (convert[ic]->destination==0 && is_free_in_hands(user)==-1 && hks==0) {
   write_user(user,"Mas plne ruky, musis nieco zahodit.\n");
   return 1;
  }
 if (convert[ic]->destination==1 && is_free_in_room(user->room)==-1 && rks==0) {
   write_user(user,"Uz tu nie je dost miesta, zahod nieco do vedlajsej miestnosti.\n");
   return 1;
  }
 if (convert[ic]->heal>0 && user->health-convert[ic]->heal<15) {
   sprintf(text,"Nevladzes.. si zranen%s.\n",pohl(user,"y","a"));
   write_user(user,text);
   return 1;
  }
 if (spell==1) {
   if (user->mana<convert[ic]->mana) {
     user->mana=0;
     switch(rand()%3) {
       case 0: write_user(user,"~FGKuzlo sa takmer podarilo, no zlyhalo na nedostatku tvojej magickej energie...\n");
       break;
       case 1: write_user(user,"~FGKuzlo bolo spravne, no tvoja sila je primala...\n");
       break;
       case 2: write_user(user,"~FGVzduch sa zachvel, no tvoje sily na dokoncenie kuzla nestacili...\n");
       break;
      }
     return 1;
    }
   else user->mana-=convert[ic]->mana;  
  }
 user->health-=convert[ic]->heal;
 if (convert[ic]->usermsg!=NULL) {
   sprintf(text,"%s\n",parse_phrase(convert[ic]->usermsg,user,NULL,user->room,0));
   write_user(user,text);
  } 
 if (convert[ic]->roommsg!=NULL) {
   sprintf(text,"%s\n",parse_phrase(convert[ic]->roommsg,user,NULL,user->room,0));
   write_room_except(user->room,text,user);
  } 
 for(i=4-cnt;i<4;i++) if (convert[ic]->eliminate[i]) {
   if (foundh[i]>-1) user->predmet[foundh[i]]=-1;
   else if (foundr[i]>-1) user->room->predmet[foundr[i]]=-1;
  }
 if (convert[ic]->destination==0) {
   p=put_in_hands(user,convert[ic]->product,predmet[convert[ic]->product]->dur);
   if (p>-1 && convert[ic]->setowner) {
     user->dur[p]=10000+user->socket;
     if (predmet[convert[ic]->product]->function==24)
      user->dur[p]=predmet[convert[ic]->product]->dur*10000+user->socket;
    }
  }
 else if (convert[ic]->destination==1) {
   p=put_in_room(user->room,convert[ic]->product,predmet[convert[ic]->product]->dur);
   if (p>-1 && convert[ic]->setowner) {
     user->room->dur[p]=10000+user->socket;
     if (predmet[convert[ic]->product]->function==24)
      user->room->dur[p]=predmet[convert[ic]->product]->dur*10000+user->socket;
    }
  }
 carry_refresh(user);
 return 2;
}

void use_predmet(UR_OBJECT user,char *inpstr)
{
int vec=-1,vec2=-1,dur=0,dur2=0,kder=-1,kdeh=-1,pos2=-1;
UR_OBJECT u=NULL;
int i,nah,where,pos;
char type[50], *name;

if (word_count<2) {
	write_user(user,"Pouzi: .use <predmet> [<uzivatel> | <predmet>]\n");
	return;
	}
if (convert_predmet(user,0)) return;

if (user->vis) name=user->name; else name=invisname(user);

  if (!strncmp("riadenie",word[1],strlen(word[1])) || !strncmp("letun",word[1],strlen(word[1]))) {
    if (user->room!=get_room(FLYER_ROOM,NULL)) {
      write_user(user,"Tu niet co riadit, skus vojst do letuna...\n");
      return;
     }
    if (flyer.pozicia) {
      if (!strcmp(user->name,flyer.pilot)) 
       write_user(user,"Co vymyslas, ved letun prave riadis!\n");
      else
       write_user(user,"Bohuzial, teraz letun nemozes ovladat...\n");
      return;
     }
    write_user(user,"~OL~FGOK: riadenie letunu je teraz v ~LItvojich~RS~OL~FG rukach!\n");
    sprintf(text,"~OL~FG%s sa %s riadenia letunu...\n",name,pohl(user,"ujal","ujala"));
    write_room(get_room(FLYER_ROOM,NULL),text);
    strcpy(flyer.pilot, user->name);
    flyer.pozicia=1;
    flyer.gotta_write=1;
    return;
   }

vec=expand_predmet(word[1]);
if (vec==-1) {
  sprintf(text,"Predmet '%s' neexistuje.\n",word[1]);
  write_user(user,text);
  return;
 }
pos=is_in_hands(user,vec);
where=is_in_room(user->room,vec);

if (predmet[vec]->function==24) {  /* zombie-like entities */
  where=-1;
  for(i=0;i<MPVM;i++)
   if (user->room->predmet[i]==vec && user->room->dur[i]>9999
   && user->room->dur[i]%10000==user->socket)
    where=i;
  
  if (where>-1) {
    if (word_count<3) {
      if (user->target==-1) {
        sprintf(text,"Na koho chces poslat %s ?\n",predmet[vec]->akuzativ);
        write_user(user,text);
       }
      else {
        u=get_user_by_sock(user->target);
        if (predmet[vec]->ustop!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,u,user->room,0));
          write_user(user,text);
         }
        if (predmet[vec]->rstop!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,u,user->room,10));
          write_room_except(user->room,text,user);
         }
        user->target=-1;
       }
      return;
     }
    u=get_user(word[2]);
    if (u==NULL) {
      write_user(user,notloggedon);
      return;
     }
    if (u==user) {
      sprintf(text,"Nemozes pouzit %s na seba!\n",predmet[vec]->akuzativ);
      write_user(user,text);
      return;
     }
    if (user->target>-1 && u->socket==user->target) {
      if (predmet[vec]->ustop!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,u,user->room,0));
        write_user(user,text);
       }
      if (predmet[vec]->rstop!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,u,user->room,10));
        write_room_except(user->room,text,user);
       }
      user->target=-1;
      return;
     }
    if (user->room!=u->room) {
      sprintf(text,"%s nie je s tebou v miestnosti!\n",u->name);
      write_user(user,text);
      return;
     }
    user->target=u->socket;
    if (predmet[vec]->userphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,u,user->room,0));
      write_user(user,text);
     }
    if (predmet[vec]->victimphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->victimphr,user,u,user->room,10));
      write_user(u,text);
     }
    if (predmet[vec]->roomphr!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,u,user->room,10));
      write_room_except2users(user->room,text,user,u);
     }
    return;
   }
  sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
  write_user(user,text);
  return;
 }

if ((pos==-1) && (predmet[vec]->function!=1) && (predmet[vec]->function!=16) // VEHICLE
&& (predmet[vec]->function!=20) && (predmet[vec]->function!=21) && (predmet[vec]->function!=22)) { 
  if (where>-1 && predmet[vec]->function==5) {
    for(i=0;i<MPVM;i++)
     if(user->room->predmet[i]==vec && user->room->dur[i]%10000==user->socket) where=i;
    dur=user->room->dur[where];
    u=get_user_by_sock(dur%10000);
    if (u==user) {
      if (predmet[vec]->ustop!=NULL) {
        sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->ustop,user,NULL,NULL,0));
        write_user(user,text);
       }
      if (predmet[vec]->rstop!=NULL) {
        sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->rstop,user,NULL,NULL,10));
        write_room_except(user->room,text,user);
       }
      user->room->dur[where]=default_dur(vec);
      return;
     }
    else if (u!=NULL) {
      if (predmet[vec]->error!=NULL) {
        sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->error,user,u,NULL,0));
        write_user(user,text);
       }
      return;
     }
    if (predmet[vec]->food[0]) vec2=expand_predmet(predmet[vec]->food);
    else vec2=-1;
    if (vec2>-1)
     if (is_in_hands(user,vec2)==-1) {
       sprintf(text,"~FGPotrebujes %s!\n",predmet[vec2]->akuzativ);
       write_user(user,text);
       return;
      }
    if (user->lieta) {
      sprintf(text,"~FGTazko ked prave lietas..\n");
      write_user(user,text);
      return;
     }
    show_pict_on_event(user,32,vec,0);
    if (predmet[vec]->ustart!=NULL) {
      sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
      write_user(user,text);
     }
    if (predmet[vec]->rstart!=NULL) {
      sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
      write_room_except(user->room,text,user);
     }
    user->room->dur[where]=user->socket+10000;
    for(i=0;i<MPVM;i++)
     if (user->room->predmet[i]>-1 && i!=where && user->room->dur[i]%10000==user->socket)
      user->room->dur[i]=default_dur(user->room->predmet[i]);
    return;
   }
  sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
  write_user(user,text);
  return;
 }
if (pos==-1) dur=0;
else dur=user->dur[pos];

if (word_count>2) {
  vec2=expand_predmet(word[2]);
  kder=is_in_room(user->room,vec2);
  if (kder>-1) dur2=user->room->dur[kder];
  kdeh=is_in_hands(user,vec2);
  if (kdeh>-1) dur2=user->dur[kdeh];
  u=get_user(word[2]);
 }
 
if (word_count>2) switch (predmet[vec]->function) { // ine parametre
  case 14:
    if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
      write_user(user,noswearing);
      sprintf(text,"~OL~FR\07[CENZURA: %s sa %s skaredo telefonovat!]~RS\n",user->name,pohl(user,"pokusil","pokusila"));
      writesys(WIZ,1,text,user);
      return;
     }
    if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
      write_user(user,nocolors);
      return;
     }
    if (user->muzzled) {
      sprintf(text,"Mas nasadeny nahubok, nemozes pouzit %s!\n",predmet[vec]->akuzativ);
      write_user(user,text);
      return;
     }
    if (user==user_first && user->next==NULL) {
      sprintf(text,"Nikto okrem Teba tu nieje! Nemozes pouzit %s\n",predmet[vec]->name);
      write_user(user,text);
      return;
     }
    show_pict_on_event(user,32,vec,0);
    inpstr=remove_first(inpstr);	      	
    sprintf(text,"~OLTelefonujes vsetkym:~RS %s\n",inpstr);
    write_user(user,text);
    if (user->vis) name=user->name; else name=invisname(user);	
    strcpy(type,pohl(user,"\253Ty","\253TY"));
		
    sprintf(text,"~OL~FW\252C3%s ~OL~FW\252C4%s:~RS~FW %s\n",name,type,inpstr);

    for(u=user_first;u!=NULL;u=u->next) {	
      if (u->login || u->room==NULL || u->afk || u->ignall || u->malloc_start
      || u->filepos || u->ignfun || u->room->group!=user->room->group
      || u->igntell || check_ignore_user(user,u) || u==user 
      || u->type!=USER_TYPE || u->dead) continue;
      write_user(u,text);
      strcpy(u->lasttellfrom,name);
      if (user->vis) u->lasttg=-1; else u->lasttg=user->sex;
      strcpy(user->lasttellto,u->name);
      record_tell(u,text);   
     }
    	
    if (dur<2 && predmet[vec]->error!=NULL) {
      sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));
      write_user(user,text);
     }
    remove_from_hands(user,vec,1);
    do_alt_funct(user,vec);
    return;	
  default:;
 }

if (word_count>2) {
 if (((u==NULL && vec2>-1)) || ((u!=NULL) && (kder>-1 || kdeh>-1))) { // 2 parametre - predmet, predmet  
  if (kder==-1 && kdeh==-1) {
    if (predmet[vec2]->type<3)
     sprintf(text,"Ziadn%s %s sa tu momentalne nenachadza%s.\n",zwjpohl(vec2,"y","a","e","e"),predmet[vec2]->name,zwjpohl(vec2,"","","","ju"));
    else {
      if (rand()%3==0) sprintf(text,"Hmm.. tak takto to asi nepojde.\n");
      else sprintf(text,"Takto %s nemozes pouzit.\n",predmet[vec]->akuzativ);
     }
    write_user(user,text);
    return;
   }
  if (vec2>-1 && predmet[vec2]->type<3 && !strcmp(predmet[vec2]->food,predmet[vec]->name)) {
    if (kder>-1 || kdeh>-1) {
      show_pict_on_event(user,32,vec,0);
      nah=rand()%2;
      if (!strcmp(predmet[vec]->name,"netopier")) {
       sprintf(text,"~FY%s sa vrh%s po netopierovi.. ostali z neho len kridla.\n",predmet[vec2]->name,zwjpohl(vec2,"ol","la","lo","li"));
       text[3]=toupper(text[3]);
       write_user(user,text);
       put_in_hands(user,expand_predmet("kridla"),default_dur(expand_predmet("kridla")));
      }
      else {
       sprintf(text,"~FYDal%s si %s %s.. %s si na %s pochutnal",pohl(user,"","a"),predmet[vec2]->dativ,predmet[vec]->akuzativ,predmet[vec2]->name,zwjpohl(vec,"nom","nej","nom","nich"));
       strcat(text,zwjpohl(vec2," ","a ","o ","i "));
       if (nah==0)
        strcat(text,zwjpohl(vec2,"a oblizol ti ruku.\n","a oblizla ti ruku.\n","a oblizlo ti ruku.\n","a oblizli ti ruku.\n"));
       else 
        strcat(text,zwjpohl(vec2,"a pritulil sa k tebe.\n","a pritulula sa k tebe.\n","a pritulilo sa k tebe.\n","a pritulili sa k tebe.\n"));
       write_user(user,text);
      }
      remove_from_hands(user,vec,0);
      if (is_in_hands(user,vec2)>-1) {
        user->dur[is_in_hands(user,vec2)]=10000+user->socket;
       }
      else if (is_in_room(user->room,vec2)>-1) {
        if (nah==0)
         sprintf(text,"~FY%s obliz%s %s ruku.\n",predmet[vec2]->name,zwjpohl(vec2,"ol","la","lo","li"),sklonuj(user,3));
        else
         sprintf(text,"~FY%s sa pritulil%s k %s.\n",predmet[vec2]->name,zwjpohl(vec2,"","a","o","i"),sklonuj(user,3));
        text[3]=toupper(text[3]);
        wrtype=WR_ZVERY;
        write_room_except(user->room,text,user);
        wrtype=0;
        user->room->dur[is_in_room(user->room,vec2)]=10000+user->socket;
       }
      /* V) zver ma svojho noveho pana :) */
      return;
     }
   }
  switch (predmet[vec]->function) {
   case 9:
    strtolower(word[2]);
    if (word_count>2) vec2=expand_predmet(word[2]);
    else vec2=-1;
    if (vec2>-1 && predmet[vec2]->type<3) {
      if (is_in_room(user->room,vec2)>-1) {
        if (predmet[vec2]->function==24) { // mlatenie ZOMBIKA
          if (user->attacking) {
            sprintf(text,"Este si nedokoncil%s utok.\n",pohl(user,"","a"));
            write_user(user,text);
            return;
           }
          show_pict_on_event(user,32,vec,0);
          if (user->health<15) {
            sprintf(text,"Si tazko zranen%s, nevladzes..\n",pohl(user,"y","a"));
            write_user(user,text);
            return;
           }
          if (dur==0) {
            if (predmet[vec]->ustop) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,u,NULL,0));
              write_user(user,text);
             }
            if (predmet[vec]->rstop!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,u,NULL,10));
              write_room_except(user->room,text,user);
             }
            return;
           }
          sendvec=vec2;
          if (predmet[vec]->userphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,NULL,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->roomphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,NULL,NULL,10));
            write_room_except(user->room,text,user);
           }
          sendvec=-1;
          i=remove_from_hands(user,vec,1);
          if (i==1) {
            if (predmet[vec]->udestroy) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->udestroy,user,u,NULL,0));
              write_user(user,text);
             }
            if (predmet[vec]->rdestroy!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->rdestroy,user,u,NULL,10));
              write_room_except(user->room,text,user);
             }
            if (predmet[vec]->food[0]) {
              user->predmet[pos]=vec;
              user->dur[pos]=0;
             }
           }
          user->attacking+=predmet[vec]->seconddur;

          nah=dec_dur(user->room->dur[kder],predmet[vec]->ofense);
	  user->room->dur[kder]=nah;
          if (nah==0) {
	    user->kills++;
            add_point(user,DB_BRUTALIS,1,0);
            if (predmet[vec2]->rdestroy!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec2]->rdestroy,user,NULL,user->room,0));
              write_room(user->room,text);
             }
            user->gold+=10;
	    user->room->predmet[kder]=-1;
	    user->room->dur[kder]=0;
           }
          else { /* ak biju zombika, bude sa branit */
            if (user->room->dur[kder]<10000) user->room->dur[kder]=-10000*abs_dur(user->room->dur[kder])-user->socket;
            else if (abs_dur(user->room->dur[kder])<predmet[user->room->predmet[kder]]->dur/3) {
              u=get_user_by_sock(user->room->dur[kder]%10000);
              if (u!=NULL && u->target==-1) user->room->dur[kder]=-10000*abs_dur(user->room->dur[kder])-user->socket;
             }
           } 
	  return;
	 }
        if (predmet[vec2]->attack<0) {
          show_pict_on_event(user,32,vec,0);
          if (!strcmp(predmet[vec2]->name,"netopier"))
           sprintf(text,"~FYPriblizil%s si sa k netopierovi, ale ten vzal kridla na plecia.\n",pohl(user,"","a"));
          else
           sprintf(text,"~FYPriblizil%s si sa k %s, ale %s nohy na plecia.\n",pohl(user,"","a"),predmet[vec2]->dativ,zwjpohl(vec2,"ten vzal","ta vzala","to vzalo",""));
          write_user(user,text);
          user->room->dur[is_in_room(user->room,vec2)]=0;
          do_predmety(1);
          return;
         }
        show_pict_on_event(user,32,vec,0);
        sprintf(text,"~FYZahnal%s %s. Uz viac behat nebude.\n",pohl(user," si sa a sakredo si ublizil","a si sa a skaredo si ublizila"),predmet[vec2]->dativ);
        write_user(user,text);
        sprintf(text,"~FY%s skaredo ublizil%s %s.\n",name,pohl(user,"","a"),predmet[vec2]->dativ);
        wrtype=WR_ZVERY;
        write_room_except(user->room,text,user);
        wrtype=0;
        remove_from_room(user->room,vec2,1);
       }
      else {
        sprintf(text,"~FYZiadn%s %s sa tu %s momentalne nenachadza%s.\n",zwjpohl(vec2,"y","a","e","e"),predmet[vec2]->name,user->room->where,zwjpohl(vec2,"","","","ju"));
        write_user(user,text);
       }
     }
    return;
    default: {
      if (predmet[vec]->function==23) {
        word_count=2;
        break;
       }
      if (predmet[vec2]->type<3) {
        if (rand()%3==0)
         sprintf(text,"~FY%s ta uplne ignoruje.\n",predmet[vec2]->name);
        else
         sprintf(text,"~FY%s sa na teba nechapavo pozera.\n",predmet[vec2]->name);
        text[3]=toupper(text[3]);
       }
      else {
        if (rand()%3==0) sprintf(text,"Hmm.. tak takto to asi nepojde.\n");
        else sprintf(text,"Takto %s nemozes pouzit.\n",predmet[vec]->akuzativ);
       }
      write_user(user,text);
      return;
     }
   }
 }
else { // 2 parametre - predmet, user
  if (u==NULL) {
    write_user(user,notloggedon);
    return;
   }
  if (u==user) word_count=2;
  else {
    if (!strcmp(u->name,"Smsbot")) {
      write_user(user,"Prepac, s tymto uzivatelom to nejde ...\n");
      return;
     }
    if (user->room!=u->room) {
      sprintf(text,"%s nie je s tebou %s.\n",u->name,user->room->where);
      write_user(user,text);
      return;
     }
    if (!strcmp(u->room->name,"svatyna")) {
      sprintf(text,"%s prave medituje.\n",u->name);
      write_user(user,text);
      return;
     }
    if (u->ignall) {
      if (u->filepos) sprintf(text,"%s prave cita nejaky text a nechce byt rusen%s.\n",u->name,pohl(u,"y","a"));
      else if (u->malloc_start!=NULL) sprintf(text,"%s prave edituje nejaky text, a nechce byt rusen%s.\n",u->name,pohl(u,"y","a"));
      else sprintf(text,"%s teraz vsetkych ignoruje.\n",u->name);
      write_user(user,text);
      return;
     }	
    switch (predmet[vec]->function) {
      case 1:
        if (is_in_hands(user,vec)==-1) {
          sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
          write_user(user,text);
          return;
         }
        if (u->level>=user->level) {
          write_user(user,"Mozes zalepit len uzivatela s nizsim levelom ako mas!\n");
          return;
         }
        if (user->dur[is_in_hands(user,vec)]>predmet[vec]->dur-10 && predmet[vec]->error!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));
          write_user(user,text);
          return;
         }
        show_pict_on_event(user,32,vec,0);
        show_pict_on_event(u,32,vec,1);
        if (user->glue) {
          u->glue+=120;
          sprintf(text,"%s uz %s ",u->name,pohl(u,"bol zalepeny, tak si ho","bola zalepena, tak si ju"));
          write_user(user,text);
          sprintf(text,"zalepil%s este viac!\n",pohl(user,"","a"));
          write_user(user,text);
          sprintf(text,"%s %s ta este viac!\n",name,pohl(user,"prilozil ruku k dielu a zalepil","prilozila ruku k dielu a zalepila"));
          write_user(u,text);
         }
        else {
          u->glue=240;
          sprintf(text,"%s si %s! Teraz sa par minut odtialto nepohne.\n",pohl(user,"Zalepil","Zalepila"),sklonuj(u, 4));
          write_user(user,text);
          sprintf(text,"%s Ta %s k zemi! Teraz sa par minut nemozes ani pohnut!\n",name,pohl(user,"prilepil","prilepila"));
          write_user(u,text);
         }
        remove_from_hands(user,vec,0);
        return;
      case 3:
	if (is_in_hands(u,expand_predmet("amulet"))>-1) {
	  write_user(user,"Obet ma na sebe magicky ziariaci amulet, ktory okrem ineho odhana upirov! ;>\n");
	  return;		
	 }
	nah=rand()%3;
	if ((!u->afk) && (nah==2)) {
	  sprintf(text,"%s Ta tak silno %s po hlave, ze si to 2. krat dobre rozmyslis!\n",u->name,pohl(u,"prastil","prastila"));
	  write_user(user,text);
	  setpp(user,0);
	  return;
	 }
	if (!u->afk) {
	  nah=rand()%10+1;
	  if (u->pp-nah<10) {
	    write_user(user,"Obet je vycicana az-az!\n");
	    return;
	   }
          show_pict_on_event(user,32,vec,0);
          show_pict_on_event(u,32,vec,1);
	  write_user(user,"Bohuzial, obet sa rychlo spamatala a prekazila ti tvoju upirsku robotu.\n");
	  sprintf(text,"Za ten cas si ale %s vycicat (sice malo, ale predsa) %d %s.\n",pohl(user,"stihol","stihla"),nah,skloncislo(nah,"bod","body","bodov"));
	  write_user(user,text);	
	  setpp(u,u->pp-nah);		
	  setpp(user,user->pp+nah);
	  sprintf(text,"~OL~FM%s ti %s niekolko pp-bodov, pozor!\n",name,pohl(user,"vycical","vycicala"));
	  write_user(u,text);
	 }
	if (u->afk) {
	  nah=rand()%40+10;
	  if (u->pp-nah<10) {
	    write_user(user,"Obet je vycicana az-az!\n");
	    return;
	   }
          show_pict_on_event(user,32,vec,0);
	  sprintf(text,"Vyborne, toto bola dobra duchom-nepritomna obet. %s si sa do jej tela a\n",pohl(user,"Zakusol","Zakusla"));
	  write_user(user,text);
	  sprintf(text,"%s si vyciciavat pp-cky ... Podarilo sa ti vycicat %d pp-bodov!\n",pohl(user,"zacal","zacala"),nah);		
	  write_user(user,text);				
	  setpp(u,u->pp-nah);	
	  setpp(user,user->pp+nah);
	 }
	nah=rand()%4;
	if (nah==2) {
	  sprintf(text,"%s sa zasek%s v tele obete! ",predmet[vec]->name,zwjpohl(vec,"ol","la","lo","li"));
          if (!strcmp(predmet[vec]->name,"zuby")) strcat(text,"Len-len sa ti podarilo vyslobodit si hlavu ;>>");
          strcat(text,"\n");
          text[0]=toupper(text[0]);
	  write_user(user,text);				
          remove_from_hands(user,vec,0);
          if (is_free_in_hands(u)>-1) put_in_hands(u,vec,dur);
         }
	return;
      case 8:
        if (forbidden(user,u,vec)) return;
        show_pict_on_event(user,32,vec,0);
        show_pict_on_event(u,32,vec,1);
        nah=0;
	if (rand()%100+1<predmet[vec]->seconddur) nah=1;
        remove_from_hands(user,vec,1);
        if (predmet[vec]->userphr!=NULL) {
  	  sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,u,NULL,1));
	  write_user(user,text);
         }
        if (predmet[vec]->victimphr!=NULL) {
  	  sprintf(text,"%s\n",parse_phrase(predmet[vec]->victimphr,user,u,NULL,11));
	  write_user(u,text);
          do_alt_funct(u,vec);
         }
        if (predmet[vec]->roomphr!=NULL) {
  	  sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,u,NULL,11));
	  write_room_except2users(user->room,text,user,u);
         }
	if (nah==1 && predmet[vec]->error!=NULL) {
  	  sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,1));
	  write_user(user,text);
          do_alt_funct(user,vec);
          remove_from_hands(user,vec,0);
	 }
        else if ((predmet[vec]->altfunct & 8) && (dur==1)) {
          if (predmet[vec]->udestroy!=NULL) {
    	    sprintf(text,"%s\n",parse_phrase(predmet[vec]->udestroy,user,u,NULL,1));
  	    write_user(user,text);
           }
          if (predmet[vec]->rdestroy!=NULL) {
    	    sprintf(text,"%s\n",parse_phrase(predmet[vec]->rdestroy,user,u,NULL,11));
  	    write_room_except(user->room,text,user);
           }
         }
	return;
      case 7:
      case 9:
        if (user->attacking) {
          sprintf(text,"Este si nedokoncil%s utok.\n",pohl(user,"","a"));
          write_user(user,text);
          return;
         }
        if (forbidden(user,u,vec)) return;
        show_pict_on_event(user,32,vec,0);
        show_pict_on_event(u,32,vec,1);
        if (user->health<20) {
          sprintf(text,"Si tazko zranen%s, nevladzes..\n",pohl(user,"y","a"));
          write_user(user,text);
          return;
         }
        if (dur==0) {
          if (predmet[vec]->ustop) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,u,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->rstop!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,u,NULL,10));
            write_room_except(user->room,text,user);
           }
          return;
         }
        if (predmet[vec]->userphr!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,u,NULL,1));
          write_user(user,text);
         }
        if (predmet[vec]->victimphr!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->victimphr,user,u,NULL,11));
          write_user(u,text);
          do_alt_funct(u,vec);
         }
        if (predmet[vec]->roomphr!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,u,NULL,11));
          write_room_except2users(user->room,text,user,u);
         }
        hurt(user,u,vec);
        i=remove_from_hands(user,vec,1);
        if (i==1) {
          if (predmet[vec]->udestroy) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->udestroy,user,u,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->rdestroy!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->rdestroy,user,u,NULL,10));
            write_room_except(user->room,text,user);
           }
          if (predmet[vec]->food[0]) {
            user->predmet[pos]=vec;
            user->dur[pos]=0;
           }
         }
        user->attacking+=predmet[vec]->seconddur;
      return;
      case 17:
	if (user->level<WAR) {
	  write_user(user,"Bohuzial, takyto druh utoku ma silu a schopnosti az level HRDINA!\n");
	  return;
	 }
	if (!strcmp(u->room->name,"namestie")) {
	  write_user(user,"Na takuto cinnost si najdite odlahlejsie miesto!\n");
	  return;
	 }
	if (user->level<u->level) {
	  sprintf(text,"%s ma vyssi level ako ty, nemozes si dovolit takyto utok.\n",u->name);
	  write_user(user,text);
	  return;
	 }
        show_pict_on_event(user,32,vec,0);
        show_pict_on_event(u,32,vec,1);
	remove_from_hands(user,vec,0);
	nah=rand()%2;
	if (nah==1) {
	  sprintf(text,"~FG%s %s uspesny samovrazedny utok a %s obetou %s: %s!\n",name,pohl(user,"uskutocnil","uskutocnila"),pohl(user,"jeho","jej"),pohl(u,"bol","bola"),u->name);
	  write_room_except2users(user->room,text,user,u);
	  write_user(user,"~LB~OL~FYKawasakiii, toyota mitsubishi karoshi!!! ~RS~FWS tymto pokrikom si sa sebevrazedne\n");
	  sprintf(text,"%s na svoju obet, a obaja ste padli mrtvi na zem!\n",pohl(user,"vrhol","vrhla"));
	  write_user(user,text);
	  sprintf(text,"~LB~OL~FYKawasakiii, toyota mitsubishi karoshi!!! ~RS~FWS tymto pokrikom sa %s sebe-\n",name);
	  write_user(u,text);
	  sprintf(text,"vrazedne na Teba %s, a obaja ste padli mrtvi na zem!\n",pohl(user,"vrhol","vrhla"));
	  write_user(u,text);
	  disconnect_user(user,1,NULL);
	  u->dead=6;
	 }
        else {
	  sprintf(text,"~FG%s %s samovrazedny utok, no %s %s ciel, ktory %s!\n",name,pohl(user,"uskutocnil","uskutocnila"),u->name,pohl(u,"bol","bola"),pohl(user,"nezasiahol","nezasiahla"));
	  write_room_except2users(user->room,text,user,u);
	  write_user(user,"~LB~OL~FYYamahaa, kung-pao nagasaki tamagotchi!!! ~RS~FWS tymto pokrikom si sa sebevrazedne\n");
	  sprintf(text,"%s na svoju obet, no %s si %s o tesny kusok!\n",pohl(user,"vrhol","vrhla"),pohl(user,"minul","minula"),pohl(u,"ho","ju"));
	  write_user(user,text);
	  sprintf(text,"~LB~OL~FYYamahaa, kung-pao nagasaki tamagotchi!!!~RS~FW S tymto pokrikom sa %s sebe-\n",name);
	  write_user(u,text);
	  sprintf(text,"vrazedne na Teba %s, ale nastastie ta %s!!!\n",pohl(user,"vrhol","vrhla"),pohl(user,"minul","minula"));
	  write_user(u,text);
	  disconnect_user(user,1,NULL);
         }
	return;
      default: word_count=2;
     }
   }
 }
}


if (word_count==2) { // 1 parameter - predmet
  //tunelovanie dvoma krtkami
  if (!strcmp(predmet[vec]->name,"krtko") && tunelik>0 && (!strcmp(user->room->name,"pohorie")
  || !strcmp(user->room->name,"vrchol") || !strcmp(user->room->name,"husty_les"))) {
    vec2=0;
    for(i=0;i<HANDS;i++) if (user->predmet[i]==vec) vec2++;
    if (vec2>1) {
      sprintf(text,"~FYPchas si jedneho krtka do ust..\n~FYKrtkovia zosaleli, vytrhli sa ti a ostala po nich velka diera v zemi.\n");
      write_user(user,text);
      sprintf(text,"~FY%s na smrt vystrasil%s krtkov, ostala po nich len velka diera v zemi.\n",name,pohl(user,"","a"));
      wrtype=WR_ZVERY;
      write_room_except(user->room,text,user);
      wrtype=0;
      do_tunel(user->room);
      for(i=0;i<vec2;i++) remove_from_hands(user,vec,0);
      return;
     }
   } 

  switch (predmet[vec]->function) {
    case 1: // zviachacka
      if (user->zuje && user->zuje_t==vec) {
        if (is_free_in_hands(user)>-1) {
          show_pict_on_event(user,32,vec,0);
          put_in_hands(user,vec,user->zuje);
          user->zuje=0;
          user->zuje_t=-1;
          if (predmet[vec]->uoutphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->uoutphr,user,NULL,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->routphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->routphr,user,NULL,NULL,10));
            write_room_except(user->room,text,user);
           }
          return;
         }
        sprintf(text,"Tvoje ruky su plne, nemas do coho chytit %s!\n",predmet[vec]->akuzativ);
        write_user(user,text);
        return;
       }
      if (is_in_hands(user,vec)==-1) {
        sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
        write_user(user,text);
        return;
       }
      if (user->zuje && user->zuje_t!=-1) {
        sprintf(text,"Uz mas v ustach %s!\n",predmet[user->zuje_t]->akuzativ);
        write_user(user,text);
        return;
       }
      show_pict_on_event(user,32,vec,0);
      remove_from_hands(user,vec,0);
      user->zuje=dur;
      user->zuje_t=vec;
      if (predmet[vec]->ustart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
        write_user(user,text);
       }
      if (predmet[vec]->rstart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
        write_room_except(user->room,text,user);
       }
      do_alt_funct(user,vec);
      return;
    case 2: // refolfer
      show_pict_on_event(user,32,vec,0);
      if (dur>1) {
        user->dur[is_in_hands(user,vec)]-=1;
        sprintf(text,"Vsetci okolostojaci strpli od napatia, ked tvoj prst siahol po spusti a\npotiahol ju ... CLICK! ... tentokrat si %s stastie!~LB\n",pohl(user,"mal","mala"));
        write_user(user,text);
        sprintf(text,"~FG%s si %s spust ... CLICK!\n~FGTo ma niekto stastie ...\n",name,pohl(user,"prilozil revolver k spanku a stlacil","prilozila revolver k spanku a stlacila"));
        write_room_except(user->room,text,user);
        do_alt_funct(user,vec);
        return;
       }
      user->dur[is_in_hands(user,vec)]=(rand()%6)+1;
      sprintf(text,"~LB~OL~LIBANG!!! ~RS~FRMiestnostou sa ozval tupy vystrel a v bolestiach si sa %s na zem.\n",pohl(user,"zrutil","zrutila"));
      write_user(user,text);
      sprintf(text,"~LB~FG%s si prilozil%s %s k spanku a stlacil%s spust ... ~OL~LI~FWBANG!~RS~FG Game over!\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"));
      write_room_except(user->room,text,user);
      disconnect_user(user, 3, NULL);
      return;
    case 3:  // zuby
      sprintf(text,"Pouzi: .use %s <obet>\n",predmet[vec]->name);
      write_user(user,text);
      return;
    case 4: // premena na iny predmet pri pouziti
      show_pict_on_event(user,32,vec,0);
      if (predmet[vec]->ustart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
        write_user(user,text);
       }
      if (predmet[vec]->rstart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
        write_room_except(user->room,text,user);
       }
      remove_from_hands(user,vec,0);
      if (predmet[vec]->food[0]) {
        vec2=expand_predmet(predmet[vec]->food);
        if (vec2>-1) {
          if (predmet[vec]->seconddur==0)
           put_in_hands(user,vec2,default_dur(vec2));
          else 
           put_in_hands(user,vec2,predmet[vec]->seconddur);
         }
       }
      return;
    case 5: // vehikel
      if (is_free_in_room(user->room)==-1) {
        sprintf(text,"%s uz nie je miesto.\n",user->room->where);
        text[0]=toupper(text[0]);
        write_user(user,text);
        return;
       }
      put_in_room(user->room,vec,dur);
      remove_from_hands(user,vec,0);
      use_predmet(user,"");
      return;
    case 8: // zbran pouzitelna aj na sebe.
      if (predmet[vec]->ustart!=NULL) {
        if (predmet[vec]->restrikt & 128 && !(predmet[vec]->restrikt & 1) && user->muzzled) {
          sprintf(text,"Ved uz si umlcan%s, naco ti je %s ?\n",pohl(user,"y","a"),predmet[vec]->name);
          write_user(user,text);
          return;
         }
        if (forbidden(user,NULL,vec)) return;
        show_pict_on_event(user,32,vec,0);
        remove_from_hands(user,vec,1);
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
        write_user(user,text);
        if (predmet[vec]->rstart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
          write_room_except(user->room,text,user);
         }
        do_alt_funct(user,vec);
       }
      else {
        sprintf(text,"Pouzi: .use %s <obet>\n",predmet[vec]->name);
        write_user(user,text);
       }
      return;
    case 7: // long range weapon - nastavenie primarnej zbrani
    case 9: // close combat weapon - detto
      if (user->weapon!=vec) {
        show_pict_on_event(user,32,vec,0);
        sprintf(text,"Odteraz budes pouzivat %s pri boji aj ak mas viac zbrani.\n",predmet[vec]->akuzativ);
        write_user(user,text);
        user->weapon=vec;
       }
      else {
        sprintf(text,"%s uz je tvoja primarna zbran.\n",predmet[vec]->akuzativ);
        text[0]=toupper(text[0]);
        write_user(user,text);
       }
      return;
    case 10: // consumable / predmet pouzitelny iba na sebe.
      if (forbidden(user,NULL,vec)) return;
      show_pict_on_event(user,32,vec,0);
      remove_from_hands(user,vec,1);
      if (predmet[vec]->ustart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
        write_user(user,text);
       }
      if (predmet[vec]->rstart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
        write_room_except(user->room,text,user);
       }
      if (predmet[vec]->altfunct & 8 && dur<2 && predmet[vec]->error!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));
        write_user(user,text);
       }
      do_alt_funct(user,vec);
      if (predmet[vec]->ofense<0) {
        user->heal+=-predmet[vec]->ofense;
	if (user->heal>100-user->health) user->heal=100-user->health;
       }
      return;
    case 11: // kniha
      show_pict_on_event(user,32,vec,0);
      user->dur[is_in_hands(user,vec)]--;
      show_kniha_kuziel(user,dur);
      do_alt_funct(user,vec);
      return;
    case 14: // telefon
      sprintf(text,"Pouzi: .use %s <sprava>!\n",predmet[vec]->name);
      write_user(user,text);
      return;
    case 15: // explosive
      if (user->dur[is_in_hands(user,vec)]>0 && predmet[vec]->error!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));
        write_user(user,text);
        return;
       }
      show_pict_on_event(user,32,vec,0);
      user->dur[is_in_hands(user,vec)]=predmet[vec]->dur;
      if (predmet[vec]->ustart!=NULL) {
        sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));	
        write_user(user,text);
       }
      do_alt_funct(user,vec);
      return;
    case 16: //kridla
      if (user->lieta) {
        if (user->lieta>1 && is_free_in_hands(user)>-1) {
          show_pict_on_event(user,32,vec,0);
          if (predmet[vec]->ustop!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,NULL,NULL,0));	
            write_user(user,text);
           }
          if (predmet[vec]->rstop!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,NULL,NULL,10));	
            write_room_except(user->room,text,user);
           }
          put_in_hands(user,vec,user->lieta-1);
         }
        else {
          show_pict_on_event(user,32,vec,0);
          if (predmet[vec]->error!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));	
            write_user(user,text);
           }
         }
        user->lieta=0;
        return;
       }
      if (is_in_hands(user,vec)>-1) {
        for(i=0;i<MPVM;i++)
         if (user->room->predmet[i]>-1 && predmet[user->room->predmet[i]]->function==5 && user->room->dur[i]>9999) {
           u=get_user_by_sock(user->room->dur[i]%10000);
           if (u!=NULL && u==user) user->room->dur[i]=default_dur(user->room->predmet[i]);
         }
        show_pict_on_event(user,32,vec,0);
        user->lieta=dur;
        if (predmet[vec]->ustart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));	
          write_user(user,text);
         }
        if (predmet[vec]->rstart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));	
          write_room_except(user->room,text,user);
         }
	remove_from_hands(user,vec,0);
        do_alt_funct(user,vec);
        return;
       }
      sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
      write_user(user,text);
      return;	
    case 17: //satka
      if (user->level<WAR) {
     	write_user(user,"Bohuzial, takyto druh utoku ma silu a schopnosti az level HRDINA!\n");
  	return;
       }
      sprintf(text,"Pouzi: .use %s <obet>\n",predmet[vec]->name);
      write_user(user,text);
      return;
    case 19: // zapisovaci predmet
      if (predmet[vec]->restrikt & 16 && !(predmet[vec]->restrikt & 1) && user->afro) {
        sprintf(text,"Ale no tak, jedno afrodiziakum ti nestaci ?!\n");
        write_user(user,text);
 	return;
       }
      if (predmet[vec]->restrikt & 32 && !(predmet[vec]->restrikt & 1) && user->afro) {
        sprintf(text,"Prehanadlo radsej neuzivaj ked si zalepen%s, mohlo by to zle dopadnut.\n",pohl(user,"y","a"));
        write_user(user,text);
 	return;
       }
      if (predmet[vec]->restrikt & 256 && !(predmet[vec]->restrikt & 1)) {
        if (!user->vis) {
          sprintf(text,"Nema zmysel pouzit %s, ked uz si %s!\n",predmet[vec]->akuzativ,pohl(user,"neviditelny","neviditelna"));
          write_user(user, text);
          return;
         }
        if (user->viscount!=0) {
     	  write_user(user,"Ved uz miznes!\n");
 	  return;
         }
       }
      if (strncmp(word[2],"wri",3)) {
        if (predmet[vec]->ustart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
          write_user(user,text);
         } 
        if (predmet[vec]->rstart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
          write_room_except(user->room,text,user);
         }
        show_pict_on_event(user,32,vec,0);
        if (predmet[vec]->altfunct & 8 && dur<2 && predmet[vec]->error!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->error,user,NULL,NULL,0));
          write_user(user,text);
         }
        do_alt_funct(user,vec);
       }
      else {
        if (predmet[vec]->userphr!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,NULL,NULL,0));
          write_user(user,text);
         } 
        if (predmet[vec]->roomphr!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,NULL,NULL,10));
          write_room_except(user->room,text,user);
         }
        user->misc_op=21;
        strcpy(user->mail_to,predmet[vec]->picture);
        editor(user,NULL);
       }
      return;
    case 20: /* na tekvicu */
    case 21: /* oblecko */
    case 22: /* cizmy */
      if (user->predmet[HANDS+predmet[vec]->function-20]>-1) {
        if (vec==user->predmet[HANDS+predmet[vec]->function-20]) { /* dame dole */
          if (is_free_in_hands(user)==-1) {
            write_user(user,"Ved mas plne ruky..\n");
            return;
           }
          if (predmet[vec]->ustop!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,NULL,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->rstop!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,NULL,NULL,10));
            write_room_except(user->room,text,user);
           }
          put_in_hands(user,user->predmet[HANDS+predmet[vec]->function-20],user->dur[HANDS+predmet[vec]->function-20]);
          user->predmet[HANDS+predmet[vec]->function-20]=-1;
          user->dur[HANDS+predmet[vec]->function-20]=0;
          return;
         }
        else { /* neda sa oblject */
          if (is_in_hands(user,vec)==-1) {
            sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
            write_user(user,text);
            return;
           }
          if (predmet[vec]->function==20) 
            sprintf(text,"Ved uz mas na hlave %s.\n",predmet[user->predmet[HANDS+predmet[vec]->function-20]]->akuzativ);
          else if (predmet[vec]->function==21) 
            sprintf(text,"Ved uz mas oblecen%s %s.\n",zwjpohl(user->predmet[HANDS+predmet[vec]->function-20],"y","u","e","e"),predmet[user->predmet[HANDS+predmet[vec]->function-20]]->akuzativ);
          else if (predmet[vec]->function==22) 
            sprintf(text,"Ved uz mas obut%s %s.\n",zwjpohl(user->predmet[HANDS+predmet[vec]->function-20],"y","u","e","e"),predmet[user->predmet[HANDS+predmet[vec]->function-20]]->akuzativ);
          write_user(user,text);
          return;
         }
       }
      else { /* mozme obliect */
        if (is_in_hands(user,vec)==-1) {
          sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
          write_user(user,text);
          return;
         }
        if (predmet[vec]->ustart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,user,NULL,NULL,0));
          write_user(user,text);
         }
        if (predmet[vec]->rstart!=NULL) {
          sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,user,NULL,NULL,10));
          write_room_except(user->room,text,user);
         }
        user->predmet[HANDS+predmet[vec]->function-20]=vec;
        user->dur[HANDS+predmet[vec]->function-20]=dur;
        remove_from_hands(user,vec,0);
        return;
       }
    case 23:
      if (forbidden(user,NULL,vec)) return;
      show_pict_on_event(user,32,vec,0);
      if (predmet[vec]->food[0]) {
        vec2=expand_predmet(predmet[vec]->food);
        if (vec2>-1) {
	  if (predmet[vec2]->function==24) {
	    kder=-1;
	    kdeh=-1;
	    nah=0;
	    for(i=0;i<MPVM;i++) if (user->room->predmet[i]==vec2) { kder=i; nah=user->room->dur[i]; break; }
	    for(i=0;i<MPVM;i++)
	     if (user->room->predmet[i]==vec2
	     && user->room->dur[i]>9999 && user->room->dur[i]%10000==user->socket)
	      { kdeh=i; nah=user->room->dur[i]/10000; break; }
	    if (kdeh==-1) kdeh=kder;
            if (kdeh==-1) {
              if (predmet[vec]->ujoinphr!=NULL) {
                sprintf(text,"%s\n",parse_phrase(predmet[vec]->ujoinphr,user,NULL,user->room,0));
                write_user(user,text);
               }
              if (predmet[vec]->rjoinphr!=NULL) {
                sprintf(text,"%s\n",parse_phrase(predmet[vec]->rjoinphr,user,NULL,user->room,10));
                write_room_except(user->room,text,user);
               }
              return;
             }
	    if (abs_dur(dec_dur(nah,-predmet[vec]->ofense))>predmet[vec2]->dur*2) {
              if (predmet[vec]->ustop!=NULL) {
                sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustop,user,NULL,user->room,0));
                write_user(user,text);
               }
              if (predmet[vec]->rstop!=NULL) {
                sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstop,user,NULL,user->room,10));
                write_room_except(user->room,text,user);
               }
	      return;
	     }
            if (predmet[vec]->userphr!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,NULL,NULL,0));
              write_user(user,text);
             }
            if (predmet[vec]->roomphr!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,NULL,NULL,10));
              write_room_except(user->room,text,user);
             }
            if (predmet[vec]->seconddur || user->room->dur[kdeh]>9999) 
	      user->room->dur[kdeh]=(nah-predmet[vec]->ofense)*10000+user->socket;
	    else
    	      user->room->dur[kdeh]=dec_dur(nah,predmet[vec]->ofense);
            
            do_alt_funct(user,vec);
	    remove_from_hands(user,vec,1);
	    return;
	   }
          pos2=is_in_hands(user,vec2);
          if (pos2==-1) {
            sprintf(text,"~FTVed nemas %s!\n",predmet[vec2]->akuzativ);
            write_user(user,text);
            return;
           }
          if (predmet[vec]->userphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->userphr,user,NULL,NULL,0));
            write_user(user,text);
           }
          if (predmet[vec]->roomphr!=NULL) {
            sprintf(text,"%s\n",parse_phrase(predmet[vec]->roomphr,user,NULL,NULL,10));
            write_room_except(user->room,text,user);
           }
          do_alt_funct(user,vec);
          if (user->dur[pos2]>0) {
            user->dur[pos]=user->dur[pos2];
            user->dur[pos2]=dur;
           }
          else { 
            user->dur[pos2]=dur;
            remove_from_hands(user,vec,0);
           }
         }
       }
      return;
    default: {
      if (predmet[vec]->type<3) {
        if (predmet[vec]->pp==0) {
          write_user(user,"Prosim Ta co chces robit s tym zverom ?\n");
          return;
         }
        show_pict_on_event(user,32,vec,0);
        i=(int)(rand()%2);
        wrtype=WR_ZVERY;
        if (i==0) {
          sprintf(text,"~FYS citom si vzal%s %s a pochrumal%s si %s.\n",pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
          write_user(user,text);
          sprintf(text,"~FY%s s citom vzal%s %s a pochrumal%s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
          write_room_except(user->room,text,user);
         }
        else {
          sprintf(text,"~FYPozrel%s si sa na %s, usmial%s sa a vopchal%s si si %s do ust.\n",pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
          write_user(user,text);
          sprintf(text,"~FY%s sa pozrel%s na %s, usmial%s sa a vopchal%s si %s do ust.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho",""));
          write_room_except(user->room,text,user);
         }
        if (predmet[vec]->altfunct & 2048) user->lsd+=90;
        wrtype=0;
        remove_from_hands(user,vec,0);
        setpp(user,user->pp+predmet[vec]->pp);
        do_alt_funct(user,vec);
        return;
       }
      else
       write_user(user,"Tento predmet nemozes pouzit. Aspon nie takto...\n");
     }
   }  
 } 
}

void throw_predmet(user)
UR_OBJECT user;
{
RM_OBJECT rm;
UR_OBJECT u;
int vec,dur,ok,i;
char *name,*uname;

if (user->vis) name=user->name; else name=invisname(user);
if (word_count==3) {
  vec=expand_predmet(word[1]);
  if (vec==-1) {
    sprintf(text,"Predmet '%s' neexistuje.\n",word[1]);
    write_user(user,text);
    return;
   }
  if (is_in_hands(user,vec)==-1) { 
    sprintf(text,"Nemas %s!\n",predmet[vec]->akuzativ);
    write_user(user,text);
    return;
   }
  if (user->pp < THROWCAN) {
    sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, THROWCAN);
    write_user(user,text); 
    return; 
   } 
  rm=get_room(word[2],user);
  u=NULL;
  if (user->room->group==4 || user->level==GOD || predmet[vec]->altfunct & 2) u=get_user(word[2]);
  if (rm==NULL && u==NULL) {
    write_user(user,"Taka miestnosti neexistuje.\n");
    return;
   }
  ok=0;
  if (rm!=NULL) {
    for(i=0;i<MAX_LINKS;++i) if (user->room->link[i]==rm) ok=1;
    if (user->level==GOD) ok=1;
   }
  if (ok==0) {
    if (u!=NULL) { // heh, hadzeme do usera
      if (u==user) {
        sprintf(text,"Nemas take dlhe hnaty aby si moh%s hadzat do seba.\n",pohl(user,"ol","la"));
        write_user(user,text);
        return;
       }
      if (check_ignore_user(user,u)) {
        sprintf(text,"%s ta ignoruje.\n",u->name);
        write_user(user,text);
        return;
       }
      if (u->room==user->room && !(predmet[vec]->altfunct & 2)) { // inac by sme mohli hadzat a dvihat doblba..
        sprintf(text,"Nehanbis sa hadzat takto z blizka ?!\n");
        write_user(user,text); 
        return;
       }
      ok=0; // dohodime ?
      for(i=0;i<MAX_LINKS;++i) if (u->room->link[i]==user->room) ok=1;
      if (user->room==u->room || user->level==GOD) ok=1;// ujo god dohodi sade ;)
      if (ok==0) { // nedohodime ;(
        sprintf(text,"%s nedohodis, je to prilis daleko!\n",u->room->into);
        text[0]=toupper(text[0]);
        write_user(user,text); 
        return;
       }
      if (is_free_in_room(u->room)==-1) {
        sprintf(text,"Je tam vela harabudria, %s tam nevidno.\n",sklonuj(u,4));
        write_user(user,text); 
        return;
       }
      show_pict_on_event(u,8,vec,1);
      ok=rand()%5;
      if (u->vis) uname=u->name; else uname=invisname(u);

      if (ok==0) {
        if (user->room==u->room) {
          sprintf(text,"~FTHodil%s si %s a %s to schytal%s rovno do hlavy :)\n",pohl(user,"","a"),predmet[vec]->akuzativ,u->name,pohl(u,"","a"));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s %s a schytal%s si to rovno do hlavy :)\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(u,"","a"));
          write_user(u,text);
          sprintf(text,"~FT%s hodil%s %s a %s to schytal%s rovno do hlavy :)\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,uname,pohl(u,"","a"));
          write_room_except2users(user->room,text,user,u);
         }
        else {
          sprintf(text,"~FTHodil%s si %s %s a %s to schytal%s rovno do hlavy :)\n",pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into,u->name,pohl(u,"","a"));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s %s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into);
          write_room_except(user->room,text,user);
          sprintf(text,"~FTNiekto sem %s hodil %s a schytal%s si to rovno do hlavy :)\n",user->room->from,predmet[vec]->akuzativ,pohl(u,"","a"));
          write_user(u,text);
          sprintf(text,"~FTNiekto sem %s hodil %s a %s to schytal%s rovno do hlavy :)\n",user->room->from,predmet[vec]->akuzativ,uname,pohl(u,"","a"));
          write_room_except(u->room,text,u);
         }
        setpp(u,u->pp-predmet[vec]->weight*2);
       }
      else if (ok==1) {
        if (user->room==u->room) {
          sprintf(text,"~FTHodil%s si %s ale %s sa stih%s uhnut.\n",pohl(user,"","a"),predmet[vec]->akuzativ,u->name,pohl(u,"ol","la"));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s po tebe %s ale stih%s si sa uhnut.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(u,"ol","la"));
          write_user(u,text);
          sprintf(text,"~FT%s hodil%s %s a tesne minul%s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),sklonuj(u,4));
          write_room_except2users(user->room,text,user,u);
         }
        else {
          sprintf(text,"~FTHodil%s si %s %s ale %s sa stih%s uhnut.\n",pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into,u->name,pohl(u,"ol","la"));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s %s %s.\n",user->name,pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into);
          write_room_except(user->room,text,user);
          sprintf(text,"~FTNiekto sem %s hodil %s a tesne ta minul.\n",user->room->from,predmet[vec]->akuzativ);
          write_user(u,text);
          sprintf(text,"~FTNiekto sem %s hodil %s a tesne minul %s.\n",user->room->from,predmet[vec]->akuzativ,sklonuj(u,4));
          write_room_except(u->room,text,u);
         }
       }
      else {
        if (user->room==u->room) {
          sprintf(text,"~FTHodil%s si %s a trafil%s si %s.\n",pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),sklonuj(u,4));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s %s a trafil%s ta.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"));
          write_user(u,text);
          sprintf(text,"~FT%s hodil%s %s a trafil%s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),sklonuj(u,4));
          write_room_except2users(user->room,text,user,u);
         }
        else {
          sprintf(text,"~FTHodil%s si %s %s a trafil%s si %s.\n",pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into,pohl(user,"","a"),sklonuj(u,4));
          write_user(user,text);
          sprintf(text,"~FT%s hodil%s %s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,u->room->into);
          write_room_except(user->room,text,user);
          sprintf(text,"~FTNiekto sem %s hodil %s a trafil ta.\n",user->room->from,predmet[vec]->akuzativ);
          write_user(u,text);
          sprintf(text,"~FTNiekto sem %s hodil %s a trafil %s.\n",user->room->from,predmet[vec]->akuzativ,sklonuj(u,4));
          write_room_except(u->room,text,u);
         }
        setpp(u,u->pp-predmet[vec]->weight);
       }
      if (vec>-1) dur=user->dur[is_in_hands(user,vec)];
      else dur=0;
      remove_from_hands(user,vec,0);
      if (predmet[vec]->altfunct & 2) {
        if (dur>1) put_in_room(u->room,vec,dur-1);
       }
      else put_in_room(u->room,vec,dur);
      return;
     }
    write_user(user,"Do tej miestnosti odtialto nedohodis.\n");
    return;
   }  
  if (is_free_in_room(rm)==-1) {
    write_user(user,"V tej miestnosti uz nie je miesto.\n");
    return;
   }  
  if (vec>-1) dur=user->dur[is_in_hands(user,vec)];
  else dur=0;
  remove_from_hands(user,vec,0);
  if (predmet[vec]->altfunct & 2) {
    if (dur>1) put_in_room(rm,vec,dur-1);
   }
  else put_in_room(rm,vec,dur);
  sprintf(text,"Uchopil%s si %s a zahodil%s si %s %s.\n",pohl(user,"","a"),predmet[vec]->akuzativ,pohl(user,"","a"),zwjpohl(vec,"ho","ju","ho","ich"),rm->into);
  write_user(user,text);
  sprintf(text,"%s zahodil%s %s %s.\n",name,pohl(user,"","a"),predmet[vec]->akuzativ,rm->into);
  write_room_except(user->room,text,user);
  sprintf(text,"Niekto sem %s hodil %s.\n",user->room->from,predmet[vec]->akuzativ);
  write_room(rm,text);
  decrease_pp(user,THROWDEC+(predmet[vec]->weight),NODYNAMIC);
  if (user->sell_what==vec) {
    user->sell_what=-1;
    user->sell_price=0;
    user->sell_to[0]='\0';
   }
  return;
 }
if (user->room->group==4 || user->level==GOD) write_user(user,"Co chces kam hodit ? Pouzi: .throw <vec> <miestnost> alebo: .throw <vec> <user>\n");
else write_user(user,"Co chces kam hodit ? Pouzi: .throw <vec> <miestnost>\n");
}

void dispose_predmet(user)
UR_OBJECT user;
{
int vec=0,i,cnt=0;
RM_OBJECT rm,rm2;
UR_OBJECT u;
char *name;

if (word_count<2) {
  if (user->level==GOD) write_user(user,"Pouzi: .dispose <predmet> [hand | quiet]\n       .dispose [everything | everyuser | everyroom]\n       .dispose every <predmet>\n       .dispose roomlink <room1> <room2>\n");
  else write_user(user,"Pouzi: .dispose <predmet>\n");
  return;
 }
if (user->level==GOD && (!strcmp(word[1],"everyroom") || !strcmp(word[1],"everything"))) {
  for(rm=room_first;rm!=NULL;rm=rm->next)
   for(i=0;i<MPVM;i++) if (rm->predmet[i]>-1) {
     rm->predmet[i]=-1;
     rm->dur[i]=0;
     vec++;
    }
  if (!strcmp(word[1],"everyroom")) {
    sprintf(text,"%s %d predmet%s.\n",skloncislo(vec,"Bol zlikvidovany","Boli zlikvidovane","Bolo zlikvidovanych"),vec,skloncislo(vec,"","y","ov"));
    write_user(user,text);
    return;
   }
 }
if (user->level==GOD && (!strcmp(word[1],"everyuser") || !strcmp(word[1],"everything"))) {
  for(u=user_first;u!=NULL;u=u->next)
   for(i=0;i<HANDS;i++) if (u->predmet[i]>-1) {
     u->predmet[i]=-1;
     u->dur[i]=0;
     vec++;
    }
  sprintf(text,"%s %d predmet%s.\n",skloncislo(vec,"Bol zlikvidovany","Boli zlikvidovane","Bolo zlikvidovanych"),vec,skloncislo(vec,"","y","ov"));
  write_user(user,text);
  return;
 }

if (user->level==GOD && !strcmp(word[1],"roomlink")) {
  rm=get_room(word[2],user);
  rm2=get_room(word[3],user);
  if (rm==NULL || rm2==NULL || word_count<4) {
    write_user(user,"Pouzi: .dispose roomlink <miestnost1> <miestnost2>\n");
    return;
   }
  unlink_room(rm,rm2);
  sprintf(text,"%s sa uz %s nedostanes a ani opacne.\n",rm->into,rm2->from);
  text[0]=toupper(text[0]);
  write_user(user,text);
  return;
 }

if (user->level==GOD && (!strcmp(word[1],"every"))) {
  vec=expand_predmet(word[2]);
  if (vec==-1) {
	sprintf(text,"Predmet '%s' neexistuje.\n",word[2]);
	write_user(user,text);
	return;
	}
  for(rm=room_first;rm!=NULL;rm=rm->next)
   for(i=0;i<MPVM;i++) if (rm->predmet[i]==vec) {
     rm->predmet[i]=-1;
     rm->dur[i]=0;
     cnt++;
    }
  for(u=user_first;u!=NULL;u=u->next)
   for(i=0;i<HANDS;i++) if (u->predmet[i]==vec) {
     u->predmet[i]=-1;
     u->dur[i]=0;
     cnt++;
    }
  sprintf(text,"%s %d ks predmetu %s.\n",skloncislo(cnt,"Bol zlikvidovany","Boli zlikvidovane","Bolo zlikvidovanych"),cnt,predmet[vec]->name);
  write_user(user,text);
  return;
 }

vec=expand_predmet(word[1]);
if (vec==-1) {
	sprintf(text,"Predmet '%s' neexistuje.\n",word[1]);
	write_user(user,text);
	return;
	}
if (!strcmp(word[word_count-1],"hand")) {
  if (is_in_hands(user,vec)==-1) {
	sprintf(text,"%s nemas.\n",predmet[vec]->name);
	write_user(user,text);
	return;
	}
  remove_from_hands(user,vec,0);
  sprintf(text,"Rozdrvil si %s.\n",predmet[vec]->akuzativ);
  write_user(user,text);
  return;
 }
if (is_in_room(user->room,vec)==-1) {
	sprintf(text,"%s sa tu nenachadza%s!\n",predmet[vec]->name,zwjpohl(vec,"","","","ju"));
	write_user(user,text);
	return;
	}

if (user->vis) name=user->name; else name=invisname(user);
remove_from_room(user->room,vec,default_dur(vec));
sprintf(text,"Zrusil si %s %s.\n",predmet[vec]->akuzativ,user->room->from);
write_user(user,text);
sprintf(text,"~OL~FBBoooo! ~RS~FR%s vyriekol zaklinadlo a %s sa straca v hustej hmle!\n",name,predmet[vec]->name);
if (user->vis && strcmp(word[word_count-1],"quiet")) write_room_except(user->room,text,user);

}

void give_predmet(user)
UR_OBJECT user;
{
UR_OBJECT u;
int vec,dur;
char *name,uname[USER_NAME_LEN+10];

if (word_count<3) {
  write_user(user,"Pouzi: .give <predmet> <uzivatel>\n");
  return;
 }

vec=expand_predmet(word[1]);

if (vec==-1) {
	sprintf(text,"Predmet '%s' neexistuje!\n",word[1]);
	write_user(user,text);
	return;
	}

if (!(u=get_user(word[2]))) {
	write_user(user,notloggedon);
	return;
	}
if (u==user) {
	sprintf(text,"%s sebe?!\n",pohl(user,"Sam","Sama"));
	write_user(user,text);
	return;
	}
if (strcmp(user->room->name,u->room->name)) {
	sprintf(text,"%s nieje v tejto miestnosti!\n",u->name);
	write_user(user,text);
	return;
	}	
if (is_in_hands(user,vec)==-1) {
	sprintf(text,"Nemas %s.\n",predmet[vec]->akuzativ);
	write_user(user,text);
	return;
	}
if (is_free_in_hands(u)==-1) {
	sprintf(text,"Bohuzial, %s ma plne ruky!\n",u->name);
	write_user(user,text);
	return;
	}
if (u->carry+predmet[vec]->weight>MAXCARRY) {
	sprintf(text,"%s taku vahu uz neunesie.\n",u->name);
	write_user(user,text);
	return;
	}
if (u->ignall) { /*IGNALL*/
		if (u->filepos) write_user(user,"Uzivatel prave cita nejaky text a nechce byt ruseny.\n");
		 else if (u->malloc_start!=NULL) write_user(user,"Uzivatel prave edituje nejaky text, a nechce byt ruseny.\n");
		    else write_user(user,"Uzivatel teraz vsetkych ignoruje.\n");
		return;
		}
		
dur=user->dur[is_in_hands(user,vec)];
	
/*if ((predmet[vec]->function==12) && (dur)) {
	if (!strcmp(u->room->name,"svatyna")) {
		write_user(user,"Vo svatyni nemozes vytahovat bombu!\n");
		return;
		}
	if (u->ignfun) {
		write_user(user,"Ziadne bombove srandicky, uzivatel ma nastaveny ignFun!\n");
		return;
		}
	if (is_in_hands(u,vec)>-1 && u->dur[is_in_hands(u,vec)]) {
		sprintf(text,"%s uz ma na krku tikajucu potvoru!!!\n",u->name);
		write_user(user,text);
		return;
		}
	sprintf(text,"%s ti %s tikajuce cudo (vyzera to ako bomba :-)!!!\n",user->name,pohl(user,"podal","podala"));
	write_user(u,text);	
	sprintf(text,"%s sa %s %s!\n",user->name,pohl(user,"sa zbavil tikaj ucej bomby, ktoru podal","sa zbavila tikajucej bomby, ktoru podala"), sklonuj(u, 3));	
	write_room_except(user->room,text,u);
	put_in_hands(u,vec,dur);
	remove_from_hands(user,vec,0);
	return;
	}	
*/
if (user->vis) name=user->name; else name=invisname2(user);	
if (!u->vis) strcpy(uname,pohl(u,"Neznamemu","Neznamej"));
else strcpy(uname,sklonuj(u,3));
show_pict_on_event(u,4,vec,1);
put_in_hands(u,vec,dur);
remove_from_hands(user,vec,0);

sprintf(text,"%s si %s %s.\n",pohl(user,"Podal","Podala"),predmet[vec]->akuzativ,sklonuj(u,3));
write_user(user,text);
sprintf(text,"%s ti %s %s.\n",name, pohl(user,"podal","podala"),predmet[vec]->akuzativ);
write_user(u,text);
sprintf(text,"%s %s %s %s.\n",name,pohl(user,"podal","podala"),predmet[vec]->akuzativ,uname);
write_room_except2users(user->room,text,user,u);

if (user->sell_what==vec) {
  user->sell_what=-1;
  user->sell_price=0;
  user->sell_to[0]='\0';
 }
}

void create_predmet(user)
UR_OBJECT user;
{
int vec,i=0,len,dur;
UR_OBJECT u;
RM_OBJECT rm1,rm2;
char *name;

if (word_count<2) {
	if (user->level==GOD) write_user(user,"Pouzi: .create <predmet>  alebo: .create roomlink <room1> <room2>\n");
        else write_user(user,"Pouzi: .create <predmet>\n");
	strcpy(text,"~FTMozes vytvorit: ~FW");
	len=strlen(text);
	while(predmet[i]!=NULL) {
	  len+=strlen(predmet[i]->name)+2;
	  if (len>79) { strcat(text,"\n"); len=strlen(predmet[i]->name)+2; }
	  strcat(text,farba_p(i));
	  strcat(text,predmet[i]->name);
	  if (predmet[i+1]!=NULL) strcat(text,", ");
	  i++;
	 }
	strcat(text,"\n"); 
	write_user(user,text);
	return;
	}
if (user->level==GOD && !strcmp(word[1],"roomlink")) {
  rm1=get_room(word[2],user);
  rm2=get_room(word[3],user);
  if (rm1==NULL || rm2==NULL || word_count<4) {
    write_user(user,"Pouzi: .create roomlink <miestnost1> <miestnost2>\n");
    return;
   }
  link_room(rm1,rm2);
  sprintf(text,"Odteraz sa da %s prejst %s aj s5.\n",rm1->from,rm2->into);
  write_user(user,text);
  return;
 }
vec=expand_predmet(word[1]);

if (vec==-1) {
	sprintf(text,"Predmet '%s' neexistuje!\n",word[1]);
	write_user(user,text);
	return;
	}
if (!strcmp(word[word_count-1],"hand")) {
  if (is_free_in_hands(user)==-1) {
    write_user(user,"Ved mas plne paprce!\n");
    return;
   }
  dur=default_dur(vec);
  if (word_count>3 && (u=get_user(word[2])) && (predmet[vec]->type<3)) {
    if (predmet[vec]->function==24) dur=u->socket+predmet[vec]->dur*10000;
    else dur=u->socket+10000;
   }
  put_in_hands(user,vec,dur);
  sprintf(text,"V paprciach sa ti okotil%s %s.\n",zwjpohl(vec,"","a","o","i"),predmet[vec]->name);
  write_user(user,text);
  return;
 }

if (is_free_in_room(user->room)==-1) {
	write_user(user,"Bohuzial, miestnost je uz plna!\n");
	return;
	}
if (user->vis) name=user->name; else name=invisname(user);
dur=default_dur(vec);
if (word_count>2 && (u=get_user(word[2])) && (predmet[vec]->type<3)) {
  if (predmet[vec]->function==24) dur=u->socket+predmet[vec]->dur*10000;
  else dur=u->socket+10000;
 }
put_in_room(user->room,vec,dur);
sprintf(text,"Vytvoril si %s %s.\n",predmet[vec]->akuzativ,user->room->where);
write_user(user,text);
sprintf(text,"~OL~FRHops! ~RS~FG%s %s ~OL%s~RS~FG!\n",name,pohl(user,"vyriekol zaklinadlo a vytvoril","vyriekla zaklinadlo a vytvorila"),predmet[vec]->akuzativ);
strtolower(word[word_count-1]);
if (user->vis && strcmp(word[word_count-1],"quiet")) write_room_except(user->room,text,user);
} 

void get_predmet(user)
UR_OBJECT user;
{
UR_OBJECT u;
int vec,dur,pocet=0,i,vel,y,idle=0;
char tmp[3],*name;
char *help="~FTPouzi:~FW .get [<user>] <predmet>  ~FTak chces zdvihnut predmet [odcudzit uzivatelovi]\n";

if (word_count<2) {
  for(i=0;i<MPVM;i++)
   if (user->room->predmet[i]>-1) pocet++;
  if (pocet==0) {
    write_user(user,help);
    write_user(user,"Nenachadza sa tu ziadny predmet.\n");
    return;
   }
  vel=16;
  y=0;
  strcpy(text,"");
  for (i=0;i<MPVM;++i) {
    if (user->room->predmet[i]>-1) {
      if (y==0) {
        strcat(text,help);
        strcat(text,"~FTNachadza sa tu:~FW ");
       }
      vel+=strlen(predmet[user->room->predmet[i]]->name)+2;
      if (y>0) strcat(text,", ");
      if (vel>78) {
        strcat(text,"\n");
        write_user(user,text);
        strcpy(text,"                ");
        vel=strlen(text)+strlen(predmet[user->room->predmet[i]]->name);
       }
      if (predmet[user->room->predmet[i]]->type==3 || predmet[user->room->predmet[i]]->type==4) strcat(text,"~FW");
      else strcat(text,farba_p(user->room->predmet[i]));
      strcat(text,predmet[user->room->predmet[i]]->name);
      y++;
     }
   } 
  if (y>0) {
    strcat(text,".\n");
    write_user(user,text);
   }
  return;
 }

if (word_count==3) {

  vec=expand_predmet(word[2]);
  if (vec==-1) {
    sprintf(text,"Predmet '%s' neexistuje!\n",word[2]);
    write_user(user,text);
    return;
   }
  if (!(u=get_user(word[1]))) {
    write_user(user,notloggedon);
    return;
   }
  if (u->room!=user->room) {
    sprintf(text,"%s nie je s tebou v miestnosti.\n",u->name);
    write_user(user,text);
    return;
   }
  if (is_in_hands(u,vec)==-1) {
    sprintf(text,"%s nema %s.\n",u->name,predmet[vec]->akuzativ);
    write_user(user,text);
    return;
   }
  idle=(int)(time(0)-u->last_input)/60;
  if (idle<=10) {
    sprintf(text,"%s je cul%s odcudzit %s.\n",u->name,pohl(u,"y jak vevericka, nemozes mu","a jak vevericka, nemozes jej"),predmet[vec]->akuzativ);
    write_user(user,text);
    return;
   }
  if (user->pp<50) {
    sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp,50);
    write_user(user,text); 
    return; 
   } 
  if (rand()%100+((idle-11)*5)>70) {
    decrease_pp(user,40,0);
    sprintf(text,"~FTPodarilo sa! Nenapadne si odcudzil%s %s %s.\n",pohl(user,"","a"),sklonuj(u,3),predmet[vec]->akuzativ);
    write_user(user,text);
    sprintf(text,"~FTKym si sa stih%s spamatat, %s ti odcudzil%s %s.\n",pohl(u,"ol","la"),user->name,pohl(user,"","a"),predmet[vec]->akuzativ);
    write_user(u,text);
    sprintf(text,"~FT%s odcudzil%s %s %s.\n",user->name,pohl(user,"","a"),sklonuj(u,3),predmet[vec]->akuzativ);
    write_room_except2users(user->room,text,u,user);
    put_in_hands(user,vec,u->dur[is_in_hands(user,vec)]);
    remove_from_hands(u,vec,0);
    return;
   }
  else {
    sprintf(text,"~FTPokusas sa odcudzit %s %s.\n",sklonuj(u,3),predmet[vec]->akuzativ);
    write_user(user,text);
    sprintf(text,"~OLPlesk!~RS~FT %s ti taku struh%s ze vidis same hviezdicky.\n",u->name,pohl(u,"ol","la"));
    write_user(user,text);
    user->stars+=30;
    decrease_pp(user,30,0);
    sprintf(text,"~FT%s sa pokusa odcudzit ti %s..\n",user->name,predmet[vec]->akuzativ);
    write_user(u,text);
    sprintf(text,"~FTPlesk! Taku si %s vypalil%s ze ta boli ruka.\n",pohl(user,"mu","jej"),pohl(u,"","a"));
    write_user(u,text);
    decrease_pp(u,10,0);
    return;
   }
  return;
 }

vec=expand_predmet(word[1]);

if (vec==-1) {
  sprintf(text,"Predmet '%s' neexistuje!\n",word[1]);
  write_user(user,text);
  return;
 }

if (is_in_room(user->room,vec)==-1) {
	sprintf(text,"%s sa v tejto miestnosti nenachadza%s.\n",predmet[vec]->name,zwjpohl(vec,"","","","ju"));
	text[0]=toupper(text[0]);
	write_user(user,text);
	return;
	}
if (is_free_in_hands(user)==-1) {
	write_user(user,"Uz nemas kam davat dalsie predmety!\n");
	return;
	}
if (user->carry+predmet[vec]->weight>MAXCARRY) {
	sprintf(text,"Taku vahu uz neunesies.\n");
	write_user(user,text);
	return;
	}
dur=user->room->dur[is_in_room(user->room,vec)];
if (predmet[vec]->function==5 && dur>9999 && (u=get_user_by_sock(dur%10000))!=NULL && u!=user) {
  strcpy(tmp,zwjpohl(vec,"y","u","e","e"));
  sprintf(text,"~FGNa %s %s sedi %s, najdi si nejak%s voln%s.\n",zwjpohl(vec,"tomto","tejto","tomto","tychto"),predmet[vec]->inytiv,u->name,tmp,tmp);
  write_user(user,text);
  return;
 }
if (user->vis) name=user->name; else name=invisname2(user);
show_pict_on_event(user,16,vec,0);
if (predmet[vec]->type<3) wrtype=WR_ZVERY;
if ((predmet[vec]->type==3 || predmet[vec]->type==4) && dur==predmet[vec]->dur)
 { /* ak je rastlina neodtrhnuta, tak ju usklbneme */
  sprintf(text,"%s si %s%s~FW.\n",pohl(user,"Odtrhol","Odtrhla"),farba_p(vec),predmet[vec]->akuzativ);
  write_user(user,text);
  sprintf(text,"%s %s %s%s~FW.\n",name, pohl(user,"odtrhol","odtrhla"),farba_p(vec),predmet[vec]->akuzativ);
 }
else
 {
  sprintf(text,"%s si %s%s~FW.\n",pohl(user,"Zobral","Zobrala"),farba_p(vec),predmet[vec]->akuzativ);
  write_user(user,text);
  sprintf(text,"%s %s %s%s~FW.\n",name, pohl(user,"zobral","zobrala"),farba_p(vec),predmet[vec]->akuzativ);
 }
write_room_except(user->room,text,user);
put_in_hands(user,vec,user->room->dur[is_in_room(user->room,vec)]);
remove_from_room(user->room,vec,1);
wrtype=0;
} 

char *farba_p(int vec)
{
static char fstr[5];

 if (predmet[vec]->type<3)
  strcpy(fstr,"~FY");
 else if (predmet[vec]->type==3 || predmet[vec]->type==4)
  strcpy(fstr,"~FG");
 else strcpy(fstr,"~FW");
return fstr;
}

void put_predmet(user)
UR_OBJECT user;
{
int vec;
char text2[200],*name;
int pocet=0,y,i;

for(i=0;i<HANDS;i++)
 if (user->predmet[i]>-1) pocet++;
text2[0]='\0';
if (word_count<2) {
 if (pocet==0) {
  write_user(user,"Nemas pri sebe ziadny predmet.\n");
  return;
 }
 if (pocet>0) {
  write_user(user,"Pouzi: .put <predmet> pre vyhodenie predmetu (.put all vyhodi vsetky predmety)\n");
  sprintf(text,"~FTMas pri sebe~FW ");
  for(i=0;i<HANDS;i++) if (user->predmet[i]>-1)
   {
    strcat(text,farba_p(user->predmet[i]));
    strcat(text,predmet[user->predmet[i]]->akuzativ);
    pocet=0;
    for(y=i+1;y<HANDS;y++)
     if (user->predmet[y]>-1) pocet++;
    if (pocet>1) strcat(text,"~FW, ");
    else if (pocet==1) strcat(text,"~FW a ");
    else if (pocet==0) strcat(text,"~FW.\n");
   }
  write_user(user,text);
  return;
 }
}

vec=expand_predmet(word[1]);

if (!strcmp(word[1],"all")) {
 if (pocet==0)
  {
   write_user(user,"Nemas ziadny predmet na vyhodenie!\n");
   return;
  }
 if (pocet>is_free_in_room(user->room))
  {
   write_user(user,"Bohuzial, uz tu nie je tolko miesta.\n");
   return;
  }
/* for(i=0;i<HANDS;i++) if (user->predmet[i]==12 && user->dur[i]>0) {
  write_user(user,"Tikajucu bombu nemozes polozit na zem!\n");
  return;
 }*/
 strcpy(text2,"");
 for(i=0;i<HANDS;i++) if (user->predmet[i]>-1)
  {
   strcat(text2,farba_p(user->predmet[i]));
   strcat(text2,predmet[user->predmet[i]]->akuzativ);
   pocet=0;
   for(y=i+1;y<HANDS;y++)
    if (user->predmet[y]>-1) pocet++;
   if (pocet>1) strcat(text2,"~FW, ");
   else if (pocet==1) strcat(text2,"~FW a ");
   else if (pocet==0) strcat(text2,"~FW.\n");
   put_in_room(user->room,user->predmet[i],user->dur[i]);
   user->carry-=predmet[user->predmet[i]]->weight;
   user->predmet[i]=-1;
   user->dur[i]=0;
  }
 if (user->vis) name=user->name; else name=invisname2(user);
 sprintf(text,"%s si %s",pohl(user,"Polozil","Polozila"),text2);
 write_user(user,text);
 sprintf(text,"%s %s %s",name,pohl(user,"polozil","polozila"),text2);
 write_room_except(user->room,text,user);
 user->sell_what=-1;
 user->sell_price=0;
 user->sell_to[0]='\0';
 return;
}

if (vec==-1) {
 sprintf(text,"Predmet %s neexistuje!\n",word[1]);
 write_user(user,text);
 return;
}
if (is_in_hands(user,vec)==-1) {
 sprintf(text,"Nemas pri sebe %s.\n",predmet[vec]->akuzativ);
 write_user(user,text);
 return;
}
if (is_free_in_room(user->room)==-1) {
 write_user(user,"Bohuzial, uz tu nie je miesto.\n");
 return;
}
if (predmet[vec]->function==12 && user->dur[is_in_hands(user,vec)]) {
 write_user(user,"Tikajucu bombu nemozes polozit na zem!\n");
 return;
}

if (user->vis) name=user->name; else name=invisname2(user);
if (predmet[vec]->type<3) wrtype=WR_ZVERY;
put_in_room(user->room,vec,user->dur[is_in_hands(user,vec)]);
remove_from_hands(user,vec,0);

sprintf(text,"%s si %s%s~FW.\n",pohl(user,"Polozil","Polozila"),farba_p(vec),predmet[vec]->akuzativ);
write_user(user,text);
sprintf(text,"%s %s %s%s~FW.\n",name, pohl(user,"polozil","polozila"),farba_p(vec),predmet[vec]->akuzativ);
write_room_except(user->room,text,user);
wrtype=0;
if (user->sell_what==vec) {
  user->sell_what=-1;
  user->sell_price=0;
  user->sell_to[0]='\0';
 }
} 

void loose_predmets(UR_OBJECT user)
{
int i;

if (user==NULL) return;
 for(i=0;i<BODY;i++) {
   user->predmet[i]=-1;
   user->dur[i]=0;
  }
 user->carry=0;
}
void buy(UR_OBJECT user)
{
UR_OBJECT u;
int i,vec,cnt=0,vel=0;
char tmp[30],*name;

if (!strcmp(user->room->name,"obchod")) {
  if (word_count==1) {
    for(i=0;predmet[i]!=NULL;i++) if (predmet[i]->price>0) {
      cnt++;
      if (cnt==1) { sprintf(text,"~OLV obchode ponukaju:~RS "); vel=strlen(text)-6; }
      sprintf(tmp,"%s (%d)",predmet[i]->name,predmet[i]->price);
      vel+=strlen(tmp)+2;
      if (vel>79) {
        strcat(text,",\n");
        vel=strlen(tmp);
       }
      else if (cnt>1) strcat(text,", ");
      strcat(text,tmp);
     }
    strcat(text,".\n");
    if (cnt>0) write_user(user,text);
   }
  else {
    vec=expand_predmet(word[1]);
    u=get_user(word[1]);
    if (vec==-1 && (!(u!=NULL && u->room==user->room))) {
      sprintf(text,"Taky predmet nemame.\n");
      write_user(user,text);
      return;
     }
    if (vec>-1) {
      if (predmet[vec]->price<1) {
        sprintf(text,"Nic take sa tu neda kupit.\n");
        write_user(user,text);
        return;
       }
      if (predmet[vec]->function!=24) {
        if (is_free_in_hands(user)==-1) {
          write_user(user,"Uz nemas kam zobrat dalsi predmet.\n");
          return;
         }
        if (user->carry+predmet[vec]->weight>MAXCARRY) {
          sprintf(text,"Taku vahu uz neunesies..\n");
          write_user(user,text);
          return;
         }
       }
      if (user->gold<predmet[vec]->price) {
        sprintf(text,"%s stoji %d %s, tolko nemas.\n",predmet[vec]->name,predmet[vec]->price,skloncislo(predmet[vec]->price,"dukat","dukaty","dukatov"));
        text[0]=toupper(text[0]);
        write_user(user,text);
        return;
       }
      else {
        if (predmet[vec]->function==24) {
	  if (is_free_in_room(user->room)==-1) user->room->predmet[rand()%MPVM]=-1;
	  put_in_room(user->room,vec,predmet[vec]->dur*10000+user->socket);
	 }
	else put_in_hands(user,vec,default_dur(vec));
        user->gold-=predmet[vec]->price;
        sprintf(text,"~FYKupil si ~OL%s~RS~FY za ~OL%d~RS~FY %s.",predmet[vec]->akuzativ,predmet[vec]->price,skloncislo(predmet[vec]->price,"dukat","dukaty","dukatov"));
        word_count=1;
        gold(user,1,text);
        return;
       }
     }
   } 
 }
if (word_count==1) {
  sprintf(text,"Pouzi: buy <od_koho> alebo v obchode: buy <predmet>\n");
  for (u=user_first;u!=NULL;u=u->next)
   if (!u->login && u->type==USER_TYPE && u!=user
    && u->room==user->room && u->sell_what>-1 && u->sell_price>0
    && (u->sell_to[0]=='\0' || !strcmp(u->sell_to,user->name))) {
      if (u->sell_to[0]=='\0')
       sprintf(texthb,"%s tu ponuka %s za %d %s.\n",u->name,predmet[u->sell_what]->akuzativ,u->sell_price,skloncislo(u->sell_price,"dukat","dukaty","dukatov"));
      else
       sprintf(texthb,"~OL%s~RS ti ponuka %s za %d %s.\n",u->name,predmet[u->sell_what]->akuzativ,u->sell_price,skloncislo(u->sell_price,"dukat","dukaty","dukatov"));
      strcat(text,texthb);
     }
  write_user(user,text);
  return;
 }
if (!(u=get_user(word[1]))) {
  write_user(user,notloggedon);
  return;
 }
if (user==u) {
  write_user(user,"Skus od niekoho ineho..\n");
  return;
 }
if (u->room!=user->room) {
  write_user(user,"Musite byt v jednej miestnosti.\n");
  return;
 }
if (((u->sell_to[0]) && strcmp(user->name,u->sell_to))
    || u->sell_price==0 || u->sell_what==-1) {
  sprintf(text,"%s ti nechce nic predat.\n",u->name);
  write_user(user,text);
  return;
 }
vec=u->sell_what;
if (is_in_hands(u,vec)) {
  sprintf(text,"%s uz nema %s.\n",u->name,predmet[vec]->akuzativ);
  write_user(user,text);
  return;
 }
if (u->sell_price>user->gold) {
  sprintf(text,"%s chce za %s %d %s, tolko nemas.\n",u->name,predmet[vec]->akuzativ,u->sell_price,skloncislo(u->sell_price,"dukat","dukaty","dukatov"));
  write_user(user,text);
  return;
 }
if (is_free_in_hands(user)==-1) {
  write_user(user,"Uz nemas kam zobrat dalsi predmet.\n");
  return;
 }
if (user->carry+predmet[vec]->weight>MAXCARRY) {
  sprintf(text,"Taku vahu uz neunesies..\n");
  write_user(user,text);
  return;
 }
put_in_hands(user,vec,u->dur[is_in_hands(u,vec)]);
remove_from_hands(u,vec,0);
u->gold+=u->sell_price;
user->gold-=u->sell_price;
word_count=1;
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~OL%s~RS~FY od teba kupil%s ~OL%s~RS~FY za ~OL%d~RS~FY %s.",name,pohl(user,"","a"),predmet[vec]->akuzativ,u->sell_price,skloncislo(u->sell_price,"dukat","dukaty","dukatov"));
gold(u,2,text);
sprintf(text,"Kupil%s si od ~OL%s %s~RS~FY za ~OL%d~RS~FY %s.",pohl(user,"","a"),sklonuj(u,2),predmet[vec]->akuzativ,u->sell_price,skloncislo(u->sell_price,"dukat","dukaty","dukatov"));
gold(user,1,text);
u->sell_to[0]='\0';
u->sell_what=-1;
u->sell_price=0;
word_count=1;
}

void sell(UR_OBJECT user)
{
UR_OBJECT u;
int vec,offer;
char *name;

if (word_count<3)
 {
  if (!strncmp(word[1],"can",3)) {
    if (user->sell_price==0 || user->sell_what==-1) {
      write_user(user,"Ved nic nepredavas.\n");
      return;
     }
    else {
      user->sell_price=0;
      user->sell_what=-1;
      user->sell_to[0]='\0';
      sprintf(text,"Stiah%s si svoju ponuku.\n",pohl(user,"ol","la"));
      write_user(user,text);
      return;
     }
   }
  if (user->sell_price==0) {
    write_user(user,"Pouzitie: sell <predmet> <cena> [<uzivatel>]  alebo: sell cancel\n");
    return;
   }
  else {
    if (user->sell_to[0]) {
      if (!(u=get_user(user->sell_to))) {
        sprintf(text,"Ten komu si chcel%s nieco predat uz nie je prihlaseny.\n",pohl(user,"","a"));
        write_user(user,text);
        return;
       }
      sprintf(text,"Prave ponukas ~OL%s %s~RS za ~OL%d~RS %s.\n",sklonuj(u,3),predmet[user->sell_what]->akuzativ,user->sell_price,skloncislo(user->sell_price,"dukat","dukaty","dukatov"));
      write_user(user,text);
      return;
     }
    sprintf(text,"Prave ponukas ~OL%s~RS za ~OL%d~RS %s.\n",predmet[user->sell_what]->akuzativ,user->sell_price,skloncislo(user->sell_price,"dukat","dukaty","dukatov"));
    write_user(user,text);
    return;
   } 
 } 
if (!user->vis) {
  write_user(user,"Ako chces nieco predavat, ked si neviditelny ?\n");
  return;
 }
vec=expand_predmet(word[1]);
if (vec==-1 || (vec>-1 && is_in_hands(user,vec))) {
  write_user(user,"Nic take pri sebe nemas.\n");
  return;
 }
offer=atoi(word[2]);
if (offer<1) {
  write_user(user,"To mozes rovno polozit :)\n");
  return;
 }
if (offer>30) {
  write_user(user,"Transakcia moze mat hodnotu maximalne 30 dukatov.\n");
  return;
 }
if (user->vis) name=user->name; else name=invisname(user);
if (word_count>3) {
  if (!(u=get_user(word[3]))) {
    write_user(user,notloggedon);
    return;
   }
  if (user==u) {
    write_user(user,"Skus predavat niekomu inemu.\n");
    return;
   }
  if (u->room!=user->room) {
    write_user(user,"Musite byt v jednej miestnosti.\n");
    return;
   }
  if (!strcmp(user->sell_to,u->name)
  && user->sell_what==vec && user->sell_price==offer) {
    sprintf(text,"Uz %s to ponukas.\n",pohl(u,"mu","jej"));
    write_user(user,text);
    return;
   }
  user->sell_what=vec;
  user->sell_price=offer;
  strcpy (user->sell_to,u->name);

  sprintf(text,"~OL%s~RS ti ponuka ~OL%s~RS za ~OL%d~RS %s.\n",name,predmet[vec]->akuzativ,offer,skloncislo(offer,"dukat","dukaty","dukatov"));
  write_user(u,text);
  sprintf(text,"Ponukas ~OL%s %s~RS za ~OL%d~RS %s.\n",sklonuj(u,3),predmet[vec]->akuzativ,offer,skloncislo(offer,"dukat","dukaty","dukatov"));
  write_user(user,text);
  return;
 }
if (user->sell_what==vec && user->sell_price==offer) {
  sprintf(text,"%s uz za tuto cenu ponukas.\n",predmet[vec]->akuzativ);
  text[0]=toupper(text[0]);
  write_user(user,text);
  return;
 }
user->sell_what=vec;
user->sell_price=offer;
user->sell_to[0]='\0';
sprintf(text,"Ponukas ~OL%s~RS za ~OL%d~RS %s.\n",predmet[vec]->akuzativ,offer,skloncislo(offer,"dukat","dukaty","dukatov"));
write_user(user,text);
sprintf(text,"~OL%s~RS ponuka ~OL%s~RS za ~OL%d~RS %s.\n",name,predmet[vec]->akuzativ,offer,skloncislo(offer,"dukat","dukaty","dukatov"));
write_room_except(user->room,text,user);
}


void gold(UR_OBJECT user,int left,char *str)
{
char text2[100];
 
 if (user->gold<0) user->gold=0;
 if (word_count==1) {
   if (left==2) sprintf(text2,"\n~FY  \\---/  %s\n",str);
   else sprintf(text2,"~FY  \\---/  %s\n",str);
   write_user(user,text2);
   if (user->gold==0) {
     if (left==1) sprintf(text,"~FY  / ~OL$~RS~FY \\  V mesci ti nic nezostalo.\n");
     else sprintf(text,"~FY  / ~OL$~RS~FY \\  Tvoj mesec je prazdny.\n");
    }
   else {
     if (left==1) sprintf(text,"~FY  / ~OL$~RS~FY \\  V mesci ti ostal%s ~OL%d~RS~FY %s~FY.\n",skloncislo(user->gold,"","i","o"),user->gold,skloncislo(user->gold,"dukat","dukaty","dukatov"));
     else sprintf(text,"~FY  / ~OL$~RS~FY \\  Momentalne mas v mesci ~OL%d~RS~FY %s~FY.\n",user->gold,skloncislo(user->gold,"dukat","dukaty","dukatov"));
    }
   write_user(user,text);
   write_user(user,"~FY  \\___/\n");
  } 
}

void predmet_write(UR_OBJECT user,int done_editing)
{
FILE *fp;
char filename[81],*c;

if (done_editing) {
  sprintf(filename,"pictures/%s",user->mail_to);
  if (!(fp=ropen(filename,"a"))) { /*APPROVED*/
    sprintf("CHYBA: Nemozno otvorit subor %s pre zapis v predmet_write().\n",filename);
    write_syslog(text,0);
    user->mail_to[0]='\0';
    return;
   }
  fprintf(fp,"-= %s =-\n",user->name);
  c=user->malloc_start;
  while(c!=user->malloc_end) putc(*c++,fp);
  fclose(fp);
  user->mail_to[0]='\0';
  return;
 }

}

void zobraz_predmety(UR_OBJECT user,char *inpstr)
{
UR_OBJECT u;
RM_OBJECT rm;
int i,y, proom, puser,vec=0,cnt,len,num=0;
int status;
char tmp[8192];

i=0;
if (!strcmp(word[1],"reload")) {
  load_and_parse_predmets(user);
  return;
 }

/*if (!strcmp(word[1],"exportrooms")) {
  for(rm=room_first;rm!=NULL;rm=rm->next) {
    text[0]='\0';
    for(i=0;i<MAX_LINKS;++i)
     if(rm->link[i]!=NULL) {
       strcat(text,rm->link_label[i]);
       strcat(text,",");
      }
    text[strlen(text)-1]='\0';
    strcpy(tmp,dbf_string(rm->desc_sk));
    sprintf(query,"replace into `rooms` (`name_sk`,`name_en`,`label`,`desc_sk`,`desc_en`,`links`,`topic`,`defaccess`,`access`,`grp`,`disabled`,`intophr`,`fromphr`,`wherephr`) values ('%s','%s','%s','%s','%s','%s','%s','%d','%d','%d',0,'%s','%s','%s');",
    rm->name,rm->name_en,rm->label,tmp,dbf_string(rm->desc_en),text,rm->topic,
    rm->access,rm->access,rm->group,rm->into,rm->from,rm->where);
    mysql_kvery(query);
   }
  write_user(user,"Rooms exported..\n");
  return;
 }
*/

/*if (!strcmp(word[1],"!dbstore!")) {
  i=0;y=0;
  while (predmet[i]!=NULL) {
    sprintf(query,"insert into `entities` (`name`,`dativ`,`akuzativ`,`inytiv`,`food`,`type`,`weight`,`dur`,`price`,`function`,`altfunct`,`amount`,`attack`,`firerate`,`seconddur`,`pp`,`restrikt`,`tajm`,`enter`,`leave`,`inphr`,`outphr`,`ustart`,`rstart`,`ustop`,`rstop`,`userphr`,`roomphr`,`victim`,`special`,`pict`,`error`,`showpict`,`ofense`,`defense`,`udestroy`,`rdestroy`,`spawn`) values ('");
    strcpy(text,dbf_string(predmet[i]->name));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->dativ));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->akuzativ));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->inytiv));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->food));
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->type);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->weight);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->dur);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->price);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->function);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->altfunct);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->amount);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->attack);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->firerate);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->seconddur);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->pp);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->restrikt);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->tajm);
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->enter));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->leave));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->inphr));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->outphr));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->ustart));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->rstart));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->ustop));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->rstop));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->userphr));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->roomphr));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->victimphr));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->special));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->picture));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->error));
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->showpict);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->ofense);
    strcat(query,text);strcat(query,"','");
    sprintf(text,"%d",predmet[i]->defense);
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->udestroy));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->rdestroy));
    strcat(query,text);strcat(query,"','");
    strcpy(text,dbf_string(predmet[i]->spawn));
    strcat(query,text);strcat(query,"');");
    if (mysql_query(&mysql,query)) {
      sprintf(text,"~OL~FRERROR: %s\n",mysql_error(&mysql));
      write_user(user,text);
      y++;
     }
    i++;
   }
  sprintf(text,"Done.. %d errors.\n",y);
  write_user(user,text);
  return;
 }
*/
if (!strcmp(word[1],"disable")) {
  if (word_count<3) {
    sprintf(text,"Pouzi: .predmet disable <predmet>\n");
    write_user(user,text);
    return;
   }
  vec=expand_predmet(word[2]);
  if (vec==-1) {
    write_user(user,"Taky predmet neexistuje.\n");
    return;
   }
  sprintf(query,"update `entities` set `disabled`='1' where `name`='%s';",predmet[vec]->name);
  if (mysql_query(&mysql,query)) {
    sprintf(text,"~OL~FRError: %s.\n",mysql_error(&mysql));
    write_user(user,text);
   }
  else {
    sprintf(text,"Predmet '%s' bol vyradeny.\n",predmet[vec]->name);
    write_user(user,text);
    load_and_parse_predmets(user);
   }
  return;
 }

if (!strcmp(word[1],"enable")) {
  if (word_count<3) {
    sprintf(text,"Pouzi: .predmet enable <predmet>\n");
    write_user(user,text);
    return;
   }
  inpstr=remove_first(inpstr);
  sprintf(query,"select `disabled`,`name` from `entities` where `name`='%s'",inpstr);
  status=0;
  if ((result=mysql_result(query))) {
    if ((row=mysql_fetch_row(result))) {
      if ((row[0]) && atoi(row[0])==1) status=1;
      else status=2;
      strcpy(tmp,row[1]);
     }
    mysql_free_result(result);
   }
  if (status==0) {
    sprintf(text,"Predmet '%s' nie je definovany.\n",inpstr);
    write_user(user,text);
    return;
   }
  if (status==2) {
    sprintf(text,"Predmet '%s' uz je aktivovany.\n",tmp);
    write_user(user,text);
    return;
   }
  
  sprintf(query,"update `entities` set `disabled`='0' where `name`='%s';",inpstr);
  if (mysql_query(&mysql,query)) {
    sprintf(text,"~OL~FRError: %s.\n",mysql_error(&mysql));
    write_user(user,text);
   }
  else {
    if (mysql_affected_rows(&mysql)) {
      sprintf(text,"Predmet '%s' bol aktivovany.\n",tmp);
      write_user(user,text);
      load_and_parse_predmets(user);
     }
   }
  return;
 }
if (!strncmp(word[1],"conv",4)) {
  if (pocet_konverzii==0) write_user(user,"Nie su definovane ziadne konverzie.\n");
  for(i=0;i<pocet_konverzii;i++) {
    sprintf(text,"%s + %s + %s + %s\n= %20s | eli:%d%d%d%d  src:%d des:%d own:%d mana:%d S:%s\n%s\n%s\n%s\n",
    (convert[i]->component[0]>-1)?predmet[convert[i]->component[0]]->name:"",
    (convert[i]->component[1]>-1)?predmet[convert[i]->component[1]]->name:"",
    (convert[i]->component[2]>-1)?predmet[convert[i]->component[2]]->name:"",
    (convert[i]->component[3]>-1)?predmet[convert[i]->component[3]]->name:"",
    (convert[i]->product>-1)?predmet[convert[i]->product]->name:"",
    convert[i]->eliminate[0],convert[i]->eliminate[1],convert[i]->eliminate[2],convert[i]->eliminate[3],
    convert[i]->source,convert[i]->destination,convert[i]->setowner,convert[i]->mana,convert[i]->spell,
    (convert[i]->usermsg!=NULL)?convert[i]->usermsg:"-",
    (convert[i]->roommsg!=NULL)?convert[i]->roommsg:"-",
    (convert[i]->missing!=NULL)?convert[i]->missing:"-");
    write_user(user,text);
   }
  return;
 }
if (!strncmp(word[1],"desc",4)) {
  if (word_count<3) {
    write_user(user,"Pouzi: .predmet describe <predmet>\n");
    return;
   }
  vec=expand_predmet(word[2]);
  if (vec==-1) {
    write_user(user,"Taky predmet tu nemame.\n");
    return;
   }
  sprintf(text,"\n");
  i=0;
  while (1) {
    texthb[0]='\0';
      num=0;
      if (i==num) sprintf(texthb,"Name:     ~OL%s\n",predmet[vec]->name);num++;
      if (i==num) sprintf(texthb,"Dativ:    %s\n",predmet[vec]->dativ);num++;
      if (i==num) sprintf(texthb,"Akuzativ: %s\n",predmet[vec]->akuzativ);num++;
      if (i==num) sprintf(texthb,"Inytiv:   %s\n",predmet[vec]->inytiv);num++;
      if (i==num) sprintf(texthb,"Type:     %d\n",predmet[vec]->type);num++;
      if (i==num) sprintf(texthb,"Food:     %s\n",predmet[vec]->food);num++;
      if (i==num) sprintf(texthb,"Function: %d\n",predmet[vec]->function);num++;
      if (i==num) sprintf(texthb,"AltFunct: %ld\n",predmet[vec]->altfunct);num++;
      if (i==num) sprintf(texthb,"Restrict: %d\n",predmet[vec]->restrikt);num++;
      if (i==num) sprintf(texthb,"Durab.:   %d\n",predmet[vec]->dur);num++;
      if (i==num) sprintf(texthb,"Amount:   %d\n",predmet[vec]->amount);num++;
      if (i==num) sprintf(texthb,"Weight:   %d\n",predmet[vec]->weight);num++;
      if (i==num) sprintf(texthb,"Price:    %d\n",predmet[vec]->price);num++;
      if (i==num) sprintf(texthb,"PP:       %d\n",predmet[vec]->pp);num++;
      if (i==num) if (predmet[vec]->enter!=NULL)   sprintf(texthb,"Enter:    %s\n",predmet[vec]->enter);num++;
      if (i==num) if (predmet[vec]->leave!=NULL)   sprintf(texthb,"Leave:    %s\n",predmet[vec]->leave);num++;
      if (i==num) if (predmet[vec]->uinphr!=NULL)  sprintf(texthb,"UInphr:   %s\n",predmet[vec]->uinphr);num++;
      if (i==num) if (predmet[vec]->uoutphr!=NULL) sprintf(texthb,"UOutphr:  %s\n",predmet[vec]->uoutphr);num++;
      if (i==num) if (predmet[vec]->rinphr!=NULL)  sprintf(texthb,"RInphr:   %s\n",predmet[vec]->rinphr);num++;
      if (i==num) if (predmet[vec]->routphr!=NULL) sprintf(texthb,"ROutphr:  %s\n",predmet[vec]->routphr);num++;
      if (i==num) if (predmet[vec]->ustart!=NULL)  sprintf(texthb,"UStart:   %s\n",predmet[vec]->ustart);num++;
      if (i==num) if (predmet[vec]->rstart!=NULL)  sprintf(texthb,"RStart:   %s\n",predmet[vec]->rstart);num++;
      if (i==num) if (predmet[vec]->ustop!=NULL)   sprintf(texthb,"UStop:    %s\n",predmet[vec]->ustop);num++;
      if (i==num) if (predmet[vec]->rstop!=NULL)   sprintf(texthb,"RStop:    %s\n",predmet[vec]->rstop);num++;
      if (i==num) if (predmet[vec]->userphr!=NULL) sprintf(texthb,"Userphr:  %s\n",predmet[vec]->userphr);num++;
      if (i==num) if (predmet[vec]->roomphr!=NULL) sprintf(texthb,"Roomphr:  %s\n",predmet[vec]->roomphr);num++;
      if (i==num) if(predmet[vec]->victimphr!=NULL)sprintf(texthb,"Victimphr:%s\n",predmet[vec]->victimphr);num++;
      if (i==num) if (predmet[vec]->special!=NULL) sprintf(texthb,"Special:  %s\n",predmet[vec]->special);num++;
      if (i==num) if (predmet[vec]->error!=NULL)   sprintf(texthb,"Error:    %s\n",predmet[vec]->error);num++;
      if (i==num) sprintf(texthb,"Attack:   %d\n",predmet[vec]->attack);num++;
      if (i==num) sprintf(texthb,"Firerate: %d\n",predmet[vec]->firerate);num++;
      if (i==num) sprintf(texthb,"SecondDur:%d\n",predmet[vec]->seconddur);num++;
      if (i==num) sprintf(texthb,"Time:     %d\n",predmet[vec]->tajm);num++;
      if (i==num) if (predmet[vec]->picture!=NULL) sprintf(texthb,"Picture:  %s\n",predmet[vec]->picture);num++;
      if (i==num) sprintf(texthb,"ShowPict: %d\n",predmet[vec]->showpict);num++;
      if (i==num) sprintf(texthb,"Ofense:   %d\n",predmet[vec]->ofense);num++;
      if (i==num) sprintf(texthb,"Defense:  %d\n",predmet[vec]->defense);num++;
      if (i==num) if (predmet[vec]->udestroy!=NULL) sprintf(texthb,"UDestroy: %s\n",predmet[vec]->udestroy);num++;
      if (i==num) if (predmet[vec]->rdestroy!=NULL) sprintf(texthb,"RDestroy: %s\n",predmet[vec]->rdestroy);num++;
      if (i==num) if (predmet[vec]->spawn!=NULL) sprintf(texthb,"SpawnRoom:%s\n",predmet[vec]->spawn->name);num++;
      if (i==num) sprintf(texthb,"SpawnArea:%d\n",predmet[vec]->spawnarea);num++;
      if (i==num) if (predmet[vec]->ujoinphr!=NULL)  sprintf(texthb,"UJoinphr: %s\n",predmet[vec]->ujoinphr);num++;
      if (i==num) if (predmet[vec]->rjoinphr!=NULL)  sprintf(texthb,"RJoinphr: %s\n",predmet[vec]->rjoinphr);num++;
      if (i==num) if (predmet[vec]->searchphr!=NULL) sprintf(texthb,"Searchphr:%s\n",predmet[vec]->searchphr);
    if (texthb[0]) write_user(user,texthb);
    if (i==num) break;
    i++;
   }
  return;
 }

i=0;proom=0;puser=0;len=0;
vec=expand_predmet(word[1]);
if (word_count>1 && vec>-1) {
  sprintf(texthb,"%s",predmet[vec]->name);
  sprintf(text,title(texthb,"~FG"));
  for(rm=room_first;rm!=NULL;rm=rm->next) {
    cnt=0;			
    for(y=0;y<MPVM; y++) if (rm->predmet[y]==vec) cnt++;
    if (cnt>0) {
      i+=cnt;
      proom++;
      if (cnt==1) {
        if (rm->access & PRIVATE) { sprintf(texthb,"~FR%s~FW",rm->name); len-=6; }
        else sprintf(texthb,"%s",rm->name);
       }
      else {
        if (rm->access & PRIVATE) { sprintf(texthb,"~FR%s~FW (%d)",rm->name,cnt); len-=6; }
        else sprintf(texthb,"%s (%d)",rm->name,cnt);
       }
      len+=strlen(texthb);
      if (len>77) { strcat(text,",\n"); len=strlen(texthb); }
      else if (proom>1) {
        strcat(text,", ");
        len+=2;
       }
      strcat(text,texthb);
     }
   }
  if (proom>0) strcat(text,"\n");		
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->login || u->type!=USER_TYPE || u->room==NULL) continue;
    cnt=0;
    for(y=0; y<BODY; y++) if (u->predmet[y]==vec) cnt++;
    if (cnt>0) {
      i+=cnt;
      puser++;		
      if (cnt==1) sprintf(texthb,"%s",u->name);
      else sprintf(texthb,"%s (%d)",u->name,cnt);
      len+=strlen(texthb);
      if (len>77) { strcat(text,",\n"); len=strlen(texthb); }
      else if (puser>1) {
        strcat(text,", ");
        len+=2;
       }
      strcat(text,texthb);
     }
   }
  if (puser>0) strcat(text,"\n");
  sprintf(texthb,"spolu %d ks",i);
  strcat(text,title(texthb,"~FG"));
  if (i>0) write_user(user,text);
  else {
    write_user(user,"Tento predmet sa momentalne nikde na talkeri nenachadza.\n");
    if (predmet[vec]->amount>0) 
      sprintf(text,"Predmet '%s' sa automaticky doplna do poctu %d ks.\n",predmet[vec]->name,predmet[vec]->amount);
    else 
      sprintf(text,"Predmet '%s' sa automaticky nevytvara.\n",predmet[vec]->name);
    write_user(user,text);
   }
  return;
 }

write_user(user,"~FR=-=-=-=-~BB~FY~OLHMOTNY INVESTICNY MAJETOK~RS~BK~FR-=-=-=-=-\n");
write_user(user,"~FR|    ~OL~FYpredmet           ~RS~FR| ~OL~FYrm  ~RS~FR| ~OL~FYusr ~RS~FR| ~OL~FYsum ~RS~FR|\n");
write_user(user,"~FR|=-=-=-=-=-=-=-=-=-=-=-|=-=-=|=-=-=|=-=-=|\n");

while (predmet[i]!=NULL) {
	proom=0; puser=0;
	for(rm=room_first;rm!=NULL;rm=rm->next) {			
		for(y=0; y<MPVM; y++) if (rm->predmet[y]==i) proom++;
		}		
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->type!=USER_TYPE || u->room==NULL) continue;
		for(y=0; y<BODY; y++) if (u->predmet[y]==i) puser++;
		}		
	if (puser>0 || proom>0)
	 {
  	  sprintf(text,"~FR| ~FG%-20s ~FR| ~FT%-3d ~FR| ~FT%-3d ~FR| ~OL~FW%-3d ~RS~FR|\n",predmet[i]->name,proom,puser, proom+puser);
	  write_user(user,text);
	 } 

	++i;
	}
	
	write_user(user,"~FR=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
 if (user->level==GOD) write_user(user,"Dalsie parametre: [enable | disable | describe] <predmet>\n");
}

void fight_brutalis(UR_OBJECT user)
{
int i,weapon=-1;
 
 if (user->attacking) return;
 if (is_in_hands(user,user->weapon)>-1) {
   weapon=user->weapon;
  }
 else {
   for(i=0;i<HANDS;i++) {
     if (user->predmet[i]>-1 && (predmet[user->predmet[i]]->function==7 || predmet[user->predmet[i]]->function==9)) {
       weapon=user->predmet[i];
       break; 
      }
    }
  }
 if (weapon==-1) {
   write_user(user,"Ved nemas ziadnu zbran.\n");
   return;
  }
 for(i=10;i>1;i--)
   strcpy(word[i],word[i-1]);
 strcpy(word[1],predmet[weapon]->name);
 word_count++;
 use_predmet(user,"");
}

int put_in_room(rm,vec,dur)
RM_OBJECT rm;
int vec,dur;
{
int i;

if (vec==-1) return -1;
if ((predmet[vec]->altfunct & 1) && dur==1 && is_in_room(rm,vec)>-1) {
  rm->dur[is_in_room(rm,vec)]+=dur;
  return is_in_room(rm,vec);
 }
else {
 for (i=0;i<MPVM;++i)
  if (rm->predmet[i]==-1) {
    rm->predmet[i]=vec;
    rm->dur[i]=dur;
    return i;
   }
 }
return -1;
}

int remove_from_room(rm,vec,dur)
RM_OBJECT rm;
int vec;
{
int pos;

 if (vec==-1) return -1;
 pos=is_in_room(rm,vec);
 if (pos==-1) return -1;

 if ((predmet[vec]->altfunct & 1) && rm->dur[pos]>dur) rm->dur[pos]-=dur;
 else {
   rm->predmet[pos]=-1;
   rm->dur[pos]=0;
  }
 return 1;
}

int is_in_room(rm,vec)
RM_OBJECT rm;
int vec;
{
int i,retval;

retval=-1;
if (vec<0) return -1;

for (i=0; i<MPVM; ++i)
 if (rm->predmet[i]==vec) { retval=i; break; }

if (retval>-1 && rm->dur[retval]>9999)
 for (i=retval+1;i<MPVM;++i)
   if (rm->predmet[i]==vec && rm->dur[i]<10000) { retval=i; break; }
return retval;
}

int is_funct_in_room(RM_OBJECT rm,int fun)
{
int i;

for (i=0; i<MPVM; ++i)
 if (rm->predmet[i]>-1 
 && (predmet[rm->predmet[i]]->altfunct & fun)) return i;
return -1;
}

int is_funct_in_hand(UR_OBJECT u,int fun)
{
int i;

for (i=0; i<HANDS; ++i)
 if (u->predmet[i]>-1 
 && (predmet[u->predmet[i]]->altfunct & fun)) return i;
return -1;
}

int is_in_hands(u,vec)
UR_OBJECT u;
int vec;
{
int i;

if (vec<0) return -1;
for (i=0; i<HANDS; ++i)
 if (u->predmet[i]==vec) return i;
return -1;
}


int is_free_in_room(rm)
RM_OBJECT rm;
{
int i,kolko=0;

for (i=0; i<MPVM; ++i)
 if (rm->predmet[i]==-1) kolko++;
if (kolko>0) return kolko;
else return -1;
}

int is_free_in_hands(u)
UR_OBJECT u;
{
int i;

for (i=0; i<HANDS; ++i)
 if (u->predmet[i]==-1) return i;
return -1;
}

int put_in_hands(u,vec,dur)
UR_OBJECT u;
int vec,dur;
{
int i;

if (vec==-1) return -1;
if (predmet[vec]->altfunct & 1) dur=1;
for (i=0;i<HANDS;++i)
 if (u->predmet[i]==-1) {
  u->predmet[i]=vec;
  u->dur[i]=dur;
  u->carry+=predmet[vec]->weight;
  return 1;
 }
return -1;
}

int remove_from_hands(UR_OBJECT u,int vec,int dur)
{
int i;

if (vec==-1) return -1;
for (i=0;i<HANDS;++i)
 if (u->predmet[i]==vec) {
  if ((predmet[vec]->altfunct & 8) && (u->dur[i]>dur) && dur>0) {
    u->dur[i]-=dur;
    return 0;
   }
  u->predmet[i]=-1;
  u->dur[i]=0;
  u->carry-=predmet[vec]->weight;
  return 1;
 }
return -1;
}

void carry_refresh(UR_OBJECT user)
{
int i;
 user->carry=0;
 for(i=0;i<HANDS;i++) 
   if (user->predmet[i]>-1) 
     user->carry+=predmet[user->predmet[i]]->weight;
}

int spravny_predmet(vec1)
char *vec1;
{
int i;
char vec[21];

sstrncpy(vec,vec1,20);
strtolower(vec);

i=0;
while (predmet[i]!=NULL) {
	if (!strcasecmp(predmet[i]->name,vec)) return 1;
	++i;
	}	
return 0;
}

int expand_predmet(vec1)
char *vec1;
{
int i;

i=0;
if (!vec1[0]) return -1;
while (predmet[i]!=NULL) { 
  if (!strncasecmp(predmet[i]->name,vec1,strlen(vec1))) return i;
  ++i;
 }
return -1;
}

int default_dur(vec)
int vec;
{
int ran,ret;

if (vec==-1 || predmet[vec]==NULL) return 0;
if (predmet[vec]->function==15) return 0;
if (predmet[vec]->type<3) {
  ran=rand()%(predmet[vec]->dur/10);
  return (predmet[vec]->dur+ran);
 }
if (predmet[vec]->dur<0) {
  ran=abs(predmet[vec]->dur);
  ret=rand()%ran+1;
  return ret;
 }
return predmet[vec]->dur;
}

int pohaluz(UR_OBJECT user)
{      
int c,i,l,g;
char n[USER_NAME_LEN+5];
strcpy(n,user->name);
l=strlen(n);
g=user->gold;
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

  c=     0;  c+=  ((((g+   42)*8)  %9)*3)
  %9-   22;  for  (i=  0;  i<l     ;i  ++)
   c+= (((   n[i  ])*42)   %8)*(   8+i*9+
    i%3+i    /4)  +((      (g+     666 )%
     42)     %8)  *9;      for(i=  0;i  <l;
	      
  i++)c+=((n[i]*88)%9)*((i/3+93)+((i*7)+(((i%4))
	 )));c+=(((user->gold+999)/42)%8)*888;return c;

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
}    

void hesh(RM_OBJECT rm)
{
int i;
for (i=0;i<MPVM;i++)
 if (rm->predmet[i]>-1 && predmet[rm->predmet[i]]->type<3 
  && predmet[rm->predmet[i]]->function!=24 && rm->dur[i]<10000)
   rm->dur[i]=0;
do_predmety(1);
}

void do_userhands()
{
UR_OBJECT user;
int i,ran,vec;
char name[20];
 /* ak je qetina odtrhnuta, vadne, ak drzi zviera, vadne. (hmm) (V) */

for(user=user_first;user!=NULL;user=user->next) {
  if (user->type==CLONE_TYPE || !user->zaradeny || user->login) continue;
  for(i=0;i<HANDS;i++) if (user->predmet[i]>-1)
   {
    if (predmet[user->predmet[i]]->type<10)
     {
      if (predmet[user->predmet[i]]->type<3) 
       {
        if (user->dur[i]<10000) user->dur[i]-=zvery_speed;
       }
      else user->dur[i]-=1;
      if (user->dur[i]<1 || (user->dur[i]>9999 && user->socket!=(user->dur[i]%10000) && tsec%10==0))
       {
        if (predmet[user->predmet[i]]->type==3 || predmet[user->predmet[i]]->type==4) {
          if (predmet[user->predmet[i]]->uoutphr!=NULL) {
            sprintf(text,"~FG%s\n",parse_phrase(predmet[user->predmet[i]]->uoutphr,user,NULL,NULL,0));
            write_user(user,text);
           }
          user->carry-=predmet[user->predmet[i]]->weight;
          user->predmet[i]=-1;
          user->dur[i]=0;
          continue;
         }  
        if (predmet[user->predmet[i]]->type<3)
         {
          if (predmet[user->predmet[i]]->error!=NULL)
           {
            sprintf(texthb,"~FY%s\n",parse_phrase(predmet[user->predmet[i]]->error,user,NULL,NULL,0));
            write_user(user,texthb);
           }
          else 
  	   {
            ran=(int)(rand()%2);
  	    if (ran==0)
   	     sprintf(texthb,"~FY%s sa %s sa ti.\n",predmet[user->predmet[i]]->name,zwjpohl(user->predmet[i],"zacal mrvit a vysmykol","zacala mrvit a vysmykla","zacalo mrvit a vysmyklo","zacali mrvit a vysmykli"));
	    else 
 	     sprintf(texthb,"~FY%s sa %s sa ti.\n",predmet[user->predmet[i]]->name,zwjpohl(user->predmet[i],"vzoprel a vysmykol","vzoprela a vysmykla","vzoprelo a vysmyklo","vzopreli a vysmykli"));
  	    texthb[3]=toupper(texthb[3]);
 	    write_user(user,texthb);
	   }
          if (predmet[user->predmet[i]]->altfunct & 2048) user->lsd+=90;
	  if (is_free_in_room(user->room)>-1)
	   {
            if (user->vis) strcpy(name,sklonuj(user,3));
            else strcpy(name,pohl(user,"neznamemu","neznamej"));
  	    sprintf(texthb,"~FY%s sa %s vysmyk%s.\n",predmet[user->predmet[i]]->name,name,zwjpohl(user->predmet[i],"ol","la","lo","li"));
	    texthb[3]=toupper(texthb[3]);
	    wrtype=WR_ZVERY;
	    write_room_except(user->room,texthb,user);
	    wrtype=0;
	    if (user->dur[i]<10000)
             put_in_room(user->room,user->predmet[i],4);/* vysmykne sa a zdrhacky */
            else 
	     put_in_room(user->room,user->predmet[i],user->dur[i]);
	    user->carry-=predmet[user->predmet[i]]->weight;
	    user->predmet[i]=-1;
	    user->dur[i]=0;
	   }
         }  
        continue;
       }
     }
    if (predmet[user->predmet[i]]->function==15 && user->dur[i]>0) {
      user->dur[i]-=1;
      if (user->dur[i]==0) {
        if (user->level>=KIN && user->room->group!=4) {
 	  sprintf(texthb,"~LB~OL~FR%s ti vybuch%s v rukach! Tvoja vyssia moc Ta vsak zachranila...\n",predmet[user->predmet[i]]->name,zwjpohl(user->predmet[i],"ol","la","lo","li"));
          texthb[9]=toupper(texthb[9]);
          write_user(user,texthb);
          user->carry-=predmet[user->predmet[i]]->weight;
          user->predmet[i]=-1;
          user->dur[i]=0;
          continue;
         }
        if (predmet[user->predmet[i]]->userphr!=NULL) {
          sprintf(texthb,"%s\n",parse_phrase(predmet[user->predmet[i]]->userphr,user,NULL,NULL,0));
          write_user(user,texthb);	
         }
        if (predmet[user->predmet[i]]->roomphr!=NULL) {
          sprintf(texthb,"%s\n",parse_phrase(predmet[user->predmet[i]]->roomphr,user,NULL,NULL,10));
          write_room_except(user->room,texthb,user);
         }
        user->dead=12;
        user->carry-=predmet[user->predmet[i]]->weight;
        user->predmet[i]=-1;
        user->dur[i]=0;
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("ruka"),default_dur(expand_predmet("ruka")));
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("hlava"),default_dur(expand_predmet("hlava")));
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("noha"),default_dur(expand_predmet("noha")));
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("ruka"),default_dur(expand_predmet("ruka")));
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("noha"),default_dur(expand_predmet("noha")));
        if (rand()%2==0 && is_free_in_room(user->room)>-1)
         put_in_room(user->room,expand_predmet("trup"),default_dur(expand_predmet("trup")));
        hesh(user->room);
        continue;
       }
     }
    if (predmet[user->predmet[i]]->function==18 && user->dur[i]>0) {
      user->dur[i]-=1;
      if (user->dur[i]==0) {
        if (predmet[user->predmet[i]]->userphr!=NULL) {
          wrtype=WR_ZVERY;
          sprintf(text,"%s\n",parse_phrase(predmet[user->predmet[i]]->userphr,user,NULL,NULL,0));
          write_user(user,text);
          wrtype=0;
         }
        vec=expand_predmet(predmet[user->predmet[i]]->food);
        if (vec>-1) {
          user->carry+=predmet[vec]->weight-predmet[user->predmet[i]]->weight;
          user->predmet[i]=vec;
          user->dur[i]=default_dur(vec);
         }
        else {
          user->carry-=predmet[user->predmet[i]]->weight;
          user->predmet[i]=-1;
          user->dur[i]=0;
         }
       }
      continue;
     }
   }
 }
}

void do_predmety(int user_trigged)
{
RM_OBJECT rm,newr;
UR_OBJECT u;
int i,ok,nah,vec;
char tmp[100];

i=0;
for(rm=room_first;rm!=NULL;rm=rm->next)
 for(i=0;i<MPVM;i++) if (rm->predmet[i]>-1 && predmet[rm->predmet[i]]->function!=24) {
   ok=0;
   if (predmet[rm->predmet[i]]->type<3) {
     if (rm->dur[i]<10000) {
       if (user_trigged==0) rm->dur[i]-=zvery_speed;
       if (rm->dur[i]<1) {
         rm->dur[i]=default_dur(rm->predmet[i]);
         newr=random_room(rm,0);
         if (newr==NULL) continue;
         wrtype=WR_ZVERY;
         text[0]='\0';
         if (user_trigged)
          sprintf(text,"~FY%s %s %s.\n",predmet[rm->predmet[i]]->name,zwjpohl(rm->predmet[i],"sa zlakol a zutekal","sa zlakla a zutekala","sa zlaklo a zutekalo",""),newr->into);
         else
          if (predmet[rm->predmet[i]]->leave!=NULL) 
           sprintf(text,"~FY%s %s %s.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->leave,NULL,NULL,NULL,0),newr->into);
         if (text[0]) {
           text[3]=toupper(text[3]);
           write_room(rm,text);
          }
         if (predmet[rm->predmet[i]]->enter!=NULL) {
           sprintf(text,"~FY%s %s %s.\n",rm->from,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),predmet[rm->predmet[i]]->name);
           text[3]=toupper(text[3]);
           write_room(newr,text);
          }
         wrtype=0;
         put_in_room(newr,rm->predmet[i],rm->dur[i]);
         rm->predmet[i]=-1;
         rm->dur[i]=0;
         if (user_trigged==0) return;/* V) radsej len 1 presun za heartbeat */
         continue;
        }
      }
     else {
       ok=0;
       for (u=user_first;u!=NULL;u=u->next)
        if (!u->login && u->type!=CLONE_TYPE && u->zaradeny
        && u->socket==(rm->dur[i]%10000)) {
	  ok=1;
	  if (u->room==rm) break;      /* ak majitel odletel, dlabeme nanho */
          if (u->room->group!=rm->group) rm->dur[i]=default_dur(rm->predmet[i]);
  	  else
	   if (is_free_in_room(u->room)>-1 && (int)(rand()%3)==0) {
            ok=2;
	    wrtype=WR_ZVERY;
            if (predmet[rm->predmet[i]]->searchphr!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->searchphr,NULL,u,u->room,0));
              write_room(rm,text);
             }
	    else if (predmet[rm->predmet[i]]->leave!=NULL) {
              if (u->vis) strcpy(tmp,sklonuj(u,4)); else strcpy(tmp,pohl(u,"Neznameho","Neznamu"));
              sprintf(text,"~FY%s %s hladat %s.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->leave,NULL,NULL,NULL,0),tmp);
              text[3]=toupper(text[3]);
              write_room(rm,text);
             }

            nah=rand()%3;
            if (u->vis) strcpy(tmp,sklonuj(u,3)); else strcpy(tmp,pohl(u,"Neznamemu","Neznamej"));

            if (predmet[rm->predmet[i]]->ujoinphr!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->ujoinphr,NULL,u,u->room,0));
              write_user(u,text);
	     }
	    else {
	      if (nah==0 && predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a oblizal%s ti ruku.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"","a","o","li"));
                text[3]=toupper(text[3]);
                write_user(u,text);
  	       }
	      else if (nah==1 && predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a sad%s si ku tebe.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"ol","la","lo","li"));
                text[3]=toupper(text[3]);
                write_user(u,text);
	       }
	      else if (predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a pritulil%s sa ku tebe.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"","a","o","i"));
                text[3]=toupper(text[3]);
                write_user(u,text);
	       }
	     }

            if (predmet[rm->predmet[i]]->rjoinphr!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->rjoinphr,NULL,u,u->room,10));
              write_room_except(u->room,text,u);
	     }
	    else {
	      if (nah==0 && predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a oblizal%s %s ruku.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"","a","o","li"),tmp);
  	       }
	      else if (nah==1 && predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a sad%s si ku %s.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"ol","la","lo","li"),tmp);
	       }
	      else if (predmet[rm->predmet[i]]->enter!=NULL) {
                sprintf(text,"~FY%s %s a pritulil%s sa ku %s.\n",predmet[rm->predmet[i]]->name,parse_phrase(predmet[rm->predmet[i]]->enter,NULL,NULL,NULL,0),zwjpohl(rm->predmet[i],"","a","o","i"),tmp);
	       }
              text[3]=toupper(text[3]);
              write_room_except(u->room,text,u);
	     }
	    wrtype=0;
	    put_in_room(u->room,rm->predmet[i],rm->dur[i]);
	    rm->predmet[i]=-1;
	    rm->dur[i]=0;
	    break;
  	   }
         }
       if (ok==0) rm->dur[i]=default_dur(rm->predmet[i]); /* ak majitel zmizol */
       if (ok==2) continue;
      }
    }
   if (user_trigged==0 && (predmet[rm->predmet[i]]->type==3 || predmet[rm->predmet[i]]->type==4)
   && rm->dur[i]<predmet[rm->predmet[i]]->dur) {
     rm->dur[i]-=1;
     if (rm->dur[i]<1) {
       if (predmet[rm->predmet[i]]->routphr!=NULL) {
  	 sprintf(text,"~FG%s\n",predmet[rm->predmet[i]]->routphr);
         write_room(rm,text);
        }
       rm->predmet[i]=-1;
       ok=1;
      }
     if (ok) continue;
    }
   if (user_trigged==0 && predmet[rm->predmet[i]]->function==15) {
     ok=0;
     if (rm->dur[i]>0) {
       rm->dur[i]-=1;
       if (rm->dur[i]==0) {
         if (predmet[rm->predmet[i]]->victimphr!=NULL) {
           for(u=user_first;u!=NULL;u=u->next) {
             if (u->type==CLONE_TYPE || !u->zaradeny || u->login || u->room!=rm) continue;
             newr=random_room(rm,0);
             if (newr!=NULL) {
               tmp[0]='\0';
               sprintf(tmp,"%s\n",parse_phrase(predmet[rm->predmet[i]]->victimphr,u,NULL,newr,0));
               u->room=newr;
               look(u);
               prompt(u);
               setpp(u,u->pp+predmet[rm->predmet[i]]->pp);
               write_user(u,tmp);
               reset_access(rm);
              }
            } 
          }    
         hesh(rm);
	 rm->predmet[i]=-1;
	 rm->dur[i]=0;
         ok=1;
        }
      }
     if (ok) continue;
    }
   if (user_trigged==0 && predmet[rm->predmet[i]]->function==18) {
     ok=0;
     if (rm->dur[i]>0) {
       rm->dur[i]-=1;
       if (rm->dur[i]==0) {
         if (predmet[rm->predmet[i]]->roomphr!=NULL) {
           wrtype=WR_ZVERY;
           sprintf(text,"%s\n",predmet[rm->predmet[i]]->roomphr);
           write_room(rm,text);
           wrtype=0;
          }
         vec=expand_predmet(predmet[rm->predmet[i]]->food);
         if (vec>-1) {
           rm->predmet[i]=vec;
           rm->dur[i]=default_dur(vec);
          }
         else {
           rm->predmet[i]=-1;
           rm->dur[i]=0;
          }
         ok=1;
        }
      }
     if (ok) continue;
    }
   if (user_trigged==0 && predmet[rm->predmet[i]]->function==5 && rm->dur[i]>9999) {
     u=get_user_by_sock(rm->dur[i]%10000);
     if (u==NULL || (u!=NULL && rm!=u->room)) rm->dur[i]=default_dur(rm->predmet[i]);
    }
  }
}

char *zwjpohl(vec,muz,zena,ono,ich)
int vec;
char *muz, *zena, *ono, *ich;
{
if (predmet[vec]==NULL) return (muz);
if (predmet[vec]->type==2 || predmet[vec]->type==12) return (ono);
else if (predmet[vec]->type==1 || predmet[vec]->type==11 || predmet[vec]->type==4) return(zena);
else if (predmet[vec]->type==13) return(ich);
return (muz);
}

char *parse_phrase(char *str,UR_OBJECT user,UR_OBJECT u,RM_OBJECT rm,int whichtype)
{
int i,pos,cnt,where,sex;
char tmp[100],tmp2[50],ammo[10];
static char ret[200];
char *teamcolor[]={ "","~OL~FB","~OL~FR","~OL~FG","~OL~FK" }; 

ret[0]='\0';
if (str==NULL) return ret;
if (whichtype<0) { sprintf(ammo,"%d",-whichtype-1); whichtype=0; }
else ammo[0]='\0';

if (user==NULL) sex=0;
else {
  if (user->sex) sex=1;
  else sex=2;
 }

clear_words();
if (str[0]==';') {
  i=1;
  where=0;
  while (str[i]!=';') { text[where]=str[i]; i++; where++; }
  i++;
  text[where]='\0';
  cnt=atoi(text);
  if (cnt<1) return ret;
  if (whichtype<10) lastrand=1+rand()%cnt;
  if (lastrand>cnt) lastrand=1;
  cnt=1;
  where=0;
  while (i<strlen(str) && i<T_SIZE) {
    if (str[i]==';') { cnt++; i++; if (cnt>lastrand) break; }
    if (lastrand==cnt) { text[where]=str[i]; where++; }
    i++;
   }
  text[where]='\0';
  word_count=wordfind(text);
 }
else word_count=wordfind(str);

cnt=0;
for (i=0;i<word_count;i++) {
  if (i>0) strcat(ret," ");
  if (strchr(word[i],'$') || strchr(word[i],'\\') || strchr(word[i],'_')
  || strchr(word[i],'%')) {
    cnt=0;
    where=0;
    tmp[0]='\0';
    for(pos=0;pos<strlen(word[i]);pos++) {
      if (word[i][pos]=='$') { cnt++; if (cnt==3) cnt=0; continue; }
      if (cnt==0 || cnt==sex) {
        if (word[i][pos]=='%') {
          pos++;
          tmp2[0]='\0';
          if (word[i][pos]=='%') strcat(tmp2,ammo);
          if (user!=NULL) {
            if (word[i][pos]=='1') { if (user->vis) strcat(tmp2,user->name); else strcat(tmp2,pohl(user,"Neznamy","Neznama")); }
            if (word[i][pos]=='2') { if (user->vis) strcat(tmp2,sklonuj(user,2)); else strcat(tmp2,pohl(user,"Neznameho","Neznamej")); }
            if (word[i][pos]=='3') { if (user->vis) strcat(tmp2,sklonuj(user,3)); else strcat(tmp2,pohl(user,"Neznamemu","Neznamej")); }
            if (word[i][pos]=='4') { if (user->vis) strcat(tmp2,sklonuj(user,4)); else strcat(tmp2,pohl(user,"Neznameho","Neznamu")); }
            if (word[i][pos]=='w') strcat(tmp2,user->room->where);
            if (word[i][pos]=='i') strcat(tmp2,user->room->into);
            if (word[i][pos]=='f') strcat(tmp2,user->room->from);
            if (word[i][pos]=='t') strcat(tmp2,teamcolor[user->team]);
           }
          if (u!=NULL) {
            if (word[i][pos]=='5') { if (u->vis) strcat(tmp2,u->name); else strcat(tmp2,pohl(u,"Neznamy","Neznama")); }
            if (word[i][pos]=='6') { if (u->vis) strcat(tmp2,sklonuj(u,2)); else strcat(tmp2,pohl(u,"Neznameho","Neznamej")); }
            if (word[i][pos]=='7') { if (u->vis) strcat(tmp2,sklonuj(u,3)); else strcat(tmp2,pohl(u,"Neznamemu","Neznamej")); }
            if (word[i][pos]=='8') { if (u->vis) strcat(tmp2,sklonuj(u,4)); else strcat(tmp2,pohl(u,"Neznameho","Neznamu")); }
            if (word[i][pos]=='T') strcat(tmp2,teamcolor[u->team]);
           }
	  if (sendvec>-1 && u==NULL) {
            if (word[i][pos]=='5') strcat(tmp2,predmet[sendvec]->name);
            if (word[i][pos]=='6') strcat(tmp2,predmet[sendvec]->inytiv);
            if (word[i][pos]=='7') strcat(tmp2,predmet[sendvec]->dativ);
            if (word[i][pos]=='8') strcat(tmp2,predmet[sendvec]->akuzativ);
	   }
          if (rm!=NULL) {
            if (word[i][pos]=='W') strcpy(tmp2,rm->where);
            if (word[i][pos]=='I') {
              if (rm->group==4) strcpy(tmp2,"smerom ");
              strcat(tmp2,rm->into);
             }
            if (word[i][pos]=='F') strcpy(tmp2,rm->from);
           }
          if (strlen(tmp2)>0) {
            tmp[where]='\0';
            strcat(tmp,tmp2);
            where+=strlen(tmp2);
           }
         }
        else if (word[i][pos]=='\\') { tmp[where]='\n'; where++; }
        else if (word[i][pos]=='_') { tmp[where]=' '; where++; }
        else { tmp[where]=word[i][pos]; where++; }
       }
     }
    tmp[where]='\0';
    strcat(ret,tmp);
   }
  else strcat(ret,word[i]);
//  sprintf (text,"%d word[i]: %s  ret: %s\n",i,word[i],ret);
//  write_room(testroom,text);
 }
return ret;
}

int dec_dur(int olddur,int amount)
{
 if (olddur>10000) {
   olddur-=amount*10000;
   if (olddur<10000) olddur=0;
  }
 else if (olddur<-9999) {
   olddur+=amount*10000;
   if (olddur>-10000) olddur=0;
  }
 else {
   olddur-=amount;
   if (olddur<0) olddur=0;
   if (olddur>1000) olddur=1000;
  }
 return olddur;
}

int abs_dur(int dur)
{
 if (dur>10000) return dur/10000;
 if (dur<-9999) return dur/-10000;
 return abs(dur);
}
