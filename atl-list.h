/*****************************************************************************
   	          -  - --=> ATLANTIS LISTS HEADER <=--- -  -
	             ged reedy four a phun-toastic staph!
******************************************************************************/ 

/* Wow, a toto su levely, vecna to tema ... */
 
char *level_name[]={ 
//"SLAVE","CITIZEN","SOLDIER","WARRIOR","SAGE","PRIEST","WIZARD","KING","GOD","GODBOT","*" };
"OTROK","OBCAN","VOJAK","BOJOVNIK","MUDRC","KNAZ","MAG","KRAL","BOH","SASO","OBCAN","*" };

int tlt4level[]={ 0,0,2,5,10,18,0,0,0,0 };

/* A prikazy, este vecnejsia tema .. dokedy sakra este budu pribudat ?! */
/* asi kym neupalime zdrojaky atlantisu.. */
/* Label 4 command editor, do not delete: <Beginning of commands section !@#$%> */

char *command[]={
"alarm",       "afk",         "allclones",   "analyze",     "buy",         
"bcast",       "ban",         "bomb",        "banner",      "bjack",       
"cls",         "charecho",    "clone",       "csay",        "chear",       
"clearline",   "colour",      "commands",    "ctalk",       "ctopic",      
"cdesc",       "cow",         "complaint",   "chname",      "copies",      
"create",      "call",        "calendar",    "client",      "comlevel",    
"clovece",     "desc",        "dmail",       "demote",      "destroy",     
"delete",      "djoke",       "dispose",     "doom",        "dnote",       
"dama",        "emote",       "echo",        "examine",     "excuse",      
"exec",        "from",        "fight",       "fix",         "fortune",     
"follow",      "faq",         "fmail",       "finger",      "faraon",      
"go",          "gold",        "godpriv",     "get",         "geo",         
"gemote",      "give",        "gossip",      "guest",       "games",       
"help",        "hug",         "hint",        "hangman",     "history",     
"hide",        "inphr",       "invite",      "invis",       "insult",      
"ignore",      "info",        "idletime",    "icq",         "irc",         
"jail",        "join",        "kill",        "kiss",        "kidnap",      
"kick",        "look",        "letmein",     "listbans",    "logtime",     
"last",        "lodicky",     "labyrint",    "links",       "lynx",        
"mode",        "move",        "muzzle",      "map",         "mymail",      
"myclones",    "macro",       "magazine",    "miny",        "news",        
"notify",      "notice",      "nuke",        "newban",      "newunban",    
"note",        "outphr",      "put",         "pemote",      "prompt",      
"private",     "people",      "profile",     "passwd",      "promote",     
"pictell",     "pomoc",       "prikazy",     "pecho",       "poet",        
"pp",          "play",        "piskvorky",   "public",      "predmety",    
"pridaj",      "quit",        "query",       "quest",       "read",        
"review",      "rmail",       "rooms",       "request",     "revclr",      
"reboot",      "recount",     "revtell",     "ranks",       "rjoke",       
"reply",       "rules",       "realuser",    "revshout",    "revwiz",      
"revgossip",   "remote",      "revirc",      "reversi",     "revbcast",    
"revsos",      "rebirth",     "shout",       "say",         "semote",      
"shutdown",    "search",      "status",      "smail",       "site",        
"system",      "switch",      "sell",        "suicide",     "sos",         
"suggestion",  "sing",        "secho",       "sign",        "set",         
"sbanner",     "script",      "sndproof",    "save",        "spell",       
"sms",         "statistic",   "sclerotic",   "tell",        "to",          
"topic",       "throw",       "time",        "think",       "tellall",     
"tbanner",     "talkers",     "use",         "unfollow",    "unfix",       
"unban",       "uptime",      "unmuzzle",    "uber",        "update",      
"version",     "vis",         "viewlog",     "vote",        "who",         
"write",       "wipe",        "wake",        "wizshout",    "wjoke",       
"wfrom",       "winners",     "weather",     "wizzes",      "xsay",        
"xannel",      "module",
"*" };

