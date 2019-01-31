/*****************************************************************************
  	           ---=> ATLANTIS 4.xx (pow!) HEADER <=---
	             ged reedy four a phun-tastic staff!
      	         (a verte tomu, ze je NAOZAJ phun-toastycs!)
******************************************************************************/ 
   /* aj procko by ziral ... */
                                 /* WHOARN(IE)YNG! BLBOSTI INSIDE! KIPE UTE! */

/* Takze najskor budu INCLUDY - preco ich nedat sem, ked mozem? */

#include <stdio.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <fcntl.h>
#include <time.h>

#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <dirent.h>
#include <stddef.h>

#include <stdlib.h>
#include <ctype.h>

#include <sys/vfs.h> /* koli ropen */
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h> /* koli poste ... */
#include <sys/wait.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <sys/poll.h>
#include <setjmp.h>
#include <stdarg.h>
#include <libxml/xmlwriter.h>
#include <locale.h>

#ifdef HPUX_SB
  #define FD_CAST (int *)
#else
  #define FD_CAST (fd_set *)
#endif

#ifdef GLIBC207
  #define SIGNAL(x,y) sysv_signal(x,y)
#else
  #define SIGNAL(x,y) signal(x,y)
#endif


int wrtype,pollerror;

#define WR_IOPHR 1     /* i/o phrase ignorovanie */
#define WR_ZVERY 2
#define WR_NEWBIE 3
#define WR_AVOID_PREDMETS 4
#define WR_NOEDITOR 5
#define WR_NOCOL 6
#define WR_WITHCOL 7


#define RESCNUM 500
char rescip[RESCNUM][17];
char rescho[RESCNUM][52];
int  rescn;
int resc_resolved;
int resc_cached;

/* Teraz dake deklaracie, ktore boli tam, kde byt NEMALI! */

float counter(int zobraz);

int pocet_vtipov;
int newusers;

int doplnovanie_predmetov,ryxlost_doplnovania,predmety_dynamic;
int pocet_predmetov,predmety_time,predmety_position,pocet_konverzii;
int zistuj_identitu;
int inyjefuc;

int max_users_was;
int all_logins;
int sayfloodnum;
int passwd_simplex; /* Ahem ... Brezina, Ivanicova!!!! - ale nestras! */
int hb_can;
int hb_sec;

int daily_num;
int every_min_num,burkamin,tunelik,brutalis_gate,portalis_gate;
size_t tajmauts,tajmautz;
int writeerror,max_timeouts,max_timeoutz;

#define VERSION "3.3.3"
#define WORK "modifikacie: Spartakus, Rider, Buko, Hruza, Viper"
#define MOZENADAVAT GOD  /* Ze ktory level mozenadavat */
#define MOZEFARBICKOVAT KIN  /* Ze ktory level pouzivat farbicky */
#define TIMEOUT 3

/* Zabezpecenie otvarania suborov (ropen) */

#define FILESYSTEM "/"  /* cesticka k disk-device */
#define LOW_DISK_LIMIT 1024     /* minimum volneho v KB   */

#define POCET_KUZIEL 16
/* Definicie Power-Pointov */

#define SPELLK	   20  /* konstanta pre vypocet many */ 
#define MAXPP      200 /* maximalny pocet pp */
#define INCPP      1   /* kolko pp prida za 1 hearbeat */
#define NODYNAMIC  0   /* nie je dynamicky uberane */
 
#define SHOUTDEC   35  /* kolko uberie shout */ 
#define SHOUTCAN   105 /* kedy uz moze shoutovat */ 
#define SHOUTDYN   85  /* dynamicka konstanta v percentach */   
  		        /* Dalej uz tak isto - PRIKAZDEC, PRIKAZCAN, PRIKAZDYN */ 
#define SEMOTEDEC  35 
#define SEMOTECAN  105
#define SEMOTEDYN  85
 
#define SECHODEC   55 
#define SECHOCAN   130
#define SECHODYN   120 
 
#define BANNERDEC  70 
#define BANNERCAN  140
  
#define SBANNERDEC 200 
#define SBANNERCAN 170
#define SBANNERDYN 200
 
#define HUGDEC     10 
#define HUGCAN     40 
 
#define WAKEDEC    10 
#define WAKECAN    40   
 
#define FIGHTDEC   80 
#define FIGHTCAN   160 
#define FIGHTDECARENA 60
#define FIGHTCANARENA 100 

#define KICKDEC   100 
#define KICKCAN   120 

#define KISSDEC    10
#define KISSCAN    60

#define KIDDEC     50
#define KIDCAN     80

#define TOPICDEC     40
#define TOPICCAN     100
 
#define THROWDEC     20
#define THROWCAN     50

#define PRIVPUBDEC   40
#define PRIVPUBCAN   100

/* Definicie akcii - pokusne */ 
/* Poznamka: Aj tak je teraz osobitny casovac pre kazdy, predmet, pche! :>> */
/* No jo, a potom nefunguje .caction, pajada... :( */
/* V) Akcije su na meno, na dorucitela a tu v zdojaku su na howno!, lebo potom
   1 user nemoze byt naraz vo viac akcijach (napr. naraz jail a muzzle). ;))) */

/* Sortovanie v tabulkach */
#define MAX_ENTRIES 1000 /* No, viac ako 1000 ix neviem ci bude */

/* First of all, nejake definicie suborov ... */

#define DICT "misc/dict"                    /* slovnik lamerskyx hesiel */
#define DEFAULT_TOPIC_FILE "topic"          /* topikovy subor */
#define LAST_TOPIC_FILE "last_topic"        /* posledne topiky */
#define ROZLOZENIE_P "rozlozenie"           /* rozlozenie predmetov */
#define ZOZNAM_USEROV "misc/users.new"      /* zoznam userov */
#define MULTI_REQUEST "misc/multirequest"   /* pokusy requestov na uz pouzite adresy */
#define MENINY_FILE "datafiles/meniny.dat"  /* kalendar */
#define LABYRINT_WINNERS "labyrint.win"     /* vitazi labu */
#define LODICKY_SAVE_DIR "lodicky-save"     /* adresar pre save v lodickax */
#define HANGMAN_WORDS "hangman"             /* slova do obesenca */
#define WEB_BOARD_DAT "board.log"           /* kto pisal na board a kde */
#define PISKFILE "piskvorky.top"            /* piskvorove skore */
#define NO_QUOTA "misc/noquota"             /* kto nema kvotu na postu */
#define AKLIENT_LOG_USERS "log/akl_user.log"/* log userov z klienta */
#define AKLIENT_LOG_VER "log/akl_ver.log"   /* log pristupov z verzii klienta */
#define LEVLOG "log/level.log"              /* log promotes/demotes */
#define HINT_FILE "misc/hints"              /* zoznam hintov */
#define COMMLOG "log/commlog2.log"          /* command debug log */
#define COUNTERFILE "misc/counter"          /* pocitadlo pristupov */
#define WHO_FOR_WEB "../public_html/who.raw"/* who pre web - surovy fajl */
#define TLT_LOG "log/tlt.log"               /* TLT +, - log */
#define NICKBLOCK  "misc/nickblock"         /* vyblokovane nicky */                                               
#define RESC_CACHE "misc/resc_cache"        /* cache pre resolvera */
#define NEWUSER_SITEBAN "newban"            /* misc/newban */
#define REMOTE_FILE "remote"                /* konfigurak connecticutu :-) */
/* #define REMOTEDIR "remote" */                 /* kde budu uzivatelske remote hesla */
#define NOTESDIR  "notes"                   /* poznamkove bloky uzivatelov */
#define GOT_AUTH  "misc/users.auth"         /* real identity z forknuteho tisu */
/* #define IRC_DIR "irc" */                      /* userovske IRC veci */
#define IRC_SERVERS "misc/ircserv"          /* povolene IRC servre */
#define COM_LEVEL "misc/comlevel"           /* prehodene levely k prikazom */
#define REVERSIFILE "reversi.top"           /* tabulka reversiarov */
#define DAMAFILE "dama.top"                 /* tabulka damy! */
#define DAMAFILES "dama-save"               /* save adresar pre dddamu :> */
#define REPOSITORY "repository"             /* adresar pre repository */
#define MINYTOP "miny.top"                  /* tabulka min! */
#define NOTICE_DIGEST "misc/notice_digest"  /* digest z noticof */
#define REVTELL_TMP "mailspool/rt_tmp"      /* revtell tempfile */
#define CLOVECEFILES "xclosave"             /* clovece */
#define BACKUP_DIR "backup_db"              /* adresar na zalohy db */
#define TALKER_TELNET_HOST "vps.vudiq.sk"   /* host, kde bezi talker */
#define TALKER_EMAIL_HOST "vps.vudiq.sk"    /* e-mailovy host */
#define WIZZES_EMAIL_ALIAS "wizzes"         /* alias, na ktory sa posieju e-maily pre spravcov */
#define TALKER_EMAIL_ALIAS "talker"         /* alias, na ktory sa posielaju vseobecne e-maily */
#define TALKER_WEB_SITE "https://vps.vudiq.sk/talker" /* site, kde bezi web talkera */
#define TALKER_CITY "Bratislava"            /* mesto, kde talker bezi */
#define TALKER_COUNTRY "Slovensko"          /* krajina, kde talker bezi */

/* Status line */
enum statline {NONEMODE,UNKNOWN,LINEMODE,CHARMODE};

/* REM0TE */
#define MAX_CONNECTIONS 5 /* ZMENIT NA 5 !!!!!!! */
#define MAX_POLL 2000

#define AUTOSAVE 300   /* autosave (krat dva) -> 600 = 20 minut */

#define HISTORY_LINES 10
#define HISTORY_LEN 20

#define RULES_HTML "../public_html/rules/rules.htm"
#define FAQ_HTML   "../public_html/atlantis/faq.htm"
#define NEWS_HTML  "../public_html/talnews.htm"

#define DB_JAIL 1
#define DB_DEMOTE 2
#define DB_BAN 3

#define COUNT 111                           /* fortune cookies */

#define MAIL_QUOTA 50000 /* odkedy bude upozornovat na preplnenu sxranku */
#define MAIL_LIMIT 55000 /* odkedy uz nebude moct dostavat postu!!! */

/* Toto je na HNUSNY resolver. */
#define NUMBER_OF_LOOPS 75 /* kolko desiatok microsekund na resolvnutie */
#define ADRESIZE 512       /* maximalna dlzka hostname         */

#define PROMOTE_PASSWD "SSChoredaj*PB%f,T4BnP{pi}%^(jPjf&"
#define X_PASSWD "SSCmarine*N(k0>x,tiGnV%J;l<pVz^&"

/* Second of all, some macros and other stuff */

#define MAX_POCET_LOGINOV 10 /* kolko ludi sa moze z 1 adresy naraz prihlasovat
			       (rata sa len login stage), prevencia proti floodu */
			     /* Wilderovi sa to tiez lubilo, ako to mame spravene,
			        hehe :-) */
			        
#define PROMPT_LEN 120 /*dlzka promptu!*/

/* REM0TE */

#define REMOTE_NAME_LEN 256
#define REMOTE_DESC_LEN 256

#define ANTI_IDLE_TIME_SEC 0

int size_fds;

struct remote_struct {
			char name[REMOTE_NAME_LEN];
                        char desc[REMOTE_DESC_LEN];
			char shortcut;
			int port;
			struct hostent *he;
		        struct remote_struct *next;
		      };
		     
typedef struct remote_struct *RN_OBJECT;
RN_OBJECT remote_start,remote_last,remote_active;
int remote_slot_active;
sigjmp_buf save_state;

extern int h_errno;
			       
/* #define MACRODIR "macros" */
#define PRIKAZDIR "prikazy"
#define MACROFILE "macrofile" 
#define FORTUNE_FILE "misc/fortune" 
#define POMOCFILES "pomocfiles" 
#define USERLIST "misc/users.old" 
#define NOEXITS "noexits" 
#define DATAFILES "datafiles" 
/* #define MAILFILES "userfiles" 
#define HELPFILES "helpfiles" 
#define PICTURES "pictures" */
#define NOTICEBOARD "misc/noticeboard" 
#define MAILSPOOL "mailspool" 
#define CONFIGFILE "config.new" 
#define MAPFILE "mapfile" 
#define SITEBAN "siteban" 
#define USERBAN "userban" /* uloupit/neloupit sezrat/nesezrat */
#define SYSLOG "log/syslog" /* zabijet/nezabijet sezrat/nesezrat */
#define WIZLOG "log/wizlog" 
#define GAMES_LOG "log/games"

/* Definicie pre labyrint. Dufajuc ze sa v tom nestratite ! :> */
#define LAB_ROOMS 65            /* Maximalny pocet miestnosti */
#define LAB_DESC  900            /* Maximalna velkost popisu miestnosti */
#define MAX_LINE  100            /* Maximalna dlzka jedneho riadku z fajlu */
#define LAB_FILE  "datafiles/labdata.dat"  /* Nazov datoveho suboru */
        /* do laby!!! :>> */  /* do laby-netlaby */ /* do nextraby */

/* Pokracujeme v definiciax. */
 
