/**********************  Buko:  B A N N E R  stuff  ************************/
/**************************************************************************/
#include "atl-head.h"
#include "atl-mydb.h"

#define MAX_BANNER_LEN 40       /* max dlzka banneru */
#define MAX_BANNER_LEN_LUSER 20 /* max dlzka banneru pre luzrof (<WIZ) ;) */
#define MYSTRLEN(x) ((int)strlen(x)) /* Eliminate ANSI problem */
/* DIRSEP - make this '\\' for an MS-DOS port. */
/* Note: '/' also used in filename in get_columns(). */
#define DIRSEP '/'
#define FONTFILEMAGICNUMBER "flf2"
#define FSUFFIXLEN MYSTRLEN(TMPSUFFIX)
#define DEFAULTCOLUMNS 80
#define MAXFIRSTLINELEN 1000

/***
* deklaracie internych funkcii banneru
***/

int create_banner(UR_OBJECT user1, UR_OBJECT user2, int type, char *inpstr, int output);
int freefont();
void splitline(UR_OBJECT user1, UR_OBJECT user2, int output);

/***
* figlet (C) 1991, 1993, 1994 Glenn Chappell and Ian Chai
* Internet: <ggc@uiuc.edu> and <chai@uiuc.edu>
* Modified by Buko <buko@innocent.com> for Atlantis talker <www.atlantis.sk
* Banner (C) 1997,1998 by Buko
***/

int x;

/***
* POZOR NA TOTO: ak sa vyskytne font, ktory je vacsi ako M alebo N, vznikne overflow, ktory zhodi talker!!!
***/
#define M 30 /* vyska fontu -> dalej alokovane v strukture fcharnode! */
#define N 30 /* sirka fontu    cim vacsi rozmer, tym vasciu cast pamate zaberie */
#define DEFAULTFONTFILE "small.flf" /* defaultny font ;) */

/***
* Globals dealing with chars that are read
***/
typedef long inchr; /* "char" read from stdin */

inchr inchrline[2000];  /* Alloc'd inchr inchrline[inchrlinelenlimit+1]; */
/* Note: not null-terminated. */
int inchrlinelen,inchrlinelenlimit;

/***
* Globals dealing with chars that are written
***/
typedef struct fc {
	inchr ord;
	char thechar[M][N];  /* Alloc'd char thechar[charheight][]; */
	struct fc *next;
} fcharnode;

fcharnode *fcharlist;

char currchar[M][N];
int currcharwidth;
char outline[M][80];    /* Alloc'd char outline[charheight][outlinelenlimit+1]; */
int outlinelen;

/***
* Globals affected by command line options
***/
int deutschflag,justification,paragraphflag,right2left;
/***
* smushmode: (given after "-m" command line switch)
* -2: Get default value from font file (default)
* -1: Do not smush
* For explanation of non-negative values, see smushem().
***/
int smushmode;
int outputwidth;
int outlinelenlimit;
char *fontdirname,*fontname;

/***
* Globals read from font file
***/
char hardblank;
int charheight,defaultmode;