enum comvals {
ALARM,         AFK,           ALLCLONES,     ANALYZE,       BUY,           
BCAST,         BAN,           BOMB,          BANNER,        BJACK,         
CLS,           CHARECHO,      CLONE,         CSAY,          CHEAR,         
CLEARLINE,     COLOUR,        COMMANDS,      COOLTALK,      COOLTOPIC,     
CDESC,         COW,           COMPLAINT,     CHNAME,        COPIES,        
CREATE,        CALL,          CALENDAR,      CLIENT,        COMLEVEL,      
CLOVECE,       DESC,          DMAIL,         DEMOTE,        DESTROY,       
DELETE,        DJOKE,         DISPOSE,       DOOM,          DNOTE,         
DAMA,          EMOTE,         ECHO,          EXAMINE,       EXCUSE,        
EXEC,          FROM,          FIGHT,         FIX,           FORTUNE,       
FOLLOW,        FAQ,           FMAIL,         FINGER,        FARAON,        
GO,            GOLD,          GODPRIV,       GET,           GEO,           
GEMOTE,        GIVE,          GOSSIP,        GUEST,         GAMES,         
HELP,          HUG,           HINT,          HANGMAN,       HISTORY,       
HIDE,          INPHRASE,      INVITE,        INVIS,         INSULT,        
IGNORE,        INFO,          IDLETIME,      ICQ,           IRC,           
JAIL,          JOIN,          KILL,          KISS,          KIDNAP,        
KICK,          LOOK,          LETMEIN,       LISTBANS,      LOGTIME,       
LAST,          LODICKY,       LABYRINT,      LINKS,         LYNX,          
MODE,          MOVE,          MUZZLE,        MAP,           MYMAIL,        
MYCLONES,      MACROCMD,      MAGAZINE,      MINY,          NEWS,          
NOTIFY_CMD,    NOTICE,        NUKE,          NEWBAN,        NEWUNBAN,      
NOTE,          OUTPHRASE,     PUT,           PEMOTE,        PROMPT,        
PRIVCOM,       PEOPLE,        PROFILE,       PASSWD,        PROMOTE,       
PICTELL,       POMOC,         PRIKAZY,       PECHO,         POET,          
PP,            PLAY,          PISKVORKY,     PUBCOM,        PREDMETY,      
PRIDAJ,        QUIT,          QUERY_COM,     QUEST,         READ,          
REVIEW,        RMAIL,         ROOMS,         ACCREQ,        REVCLR,        
REBOOT,        RECOUNT,       REVTELL,       RANKS,         RJOKE,         
REPLY,         RULES,         REALUSER,      REVSHOUT,      REVWIZ,        
REVGOSSIP,     REMOTE,        REVIRC,        REVERSI,       REVBCAST,      
REVSOS,        REBIRTH,       SHOUT,         SAY,           SEMOTE,        
SHUTDOWN,      SEARCH,        STATUS,        SMAIL,         SITE,          
SYSTEM,        SWITCH,        SELL,          SUICIDE,       SOS,           
SUGGESTION,    SING,          SECHO,         SIGN,          SET,           
SBANNER,       SKRIPT,        SNDPROOF,      SAVE,          SPELL,         
SMS,           STATISTIC,     SCLEROTIC,     TELL,          TO_USER,       
TOPIC,         THROW,         TIME,          THINK,         TELLALL,       
TBANNER,       TALKERS,       USE,           UNFOLLOW,      UNFIX,         
UNBAN,         UPTIME,        UNMUZZLE,      UBER,          UPDATE,        
VER,           VIS,           VIEWLOG,       VOTE,          WHO,           
WRITE,         WIPE,          WAKE,          WIZSHOUT,      WJOKE,         
WFROM,         WINNERS,       WEATHER,       WIZZES,        XSAY,          
XANNEL,        CMD_MODULE,
};
int com_num;