#define OUT_BUFF_SIZE 3000
#define MAX_WORDS 100
#define MAX_IRC_WORDS 15
#define IRC_WORD_LEN 600
#define WORD_LEN 120  
#define ARR_SIZE 800
#define MAX_LINES 100 
#define NUM_COLS 38

#define MAX_COPIES 10 /* malo/vela?! */ /* coby kamenom dohodil.. */

/* sign on to room: */ 
#define ARENA_ROOM "arena"   /* Rooma reprezentujuca arenu */

#define DESC_CHAR_LEN 400 /* Kolko ZNAKOV (aj tyx coolblbosti) moze mat desc */
#define COOLBUF_LEN 1500   
#define MAX_LAME 1450     /* Cooltalk */
#define MAX_CISLO 8
#define COOLTALK_LEN 80

#define USER_NAME_LEN 12
#define USER_DESC_LEN 40 /* C00ltalk */

#define AFK_MESG_LEN 120
#define PHRASE_LEN 45
#define MAXSLOHY 4    /* kolko sluoh mozno naraz vypotit .poet-om (flood)  */


#define EMAIL_LEN 60 /* max. dlzka emailu */
#define PASS_LEN 10 /* only the 1st 8 chars will be used by crypt() though */
#define BUFSIZE 900
#define ROOM_NAME_LEN 20
#define ROOM_LABEL_LEN 5
#define TOPIC_LEN 70  /* C00ltalk */
#define MAX_LINKS 8
#define REVB_LINES 10
#define REVIEW_LINES 30 /* bolo 22 */
#define REVTELL_LINES 50 /* bolo 30 */
#define REVIRC_LINES 50
#define LASTLOG_LINES 50
#define REVIEW_LEN 300 /* bolo 1000 - potom 500 ci kolko */
#define HOMEPAGE_LEN 60 /* Kapitan: Jsi v pasti a my te ted oloupime! */
                        /* Ivan: Mne?!?!? */
                        /* Kapitan: Kua do paze koho ineho?!? */
                        /* Ivan: Tak cooo, volupte mne vole vozralej! (hodi na zem brasnu) */


#define MPVM 25 /* Maximum predmetov v miestnosti */
#define HANDS 4 /* pocet chapadiel usera */
#define BODY HANDS+3 /* HANDS + cepicka + oblecenie + boty */

/* Arena stuff. Zurive bitky mozu zacat !!!! */
#define ARENA_FILE "arena.top"
#define ARENA_TEMPFILE "arena.tempfile"

#define HANGTOP "hangman.top" /* Aj tak bude Mia na 1. mieste :* */
#define FLYTOP "flyer.top" /* najuspesnejsi piloti ... */

/* Weather report stuff. Lepsie ako CNN !!! Aktualne spravodajstvo, priamo
   na Atlantise priamo zo Slovenskeho Hydrometeorologickeho Ustavu !! */
   
#define WEATHER_TODAY_FILE    "pocasie/weather.today"
#define WEATHER_TOMORROW_FILE "pocasie/weather.tomorrow"
#define WEATHER_STRED_FILE    "pocasie/weather.stred"
#define WEATHER_VYCHOD_FILE   "pocasie/weather.vychod"
#define WEATHER_ZAPAD_FILE    "pocasie/weather.zapad"
#define WEATHER_BLAVA_FILE    "pocasie/weather.blava"
#define WEATHER_TATRY_FILE    "pocasie/weather.tatry"

/* notify .......... Ak tam bude zasa nejaky bug, Buk(g)o je dead ! :> */
/* ludia sa stazovali ze 30 je malo .... tak zvysit? No dobre tak 35 :> */
/* TAKZE Buko je dead, lebo dalsi bug sa nasiel :>>> Gde mam tu devinu kua!*/
/* Pod wankusom... alebo este skor pod psiou budou, vedla toho kluca :> */
/* Juuuj clowece to je naaapad, zbrane skryvat pod psou budou :> Ale to este
   nepoznas nasho psa .. to keby si tu zbran ryxlo potreboval tak by si sa
   k nej nedostal lebo Cezarko by Ta najprv poriadne pooblizoval, oslintal,
   oskakal, prinajhorsom este aj ocikal, takze za ten cas by vyrabovali
   cely barak aj s kompom na ktorom su ... SAKRA ZDROJAKY ATLANTISU!!!!!!!!!!
   Okay vsetky zbrane vratane granatov presuvam pod vankus ... M16ka, M203ka
   aj par M249 nabite, of course, pre pripad nudze ...
   Kurnik ale ten kluc nieje celkom pod budou, ale je v areali strazenom 
   psom :>>> no a aby som nezabudol, F16ky su v garazi ... to pre pripad
   leteckeho utoku z Kosic (?! $&!@(# !!!!!!) sakra nikdy som nebol militantny!*/
/* Vies co? To bude uplne super, ked bueds musiet narukvat a budes osisany na
   kratucke vlasky (ako ty zvyknes) a budes musiet pochodovat, cistit latriny,
   drhnut si boty, skakat na poplachy, zakopavat tanky a tak... Potom ta ta
   militantna nalada celkom iste prejde ;-) */

struct 
 {
 int start;
 int end;
 int sever[LAB_ROOMS],juh[LAB_ROOMS],vychod[LAB_ROOMS],zapad[LAB_ROOMS],hore[LAB_ROOMS],dole[LAB_ROOMS];
 char popiska[LAB_ROOMS][LAB_DESC];
 int dostupny;
 } lab_room;

struct pager_struct {
	int filepos;
	int messnum;
	struct pager_struct *next;
	};                                   
typedef struct pager_struct *PAGER; 
   
  /*  (S) ZMENA!!!!!!!!!! DYNAMICKY NOTIFY LIST -> MOZNOST ZARADIT LUBOVOLNY
      POCET JUZROV!!! 
      cele som to teda prerobil, hlavna funkcia je teraz ovela prehladnejsia
      a tiez cele to je efektivnejsie, krajsie, genialnejsie, proste totalne
      dokonale :->>>>>>>>>      
      Zabudol si napisat, ze je to "transcendentalne!!!"
      */
      
struct notify_struct {
	char meno[13];
	struct notify_struct *next;         
	};                                    
typedef struct notify_struct *NOTIFY;

struct dama_struct {
	struct user_struct *hrac[2];
        int plocha[8][8],forcejump,remiza[2],monochrom[2];
	int natahu,lastmove[2],disptype[2]; /* posledny tah [x,y],typ zobrazenia */
        int tah;
        };
typedef struct dama_struct* DAMA_OBJECT;
   
/* ... aha, to vsetko sa tykalo toho notify?!?! :> WOW! */

/* DNL (Date Number Length) will have to become 12 on Sun Sep 9 02:46:40 2001
   when all the unix timers will flip to 1000000000 :) */
/* No a my si natolko verime ze Atlantida bude existovat aj po roku 2001, ze
   toto cislo ihned ako to bude mozne, zmenime na 12.  */
/* Ovsem, ak nam ho ten Coronerov 'kamos' neoskalpuje, ako slubil :))) */
/* ... uz nas skoro oskalpoval Wilder so Somebodym, suuuux! */
/* !!! A UZ SA XYSTAJU AJ NA NETLAB !!! Musime zavolat TPSovho kocura Mexana :>*/ 
/* ..a uz ho dostali. Cest jeho pamiatke. Co myslis, nastahuju sa nam aj na
   Platona? Mozno mame remote exploit v drivery sietovice... juuuuj!! */

/* #define DNL 11 */
/* fok DNL, zjadne DNL! MYSQL RULEZ!! */

#define PUBLIC 0
#define PRIVATE 1
#define FIXED 2
#define FIXED_PUBLIC 2
#define FIXED_PRIVATE 3
#define GOD_PRIVATE 5

#define NEW 0  /* ZMENA - Nove levely, nove skratky !!! */
#define CIT 1  /* USER */
#define SOL 2  /* BC */       /* <--- aj to by (zatial) stacilo!!! :>   */
#define WAR 3  /* ING */      /* <--- toto by sme fsetci xceli bytj! :> */
#define SAG 4
#define PRI 5
#define WIZ 6
#define KIN 7  /* ARCH */     /* <--- Hnuuusny Archangel! Este aj level ma! */
                              /* Clovece Archangel ... to mi ani nespominaj!*/
#define GOD 8
#define POS 9 /* Poseidon OberGod! ;) */ /* Vidis, nazvem ho BohBot! */
          /*     ^^^^^^^^^^^^^^^^^^^^ to tu napisal BUKO! */

#define DB_ARENA     1  /* Nemenit! bo sa presunu body a mena v tabulach (V) */
#define DB_PISKVORKY 2  
#define DB_HANGMAN   3
#define DB_QUEST     4
#define DB_DOOM      5
#define DB_LETUN     6
#define DB_REVERSI   7
#define DB_DAMA      8
#define DB_MINY      9
#define DB_BRUTALIS 10

/* Kurnajs, sak tie klony NIKTO nepouziva !!! */
/* S> hmmm . o O ( co keby sme vyhodili aj tie ... :-))) */
/* R> Sa neopovaz, ja to nahodof casto (raz za mesiac) pouzivam!!!! */
/* S> No fidis, tak ked ix budes xciet pouzit, tak zoberies zdrojak a
      doprogramujes si to tam, je to taky problem?! :>>> */
/* R> Nijaky. Nakoniec to ide aj bez toho, staci mat paky na spravnych 
      mjestach, ze ano ;-) */

#define USER_TYPE 0
#define CLONE_TYPE 1
#define CLONE_HEAR_NOTHING 0
#define CLONE_HEAR_SWEARS 1
#define CLONE_HEAR_ALL 2

/* ourSQL .. the future is our world, the future is our time .. */
#define HOSTNAME	"localhost"
#define USER		"atlantis"
#define PASSWORD	"datlanbaza"
//#define PASSWORD	""
#define DATABASE	"atlantis"

int resolve_ip; /* ci ma resolvovat tu IPcku alebo nema resolvovat tu IPcku
                   ... "Byt .... ci nebyt . ... TO JE OTAZKA!!!!!!!!!!!!"
                   po nasom: "Resolvovatj ... ci neresolvovatj ... to je taaa
                   spraaaavna otaaaazka!!!" Let it beeeee.... slafko... */

/*Sem, neviem preco prave sem, ale sem pojdu tie buffery ;) */
char revshoutbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revshoutline;
char revbcastbuff[REVB_LINES+1][REVIEW_LEN*2+2];
int revbcastline;
char revsosbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revsosline;
char revporshbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revporshline;
/* preco prave SEM? No neviem, ale SEM pojdu buffery pre gossip a quest! */
char revgossipbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revgossipline;
char revquestbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revquestline;
char revwizshoutbuff[REVIEW_LINES+1][REVIEW_LEN+2];
int revwizshoutline;
/*Sem, vobec neviem preco akurat sem, ale predsa sem, je to az neuveritelne
  ze sem, ale kedze sem, tak sem, pojdu tie lastlogy !!!!!!! ;) SEM !!! */
char lastlog[LASTLOG_LINES+1][REVIEW_LEN+2];
int lastlogline;

struct cl_struct
 {
  struct user_struct *hrac[4],*loadhrac[4];
  int monochrom[4],figurka[4][4],usertahs[4],remiza[4];
  int natahu,tahov,locked,kocka,loadlock;
  char revbuff[REVIEW_LINES+1][REVIEW_LEN+2];
  int revline;
 };
typedef struct cl_struct* CL_OBJECT;

struct fr_struct /* V) pharaon .. trosqu sa to scvarklo oproti clovece.. */
 {
  struct user_struct *hrac[5];
  int karty[32];
  int active[5];
  int natahu,action;
  char revbuff[REVIEW_LINES+1][REVIEW_LEN+2];
  int revline;
 };
typedef struct fr_struct* FR_OBJECT;

struct xa_struct
 {
  struct user_struct *creator;
  char *buffer[REVIEW_LINES+1];
  char topic[60];
  int line,type;
 };
typedef struct xa_struct* XA_OBJECT;

struct p_struct
 {
  char name[21];
  char dativ[21];
  char akuzativ[21];
  char inytiv[21];
  char food[21];
  int type,weight,dur,price,function,amount;
  int attack,firerate,seconddur,pp,restrikt,tajm;
  long altfunct;
  char *enter, *leave, *ustart, *rstart, *ustop,*rstop;
  char  *uinphr, *uoutphr, *rinphr, *routphr;
  char *userphr, *roomphr, *victimphr;
  char *special, *picture, *error;
  int showpict,ofense,defense;
  char *udestroy,*rdestroy;
  char *ujoinphr,*rjoinphr,*searchphr;
  struct room_struct *spawn;
  int spawnarea;
 };
typedef struct p_struct* P_OBJECT;

struct co_struct
 {
  int component[4];
  int eliminate[4],setowner;
  int product,source,destination,mana,heal;
  char *usermsg, *roommsg,*missing;
  char spell[21];
 };
typedef struct co_struct* CO_OBJECT;


/* The elements vis, ignall, prompt, command_mode etc could all be bits in 
   one flag variable as they're only ever 0 or 1, but I tried it and it 
   made the code unreadable. Better to waste a few bytes */ 
   
