/****************************************************************************
        Kuzla, cary a cierna magia - (C)1999/2000 Rider of LoneStar
 ****************************************************************************/
#include "atl-head.h"

extern void show_kniha_kuziel(UR_OBJECT user,int dur)
{
 char filename[100];
 sprintf(filename,"predmety/kuzlo.%02d",rand()%(POCET_KUZIEL+4));
 switch(more(user,user->socket,filename)) {
     case 0: write_user(user,"Tato cast knihy magie sa neda citat...\n");  break;
     case 1: user->misc_op=2;
     }
if (dur<1) {
     write_user(user,"~FGKniha sa zrazu rozplynula v oblaku prachu... smola...\n");
     remove_from_hands(user,expand_predmet("kniha"),0);
     return;
     }
 return;
}

/****************************************************************************
 Odpocitavac many, navratove hodnoty:
   0 - OK, kuzlo sa vykona
   1 - Failed (zlyhanie kuzla, nedostatok many, nevnhodne prisady...)
 first a second su prisady, NULL znamena, ze prisada nie je nutna
 ****************************************************************************/
int mana_check(UR_OBJECT user, int howmuch, int first, int second)
{
int prob,ok,i;


if (first==-1 && second!=-1)
   if (is_in_hands(user,second)==-1) {
        write_user(user,"~FGAjajaj, na toto kuzlo potrebujes ingrediencie, ktore veru nemas...\n");
	return 1;
       }

if (first!=-1 && second==-1)
   if (is_in_hands(user,first)==-1) {
        write_user(user,"~FGHmm... Asi by to chcelo este mat nejake prisady...\n");
	return 1;
       }
if (first!=-1 && second!=-1) {
  ok=0;
  for (i=0;i<HANDS;i++) {
    if (user->predmet[i]==first) { ok++; continue; }
    if (user->predmet[i]==second) ok++;
   }
  if (ok<2) {
    write_user(user,"~FGBez presnych prisad sa ti kuzlo nemoze podarit...\n");
    return 1;
   }
 }

 if (!howmuch) howmuch++;
 if (user->mana<howmuch) {
  user->mana-=((rand()%howmuch)/2);
  if (user->mana<0) user->mana=0;
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

prob=((GOD*SPELLK)-(user->mana))/30-1;
if (prob<1) prob=1;

if (rand()%prob) {
  switch(rand()%5) {
     case 0: write_user(user,"~FGHoci formulka bola spravna, kuzlo zlyhalo... Zeby chybali skusenosti?\n");
             break;
     case 1: write_user(user,"~FGVzduch sa zviril, kuzlo sa zdvihlo... no zoslablo a padlo...\n");
	     break;
     case 2: write_user(user,"~FGKuzlo sa roztocilo, vzlietlo... no zrutilo sa nazad na zem...\n");     
      	     break;
     case 3: write_user(user,"~FGMoc kuzla bola zahadne oslabena a kuzlo sa rozpadlo na criepky...\n");
	     break;
     case 4: write_user(user,"~FGKuzlo vzlietlo, no premenilo sa na dym a rozplynulo sa...\n");
             break;
     }
   user->mana-=((rand()%howmuch)/2);
   return 1;
   }

user->mana-=((howmuch-5)+(rand()%5));
if (first!=-1) remove_from_hands(user,first,0);
if (second!=-1) remove_from_hands(user,second,0);
return 0;
}

extern UR_OBJECT leave_me_alone(UR_OBJECT user,UR_OBJECT u)
{
 if (u==NULL) return NULL;
 if (u->room!=NULL && !strcmp(u->room->name,"svatyna")) {
   write_user(user,"~FGKuzlo sa odrazilo od magickej energie ktoru vyzaruje svatyna.\n");
   return user;
  }
 if (check_ignore_user(user,u)) {
   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
   return user;
  }
 return u;
}

extern void spell(UR_OBJECT user, char *inpstr)
{
char *runes[]={"ar","bro","fur","grd","ku","jag","lo","me","nop","paw",
		"sup","tra","vis","woch","yo","*"};
char *abc[]={"a","b","f","g","k","j","l","m","n","p",
	       "s","t","v","w","y"};
char *allowcom[]={".kiss",".go",".hug",".shout",".tell","*"};
/* v allowcom su prikazy, ktore je mozno forcovat cez kuzla ;-) */

UR_OBJECT target=user;
RM_OBJECT room;
char mword[1024], *ptr;
int i;
char *name;

if (word_count<2) {
   write_user(user,"Pouzitie: .spell <formulka> [parametre]\n");
   return;
   }

if (convert_predmet(user,1)) return;

ptr=word[1]; mword[0]='\0';
while (*ptr) {
   for (i=0;*runes[i]!='*';i++) {
     if (!strncmp(ptr,runes[i],strlen(runes[i]))) {
	strcat(mword,abc[i]);
	ptr+=(strlen(runes[i]));
	break;
	}
     }
   if (*runes[i]=='*') {
       write_user(user,"~FGMumles nezmysly, radsej si najskor nastuduj runove pismo...\n");
       return;
       }
   }

if (strlen(mword)<3) {
   write_user(user,"~FGNevedno urcit, ci to bola snaha o kuzlo, alebo kychnutie...\n");
   return;
   }


switch (mword[1]){
	case 'y': for (i=2; i<word_count;i++) {
	              strcpy(text,word[i]);
	              if ((target=get_user(text))!=NULL) break;
	              }
	          if (i==word_count) {
		       write_user(user,"~FGZ ruk ti vstupol mierny dym, bezhlavo sa zakrutil a rozplynul sa.\n");
		       return;
		       }
		   break;
	case 'm': target=user;
		  break;
	case 'p': target=NULL;
		  break;
	default : switch(rand()%5) {
	              case 0: write_user(user,"~FGOpakujes si runovu abecedu?\n");
		              break;
		      case 1: write_user(user,"~FGTaketo kuzlo nema hlavu ani patu...\n");
		              break;
		      case 2: write_user(user,"~FGNechces si najskor nastudovat kuzelnicku gramatiku?\n");
		              break;
		      case 3: write_user(user,"~FGSlova, iba slova... tie kuzla nestvoria!\n");
		              break;
		      case 4: write_user(user,"~FGKuzlo potrebuje svoj ciel a vediet, co ma robit...\n");
		              break;
		      }
		  return;
	}
/* v target je meno "obete" alebo NULL ak to nesuvisi s clovekom */

ptr=&mword[2];
if (user->vis) name=user->name; else name=invisname(user);
sprintf(text,"~FG%s mrmle nejake kuzelne slovicka...\n",name);
write_room_except(user->room,text,user);

switch (mword[0]) /* hlavny vypinac (switch) */
  {
   case 'b': /* vytvorenie */
             if (target==NULL) break;
   	     if (!strcmp(ptr,"p")) { /* pridavanie PP */
                 if (mana_check(user,20,expand_predmet("skorocel"),-1)) return;
   	         write_user(user,"~FGSilove kuzlo sa podarilo!\n");
                 target=leave_me_alone(user,target);
   	         write_user(target,"~FGCitis, ako ti magicky rastie sila...\n");
		 target->pp=target->pp+rand()%50;
		 return; 
		 }   	         
             else if (!strcmp(ptr,"g")) { /* vytvaranie predmetu */
		 if (mana_check(user,50,expand_predmet("pupava"),expand_predmet("pupava"))) return;
                 if (word_count<3 ||(!spravny_predmet(word[2])) || is_free_in_hands(target)==-1) {
                    write_user(user,"~FGKulzo zakruzilo dookola a bezcielne sa rozpynulo...\n");
                    return;
                    }
                 i=expand_predmet(word[2]);
                 if (predmet[i]->amount==0 || predmet[i]->spawnarea==3) {
                    write_user(user,"~FGKulzo sa zhulilo, zakruzilo dookola, spadlo na zem a rozbilo si hlavu..\n");
                    return;
                  }
 		 //remove_from_hands(target,24);
		 write_user(user,"~FGOooopsss... Kuzlo sa ti podarilo!\n");
                 target=leave_me_alone(user,target);
		 put_in_hands(target,expand_predmet(word[2]),default_dur(expand_predmet(word[2])));
		 write_user(target,"~FGPosobenim magie citis nejaku zmenu v pravej ruke!\n");
		 return;
                 }
	     else if (!strcmp(ptr,"sv")) { /* halucinacie (lsd) */
	         if (mana_check(user,15,expand_predmet("cernobyl"),expand_predmet("skorocel"))) return;
	         write_user(user,"~FGHalucinogenne kuzlo bolo uspesne!\n");
                 target=leave_me_alone(user,target);
	         write_user(target,"~FGCitiz ucinok magickeho halucinogenu...\n");
	         target->lsd+=90;
	         return;
	         }
	     else if (!strcmp(ptr,"pjs")) { /* exec a.k.a. force */
		 if (mana_check(user,80,expand_predmet("pupava"),expand_predmet("skorocel"))) return;
	         if (word_count<3) {
	             write_user(user,"~FGNeuplne kuzlo sa rozpadlo na kusky...\n"); 
	             return;
	             }
	         if (target->level>=WIZ) {
	         	write_user(user,"~FGNeznama magicka sila zabranila vykonaniu kuzla...\n");
	         	return;
	         	}
	         for (i=0;allowcom[i][0]!='*';i++)
	             if (!strcmp(allowcom[i],word[3])) break;
		 if ((user==target) || (allowcom[i][0]=='*')) {
		     write_user(user,"~FGKuzlo nemohlo vykonat poslanie a rozsypalo sa...\n");
		     return;
		     }
		 inpstr=remove_first(inpstr);
		 inpstr=remove_first(inpstr);
		 strcpy(text,strstr(inpstr,word[3]));
		 write_user(user,"~FGKuzlo magickej sily sa ti podarilo!\n");
                 target=leave_me_alone(user,target);
		 write_user(target,"~FGNejaka magicka sila ta nuti konat proti tvojej voli...\n");
		 word_count=wordfind(inpstr);
		 exec_com(target,inpstr);
		 return;
	         }
   	     break;
   case 'k': /* likvidacia */
   	     if (target!=NULL && !strcmp(ptr,"p")) { /* uberanie PP */
   	         if (mana_check(user,50,expand_predmet("cernobyl"),-1)) return;
	         write_user(user,"~FGZoslabovacie kuzlo sa podarilo!\n");
                 target=leave_me_alone(user,target);
   	         write_user(target,"~FGCitis, ako ti magicky ubudaju sily...\n");
		 target->pp=target->pp-rand()%50;
		 if (target->pp<0) target->pp=0;
		 return; 
		 }
	     else if (target!=NULL && !strcmp(ptr,"fg")) { /* hellfire */
		 if (mana_check(user,90,expand_predmet("cernobyl"),expand_predmet("cernobyl"))) return;
	         write_user(user,"~FGPodarilo sa ti zvolat mocnosti pekla!\n");
		 if (check_ignore_user(user,target) || !strcmp(target->room->name,"zalar")) {
                   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
		   target=user;
                  }
		 if (user->level<target->level) {
		 	write_user(user,"~FGVyssia moc zastavila ucinok kuzla!\n");
		 	return;
		 	}
                 target=leave_me_alone(user,target);
                 if (user->level<KIN && target->room!=NULL && (target->room->access==FIXED_PRIVATE || target->room->access==PRIVATE || target->room->access==GOD_PRIVATE)) {
                   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
  	           target=user;
                  }
		 if (target!=user) { 
                   sprintf(text,"~FGZ utrob zeme pocujes hlas %s..\n",sklonuj(user,2));
  	           write_user(target,text);
                  }
	         write_user(target,"~FGZem sa otvorila a vyslahli na teba pekelne plamene!\n");
		 sprintf(text,"~FR%s magickym plamenom", pohl(target,"spaleny","spalena"));
		 logout_user(target,text);
		 if (user!=target) inyjefuc=1;
		 return;
	         }
	     else if (target!=NULL && !strcmp(ptr,"fa")) { /* blesk */
	         if (mana_check(user,90,expand_predmet("cernobyl"),expand_predmet("pupava"))) return;
	         write_user(user,"~FGPodarilo sa ti zvolat mocnosti nebies!\n");
		 if (check_ignore_user(user,target) || !strcmp(target->room->name,"zalar")) {
                   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
		   target=user;
                  }
		 if (user->level<target->level) {
		 	write_user(user,"~FGVyssia moc zastavila ucinok kuzla!\n");
		 	return;
		 	}
                 target=leave_me_alone(user,target);
                 if (user->level<KIN && target->room!=NULL && (target->room->access==FIXED_PRIVATE || target->room->access==PRIVATE || target->room->access==GOD_PRIVATE)) {
                   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
  	           target=user;
                  }
		 if (target!=user) { 
  		   sprintf(text,"~FGV dialke si zacul%s hlas %s..\n",pohl(target,"","a"),sklonuj(user,2));
	           write_user(target,text);
                  }
	         write_user(target,"~FGBlesk z jasneho neba ta sprazil na oskvarky!\n");
		 sprintf(text,"~FR%s magickym bleskom",pohl(target,"spaleny","spalena"));
	         logout_user(target,text);
		 if (user!=target) inyjefuc=1;
		 return;
	         }
	     else if (target==NULL && !strcmp(ptr,"takgf")){ /* pad lietadla */
	         if (mana_check(user,70,expand_predmet("cernobyl"),expand_predmet("pupava"))) return;
	         if (!flyer.pozicia) {
	             write_user(user,"~FGKuzlo naslo letun na zemi a rozplynulo sa...\n");
	             return;
	             }
	         write_user(user,"~FGKuzlo naslo letun a strhlo ho k zemi!\n");
	         flyer.vykon=0;
	         flyer.vyska=1;
	         return;
	         }
   	     break;
   case 's': /* premena */
	     if (target!=NULL && !strcmp(ptr,"vnv")) { /* neviditelnost */
	        if (mana_check(user,50,expand_predmet("invisibylina"),-1)) return;
	        write_user(user,"~FGPodarilo sa ti kuzlo neviditelnosti!\n");
                target=leave_me_alone(user,target);
	        write_user(target,"~FGVplyvom magickej sily tvoje telo spriehladnelo... a uz ta nevidiet vobec!\n");
	        target->vis=0;
	        return;
	        }
	     else if (target!=NULL && !strcmp(ptr,"jv")) { /* zviditelnenie */
		if (mana_check(user,10,expand_predmet("invisibylina"),-1)) return;
	        write_user(user,"~FGPodarilo sa ti kuzlo zviditelnenia!\n");
                target=leave_me_alone(user,target);
	        write_user(target,"~FGNech ta videl ktokolvek, teraz ta vidia vsetci!\n");
	        target->vis=1;
	        return;
	        }
	     else if (target!=NULL && !strcmp(ptr,"lgw")) { /* zmena na zabu */
		if (mana_check(user,20,expand_predmet("zaba"),-1)) return;
//		if (mana_check(user,20,24,23)) return;
	        write_user(user,"~FGPodarilo sa ti zabacie kuzlo!\n");
                target=leave_me_alone(user,target);
	        write_user(target,"~FGKva, citis sa akosi nazelenalo, kva...!!\n");
                do_alt_funct(target,expand_predmet("zaba"));
	        return;
	        }
	     else if (target!=NULL && !strcmp(ptr,"lana")) { /* zmena na sliepku */
		if (mana_check(user,20,expand_predmet("sliepka"),-1)) return;
//		if (mana_check(user,20,expand_predmet("pupava"),expand_predmet("skorocel"))) return;
	        write_user(user,"~FGPodarilo sa ti slepacie kuzlo!\n");
                target=leave_me_alone(user,target);
	        write_user(target,"~FGPraa..papapa...kotkodaak! Menis sa na sliepku, kotkodaak...!!\n");
	        do_alt_funct(target,expand_predmet("sliepka"));
	        return;
	        }
	     else if (target!=NULL && !strcmp(ptr,"vnjv")) { /* prekrytie levelu */
		if (mana_check(user,30,expand_predmet("invisibylina"),expand_predmet("skorocel"))) return;
                if (target->level>=KIN) {
                  write_user(user,"~FGKuzlo narazilo na magicku silu a odrazilo sa!\n");
                  target=user;
                 }
	        write_user(user,"~FGKuzlo prekrytia levelu sa ti podarilo!\n");
                target=leave_me_alone(user,target);
	        write_user(target,"~FGKuzlo ti prekrylo level!\n");
		target->cloak=rand()%6;
	        return;
	        }
	     else if (target!=NULL && !strcmp(ptr,"vjv")) { /* odkrytie */
		if (mana_check(user,30,expand_predmet("invisibylina"),expand_predmet("skorocel"))) return;
                target=leave_me_alone(user,target);
		if (target->cloak){
		    write_user(user,"~FGKuzlo odkrytia levelu sa ti podarilo!\n");
	            write_user(target,"~FGKuzlo ti odkrylo level!\n");
	            target->cloak=0;
	            }
	        else write_user(user,"~FGKuzlo odkrytia levelu nenaslo level zakryty...\n");
	        return;
	        }
	        
   	     break;
   case 't': /* presun */
             if (target!=NULL && !strcmp(ptr,"gwa")) { /* presun kamsi odveci */
		 if (mana_check(user,50,expand_predmet("skorocel"),expand_predmet("skorocel"))) return;
                 if (word_count<3 ||((room=get_room(word[2],user))==NULL)) {
                   write_user(user,"~FGKulzo vzlietlo, zafucalo a rozpynulo sa...\n");
                   return;
                  }
                 if (room==target->room || room->access==FIXED_PRIVATE || room->access==PRIVATE || room->access==GOD_PRIVATE) {
                   write_user(user,"~FGKuzlo vzlietlo, no minulo sa ucinkom...\n");
		   return;
                  }
                 if (!strcmp(room->name,"zalar") || !strcmp(room->name,"arena")) {
                   write_user(user,"~FGKuzlo narazilo na odpor a odrazilo sa..\n");
                   target=user;
                  }
                 if ((target->room->group!=4 && room->group==4) || (target->room->group==4 && room->group!=4)) {
                   write_user(user,"~FGKuzlo sa po ceste unavilo a chciplo.\n");
                   return;
                  }
		 write_user(user,"~FGPremiestnovacie kuzlo sa podarilo!\n");
                 target=leave_me_alone(user,target);
		 move_user(target,room,2);
		 write_user(target,"~FGNejaka magicka sila ta preniesla vzduchom!\n");
		 prompt(target);
		 return;
                 }
	     else if (target==NULL && !strcmp(ptr,"twtm")) { /* privolanie lode */
		if (mana_check(user,70,expand_predmet("invisibylina"),expand_predmet("invisibylina"))) return;
	        if (user->room->group==2) { /* portalis */
			write_user(user,"~FGMagicka sila pritiahla plachetnicu k ostrovu!\n");
			while (ship_timer!=4) make_travel();
			}
		else { /* atlantis  */
			write_user(user,"~FGMagicka sila pritiahla plachetnicu do pristavu!\n");
			while (ship_timer!=2) make_travel();
			}
	        return;
	        }
   	     break;
   default: /* uplne haluzoidne kuzla ;-) */
   	     break;
   }
switch(rand()%5) {
 case 0: write_user(user,"~FGNepodarilo sa ti vykuzlit nic...\n");
 	 break;
 case 1: write_user(user,"~FGKuzlo nema nijaky zmysel...\n");
 	 break;
 case 2: write_user(user,"~FGPar run pokope kuzlo netvori...\n");
 	 break;
 case 3: write_user(user,"~FGSnaha sa ceni, no znalosti ti chybaju...\n");
 	 break;
 case 4: write_user(user,"~FGZ tohoto sa veru kuzlo nezrodi...\n");
 	 break;
 }
}

XA_OBJECT create_xannel()
{
XA_OBJECT xannel;
int i;
 if ((xannel=(XA_OBJECT)malloc(sizeof(struct xa_struct)))==NULL)
  {
   write_syslog("HUH: Memory allocation failure in create_xannel().\n",0);
   return NULL;
  }
 xannel->line=0;
 xannel->type=0;
 for(i=0;i<REVIEW_LINES;i++) xannel->buffer[i]=NULL;
 xannel->creator=NULL;
 xannel->topic[0]='\0';
 return xannel;
}

void destruct_xannel(user)
UR_OBJECT user;
{
 free(user->xannel);
}

void xsay(UR_OBJECT user,char *inpstr)
{
UR_OBJECT u;
int cnt=0;

if (word_count<2) {
  write_user(user,"Pouzi: .xsay <text>.\n");
  return;
 }
if (user->xannel==NULL) {
  write_user(user,"~FG\252C5Nie si v ziadnom kanali.\n");
  return;
 }
if (user->muzzled) {
  write_user(user,"~FG\252C5Si umlcany, nemozes rozpravat.\n");
  return;
 }
if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
  write_user(user,noswearing);  
  sprintf(text,"~OL~FR\07[CENZURA: %s sa %s kanalovat oplzle vyrazy!]~RS\n",user->name, pohl(user,"pokusil","pokusila"));
  writesys(WIZ,1,text,user);
  return;
 }
if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
  write_user(user,nocolors);  
  return;
 }