int com_level[]={
CIT, CIT, KIN, KIN, SOL, 
KIN, KIN, KIN, PRI, GOD, 
NEW, NEW, KIN, KIN, KIN, 
KIN, NEW, NEW, PRI, PRI, 
PRI, SAG, CIT, KIN, SOL, 
GOD, CIT, CIT, NEW, GOD, 
CIT, NEW, CIT, KIN, KIN, 
GOD, GOD, KIN, PRI, SOL, 
SOL, SOL, WAR, CIT, CIT, 
GOD, CIT, SOL, KIN, SOL, 
WAR, NEW, SOL, GOD, SOL, 
CIT, SOL, GOD, CIT, WAR, 
SOL, SOL, SOL, GOD, CIT, 
NEW, CIT, CIT, CIT, KIN, 
SOL, SOL, SAG, PRI, SAG, 
CIT, KIN, KIN, SOL, SAG, 
KIN, WAR, KIN, SOL, PRI, 
SAG, NEW, CIT, KIN, GOD, 
CIT, SAG, SAG, GOD, WAR, 
SOL, KIN, KIN, CIT, SOL, 
KIN, SOL, CIT, WAR, NEW, 
CIT, KIN, KIN, GOD, GOD, 
SOL, SOL, CIT, WAR, SOL, 
SAG, KIN, CIT, CIT, KIN, 
SAG, NEW, NEW, SAG, SAG, 
NEW, CIT, SOL, SAG, KIN, 
KIN, NEW, GOD, CIT, CIT, 
CIT, CIT, NEW, NEW, WAR, 
GOD, KIN, CIT, NEW, CIT, 
CIT, NEW, KIN, CIT, KIN, 
SOL, SAG, SAG, CIT, SOL, 
KIN, GOD, CIT, NEW, WAR, 
GOD, WAR, NEW, CIT, KIN, 
KIN, KIN, SOL, NEW, NEW, 
SOL, SOL, PRI, SAG, NEW, 
PRI, KIN, KIN, KIN, PRI, 
SOL, KIN, KIN, CIT, NEW, 
SAG, WAR, NEW, SOL, KIN, 
PRI, SOL, CIT, WAR, KIN, 
KIN, KIN, KIN, KIN, GOD, 
CIT, PRI, KIN, SOL, NEW, 
CIT, CIT, SOL, KIN, CIT, 
KIN, CIT, WAR, NEW, SOL, 
SOL, GOD,
};

char *command_sk[]={
"alarm",       "adresa",      "amfiteater",  "analyzuj",    "aktualizuj",  
"basen",       "bomba",       "bozkaj",      "bozska",      "chod",        
"cas",         "caruj",       "casteotazky", "casbehu",     "celkovycas",  
"citaj",       "cistilinku",  "clovece",     "commands",    "cpostu",      
"daj",         "dama",        "degraduj",    "doom",        "dolezitasp",  
"dopyt",       "dukaty",      "dzopakuj",    "echo",        "farba",       
"faraon",      "faq",         "fhovor",      "fpopis",      "fpostu",      
"ftema",       "hry",         "geo",         "tja",         "hovor",       
"heslo",       "historia",    "hladaj",      "hlasuj",      "hodnosti",    
"host",        "ignoruj",     "icq",         "info",        "irc",         
"irczopakuj",  "ja",          "k",           "kalendar",    "kanal",       
"khovor",      "kignoruj",    "kja",         "klient",      "klon",        
"knapis",      "koniec",      "kopni",       "kopie",       "kric",        
"kravina",     "kto",         "kup",         "kvyjadri",    "kwizardom",   
"kzopakuj",    "labyrint",    "lodicky",     "ludia",       "makro",       
"magazin",     "makro",       "mapa",        "miestnosti",  "miny",        
"mod",         "mojaposta",   "mojeklony",   "mudruj",      "mysli",       
"nastav",      "nanuk",       "napis",       "naraznik",    "navstiv",     
"navrhni",     "necinnost",   "nesleduj",    "novinky",     "nvtip",       
"odpovedz",    "obimaj",      "obesenec",    "obrazok",     "ocko",        
"odhlaseni",   "odfixuj",     "odmlc",       "odozvaznak",  "odstran",     
"ospravedln",  "povedz",      "pecho",       "pis",         "piskvorky",   
"pja",         "pnapis",      "pocasie",     "poloz",       "pomoc",       
"help",        "popis",       "postaod",     "pouzi",       "povol",       
"povolnovych", "povys",       "pozri",       "poznamka",    "pozvi",       
"pp",          "ppostu",      "preskumaj",   "pravidla",    "predaj",      
"predmety",    "premenuj",    "prepni",      "prepocitaj",  "presun",      
"prikazy",     "pridaj",      "priklevel",   "prinut",      "pripoj",      
"profil",      "prstuj",      "pustimadnu",  "pvyjadri",    "pvsetkym",    
"pzopakuj",    "registruj",   "restart",     "reversi",     "lynx",        
"somprec",     "samovrazda",  "schovajsa",   "kecho",       "skratka",     
"sklerotik",   "skripty",     "skutocny",    "sleduj",      "sms",         
"sos",         "spravcovia",  "spievaj",     "spojenia",    "sprivatni",   
"stav",        "staznost",    "statistika",  "sutaz",       "system",      
"soszopakuj",  "tema",        "taraj",       "tabula",      "tip",         
"tlachadla",   "tzopakuj",    "upozorni",    "uber",        "ukazlog",     
"uloz",        "umlc",        "unes",        "uraz",        "uvazni",      
"vyjadri",     "verzia",      "vitazi",      "vstupnafr",   "vsetkyklony", 
"vtipy",       "vycisti",     "vypni",       "vystupnafr",  "vytvor",      
"vzduchotes",  "wzopakuj",    "zober",       "zahod",       "zadresy",     
"zafixuj",     "zakaz",       "zakaznovych", "zavrazdi",    "zaznam",      
"zlikviduj",   "zmaz",        "zmazhovor",   "zmlat",       "znevidit",    
"znic",        "znovuzrod",   "zober",       "zobrazzak",   "zobud",       
"zopakuj",     "zpostu",      "zpoznamku",   "zverejni",    "zviditelni",  
"zvtip",       "*" };