/* Tak, tak. Uz je to natolko neprehladne, ze aj sam Neil by ziral. Vy zirate
   my zirame a zurime, lebo pouzivame Azur. Nex zuju nove premenne pridane
   do tejto struktury ! :> */
/* Juuuj aj preto to asi tak padalo, ze to uz je take velke ze to v pamati
   blbne pri tej zlej praci s tymi hnusnymi pointrami, ale nejde o tooo! */
/* Kazdopadne, niekde nam tam desne uteka pointer, a nevieme kde ... uz to
   je asi fixnute, takze ked este tato verzia (3.0) nebezi, tak sa statocne
   vyhovarame na ten uz fixnuty pointer ... som zvedavy, na co sa teda budeme
   vyhovarat ked nahodime tuto verziu, a nebude to fungovatj?! Ja uz teda
   neviem, a ani radsej nexcem vediet ... :(( */
/* Co sa bojis. Ja zu daco na co to zvalime najdem. A ked nie, tak vyskusame,
   ci je suicide skutocne painless... */
/* Ale zda sa ze to zatial ide ... sice to fclose() ma riadne nasrrr... ehm,
   hnevalo, ale pohoda uptime uz bol cez 1 den na netlabe :> A cil ked sa este
   ma upgradovatj server, tak to bude fakt spica! */   
/* ... uz to aj je spica, ten qwyx je velmi, ale velmi dobry stroj! Ozaj
   presunieme to na mechan0.tps.sk :-> */
/* Jo, velmi dobry stroj, akurat ze si tam uz tiez rootuje kde-kto. Ale teraz 
   je to este spicovejsie, ten platon je velmi, ale velmi dobry stroj! */

struct user_struct { 
      int id;
      float visitor;
      char name[USER_NAME_LEN+1]; 
      char desc[DESC_CHAR_LEN+1];
      char pass[PASS_LEN+50]; 
      char wizpass[PASS_LEN+50]; 
      char email[EMAIL_LEN+1],requestemail[EMAIL_LEN+1];
      char sname[3][20];
      int zaradeny; /* ze ci je zaradeny do pouzivania! */
      int sex; /*SEX - pche, kam sa hrabe daky sex na riadnu partiu DOOMa! :>*/ 
               /* Rider, konecne spravna rec! :>>> HNUUUUSNY SEGZ! */
               /* Ale zasa dooma mam furt, hmmm... s tym treba cosi robit :> */
               /* A sex nemas furt? No neviem ja hej :( */
	       /* No, neviem, ci by tvoja pravacka neprotestovala ;-) */
	       /* KOKSO co tam po doome, QUAKE ]I[ ARENA je TOTAL SPICA 
	          3D DRTICKA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	       /* QUAKE ]I[ CIRCUS.. oko na nohach behat, who to kedy videl..
		  Unreal Tournament: Tactical Ops || HalfLife: CounterStrike! */
      char lasttellfrom[USER_NAME_LEN+1],lasttellto[USER_NAME_LEN+1];
      int lasttg;
      char prevname[USER_NAME_LEN+1]; 
      char follow[USER_NAME_LEN+1],smsfollow[15]; 
      char sell_to[USER_NAME_LEN+1]; 
      char in_phrase[PHRASE_LEN+1],out_phrase[PHRASE_LEN+1]; 
      char buff[BUFSIZE+10],site[82],last_site[82],page_file[82]; 
      char mail_to[WORD_LEN+1], *revbuff[REVTELL_LINES+1]; /* (S) -> DYNAMICKY */
      char afk_mesg[AFK_MESG_LEN+1],inpstr_old[REVIEW_LEN+2]; 
      struct room_struct *room,*invite_room,*into,*from; 
      int type,port,site_port,login,socket,attempts,buffpos,filepos; 
      int vis,ignall,prompt,command_mode,muzzled,muzzletime,charmode_echo; 
      int level,cloak,misc_op,/*remote_com,*/edit_line,charcnt,warned; 
      int accreq,last_login_len,ignall_store,clone_hear,afk; 
      int edit_op,colour,ignore,ignshout,igntell,ignportal,ignbeep,ignio, revline,jailed;
      int ignsys,ignfun,ignzvery,igncoltell,ignportalis;
      int ignblink,ignpict,ignspell;
      time_t last_input,last_login,total_login,read_mail,first_login,idletime;
      char *malloc_start,*malloc_end;       
      struct user_struct *prev,*next,*owner; 
      int pp, mana; /*PowerPoints, Mana*/
      char homepage[HOMEPAGE_LEN+1];
      char lastcommand[23],lastcommand2[23];
      int pridavacas; /* male opatrenie proti skriptarom (S) */
      int lastidle,lastidle2,lastidle3; /* vylepsene opatrenie proti skriptarom (V) */
      char newpass[PASS_LEN+50]; /* pre zmenu hesla */
      char chanp[USER_NAME_LEN+1]; /* pre zmenu hesla */
      int has_ignall;
      char uname[20],pis[21][21],znak;
      int joined,tah,last_x,last_y,game,moves;
      int alarm, hang_stage;
      char copyto[MAX_COPIES+1][USER_NAME_LEN+1]; /* Hnusni kopytovci!! */
      char hang_word[60], hang_word_show[60], hang_guess[60];      
      int lines,storelines,colms,wrap,who_type,rt_on_exit;
      int predmet[BODY],dur[BODY],carry; /* setky ruky usera, vaha */
      int zuje, glue, prehana,stars,lsd;
      int zuje_t, kridla_t,prehana_t,prehana_t2, muzzle_t;
      int lieta, viscount, afro;
      char call[21];
      char shtable[10][10];   /*10d1cky*/
      int shmode, shbuild, shstatus, zasah; /*10d1cky*/
      int browsing, messnum, lab;
      int lastm, autofwd, pagewho, examine, lang,com_priority; /*llang*/
      char real_ident[101]; /* Don't ask why :>> *//* Tell me why you cry...*/
                             /* And why u lie 2 meeee!!! */
      char remote_ident[81];
      char subject[150];
      int doom;         /* D00M je naaas...    */ /* dela dobre mne i tobje */
      int doom_energy;  /* zivot v D00Me       */
      int doom_score;   /* skore               */
      int doom_ammo[3]; /* strelivo pre zbrane */
      int doom_loading; /* nabijanie zbrane    */
      int doom_weapon;  /* zvolena zbran       */
      int doom_heading; /* pohlad v D00Me      */
      int doom_x;       /* suradnice s         */
      int doom_y;       /* polohou v D00Me     */
      int doom_sayon;   /* poistka na say :)   */
      int newtell, tellpos;  /* ci a kolko novych tellov ma.. */
      int com_counter;   /* M@CRO stuff*/ 
      struct macro_struct *macrolist;
      struct notify_struct *notifylist,*ignorelist,*combanlist; /* dynamicky notify-list */
      int igngossip,quest,flood;
      char prompt_string[PROMPT_LEN+2]; /*PROMMMPT*/
      int dead; /* ak ho to ma odlognut v heartbeate */
      int rjoke_from; /* Jose stuff ... */ /* Jose rocks.. Btw, nesiel by si 
                                              slavo s Josem & staf na vylet? */
                      /* no to je problem, ale KEDY? A islo by aj Kukadlove?
                         nefim zatial Ti povedatj ... */
		    /* neslo. Lebo nexcelo. Lebo.. Tchfuj :( */
      struct pager_struct *pager; /* dynamicky pager - aj ja xcem kupit pager! */
      int pageno;
      char ignword[WORD_LEN+1];
      int saturate,idle; /* kolko je spolu idle */
      int autosave,histpos;
      char history[HISTORY_LINES+1][HISTORY_LEN+1];
      /********************/
      char remote_name[USER_NAME_LEN+1],remote_passwd[PASS_LEN+100];
      int remote_login_socket,remote_login;
      struct remote_struct *remote[MAX_CONNECTIONS];
      int remote_socket[MAX_CONNECTIONS], ircsocknum;
      int actual_remote_socket,fds,remote_fds[MAX_CONNECTIONS];                   
      int akl;
      char irc_chan[50], irc_nick[21], irc_defnick[21], irc_name[51], irc_serv[12], *revirc[REVIRC_LINES+1];
      char channels[10][50];
      pid_t lynx;
      int sayflood, revircline, irc_reg;
      int statline, statlcount, newline, statlcan, remtelopt;
      struct blackjack_game_struct *bj_game;
      int ignlook;      
      int reversi_plan[8][8];
      struct user_struct *reversi_sh;  /*SpoluHrac*/
      int reversi_natahu;  /* 0 - spoluhrac   1 - ja   2 - nikto */
      int reversi_znak;  /*moj znak: 1 - X   2 - O */
      int reversi_cislotahu;  /*poradove cislo tahu  0 - nehra */
      int reversi_jept;  /*Je Posledny Tah*/
      int reversi_ptX,reversi_ptY;  /*Posledny Tah X Y*/
      /* Miny Struktura */
      unsigned char **miny_tab;
      unsigned int miny_x,miny_y,miny_h;
      struct dama_struct *dama;
      struct cl_struct *clovece;  
      int chrac,clovecewin,dhrac;
      struct user_struct *clovece_starter,*dama_opp;
      char ipcka[20];
      int col[7]; /* V) pastelky pre: shout, gossip, say, tell_name, tell, xannel */
      struct user_struct *farar_starter;
      struct fr_struct *farar; /* V) kartova gamesa narocna na rozmyslanie */
      int fhrac,fararwin,cps,goafkafter,killmeafter; /* cps = commands per session */
      char reserved[10];
      int commused[11],ignafktell,igngames,mailnotify,ignautopromote;
      int totaljailtime,killed,totalmuzzletime,nontimejails;
      int gold,swapped,shoutswears,sayswears,hangups,hangupz;
      int ignxannel,ignnongreenhorn,multiply,lastrevt,lastrevs,lastrevi;
      struct xa_struct *xannel;
      struct user_struct *invite2xannel,*nox; // next on xannel
      int sell_price,sell_what,ep_line,affected,affecttime,affpermanent;
      int smsssent,smsday,allowsms,smswait,smsgate,smschars;
      char mobile[21],logoutmsg[61];
      int team,way,weapon,hidden,reveal,attacking; //atlantis wars stuff
      int kills,deaths,health,heal,deathtype,head,body,feets,target;
      int switches,shortcutwarning,temp,wizactivity,age,agecode;
      int longestsession,mail2sms,timeinrevt,miscoptime;
      int macro_num,notify_num,ap,websecond;
      int skip,exams,kontrola,messsize,special;
      char findstr[20],request[7];
      char shoutmsg[21],shoutmsg2[21],gossipmsg[21],gossipmsg2[21];
      char saymsg[21],saymsg2[21],tellmsg[21],tellmsg2[21];
      char wizshmsg[21],wizshmsg2[21];
      char where[31];

      /* XML handling usera a jeho nastavenia */
      xmlTextWriterPtr xml_writer;
      xmlBufferPtr xml_buffer;
      int output_format;
      };   

/* A aby nebolo luto aj nasim drahym miestnostickam ... */

typedef struct user_struct* UR_OBJECT;

/* REM0TE */
struct pollfd fds[MAX_POLL];

UR_OBJECT user_first,user_last, remote_user_active;

struct room_struct {
      char name[ROOM_NAME_LEN+1]; /* meno miestnosti */
      char name_en[ROOM_NAME_LEN+1];
      char label[ROOM_LABEL_LEN+1]; /* label miestnosti */
      char *desc_sk; /* deskripcia miestnosti :> */ /* (S) -> DYNAMICKY! */
      char *desc_en;
      char topic[COOLBUF_LEN+1]; /* topik miestnosti */
      char *revbuff[REVIEW_LINES+1]; /* revbuff miestnosti (S) -> DYNAMICKY!*/
      char from[ROOM_NAME_LEN+21];/* "odisiel do miestnosti luka", luka njeje */
      char into[ROOM_NAME_LEN+21];/* miestnost, luka je luka, odisiel na luku (V) */
      char where[ROOM_NAME_LEN+21];/* whereami? na luke. (V) */
      char lemmein[USER_NAME_LEN+1];
      char topicmaker[USER_NAME_LEN+1];
      int countdown;
      int access; /* public , private etc */
      int revline; /* line number for review */
      int group; /* cislo skupiny miestnosti - ()STROV */
      int mesg_cnt; /* kolko je messagesov! */
      int sndproof; /* ci je zvukotesna */
      int invisible; /* ci je neviditelna :> */ /* TO JE AKOZE CO?! :> */
 		     /* To akoze ju neviediet v rooms pri levele < wiz */
      int logging; /* ZMENA - logovanie, pozri rlogging() */ /* Tfuj!! :(( */
      char link_label[MAX_LINKS][ROOM_LABEL_LEN+1]; /* temp store for parse */
      int predmet[MPVM+1]; /* predmety v miestonosti (V) */
      int dur[MPVM+1]; /* durability || timer of a predmet*/
      struct room_struct *link[MAX_LINKS];
      struct room_struct *next; 
      }; 
 