/***
* myalloc
* Calls malloc.  If malloc returns error, prints error message and quits.
***/
char *myalloc(size_t size)
{
	char *ptr;

	if ((ptr = (char*)malloc(size))==NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	else {
		return ptr;
	}
}

/***
* skiptoeol
* Skips to the end of a line, given a stream.
***/
void skiptoeol(FILE *fp)
{
	int dummy;

	while (dummy=getc(fp),dummy!='\n'&&dummy!=EOF);
}


/***
* getparams
* Handles all command-line parameters.  Puts all parameters within bounds except smushmode.
* Zatial som to spravil tak, ze to len nastavuje default parametre. Najdolezitejsi je fontname
***/
void getparams(char *fontsubor)
{
	int firstfont;

	fontdirname = TMPFOLDER;
	firstfont = 1;
	fontname = (char*)myalloc(sizeof(char)*(MYSTRLEN(DEFAULTFONTFILE)+1));
	strcpy(fontname,DEFAULTFONTFILE); /* Some systems don't have strdup() */
	if ((MYSTRLEN(fontname)>=FSUFFIXLEN)?!strcmp(fontname+MYSTRLEN(fontname)-FSUFFIXLEN,TMPSUFFIX):0) {
		fontname[MYSTRLEN(fontname)-FSUFFIXLEN]='\0';
	}
	smushmode = -2; /* default -2 - berie smushing nastavenia z fontfile, -1 - do not smush! */
	deutschflag = 0;
	justification = 1; /* 1 == centered -1 == default */
	right2left = -1; /* default -1, 1 == odzadu */
	paragraphflag = 0;
	outputwidth = DEFAULTCOLUMNS; /* sirka outputu */
	outlinelenlimit = outputwidth-1;
	if (firstfont) {
		free(fontname);
		firstfont = 0;
	}
	fontname = fontsubor;
	if ((MYSTRLEN(fontname)>=FSUFFIXLEN)?!strcmp(fontname+MYSTRLEN(fontname)-FSUFFIXLEN,TMPSUFFIX):0) {
		fontname[MYSTRLEN(fontname)-FSUFFIXLEN] = '\0';
	}
}

/***
* clrline
* Clears both the input (inchrline) and output (outline) storage.
***/
void clrline(void)
{
	int i;

	for (i=0;i<charheight;i++) {
		outline[i][0] = '\0';
	}
	outlinelen = 0;
	inchrlinelen = 0;
}

/***
* readfontchar
* Reads a font character from the font file, and places it in a newly-allocated entry in the list.
***/
void readfontchar(FILE *file,inchr theord,char *line,int maxlen)
{
	int row,k;
	char endchar;
	fcharnode *fclsave;

	fclsave = fcharlist;
	fcharlist = (fcharnode*)malloc(sizeof(fcharnode));
	fcharlist->ord = theord;
	for (x=0;x<M;x++) strcpy(fcharlist->thechar[x],"");
	fcharlist->next = fclsave;
	for (row=0;row<charheight;row++) {
		if (fgets(line,maxlen+1,file)==NULL) {
			line[0] = '\0';
		}
		k = MYSTRLEN(line)-1;
		while (k>=0 && isspace(line[k])) {
			k--;
		}
		if (k>=0) {
			endchar = line[k];
			while (k>=0 ? line[k]==endchar : 0) {
				k--;
			}
		}
		line[k+1] = '\0';
		strcpy(fcharlist->thechar[row],line);
	}
}

/***
* readfont
* Allocates memory, initializes variables, and reads in the font.
***/
int readfont(void)
{
	int i,row,numsread;
	inchr theord;
	int maxlen,cmtlines,ffright2left;
	char *fontpath,*fileline,magicnum[5];
	FILE *fontfile;
	int namelen;

	namelen=MYSTRLEN(fontdirname);
	fontpath=(char*)myalloc(sizeof(char)*(namelen+MYSTRLEN(fontname)+FSUFFIXLEN+2));
	fontfile=NULL;
	if (!strchr(fontname,DIRSEP)) {
		strcpy(fontpath,fontdirname);
		fontpath[namelen]=DIRSEP;
		fontpath[namelen+1]='\0';
		strcat(fontpath,fontname);
		strcat(fontpath,TMPSUFFIX);
		fontfile=ropen(fontpath,"r");
	}
	if (fontfile==NULL) {
		strcpy(fontpath,fontname);
		strcat(fontpath,TMPSUFFIX);
		fontfile=ropen(fontpath,"r");
		if (fontfile==NULL) return 0;
	}
	fscanf(fontfile,"%4s",magicnum);
	fileline=(char*)myalloc(sizeof(char)*(MAXFIRSTLINELEN+1));
	if (fgets(fileline,MAXFIRSTLINELEN+1,fontfile)==NULL) fileline[0]='\0';
	if (MYSTRLEN(fileline)>0?fileline[MYSTRLEN(fileline)-1]!='\n':0) skiptoeol(stdin);
	/* Nacitanie dolezitych parametrov fontu!! */
	numsread = sscanf(fileline,"%*c%c %d %*d %d %d %d%*[ \t]%d",&hardblank,&charheight,&maxlen,&defaultmode,&cmtlines,&ffright2left);
	free(fileline);
	if (strcmp(magicnum,FONTFILEMAGICNUMBER) || numsread<5) return 1;
	for (i=1;i<=cmtlines;i++) skiptoeol(fontfile);
	if (numsread<6) ffright2left = 0;
	if (charheight<1) charheight = 1;
	if (maxlen<1) maxlen = 1;
	maxlen+=100; /* Give ourselves some extra room */
	if (smushmode<-1) smushmode = -1;
	if (right2left<0) right2left = ffright2left;
	if (justification<0) justification = 2*right2left;
	fileline=(char*)myalloc(sizeof(char)*(maxlen+1));
	/* Allocate "missing" character */
	fcharlist=(fcharnode*)malloc(sizeof(fcharnode));
	fcharlist->ord=0;
	for (x=0;x<M;x++) strcpy(fcharlist->thechar[x],"");
	fcharlist->next=NULL;
	for (row=0;row<charheight;row++) fcharlist->thechar[row][0]='\0';
	for (theord=' ';theord<='~';theord++) readfontchar(fontfile,theord,fileline,maxlen);
	fclose(fontfile);
	deltempfile(fontpath); /* vymazanie docasneho suboru s fontom */
	free(fontpath);
	free(fileline);
	return 1;
}

/***
* linealloc
***/
void linealloc(void)
{

	inchrlinelenlimit=outputwidth*4+100;
	inchrline[0]='\0';
	clrline();
}

/***
* getletter
* Sets currchar to point to the font entry for the given character.
* Sets currcharwidth to the width of this character.
***/
void getletter(inchr c)
{
	int x;
	fcharnode *charptr;

	for (charptr=fcharlist;charptr==NULL?0:charptr->ord!=c;charptr=charptr->next);
	if (charptr!=NULL) {
		for (x=0;x<M;x++) strcpy(currchar[x],charptr->thechar[x]);
	}
	else {
		for (charptr=fcharlist;charptr==NULL?0:charptr->ord!=0;charptr=charptr->next);
		for (x=0;x<M;x++) strcpy(currchar[x],charptr->thechar[x]);
	}
	currcharwidth = MYSTRLEN(currchar[0]);
}

/***
* smushem
* Given 2 characters, attempts to smush them into 1, according to smushmode. Returns smushed character or '\0' if no smushing can be done. Assumes smushmode >= 0.
* smushmode values are sum of following (all values smush blanks):
*  1: Smush equal chars (not hardblanks)
*  2: Smush '_' with any char in hierarchy below
*  4: hierarchy: "|", "/\", "[]", "{}", "()", "<>"
*     Each class in hier. can be replaced by later class.
*  8: [ + ] -> |, { + } -> |, ( + ) -> |
* 16: / + \ -> X, > + < -> X (only in that order)
* 32: hardblank + hardblank -> hardblank
***/
char smushem(char lch,char rch)
{
	if (lch==' ') return rch;
	if (rch==' ') return lch;

	if (smushmode & 32) {
		if (lch==hardblank && rch==hardblank) return lch;
	}

	if (lch==hardblank || rch==hardblank) return '\0';

	if (smushmode & 1) {
		if (lch==rch) return lch;
	}

	if (smushmode & 2) {
		if (lch=='_' && strchr("|/\\[]{}()<>",rch)) return rch;
		if (rch=='_' && strchr("|/\\[]{}()<>",lch)) return lch;
	}

	if (smushmode & 4) {
		if (lch=='|' && strchr("/\\[]{}()<>",rch)) return rch;
		if (rch=='|' && strchr("/\\[]{}()<>",lch)) return lch;
		if (strchr("/\\",lch) && strchr("[]{}()<>",rch)) return rch;
		if (strchr("/\\",rch) && strchr("[]{}()<>",lch)) return lch;
		if (strchr("[]",lch) && strchr("{}()<>",rch)) return rch;
		if (strchr("[]",rch) && strchr("{}()<>",lch)) return lch;
		if (strchr("{}",lch) && strchr("()<>",rch)) return rch;
		if (strchr("{}",rch) && strchr("()<>",lch)) return lch;
		if (strchr("()",lch) && strchr("<>",rch)) return rch;
		if (strchr("()",rch) && strchr("<>",lch)) return lch;
	}

	if (smushmode & 8) {
		if (lch=='[' && rch==']') return '|';
		if (rch=='[' && lch==']') return '|';
		if (lch=='{' && rch=='}') return '|';
		if (rch=='{' && lch=='}') return '|';
		if (lch=='(' && rch==')') return '|';
		if (rch=='(' && lch==')') return '|';
	}

	if (smushmode & 16) {
		if (lch=='/' && rch=='\\') return 'X';
		if (rch=='/' && lch=='\\') return 'X';
		if (lch=='>' && rch=='<') return 'X';
		/* Don't want the reverse of above to give 'X'. */
	}
	return '\0';
}


/***
* smushamt
* Returns the maximum amount that the current character can be smushed into the current line.
***/
int smushamt(void)
{
	int maxsmush,amt;
	int row,linebd,charbd;
	char ch1,ch2;

	if (smushmode== -1) return 0;
	maxsmush=currcharwidth;
	for (row=0;row<charheight;row++) {
		if (right2left) {
			for (charbd=MYSTRLEN(currchar[row]);ch1=currchar[row][charbd],(charbd>0&&(!ch1||ch1==' '));charbd--);
			for (linebd=0;ch2=outline[row][linebd],ch2==' ';linebd++);
			amt=linebd+currcharwidth-1-charbd;
		}
		else {
			for (linebd=MYSTRLEN(outline[row]);ch1 = outline[row][linebd],(linebd>0&&(!ch1||ch1==' '));linebd--);
			for (charbd=0;ch2=currchar[row][charbd],ch2==' ';charbd++);
			amt=charbd+outlinelen-1-linebd;
		}
		if (!ch1||ch1==' ') amt++;
		else if (ch2) if (smushem(ch1,ch2)!='\0') amt++;
		if (amt<maxsmush) maxsmush = amt;
	}
	return maxsmush;
}

/***
* addchar
* Attempts to add the given character onto the end of the current line.
* Returns 1 if this can be done, 0 otherwise.
***/
int addchar(inchr c)
{
	int smushamount,row,k;
	char templine[500];

	getletter(c);
	smushamount=smushamt();
	if (outlinelen+currcharwidth-smushamount>outlinelenlimit || inchrlinelen+1>inchrlinelenlimit) return 0;
	strcpy(templine,"");
	for (row=0;row<charheight;row++) {
		if (right2left) {
			strcpy(templine,currchar[row]);
			for (k=0;k<smushamount;k++) templine[currcharwidth-smushamount+k]=smushem(templine[currcharwidth-smushamount+k],outline[row][k]);
			strcat(templine,outline[row]+smushamount);
			strcpy(outline[row],templine);
		}
		else {
			for (k=0;k<smushamount;k++) outline[row][outlinelen-smushamount+k]=smushem(outline[row][outlinelen-smushamount+k],currchar[row][k]);
			strcat(outline[row],currchar[row]+smushamount);
		}
	}
	outlinelen=MYSTRLEN(outline[0]);
	inchrline[inchrlinelen++] = c;
	return 1;
}

/***
* putstring
* Prints out the given null-terminated string, substituting blanks for hardblanks.
* If outputwidth is 1, prints the entire string; otherwise prints at most outputwidth-1 characters.
* Prints a newline at the end of the string.
* The string is left-justified, centered or right-justified (taking outputwidth as the screen width) if justification is 0, 1 or 2, respectively.
***/
char *putstring(char string[80])
{
	static char out[2000];
	int i,len,ii=0;

	for (i=0;i<2000;i++) out[i]='\0';
	len=MYSTRLEN(string);
	if (outputwidth>1) {
		if (len>outputwidth-1) len=outputwidth-1;
		if (justification>0) for (i=1;(3-justification)*i+len+justification-2<outputwidth;i++) out[ii++]=' ';
	}
	for (i=0;i<len;i++) out[ii++]=string[i]==hardblank?' ':string[i];
	out[ii++]='\n';
	out[ii]='\0';
	return out;
}

/***
* printline
* Prints outline using putstring, then clears the current line.
***/
void printline(UR_OBJECT user1, UR_OBJECT user2, int output)
{
	int i, j;
	char vystupny_banner[2000];

	/* banner() */
	if (output==1) {
		for (i=0;i<charheight;i++) {
			strcpy(vystupny_banner,putstring(outline[i]));
			write_room(user1->room,vystupny_banner);
		}

		clrline();
	}
	/* tbanner() */
	if (output==2) {
		for (i=0;i<charheight;i++) {
			strcpy(vystupny_banner,putstring(outline[i]));
			sprintf(text,"~OL%s~RS",vystupny_banner);
			write_user(user1, text);
			if (user1!=user2) write_user(user2, text);
		}
		clrline();
	}
	/* sbanner() */
	if (output==3) {
		for (i=0;i<charheight;i++) {
			strcpy(vystupny_banner,putstring(outline[i]));
			if (user1->room->group==2) { /* OSTROV => do roomy a vsetkych okolo */
				write_room(user1->room,vystupny_banner);
				for(j=0;j<MAX_LINKS;j++) if (user1->room->link[j]!=NULL) write_room_except(user1->room->link[j],vystupny_banner, user1);
			}
			else {
				write_room_except(NULL,vystupny_banner,user1);
				write_user(user1,vystupny_banner);
			}
		}
		clrline();
	}
}

/***
* splitline
* Splits inchrline at the last word break (bunch of consecutive blanks).
* Makes a new line out of the first part and prints it using printline.
* Makes a new line out of the second part and returns.
***/

void splitline(UR_OBJECT user1,UR_OBJECT user2, int output)
{
	int i,gotspace,lastspace,len1,len2;
	inchr *part1,*part2;

	part1=(inchr*)myalloc(sizeof(inchr)*(inchrlinelen+1));
	part2=(inchr*)myalloc(sizeof(inchr)*(inchrlinelen+1));
	gotspace=lastspace=0;
	for (i=inchrlinelen-1;i>=0;i--) {
		if (!gotspace && inchrline[i]==' ') {
			gotspace = 1;
			lastspace = i;
		}
		if (gotspace && inchrline[i]!=' ') {
			break;
		}
	}
	len1=i+1;
	len2=inchrlinelen-lastspace-1;
	for (i=0;i<len1;i++) part1[i]=inchrline[i];
	for (i=0;i<len2;i++) part2[i]=inchrline[lastspace+1+i];
	clrline();
	for (i=0;i<len1;i++) addchar(part1[i]);
	printline(user1, user2, output);
	for (i=0;i<len2;i++) addchar(part2[i]);
	free(part1);
	free(part2);
}

/***
* Agetchar
* Replacement to getchar().
* Acts exactly like getchar if -A is NOT specified, else obtains input from All remaining command line words.
***/
int Agetchar(int flag,char *buffer)
{
	int c; /* current character */

	/* find next character */
	c=buffer[flag++]&0xFF;	/* get appropriate char of buffer */
	if (!c) /* at '\0' that terminates word? */
		c='\0'; /* (allows "hello '' world" to do \n at '') */
	return(c); /* return appropriate character */
}


/***
* banner - klasika, vypise vsetkym v miestnosti
***/
extern void banner(UR_OBJECT user,char *inpstr)
{
	char name[15];
	int type;
	int max_font_id;
	int min_font_id;

	max_font_id=getfontid(1);
	min_font_id=getfontid(0);

	if (user->muzzled) {
		write_user(user,"Si umlcany - nemozes pouzivat banner.\n");
		return;
	}
	if (!strcmp(word[1],"-h")) {
		help_fonts(user);
		return;
	}
	if (word_count<3 || !is_number(word[1])) {
		sprintf(   text,"Pouzi: .banner <typ> <sprava>\n       Dostupne typy: ~OL%d-%d~RS\n",min_font_id,max_font_id);
		write_user(user,text);
		return;
	}
	type=atoi(word[1]);
	if (type<min_font_id || type>max_font_id) {
		sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu %d-%d.\n",min_font_id,max_font_id);
		write_user(user,text);
		return;
	}
	if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
		write_user(user,noswearing);
		sprintf(text,"~OL~FR\07[CENZURA: %s sa pokusil%s napisat skaredy banner!]~RS\n",user->name,pohl(user,"","a"));
		writesys(WIZ,1,text,user);
		return;
	}
	if ((contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT))) {
		write_user(user,nocolors);
		return;
	}
	if (user->vis) strcpy(name,user->name);
	else strcpy(name,invisname(user));
	inpstr=remove_first(inpstr);
	if (user->level<WIZ && strlen(inpstr)>MAX_BANNER_LEN_LUSER) {
		sprintf(text,"Nemozes generovat banner dlhsi ako %d znakov.\n",MAX_BANNER_LEN_LUSER);
		write_user(user,text);
		return;
	}
	if (strlen(inpstr)>MAX_BANNER_LEN) {
		sprintf(text,"Sorry, maximalna dlzka banneru je %d znakov.\n",MAX_BANNER_LEN);
		write_user(user,text);
		return;
	}
	if (user->pp < BANNERCAN) { /*PP*/
		sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, BANNERCAN);
		write_user(user,text);
		return;
	}
	sprintf(text,"~FR%s ~RS~FWnapisal%s banner[~OL%d~RS]:\n",name,pohl(user,"","a"),type);
	write_room(user->room,text);
	if (!create_banner(user, NULL, type,inpstr,1)) {
		write_user(user,"Chyba pri nacitavani fontu! ;(\n");
		return;
	}
	decrease_pp(user,BANNERDEC,0);
	freefont();
	clrline();
}