int sk_to_en_com[]={
  0, 164, 126,   3, 199, 
124,   7,  83,  57,  55, 
187, 179,  51, 196,  89, 
134,  15,  30,  17, 136, 
 61,  40,  33,  38,   5, 
132,  56, 154,  42,  16, 
 54,  51,  18,  20,  52, 
 19,  64,  59,  60, 158, 
118,  69, 161, 203, 143, 
 63,  75,  78,  76,  79, 
152,  41, 184,  27, 215, 
 13,  14, 159,  28,  12, 
175, 131,  85,  24, 157, 
 21, 204,   4, 159, 208, 
148,  92,  91, 116, 101, 
102, 101,  98, 137, 103, 
 95,  99, 100,  49, 188, 
174, 107,   8, 114,  81, 
170,  77, 193, 104, 209, 
145,  66,  68, 120,   9, 
 90, 194, 197,  11,  35, 
 44, 183, 123, 205, 127, 
113, 190, 212, 112, 121, 
 65,  31,  46, 192, 195, 
109, 119,  86, 110,  72, 
125, 163,  43, 146, 167, 
129,  23, 166, 141,  96, 
122, 130,  29,  45, 151, 
117,  53,  87, 113, 189, 
142, 138, 140, 153,  94, 
  1, 168,  70, 172,  26, 
182, 176, 147,  50, 180, 
169, 213, 171,  93, 115, 
162,  22, 181, 133, 165, 
155, 185,  62, 173,  67, 
191, 150, 105, 198, 202, 
178,  97,  84,  74,  80, 
 41, 200, 211,  71,   2, 
144,  10, 160, 111,  25, 
177, 149,  58, 186, 210, 
 48,   6, 108,  82, 106, 
 37, 206, 139,  47,  73, 
 34, 156,  58,  88, 207, 
135,  32,  39, 128, 201, 
 36, 
};

/* Label 4 command editor, do not delete: <End of commands section !@#$%> */
/*
Colcode values equal the following:
RESET,BOLD,UNDERLINE,BLINK,REVERSE

Foreground & background colours in order..
BLACK,RED,GREEN,YELLOW/ORANGE,
BLUE,MAGENTA,TURQUIOSE,WHITE
Plus pipanie:
BEEP 
*/

/* S farbickami je Atlantis ovela krajsi ... btw. co mate do paze vsetci
   proti tej ruzovej?! Sak to je tak kraaaasna farba, vsak zlatko!? .. :))) */
/* no, samozrejme niektori (Virgo) si myslia pravy opak o farbach, ale my
   nemozeme za to ze niekto ma ciernobiele alebo dokonca zelene zabove
   uxylky, vsakaaaano :> */ /* Nahodou, Zaba je zelena len po opici :>>> */
/* Zaba sux ... ona je sice "akoze" taka pekna, ale je tlsta a ked bude
   z nej stara skatula, tak bude tucna jak opaxa ... pfuj! To len teraz sa
   zda ze take peknucke, ale ked to bude stare tak potom uvidime co to
   dokaze :>>> */

char *colcom[NUM_COLS]={ 
"RS","OL","UL","LI","RV",
"FK","FR","FG","FY", 
"FB","FM","FT","FW", 
"BK","BR","BG","BY", 
"BB","BM","BT","BW", 
"HK","HR","HG","HY", 
"HB","HM","HT","HW", 
"DK","DR","DG","DY", 
"DB","DM","DT","DW", 
"LB" 
}; 
 