typedef struct room_struct *RM_OBJECT; 
RM_OBJECT room_first,room_last; 
RM_OBJECT create_room(); 

#define syserror    "Prepac, nastala systemova chyba" /* systemova xyba sa obskurla! */
#define nosuchroom  "Taka miestnost neexistuje.\n"
#define nosuchroom_en  "There is no such room.\n"
#define nosuchuser  "Taky uzivatel neexistuje.\n"
#define notloggedon "Nikto s takym menom nie je momentalne prihlaseny.\n"
#define invisenter2  "Neznamy vstupil do miestnosti...\n"
#define invisenter(user) pohl(user,"Neznamy vstupil do miestnosti...\n","Neznama vstupila do miestnosti...\n")
#define invisleave2  "Neznamy opustil miestnost...\n"
#define invisleave(user) pohl(user,"Neznamy opustil miestnost...\n","Neznama opustila miestnost...\n")

#define invisname(user) pohl(user,"\253Im","\253Iz")
#define invisname2(user) pohl(user,"Neznamy","Neznama")

#define nocolors  "Nemozes pouzivat farby.\n" /* Papluha, ogrgel akysi! */
#define noswearing  "U nas sa nenadava, ty grobian!\n" /* Papluha, ogrgel akysi! */
                                                   /*     ^^^^^^^^^^^^^^^^^^^^^^ */
#define ZALAR_TLT_CONSUMPTION 30

/* S> Ta hnusna Hvozdarka ma sakra 2x za sebou vyrazila .. strasne nexutne!
      tak nex je aspon takto potrestana ... JUUJ! To bola prva trauma z
      ustnej skusky ... sakra ja neznasam uuustne skuusky :-((( */      
/* no asi sa k tomu pridaju aj "tomaska" a "zalezacka" .... :(( Hnusny
   odporny managedement, kua tie hnusoby ma uz museli vyrazit?!?! */
/* a Starla vraj este ze "to maaas f pohode ked to maaas u Tomasovej" !!! no
   sisku drevenu v druhej maringotke to bolo f pohode !!! :(( A to Starla ma
   uz neraz takto oblbla, tiez s Ekonometriou ... vraj "Hatraaak je v pohode,
   ten to daaa kazdeeemu!!!", no a xybali mi 3 body, a vyflakol ma a nexcel
   mi to dat :((( Juuuj! Sakra ale som sa nejako rozpisal ... :> */
/* Zaver: ja uz tu Starlu NEBUDEM pocuvat, len ma oblbuje :(((( */
/* PS: este stastie ze mi to iste nehovorila o Operaku, lebo to by ma uz asi
   naozaj priviedlo do hrobu :( */
/* Boooze Spartakus, ty sa vies rozkecat... Uz skoro ako ja... Nechces sa
   liecit? A ake z toho vyplyva ponaucenie: Nepocuvaj rady bab, lebo skoncis
   nahouby!!!! Tak, tak, ver skusenemu starsiemu cloveku... :>> */
/* Tak pocuj o tomto sme sa uz raz bavili ... vraj STARSI? Haha, o 1.5
   mesiaca, ale tu sa neberie ze starsi ale skusenejsi a to som ja, pretoze
   sice ty nevidis ked sa nieco deje, ale potom zas dostanes facku :>> -sice
   aj to je skusenost, ale ja vidim cely priebeh :> No co uz, clovek si svoj
   osud nevyberie, vsakaaaano :> */         
/* No a CO ze o jeden a pol mesiaca! Ale starsi a to je podstatne! Takze ty
   ucho jedno, nevyskakuj tu na nas, starsich a skusenejsich a budes pekne
   posluchat a nosit desiate a vreckove! :>> */

/* Uff, globalky !!!!!!!!!!!!!!! */

#define T_SIZE ARR_SIZE*3 /*dlzka textu!!! ('koze sorry ...) */

void query_com(UR_OBJECT user,char *inpstr);
char ipcka_glob[20]; 
char text[T_SIZE+2]; 
char texthb[T_SIZE+2];

char word[MAX_WORDS+1][WORD_LEN*2];
char irc_word[MAX_IRC_WORDS+1][IRC_WORD_LEN+1];
char irc_lw[IRC_WORD_LEN+1];
char wrd[10][100]; 
char progname[40],confile[40]; 
char motd2_file[100];
char teamname[5][31];
char *motd;
time_t boot_time; 
jmp_buf jmpvar; 
 
int port[2],listen_sock[2],wizport_level,minlogin_level; 
int colour_def,password_echo,ignore_sigterm; 
int max_users,max_clones,num_of_users,num_of_logins,heartbeat,disable_web; 
int login_idle_time,user_idle_time,config_line,word_count; 
int tyear,tmonth,tday,tmday,twday,thour,tmin,tsec; 
int mesg_life,system_logging,prompt_def,no_prompt; 
int force_listen,gatecrash_level,min_private_users; 
int ignore_mp_level,max_sms,last_memory,smscycle;
int destructed,mesg_check_hour,mesg_check_min,net_idle_time; 
int ban_swearing,crash_action,april,lastrand;
int time_out_afks,allow_caps_in_name,rs_countdown,pp_zapnute; 
int charecho_def,time_out_maxlevel,zvery_speed,visit; 
int max_id,backuptime,sendvec;
time_t rs_announce,rs_which; 
UR_OBJECT rs_user;
UR_OBJECT current_user;

#define MAXCARRY 40
#define MAX_PREDMETY 300
#define MAX_CONVERT 100
P_OBJECT predmet[MAX_PREDMETY+1];
CO_OBJECT convert[MAX_CONVERT];

/*extern char *sys_errlist[];*/
char *long_date(); 
struct macro_struct *defaultmacrolist;
struct room_struct *testroom; /* aby som furt nedavel write_room(NULL,text);
 lebo potom na to zabudnem a ked to nahodim tak je z toho haluz prevelika (V) */

/* Oh boze, naozaj to xcete?! Tak dobre, tu je to - autorizacia usera! */
/* polka veci tam je zbytocnyx!!! ak sa vam v tom xce rypat, prosim, mne
   nie :> */
/* Mne tiez ne... */
 
#ifndef AUTHUSER_H 
#define AUTHUSER_H 
 
extern unsigned short auth_tcpport; 
extern int            auth_rtimeout; 
extern char *auth_xline(); 
extern int auth_fd(); 
extern int auth_fd2(); 
extern int auth_tcpsock(); 
extern char *auth_tcpuser(); 
extern char *auth_tcpuser2(); 
extern char *auth_tcpuser3(); 
extern char *auth_tcpuser4(); 
extern char *auth_sockuser(); 
extern char *auth_sockuser2(); 
#endif 


/* (R) definicie Kolosea + prislusne funkcie (deklaracia) (S) uprava*/
#define MAXSTRING 150  /* maximalna dlzka stringu v scenari       */
#define ZERO 0 /* toto bolo zmysluplne, Rider :>> */
               /* okay ked mozes ty, tak idem aj ja:*/
#define JEDNA 1 /* S~me s~e pobavili, Slafko..! */
#define DVA 2
               /* no dobre, stacilo :->> Kruci, tiez musis definovat krawiny */
#define TRI 3 /* no jo, co uz ... */ /* Nic... zastrelte ho! */

#define KOLOS_PROGRAM "amfiteater/program.atl" /* nikto nam nexce pisat scanare */
       /* keby tam tak dame tie Miloskove porno-poviedky ... ale sakra co
          keby to videla Mamina ... :>>> Co ty vies, mozno by tu bola castejsie :> */
       /* myslis? :> Ne, ona xodi na StarWars ovela castejsie ako na Tis ...
          (prip. Talis :>) ... zeby tiez pa3la do Igiho tajnej bandy? No teda,
          nevyzera na to :> */
       /* Co myslis, (v)zrusia ich pri upgrade Qwyxa? Inak, ked uz spominas
          Talis, je to celkom milucke blondave diefcatko... Teda, nic moc, ale
	  zasa lepsie ako draatom do oka :) Sme minule fayn pokecali, jux! */

/* deklaracia funkcii amfiteatru: */
long play_nxt(void);
long asctomin(char string[], int upd);
char *chrcat(char *dest, char src);

RN_OBJECT create_remote(UR_OBJECT user);
int init_remote_connections(UR_OBJECT user);
int connect_to_site();
int exec_remote_com();
void remote_close();

/* Definicie deklaracie D00Ma */
#define D_LINES 7    /* Velkost okna s DOOMom */
#define D_COLMS 100  /* Buffer pre dlzku okna */
#define D_LOCAT 21   /* Pocet lokaci v grfile */
#define D_MAPX  5    /* X-ova velkost mapy (sirka) */
#define D_MAPY  5    /* Y-ova velkost mapy (vyska) */
#define D_AMMO  20   /* Gulovnica */
#define D_GRAPH "doom/graph.doom"
#define D_DATA  "doom/data.doom"
#define D_FILE  "doom.top" /* aj tak bude Rexo prvy :> No iste, zapomen! :> */
                           /* No dobre, tak Dar Kvader :> */ /* Ee, Amazonka! */

/* Externe premenne D00Ma */
int doom_status;     /* Ci DOOM fici alebo je v... pazi */
                     /* S>... alebo je KDE, RIDER????? v &lt;&gt, hehehe */
                     /* Rider sa nam KAZI, JEDNOZNACNE!! */
                     /* Rider sa nekazi, iba Spakky mysli na same prasaciny! */
                     /* pockaj pockaj kto napisal ten prvy riadok? A je uplne
                        jasne na co si pritom myslel!!! */
		     /* JASNE? A na co teda podla teba?! */
int doom_players;    /* Pocet hracov */
char doom_grf[D_LOCAT][D_LINES][D_COLMS]; /* D_LINES-1 */
char doom_map[D_MAPY][D_MAPX][5];

/* Deklaracie D00Ma */                 /* .... proste neuveritelnet!!! */
extern UR_OBJECT doom_get_user(int x,int y);
extern UR_OBJECT doom_check_view(UR_OBJECT user);
extern void doom(UR_OBJECT user,char *inpstr);
extern void doom_showmap(UR_OBJECT user);
extern void write_doom(UR_OBJECT user,int location);
extern void doom_checkround(int x,int y);
extern void doom_load_users(void);
extern void doom_loser(UR_OBJECT user);
extern int doom_join(UR_OBJECT user);
extern int doom_check(int x,int y,int smer);
extern int doom_init(void);
extern int doom_points_check(UR_OBJECT user);
extern void doom_text(UR_OBJECT user,char *msg,int beep);
extern void doom_quit(UR_OBJECT user);
extern void doom_who(UR_OBJECT user);
extern void doom_wall(UR_OBJECT user,char *msg);

/* Globalna struktura play - ked viete ako bez nej, tak prerobte... */
/* Zdalo sa mi to sikovnejsie ako naflakat 20 roznych premennych... */
/* S> tusim ze tuto to vsetko zacalo, ze?? :>> */
/* No jasne, a niekto to fuuurt musi opakovat... ;-) */

struct {
int time;    /* Uz bude sluzit len pocas playovania              */
int hodina, minuta; /*Hodina a minuta zaciatku dalsieho predstavenia */
char on;          /* flag: 0-caka hru; 1-hra; 2-vypnute       */
char name[80];   /* dalsi citany "scenar", meno suboru       */
char nazov[80];
FILE *file;       /* subor so "scenarom", resp. pointer nan   */
} play;

/* (S) DEFINICIE suborov pre WEB publikovanie 
   Pozor - v ceste k suborom nepouzivat ~ lebo to asi nejde :> */
 /* Kua samozrejme ze to nejde, a ani nepojde !!!! :<< */
 /* Teeeda, spakky, no fujha, tak sa nehovori :>> yasne ze to nejde, kam
    bys sel kdyz nemas shell, ze ano :> */
 /* S> Blbost, ved to ide aj s nie-shellom, ved sa to pouziva napr. aj vo
       webkax, aj vsade ... tak by to malo inct!!! */
/* ale problem je v tom ze to nejde :< no nevadi, ale dolezitejsi je ten
   blby resolver */       
/* (Spartakus sings: *she sells Bourne shells by the sea shore ...*) */   
/* Ja asi odidem do doxodku na stare kolena ... akoze sorry, ale ten hnusny
   resolver blbne aj na tom Netlabe! Do (net)laby!!! Fakt uz nexapem co toto
   ma znamenat...*/
/* takze Rider ten resolverisko uz vopravil, ale robi to zombikov, aspon
   u mna doma. On si akoze mysli, ze ked sepali zombika devinou, tak mu
   to pojde. HOVNO!! (:>) Zombik sa neda sepalit. Zombik sa odpazi az ked
   sa odpazi proces ktory ho tam zapazil. A double fork nefunguje. RIDER!
*/      
/* Yag zombikof, ti dam zobikof, zjadnyx zobikof... xybalo tam dake wajtpid,
   tak som to bugfixol a Zombici sa odobrali nazad do pekiel jazernych, johoho,
   aj s flasof rumu... */
   
#define KOLOS_HTML "../public_html/amfiteater/index.html"
#define JOKEBOARD_HTML "../public_html/jokeboard/index.html"