sprintf(text,"~FG\252C5%s \253Hh: ~RS~FW%s\n",user->name,inpstr);
u=user->nox;
while (u!=user) {
  cnt++;
  if (!u->ignall) write_user(u,text);
  u=u->nox;
 } 
if (cnt>1) {
  sprintf(text,"~FG\252C5%s %s: ~RS~FW%s\n",user->name,pohl(user,"\253P1","\253P2"),inpstr);
  record_xannel(user->xannel,text);
  sprintf(text,"~FG\252C5\253HH: ~RS~FW%s\n",inpstr);
 }
else if (cnt==1) sprintf(text,"~FG\252C5Ak sa chcete rozpravat dvaja, vyskusajte prikaz tell.\n");
else sprintf(text,"~FG\252C5Samomluva je 666-tym priznakom sialenstva.\n");
write_user(user,text);
}

void record_xannel(xannel,str)
XA_OBJECT xannel;
char *str;
{
 wash_bell(str);
 if (xannel->buffer[xannel->line]!=NULL) free ((void *) xannel->buffer[xannel->line]);
 xannel->buffer[xannel->line]=(char *) malloc ((strlen(str)+2)*sizeof(char));
 strcpy(xannel->buffer[xannel->line],str);
 xannel->line=(xannel->line+1)%REVIEW_LINES;
}