char *colcode[NUM_COLS]={ 
/* Standard stuff */ 
"\033[0m", "\033[1m", "\033[4m", "\033[5m", "\033[7m", 
/* Foreground colour */ 
"\033[30m","\033[31m","\033[32m","\033[33m", 
"\033[34m","\033[35m","\033[36m","\033[37m", 
/* Background colour */ 
"\033[40m","\033[41m","\033[42m","\033[43m", 
"\033[44m","\033[45m","\033[46m","\033[47m", 
/* Highlighted foreground */
"\033[1m\033[30m","\033[1m\033[31m","\033[1m\033[32m","\033[1m\033[33m", 
"\033[1m\033[34m","\033[1m\033[35m","\033[1m\033[36m","\033[1m\033[37m", 
/* Dark foreground (autoOLreset) */
"\033[0m\033[30m","\033[0m\033[31m","\033[0m\033[32m","\033[0m\033[33m", 
"\033[0m\033[34m","\033[0m\033[35m","\033[0m\033[36m","\033[0m\033[37m", 
/* James Bond */ 
"\007",   /* To nieje ziaden James Bond, to je BEEEP !!!!!!!!!! */

};
 
#define JAMES_BOND 007    /* TOTO je James Bond !!!!!!!! :>>> */
			  /* Uaaxxx, World Is Not Enough R00LZ! */
/* Rider, ale tej Xudej si to tam nemusel napisat vtedy :>>>>>> */
/* Ale nezavid! Ty by si sa bal... Meeting you with a view to a kill... */
/* Ja by som sa bal, ale ved to mame za jedna :>> Hnuuuusna Xuda! :> Este
   keby aj databazove systemy, to by bol cucipop! ... */
   
  /* ^^^^^^^ AHAAAA, TAKZE XUDAAAA!! No teda ta zenska ma sklamala az to
     pekne nieje ... a DBS tak lahko nebudu!! XUDA!XUDA!XUDA!!
     Cize: XUDA NO MORE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     Spaky, to si pisal TY?! A preco si sa potom prihlasi na sfocku k xudej?
     Ocuvaj, ze ty po nej este stale ides?! Ja som ta prekukol! :> Teda
     Slafko, vies co - ved ona ma decko, mozno aj dve, a ty .. no fuj! :)
  */
 
/* Codes used in a string to produce the colours when prepended with a '~' */ 
/* Sem by sa dalo este vselico pridat, ale ... no nexame to na novu verziu */
/* Nedalo, dal som to prec, lebo s tym Milo blbol - s tym ~CL, heh :>
   teda ne ze by som s tym ja neblbol, ale ja som GOD a Milo len priest :>
   No tak fidis... to mash za to, ze vymyslas xnupoviny :> */
/* Ale kto mohol vediet ze Milo to bude vidiet ako to ja pisem a potom to
   zneuzivatj?! */

char *usercols[16]={   /* pastelky ocislovane podla textoveho modu */
"",
"\033[0m\033[34m","\033[0m\033[32m","\033[0m\033[35m","\033[0m\033[31m",
"\033[0m\033[36m","\033[0m\033[33m","\033[0m\033[37m","\033[1m\033[30m",
"\033[1m\033[34m","\033[1m\033[32m","\033[1m\033[35m","\033[1m\033[31m",
"\033[1m\033[36m","\033[1m\033[33m","\033[1m\033[37m" }; //zwj

char *usercolcodes[16]={
"",
"~RS~FB","~RS~FG","~RS~FM","~RS~FR",
"~RS~FT","~RS~FY","~RS~FW","~OL~FK",
"~OL~FB","~OL~FG","~OL~FM","~OL~FR",
"~OL~FT","~OL~FY","~OL~FW" };

int defaultcols[7]={ 14,12,5,15,15,2,15 };
 
char *month[12]={ 
"Januar","Februar","Marec","April","Maj","Jun", 
"Jul","August","September","Oktober","November","December" 
}; 
 
char *day[7]={ 
"Nedela","Pondelok","Utorok","Streda","Stvrtok","Piatok","Sobota" 
}; 