/* kontrolne cisielko pre suicide (that is painless) */
/* RIDEEEEEEEEEEEEEEEEEEEEEEER!!! Co som Ti povedal?!?! :>> */
/* Co peknuckeho? Nahodou... Co mas furt proti tej Qjetyne? Ved ona je celkom
   fajn diefcatko... Pravda, este stale nemozem odprisahat, ze ma nevodi za
   nos, ale ked je to take hrozne zlate... Ako vobec na to mozes kaslat? Ved
   diefcatka su take pjekne makkucke milucke a pritulne... Teda akoze da sa zit
   aj bez nich, ale je to tak troxu nuda... :( */
/* Ja mozem odprisahat ze neviem o co vam vlastne ide :> Ale zda sa ze sa
   Kukadlove uz konecne trosku umudrilo, a ze uz nerobi take krawiny ako
   predtym - myslim ze ta lekcia riadne pomohla :>>> Uz aj ku mne sa sprava
   trosku inak, cece :) */   
/* a ja som sa pre zmenu zalubil - do Danky Chudej ... sakra ona je SUPER!*/
/* ... ale minule som nepoxopil co to akoze malo znamenat na tych datate.
   Sme tam celu hodinu a pol sedeli, potom prisiel Nehez, povedal nieco co
   som vobec nexapal akoze o com malo bytj, a isli sme prec ...
   Ale aspon som sa dozvedel, ze letiaceho ftaka nemozes sepalit sipom :>
*/
  /* Tak moment ten riadok o priblizne 6 vyssie beriem spat ... */
  /* No a samozrejme Rider ty budes mat tiez xut asi z tych replik hore
     nieco zobrat spat ... :( */
  /* Ja? Ani za nic! Fakt su take mile, pjekne, pritulne, makkucke, sladucke a
     tag... Akurat ma smolku. Btw, ten ony, co mu na meno prist nefim, je
     vlastne celkom v pohode xalan, snad jej s nim bude aj celkom dobre,
     myslim, ze ju ma celkom rad... Ale aj tak myslim, ze somnou by jej bolo 
     lepsie, viac sa k sebe hodime ;-) Este aj Steelka to potrvdila ;-)
     Err... a... btw, ked tak neznasas Danku, preco jej robis tu svocku?! :>
  */
  
int logcommands; /* ci ma logovat prikazy */
/* samozrejme ze ma, mohlo by to byt default na 1 !!!!!! */
/* JO, A KTO BUDE KONTROLOVAT QUOTU?!?!?! No way, Sir!!! */
/* Aye Sir, je to v tempe, takze pohoda :->> */
/* Hnusny Sir!! */
/* ??? ^^ to kto pisal?!?! ja ne, sakra a predtym to tu nebolo :> ??? */
/* UFONI ZELENI!!!!!!!!!!!!!!!!!!!!! */
/* Zeleni nie su ufoni, ale policajti a Zaba... :>> */
/* Zaba ma este k tomu aj water-resist-eyes ... Hnusna Zaba! Ktovie ci
   len do 200 metrov, musime vyskusat ... alebo by zvladla 2k4 mil pod
   morom? Ak hej tak ja nexcem byt Kapitan Nemo!!!!!!!!!!!!!!!*/
/* Ty ses vuul! Tyx 2E+4 mil pod morom bolo myslene ako vzdialenost, nie
   ako hlbka! Taka priepast tu nikde na zemeguli neexistuje! Iba cosi okolo
   11km (a o je cca 6 mil, takze...) 20 tisic mil, to mas.. err.. ax, ta
   moja matika... ak dobre ratam, okolo 37000 kilometrof... HUH! Zacinaju ma
   tie vypocty bavit. Takze, keby si sa ponoril do hlbky 20 000 mil, tak by
   si preliezol durch zemegulov a este by si vyskocil 24 000 kilometrov do
   vesmiru. Alebo inak, ak by si sa na druhej strane zeme otocil, tak by si
   ju presiel celu dva krat skrz-naskrz a este by ti ostala rezerva na
   vytrepanie sa do stratosfery... Este ma napadla taka vec, ze vyratat
   hypoteticky tlak na ponorku v takej hlbke, ale nexce sa mi... ;-))*/
 
/* Definicie pre autorizaciu usera. ZASA !! */ 
/* ... a tu je dokonca tusim zbytocne skoro vsetko :>>> */
/* Ale rypat sa mi v tom nexce, takze smolka! :>> */
/* prizo mi na zaciatku nexcel verit ze viem ako to pracuje */
/* No a? Ja ti to neverim doteraz a vobec sa tym netajim :> */
/* Akoze sorry, ale: connectne sa na port 113 daneho servera odkial ide
   luzer, posle 2 cisla: nas port (7000) a cislo portu z ktoreho ide
   luzer, a server mu posle userid usera ktory robi danu tcp connection.
   Je to proste ako facka, tak co :> */
/* Akoze sorry, ale ked je to take lahke, tak preco si to nenapisal sam? */

unsigned short auth_tcpport;
int            auth_rtimeout;
 
#define SIZ            500 
#define MAX_CHAR_BUFF  380 
#ifdef FNONBLOCK                     /* SYSV,AIX,SOLARIS,IRIX,HP-UX */ 
# define NBLOCK_CMD FNONBLOCK
#else
# ifdef O_NDELAY                     /* BSD,LINUX,SOLARIS,IRIX */
#  define NBLOCK_CMD O_NDELAY
# else
#  ifdef FNDELAY                     /* BSD,LINUX,SOLARIS,IRIX */
#   define NBLOCK_CMD FNDELAY
#  else
#   ifdef FNBIO                      /* SYSV */
#    define NBLOCK_CMD FNBIO
#   else
#    ifdef FNONBIO                   /* ? */
#     define NBLOCK_CMD FNONBIO
#    else
#     ifdef FNONBLK                  /* IRIX */
#      define NBLOCK_CMD FNONBLK
#     else
#      define NBLOCK_CMD 0
#     endif
#    endif
#   endif                                                                                                                                                                                                /* hnusne Kukadlove! */
#  endif /* To Kukadlove tam za obzorom minuleho riadku CO MA BYT?! */                                                                                                                                   /* ona je aj tak strasne super...*/
# endif  /* Teda vies co, a mne hovoris ze na nu furt myslim. A pritom vobec */
#endif   /* nie furt. Iba rano, naobed, vecer, v noci a .. no dobre, no a co!*/
       /* no vidis ... Is it love, tak co :> - Hnuuusne Kukadlove! Ale na
          druhej strane, keby si si to neprezeral v t602-ke alebo v norton
          editore tak by si si to nebol vsimol!!! Juuuj! Hnuuusny lord norton!
          a bolo by to tam az do umrtia smutku, a nikto by si to nevsimol ...
          a o 50 rokov by sme na to pozreli a "... Kukadlove? Jaj to je
          moja staraaaa ...." */
     /*  na tomto ^^^^^^^^ sa vraj Rider rehotal az sa za bruxo xytal. Ja
         teda neviem co je na tom take ftipne, mne sa to ftipne nezda :> */

/* Sibenica. Ak mate slabe nervy, keep out!! Ak mate slabe nervy a chceli 
   by ste prerabat dizajn tejto milej hry, tak keep out double!
   Pozrite sa na dlzku tyx riadkov ... :> Ale ved: NEJDE O TOOOOO ! :> */
/* Ocuj, nesibalo ti z tej sibenice ze si to takto robil? :> */
/* S> No tak to skus spravit inak ked si taky mudry :>> Nic rozumnejsie ma
      v tej xvili nenapadlo, tak to bude takto .. kazdopadne menit ten dizajn
      fakt neodporucam, koli moznej vaznej psyxickej ujme :->> */
      /* heheh aspon tam bude musiet NAVEKY byt ten moj krasny nadherny
         cacany dizajn, heheheheheh!!!! Este dobre ze tam nieje ruzova :> */
/* ... zabudol som este: ... NADOPOZEMSKY A TRANSCENDENTALNY!!! */
/* Mno, ono z globalneho hladiska pohladu na danu problematiku mozno na zaklade
   emprickych znalosti konstatovat, ze tento dizajn bude na houby, ale v ramci
   aspektu standarizacie a unitarity, takze v podstate nepovazujem za akutne
   nutne prevadzat akekolvek interne diferenciacie (ak nevies co som tym chcel
   povedat, tak sa netrap, ja tiez nie :>) */
/* Keby tam nebol ten spicaty ostry smajlik na konci, tak by som si na moj
   dusu myslel ze to pisal Buko :>>> */
/* Apropos, Buko .. uz ZASA (9.4.98, 17:10) zasiera qwyxa MP3-kami!! Teda,
   akoze sorry, ale TOTO NE! :((( */
/* Mno a co, dnes (4.6.1999) takisto mp3kuje .. mimoxodom Slafko (teda ja)
   tam vyssie ma byt 9.4.99 asi, a nie 98, co uz nefis ani aky rok je?!?!)
   ..ale zasa to ma aj vyhodu: nehrozi ti kriza z y2k, az o rok ;-)
*/


/* Tam niesu len rovnitka, ono to pokracuje aj dalej, staci sa len
   kuknut .... a pripadne zmenit, ale nexajte to tak ako to je, je to dobre
   :> */
/*   
#define talker_signature  "===============================================================================\nForwardovana sprava z Atlantis Talkera (telnet %s %d)\n"
#define talker_signature2 "===============================================================================\nPoslane z Atlantis Talkera (telnet %s %d, %s)\n"
*/

#define talker_signature  "-------\nAtlantis talker (telnet %s %d, %s)\n"

/* Sem, ale to uz fakt presahuje vsetko, neviem preco akurat sem, ale predsa-
   len som sa rozhodol ze sem, aj ked to je uplne f pazi, ale sem, sem, sem,
   sem, sem, sem, sem, sem a just sem, aj keby to nemalo koli tomu fungovat
   ze sem, ale predsalen sem, pojdu tie poondiate hnusne idiotske posrane
   lamerske blbe predmety !!!!!!! :> */ /* ale no, mojko, pozor na pusu, oki? */
/* pocuj nepouzivaj slovo Mojko, lebo tak mi hovoril Pasmo ked sme este boli
   milenci, a teda to vo mne vyvolava spomienky ... sice prijemne, ale potom
   skor neprijemne nad stratou Pasma ... :((( Juuj! :> */
/* Ty si chodil s Pasmom?? A mne ste sa hned vy dvaja zdali akysi mierne
   inak zamerany, ale ved neeejde otoo.. a Jester v tom ide s vami? Alebo to
   bol prave on, kto vam ten krasny vztah rozbil?? :>>> */

/* (konecny verdikt: HNUUUSNY PASMO!) */


/* Quest - questoviny */
#define QLEADER 6         /* min. level "questleadra" */
#define QUEST_DIR "quest"
#define QUEST_FILE "quest.top"
#define QUEST_TEMPFILE "questers.tmp"

/* Globalna struktura quest - ked viete ako bez nej, tak prerobte... */
/* Zdalo sa mi to sikovnejsie ako naflakat 20 roznych premennych... */
/* (text above copyright by (R), used with NO permission by (S) :> */
struct {  /* Struktura pre quest, zmena (S) */
	int queston;              /* Flag ci bezi ci nje      */
	int lastquest;            /* Iba jeden quest za den   */
	char lq_time[10];          /* Kedy bol posl. quest */
	char lq_leader[13];       /* Leader posl. questu */
	char lq_winner[13];       /* Vitaz posl. questu */
	char questfilename[80];   /* Zaznam questu - filename */
} quest;

/* definicie shipping - ()STROV 
   Toto su definicie pre nasu lodicku. */
            /* je to jednoduhsie ako naflakat tam 20 roznyx premennyx :) */
int ship_timer;
#define SHIPPING_ISLAND  "ostrov"
#define SHIPPING_HOME    "pristav"
#define SHIPPING_SHIP    "plachetnica"
#define PORTALIS_KISS    "luka"
#define BRUTALIS         "brutalis"

/* Velmi cool bolo ked sme nahodili novu Atlantis (vsimas si ako sa ucim -
   novU Atlantis nie novY Atlantis :>), tak ludia zacali shoutovat ze
   "sakra kedy pojde atlantis, kedy to nahodite, kedy kedy .." a my ze
   "do paze VED STE NA NOM!!" a oni furt to svoje... zaujimave, ze na
   atlantise kricali ze kedy pojde atlantis, to je uz uplna zawislost :>
   A korunu tomu nasadil Tomas, ked sa prihlasil a shoutol:
   Tomas shouts: Kua jaky pertalis?
   :->>>>>>>>>>
   Alebo raz som bol v sound-proof miestnosti, a zrazu som dostal .smail od
   Tomasa: Kua co si to v jakej vzduchotesnej miestnosti!
*/

/* Alternativne desky pre island, home a lodicku! */
/* neviem naco sa tu s tym babreme, aj tak si to nikto nebude vazit */
/* Lebo nas to tesi ;>> A robi nam to dobre ;>>>*/

char *alt_island_desc;
char *alt_home_desc;
char *alt_plachetnica_desc;

