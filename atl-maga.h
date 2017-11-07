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

//#define HTML

#define title_path "/usr/home/atlantis/talker/issues"
#define WEB_CGI_PATH "http://www.atlantis.sk/cgy/magazine.cgi"
#define title_name "dialogs"
#define TITLEN sizeof("[title]")
#define WRAPCHAR 79

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

char *taglist[]={
"title","/title",
"bold","/bold",
"eol","tab",
"italic","/italic",
"space",
"author","/author"
};
#define TAGS 11
#ifndef HTML
int htmode;
int waspace=0;
#endif

sprintf (filename,"%s/%s.%03d",title_path,title_name,issue);
#ifndef HTML
if ((file=ropen(filename,"r"))==NULL)
#endif
#ifdef HTML
if ((file=fopen(filename,"r"))==NULL)
#endif
    {
#ifdef HTML
     fprintf(subor,"<center><img src=\"/images/dialogy.jpg\"></center>");
#endif
#ifndef HTML     
     fprintf(subor,"~OL~FRToto vydanie casopisu (cislo ~FW%d~FR) nebolo najdene!~RS\n", issue);
#else
     fprintf(subor,"<h3><font color=\"#ff0000\">Oooops!</font> Toto vydanie casopisu (cislo %d) nebolo najdene!\n", issue);
#endif     
     return 1;
    }