int cal_days[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char *cal_daynames[7]={ "Pon","Uto","Str","Stv","Pia","Sob","Ned" };
 
/* hehe, fajn - ked raz tieto switche pretiekli, tak to tam hadzalo nadafky,
   kedze boli hned za tym v pamati, cize asi tak, ze:
   "CHARECHO is turned kokot"
   :>>> [sorry za vyraz :] */ 

/* 
char *noyes1[]={ " NO","YES" }; 
char *noyes2[]={ "NO ","YES" }; 
char *offon[]={ "OFF","ON " }; 
*/
/* NA LOVENSKU PO LOWENSKI! */
/* Lewinski ? */
char *no_or_yes[]={ "NIE","ANO" }; 
char *offon[]={ "VYP","ZAP" }; 
char *teamcolor[]={ "","~OL~FB","~OL~FR","~OL~FG","~OL~FK" }; 
  
/* These MUST be in lower case - the contains_swearing() function converts 
   the string to be checked to lower case before it compares it against 
   these. Also even if you dont want to ban any words you must keep the 
   star as the first element in the array. Doplnte podla potreby... ;) */ 

/* .... asi sem o xvilu pridam aj slovo "muslicka" :>>>
   No a co mash proti skeblam??? :>> Nahodof, taka muslicka.. cecece :> */
/* No fuuuuj, stare hnusne skarede spraxnivene skeble .. si predstava aku
   ju moze mat taka stara baronka .. no fuj! Ja toho Evzenka nexapem .. :( */   
/* HNUUUUUUUUUSNYYYYYYY EVZENKO!!!!!!!!!*/   
   
char *swear_words[]={ 
"fuck","shit","cunt","bitch","kokot","kurva","pica","pice","jebat","jebak","chuj",
"whore","hovno", "cock", "pici", "picu", "jebly", "picovina", "pojeb",
"jebnut","jeblina", "jeblost", "jebem", "jebne", "jebk", "jeba", 
"curak", "keket", "kokkot", "kokoot", "kurv", "pico","jebe",
"picovina","k o k o t","k0k0t","kok0t","k0kot","k 0 k 0 t",
"talker.sk","t a l k e r.sk","www.talker.",".talker.",
"*" };                                                 
char *exceptions[]={
"pica","opica","pica","slepica","pica","capica","pica","spica","chuj","sprchuj",
"picu","topicu","chuj","neechuj","chuj","krachuj","chuj","dychuj",
"*","*" };


char *advert_words[]={"~rs","~ol","~ul","~li","~rv",
"~fk","~fr","~fg","~fy","~fb","~fm","~ft","~fw",
"~dk","~dr","~dg","~dy","~db","~dm","~dt","~dw",
"~hk","~hr","~hg","~hy","~hb","~hm","~ht","~hw",
"~bk","~br","~bg","~by","~bb","~bm","~bt","~bw",
"~lb", "~~", "*"};

/* Autorizacia usera... */
struct timeval timeout={10, 0}; 

/* No a kam sa nam maju predmetiky doplnovat? Jednoduxe - SEM!: */
/* ... neviem sice preco akurat SEM, ale ... sem :)) */
char *predmet_do_miestnosti[]={
"namestie","afroditin_chram","krcma","trhovisko","academia","poseidonov_chram",
"pristav","arena","svatyna","amfiteater","*"
};

char *bylina_do_miestnosti[]={
"ostrov","mys_lorus","vodopad","strom_zivota","husty_les","majak",
"vchod_do_jaskyne","skala_samovrahov","jaskyna_pravdy","jazierko",
"luka","gejzir","udolie","riecka", "*"
};

char *predmet_na_brutalis[]={
"cierna_pust","vulkan","krater","hajik","zaliv","*"
};

/* PREDMETY:

- nahubok  - zamuzzluje usera na 5 min., a presunie nahubok k 'obeti' :)
- amulet   - dvojnasobne pridava PP-cky, znizuje sance vo fighte, po pouziti
             prida vsetky PPcky a zmizne. Tiez chrani pred vyciciavanim
             pp-ciek pomocou zubov :>
- mec      - zvysuje sance vo fighte. ;-))
- vino     - po 'poziti' budu userove hlasky trosicku upravene :)
- zuvacka  - po pouziti zacne user mlaskat, po dalsom pouziti zuvacku vyberie
             z huby a prestane mlaskat ;) Ak sa pouzije na userovi, zalepi
             ho to tak, ze sa nebude moct pohnut z miestnosti ! :>
- revolver - pre potreby ruskej rulety ... po pouziti user na seba vystreli -
             ci s nabojom, to zavisi od rand(). Potom revolver moze podat
             dalsiemu 'hracovi', etc ... :))))
- lamerizator- Spravi z usera lamera :> 
- zuby     - odcicia obeti niekolko pp-ciek ...  ;>>>>
             malo ked nieje obet afk, viac ked obet je afk. Ak je afk, tak 
             nehrozi nebezpecenstvo ze Ta obet nacapa. Niekedy zuby zostanu
             zaseknute v obeti :))
- debilizator- Spravi z usera debilneho ze bude debilneho kazde tretie
             debilneho :) Po chvili snad vyprcha...
- prehanadlo - Movuje usera z miestnosti-do-miestnosti, kazdych niekolko
               sekund ! ;)))
- kridla     - user moze lietat kam xce, len nie do private ! :>>
- spiritus   - ako vino, len dlhsi ucinok (240 sek), a lepsie prejavy ! :>
- bomba      - po pouziti sa inicializuje na 1 min., po podani (give) stale
               'tika', po polozeni sa zastavi. Po odpocitani 1 min. vybuchne
               majitelovi v rukach, a odhlasi ho. :>>
- lsd        - Zmeni 'pohlad na svet' ;>. Pouzije raz: kazde slovo bude mat
               inu fabu. Pouzije 2. krat: kazde pismeno bude mat inu farbu ;>
               Pouzije 3. krat: este aj vsetko bude naopak :>>
- stit       - ak ho user ma, nik s nim nemoze bojovat! Ak ho ma v arene,
               tak mu pri napadnuti znizi utocnikovi sancu. :>>>>>>>>>>>>
- satka      - sebevrazednu utrok kamikadze na daneho usera, odhlasi obox :)
- napoj      - napoj neviditelnosti. Kto ho vypije, nieto ho vidno viac ! :>
- telefon    - uzivatel moze tellallnut nejaku spravu :>
- afrodiziakum - Uzivatelovi sa neminaju urcity cas neminaju PP-cky za kiss
                 a hug :)
- kniha	     - obsahuje zoznam kuziel a prisad... teda ak ich niekto napise :)
*/

/* A kde je maska? F pazi. Mozno to koli tej blbej maske blblo ...
   aj tak s tym kazdy len robil blbosti ... :-} */
/* ... Tajtrlik rules! :>>> heh mohli by sme to zasa obnovitj :> */
/* Yasneeee!! :> */
/* ale mne sa to uz nexce vsetko osetrovatj ... */

char *langword[]={ /* nezabudajte ciarky ! potom to haluzi */
"??","sk","en","de","fr","hu","it", /* skratka v .set lang */
"?L","Slovensky","English","Deutsch","Francais","Magyar","Italiano",
"?S"," jazyk nastaveny"," language set"," eingestellt",
     " langage ajuste"," nyelv beallitva"," lingua tendera",
"?:","Nastaveny jazyk:","Your language:","Sprache:",
     "Le langage ajuste:","Beallitott nyelv:","La lingua tendera",
"S1","zakrical","shouts","schreit","crie","orditodta","grida",
"S2","zakricala","shouts","schreit","crie","orditodta","grida",
"S3","Zakrical si","You shout","Du schreist","Tu cries","Orditodtad","Tu gridi",
"S4","Zakricala si","You shout","Du schreist","Tu cries","Orditodtad","Tu gridi",
"G1","kecol","gossips","quatscht","cancane","fecsegte","pettegola",
"G2","kecla","gossips","quatscht","cancane","fecsegte","pettegola",
"G3","Kecas","You gossip","Du quatschst","Tu cancanes","Fecsegsz","Tu pettegoli",
"P1","povedal","says","sagt","dit","mondta","dice",
"P2","povedala","says","sagt","dit","mondta","dice",
"P3","Povedal si","You say","Du sagst","Tu dis","Mondtad","Tu dici",
"P4","Povedala si","You say","Du sagst","Tu dis","Mondtad","Tu dici",
"Hh","hovori","says","sagt","dit","mondta","dice",
"HH","Hovoris","You say","Du sagst","Tu dis","Mondtad","Tu dici",
"Q1","sa spytal","asks","fragt","demande","kerdezi","domanda",
"Q2","sa spytala","asks","fragt","demande","kerdezi","domanda",
"Q3","Spytal si sa","You ask","Du fragst","Tu demandes","Megkerdezted","Tu domandi",
"Q4","Spytala si sa","You ask","Du fragst","Tu demandes","Megkerdezted","Tu domandi",
"E1","zvolal","exclaims","ruft","crie","kialtotta","grida",
"E2","zvolala","exclaims","ruft","crie","kialtotta","grida",
"E3","Zvolal si","You exclaim","Du rufst","Tu cries","Kialtodtad","Tu gridi",
"E4","Zvolala si","You exclaim","Du rufst","Tu cries","Kialtodtad","Tu gridi",
"TH","si mysli","thinks","denkt sich","pense","gondolja","pensa",
"YT","Myslis si","You think","Du denkst","Tu penses","Azt gondolod","Tu pensi",
"Z1","zaspieval","sings","singt","chante","elenekelte","canta",
"Z2","zaspievala","sings","singt","chante","elenekelte","canta",
"Z3","Zaspieval si","You sing","Du singst","Tu chantes","Elenekelted","Tu canti",
"Z4","Zaspievala si","You sing","Du singst","Tu chantes","Elenekelted","Tu canti",
"Ty","ti povedal","tells you","sagt dir","dit toi","mondta neked","parla a te",
"TY","ti povedala","tells you","sagt dir","dit toi","mondta neked","parla a te",
"Tt","Povedal si","You tell","Du sagst","Tu dis a","Mondtad","Tu parli",
"TT","Povedala si","You tell","Du sagst","Tu dis a","Mondtad","Tu parli",
"T?","Komu a co chces povedat? (Pouzi: .tell <uzivatel> <text>)",
     "Tell who what? (Use: .tell <user> <text>)",
     "Wem mochtest du etwas sagen? (Verwende .tell <wem> <was>)",
     "A qui est-ce que tu veut dire quoi? (Emloye .tell <qui> <quoi>)",
     "Kinek mit akarsz mondani? (Hasznald: .tell <felhasznalo> <szoveg>)",
     "A chi e che cosa vuoi parle? (usi: .tell <utente> <lettera>)",
"AK","Bol(a) si afk od","You were afk since","Du warst afk seit","Tu a ete afk depuis",
     "Afk voltal(ol)","Tu sei stato(a) a afk da(di).",
"ED","Pisal(a) si v editore od",
     "You were writing in editor since",
     "Du hast im Editor geschrieben seit",
     "Tu a dit a l'editor depuis",
     "Szovegszerkesztoben irtal(ol)",
     "Tu hai scritto nel' editor da(i).",
"RE","Cital(a) si nieco od",
     "You were reading something since",
     "Du hast etwas gelesen seit",
     "Tu a dit a l'editor depuis",
     "Szovegszerkesztoben irtal(ol)",
     "Tu hai scritto nel' editor da(i).",
"MZ","Si umlcany.","You are muzzled.","Du bist geknebelt.",
     "Tu es baillone","El vagy halgattatva.","Tu sei silenzioso.",
"Im","Neznamy","Someone","Unbekannter","Inconnu","Ismeretlen","Sconoscinto",
"Iz","Neznama","Someone","Unbekannte","Inconnue","Ismeretlen","Sconoscinta",
"Un","neznamemu","to someone","unbekanntem","a l'inconnu","ismeretlennek","a sconoscinto",
"UN","neznamej","to someone","unbekannter","a l'inconnue","ismeretlennek","a sconoscinta",
"WK","Kricis kralom","You wizshout","Du schreist den Zauberern",
      "Tu cries aux magiciens","Kiralyoknak kialtod","Tu gridi ai re",
"WG","Kricis godom","You shout to gods","Du schreist den Gotter",
     "Tu cries aux gods","Isteneknek kialtod","Tu gridi agli dei",
"Wk","krici kralom","wizshouts","schreit den Zauberen","crie aux magiciens","kiralyoknak kialtja","grida ai re",
"Wg","krici godom","shouts to gods","schreit zu Gotter","crie aux gods","isteneknek kialtja","grida agli dei",
"UC","Neznamy prikaz","Unknown command","Eingabe inkorrekt","Uh-huh?","Ismeretlen utasitas","Ordine sconosciuto",
"*"
};

#define LANGUAGES 6
