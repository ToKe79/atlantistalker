/******************************************************************
	      Atlantis magazine parser v1.00 HTML beta
		(C) 1999 Rider of LoneStar Software
           Revision: 26.4.2000 (Spartakus) [auth][/auth]
 ******************************************************************
 Pouzitie: int parse_ezin(FILE *subor, int issue, char *string)
      subor: kam presmerovat vystup
      issue: cislo vydania
     string: 1) alfanumericky - vyhladavanie retazca
	     2) numericky     - zobrazi iba prislusnu stat z obsahu
	     3) "obsah"	      - zobrazi obsah
	     4) NULL	      - zobrazi vsetko
  Navratove
    hodnoty: 0 - vsetko OK
             1 - issue nenajdene

 Pozn.: definicia HTML prepina rezim atlantis/web
 ******************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define title_path "issues"
#define title_name "dialogs"
#define TITLEN sizeof("[title]")
#define WRAPCHAR 79

#define TAGS 11
char *taglist[]={
	"title","/title",
	"bold","/bold",
	"eol","tab",
	"italic","/italic",
	"space",
	"author","/author"
};

char *lowercase(char *string)
{
	static char buffer[4096];
	char *ptr;
	strncpy(buffer,string,4090);
	ptr=buffer;
	while (*ptr){*ptr=tolower(*ptr); ptr++;}
	return buffer;
}

int parse_ezin(FILE *subor,int issue,char *string)
{
	int qm,i,tagmode,count,howmuch=0;
	char c,filename[256],tagcache[128],buffer[4096],tmp[100];
	int buffer_size, italic, bold, author;
	char *p1, *p2;
	FILE *file;

	int htmode;
	int waspace=0;
	sprintf (filename,"%s/%s.%03d",title_path,title_name,issue);
	if ((file=ropen(filename,"r"))==NULL) {
		if ((file=fopen(filename,"r"))==NULL)
		{
			fprintf(subor,"~OL~FRToto vydanie casopisu (cislo ~FW%d~FR) nebolo najdene!~RS\n", issue);
			return 1;
		}
	}
	if (string!=NULL && !strcmp(string,"obsah")) {
		count=1;
		fprintf(subor,"~BB~OL~FY          O B S A H   C I S L A  %d          ~RS~BK~FW\n\n",issue);
		while (fgets(buffer,4090,file)!=NULL) {
			if ((p1=strstr(lowercase(buffer),"[title]"))!=NULL && (p2=strstr(lowercase(buffer),"[/title]"))!=NULL) {
				i=p2-p1-TITLEN; buffer[TITLEN+i]='\0';
				fprintf(subor,"%02d: %s\n",count++,&(buffer[TITLEN-1]));
			}
		}
		fclose(file);
		return 0;
	}
	if (string!=NULL && atoi(string)>0) qm=atoi(string)+1;
	else qm=0;

	if ((qm==0) && (string!=NULL)) {
		count=0;i=0;
		fprintf(subor,"~BB~OL~FY          V Y S L E D O K  H L A D A N I A          ~RS~BK~FW\n\n");
		while (fgets(buffer,4090,file)!=NULL) {
			if ((strstr(lowercase(buffer),"[title]"))!=NULL) i++;
			if (strstr(buffer,string)!=NULL) {
				fprintf(subor,"Najdene v clanku c. %02d\n",i); count++;
				if (count > 15) {
					fprintf(subor,"Prilis vela najdenych zhod!\n");
					fclose(file);
					return 0;
				}
			}
		}
		fprintf(subor,"Najdene celkom %d krat",count);
		fclose(file);
		return 0;
	}
	htmode=0;
	fprintf(subor,"    ~RS~OL~FB____                       __\n~OL~FB   (___  |_ ~FT._ _  ~FB|_  ~FT_ ._  _~FB' | `.. ~FT_ ~FB|  ~FT_~FB' ~FT_.      ~FY.| | |.\n~OL~FY ~FW_ ~FT____) |_.| |_\\ |_.|_'| ||_' |_.'||_\\|_|_||_|`\\_/ ~FW_ ~FY(_|_) ~FW__\n~OL~FW(== ~FBE~RS~FB-zin ~OL~FBt~RS~FBalkeru ~OL~FBA~RS~FBtlantis ~OL~FW======== ~FR%03d ~FW==== ~FT_) ./ ~FW==== ~FY| ~FW==(~FG*~FW)\n\n",issue);
	tagmode=buffer_size=italic=bold=author=0; buffer[0]='\0';

	while ((c=fgetc(file))!=EOF)
	{
		if (c=='\n') c=' ';
		if (isspace(c) && waspace) continue;
		else if (isspace(c) && !waspace) {
			waspace=1;
			if (buffer[0]!='\0') {
				if (howmuch > WRAPCHAR) {
					tmp[0]='\0';
					if (italic) strcat(tmp,"~FT");
					if (bold) strcat(tmp,"~OL");
					if (author) strcat(tmp,"~FG");
					if (isspace(buffer[0])) fprintf(subor,"\n%s%s ",tmp,(buffer+1));
					else fprintf(subor,"\n%s ",(buffer));
					howmuch=strlen(buffer); waspace=1;
					buffer_size=0; buffer[0]='\0'; continue;
				}
				fputs(buffer,subor);
				buffer_size=0; buffer[0]='\0';
			}
		}
		else waspace=0;

		if (c=='<') {htmode=1; continue;}
		else if (c=='>' && htmode) {htmode=0; waspace=1; continue;}
		else if (htmode) continue;
		if (c=='[') {
			tagmode=1;
			if (buffer[0]!='\0') {
				if (howmuch > WRAPCHAR) {
					tmp[0]='\0';
					if (italic) strcat(tmp,"~FT");
					if (bold) strcat(tmp,"~OL");
					if (author) strcat(tmp,"~FG");
					if (isspace(buffer[0])) fprintf(subor,"\n%s%s ",tmp,(buffer+1));
					else fprintf(subor,"\n%s ",(buffer));
					howmuch=strlen(buffer);
					waspace=1;
					buffer_size=0; buffer[0]='\0';
				}
				fputs(buffer,subor);
				buffer_size=0; buffer[0]='\0';
			}
		}
		else if (tagmode && c!=']') {
			tagcache[tagmode-1]=c;
			tagcache[tagmode]='\0';
			tagmode++;
		}
		else if (tagmode) {
			tagmode=0;
			for (i=0;i<TAGS;i++) if (!strncmp(lowercase(tagcache),taglist[i],strlen(tagcache))) break; /*CHMEM*/
			if (i==TAGS) continue;
			if (i==0 && qm >1) qm--;
			else if (i==0 && qm==1) qm=-1;
			if (!qm || qm==1) {
				switch (i) {
					case 0: /* title */
						fprintf(subor,"\n\n~OL~BB~FY   ");
						howmuch=0;
						break;
					case 1: /* /title */
						fprintf(subor,"   ~RS~BK~FW\n\n");
						waspace=1; howmuch=0;
						break;
					case 2: /* bold */
						fprintf(subor,"~OL"); bold=waspace=1;
						break;
					case 3:  /* /bold */
						fprintf(subor,"~RS"); bold=0;
						break;
					case 4: /* eol */
						fprintf(subor,"\n");
						if (bold) fprintf(subor,"~OL");
						if (italic) fprintf(subor,"~FT");
						waspace=1; howmuch=0;
						break;
					case 5: /* tab */
						fprintf(subor,"     ");
						waspace=1; howmuch+=5;
						break;
					case 6: /* italic */
						fprintf(subor,"~FT"); italic=waspace=1;
						break;
					case 7: /* /italic */
						fprintf(subor,"~FW"); italic=0;
						break;
					case 8: /* space */
						fprintf(subor," ");
						waspace=1; howmuch++;
						break;
					case 9: /* author */
						fprintf(subor,"~FG");
						waspace=1; howmuch=0; author=1;
						break;
					case 10: /* /author */
						fprintf(subor,"~RS~FW\n\n");
						waspace=1; howmuch=0; author=0;
						break;
				}
			}
		}
		else if (!qm || qm==1) {
			if (buffer_size < (sizeof(buffer)-2)) { /*CHMEM*/
				buffer[buffer_size]=c;
				buffer[buffer_size+1]='\0';
				buffer_size++;
			}
			howmuch++;
		}
	}
	fclose(file);
	return 0;
}