if (string!=NULL && !strcmp(string,"obsah"))
 {
 count=1;
#ifdef HTML
 fprintf(subor,"<center><img src=\"/images/dialogy.jpg\"></center>");
 fprintf(subor,"<table border=0 width=\"100%%\"><tr><td class=\"back\"><font class=\"title\">O B S A H &nbsp; C I S L A &nbsp  %d</td></tr></table><br>",issue);
#else
 fprintf(subor,"~BB~OL~FY          O B S A H   C I S L A  %d          ~RS~BK~FW\n\n",issue);
#endif
 while (fgets(buffer,4090,file)!=NULL)
     {
      if ((p1=strstr(lowercase(buffer),"[title]"))!=NULL && (p2=strstr(lowercase(buffer),"[/title]"))!=NULL) {
	 i=p2-p1-TITLEN; buffer[TITLEN+i]='\0';
#ifdef HTML
	 fprintf(subor,"<a href=\"%s?issue=%d&string=%d\">",WEB_CGI_PATH,issue,count);
#endif
	 fprintf(subor,"%02d: %s\n",count++,&(buffer[TITLEN-1]));
#ifdef HTML
	 fprintf(subor,"</a><BR>");
#endif
	 }
     }
 fclose(file); return 0;
 }
 if (string!=NULL && atoi(string)>0) qm=atoi(string)+1;
 else qm=0;

 if ((qm==0) && (string!=NULL)) {
   count=0;i=0;
#ifdef HTML
   fprintf(subor,"<center><img src=\"/images/dialogy.jpg\"></center>");
   fprintf(subor,"<table border=0 width=\"100%%\"><tr><td class=\"back\"><font class=\"title\">V Y S L E D O K &nbsp; H L A D A N I A</td></tr></table><p>");
#else
   fprintf(subor,"~BB~OL~FY          V Y S L E D O K  H L A D A N I A          ~RS~BK~FW\n\n");
#endif
   while (fgets(buffer,4090,file)!=NULL) {
	if ((strstr(lowercase(buffer),"[title]"))!=NULL) i++;
	if (strstr(buffer,string)!=NULL) {
#ifdef HTML
	   fprintf(subor,"<A HREF=\"%s?issue=%d&string=%d\">",WEB_CGI_PATH,issue,i);
#endif
	   fprintf(subor,"Najdene v clanku c. %02d\n",i); count++;
#ifdef HTML
	   fprintf(subor,"</A><BR>");
#endif
	   if (count > 15) {
#ifdef HTML
	      fprintf(subor,"<HR>");
#endif
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
#ifdef HTML
fprintf (subor,"<HTML><HEAD><STYLE>\nTD.back {background : #002c67}\nTD.black {background: #000000}\nFONT.title {color: #eae500}\nFONT.link {color: #c0e0f0}\n</STYLE></head>\n<body bgcolor=\"#000000\" text=\"#c0d0f0\" link=\"#e0d070\" vlink=\"#909090\" alink=\"#ffffff\"><center><img src=\"/images/dialogy.jpg\"></center>");

#else
htmode=0;
fprintf(subor,"    ~RS~OL~FB____                       __\n~OL~FB   (___  |_ ~FT._ _  ~FB|_  ~FT_ ._  _~FB' | `.. ~FT_ ~FB|  ~FT_~FB' ~FT_.      ~FY.| | |.\n~OL~FY ~FW_ ~FT____) |_.| |_\\ |_.|_'| ||_' |_.'||_\\|_|_||_|`\\_/ ~FW_ ~FY(_|_) ~FW__\n~OL~FW(== ~FBE~RS~FB-zin ~OL~FBt~RS~FBalkeru ~OL~FBA~RS~FBtlantis ~OL~FW======== ~FR%03d ~FW==== ~FT_) ./ ~FW==== ~FY| ~FW==(~FG*~FW)\n\n",issue);
#endif

tagmode=buffer_size=italic=bold=author=0; buffer[0]='\0';

while ((c=fgetc(file))!=EOF)
  {
#ifndef HTML
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
#endif
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
#ifndef HTML
   	        waspace=1;
#endif
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
	if (!qm || qm==1) switch (i) {
	   case 0: /* title */
#ifdef HTML
		   fprintf(subor,"<p><table border=0 width=\"100%%\"><tr><td class=\"back\"><font class=\"title\" size=\"+1\">");
#else
		   fprintf(subor,"\n\n~OL~BB~FY   ");
		   howmuch=0;
#endif
		   break;
	   case 1: /* /title */
#ifdef HTML
		   fprintf(subor,"</font></td></tr></table><p>");
#else
		   fprintf(subor,"   ~RS~BK~FW\n\n");
		   waspace=1; howmuch=0;
#endif
		   break;
	   case 2: /* bold */
#ifdef HTML
		   fprintf(subor,"<B>");
#else
		   fprintf(subor,"~OL"); bold=waspace=1;

#endif
		   break;
	  case 3:  /* /bold */
#ifdef HTML
		   fprintf(subor,"</B>");
#else
		   fprintf(subor,"~RS"); bold=0;
#endif
		   break;
	  case 4: /* eol */
#ifdef HTML
		   fprintf(subor,"<BR>\n");
#else
		   fprintf(subor,"\n");
		   if (bold) fprintf(subor,"~OL");
		   if (italic) fprintf(subor,"~FT");
		   waspace=1; howmuch=0;
#endif
		   break;
	  case 5: /* tab */
#ifdef HTML
		   fprintf(subor,"<div align=\"justify\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
#else
		   fprintf(subor,"     ");
		   waspace=1; howmuch+=5;
#endif
		   break;
	  case 6: /* italic */
#ifdef HTML
		   fprintf(subor,"<i>");
#else
		   fprintf(subor,"~FT"); italic=waspace=1;
#endif
		   break;
	  case 7: /* /italic */
#ifdef HTML
		   fprintf(subor,"</i>");
#else
		   fprintf(subor,"~FW"); italic=0;
#endif
		   break;
	  case 8: /* space */
#ifdef HTML
		   fprintf(subor,"&nbsp;");
#else
		   fprintf(subor," ");
		   waspace=1; howmuch++;
#endif
		   break;

	  case 9: /* author */
#ifdef HTML
		  //fprintf(subor,"<div align=\"right\"><strong>=");
	            fprintf(subor,"<div align=\"left\">");
#else
		  // fprintf(subor,"                                                 ~OL~FY=");
		   fprintf(subor,"~FG");
		   waspace=1; howmuch=0; author=1;
#endif
		   break;

	  case 10: /* /author */
#ifdef HTML
		  // fprintf(subor,"=</strong></div><p>\n");
		  fprintf(subor,"</div><p>\n");
#else
		   //fprintf(subor,"=~RS~FW\n\n");
		   fprintf(subor,"~RS~FW\n\n");
		   waspace=1; howmuch=0; author=0;
#endif
		   break;

	  }
       }
   else if (!qm || qm==1) {
	    if (buffer_size < (sizeof(buffer)-2)) {  /*CHMEM*/
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