/***
* tbanner
* tell_banner, je mozne tellovat aj sam sebe
***/
extern void tbanner(UR_OBJECT user,char *inpstr)
{
	int max_font_id;
	int min_font_id;
	char name[15];
	int type;
	UR_OBJECT u;

	max_font_id=getfontid(1);
	min_font_id=getfontid(0);

	if (user->muzzled) {
		write_user(user,"Si umlcany - nemozes pouzivat tbanner.\n");
		return;
	}
	if (!strcmp(word[1],"-h")) {
		help_fonts(user);
		return;
	}
	if (is_number(word[1]) && word_count>2) {
		u=user;
		type=atoi(word[1]);
	}
	else {
		if (word_count<4 || !is_number(word[2])) {
			sprintf(text,"Pouzi: .tbanner [user] <typ> <sprava>\n       Dostupne typy: ~OL%d-%d~RS\n",min_font_id,max_font_id);
			write_user(user,text);
			return;
		}
		type=atoi(word[2]);
		if (!(u=get_user(word[1]))) {
			write_user(user,notloggedon);
			return;
		}
		inpstr=remove_first(inpstr);
	}
	if (type<min_font_id || type>max_font_id) {
		sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu %d-%d.\n",min_font_id,max_font_id);
		write_user(user,text);
		return;
	}
	if (user->vis) strcpy(name,user->name);
	else strcpy(name,invisname(user));
	inpstr=remove_first(inpstr);
	if (user->level<WIZ && strlen(inpstr)>MAX_BANNER_LEN_LUSER) {
		sprintf(text,"Nemozes generovat banner dlhsi ako %d znakov.\n",MAX_BANNER_LEN_LUSER);
		write_user(user,text);
		return;
	}
	if (strlen(inpstr)>MAX_BANNER_LEN) {
		sprintf(text,"Sorry, maximalna dlzka banneru je %d znakov.\n",MAX_BANNER_LEN);
		write_user(user,text);
		return;
	}
	if (u->ignall) {
		if (u->malloc_start!=NULL) sprintf(text,"%s prave nieco pise v editore.\n",u->name);
		else if (u->filepos) sprintf(text,"%s prave cita nejaky text.\n",u->name);
		else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
		write_user(user,text);
		return;
	}
	if (u->igntell) {
		if (user->level<WIZ || u->level>=user->level) {
			sprintf(text,"%s ignoruje telly a ostatne privatne hlasky.\n",u->name);
			write_user(user,text);
			return;
		}
		else if (user->level>WIZ || u->level<user->level) {
			sprintf(text,"%s by ta normalne mal%s ignorovat.\n",u->name,pohl(u,"","a"));
			write_user(user,text);
		}
	}
	if (check_ignore_user(user,u)) { /* ignorovanie single usera */
		sprintf(text,"%s Ta ignoruje.\n",u->name);
		write_user(user,text);
		return;
	}
	if (u->room->group!=user->room->group) {
		sprintf(text,"%s je od teba prilis vzdialen%s na bannerovanie!\n",u->name,pohl(u,"y","a"));
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
		write_user(user,"V zalari LEN z plneho hrdla (s plnymi pp-ckami ;)!\n");
		return;
	}
	if (user!=u) {
		sprintf(text,"~OL~FW%s dostal%s banner:\n", u->name, pohl(u,"","a"));
		write_user(user,text);
		sprintf(text,"~OL~FW%s Ti posiela banner[%d]:\n",name,type);
		write_user(u,text);
	}
	else {
		sprintf(text,"Takto vyzera banner ~OL%d~RS:\n", type);
		write_user(user,text);
	}
	if (!create_banner(u, user, type,inpstr,2)) {
		write_user(user,"Chyba pri nacitavani fontu! ;(\n");
		return;
	}
	freefont();
	clrline();
}