void add2xannel(UR_OBJECT user,UR_OBJECT u)
{
UR_OBJECT uu;
char hlp[100];
int cnt=0;

 if (u->xannel==NULL) return;
 user->xannel=u->xannel;
 user->nox=u->nox;
 u->nox=user; 

 sprintf(text,"\252C5~FW+ %s~FG\252C5 sem vliez%s.\n",user->name,pohl(user,"ol","la"));
 uu=user->nox;
 while (uu!=user) {
  cnt++;
  write_user(uu,text);
  uu=uu->nox;
 } 
 sprintf(text,"~FG\252C5Vliez%s si do ~FW",pohl(user,"ol","la"));
 if (user->xannel->type==2) strcat(text,"privatneho");
 else if (user->xannel->type==1) strcat(text,"zavreteho");
 else strcat(text,"verejneho");
 sprintf(hlp," ~FG\252C5kanala. Okrem teba %s tu ~FW%d~FG\252C5 %s.\n",skloncislo(cnt,"je","su","je"),cnt,skloncislo(cnt,"clovek","ludia","ludi"));
 strcat(text,hlp);
 write_user(user,text);
}

void xannel(UR_OBJECT user,char *inpstr)
{
UR_OBJECT u,uu;
int type=1; // 0 public, 1 closed, 2 private
int i,cnt=0,line=0,kolko,total=0,len=0;
char *pom;

if (!strcmp(word[1],"open") || !strcmp(word[1],"public") || !strcmp(word[1],"private")) {
  if (user->xannel!=NULL) {
    write_user(user,"~FG\252C5Uz si v kanali, ak chces otvorit novy, musis najprv vyliezt.\n");
    return;
   }
  if (!strcmp(word[1],"private")) {
    if (user->level>WAR) type=2;
   }
  if (user->level<WAR || !strcmp(word[1],"public")) {
    type=0;
   }
  if (word_count<3) {
    write_user(user,"~FG\252C5Musis zadat nejaku kanalovu temu.\n");
    return;
   }
  inpstr=remove_first(inpstr);
  if (strlen(inpstr)>58) {
    write_user(user,"~FG\252C5Prilis dlha tema pre kanal. Skus nieco kratsie.\n");
    return;
   }
  if ((user->xannel=create_xannel())==NULL) {
    write_user(user,"~FG\252C5Bohuzial poklop nejde vybrat, kanal sa neda otvorit.\n");
    return;
   }
  user->xannel->type=type;
  user->xannel->creator=user;
  user->nox=user;
  user->invite2xannel=NULL;
  strcpy(user->xannel->topic,inpstr);
  if (type==2) sprintf(text,"~FG\252C5Otvoril%s si si ~FWprivatny~FG\252C5 kanal na temu: ~OL~FW%s\n",pohl(user,"","a"),inpstr);
  else if (type==1) sprintf(text,"~FG\252C5Otvoril%s si ~FWuzavrety~FG\252C5 kanal na temu: ~OL~FW%s\n",pohl(user,"","a"),inpstr);
  else sprintf(text,"~FG\252C5Otvoril%s si ~FWverejny~FG\252C5 kanal na temu: ~OL~FW%s\n",pohl(user,"","a"),inpstr);
  write_user(user,text);
  return;
 }

if (!strcmp(word[1],"who")) {
  if (user->xannel==NULL) {
    write_user(user,"~FG\252C5Nie si v ziadnom kanali.\n");
    return;
   }
  if (user->xannel->type==2) sprintf(text,"~FG\252C5Momentalne sa nachadzas v ~FWprivatnom~FG\252C5 kanali.\n");
  else if (user->xannel->type==1) sprintf(text,"~FG\252C5Momentalne sa nachadzas v ~FWzavretom~FG\252C5 kanali.\n");
  else sprintf(text,"~FG\252C5Momentalne sa nachadzas vo ~FWverejnom~FG\252C5 kanali.\n");
  write_user(user,text);
  sprintf(text,"~FG\252C5Tema:~FW~OL %s\n",user->xannel->topic);
  write_user(user,text);
  sprintf(text,"~FG\252C5Okrem teba su tu:~RS~FW ");
  len=strlen(text)-2;
  u=user->nox;
  while (u!=user) {
    cnt++;
    len+=strlen(u->name)+2;
    if (len>80) {
      strcat(text,",\n");
     }
    else if (cnt>1) strcat(text,", ");
    strcat(text,u->name);
    u=u->nox;
   } 
  strcat(text,".\n");
  if (!cnt) {
    sprintf(text,"~FG\252C5Si tu uplne sam.\n");
    write_user(user,text);
   }
  else {
    write_user(user,text);
    if (cnt>9) {
      sprintf(text,"~FG\252C5Dokopy je tu ~FW%d~FG\252C5 ludi.\n",cnt);
      write_user(user,text);
     }
   }
  return;
 }
 
if (!strcmp(word[1],"all")) {
  total=0;
  for (u=user_first;u!=NULL;u=u->next)
   if (u->xannel!=NULL && u->xannel->creator==u) {
    total++;
    if (total==1) write_user(user,title("~FG\252C5Zoznam kanalov","~FG"));
    if (u->xannel->type==2) sprintf(text,"~FG\252C5Privatny:~FW~OL %s \n ",u->xannel->topic);
    else if (u->xannel->type==1) sprintf(text,"~FG\252C5Uzavrety:~FW~OL %s\n ",u->xannel->topic);
    else sprintf(text,"~FG\252C5Verejny:~FW~OL %s\n ",u->xannel->topic);
    len=1;
    cnt=0;
    uu=u;
    while (cnt==0 || uu!=u) {
      cnt++;
      len+=strlen(uu->name)+2;
      if (len>80) {
        strcat(text,",\n");
       }
      else if (cnt>1) strcat(text,", ");
      strcat(text,uu->name);
      uu=uu->nox;
     } 
    strcat(text,".\n");
    write_user(user,text);
   }
  if (total) write_user(user,"~FG.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-\n");
  else write_user(user,"~FG\252C5Nie su otvorene ziadne kanaly.\n");
  return;
 }

if (!strcmp(word[1],"rev") || !strcmp(word[1],"review")) {
  if (user->xannel==NULL) {
    write_user(user,"~FG\252C5Nie si v ziadnom kanali.\n");
    return;
   }
  if (word_count==2) {
    kolko=atoi(word[2]);
    if (kolko<1 || REVIEW_LINES<kolko) kolko=REVIEW_LINES;
   }
  else kolko=REVIEW_LINES;
  for(i=0;i<REVIEW_LINES;++i)
   if (user->xannel->buffer[(user->xannel->line+i)%REVIEW_LINES]!=NULL) total++;
  cnt=0;
  strtolower(word[2]);
  strtolower(word[3]);
  for(i=0;i<REVIEW_LINES;++i) {
    line=(user->xannel->line+i)%REVIEW_LINES;
    if (user->xannel->buffer[line]!=NULL) {
      cnt++;
      if (cnt==1) write_user(user,title("~FG\252C5Posledne hlasky v kanali","~FG"));
      if (total-kolko<cnt) {
        pom=(char *) malloc ((strlen(user->xannel->buffer[line])*sizeof(char))+1);
        strcpy(pom, user->xannel->buffer[line]);
        strtolower(pom);
        if (word_count>1 && strlen(word[2])>1 && !strstr(pom,word[2])) continue;
        if (word_count==3 && !strstr(pom,word[3])) continue;
        write_user(user,user->xannel->buffer[line]);
        free(pom); 
       }
     }
   }
  if (!cnt) write_user(user,"~FG\252C5V kanali sa este nehovorilo.\n");
  else write_user(user,"~FG.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-\n");
  return;
 }

if (!strcmp(word[1],"leave") || !strcmp(word[1],"stop")
|| !strcmp(word[1],"quit") || !strcmp(word[1],"close")) {
  if (user->xannel==NULL) {
    write_user(user,"~FG\252C5Nie si v ziadnom kanali.\n");
    return;
   }
  sprintf(text,"\252C5~FW- %s~FG\252C5 odtialto vyliez%s.\n",user->name,pohl(user,"ol","la"));
  cnt=0;
  u=user;
  while (u->nox!=user) {
    cnt++;
    u=u->nox;
    write_user(u,text);
   }    
  if (cnt==0) {
    user->invite2xannel=NULL;
    user->nox=NULL;
    destruct_xannel(user);
    user->xannel=NULL;
    sprintf(text,"~FG\252C5Vyliez%s si z kanala a zavrel si za sebou poklop.\n",pohl(user,"ol","la"));
    write_user(user,text);
    return;
   } 
  u->nox=user->nox;
  if (user->xannel->type==2 && user->xannel->creator==user) {
    user->xannel->type=1;
    user->xannel->creator=u;
   }
  if (user->xannel->creator==user) user->xannel->creator=u;
  user->invite2xannel=NULL;
  user->nox=NULL;
  user->xannel=NULL;
  sprintf(text,"~FG\252C5%s si z kanala.\n",pohl(user,"Odisiel","Odisla"));
  write_user(user,text);
  return;
 }

if (!strcmp(word[1],"kick")) {
  if (user->xannel==NULL) {
    write_user(user,"~FG\252C5Nie si v ziadnom kanali.\n");
    return;
   }
  if (user->xannel->type!=2) {
    write_user(user,"~FG\252C5Tento kanal nie je privatny, neda sa z neho vykopavat.\n");
    return;
   }
  if (user->xannel->creator!=user) {
    write_user(user,"~FG\252C5Vykopavat moze len ten kto otvoril tento privatny kanal.\n");
    return;
   }
  if (!(uu=get_user(word[2]))) {
    write_user(user,notloggedon);
    return;
   }
  if (user==uu) {
    write_user(user,"~FG\252C5Skus dat radsej '.xannel leave'.\n");
    return;
   }
  if (user->xannel!=uu->xannel) {
    sprintf(text,"\252C5~FW%s~FG\252C5 nie je v tomto kanali.\n",uu->name);
    write_user(user,text);
    return;
   }
  sprintf(text,"\252C5~FW- %s~FG\252C5 %s.\n",uu->name,pohl(uu,"bol odtialto vystrnadeny","bola odtialto vystrnadena"));
  cnt=0;
  u=uu;
  while (u->nox!=uu) {
    cnt++;
    u=u->nox;
    write_user(u,text);
   }    
  u->nox=uu->nox;
  uu->invite2xannel=NULL;
  uu->nox=NULL;
  uu->xannel=NULL;
  sprintf(text,"~FW\252C5%s~FG\252C5 ta vystrnadil%s z kanala.\n",user->name,pohl(user,"","a"));
  write_user(uu,text);  return;
 }

if (word_count==2) {
  if (!(u=get_user(word[1]))) {
    write_user(user,notloggedon);
    return;
   }
  if (user==u) {
    write_user(user,"~FG\252C5You're never alone with schizophrenia..\n");
    return;
   }
  if (user->xannel!=NULL) {
    if (user->xannel->type==2 && user->xannel->creator!=user) {
      sprintf(text,"~FG\252C5Tento kanal otvoril ~FW%s~FG\252C5, len %s moze pozyvat.\n",user->xannel->creator->name,pohl(user->xannel->creator,"on","ona"));
      write_user(user,text);
      return;
     }
    if (user->xannel->type==2) {
      cnt=1;
      uu=user;
      while (uu->nox!=user) {
        cnt++;
        uu=uu->nox;
       }    
      if (cnt>5+3*(user->level-SAG)) {
        sprintf(text,"~FG\252C5Tento kanal uz je plny, viac ludi sa sem nezmesti.\n");
        write_user(user,text);
        return;
       }
     }
    if (u->xannel!=NULL) {
      sprintf(text,"\252C5~FW%s~FG\252C5 uz je zalezen%s v nejakom kanali.\n",u->name,pohl(u,"y","a"));
      write_user(user,text);
      return;
     }
    if (user->invite2xannel==u) {
      sprintf(text,"\252C5~FW%s~FG\252C5 ta pozval%s do kanala.\n",user->name,pohl(user,"","a"));
      write_user(u,text);
      add2xannel(u,user);
      return;
     }
    if (u->invite2xannel==user) {
      sprintf(text,"~FG\252C5Uz si %s pozyval%s do kanala.\n",pohl(u,"ho","ju"),pohl(user,"","a"));
      write_user(user,text);
      return;
     }
    else
     { 
      if (u->ignxannel) {
        sprintf(text,"\252C5~FW%s~FG\252C5 nema zaujem lozit do kanalov.\n",u->name);
        write_user(user,text);
        return;
       }
      if (u->level<SOL) {
        sprintf(text,"\252C5~FW%s~FG\252C5 este nemoze lozit po kanaloch.\n",u->name);
        write_user(user,text);
        return;
       }
      sprintf(text,"~FG\252C5Pytas sa ~FW%s~FG\252C5, ci nechce ist za tebou do kanala.\n",sklonuj(u,2));
      write_user(user,text);
      sprintf(text,"\252C5~FW%s~FG\252C5 sa ta pyta ci nejdes za %s do kanala. (.xannel)\n",user->name,pohl(user,"nim","nou"));
      write_user(u,text);
      u->invite2xannel=user;
      return;
     }
   }
  else {
    if (u->xannel==NULL) {
      sprintf(text,"\252C5~FW%s~FG\252C5 nie je v ziadnom kanali.\n",u->name);
      write_user(user,text);
      return;
     }
    if (u->xannel->type==0 || user->invite2xannel==u) {
      add2xannel(user,u);
      return;
     }
    if (u->xannel->type==2) {
      cnt=1;
      uu=u;
      while (uu->nox!=u) {
        cnt++;
        uu=uu->nox;
       }    
      if (cnt>5+3*(u->level-SAG)) {
        sprintf(text,"\252C5~FW%s~FG\252C5 kanal uz je plny, viac ludi sa tam nezmesti.\n",sklonuj(u->xannel->creator,2));
        write_user(user,text);
        return;
       }
     }
    if (u->xannel->type==2 && u!=u->xannel->creator) {
      sprintf(text,"\252C5~FW%s~FG\252C5 je v privatnom kanali, ktory otvoril%s ~FW%s~FG\252C5, opytaj sa %s.\n",u->name,pohl(u->xannel->creator,"","a"),u->xannel->creator->name,pohl(u->xannel->creator,"jeho","jej"));
      write_user(user,text);
      return;
     }
    if (u->invite2xannel==user) {
      sprintf(text,"~FG\252C5Uz si sa %s raz pytal%s.\n",pohl(u,"ho","jej"),pohl(user,"","a"));
      write_user(user,text);
      return;
     }
    else { 
      sprintf(text,"~FG\252C5Pytas sa ~FW%s~FG\252C5 ci mozes ist za %s do kanala.\n",sklonuj(u,2),pohl(u,"nou","nim"));
      write_user(user,text);
      sprintf(text,"\252C5~FW%s~FG\252C5 sa ta pyta ci moze ist za tebou do kanala.\n",user->name);
      write_user(u,text);
      u->invite2xannel=user;
      return;
     }
   }
 }

if (user->level>SOL) 
write_user(user,"xannel open <tema>   - otvoris standardny kanal, pozyvat mozu vsetci v kanali   \n");
write_user(user,"xannel public <tema> - otvoris kanal do ktoreho moze vojst kazdy aj bez pozvania\n");
if (user->level>WAR) 
write_user(user,"xannel private <tema>- otvoris kanal kam mozes pozyvat iba ty\n");
write_user(user,"xannel leave        - odides z kanalu (zavries kanal)\n");
write_user(user,"xannel review       - zobrazi posledne hlasky v kanali\n");
write_user(user,"xannel who          - zobrazi ludi v kanali v ktorom prave si\n");
write_user(user,"xannel all          - zobrazi vsetky otvorene kanaly a ludi v nich\n");
write_user(user,"xannel kick <user>  - ten kto otvoril privatny kanal, vykopne z neho uzivatela  \n");
write_user(user,"xannel <user>       - ak si v kanali, pozves uzivatela do kanala v ktorom si,   \n");
write_user(user,"xannel <user>       - ak nie si, pokusis sa vojst do kanala v ktorom je user    \n\n");
write_user(user,"Pozn: ignorovanie vyzyvania do kanalov je mozne cez '.ign xannel',              \n");
write_user(user,"      farbu hlasok v kanali si mozes prestavit pomocou '.set col xannel'        \n");
write_user(user,"xsay <text>         - povies <text> do kanala\n");
}