void make_travel();
void link_room(RM_OBJECT, RM_OBJECT);
void unlink_room(RM_OBJECT, RM_OBJECT);
void link_room_oneway(RM_OBJECT room,RM_OBJECT rm);

/* FLYER - a mame v Atlantide po Doomovi letecky simulator :>> 
   (no a co ze som blazon, liecit sa aj tak nebudem! :)) (R) 
   S> vsetci su blazni, len ja som LIETADIELKO ( <--cim samozrejme nexcem
      nic naznacitj! :-))) */ /* ... ((( but it's made of steel ... :> )))
   R> Teda slafko... No fuj! Takto rypat do nevinnych ludi.. "All alone, I
       have cried, silent tears, full of pride, in the world made of steel...
       No a COO!! Ostatne, teraz to je ganz fuka (frantisek), ale vies co je
       zaujimave? Ze to Lietadielko sa dost podoba na Agnethku... Teda aspon
       na jednom obrazku co mam UPLNE! A zasa Tynka je zasa hotova Fridushka..
       Je to vobec mozne? Vies co, hovor mi Benny, oki Bjorn..er, Slavo? :> */
   /* ja niesom ziadny bjorn ani bjork ... bleeah ... ja som ... co som
      sakra? Jaj uz viem ... ja som Majkl Dzekson!!! A kto je viac?? :> */
   /* no, ako sa tak nad tym zamyslam, tak v podstate uplne kazdy... :>> */       
   
#define FLYER_TIMEOUT 15  /* 30 sekund na odlepenie sa od zeme :> */
                          /* Logicke! Tak ma tam 15, a pritom pise ze
                             30 sekund ... akoze sorry Rider :>> (ale no
                             ja viem ze heartbeat, ale aj tak :> */
#define FLYER_ROOM "letun" /* hnusny letun */
#define FLYER_HOME "vzdusny_pristav" /* hnusny vzdusny_pristav */
#define FLYER_LAND "namestie" /* namestie (hnusne) */

/* Globalna struktura flyer - ked viete ako bez nej, tak prerobte...
   Zdalo sa mi to sikovnejsie ako naflakat 20 roznych premennych...
   (spakky, ty mi nekradni moje texty, anoo??? :>>>>>> (R):> 
   S> Px, ja si budem kradnutj co sa mi zaxce, ale ved som tam napisal
      ze used without permission, tak co?! 
   R> Jak "tak co"? Dostanem dvanast tisic za pouzivanie autorskych prav!!      
  */

   
struct {  /* Struktura pre flyer, zmena (R) */
	int pozicia;              /* Pozicia letuna            */
        int vykon;		  /* Vykon eletioveho cerpadla */
	int mind;		  /* Posobenie silou vole :>   */
        int vyska;		  /* ...co to asi bude...      */
        int palivo;		  /* Stava! :>                 */
        int vzdialenost;          /* Kolko uz toho preletel... */
        int timeout;              /* Nespiiime, startujeme!    */
        int gotta_write;          /* ci zapise skore a ci nie  */
        char pilot[20];          /* Kto to bude pilotovat :>  - ZMENA - meno!*/
   /*   ^^^^ Rider pliz do not kill me now ... but LATER you won't run away! */
} flyer;

struct {        /* GUEST struktura */
	int on;               /* ci guest mode prave bezi */
	int getuser;          /* ci ma vediet getnut usera guest */
	UR_OBJECT user;       /* pointer na usera guestoweho */
	UR_OBJECT moderator;  /* pointer na moderatora */
	UR_OBJECT talk;       /* pointer na toho kto moze polozit otazku */		
	char name[40];        /* meno - pojde do descu a do sayu */
} guest;

#define GUEST_ROOM  "krcma"

void flyer_fly();
int check_crash_flyer();
void eject(UR_OBJECT user);

/* M@CRO tak odtialto su to tie makroidne blbosticky ...
   Goda jeho Spartaka, to je ale bordel! :> 
   S>No a co, hlavne ze je pohoda a ze to funguje :>>> */
/* apropos Spartakus ... prave bola v pokuseni taka otazka, ze:
   Kto bol vodca rimskych otrokov v roku 73 pred n.l. ??? ;-))) -> akoze
   sorry, ale toto ne ;> */ 
/* R|msky 0tr0k.FMX rules!!! */
/* Jo, hlavne to FMX! :> */
   
#define MAX_COMMANDS 5
#define TRUE 1
#define FALSE 0

#define OUTPUT_FORMAT_PLAIN 1
#define OUTPUT_FORMAT_XML   2

#define INPUT_SEP '\\' /* cim sa bude separovatj */
#define NUM_MAC_PARAM 10 /* max. pocet parametrof */
struct macro_struct {
	char *name; /* meno makra */
	char *value; /* nahradzovaci string pre makro */
	int star;	/* zaciatocny word pre $* expresiu :> */
	int is_running; /* ci to makro bezi alebo nie! */
	struct macro_struct *next; /* pointer na dalsie makro, alebo NULL */
	};
typedef struct macro_struct *MACRO;
enum result {SUCCESS,FAIL};  
void free_macrolist(MACRO *list);
void free_macro(MACRO macro);
int save_macros(MACRO *list,char *filename);
int load_macros(MACRO *list,int id);
void macro(MACRO *list,char *input,UR_OBJECT user,int is_action);
void global_macros(UR_OBJECT user);
void delete_macro(UR_OBJECT user, MACRO *list,char *name);
int update_macro(UR_OBJECT user, MACRO macro,char *value);
void add_macro(MACRO *list,char *name,char *value);
MACRO allocate_macro(char *macroname,char *macrovalue);
MACRO ismacro(MACRO *list,MACRO *deflist,char *name);
MACRO findmacro(MACRO *list,MACRO *deflist,char *name);
MACRO findprevmacro(MACRO *list,char *name);
void parse(UR_OBJECT user,char *string, int irc);
int macroexpand(MACRO *list,MACRO *deflist,char *expansion,UR_OBJECT user, int irc);
void fix_separators(char *input,char *output);
char todigit(char c);
void check_death();
void got_line(UR_OBJECT user,char *inpstr);
/* definicia funkcii pre notify */

void free_notifylist(NOTIFY *list);
//int save_notifylist(NOTIFY *list, char *filename);
int load_notifylist(NOTIFY *list,int id,int noti);
int delete_notify(NOTIFY *list, char *meno);
void add_notify(NOTIFY *list, char *name);

/* game definitions */

#define DEFAULT_BJ_BET   10
#define USE_MONEY_SYSTEM  0

/* game structures */

struct blackjack_game_struct {
  short int deck[60],hand[10],dealer_hand[10],bet,cardpos;
};
typedef struct blackjack_game_struct *BJ_GAME;


/************************* FUNKCIE!!!!!!!!!!! ******************
 A sem, neviem preco prave sem, ale skratka sem pojdu deklaracie
 funkcii... Kto sa obetuje a spravi to??
 R> No ako vzdy.. daky upny kreten, co si nevazi vlastny zivot
    a pusti sa do toho... Takze zasaa jaaa....
 **************************************************************/


RM_OBJECT get_room(char *name,UR_OBJECT user);   /* deklaracie */
RM_OBJECT get_linked_room(char *name,RM_OBJECT room);
char *meniny(int mday, int mmonth);
char *datum_menin(char *meno);
char *get_temp_file();
char *pohl(UR_OBJECT user, char *muz, char *zena);
char *zwjpohl(int vec, char *muz, char *zena, char *ono, char *ich);
int expand_predmet(char *vec1);
void zazuvackuj(char *inpstr, int ok);
void zachlastaj(char *inpstr, int ok);
void zachlastaj2(char *inpstr, int ok);
void zalamerizuj(char *inpstr, int ok);
void debilneho(char *inpstr, int ok);
void pohaluz_predmetom(char *inpstr,int ok,int vec);
void do_funct_stuff(UR_OBJECT user,char *inpstr,int start);
char *real_user(UR_OBJECT gdo);
char *sklonuj(UR_OBJECT juzer, int pad);
char *skloncislo(int pocet,char *squirrel,char *squirrle,char *squirrelov);

char *lamerize(char buf[]);       /* nadefinovacie fcii, ktore grcaju char */
char *lame_color(char buf[],int typ);
char *revert(char vstup[]);
char *colour_com_strip2(char *str,int usercol);
char nxtznak(char *string);

int check_passwd_simplex(char *passwd);
char *expand_password(char *pass);

void zrus_pager_haldu(UR_OBJECT user);
void aklient_log(char *meno);
 
int skontroluj();
int quotacheck(char *username);
int showfile(UR_OBJECT user, char filename[90]);
int showrow(UR_OBJECT user);

void write_user(UR_OBJECT user,char *str);
void write_level(int level,int above,char *str,UR_OBJECT user);
void writesys(int level, int above,char *str,UR_OBJECT user);
void write_room(RM_OBJECT rm, char *str);
void write_room_except(RM_OBJECT rm, char *str, UR_OBJECT user);
void write_room_except2users(RM_OBJECT rm, char *str, UR_OBJECT user, UR_OBJECT user2);
void write_syslog(char *str,int write_time);

void record(RM_OBJECT rm,char *str);
void record_tell(UR_OBJECT user,char *str);
void record_shout(char *str);
void record_portalisshout(char *lbl,char *str);
void record_gossip(char *str);
void record_quest(char *str);
void record_wizshout(char *str);
void record_lastlog(UR_OBJECT user, int spent, char *dovod);

int more(UR_OBJECT user,int sock,char *filename);
int sqlmore(UR_OBJECT user,int sock,char* queryname);
int mailmore(UR_OBJECT user);
char *sqldatum(char *str,int telnet_cols,int type);
char *boarddate(char *dejt);
int is_webuser(UR_OBJECT user);
void highlight_write2sock(UR_OBJECT user,char *buff,size_t num);
int is_number(char *str);
int contains_swearing2(char *str);
int contains_swearing(char *str,UR_OBJECT user);
int contains_advert(char *str);
int colour_com_count(char *str);
char *colour_code_show(char *str);
char *remove_first(char *inpstr);
void init_globals();
void set_date_time();
void init_signals();
void load_and_parse_config();
void check_messages_night(int vculeky);
int countjoke();
void log_commands(char user[],char str[], int timeword);
extern int lab_load();
void init_sockets();
void boot_exit(int code);
void amfiteater(UR_OBJECT user, int force, char *inpstr);
void reset_alarm();
void setup_readmask(fd_set *mask);
void accept_connection(int lsock,int num);
void disconnect_user(UR_OBJECT user, int message, char *dovod);
int get_charclient_line(UR_OBJECT user, char *inpstr, int len);
void terminate(char *str);
void clear_words();
void login(UR_OBJECT user,char *inpstr);
int wordfind(char *inpstr);
void prompt(UR_OBJECT user);
void echo_on(UR_OBJECT user);
void show_recent_tells(UR_OBJECT user);
void show_recent_notifies(UR_OBJECT user);
int misc_ops(UR_OBJECT user, char *inpstr);
int site_banned(char *site);

char *check_shortcut(UR_OBJECT user);
UR_OBJECT get_user(char *name);
UR_OBJECT get_user_exact(char *name);
UR_OBJECT get_user_by_sock(int sock);
UR_OBJECT get_user_in_room(char *name,UR_OBJECT user);
void decrease_pp(UR_OBJECT user, int value, int dynamic);
int check_ignore_user(UR_OBJECT user,UR_OBJECT target); 

void strtolower(char *str);
void parse_init_section();
void parse_rooms_section();
void load_rooms_desc(UR_OBJECT user);