/***
* sbanner
* shout_banner - vidia vsetci (okem tych, co maju ignshout)
***/
extern void sbanner(UR_OBJECT user,char *inpstr)
{
	int max_font_id;
	int min_font_id;
	char name[15];
	int type,i;

	max_font_id=getfontid(1);
	min_font_id=getfontid(0);

	if (user->muzzled) {
		write_user(user,"Si umlcany - nemozes pouzivat sbanner.\n");
		return;
	}
	if (!strcmp(word[1],"-h")) {
		help_fonts(user);
		return;
	}
	if (word_count<3 || !is_number(word[1])) {
		sprintf(text,"Pouzi: .sbanner <typ> <sprava>\n       Dostupne typy: ~OL%d-%d~RS\n",min_font_id,max_font_id);
		write_user(user,text);  return;
	}
	type=atoi(word[1]);
	if (type<min_font_id || type>max_font_id) {
		sprintf(text,"Typ musi byt cele kladne cislo a musi byt z rozsahu %d-%d.\n",min_font_id,max_font_id);
		write_user(user,text);
		return;
	}
	if ((user->level<MOZENADAVAT) && (contains_swearing(inpstr,user)) && (ban_swearing)) {
		write_user(user,noswearing);
		sprintf(text,"~OL~FR\07[CENZURA: %s sa pokusil%s napisat skaredy banner!]~RS\n",user->name,pohl(user,"","a"));
		writesys(WIZ,1,text,user);
		return;
	}
	if (contains_advert(inpstr) && (user->level<MOZEFARBICKOVAT)) {
		write_user(user,nocolors);
		return;
	}
	if (user->vis) strcpy(name,user->name); else strcpy(name,invisname(user));
	inpstr=remove_first(inpstr);
	if (user->level<WIZ && strlen(inpstr)>MAX_BANNER_LEN_LUSER) {
		sprintf(text,"Nemozes generovat banner dlhsi ako %d znakov.\n",MAX_BANNER_LEN_LUSER);
		write_user(user,text);
		return;
	}
	if (strlen(inpstr)>MAX_BANNER_LEN) {
		sprintf(text,"Sorry, maximalna dlzka banneru je %d znakov.\n",MAX_BANNER_LEN);
		write_user(user,text);
		return;
	}

	if (user->pp < SBANNERCAN) {
		sprintf(text,"Mas malo energie (%d), potrebujes aspon %d!\n",user->pp, SBANNERCAN);
		write_user(user,text);
		return;
	}

	sprintf(text,"~OL~FYVyslal%s si banner:\n",pohl(user,"","a"));
	write_user(user,text);
	sprintf(text,"~OL~FY%s vyslal%s banner[~FW%d~FY]: ~RS~FW%s\n",name,pohl(user,"","a"),type, inpstr);
	if (user->room->group==2) {/* OSTROV */
		write_room_except(user->room,text,user); /* do roomy */
		for(i=0;i<MAX_LINKS;i++) { /* a vsetkych okolo */
			if (user->room->link[i]!=NULL)
				write_room_except(user->room->link[i],text,user);
		}
		sprintf(text,"~OL~FY%s vyslal%s banner: ~FG[~RS~FW%s~OL~FG]~RS\n",name,pohl(user,"","a"),inpstr);
		record_portalisshout(user->room->label,text);
	}
	else {
		write_room_except(NULL,text,user);
		sprintf(text,"~OL~FY%s %s banner: ~FG[~RS~FW%s~OL~FG]~RS\n",name,pohl(user,"vyslal","vyslala"),inpstr);
		record_shout(text);
	}
	if (!create_banner(user, NULL, type,inpstr,3)) {
		write_user(user,"Chyba pri nacitavani fontu! ;(\n");
		return;
	}
	decrease_pp(user,SBANNERDEC,SBANNERDYN);
	freefont();
	clrline();
}