int onoff_check(char *wd);
int get_level(char *name);
int yn_check(char *wd);
void talker_shutdown(UR_OBJECT user,char *str,int reboot);
void level_log(char *str);
int load_user_password(UR_OBJECT user);
void who(UR_OBJECT user,int people);
char *parse_who_line(UR_OBJECT u,char *line,int wizzes,int users,int userlevel);
void newwho(UR_OBJECT user,char *username);
void testwho(UR_OBJECT user);
void customexamine(UR_OBJECT user,char *username,int testall);
char *parse_ex_line(UR_OBJECT u,char *line,int profile,int userlevel,int oflajn);
int search_success(UR_OBJECT user,UR_OBJECT u,char *lookat,int words);
void show_timeouts(UR_OBJECT user);
void show_version(UR_OBJECT user);
void system_details(UR_OBJECT user,int typ);
void attempts(UR_OBJECT user);
void connect_user(UR_OBJECT user);
int load_user_details(UR_OBJECT user);
int load_user_details_old(UR_OBJECT user);
int get_age(int agecode);
void echo_off(UR_OBJECT user);
int save_user_details(UR_OBJECT user,int save_current);
int save_user_details_old(UR_OBJECT user,int save_current);
void send_mail(UR_OBJECT user,char *to,char *ptr);
void endstring(char *inpstr);
void colour_com_strip(char *str);
void force_language(char *str,int lang,int stripc);
void destruct_user(UR_OBJECT user);
void look(UR_OBJECT user);
int sk(UR_OBJECT user);
void do_db_backup(int full);
void misc_stuff(int level);
char *langselect(UR_OBJECT user,char *slovak,char *english);
int check_notify(UR_OBJECT user);
void who_from_notify_is_online(UR_OBJECT user);
void hint(UR_OBJECT user, int ihned);
UR_OBJECT check_double_identity(UR_OBJECT user);
UR_OBJECT check_double_real_identity(UR_OBJECT user);
int has_unread_mail(UR_OBJECT user);
int check_notify_user(UR_OBJECT user,UR_OBJECT target);
void toggle_prompt(UR_OBJECT user,char *inpstr);
void revtell(UR_OBJECT user,char *inpstr);
extern void writecent(UR_OBJECT user, char *txt);
void destroy_user_clones(UR_OBJECT user);
void reset_access(RM_OBJECT rm);
void editor(UR_OBJECT user,char *inpstr);
void adv_edit(UR_OBJECT user);
char *adv_e_str(UR_OBJECT user);
void wipe_user(char meno[]);
void delete_user(UR_OBJECT user,int this_user);
void posielanie(UR_OBJECT user);
int zmenheslo(UR_OBJECT user,char *inpstr,int wizpass);
void police_freeze(UR_OBJECT user,int priority);
void send_mailinglist_request(UR_OBJECT user);
int double_fork(); 
void write_board(UR_OBJECT user,char *inpstr,int done_editing);
void smail(UR_OBJECT user,char *inpstr,int done_editing);
void enter_profile(UR_OBJECT user,int done_editing);
void write_joke(UR_OBJECT user,char *inpstr,int done_editing);
void vote(UR_OBJECT user, char *inpstr, int done_editing,int zedit);
void votenew(UR_OBJECT user);
int note_vote(UR_OBJECT user);
void editor_done(UR_OBJECT user);
void clear_revbuff(RM_OBJECT rm);
void send_email(UR_OBJECT user,char *to, char *message);
void forward_email(char *name,char *from, char *from2, char *message);
void cls(UR_OBJECT user, int howmuch);
void strtoupper(char *str);
void exec_com(UR_OBJECT user,char *inpstr);
int has_room_access(UR_OBJECT user,RM_OBJECT rm);
void tell(UR_OBJECT user,char *inpstr,int reply);
void to_user(UR_OBJECT user,char *inpstr);
void pemote(UR_OBJECT user,char *inpstr);
void move(UR_OBJECT user);
void kill_user(UR_OBJECT user, char *inpstr);
void wake(UR_OBJECT user,char *inpstr);
void send_hug(UR_OBJECT user,char *inpstr);
void send_bomb(UR_OBJECT user);
void send_kiss(UR_OBJECT user,char *inpstr);
void fight_user(UR_OBJECT user);
void fight_brutalis(UR_OBJECT user);
void notify_user(UR_OBJECT user);
void pictell(UR_OBJECT user,char *inpstr);
void quit_user(UR_OBJECT user,char *inpstr);
void toggle_mode(UR_OBJECT user);
void say(UR_OBJECT user,char *inpstr,int comm);
void shout(UR_OBJECT user,char *inpstr);
void emote(UR_OBJECT user,char *inpstr);
void semote(UR_OBJECT user, char *inpstr);
void echo(UR_OBJECT user, char *inpstr);
void go(UR_OBJECT user);
void set_desc(UR_OBJECT user, char *inpstr);
void set_iophrase(UR_OBJECT user, char *inpstr);
char *expand_outphr(UR_OBJECT user,RM_OBJECT destination);
void set_room_access(UR_OBJECT user);
void letmein(UR_OBJECT user);
void invite(UR_OBJECT user);
void set_topic(UR_OBJECT user, char *inpstr);
void bcast(UR_OBJECT user, char *inpstr,int done_editing);
void who_alt1(UR_OBJECT user);
void who_alt2(UR_OBJECT user);
void who_alt3(UR_OBJECT user);
void who_alt4(UR_OBJECT user);
void who_alt5(UR_OBJECT user);
void who_alt6(UR_OBJECT user);
void who_alt7(UR_OBJECT user);
void who_alt8(UR_OBJECT user);
void who_alt9(UR_OBJECT user);
void help(UR_OBJECT user,int flag);
void shutdown_com(UR_OBJECT user);
void read_board(UR_OBJECT user);
void wipe_board(UR_OBJECT user);
void search_boards(UR_OBJECT user);
void review(UR_OBJECT user, char *inpstr);
void status(UR_OBJECT user);
void mymail(UR_OBJECT user,int self);
void rmail(UR_OBJECT user);
void dmail(UR_OBJECT user);
int mail_from(UR_OBJECT user, int echo);
void examine(UR_OBJECT user);
void analyze(UR_OBJECT user);
void rooms(UR_OBJECT user);
void change_pass(UR_OBJECT user);
void autopromote(UR_OBJECT user,int promote);
void promote(UR_OBJECT user,char *inpstr);
void demote(UR_OBJECT user,char *inpstr);
void listbans(UR_OBJECT user);
void ban(UR_OBJECT user, char *inpstr);
void unban(UR_OBJECT user);
void visibility(UR_OBJECT user,int vis);
void hide(UR_OBJECT user);
void site(UR_OBJECT user);
void wizshout(UR_OBJECT user,char *inpstr);
void muzzle(UR_OBJECT user);
void unmuzzle(UR_OBJECT user);
void show_map(UR_OBJECT user);
void minlogin(UR_OBJECT user);
void toggle_charecho(UR_OBJECT user,char *inpstr);
void clearline(UR_OBJECT user);
void change_room_fix(UR_OBJECT user,int fix);
void viewlog(UR_OBJECT user, char *inpstr);
void account_request(UR_OBJECT user,char *inpstr);
void revclr(UR_OBJECT user);
void create_clone(UR_OBJECT user);
void destroy_clone(UR_OBJECT user);
void myclones(UR_OBJECT user);
void allclones(UR_OBJECT user);
void clone_switch(UR_OBJECT user);
void clone_say(UR_OBJECT user,char *inpstr);
void clone_hear(UR_OBJECT user);
void afk(UR_OBJECT user,char *inpstr);
void exec_command(UR_OBJECT user,char *inpstr);
void toggle_colour(UR_OBJECT user);
void suicide(UR_OBJECT user);
void reboot_com(UR_OBJECT user);
void check_messages(UR_OBJECT user);
void send_sos(UR_OBJECT user,char *inpstr);
void write_noticeboard(UR_OBJECT user, int what, int done_editing);
void read_notices(UR_OBJECT user);
void send_to_jail(UR_OBJECT user, char *inpstr);
void cooltalk(UR_OBJECT user,char *inpstr);
void cooltopic(UR_OBJECT user,char *inpstr);
void cdesc(UR_OBJECT user,char *inpstr);
void fortune_cookies(UR_OBJECT user);
void think(UR_OBJECT user,char *inpstr);
void cow(UR_OBJECT user);
void sing(UR_OBJECT user,char *inpstr);
void kidnap(UR_OBJECT user);
void follow(UR_OBJECT user, int unfol);
void ranks(UR_OBJECT user);
void secho(UR_OBJECT user,char *inpstr);
void read_joke(UR_OBJECT user);
void delete_joke(UR_OBJECT user);
void sign(UR_OBJECT user,char *inpstr);
void tellall(UR_OBJECT user,char *inpstr);
void commands(UR_OBJECT user,int engl);
void logtime(UR_OBJECT user, char *inpstr);
void info_users(UR_OBJECT user);
void info(UR_OBJECT user, int typ);
void pecho(UR_OBJECT user, char *inpstr);
void whois(UR_OBJECT user, char *inpstr);
void shset(UR_OBJECT user);
void set(UR_OBJECT user,char *inpstr);
void fmail(UR_OBJECT user,int what);
void poetizuj(UR_OBJECT user);
void revshout(UR_OBJECT user,char *inpstr);
void revbcast(UR_OBJECT user,char *inpstr);
void revsos(UR_OBJECT user,char *inpstr);
void revwizshout(UR_OBJECT user, char *inpstr);
void last(UR_OBJECT user);
void who_from(UR_OBJECT user, char *inpstr);
void insult_user(UR_OBJECT user);
void chname(UR_OBJECT user,char *inpstr);
void power_points(UR_OBJECT user);
void winners(UR_OBJECT user,int type_def,int pocet);
void add_point(UR_OBJECT user,int type,int amount,int fuel);
void weather(UR_OBJECT user, char *inpstr);
void skript(UR_OBJECT user);
void kick(UR_OBJECT user);
void ignore(UR_OBJECT user, char *inpstr);
void join(UR_OBJECT user);
void nuke_user(UR_OBJECT user);
extern void piskvorky(UR_OBJECT user, char *inpstr);
void alarm_clock(UR_OBJECT user);
void copies_to(UR_OBJECT user);
void save_users(UR_OBJECT user);
extern void play_hangman(UR_OBJECT user, char *inpstr);
void wizzes(UR_OBJECT user);
extern void play_geo(UR_OBJECT user,char *inpstr);

void get_predmet(UR_OBJECT user);
void put_predmet(UR_OBJECT user);
void loose_predmets(UR_OBJECT user);
void carry_refresh(UR_OBJECT user);
char *farba_p(int vec);
void create_predmet(UR_OBJECT user);
void give_predmet(UR_OBJECT user);
void dispose_predmet(UR_OBJECT user);
int forbidden(UR_OBJECT user,UR_OBJECT u,int vec);
void hurt(UR_OBJECT user,UR_OBJECT u,int vec);
void use_predmet(UR_OBJECT user,char *inpstr);
int convert_predmet(UR_OBJECT user,int spell);
CO_OBJECT create_convert();
void destruct_convert(CO_OBJECT con);
void throw_predmet(UR_OBJECT user);
void zobraz_predmety(UR_OBJECT user,char *inpstr);
P_OBJECT create_vec();
void destruct_vec(P_OBJECT vec);
void load_and_parse_predmets(UR_OBJECT user);
char *fetchstring(char *str);
char *parse_phrase(char *str,UR_OBJECT user,UR_OBJECT u,RM_OBJECT rm,int whichtype);
void do_alt_funct(UR_OBJECT user,int vec);
void show_pict_on_event(UR_OBJECT user,int event,int vec,int newline);
void predmet_write(UR_OBJECT user,int done_editing);
int is_affected(UR_OBJECT user,int flag);

void call_user(UR_OBJECT user);
void p_u_prikaz(UR_OBJECT user, int pridaj,char *inpstr);
extern void lod(UR_OBJECT user);
extern void lab(UR_OBJECT user);
void check_ident(UR_OBJECT user);
void update_web(UR_OBJECT user);
void gossip(UR_OBJECT user,char *inpstr);
void gemote(UR_OBJECT user,char *inpstr);
void revgossip(UR_OBJECT user, char *inpstr);
void quest_command(UR_OBJECT user,char *inpstr);
void sclerotic(UR_OBJECT user);

extern void shprn2(UR_OBJECT user,UR_OBJECT u);
extern void vynuluj_lod(UR_OBJECT user);
extern void shprn(UR_OBJECT user);
extern int shtest(UR_OBJECT user, int x, int y);
extern int shreset(UR_OBJECT user);
extern void shset(UR_OBJECT user);
extern int shsunken(UR_OBJECT user, int x,int y);
extern int shsink(UR_OBJECT user, int x,int y, int count);

void turn_resolver(UR_OBJECT user);
void rules_faq_web();
void save_topic();
void send_forward_email(char *send_to, char *mail_file);
void show_user(UR_OBJECT user,char fajl[]);
int shcount(UR_OBJECT user, int x, int y);

extern void fly_write(UR_OBJECT user,int body,int fuel);
extern void stavhry(UR_OBJECT user1,UR_OBJECT user2);
extern void stavhryg(UR_OBJECT user1,UR_OBJECT user2);

void amfiteater_web();
void send_copies(UR_OBJECT user, char *ptr);
extern void nastav_hru(UR_OBJECT user);

extern void je5b(UR_OBJECT user,int n);
extern void je4b(UR_OBJECT user,int n);
extern void je5(UR_OBJECT user,int x, int y, char znak);
extern void je4(UR_OBJECT user,int x, int y, char znak);

void toggle_ignall(UR_OBJECT user);
void toggle_ignshout(UR_OBJECT user);
void toggle_igngossip(UR_OBJECT user);
void toggle_igntell(UR_OBJECT user);
void toggle_ignfun(UR_OBJECT user);
void toggle_ignportal(UR_OBJECT user);
void toggle_ignsys(UR_OBJECT user);
void ignore_user(UR_OBJECT user);
void set_homepage(UR_OBJECT user,char *inpstr);
void jokeboard_web();
void zober_predpoved(UR_OBJECT user, int force);
void webcicni(char host[], char dokument[], char filename[]);
void parsni_pocasie(char vstup[], char vystfile[]);
int connecthost(char serveridlo[], int port);
int arena_check(UR_OBJECT user);
void revquest(UR_OBJECT user,char *inpstr);
int detect_user(char *inpstr,UR_OBJECT except);
void move_user(UR_OBJECT user,RM_OBJECT rm,int teleport);
int move_vehicle(UR_OBJECT user,RM_OBJECT newroom,int quiet);
int get_vehicle(UR_OBJECT user);
void rename_user_on_list(char meno[], char novemeno[]);
void help_commands(UR_OBJECT user,int engl);
void help_credits(UR_OBJECT user);