/***
* create_banner(user,type,inpstr,output) - dostane typ banneru a inpstr a vystup je v podobe tych peknych pismeniek, ake jednoduche, ze? ;)
* output:
* 1 - banner - generuje v danej roome
* 2 - tell banner - telluje danemu luserovi
* 3 - shout banner - vidia vsetci
***/
extern int create_banner(UR_OBJECT user1,UR_OBJECT user2,int type,char *inpstr,int output)
{
	FILE *fptff;
	char meno_fontu[50];
	inchr c;
	int i;
	char tempfontfilename[80];
	char query[100];
	MYSQL_RES *result;
	MYSQL_ROW row;
	int wordbreakmode;
	int char_not_added;
	int flag=0;

	/***
	* wordbreakmode:
	* -1: /^$/ and blanks are to be absorbed (when line break was forced
	*          by a blank or character larger than outlinelenlimit)
	*  0: /^ *$/ and blanks are not to be absorbed
	*  1: /[^ ]$/ no word break yet
	*  2: /[^ ]  *$/
	*  3: /[^ ]$/ had a word break
	***/

	/* font tahame z DB, zapiseme do docasneho suboru, nakoniec zmazeme */
	sprintf(query,"SELECT `fontbody` FROM `fonts` where `fontid`=%d and `enabled`='Y'",type);
	if (!(result=mysql_result(query))) return 0;
	if (mysql_num_rows(result)!=1) return 0;
	row=mysql_fetch_row(result);
	sprintf(meno_fontu,"%s-font%s",user1->name,TMPSUFFIX);
	sprintf(tempfontfilename,"%s%s",TMPFOLDER,meno_fontu);
	if ((fptff=ropen(tempfontfilename,"w"))==NULL) return 0;
	fprintf(fptff,row[0]);
	mysql_free_result(result);
	fclose(fptff);
	getparams(meno_fontu); /* Nastavenia parametrov - dolezite!!! */
	if (!readfont()) return 0; /* neuspesne nacitanie fontov! skoncili sme, jasna sprava! ;) */
	linealloc();
	wordbreakmode = 0;  /* typ modu brejkovania slov - vid vyssie (0) */
	/* Main cyklus ;) */
	while ((c = Agetchar(flag++,inpstr))!='\0') {
		if (isascii(c)&&isspace(c)) c=(c=='\t'||c==' ')?' ':'\n';
		if ((c>'\0' && c<' ' && c!='\n') || c==127) continue;
		/***
		* Note: The following code is complex and thoroughly tested.
		* Be careful when modifying!
		***/
		do {
			char_not_added = 0;
			if (wordbreakmode== -1) {
				if (c==' ') break;
				else if (c=='\n') {
					wordbreakmode = 0;
					break;
				}
				wordbreakmode = 0;
			}
			if (c=='\n') {
				printline(user1, user2, output);
				wordbreakmode = 0;
			}
			else if (addchar(c)) {
				if (c!=' ') wordbreakmode = (wordbreakmode>=2)?3:1;
				else wordbreakmode = (wordbreakmode>0)?2:0;
			}
			else if (outlinelen==0) {
				for (i=0;i<charheight;i++) {
					if (right2left && outputwidth>1) putstring(currchar[i]+MYSTRLEN(currchar[i])-outlinelenlimit);
					else putstring(currchar[i]);
				}
				wordbreakmode = -1;
			}
			else if (c==' ') {
				if (wordbreakmode==2) splitline(user1, user2, output);
				else printline(user1, user2, output);
				wordbreakmode = -1;
			}
			else {
				if (wordbreakmode>=2) splitline(user1, user2, output);
				else printline(user1, user2, output);
				wordbreakmode = (wordbreakmode==3)?1:0;
				char_not_added = 1;
			}
		} while (char_not_added);
	} /* end while */
	if (outlinelen!=0) {
		printline(user1, user2, output);
	}
	return 1;
}

/***
* freefont
* uvolni naalokovanu pamat, pokial v nej niesu ine vnorene nedealokovane smerniky, zrusi pointer na strukturu, etc, etc.. ;)
***/
extern int freefont(void)
{
	fcharnode *pomocny;

	while (fcharlist) {
		pomocny=fcharlist->next;
		free((void *)fcharlist);
		fcharlist=pomocny;
	}
	fcharlist=NULL;
	return 1;
}

/***
* getfontid
* type:
* 0 = najnizsie fontid
* 1 = najvyssie fontid
***/
int getfontid(int type) {
	char query[100];

	sprintf(query,"SELECT %s(`fontid`) FROM `fonts` WHERE `enabled`='Y'",type?"MAX":"MIN");
	return query_to_int(query);
}