void ban_site(UR_OBJECT user);
void ban_user(UR_OBJECT user, char *inpstr);
void unban(UR_OBJECT user);
void unban_site(UR_OBJECT user);
void unban_user(UR_OBJECT user);
void otravuj_usera_s_mailinglistom(UR_OBJECT user);
void check_reboot_shutdown();
void check_idle_and_timeout();
void udalosti();
void check_cimazacat();
void check_predstavenie();
void check_security();
void dopln_predmety();
void do_predmety(int user_trigged);
int dec_dur(int olddur,int amount);
int abs_dur(int dur);
void do_userhands();
void hesh(RM_OBJECT rm);
void do_tunel(RM_OBJECT room);
void do_brutalis(UR_OBJECT user);
void do_attack(UR_OBJECT user);
RM_OBJECT random_room(RM_OBJECT rm,int ajprivat);
void zapis_statistiku();
void check_email_arrival();
void check_web_board();
void play_it();
void play_on();
void play_end();
void vypis_predstavenia(char riadok[]);
void who_for_web();
void toggle_ignword(UR_OBJECT user, char *inpstr);
void quest_logger(char *retazec);
int count_lines(char *filename);
extern char *get_hang_word(char *aword);
char *zobraz_datum(time_t *raw, int typ);
int jdb_zarad(int co, int uid, int wizzid, char *dovod);
int jdb_vyrad(int co, int uid);
char *jdb_info(int co, char *username);
char *jdb_wizz(int co, char *username);
extern char *sstrncpy(char *dst, char *src, long len);
int check_iophrase(char *inpstr);
int check_redir(UR_OBJECT user);
void idletime(UR_OBJECT user);
void guest_command(UR_OBJECT user, char *inpstr);
void icqpage(UR_OBJECT user, char *inpstr);
void send_icq_page(UR_OBJECT user, int icq_num, char *fname);
void finger(UR_OBJECT user, char *inpstr);
int user_banned(char *name);
void resc_load();
void resc_save();
void record_history(UR_OBJECT user, char *str);
void view_history(UR_OBJECT user);
char *real_auth(UR_OBJECT gdo);
void every_5min();
void do_burka();
void every_min();
void daily();
void ban_site_for_newuser(UR_OBJECT user);
void unban_site_for_newuser(UR_OBJECT user);
int newuser_siteban(char *site);

RN_OBJECT create_remote();
int init_remote_connections(UR_OBJECT user);
int connect_to_site(UR_OBJECT user, RN_OBJECT remote,int slot);
void remote_close(UR_OBJECT user,int i);
int exec_remote_com(UR_OBJECT user, char *inpstr);
void links(UR_OBJECT user);
void timeout_rem();
void remote_connect(UR_OBJECT user);
void remote_disconnect(UR_OBJECT user);
void view_remote(UR_OBJECT user);
void edit_remote_servers(UR_OBJECT user);
//void check_anti_idle();
int UPDATE_FDS();
int ADD_FDS();
void zazabuj(char *inpstr, int ok);
void zaslepac(char *inpstr, int ok);
void remote_antiidle();
void do_events();
void check_web_commands();
void brutalis_wars();
char *get_web_input(UR_OBJECT user);
void do_webusers();
void kick_webusers();
void handle_webinput();
void write_web(int socket,char *str);
void eliminate_webuser(int socket);
int get_memory_usage(void);
int get_resmem_usage(void);
void do_nothing();
void connecthost_timeout();
void cmon_auth(UR_OBJECT gdo);
void check_ident_reply();
void clear_irc_words();
int irc_wordfind(char *inpstr);
void write_irc(UR_OBJECT user, char *text);
void irc_prikaz(UR_OBJECT user, char *inpstr);
void lynx(UR_OBJECT user);
void test_lynx_done();
void revirc_command(UR_OBJECT user, char *inpstr);
void record_irc(UR_OBJECT user, char *inpstr);
void load_irc_details(UR_OBJECT user);
void save_irc_details(UR_OBJECT user);
long filesize (char *path);
void irc(UR_OBJECT user);
void statline(UR_OBJECT user);
void init_statline(UR_OBJECT user);
void show_statline(UR_OBJECT user);
void text_statline(UR_OBJECT user, int crflag);
FILE *ropen (const char *path, const char *mode);
void obnov_statline_userof();
void setpp(UR_OBJECT user, int amount);
int charsavail(int fd);
BJ_GAME   create_blackjack_game(void);
void      show_blackjack_cards(UR_OBJECT,int,int);
int       check_blackjack_total(UR_OBJECT,int);
void vwrite_user(UR_OBJECT user, char *str, ...);
char *get_ircserv_name(char *id);
void boot_statline(UR_OBJECT user);
void poprehadzuj_prikazy(UR_OBJECT user);
void toggle_io(UR_OBJECT user);
void toggle_zvery(UR_OBJECT user);
void toggle_coltell(UR_OBJECT user);
void toggle_ignlook(UR_OBJECT user);
void logout_user(UR_OBJECT, char*);
void timeout_auth(); 
void kde(UR_OBJECT user);
/* void write_sock(int sock,char *str); */
/* void write_sock2(int sock,char *str,size_t count); */
ssize_t twrite(int fd, const void *buf, size_t count);
void write2sock_ex(UR_OBJECT user,int sock,char *str,size_t count);
size_t write2sock(UR_OBJECT user,int sock,const char *str,size_t count);
void twrite_timeout();
char *noyes(int numero);

void reversi_koniechry(UR_OBJECT user);
void reversi(UR_OBJECT user);
void reversi_pishelp(UR_OBJECT user);
void reversi_pisplan(UR_OBJECT user, UR_OBJECT sh_user);
void reversi_pisplan2(UR_OBJECT user, UR_OBJECT sh_user);
void reversi_pocitajznaky(UR_OBJECT user, int* pX, int* pO);
int reversi_testok(UR_OBJECT user,int y, int x);
int reversi_mozetahat(UR_OBJECT user);

void reversi_tah(UR_OBJECT user, int y, int x);

void miny_draw(UR_OBJECT user);
void miny_numbering(UR_OBJECT, int , int);
int miny_init(UR_OBJECT);
void miny_zero_show(UR_OBJECT, int, int);
void miny_done(UR_OBJECT);
void miny(UR_OBJECT, char *inpstr);

DAMA_OBJECT create_dama();
void destruct_dama(UR_OBJECT);
void dama_save(UR_OBJECT);
void dama_load(UR_OBJECT);
char *dama_saved_opponent(UR_OBJECT);
int check_dama_jump(DAMA_OBJECT,int,int,int,int);
void dama(UR_OBJECT);
void dama_stav(UR_OBJECT);
void dama_stav2(UR_OBJECT);
char *header(char *str);
void oline(UR_OBJECT);
void do_repository(char *subor, int typ);
void repository(char *name, int typ);
void rebirth(UR_OBJECT user);
void miny_placing(UR_OBJECT,int,int);
void wash_bell(char*);
void send_noticeboard_digest(char*);
void send_recent_tells(UR_OBJECT user, int sp);
void toggle_ignbeep(UR_OBJECT);
void games(UR_OBJECT user);
UR_OBJECT hra(UR_OBJECT user, int n);
void sms(UR_OBJECT user,int done_editing);
void send_sms(char *numero,char *str,int gate);
void forward_smail2sms(char *name,char *from,char *message);
void crash_smsnotice();
char *expand_gate(UR_OBJECT user,int inc_account,char *sendtonum);
void show_gates(UR_OBJECT user);
int get_gate_no(char *str);
int get_gate_maxchars(int gate_num);
int restrict_sms(char *str,int gate_num);
void statistic(UR_OBJECT user);
char *title(char *str,char *col);
int pohaluz(UR_OBJECT user);
void gold(UR_OBJECT user,int left,char *str);
void alter_maxtimeouts(int do_what);
void calendar(UR_OBJECT user);

void free_stuff();

extern CL_OBJECT create_clovece();
extern void destruct_clovece(UR_OBJECT);
extern void record_clovece(char *str,CL_OBJECT clovece);
extern void clovece_save(UR_OBJECT);
extern void clovece_load(UR_OBJECT);
extern char *clovece_saved_opponent(UR_OBJECT);
extern void add_cloveceplayer(UR_OBJECT,UR_OBJECT);
extern void hod_kockou(UR_OBJECT, int);
extern void clovece_next(UR_OBJECT);
//void clovece(UR_OBJECT,char*);

extern FR_OBJECT create_farar();
extern void destruct_farar(UR_OBJECT);
extern void add_fararplayer(UR_OBJECT,UR_OBJECT);
extern void farar_disp(UR_OBJECT,int);
extern void record_farar(char *str,FR_OBJECT farar);
extern void farar(UR_OBJECT,char*);

extern void log_game(char*);
 /* V) hmh, extern || neextern .. what's the difference ?? */

/***********************************************************
*************** EXTERNE  FUNKCIE ***************************
*************** (uplne na konci) ***************************
************************************************************/

int put_in_room(RM_OBJECT rm,int vec,int dur);
int remove_from_room(RM_OBJECT rm,int vec,int dur);
int is_in_room(RM_OBJECT rm,int vec);
int is_funct_in_room(RM_OBJECT rm,int fun);
int is_in_hands(UR_OBJECT u,int vec);
int is_funct_in_hand(UR_OBJECT u,int fun);
int is_free_in_room(RM_OBJECT rm);
int is_free_in_hands(UR_OBJECT u);
int put_in_hands(UR_OBJECT u,int vec,int dur);
int remove_from_hands(UR_OBJECT u,int vec,int dur);
void buy(UR_OBJECT user);
void sell(UR_OBJECT user);
int spravny_predmet(char *vec1);
int default_dur(int vec);

extern void banner(UR_OBJECT user, char *inpstr);
extern void tbanner(UR_OBJECT user,char *inpstr);
extern void sbanner(UR_OBJECT user,char *inpstr);
extern void magazin(UR_OBJECT user,char *inpstr);

extern void note(UR_OBJECT user,char *inpstr);
extern void dnote(UR_OBJECT user);
extern int spracuj_remote_vstup(UR_OBJECT user, char *inpstr);
extern void parse_remote_ident(UR_OBJECT user,char *rinpstr,int len,int wsock);
extern void parse_telnet_chars(UR_OBJECT user,char *inpstr, size_t len);
extern void spracuj_irc_vstup(UR_OBJECT user, char *inpstr);
extern void prihlas_irc(UR_OBJECT user);
extern void spell(UR_OBJECT user, char *inpstr);
extern UR_OBJECT leave_me_alone(UR_OBJECT user,UR_OBJECT u);
extern void show_kniha_kuziel(UR_OBJECT user,int dur);

extern char *crypt_slavko(char *crypt, char *salt);
extern char *md5_crypt(char *pw, char *salt);
extern void      destruct_blackjack_game(UR_OBJECT);
extern void      play_blackjack(UR_OBJECT);
extern void      clovece(UR_OBJECT, char*);

extern XA_OBJECT create_xannel();
extern void destruct_xannel(UR_OBJECT user);
extern void xsay(UR_OBJECT user,char *inpstr);
extern void record_xannel(XA_OBJECT xannel,char *str);
extern void add2xannel(UR_OBJECT user,UR_OBJECT u);
extern void xannel(UR_OBJECT user,char *inpstr);
extern void cmd_module(UR_OBJECT user, char *inpstr);

char *dbf_string(char *string);
/***********************************************************/


/********* konec headru... **************/
/* Aka sucha hlaska... To sa pise inak! Asi takto:
Vazene damy, vazeni pani! Dovolujeme si vam laskavo oznamit, ze mnozstvo
bytov, urcene pre tento header sa uz racilo minut a tak vam neostava nic ine,
ako stlacit ctrl+c a ponechat tento header svojmu osudu. Pevne verime, ze sa
vam toto stretnutie pacilo a ze ste sa milo pobavili. Ak nie, hlboko sa vam
ospravedlnujeme a slubujeme, ze nabuduce to bude zasa o nieco lepsie. Tesime
sa na najblizsie stretutie s vami, vase pismenka v headri... :> GAME OVER */

       /* GODa jeho Ridera, este tam ma aj pravopysne chibi! */
  /* To ne ja, to pismenka :>> (musim sa na daco vyhovorit, nie?) */   

/*.. Rozhostila se cernocerna tma. Osvetlene je jen okno Niny Alexandrovny ..*/
/* To si tam uz wacsju krawinu dat nemohol?! Koniec musi byt uplne krasny,
 stylovy a nie ako v americkych filmox! Napriklad takyto:

 ...oprel sa o pachole a pozrel smerom, kde stala predtym nasa ponorka. Na
 hladine ostala iba skvrna po nafte a tazko rozoznatelne plavajuce predmety.
 Ludia z posadky. Jeho oci, vzdy privrete, boli teraz uplne vytrestene a meravo
 hladeli na tu skazu. Zrazu sa mu z ust vyrynul pramienok krvi...
*/
