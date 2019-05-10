#include "atl-head.h"
#define PISKVORKY_MIN_MOVES 25
#define PISKVORKY_MIN_TLT 12

char *karta_znak[8]={"7","8","9","X","D","H","K","E"};
char *karta_farba[4]={"~RS~FG98","~OL~FRv","~RS~FYA","~OL~FY()"};
char *karta_pict[8]={" VII "," VIII"," IX  ","  X  "," :)  "," ;-) "," }:) "," ESO "};
char *farba_meno[4]={"zelen","cerven","zalud","gulu"};

int clovece_plocha[]={
	49,74,72,44,44,29,30,31,44,44,83,84,49,
	49,73,71,44,44,28,85,32,44,44,81,82,49,
	49,44,44,44,44,27,86,33,44,44,44,44,49,
	49,44,44,44,44,26,87,34,44,44,44,44,49,
	49,21,22,23,24,25,88,35,36,37,38,39,49,
	49,20,75,76,77,78,44,58,57,56,55,40,49,
	49,19,18,17,16,15,68, 5, 4, 3, 2, 1,49,
	49,44,44,44,44,14,67, 6,44,44,44,44,49,
	49,44,44,44,44,13,66, 7,44,44,44,44,49,
	49,62,61,44,44,12,65, 8,44,44,51,53,49,
	49,64,63,44,44,11,10, 9,44,44,52,54,49
};

char *clovece_col[]={"~FT","~FG","~FY","~FR","*"};

extern void log_game(char *str)
{
	FILE *fp;
	char filename[255];

	sprintf(filename,"%s%c%s",LOGFILES,DIRSEP,GAMES_LOG);
	if (!system_logging || !(fp=ropen(filename,"a")))
		return;
	fprintf(fp,"%02d/%02d %02d:%02d:%02d: %s",tmday,tmonth+1,thour,tmin,tsec,str);
	fclose(fp);
}

int shsunk;

extern void lod(UR_OBJECT user)
{
	UR_OBJECT u=NULL;
	int shdef,x,y;
	int neukazuj=0;
	int cz;
	char filename[255];
	char temp[500];
	char a,b;
	int p;
	FILE *fp;

	if ((word_count<2) || (word_count>3)) {
		write_user(user,"Pouzi: .lodicky <user> [old] <X Y> <stop>\n\n");
		write_user(user,"Napr.: .lod Jano      - vyzves Jana do boja\n");
		write_user(user,"       .lod Jano old  - vyzves Jana, ale ty pouzijes minule rozostavenie lodi\n");
		write_user(user,"       .lod Jozo      - prijmes vyzvu od Joza; zacne sa rozmiestnovanim lodi\n");
		write_user(user,"       .lod Jozo old  - prijmes vyzvu, pouzijes rozmiestnenie lodi z min. hry\n");
		write_user(user,"       .lod 3 6       - Ak editujes rozmiestnenie lodi, tymto postavis cast\n");
		write_user(user,"                        lode na poziciu [3,6]; ak uz prebieha samotna hra,\n");
		write_user(user,"                        takto vypalis strelu na nepriatelske pole, na [3,6]\n");
		write_user(user,"       .lod stop      - stopnes hru, alebo rozmiestnovanie lodi.\n");
		write_user(user,"       .lod show      - ukaze stav hry.\n");
		write_user(user,"       .lod save      - ulozi aktualnu poziciu v hre pre neskorsie pouzitie.\n");
		write_user(user,"       .lod load      - vyvola ulozenu poziciu v hre.\n\n");
		write_user(user,"Viac informacii o hre cez .pomoc lodicky\n");
		return;
	}
	if (!strcmp(word[1],"save")) {
		if ((user->game!=4) || (user->shmode!=1)) {
			write_user(user,"Teraz nemozes ulozit stav hry!\n");
			return;
		}
		if ((u=get_user(user->uname))==NULL) {
			write_user(user,"Spoluhrac sa ti odhlasil!\n");
			user->game=0;
			user->uname[0]='\0';
			user->shmode=0;
			user->joined=0;
			user->shbuild=0;
			user->shstatus=0;
			return;
		}
		if (u->shmode!=1) {
			write_user(user,"Teraz nemozes ukladat stav hry.\n");
			return;
		}
		sprintf(filename,"%s%c%s.sav",LODICKY_SAVE_DIR,DIRSEP,user->name);
		if (!(fp=ropen(filename,"w"))) {
			write_user(user,"Prepac, nemozem otvorit subor na zapis.\n");
			return;
		}
		fprintf(fp,"%s\n",u->name);
		for (x=0; x<10; x++) {
			for (y=0; y<10; y++) {
				a=user->shtable[x][y];
				b=u->shtable[x][y];
				if (a==' ')
					a='-';
				if (b==' ')
					b='-';
				fprintf(fp,"%c%c",a,b);
			}
		}
		fprintf(fp,"\n");
		fprintf(fp,"%d %d %d\n",user->zasah,u->zasah,user->tah);
		fclose(fp);
		write_user(user,"Aktualna herna pozicia bola ulozena.\n");
		sprintf(text,"~FM%s ulozil%s aktualnu hernu poziciu.\n",user->name,pohl(user,"","a"));
		writecent(u,text);
		return;
	}
	if (!strcmp(word[1],"load")) {
		if (user->game!=0) {
			write_user(user,"Uz hras nejaku hru, najprv ju musis stopnut.\n");
			return;
		}
		sprintf(filename,"%s/%s.sav",LODICKY_SAVE_DIR,user->name);
		if (!(fp=ropen(filename,"r"))) {
			write_user(user,"Nemas ulozenu ziadnu poziciu!\n");
			return;
		}
		fscanf(fp,"%s",user->uname);
		if (!(u=get_user(user->uname))) {
			sprintf(text,"Hrac s ktorym si vtedy hral nieje teraz prihlaseny! (%s)\n",user->uname);
			write_user(user,text);
			user->game=0;
			user->uname[0]='\0';
			user->shmode=0;
			user->joined=0;
			user->shbuild=0;
			user->shstatus=0;
			fclose(fp);
			return;
		}
		if (u->game!=0) {
			sprintf(text,"%s teraz hra inu hru, skus neskor.\n",u->name);
			write_user(user,text);
			fclose(fp);
			return;
		}
		fscanf(fp,"%s",temp);
		p=0;
		for (x=0; x<10; x++) {
			for (y=0; y<10; y++) {
				if (temp[p]=='-')
					temp[p]=' ';
				user->shtable[x][y]=temp[p];
				p++;
				if (temp[p]=='-')
					temp[p]=' ';
				u->shtable[x][y]=temp[p];
				p++;
			}
		}
		fscanf(fp,"%d %d %d",&user->zasah,&u->zasah,&user->tah);
		fclose(fp);
		user->game=4;
		u->game=4;
		user->shmode=1;
		u->shmode=1;
		if (user->tah)
			u->tah=0;
		else
			u->tah=1;
		strcpy(u->uname,user->name);
		shprn2(user,u);
		shprn2(u,user);
		sprintf(text,"~OL~FGVRATIL%s SI ULOZENU HRU.\n",pohl(user,"","A"));
		writecent(user,text);
		sprintf(text,"~OL~FG%s VRATIL%s ULOZENU HRU.\n",user->name,pohl(user,"","A"));
		writecent(u,text);
		return;
	}
	if ((word_count==3) && (user->shmode==0) && (strcmp(word[2],"old"))) {
		write_user(user,"Nehras ani needitujes!\n");
		return;
	}
	if (!strcmp(word[1],"stop")) {
		if (user->shmode==0) {
			write_user(user,"Nemas co stopnut, lebo nehras ani needitujes!\n");
			return;
		}
		if (!(u=get_user(user->uname))) {
			write_user(user,"Spoluhrac sa ti odhlasil!\n");
			user->game=0;
			user->uname[0]='\0';
			user->shmode=0;
			user->joined=0;
			user->shbuild=0;
			user->shstatus=0;
			return;
		}
		if (user->shmode==2) {
			sprintf(text,"Dobre, vzdal%s si to este pred bojom!\n",pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"%s to vzdal%s este pred bojom, uz nemusis pokracovat v rozmiestnovani.\n",user->name,pohl(user,"","a"));
			write_user(u,text);
		}
		if (user->shmode==1) {
			sprintf(text,"Zbabelo si sa vzdal%s!\n",pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"~OL~FR%s videl%s, ze nema sancu, preto sa vzdal%s!!\n\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
			writecent(u,text);
			sprintf(text,"~OL~FY~BB G A M E  O V E R ~RS~BK~FW\n");
			writecent(user,text);
			writecent(u,text);
		}
		user->game=0;
		u->game=0;
		user->uname[0]='\0';
		u->uname[0]='\0';
		user->shmode=0;
		u->shmode=0;
		user->joined=0;
		u->joined=0;
		return;
	}
	if (!strcmp(word[1],"show")) {
		if ((user->shmode==0) || (user->game!=4)) {
			write_user(user,"Nehras lodicky!\n");
			return;
		}
		if (!(u=get_user(user->uname))) {
			write_user(user,"Spoluhrac sa ti odhlasil!\n");
			user->game=0;
			user->uname[0]='\0';
			user->shmode=0;
			user->joined=0;
			user->shbuild=0;
			user->shstatus=0;
			return;
		}
		if (user->shmode==2) {
			shprn(user);
			return;
		}
		if (user->shmode==1) {
			shprn2(user,u);
			sprintf(text,"~OL~FTToto je aktualna bojova situacia.\n");
			writecent(user,text);
			return;
		}
	}
	if ((word_count==2) && (user->shmode!=0)) {
		write_user(user,"Nespravne pouzity prikaz!\n");
		return;
	}
	if (((word_count==2) || (word_count==3)) && (user->shmode==0)) {
		if (!(u=get_user(word[1]))) {
			write_user(user,notloggedon);
			return;
		}
		if (u==user) {
			write_user(user,"Nemozes vyzvat seba na suboj! :-)\n");
			return;
		}
		if (((user->game!=0) && (user->game!=4)) || ((u->game!=0) && (u->game!=4)))  {
			write_user(user,"Ty alebo tvoj partner uz hra inu hru.\n");
			return;
		}
		if (strcmp(u->name,user->uname)) {
			if (u->shmode!=0) {
				sprintf(text,"%s uz bojuje s niekym inym.\n",u->name);
				write_user(user,text);
				return;
			}
			if (!strcmp(word[2],"old")) {
				if ((shreset(user))!=20) {
					write_user(user,"Nemas rozmiestnene lodstvo!\n");
					return;
				}
				user->joined=10;
			}
			strcpy(u->uname,user->name);
			user->game=4;
			sprintf(text,"Vyzval%s si %s k namornej bitke. Cakaj na odpoved ...\n",pohl(user,"","a"),sklonuj(u,4));
			write_user(user,text);
			if (user->joined==10)
				write_user(user,"Pouzijes rozmiestnenie svojho lodstva z minulej hry.\n");
			sprintf(text,"~OL%s Ta vyzval%s k namornej bitke! Ak chces hrat, napis: ~FT.lod %s\n",user->name,pohl(user,"","a"),user->name);
			write_user(u,text);
			return;
		}
		if (!strcmp(word[2],"old")) {
			if ((shreset(user))!=20) {
				write_user(user,"Nemas rozmiestnene lodstvo!\n");
				return;
			}
			user->joined=10;
		}
		if ((user->joined==10) && (u->joined!=10)) {
			shprn(user);
			user->shmode=1;
			sprintf(text,"Toto je Tvoje lodstvo. Pockaj, kym %s rozostavi svoje lode.\n",u->name);
			write_user(user,text);
		}
		if ((u->joined==10) && (user->joined!=10)) {
			shprn(u);
			u->shmode=1;
			sprintf(text,"Toto je Tvoje lodstvo. Pockaj, kym %s rozostavi svoje lode.\n",user->name);
			write_user(u,text);
		}
		if ((u->joined==10) && (user->joined==10)) {
			u->shmode=1;
			user->shmode=1;
			user->tah=1;
			u->tah=0;
			user->zasah=0;
			u->zasah=0;
			sprintf(filename,"%s%cships-title",DATAFILES,DIRSEP);
			show_user(user,filename);
			show_user(u,filename);
			shprn2(user,u);
			shprn2(u,user);
			strcpy(user->uname,u->name);
			strcpy(u->uname,user->name);
			sprintf(text,"~OL~FY%s rozostavil%s lodstvo, hra sa moze zacat!\n",user->name,pohl(user,"","a"));
			writecent(u,text);
			sprintf(text,"~OL~FYVsetko je v poriadku, hra sa moze zacat!\n");
			writecent(user,text);
			return;
		}
		if (u->joined!=10) {
			sprintf(text,"~OL~FT%s suhlasil%s s namornou bitkou, takze zaciname!\n~RS~FTNajprv rozmiestni svoje lodstvo...\n",user->name,pohl(user,"","a"));
			write_user(u,text);
		}
		if (user->joined!=10)
			write_user(user,"~OL~FTVyborne, zaciname! Najprv rozmiestni svoje lodstvo ...\n");
		user->game=4;
		u->game=4;
		user->tah=1;
		u->tah=0;
		if (user->joined!=10)
			user->shmode=2;
		if (u->joined!=10)
			u->shmode=2;
		user->shstatus=0;
		u->shstatus=0;
		user->shbuild=0;
		u->shbuild=0;
		if (user->joined!=10)
			vynuluj_lod(user);
		if (u->joined!=10)
			vynuluj_lod(u);
		user->zasah=0;
		u->zasah=0;
		if (user->joined!=10)
			shprn(user);
		if (u->joined!=10)
			shprn(u);
		strcpy(user->uname,u->name);
		strcpy(u->uname,user->name);
		sprintf(text,"~FGUloz ~OLBITEVNU LOD ~RS~FG(4 policka)\n");
		if (user->joined!=10)
			write_user(user,text);
		if (u->joined!=10)
			write_user(u,text);
		return;
	}

	if (user->shmode!=0) {
		if (!(u=get_user(user->uname))) {
			write_user(user,"Spoluhrac odisiel!\n");
			user->shmode=0;
			return;
		}
	}
	if ((word_count==3) && (user->shmode==2)) {
		y=atoi(word[1]);
		x=atoi(word[2]);
		if ((y>9) || (y<0) || (x>9) || (x<0)) {
			write_user(user,"Cisla musia byt z intervalu 0-9!\n");
			return;
		}
		switch (user->shstatus) {
			case 0:
				write_user(user,"Staviam BITEVNU LOD ... ");
				shdef=4;
				break;
			case 1:
				write_user(user,"Staviam prvy KRIZNIK ... ");
				shdef=3;
				break;
			case 2:
				write_user(user,"Staviam druhy KRIZNIK ... ");
				shdef=3;
				break;
			case 3:
				write_user(user,"Staviam prvy TORPEDOBOREC ... ");
				shdef=2;
				break;
			case 4:
				write_user(user,"Staviam druhy TORPEDOBOREC ... ");
				shdef=2;
				break;
			case 5:
				write_user(user,"Staviam treti TORPEDOBOREC ... ");
				shdef=2;
				break;
			case 6:
				write_user(user,"Staviam prvu PONORKU ... ");
				shdef=1;
				break;
			case 7:
				write_user(user,"Staviam druhu PONORKU ... ");
				shdef=1;
				break;
			case 8:
				write_user(user,"Staviam tretiu PONORKU ... ");
				shdef=1;
				break;
			default:
				write_user(user,"Staviam stvrtu PONORKU ... ");
				shdef=1;
				break;
		}
		if (shtest(user,x,y))
			write_user(user,"~OL~FRNemozu byt dve lode vedla seba!\n");
		else if ((shcount(user,x,y) <= 1) && user->shtable[y][x]=='X') {
			user->shtable[y][x]='.';
			user->shbuild--;
			shprn(user);
			neukazuj=1;
		}
		else if (user->shtable[y][x]=='X')
			write_user(user,"~OL~FRNemozes prestavavat lod od stredu!\n");
		else if (user->shbuild && shcount(user,x,y)==0)
			write_user(user,"~OL~FRNemozes stavat lod po kuskoch, ale pekne zasebou!\n");
		else {
			user->shtable[y][x]='X';
			user->shbuild++;
			if (user->shbuild==shdef) {
				user->shstatus++;
				user->shbuild=0;
				shset(user);
				shprn(user);
				neukazuj=1;
				write_user(user,"~FGLod dostavana! ");
				switch (user->shstatus) {
					case 0:
						write_user(user,"~FGTeraz postav ~OLBITEVNU LOD ~RS~FG(4 policka)\n");
						break;
					case 1:
						write_user(user,"~FGTeraz postav prvy ~OLKRIZNIK ~RS~FG(3 policka)\n");
						break;
					case 2:
						write_user(user,"~FGTeraz postav druhy ~OLKRIZNIK ~RS~FG(3 policka)\n");
						break;
					case 3:
						write_user(user,"~FGTeraz postav prvy ~OLTORPEDOBOREC ~RS~FG(2 policka)\n");
						break;
					case 4:
						write_user(user,"~FGTeraz postav druhy ~OLTORPEDOBOREC ~RS~FG(2 policka)\n");
						break;
					case 5:
						write_user(user,"~FGTeraz postav treti ~OLTORPEDOBOREC ~RS~FG(2 policka)\n");
						break;
					case 6:
						write_user(user,"~FGTeraz postav prvu ~OLPONORKU ~RS~FG(1 policko)\n");
						break;
					case 7:
						write_user(user,"~FGTeraz postav druhu ~OLPONORKU ~RS~FG(1 policko)\n");
						break;
					case 8:
						write_user(user,"~FGTeraz postav tretiu ~OLPONORKU ~RS~FG(1 policko)\n");
						break;
					case 9:
						write_user(user,"~FGTeraz postav stvrtu ~OLPONORKU ~RS~FG(1 policko)\n");
						break;
					default:
						write_user(user,"~OL~FGFlotila rozmiestnena!\n");
						user->shmode=1;
						sprintf(filename,"%s%cships-title",DATAFILES,DIRSEP);
						show_user(user,filename);
						if (u->shmode==2) {
							sprintf(text,"%s uz rozostavil%s svoje lodstvo! Caka sa len na Teba ...\n",user->name,pohl(user,"","a"));
							write_user(u,text);
							sprintf(text,"%s este rozostavuje lode ... chvilu pockaj.\n",u->name);
							write_user(user,text);
						}
						if (u->shmode==1) {
							show_user(u,filename);
							shprn2(user,u);
							shprn2(u,user);
							sprintf(text,"~OL~FY%s rozostavil%s lodstvo, hra sa moze zacat!\n",user->name,pohl(user,"","a"));
							writecent(u,text);
							sprintf(text,"~OL~FYVsetko je v poriadku, hra sa moze zacat!\n");
							writecent(user,text);
						}
						break;
				}

			}
		}
		if (!neukazuj)
			shprn(user);
		return;
	}
	if ((word_count==3) && (u->shmode==2)) {
		write_user(user,"Cakaj, spoluhrac este rozostavuje svoje lodstvo!\n");
		return;
	}
	if ((word_count==3) && (user->shmode==1) && (u->shmode==1)) {
		if (!user->tah) {
			write_user(user,"Pockaj az budes na rade!\n");
			return;
		}
		y=atoi(word[1]);
		x=atoi(word[2]);
		if ((y>9) || (y<0) || (x>9) || (x<0)) {
			write_user(user,"Cisla musia byt z intervalu 0-9!\n");
			return;
		}
		if (u->shtable[y][x]=='.') {
			u->shtable[y][x]=' ';
			user->tah=0;
			u->tah=1;
			shprn2(user,u);
			shprn2(u,user);
			sprintf(text,"~OL~FBTvoja strela smerovala na sire more a minula ciel!\n");
			writecent(user,text);
			sprintf(text,"~OL~FBNepriatelska strela minula ciel a stratila sa v mori.\n");
			writecent(u,text);
			return;
		}
		if ((u->shtable[y][x]==' ') || (u->shtable[y][x]=='*') || (u->shtable[y][x]=='+')) {
			sprintf(text,"Tam si uz raz triafal%s!\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		if (u->shtable[y][x]=='#') {
			user->zasah+=1;
			cz=shsunken(u,x,y);
			shprn2(user,u);
			shprn2(u,user);
			switch(cz) {
				case 0:
					sprintf(text,"~OL~FYZasah!!!!\n");
					writecent(user,text);
					sprintf(text,"~FYBuuuum! %s ti zasiah%s cast lode!\n",user->name,pohl(user,"ol","la"));
					writecent(u,text);
					break;
				case 1:
					sprintf(text,"~OL~FYZasah!!!! ~OL~FRPONORKA POTOPENA!\n");
					writecent(user,text);
					sprintf(text,"~FYTresk! Tvoja ponorka sa rozletela na marne kusky!\n");
					writecent(u,text);
					break;
				case 2:
					sprintf(text,"~OL~FYZasah!!!! ~OL~FRTORPEDOBOREC POTOPENY!\n");
					writecent(user,text);
					sprintf(text,"~FYBang! Nepriatelska strela ti prevrtala torpedoborec!\n");
					writecent(u,text);
					break;
				case 3:
					sprintf(text,"~OL~FYZasah!!!! ~OL~FRKRIZNIK POTOPENY!\n");
					writecent(user,text);
					sprintf(text,"~FYTresk! Vrak kriznika sa vydal na dlhu cestu na dno mora ...!\n");
					writecent(u,text);
					break;
				case 4:
					sprintf(text,"~OL~FYZasah!!!! ~OL~FRBITEVNA LOD POTOPENA!\n");
					writecent(user,text);
					sprintf(text,"~FYTresk! Bitevna lod sa zakymacala a s rachotom zmizla pod hladinou!\n");
					writecent(u,text);
					break;
			}
			if (user->zasah==20) {
				sprintf(text,"~OL~FTVyhral%s si! Lodstvo '~FW%s~FT' bolo navzdy pochovane pod morom ...\n\n",pohl(user,"","a"),sklonuj(u,2));
				writecent(user,text);
				sprintf(text,"~OL~FRCele Tvoje lodstvo zmizlo pod morskou hladinou ... PREHRAL%s SI!!! \n\n",pohl(u,"","A"));
				writecent(u,text);
				sprintf(text,"~OL~FY~BB G A M E  O V E R ~RS~BK~FW\n");
				writecent(user,text);
				writecent(u,text);
				sprintf(filename,"%s%cships-winner",DATAFILES,DIRSEP);
				show_user(user,filename);
				sprintf(filename,"%s%cships-loser",DATAFILES,DIRSEP);
				show_user(u,filename);
				user->game=0;
				u->game=0;
				user->uname[0]='\0';
				u->uname[0]='\0';
				user->shmode=0;
				u->shmode=0;
				sprintf(text,"Lode: %s/%s\n",user->name,u->name);
				log_game(text);
			}
		}
	}
}

extern void writecent(UR_OBJECT user,char txt[10000])
{
	char pom[10000];
	int dlza;

	dlza=strlen(txt);
	sprintf(pom,"%*s",(40+(dlza/2)+((colour_com_count(txt)))/2),txt);
	write_user(user,pom);
}

extern int shsink(UR_OBJECT user,int x,int y,int count)
{
	user->shtable[y][x]='@';
	if (
			((x+1<10) && (user->shtable[y][x+1]=='#'))
			||
			((y+1<10) && (user->shtable[y+1][x]=='#'))
			||
			((x-1>=0) && (user->shtable[y][x-1]=='#'))
			||
			((y-1>=0) && (user->shtable[y-1][x]=='#'))
	   )
		shsunk=0;
	if ((x+1<10) && (user->shtable[y][x+1]=='*'))
		count=shsink(user,x+1,y,++count);
	if ((y+1<10) && (user->shtable[y+1][x]=='*'))
		count=shsink(user,x,y+1,++count);
	if ((x-1>=0) && (user->shtable[y][x-1]=='*'))
		count=shsink(user,x-1,y,++count);
	if ((y-1>=0) && (user->shtable[y-1][x]=='*'))
		count=shsink(user,x,y-1,++count);
	return count;
}

extern int shsunken(UR_OBJECT user,int x,int y)
{
	int i,j,z;

	shsunk=1;
	user->shtable[y][x]='*';
	z=shsink(user,x,y,1);
	for (i=0;i<10;i++) {
		for (j=0;j<10;j++) {
			if (user->shtable[i][j]=='@') {
				if (shsunk)
					user->shtable[i][j]='+';
				else
					user->shtable[i][j]='*';
			}
		}
	}
	return shsunk*z;
}

extern void shset(UR_OBJECT user)
{
	int i,j;

	for (j=0;j<10;j++)
		for (i=0;i<10;i++)
			if (user->shtable[j][i]=='X')
				user->shtable[j][i]='#';
}

extern int shreset(UR_OBJECT user)
{
	int i,j,poc=0;

	for (j=0;j<10;j++)
		for (i=0;i<10;i++) {
			if (user->shtable[j][i]=='#')
				poc++;
			if (user->shtable[j][i]=='+') {
				user->shtable[j][i]='#';
				poc++;
			}
			if (user->shtable[j][i]=='*') {
				user->shtable[j][i]='#';
				poc++;
			}
			if (user->shtable[j][i]==' ')
				user->shtable[j][i]='.';
		}
	return poc;
}

extern int shtest(UR_OBJECT user,int x,int y)
{
	int r=0;

	if (user->shtable[y][x]=='#')
		r++;
	if ((x+1<10) && (user->shtable[y][x+1]=='#'))
		r++;
	if ((y+1<10) && (user->shtable[y+1][x]=='#'))
		r++;
	if ((x-1>=0) && (user->shtable[y][x-1]=='#'))
		r++;
	if ((y-1>=0) && (user->shtable[y-1][x]=='#'))
		r++;
	if ((x+1<10) && (y+1<10)&&(user->shtable[y+1][x+1]=='#'))
		r++;
	if ((x+1<10) && (y-1>=0) && (user->shtable[y-1][x+1]=='#'))
		r++;
	if ((x-1>=0) && (y+1<10) && (user->shtable[y+1][x-1]=='#'))
		r++;
	if ((x-1>=0) && (y-1>=0) && (user->shtable[y-1][x-1]=='#'))
		r++;
	return r;
}

extern int shcount(UR_OBJECT user,int x,int y)
{
	int r=0;

	if ((x+1<10) && (user->shtable[y][x+1]=='X'))
		r++;
	if ((y+1<10) && (user->shtable[y+1][x]=='X'))
		r++;
	if ((x-1>=0) && (user->shtable[y][x-1]=='X'))
		r++;
	if ((y-1>=0) && (user->shtable[y-1][x]=='X'))
		r++;
	return r;
}

extern void shprn(UR_OBJECT user)
{
	int i,j;

	sprintf(text,"\n   ~OL~FW~BB STAVANIE LODSTVA ~RS~FW~BK\n");
	write_user(user,text);
	sprintf(text,"~RS~FT  0 1 2 3 4 5 6 7 8 9  ");
	write_user(user,text);
	for (j=0;j<10;j++) {
		sprintf(text,"\n~RS~FT%d ",j);
		write_user(user,text);
		for (i=0;i<10;i++) {
			if (user->shtable[j][i]=='.')
				sprintf(text,"~OL~FB%c ",user->shtable[j][i]);
			else if (user->shtable[j][i]=='X')
				sprintf(text,"~RS~FWX ");
			else
				sprintf(text,"~OL~FW%c ",user->shtable[j][i]);
			write_user(user,text);
		}
		sprintf(text,"~RS~FT%d",j);
		write_user(user,text);
	}
	sprintf(text,"\n~RS~FT  0 1 2 3 4 5 6 7 8 9\n");
	write_user(user,text);
}

extern void shprn2(UR_OBJECT user,UR_OBJECT u)
{
	int i,j;

	sprintf(text,"\n~RS~FT      0 1 2 3 4 5 6 7 8 9  ~OL~BB~FW B A T T L E  S H I P S ~RS~BK~FT  0 1 2 3 4 5 6 7 8 9");
	write_user(user,text);
	for (j=0;j<10;j++) {
		sprintf(text,"\n    ~RS~FT%d~OL ",j);
		write_user(user,text);
		for (i=0;i<10;i++) {
			if (user->shtable[j][i]=='.')
				sprintf(text,"~OL~FB%c ",user->shtable[j][i]);
			else if (user->shtable[j][i]=='#')
				sprintf(text,"~OL~FW%c ",user->shtable[j][i]);
			else if (user->shtable[j][i]=='*')
				sprintf(text,"~OL~FY%c ",user->shtable[j][i]);
			else if (user->shtable[j][i]=='+')
				sprintf(text,"~OL~FR%c ",user->shtable[j][i]);
			else sprintf(text,"  ");
			write_user(user,text);
		}
		if (j==2)
			sprintf(text,"~RS~FT%d  ~FY+------~OL~FMZASAHY~RS~FY------+  ~FT%d ",j,j);
		else if (j==3)
			sprintf(text,"~RS~FT%d  ~FY| ~OL~FG%-12s ~RS~FY:~OL~FW%2d ~RS~FY|  ~FT%d ",j,user->name,user->zasah,j);
		else if (j==4)
			sprintf(text,"~RS~FT%d  ~FY| ~OL~FG%-12s ~RS~FY:~OL~FW%2d ~RS~FY|  ~FT%d ",j,u->name,u->zasah,j);
		else if (j==5)
			sprintf(text,"~RS~FT%d  ~FY+------------------+  ~FT%d ",j,j);
		else if ((j==7) && (user->tah))
			sprintf(text,"~RS~FT%d      ~OL~FYSI NA TAHU!       ~RS~FT%d ",j,j);
		else if (j==7)
			sprintf(text,"~RS~FT%d  ~FGNa tahu je protihrac. ~FT%d ",j,j);
		else
			sprintf(text,"~RS~FT%d                        %d ",j,j);
		write_user(user,text);
		for (i=0;i<10;i++) {
			if ((u->shtable[j][i]==' ') || (u->shtable[j][i]=='*') || (u->shtable[j][i]=='+')) {
				if (u->shtable[j][i]=='*')
					sprintf(text,"~OL~FY%c ",u->shtable[j][i]);
				if (u->shtable[j][i]=='+')
					sprintf(text,"~OL~FR%c ",u->shtable[j][i]);
				if (u->shtable[j][i]==' ')
					sprintf(text,"  ");
			}
			else
				sprintf(text,"~OL~FB. ");
			write_user(user,text);
		}
		sprintf(text,"~RS~FT%d",j);
		write_user(user,text);
	}
	sprintf(text,"\n~RS~FT      0 1 2 3 4 5 6 7 8 9                            0 1 2 3 4 5 6 7 8 9\n");
	write_user(user,text);
}

extern void vynuluj_lod(UR_OBJECT user)
{
	int x,y;

	for (y=0;y<10;y++)
		for (x=0;x<10;x++)
			user->shtable[y][x]='.';
}

extern void lab(UR_OBJECT user)
{
	int st;
	int ruma;
	FILE *fp;
	char filename[80];
	st=0;

	if (word_count<2) {
		write_user(user,"Pouzi: .labyrint [start/stop/quit/show] [s/j/v/z/h/d]\n\n");
		write_user(user,"       .lab start              - zacne hru\n");
		write_user(user,"       .lab stop (quit)        - ukonci hru\n");
		write_user(user,"       .lab show               - ukaze aktualnu poziciu\n");
		write_user(user,"       .lab s (j, v, z, h, d)  - pohyb na sever (juh, vychod, zapad, hore, dolu)\n\n");
		return;
	}
	if ((!strcmp(word[1],"update")) && (user->level>=KIN)) {
		if (lab_load()) {
			write_user(user,"Chyba pri nacitavani labyrintu.\n");
			lab_room.dostupny=0;
			return;
		}
		write_user(user,"O.K., labyrint bol inicializovany!\n");
		lab_room.dostupny=1;
		return;
	}
	if (!lab_room.dostupny) {
		write_user(user,"Sorry, labyrint je docasne nedostupny, lebo chyba scenar.\n");
		return;
	}
	if (!strcmp(word[1],"start")) {
		if (user->lab) {
			write_user(user,"Uz hras labyrint! Pouzi: .labyrint stop pre ukoncenie.\n");
			return;
		}
		user->lab=lab_room.start;
		st=1;
	}
	if (!strcmp(word[1],"--")) {
		if (word_count<3) {
			write_user(user,"Heh, zabudol si cislo! :>>\n");
			return;
		}
		ruma=atoi(word[2]);
		if ((!ruma) || (ruma<0) || (ruma>LAB_ROOMS)) {
			write_user(user,"Heh, nespravne cislo! :>>\n");
			return;
		}
		user->lab=ruma;
		st=1;
	}
	if ((!strcmp(word[1],"quit")) || (!strcmp(word[1],"stop"))) {
		if (!user->lab) {
			write_user(user,"Ved nehras labyrint!\n");
			return;
		}
		sprintf(text,"Vzdal%s si sa!\n",pohl(user,"","a"));
		write_user(user,text);
		user->lab=0;
		return;
	}
	if (!strcmp(word[1],"show")) {
		if (!user->lab) {
			write_user(user,"Nehras labyrint. Ak chces hrat, napis: .labyrint start\n");
			return;
		}
		st=1;
	}
	if ((!user->lab) && (!st)) {
		write_user(user,"Nemas zacatu hru. Pouzi: .labyrint start\n");
		return;
	}
	if (!st) {
		switch (tolower(word[1][0])) {
			case 's':
				if (lab_room.sever[user->lab]) {
					user->lab=lab_room.sever[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			case 'j':
				if (lab_room.juh[user->lab]) {
					user->lab=lab_room.juh[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			case 'v':
				if (lab_room.vychod[user->lab]) {
					user->lab=lab_room.vychod[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			case 'z':
				if (lab_room.zapad[user->lab]) {
					user->lab=lab_room.zapad[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			case 'h':
				if (lab_room.hore[user->lab]) {
					user->lab=lab_room.hore[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			case 'd':
				if (lab_room.dole[user->lab]) {
					user->lab=lab_room.dole[user->lab];
					write_user(user,"\n~OL~FMOK..\n");
				}
				else {
					write_user(user,"Tam sa neda ist!\n");
					return;
				}
				break;
			default:
				write_user(user,"Pouzi: .labyrint [start/stop/quit/show] [s/j/v/z/h/d]\n");
				return;
				break;
		}
	}
	if (user->lab==lab_room.end) {
		sprintf(text,"%s\n",lab_room.popiska[user->lab]);
		write_user(user,text);
		writecent(user,"~OL~FB** ~FYGAME OVER ~FB**\n");
		user->lab=0;
		sprintf(filename,"%s%c%s",DATAFILES,DIRSEP,LABYRINT_WINNERS);
		if (!(fp=ropen(filename,"r"))) {
			if(!(fp=ropen(filename,"w"))) {
				write_level(WIZ,1,"~OL~FRCHYBA: Nie je mozne vytvorit subor pre vitazov labyrintu!\n",NULL);
				return;
			}
			fprintf(fp,"%s\n",user->name);
			fclose(fp);
			return;
		}
		else {
			fclose(fp);
			if (!(fp=ropen(filename,"a"))) {
				write_level(WIZ,1,"~OL~FRCHYBA: Nie je mozne zapisovat do suboru pre vitazov labyrintu!\n",NULL);
				return;
			}
			fprintf(fp,"%s\n",user->name);
			fclose(fp);
			return;
		}
	}
	sprintf(text,"%s\n~FTMozes ist: ",lab_room.popiska[user->lab]);
	write_user(user,text);
	if (lab_room.sever[user->lab])
		write_user(user,"~OL~FRSEVER ");
	if (lab_room.juh[user->lab])
		write_user(user,"~OL~FRJUH ");
	if (lab_room.vychod[user->lab])
		write_user(user,"~OL~FRVYCHOD ");
	if (lab_room.zapad[user->lab])
		write_user(user,"~OL~FRZAPAD ");
	if (lab_room.hore[user->lab])
		write_user(user,"~OL~FRHORE ");
	if (lab_room.dole[user->lab])
		write_user(user,"~OL~FRDOLU ");
	write_user(user,"\n");
	if (st)
		return;
	return;
}

void piskvorky(UR_OBJECT user)
{
	UR_OBJECT u,u2;
	int x,y;
	int pocet;
	int players;

	if (word_count<2) {
		write_user(user,"Pouzi: .piskvorky <user> [o] <stop|show|back|top|pass|who> riadok stlpec\n\n");
		write_user(user,"       .pisk Jano        - zacnes partiu s Janom\n");
		write_user(user,"       .pisk Jano o      - zacnes partiu s Janom, ale ty budes znak 'O'\n");
		write_user(user,"       .pisk show [user] - ukaze stav Tvojej hry, alebo hry ktoru hra [user]\n");
		write_user(user,"       .pisk stop        - ukonci partiu (napr. ak sa ti odhlasi partner ;)\n");
		write_user(user,"       .pisk 7 9         - tahas na 7. riadok a 9. stlpec\n");
		write_user(user,"       .pisk top [n]     - ukaze tabulku najlepsich n hracov (standardne 10)\n");
		write_user(user,"       .pisk back        - vrati naposledy vykonany tah\n");
		write_user(user,"       .pisk pass        - prenechas prvy tah spoluhracovi.\n");
		write_user(user,"       .pisk who         - vypise aktualnych hracov.\n\n");
		return;
	}
	if (!(strcmp(word[1],"who"))) {
		players=0;
		write_user(user,"~OL~FB.-~FY*~FB-=-=-=~RS~FTAktualni piskvorkari~OL~FB=-=-=-~FY*~FB-.\n");
		write_user(user,"~OL~FB|   ~FWVyzyvatel~FB  |     ~FWSuper~FB    | ~FWTah~FB  |\n");
		write_user(user,"~OL~FB|=-=-=-=-=-=-=-|-=-=-=-=-=-=-=|=-=-=-|\n");
		for (u=user_first;u!=NULL;u=u->next) {
			if (u->login || u->room==NULL || u->type!=USER_TYPE)
				continue;
			if ((u->joined==2) && (u->game==2)) {
				players+=1;
				sprintf(text,"~OL~FB| ~FY%-12s~FB | ~FY%-12s~FB | ~FW%3d~FB  |\n",u->name,u->uname,u->moves);
				write_user(user,text);
			}

		}
		write_user(user,"~OL~FB`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'\n\n");
		return;
	}
	if (!(strcmp(word[1],"stop"))) {
		if (user->game==2) {
			if (!(u2=get_user(user->uname))) {
				write_user(user,"Spoluhrac ti zatial odisiel.\n");
				user->game=0;
				user->joined=0;
				user->moves=1;
				return;
			}
			sprintf(text,"Prestal%s si hrat piskvorky.\n",pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"%s s Tebou prestal%s hrat piskvorky!\n",user->name,pohl(user,"","a"));
			write_user(u2,text);
			if (user->moves>2) {
				add_point(u2,DB_PISKVORKY,1,0);
				add_point(user,DB_PISKVORKY,-1,0);
			}
			sprintf(text,"Pisk: %s:%d/%s:%d stop\n",u2->name,u2->moves,user->name,user->moves);
			log_game(text);
			user->game=0;
			user->joined=0;
			user->moves=1;
			u2->game=0;
			u2->joined=0;
			u2->moves=1;
			return;
		}
		write_user(user,"Nehras piskvorky.\n");
		return;
	}
	if (!(strcmp(word[1],"show"))) {
		if (word_count>2) {
			if (!(u=get_user(word[2]))) {
				write_user(user,notloggedon);
				return;
			}
			if (u->game!=2) {
				sprintf(text,"%s nehra piskvorky.\n",u->name);
				write_user(user,text);
				return;
			}
			stavhry(u,user);
			return;
		}
		if (user->game==2) {
			stavhry(user,user);
			return;
		}
		write_user(user,"Nemas co ukazat.\n");
		return;
	}
	if (!(strcmp(word[1],"top"))) {
		if (word_count==3)
			pocet=atoi(word[2]);
		else
			pocet=10;
		if ((pocet<1) || (pocet>99)) {
			write_user(user,"Pouzi: .piskvorky top [n]    - n je od 1 po 99\n");
			return;
		}
		winners(user,DB_PISKVORKY,pocet);
		return;
	}
	if (!(strcmp(word[1],"back"))) {
		if (user->game!=2) {
			write_user(user,"Nehras piskvorky.\n");
			return;
		}
		if (!user->joined) {
			write_user(user,"Nemas spoluhraca.\n");
			return;
		}
		if (user->tah) {
			write_user(user,"Si na tahu, nemozes vracat tah.\n");
			return;
		}
		if (!(u=get_user(user->uname))) {
			write_user(user,"Odisiel ti spoluhrac!\n");
			user->joined=0;
			user->game=0;
			user->moves=1;
			return;
		}
		if (user->joined==1) {
			x=user->last_x;
			y=user->last_y;
			if (x==-1) {
				write_user(user,"Este nebol vykonany tah.\n");
				return;
			}
			if (user->pis[x][y]=='.') {
				write_user(user,"Neda sa vratit tah.\n");
				return;
			}
			user->pis[x][y]='.';
			user->moves-=1;
			user->last_x=-1;
			user->last_y=-1;
			u->last_x=-1;
			u->last_y=-1;
			u->moves-=1;
			user->tah=1;
			u->tah=0;
			sprintf(text,"%s vracia svoj tah nazad!\n",user->name);
			write_user(user,"Vracias svoj tah nazad.\n");
			write_user(u,text);
			stavhry(user,u);
			stavhry(user,user);
		}
		else {
			x=u->last_x;
			y=u->last_y;
			if (x==-1) {
				write_user(user,"Este nebol vykonany tah.\n");
				return;
			}
			if (u->pis[x][y]=='.') {
				write_user(user,"Neda sa vratit tah.\n");
				return;
			}
			u->pis[x][y]='.';
			user->last_x=-1;
			user->last_y=-1;
			user->moves-=1;
			u->last_x=-1;
			u->last_y=-1;
			u->moves-=1;
			user->tah=1;
			u->tah=0;
			sprintf(text,"%s vracia svoj tah nazad!\n",user->name);
			write_user(user,"Vracias svoj tah nazad.\n");
			write_user(u,text);
			stavhry(u,user);
			stavhry(u,u);
		}
		return;
	}
	if ((u=get_user(word[1]))!=NULL) {
		if ((user->game!=0) || (u->game!=0)) {
			write_user(user,"Ty alebo tvoj partner uz hra inu hru!\n");
			return;
		}
		if (u==user) {
			sprintf(text,"Nemozes hrat %s so sebou.\n",pohl(user,"sam","sama"));
			write_user(user,text);
			return;
		}
		if (u->afk) {
			if (u->afk_mesg[0]) sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\n",u->name,u->afk_mesg);
			else sprintf(text,"%s je prave mimo klavesnice.\n",u->name);
			write_user(user,text);
			return;
		}
		if (u->ignall) {
			if (u->malloc_start!=NULL) sprintf(text,"%s prave nieco pise v editore.\n",u->name);
			else sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
			write_user(user,text);
			return;
		}
		if (u->igngames) {
			sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",u->name);
			write_user(user,text);
			return;
		}
		if (!(strcmp(u->room->name,"amfiteater")) && (play.on==1)) {
			write_user(user,"Pocas predstavenia nemozes hrat piskvorky.\n");
			return;
		}
		if (user->joined) {
			if (user->game!=2) {
				write_user(user,"Uz hras inu hru!\n");
				return;
			}
			write_user(user,"Uz mas partnera pre hru (pouzi: '.piskvorky stop' na ukoncenie partie)\n");
			return;
		}
		if (u->joined) {
			if (u->game!=2) {
				write_user(user,"Uzivatel uz hra inu hru!\n");
				return;
			}
			sprintf(text,"%s uz ma partnera pre hru (%s)\n",u->name,u->uname);
			write_user(user,text);
			return;
		}
		if (check_ignore_user(user,u)) {
			sprintf(text,"%s Ta ignoruje!\n",u->name);
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
		nastav_hru(user);
		user->last_x=-1;
		user->last_y=-2;
		u->last_x=-1;
		u->last_y=-2;
		sprintf(text,"~FTZacal%s si hrat piskvorky s ~OL%s~RS~FT.\n",pohl(user,"","a"),sklonuj(u,7));
		write_user(user,text);
		sprintf(text,"~FT%s ta vyzval%s hrat piskvorky.\n",user->name,pohl(user,"","a"));
		write_user(u,text);
		user->joined=1;
		user->moves=1;
		u->joined=2;
		u->game=2;
		u->moves=1;
		user->game=2;
		strcpy(user->uname,u->name);
		strcpy(u->uname,user->name);
		user->tah=1;
		if (word_count==3) {
			if ((!strcmp(word[2],"O")) || (!strcmp(word[2],"o"))) {
				user->znak='O';
				u->znak='X';
			}
			else {
				user->znak='X';
				u->znak='O';
			}
		}
		else {
			user->znak='X';
			u->znak='O';
		}
		stavhry(user,u);
		stavhry(user,user);
		return;
	}
	if (user->game!=2) {
		write_user(user,"Nehras piskvorky.\n");
		return;
	}
	if (!user->joined) {
		write_user(user,"Nemas spoluhraca.\n");
		return;
	}
	if (!user->tah) {
		write_user(user,"Pockaj az budes na tahu.\n");
		return;
	}
	if (!(strcmp(word[1],"Pass"))) {
	}
	else if (word_count<3) {
		write_user(user,"Malo parametrov.\n");
		return;
	}
	if (!(strcmp(user->room->name,"amfiteater")) && (play.on==1)) {
		write_user(user,"Pocas predstavenia nemozes hrat piskvorky.\n");
		return;
	}
	if (!(u=get_user(user->uname))) {
		write_user(user,"Odisiel ti spoluhrac!\n");
		user->joined=0;
		user->game=0;
		return;
	}
	if (!(strcmp(word[1],"Pass"))) {
		if ((user->tah) && (user->last_y==-2)) {
			user->tah=0;
			u->tah=1;
			sprintf(text,"%s ti prenechal%s moznost prveho tahu.\n",user->name,pohl(user,"","a"));
			write_user(u,text);
			sprintf(text,"Prenechal%s si prvy tah protivnikovi.\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		write_user(user,"Teraz nemozes prenechavat tah.\n");
		return;
	}
	for (x=0; x<(int)strlen(word[1]); ++x)
		if (isalpha(word[1][x])) {
			write_user(user,"Suradnice musia byt zadane ako cisla!\n");
			return;
		}
	for (x=0; x<(int)strlen(word[2]); ++x)
		if (isalpha(word[2][x])) {
			write_user(user,"Suradnice musia byt zadane ako cisla!\n");
			return;
		}
	x=atoi(word[1]);
	y=atoi(word[2]);
	if (x>15 || y>20 || x<0 || y<0) {
		write_user(user,"Cisla mozu byt v rozsahu 0-15,0-20.\n");
		return;
	}
	if (user->joined==1) {
		if (user->pis[x][y]!='.') {
			write_user(user,"Toto policko je uz obsadene.\n");
			return;
		}
		user->last_x=x;
		user->last_y=y;
		u->last_y=x;
		u->last_y=y;
		user->pis[x][y]=user->znak;
		user->moves+=1;
		u->moves+=1;
		user->tah=0;
		u->tah=1;
		stavhry(user,u);
		stavhry(user,user);
		je5(user,x,y,user->znak);
	}
	else {
		if (u->pis[x][y]!='.') {
			write_user(user,"Toto policko je uz obsadene.\n");
			return;
		}
		u->last_x=x;
		u->last_y=y;
		user->last_x=x;
		user->last_y=y;
		u->pis[x][y]=user->znak;
		u->moves+=1;
		user->moves+=1;
		user->tah=0;
		u->tah=1;
		stavhry(u,user);
		stavhry(u,u);
		je5(u,x,y,user->znak);
	}
}

void nastav_hru(UR_OBJECT user)
{
	int i,f;

	for (i=0;i<21;i++)
		for (f=0;f<21;f++)
			user->pis[i][f]='.';
}

extern void je5b(UR_OBJECT user,int n)
{
	UR_OBJECT u,u2;

	if (user->tah) {
		if (!(u=get_user(user->uname)))
			return;
	}
	else
		u=user;
	if (!(u2=get_user(u->uname)))
		return;
	if (n>=5) {
		if ((u->game==2) || (u2->game==2))
			sprintf(text,"~FGZ piskvorkoveho duelu ~OL~FY%s ~RS~FGvs ~OL~FY%s ",user->name,user->uname);
		write_room(u->room,text);
		sprintf(text,"~RS~FGvys%s ako vitaz ~OL~FY%s~RS~FG.\n",pohl(u,"iel","la"),u->name);
		write_room(u->room,text);
		sprintf(text,"~OL~FYVyhral%s si!!!\n",pohl(u,"","a"));
		write_user(u,text);
		sprintf(text,"~OL~FRPrehral%s si.\n",pohl(u2,"","a"));
		write_user(u2,text);
		sprintf(text,"Prevzal%s si hru a zacinas.\n",pohl(u,"","a"));
		write_user(u,text);
		sprintf(text,"%s prevzal%s hru a zacina.\n",u->name,pohl(u,"","a"));
		write_user(u2,text);
		u->joined=1;
		u->tah=1;
		u->last_x=-1;
		u->last_y=-2;
		u2->last_x=-1;
		u2->last_y=-2;
		u2->joined=2;
		u2->tah=0;
		nastav_hru(u);
		sprintf(text,"Pisk: %s:%d/%s:%d\n",u->name,u->moves,u2->name,u2->moves);
		log_game(text);
		if ((u->game==2) || (u2->game==2)) {
			if (u->moves<PISKVORKY_MIN_MOVES) {
				sprintf(text,"Do tabulky vitazov sa nezapisujete, pretoze mate menej ako %d tahov.\n",PISKVORKY_MIN_MOVES);
				write_user(u,text);
				write_user(u2,text);
				u->moves=1;
				u2->moves=1;
				return;
			}
			else {
				if (u->total_login<PISKVORKY_MIN_TLT*60*60) {
					sprintf(text,"Do tabulky sa nezapisujete, lebo %s ma prilis malo total login (treba aspon %d hodin).\n",u->name,PISKVORKY_MIN_TLT);
					write_user(u,text);
					write_user(u2,text);
					u->moves=1;
					u2->moves=1;
					return;
				}
				if (u2->total_login<PISKVORKY_MIN_TLT*60*60) {
					sprintf(text,"Do tabulky sa nezapisujete, lebo %s ma prilis malo total login (treba aspon %d hodin).\n",u2->name,PISKVORKY_MIN_TLT);
					write_user(u,text);
					write_user(u2,text);
					u->moves=1;
					u2->moves=1;
					return;
				}
				add_point(u,DB_PISKVORKY,1,0);
				add_point(u2,DB_PISKVORKY,-1,0);
			}
		}
		u->moves=1;
		u2->moves=1;
	}
	return;
}

extern void je4b(UR_OBJECT user,int n)
{
	UR_OBJECT u,u2;

	if (user->tah) {
		if (!(u=get_user(user->uname)))
			return;
	}
	else
		u=user;
	if (!(u2=get_user(u->uname)))
		return;
	if (n>=4) {
		if ((u->game==3) || (u2->game==3))
			sprintf(text,"~FGZ GEO duelu ~OL~FY%s ~RS~FGvs ~OL~FY%s ",user->name,user->uname);
		write_room(u->room,text);
		sprintf(text,"~RS~FGvys%s ako vitaz ~OL~FY%s~RS~FG.\n",pohl(u,"iel","la"),u->name);
		write_room(u->room,text);
		sprintf(text,"~OL~FYVyhral%s si!!!\n",pohl(u,"","a"));
		write_user(u,text);
		sprintf(text,"~OL~FRPrehral%s si.\n",pohl(u2,"","a"));
		write_user(u2,text);
		sprintf(text,"Prevzal%s si hru a zacinas.\n",pohl(u,"","a"));
		write_user(u,text);
		sprintf(text,"%s prevzal%s hru a zacina.\n",u->name,pohl(u,"","a"));
		write_user(u2,text);
		sprintf(text,"Geo : %s/%s\n",u->name,u2->name);
		log_game(text);
		u->joined=1;
		u->tah=1;
		u->last_x=-1;
		u->last_y=-2;
		u2->last_x=-1;
		u2->last_y=-2;
		u2->joined=2;
		u2->tah=0;
		nastav_hru(u);
		u->moves=1;
		u2->moves=1;
	}
	return;
}

extern void je5(UR_OBJECT user,int x,int y,char znak)
{
	int n,a,b;

	a=x;
	b=y;
	n=1;
	while (a>0) {
		a--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	while (a<15) {
		a++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je5b(user,n);
	a=x;
	n=1;
	while (b>0) {
		b--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	b=y;
	while (b<20) {
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je5b(user,n);
	b=y;
	n=1;
	while (a>0 && b>0) {
		a--;
		b--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	b=y;
	while (a<15 && b<20) {
		a++;
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je5b(user,n);
	a=x;
	b=y;
	n=1;
	while (a>0 && b<20) {
		a--;
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	b=y;
	while (a<15 && b>0) {
		a++;
		b--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je5b(user,n);
}

extern void je4(UR_OBJECT user,int x,int y,char znak)
{
	int n,a,b;

	a=x;
	b=y;
	n=1;
	while (a>0) {
		a--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	while (a<5) {
		a++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je4b(user,n);
	a=x;
	n=1;
	while (b>0) {
		b--;
		if (user->pis[a][b]==znak)n++;
		else
			break;
	}
	b=y;
	while (b<6) {
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je4b(user,n);
	b=y;
	n=1;
	while (a>0 && b>0) {
		a--;
		b--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	b=y;
	while (a<5 && b<6) {
		a++;
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je4b(user,n);
	a=x;
	b=y;
	n=1;
	while (a>0 && b<6) {
		a--;
		b++;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	a=x;
	b=y;
	while (a<5 && b>0) {
		a++;
		b--;
		if (user->pis[a][b]==znak)
			n++;
		else
			break;
	}
	je4b(user,n);
}

extern void stavhry(UR_OBJECT user1,UR_OBJECT user2)
{
	int i,f,lastx,lasty;
	UR_OBJECT u;
	char c;

	if (user1->game) {
		if (!(u=get_user(user1->uname)))
			return;
		if (user1->game==2)
			sprintf(text,"\n~FG                   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0\n");
		if (user1->game==3)
			sprintf(text,"\n~OL~FR                   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0\n");
		write_user(user2,text);
		for (i=0;i<16;i++) {
			if (user1->game==2)
				sprintf(text,"                ~FG%2d~OL ",i);
			if (user1->game==3)
				sprintf(text,"                ~OL~FR%2d~RS ",i);
			write_user(user2,text);
			for (f=0;f<21;f++) {
				if (user1->joined==1) {
					c=user1->pis[i][f];
					lastx=user1->last_x;
					lasty=user1->last_y;
				}
				else {
					c=u->pis[i][f];
					lastx=u->last_x;
					lasty=u->last_y;
				}
				if (c=='O') {
					if ((i==lastx) && (f==lasty))
						sprintf(text,"~BB~OL~FW%c~BK ",c);
					else
						sprintf(text,"~OL~FW%c ",c);
				}
				if (c=='X') {
					if ((i==lastx) && (f==lasty))
						sprintf(text,"~BB~OL~FG%c~BK ",c);
					else
						sprintf(text,"~OL~FG%c ",c);
				}
				if (c=='.') {
					if (user1->game==2)
						sprintf(text,"~FB%c ",c);
					if (user1->game==3)
						sprintf(text,"~RS~FY%c~OL ",c);
				}
				write_user(user2,text);
			}
			if (user1->game==2)
				sprintf(text,"~RS~FG%-2d\n",i);
			if (user1->game==3)
				sprintf(text,"~OL~FR%-2d\n",i);
			write_user(user2,text);
		}
		if (user1->game==2)
			sprintf(text,"~FG                   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0\n");
		if (user1->game==3)
			sprintf(text,"~OL~FR                   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0\n");
		write_user(user2,text);
		if (!user1->joined)
			write_user(user2,"Zatial nemas spoluhraca.\n");
		else {
			write_user(user2,"~RS");
			if (user1->tah) {
				if (lastx==-1)
					sprintf(text,"                       ~FTTah c. ~OL~FW%d~RS~FT, na tahu je %s (~FW%c~FT).~RS~BK\n",user1->moves,user1->name,user1->znak);
				else
					sprintf(text,"             ~FTTah c. ~OL~FW%d~RS~FT, posledny tah: [~FW%d,%d~FT]. Na tahu je %s (~FW%c~FT).~RS~BK\n",user1->moves,lastx,lasty,user1->name,user1->znak);
				write_user(user2,text);
			}
			else {
				if (!(u=get_user(user1->uname)))
					return;
				if (lastx==-1)
					sprintf(text,"                       ~FTTah c. ~OL~FW%d~RS~FT, na tahu je %s (~FW%c~FT).~RS~BK\n",u->moves,u->name,u->znak);
				else
					sprintf(text,"             ~FTTah c. ~OL~FW%d~RS~FT, posledny tah: [~FW%d,%d~FT]. Na tahu je %s (~FW%c~FT).~RS~BK\n",u->moves,lastx,lasty,u->name,u->znak);
				write_user(user2,text);
			}
		}

	}
	else {
		sprintf(text,"~OLMemontalne nehras ziadnu hru.\n~RS");
		write_user(user2,text);
	}
}

extern void stavhryg(UR_OBJECT user1,UR_OBJECT user2)
{
	int i,f,lastx,lasty;
	UR_OBJECT u;
	char c;

	if (user1->game) {
		if (!(u=get_user(user1->uname)))
			return;
		if (user1->game==3)
			sprintf(text,"\n~OL~FR                                 1 2 3 4 5 6 7\n");
		write_user(user2,text);
		for (i=0;i<6;i++) {
			if (user1->game==3)
				sprintf(text,"                               ~OL~FR|~RS ");
			write_user(user2,text);
			for (f=0;f<7;f++) {
				if (user1->joined==1) {
					c=user1->pis[i][f];
					lastx=user1->last_x;
					lasty=user1->last_y;
				}
				else {
					c=u->pis[i][f];
					lastx=u->last_x;
					lasty=u->last_y;
				}
				if (c=='O') {
					if ((i==lastx) && (f==lasty))
						sprintf(text,"~BB~OL~FW%c~BK ",c);
					else
						sprintf(text,"~OL~FW%c ",c);
				}
				if (c=='X') {
					if ((i==lastx) && (f==lasty))
						sprintf(text,"~BB~OL~FG%c~BK ",c);
					else
						sprintf(text,"~OL~FG%c ",c);
				}
				if (c=='.') {
					if (user1->game==3)
						sprintf(text,"~RS~FY%c~OL ",c);
				}
				write_user(user2,text);
			}
			if (user1->game==3)
				sprintf(text,"~OL~FR|\n");
			write_user(user2,text);
		}
		if (user1->game==3)
			sprintf(text,"~OL~FR                                 1 2 3 4 5 6 7\n");
		write_user(user2,text);
		if (!user1->joined)
			write_user(user2,"Zatial nemas spoluhraca.\n");
		else {
			write_user(user2,"~RS");
			if (user1->tah) {
				if (lastx==-1)
					sprintf(text,"~FTTah c. ~OL~FW%d~RS~FT, na tahu je %s (~FW%c~FT).~RS~BK\n",user1->moves,user1->name,user1->znak);
				else
					sprintf(text,"~FTTah c. ~OL~FW%d~RS~FT, posledny tah: [~FW%d,%d~FT]. Na tahu je %s (~FW%c~FT).~RS~BK\n",user1->moves,lastx,lasty,user1->name,user1->znak);
				writecent(user2,text);
			}
			else {
				if (!(u=get_user(user1->uname)))
					return;
				if (lastx==-1)
					sprintf(text,"~FTTah c. ~OL~FW%d~RS~FT, na tahu je %s (~FW%c~FT).~RS~BK\n",u->moves,u->name,u->znak);
				else
					sprintf(text,"~FTTah c. ~OL~FW%d~RS~FT, posledny tah: [~FW%d,%d~FT]. Na tahu je %s (~FW%c~FT).~RS~BK\n",u->moves,lastx,lasty,u->name,u->znak);
				writecent(user2,text);
			}
		}

	}
	else {
		sprintf(text,"~OLMemontalne nehras ziadnu hru.\n~RS");
		write_user(user2,text);
	}
}

extern void play_geo(UR_OBJECT user)
{
	UR_OBJECT u,u2;
	int x,y;
	int players;

	if (word_count<2) {
		write_user(user,"Pouzi: .geo <user> [o] <stop|show|back|top|pass|who> riadok stlpec\n\n");
		write_user(user,"       .geo Jano        - zacnes partiu s Janom\n");
		write_user(user,"       .geo Jano o      - zacnes partiu s Janom, ale ty budes znak 'O'\n");
		write_user(user,"       .geo show [user] - ukaze stav Tvojej hry, alebo hry ktoru hra [user]\n");
		write_user(user,"       .geo stop        - ukonci partiu (napr. ak sa ti odhlasi partner ;)\n");
		write_user(user,"       .geo 6           - hadzes svoju piskvorku do 6. stlpca\n");
		write_user(user,"       .geo back        - vrati naposledy vykonany tah\n");
		write_user(user,"       .geo pass        - prenechas prvy tah spoluhracovi.\n");
		write_user(user,"       .geo who         - vypise aktualnych hracov.\n\n");
		return;
	}
	if (!(strcmp(word[1],"who"))) {
		players=0;
		write_user(user,"~OL~FB.-~FY*~FB-=-=-=~RS~FTAktualni  GEO hraci ~OL~FB=-=-=-~FY*~FB-.\n");
		write_user(user,"~OL~FB|   ~FWVyzyvatel~FB  |     ~FWSuper~FB    | ~FWTah~FB  |\n");
		write_user(user,"~OL~FB|=-=-=-=-=-=-=-|-=-=-=-=-=-=-=|=-=-=-|\n");
		for (u=user_first;u!=NULL;u=u->next) {
			if (u->login || u->room==NULL || u->type!=USER_TYPE)
				continue;
			if ((u->joined==2) && (u->game==3)) {
				players+=1;
				sprintf(text,"~OL~FB| ~FY%-12s~FB | ~FY%-12s~FB | ~FW%3d~FB  |\n",u->name,u->uname,u->moves);
				write_user(user,text);
			}
		}
		write_user(user,"~OL~FB`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'\n\n");
		return;
	}
	if (!(strcmp(word[1],"stop"))) {
		if (user->game==3) {
			if (!(u2=get_user(user->uname))) {
				write_user(user,"Spoluhrac ti zatial odisiel.\n");
				user->game=0;
				user->joined=0;
				user->moves=1;
				return;
			}
			write_user(user,"Prestal si hrat GEO.\n");
			sprintf(text,"%s s Tebou prestal%s hrat GEO!\n",user->name,pohl(user,"","a"));
			write_user(u2,text);
			sprintf(text,"Geo:  %s:%d/%s:%d stop\n",u2->name,u2->moves,user->name,user->moves);
			log_game(text);
			user->game=0;
			user->joined=0;
			user->moves=1;
			u2->game=0;
			u2->joined=0;
			u2->moves=1;
			return;
		}
		write_user(user,"Nehras GEO.\n");
		return;
	}
	if (!(strcmp(word[1],"show"))) {
		if (word_count>2) {
			if (!(u=get_user(word[2]))) {
				write_user(user,notloggedon);
				return;
			}
			if (u->game!=3) {
				sprintf(text,"%s nehra GEO.\n",u->name);
				write_user(user,text);
				return;
			}
			stavhryg(u,user);
			return;
		}
		if (user->game==3) {
			stavhryg(user,user);
			return;
		}
		write_user(user,"Nemas co ukazat.\n");
		return;
	}
	if (!(strcmp(word[1],"back"))) {
		if (user->game!=3) {
			write_user(user,"Nehras GEO.\n");
			return;
		}
		if (!user->joined) {
			write_user(user,"Nemas spoluhraca.\n");
			return;
		}
		if (user->tah) {
			write_user(user,"Si na tahu, nemozes vracat tah.\n");
			return;
		}
		if (!(u=get_user(user->uname))) {
			write_user(user,"Odisiel ti spoluhrac!\n");
			user->joined=0;
			user->game=0;
			user->moves=1;
			return;
		}
		if (user->joined==1) {
			x=user->last_x;
			y=user->last_y;
			if (x==-1) {
				write_user(user,"Este nebol vykonany tah.\n");
				return;
			}
			if (user->pis[x][y]=='.') {
				write_user(user,"Neda sa vratit tah.\n");
				return;
			}
			user->pis[x][y]='.';
			user->moves-=1;
			user->last_x=-1;
			user->last_y=-1;
			u->last_x=-1;
			u->last_y=-1;
			u->moves-=1;
			user->tah=1;
			u->tah=0;
			sprintf(text,"%s vracia svoj tah nazad!\n",user->name);
			write_user(user,"Vracias svoj tah nazad.\n");
			write_user(u,text);
			stavhryg(user,u);
			stavhryg(user,user);
		}
		else {
			x=u->last_x;
			y=u->last_y;
			if (x==-1) {
				write_user(user,"Este nebol vykonany tah.\n");
				return;
			}
			if (u->pis[x][y]=='.') {
				write_user(user,"Neda sa vratit tah.\n");
				return;
			}
			u->pis[x][y]='.';
			user->last_x=-1;
			user->last_y=-1;
			user->moves-=1;
			u->last_x=-1;
			u->last_y=-1;
			u->moves-=1;
			user->tah=1;
			u->tah=0;
			sprintf(text,"%s vracia svoj tah nazad!\n",user->name);
			write_user(user,"Vracias svoj tah nazad.\n");
			write_user(u,text);
			stavhryg(u,user);
			stavhryg(u,u);
		}
		return;
	}
	if ((u=get_user(word[1]))!=NULL) {
		if ((user->game!=0) || (u->game!=0)) {
			write_user(user,"Ty alebo tvoj partner uz hra inu hru!\n");
			return;
		}
		if (u==user) {
			sprintf(text,"Nemozes hrat sam%s so sebou.\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		if (u->afk) {
			if (u->afk_mesg[0])
				sprintf(text,"%s je prave mimo klavesnice (~OL%s~RS)\n",u->name,u->afk_mesg);
			else
				sprintf(text,"%s je prave mimo klavesnice.\n",u->name);
			write_user(user,text);
			return;
		}
		if (u->ignall) {
			if (u->malloc_start!=NULL)
				sprintf(text,"%s prave nieco pise v editore.\n",u->name);
			else
				sprintf(text,"%s prave ignoruje vsetky hlasky.\n",u->name);
			write_user(user,text);
			return;
		}
		if (u->igngames) {
			sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",u->name);
			write_user(user,text);
			return;
		}
		if (strcmp(user->room->name,u->room->name)) {
			sprintf(text,"%s je v inej miestnosti.\n",u->name);
			write_user(user,text);
			return;
		}
		if (!(strcmp(u->room->name,"amfiteater")) && (play.on==1)) {
			write_user(user,"Pocas predstavenia nemozes hrat piskvorky.\n");
			return;
		}
		if (user->joined) {
			if (user->game!=3) {
				write_user(user,"Teraz hras inu hru!\n");
				return;
			}
			write_user(user,"Uz mas partnera pre hru (pouzi: '.piskvorky stop' na ukoncenie partie)\n");
			return;
		}
		if (u->joined) {
			if (u->game!=3) {
				write_user(user,"Uzivatel teraz hra inu hru!\n");
				return;
			}
			sprintf(text,"%s uz ma partnera pre hru (%s)\n",u->name,u->uname);
			write_user(user,text);
			return;
		}
		if (check_ignore_user(user,u)) {
			sprintf(text,"%s Ta ignoruje!\n",u->name);
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
		nastav_hru(user);
		user->last_x=-1;
		user->last_y=-2;
		u->last_x=-1;
		u->last_y=-2;
		sprintf(text,"~FTZacal%s si hrat GEO s ~OL%s~RS~FT.\n",pohl(user,"","a"),sklonuj(u,7));
		write_user(user,text);
		sprintf(text,"~FT%s ta vyzval%s hrat GEO.\n",user->name,pohl(user,"","a"));
		write_user(u,text);
		user->joined=1;
		user->moves=1;
		u->joined=2;
		u->game=3;
		u->moves=1;
		user->game=3;
		strcpy(user->uname,u->name);
		strcpy(u->uname,user->name);
		user->tah=1;
		if (word_count==3) {
			if ((!strcmp(word[2],"O")) || (!strcmp(word[2],"o"))) {
				user->znak='O';
				u->znak='X';
			}
			else {
				user->znak='X';
				u->znak='O';
			}
		}
		else {
			user->znak='X';
			u->znak='O';
		}
		stavhryg(user,u);
		stavhryg(user,user);
		return;
	}
	if (user->game!=3) {
		write_user(user,"Nehras GEO.\n");
		return;
	}
	if (!user->joined) {
		write_user(user,"Nemas spoluhraca.\n");
		return;
	}
	if (!user->tah) {
		write_user(user,"Pockaj az budes na tahu.\n");
		return;
	}
	if (!(strcmp(word[1],"Pass"))) {
	}
	else {
		if (word_count<2) {
			write_user(user,"Malo parametrov.\n");
			return;
		}
	}
	if (!(strcmp(user->room->name,"amfiteater")) && (play.on==1)) {
		write_user(user,"Pocas predstavenia nemozes hrat piskvorky.\n");
		return;
	}
	if (!(u=get_user(user->uname))) {
		write_user(user,"Odisiel ti spoluhrac!\n");
		user->joined=0;
		user->game=0;
		return;
	}
	if (!(strcmp(word[1],"Pass"))) {
		if ((user->tah) && (user->last_y==-2)) {
			user->tah=0;
			u->tah=1;
			sprintf(text,"%s ti prenechal%s moznost prveho tahu.\n",user->name,pohl(user,"","a"));
			write_user(u,text);
			sprintf(text,"Prenechal%s si prvy tah protivnikovi.\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		write_user(user,"Teraz nemozes prenechavat tah.\n");
		return;
	}
	for (x=0; x<(int)strlen(word[1]); ++x)
		if (isalpha(word[1][x])) {
			write_user(user,"Stlpec musi byt zadany ako cislo!\n");
			return;
		}
	y=atoi(word[1]);
	if (y>7 || y<1) {
		write_user(user,"Cislo moze byt v rozsahu 1-7.\n");
		return;
	}
	y--;
	if (user->joined==1) {
		if (user->pis[0][y]!='.') {
			write_user(user,"Tento stlpec je plny!\n");
			return;
		}
		x=0;
		while ((x<5) && (user->pis[x][y]=='.'))
			++x;
		if (user->pis[x][y]!='.')
			--x;
		user->last_x=x;
		user->last_y=y;
		u->last_y=x;
		u->last_y=y;
		user->pis[x][y]=user->znak;
		user->moves+=1;
		u->moves+=1;
		user->tah=0;
		u->tah=1;
		stavhryg(user,u);
		stavhryg(user,user);
		je4(user,x,y,user->znak);
	}
	else {
		if (u->pis[0][y]!='.') {
			write_user(user,"Tento stlpec je plny!\n");
			return;
		}
		x=0;
		while ((x<5) && (u->pis[x][y]=='.'))++x;
		if (u->pis[x][y]!='.')
			--x;
		u->last_x=x;
		u->last_y=y;
		user->last_x=x;
		user->last_y=y;
		u->pis[x][y]=user->znak;
		u->moves+=1;
		user->moves+=1;
		user->tah=0;
		u->tah=1;
		stavhryg(u,user);
		stavhryg(u,u);
		je4(u,x,y,user->znak);
	}
}

extern void play_hangman(UR_OBJECT user,char *inpstr)
{
	int count,blanks;
	unsigned int i;
	char *get_hang_word();

	if (word_count<2) {
		write_user(user,"Pouzi: .hangman [start/stop/status/top] [<pismeno>]\n\n");
		write_user(user,"       .hang start        - zacne hru\n");
		write_user(user,"       .hang stop         - ukonci rozohratu hru\n");
		write_user(user,"       .hang status       - ukaze stav hry\n");
		write_user(user,"       .hang p            - hadas pismeno 'p'\n");
		write_user(user,"       .hang top <n>      - ukaze tabulku <n> najlepsich (standardne 10)\n\n");
		return;
	}
	srand(time(0));
	strtolower(word[1]);
	i=0;
	if (!strcmp("status",word[1])) {
		if (user->hang_stage==-1) {
			write_user(user,"Este nehras; pouzi .hangman start\n");
			return;
		}
		write_user(user,"Toto je stav Tvojej hry:\n");
		hangman_status(user);
		write_user(user,"\n");
		return;
	}
	if (!strcmp("stop",word[1])) {
		if (user->hang_stage==-1) {
			write_user(user,"Ved nehras! Ak chces zacat hrat, pouzi .hangman start\n");
			return;
		}
		user->hang_stage=-1;
		user->hang_word[0]='\0';
		user->hang_word_show[0]='\0';
		user->hang_guess[0]='\0';
		sprintf(text,"Vzdal%s si sa, hanba!!!\n",pohl(user,"","a"));
		add_point(user,DB_HANGMAN,-1,0);
		write_user(user,text);
		return;
	}
	if (!strcmp("top",word[1])) {
		if (word_count<3) {
			winners(user,DB_HANGMAN,10);
			return;
		}
		i=atoi(word[2]);
		if ((i<1) || (i>99)) {
			write_user(user,"Cislo musi byt v rozsahu 1-99!\n");
			return;
		}
		winners(user,DB_HANGMAN,i);
		return;
	}
	if (!strcmp("start",word[1])) {
		if (user->hang_stage>-1) {
			write_user(user,"Uz predsa hras! (pouzi '~OL.hangman status~RS' pre zobrazenie stavu hry)\n");
			return;
		}
		get_hang_word(user->hang_word);
		strcpy(user->hang_word_show,user->hang_word);
		for (i=0;i<strlen(user->hang_word_show);++i) {
			if ((user->hang_word[i]==' ') || (user->hang_word[i]=='\n'))
				user->hang_word_show[i]=' ';
			else
				user->hang_word_show[i]='-';
		}
		user->hang_stage=0;
		write_user(user,"Zacinas hrat ...\n");
		hangman_status(user);
		return;
	}
	count=blanks=i=0;
	if (user->hang_stage==-1) {
		write_user(user,"Este nehras, pouzi .hangman start\n");
		return;
	}
	if (strlen(inpstr)>3) {
		if (strlen(inpstr)!=strlen(user->hang_word)-1) {
			write_user(user,"Pozor, dlzka zadaneho slova nie je rovnaka ako dlzka hadaneho slova.\n");
			return;
		}
		if (!strncasecmp(inpstr,user->hang_word,strlen(inpstr))) {
			sprintf(text,"~FY~OLVYBORNE!~RS ~FYUhad%s si slovo bez odvisnutia na sibenici!\n",pohl(user,"ol","la"));
			add_point(user,DB_HANGMAN,1,0);
			write_user(user,text);
			user->hang_stage=-1;
			user->hang_word[0]='\0';
			user->hang_word_show[0]='\0';
			user->hang_guess[0]='\0';
		}
		else {
			sprintf(text,"~OL~FRAAAaaaaargh!! ~FYNeuhad%s si slovo a odvis%s na sibenici!\n",pohl(user,"ol","la"),pohl(user,"ol","la"));
			write_user(user,text);
			sprintf(text,"Hadane slovo bolo: ~OL%s\n",user->hang_word);
			write_user(user,text);
			add_point(user,DB_HANGMAN,-1,0);
			user->hang_stage=-1;
			user->hang_word[0]='\0';
			user->hang_word_show[0]='\0';
			user->hang_guess[0]='\0';
		}
		return;
	}
	if (strlen(inpstr)>1 && strlen(inpstr)<4) {
		write_user(user,"Mozes hadat len jedno pismeno, alebo naraz cele slovo!\n");
		return;
	}
	strtolower(word[1]);
	if (strstr(user->hang_guess,word[1])) {
		user->hang_stage++;
		sprintf(text,"~OLToto pismeno si uz hadal%s! \n",pohl(user,"","a"));
		write_user(user,text);
		hangman_status(user);
		if (user->hang_stage>=7) {
			sprintf(text,"~OL~FRAAAaaaaargh!! ~FYNeuhad%s si slovo a odvis%s si na sibenici!\n",pohl(user,"ol","la"),pohl(user,"ol","la"));
			write_user(user,text);
			add_point(user,DB_HANGMAN,-1,0);
			user->hang_stage=-1;
			user->hang_word[0]='\0';
			user->hang_word_show[0]='\0';
			user->hang_guess[0]='\0';
		}
		write_user(user,"\n");
		return;
	}
	for (i=0;i<strlen(user->hang_word);++i) {
		if (user->hang_word[i]==word[1][0]) {
			user->hang_word_show[i]=user->hang_word[i];
			++count;
		}
		if (user->hang_word_show[i]=='-')
			++blanks;
	}
	strcat(user->hang_guess,word[1]);
	if (!count) {
		user->hang_stage++;
		sprintf(text,"~OLPismeno sa nenachadza v hadanom slove, ~FY%d. ~FWkrok k odvisnutiu!\n",user->hang_stage);
		write_user(user,text);
		hangman_status(user);
		if (user->hang_stage>=7) {
			sprintf(text,"~OL~FRAAAaaaaargh!! ~FYNeuhad%s si slovo a odvis%s si na sibenici!\n",pohl(user,"ol","la"),pohl(user,"ol","la"));
			write_user(user,text);
			add_point(user,DB_HANGMAN,-1,0);
			user->hang_stage=-1;
			user->hang_word[0]='\0';
			user->hang_word_show[0]='\0';
			user->hang_guess[0]='\0';
		}
		return;
	}
	if (count==1) sprintf(text,"~OL~FTDobre, pismeno '~FY%s~FT' sa nachadza v slove jeden krat.\n",word[1]);
	else sprintf(text,"~OL~FTDobre, pismeno '~FY%s~FT' sa nachadza v slove %d krat.\n",word[1],count);
	write_user(user,text);
	hangman_status(user);
	if (!blanks) {
		sprintf(text,"~FY~OLVYBORNE!~RS ~FYUhad%s si slovo bez odvisnutia na sibenici!\n",pohl(user,"ol","la"));
		add_point(user,DB_HANGMAN,1,0);
		write_user(user,text);
		user->hang_stage=-1;
		user->hang_word[0]='\0';
		user->hang_word_show[0]='\0';
		user->hang_guess[0]='\0';
	}
}

void hangman_status(UR_OBJECT user)
{
	switch (user->hang_stage) {
		case 0:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|                                                                  ~OL~FB|\n~FY|~RS           ~OL %-50s    ~OL~FB|\n~FY|~RS           ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 1:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS           ~OL %-50s    ~OL~FB|\n~FY|~RS           ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 2:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS           ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 3:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS   |       ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 4:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS  /|       ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 5:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS  /|\\      ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS                                                                  ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 6:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS  /|\\      ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS  /                                                               ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
		case 7:
			sprintf(text,"~FY~OL+~RS~FY---~OL+ ~OL~FB=-=-=-=-=< ~FYSibenica ~FB>=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+\n~FY|   |                                                              ~OL~FB|\n~FY|~RS   O       ~OL %-50s    ~OL~FB|\n~FY|~RS  /|\\      ~FGHadane pismena:~RS~FY %-30s         ~OL~FB|\n~FY|~RS  / \\                                                             ~OL~FB|\n~FY|______ ~OL~FB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+\n",user->hang_word_show,strlen(user->hang_guess)<1?"zatial ziadne":user->hang_guess);
			break;
	}
	write_user(user,text);
	return;
}

extern char *get_hang_word(char *aword)
{
	char filename[80];
	FILE *fp;
	static char fallback[15]="hangman";
	int lines=0,cnt=0,i=0;

	sprintf(filename,"%s%c%s",DATAFILES,DIRSEP,HANGMAN_WORDS);
	lines=count_lines(filename);
	if (!lines)
		return fallback;
	srand(time(0));
	cnt=rand()%lines;
	if (!(fp=ropen(filename,"r")))
		return fallback;
	fgets(aword,50,fp);
	while (fgets(aword,sizeof(aword)-1,fp)!=NULL) {
		if (i==cnt) {
			fclose(fp);
			if (strlen(aword)>0)
				return aword;
			else
				return fallback;
		}
		++i;
	}
	fclose(fp);
	return fallback;
}

extern UR_OBJECT doom_get_user(int x,int y)
{
	UR_OBJECT u;

	if (x<0 || y<0 || x>=D_MAPX || y>=D_MAPY)
		return NULL;
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->type!=USER_TYPE || u->room==NULL)
			continue;
		if (u->doom && u->doom_x==x && u->doom_y==y && u->doom_energy)
			return u;
	}
	return NULL;
}

extern void doom(UR_OBJECT user,char *inpstr)
{
	UR_OBJECT u;
	int x,y,flag;

	if (!(strcmp(word[1],"top"))) {
		if (word_count==3)
			x=atoi(word[2]);
		else
			x=10;
		if ((x<1) || (x>99)) {
			write_user(user,"Pouzi: .doom top [n]    - n je od 1 po 99\n");
			return;
		}
		winners(user,DB_DOOM,x);
		return;
	}
	if ((!strcmp(word[1],"update")) && (user->level>=KIN)) {
		if (doom_init()) {
			write_user(user,"DOOM: inicializacia ~FRzlyhala~FW\n");
			doom_status=0;
			return;
		}
		write_user(user,"DOOM: inicializacia ~FGOK~FW\n");
		doom_status=1;
		return;
	}
	if (!doom_status) {
		write_user(user,"Doom je momentalne mimo prevadzku. Kontaktujte spravcu.\n");
		return;
	}
	if (!(strcmp(word[1],"shout"))) {
		if (!user->doom || !user->doom_energy) {
			sprintf(text,"Ked nie si zapojen%s do hry, nemas sa co ozyvat!\n",pohl(user,"y","a"));
			write_user(user,text);
			return;
		}
		if (word_count<3) {
			write_user(user,"Pouzi: .doom shout <text>\n");
			return;
		}
		inpstr=remove_first(inpstr);
		switch (word[2][0]) {
			case '1':
				strcpy(text,"Dostanem ta, ty sukin syn!");
				break;
			case '2':
				strcpy(text,"Jeden z nas skape, a budes to TY!");
				break;
			case '3':
				strcpy(text,"Chachaaa, priprav sa na smrt!");
				break;
			case '4':
				strcpy(text,"Brrrr, to je ale svinstvo!");
				break;
			case '5':
				strcpy(text,"Uaaaaaaaaaaauuuuuaaaaaaaauuaaa!!!!!!!!!");
				break;
			case '6':
				strcpy(text,"Priprav si vstupenku do pekla, mojko!");
				break;
			case '7':
				strcpy(text,"Chlape, ty si ale SKAREDY!");
				break;
			case '8':
				strcpy(text,"Vrrrrr, prisiel cas odplaty!!!");
				break;
			case '9':
				strcpy(text,"Dnes ti pustim zilou motorovou pilou!!!");
				break;
			case '0':
				strcpy(text,"Prisiel cas oholit ti chlpy raketometom!");
				break;
			default:
				sstrncpy(text,inpstr,40);
				text[40]='\0';
		}
		doom_wall(NULL,text);
		return;
	}
	if (word_count!=2) {
		write_user(user,"Pouzitie: .doom <prikaz>\n");
		write_user(user,"Prikazy:  .doom start        - spustis DOOM\n");
		write_user(user,"          .doom stop         - ukoncis hru\n");
		write_user(user,"          .doom top [n]      - tabulka najlepsich\n");
		write_user(user,"          .doom who          - zoznam hrajucich\n");
		write_user(user,"          .doom join         - pridas sa do hry\n");
		write_user(user,"          .doom shout <text> - sprava pre doomujucich\n");
		write_user(user,"          .doom f,b          - dopredu (Forward) alebo dozadu (Back)\n");
		write_user(user,"          .doom l,r          - otoc sa vlavo (Left) alebo vpravo (Right)\n");
		write_user(user,"          .doom sl,sr        - bokom vlavo alebo vpravo (Strafe Left/Right)\n");
		write_user(user,"          .doom 1 - 3        - vyber zbrane (1,2,3)\n");
		write_user(user,"          .doom x            - strelba (eXecute)\n");
		write_user(user,"          .doom load         - nabijanie zbrane (Loading)\n");
		write_user(user,"          .doom say          - umoznuje pouzivat .say (poistka)\n");
		write_user(user,"Viac informacii o hre cez .pomoc doom\n");
		return;
	}
	if (!(strcmp(word[1],"who"))) {
		doom_who(user);
		return;
	}
	if (!strcmp(word[1],"start")) {
		if (user->doom) {
			write_user(user,"Neblbni, ved DOOM uz hras!\n");
			return;
		}
		sprintf(text,"\033[%d;%dr",D_LINES+1,user->lines);
		user->lines-=D_LINES;
		write_user(user,text);
		user->doom=1;
		user->doom_energy=0;
		user->doom_loading=0;
		user->doom_score=doom_points_check(user);
		write_doom(user,0);
		doom_text(user,"Vitam ta v hre DOOM: Fate of Atlantis",0);
		cls(user,D_LINES);
		cls(user,0);
		return;
	}
	if (!user->doom) {
		write_user(user,"Ak chces hrat DOOM, pripoj sa najskor prikazom '~OL.doom start~RS'!\n");
		return;
	}
	if (!strcmp(word[1],"stop")) {
		if (user->doom_energy<50 && user->doom_energy!=0) {
			doom_text(user,"Nemas dost sil opustit DOOM! Vydrz!",1);
			return;
		}
		doom_quit(user);
		return;
	}
	if (!strcmp(word[1],"join")) {
		if (user->doom_energy) {
			sprintf(text,"Uz si zapojen%s v hre!\n",pohl(user,"y","a"));
			write_user(user,text);
			return;
		}
		if (doom_join(user)) {
			write_user(user,"Je mi luto, ale hracia plocha je plna... Skus to neskor.\n");
			return;
		}
		user->doom_energy=100;
		doom_players++;
		user->doom_heading=0;
		doom_map[user->doom_y][user->doom_x][4]=2;
		write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
		doom_text(user,"Kill'em ALL!",0);
		doom_checkround(user->doom_x,user->doom_y);
		sprintf(text,"%s sa zapojil%s do hry!",user->name,pohl(user,"","a"));
		doom_wall(user,text);
		return;
	}
	if (!user->doom_energy) {
		write_user(user,"Do hry sa zapojis napisanim prikazu \".doom join\"...\n");
		return;
	}
	if (!strcmp(word[1],"say")) {
		if (user->doom_sayon) {
			write_user(user,"Prikaz ~OLsay~RS je ~FRzablokovany~FW...\n");
			user->doom_sayon=0;
		}
		else {
			write_user(user,"Prikaz ~OLsay~RS je ~FGodblokovany~FW...\n");
			user->doom_sayon=1;
		}
		return;
	}
	if (!strcmp(word[1],"sl")) {
		if ((flag=doom_check(user->doom_x,user->doom_y,user->doom_heading+3))==1) {
			doom_map[user->doom_y][user->doom_x][4]=1;
			x=user->doom_x; y=user->doom_y;
			switch (user->doom_heading) {
				case 0:
					user->doom_x--;
					break;
				case 1:
					user->doom_y++;
					break;
				case 2:
					user->doom_x++;
					break;
				case 3:
					user->doom_y--;
					break;
			}
			doom_map[user->doom_y][user->doom_x][4]=2;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			doom_checkround(x,y);
			doom_checkround(user->doom_x,user->doom_y);
			return;
		}
		if (flag) {doom_text(user,"Niekto tam je!",1);return;}
		doom_text(user,"Tadial sa NEDA ist!",1); return;
	}
	if (!strcmp(word[1],"sr")) {
		if ((flag=doom_check(user->doom_x,user->doom_y,user->doom_heading+1))==1) {
			doom_map[user->doom_y][user->doom_x][4]=1;
			x=user->doom_x; y=user->doom_y;
			switch (user->doom_heading) {
				case 0:
					user->doom_x++;
					break;
				case 1:
					user->doom_y--;
					break;
				case 2:
					user->doom_x--;
					break;
				case 3:
					user->doom_y++;
					break;
			}
			doom_map[user->doom_y][user->doom_x][4]=2;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			doom_checkround(x,y);
			doom_checkround(user->doom_x,user->doom_y);
			return;
		}
		if (flag) {
			doom_text(user,"Niekto tam je!",1);
			return;
		}
		doom_text(user,"Tadial sa NEDA ist!",1); return;
	}
	if (!strcmp(word[1],"iddqd")) {
		doom_showmap(user);
		return;
	}
	if (!strcmp(word[1],"load")) {
		if (user->doom_loading) {
			doom_text(user,"Co vymyslas, ved tu zbran prave nabijas!",1);
			return;
		}
		if (((user->doom_weapon==1) && (user->doom_ammo[1]==D_AMMO)) || ((user->doom_weapon==2) && (user->doom_ammo[2]==1))) {
			doom_text(user,"Ved mas zbran doplna nabitu!",1);
			return;
		}
		if (!user->doom_weapon) {
			sprintf(text,"Vycistil%s si motorovu pilu od krvi a naolejoval%s si ju.",pohl(user,"","a"),pohl(user,"","a"));
			doom_text(user,text,1);
			return;
		}
		user->doom_loading=user->doom_weapon*5;
		sprintf(text,"Zacal%s si znovu nabijat svoju zbran...",pohl(user,"","a"));
		doom_text(user,text,0);
		return;
	}
	switch (tolower(word[1][0])) {
		case 'f':
			if ((flag=doom_check(user->doom_x,user->doom_y,user->doom_heading))==1) {
				doom_map[user->doom_y][user->doom_x][4]=1;
				x=user->doom_x;
				y=user->doom_y;
				switch (user->doom_heading) {
					case 0:
						user->doom_y++;
						break;
					case 1:
						user->doom_x++;
						break;
					case 2:
						user->doom_y--;
						break;
					case 3:
						user->doom_x--;
						break;
				}
				doom_map[user->doom_y][user->doom_x][4]=2;
				write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
				doom_checkround(x,y);
				doom_checkround(user->doom_x,user->doom_y);
				return;
			}
			if (flag) {
				doom_text(user,"Niekto tam je!",1);
				return;
			}
			doom_text(user,"Tadial sa NEDA ist!",1);
			return;
			break;
		case 'b':
			if ((flag=doom_check(user->doom_x,user->doom_y,user->doom_heading+2))==1) {
				doom_map[user->doom_y][user->doom_x][4]=1;
				x=user->doom_x; y=user->doom_y;
				switch (user->doom_heading) {
					case 0:
						user->doom_y--;
						break;
					case 1:
						user->doom_x--;
						break;
					case 2:
						user->doom_y++;
						break;
					case 3:
						user->doom_x++;
						break;
				}
				doom_map[user->doom_y][user->doom_x][4]=2;
				write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
				doom_checkround(x,y);
				doom_checkround(user->doom_x,user->doom_y);
				return;
			}
			if (flag) {
				doom_text(user,	"Niekto tam je!",1);
				return;
			}
			doom_text(user,"Tadial sa NEDA ist!",1);
			return;
			break;
		case 'r':
			user->doom_heading++;
			if (user->doom_heading > 3)
				user->doom_heading=0;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			doom_checkround(user->doom_x,user->doom_y);
			return;
			break;
		case 'l':
			user->doom_heading--;
			if (user->doom_heading < 0)
				user->doom_heading=3;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			doom_checkround(user->doom_x,user->doom_y);
			return;
			break;
		case '1':
			if (user->doom_loading) {
				doom_text(user,"Nemozes menit zbran ked ju nabijas!",1);
				return;
			}
			user->doom_weapon=0;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			break;
		case '2':
			if (user->doom_loading) {
				doom_text(user,"Nemozes menit zbran ked ju nabijas!",1);
				return;
			}
			user->doom_weapon=1;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			break;
		case '3':
			if (user->doom_loading) {
				doom_text(user,"Nemozes menit zbran ked ju nabijas!",1);
				return;
			}
			user->doom_weapon=2;
			write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			break;
		case 'x':
			if (user->doom_loading) {
				doom_text(user,"Nemozes strielat ked nabijas zbran!",1);
				return;
			}
			if ((user->doom_ammo[user->doom_weapon]) || (user->doom_weapon==0)) {
				if (user->doom_weapon) {
					user->doom_ammo[user->doom_weapon]--;
					write_user(user,"~LB~FR~OLPrask!~RS~FW\n");
				}
				else write_user(user,"~LB~FR~OLVrrum!~RS~FW\n");
				if ((u=doom_check_view(user))!=NULL) {
					doom_text(u,"Ktosi po tebe striela!!!",1);
					switch (user->doom_weapon) {
						case 0:
							u->doom_energy -= (1+rand()%5);
							break;
						case 1:
							u->doom_energy -= (5+rand()%10);
							break;
						case 2:
							u->doom_energy -= (20+rand()%40);
							break;
					}
					if (u->doom_energy<0)
						u->doom_energy=0;
					write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
					if (u->doom_energy==0) {
						sprintf(text,"%s pad%s mrtv%s na zem!",u->name,pohl(u,"ol","la"),pohl(u,"y","a"));
						doom_text(user,text,0);
						sprintf(text,"%s ta dostal%s!",user->name,pohl(user,"","a"));
						doom_text(u,text,1);
						sprintf(text,"~OLToto bolo na teba privela a so smrtelnym vykrikom si sa zrutil%s k zemi!\n",pohl(u,"","a"));
						write_user(u,text);
						sprintf(text,"Doom: %s/%s\n",user->name,u->name);
						log_game(text);
						u->doom_score--;
						user->doom_score++;
						doom_players--;
						write_doom(u,1);
						u->doom_energy=0;
						u->doom_ammo[0]=0;
						u->doom_ammo[1]=D_AMMO;
						u->doom_ammo[2]=1;
						u->doom_loading=0;
						doom_map[u->doom_y][u->doom_x][4]=1;
						sprintf(text,"%s to schytal%s a konci...",u->name,pohl(u,"","a"));
						doom_wall(user,text);
						add_point(user,DB_DOOM,1,0);
						add_point(u,DB_DOOM,-1,0);
					}
				}
				write_doom(user,doom_map[user->doom_y][user->doom_x][user->doom_heading]);
			}
			else
				doom_text(user,"Uz nemas nijake strelivo!",1);
			break;
		default:
			write_user(user,"DOOM: taky prikaz neexistuje!\n"); return;
			break;
	}
}

extern void doom_showmap(UR_OBJECT user)
{
	int y,x;

	write_user(user,"~FTDOOM MAP:~FW\n\n");
	for (x=0;x<D_MAPX+2;x++)
		write_user(user,"+");
	write_user(user,"\n+");
	for (y=D_MAPY-1;y>=0;y--) {
		for (x=0;x<D_MAPX;x++)
			if ((x==user->doom_x) && (y==user->doom_y))
				write_user(user,"~FRo");
			else {
				switch (doom_map[y][x][4]) {
					case 0:
						write_user(user,"+");
						break;
					case 1:
						write_user(user," ");
						break;
					case 2:
						write_user(user,"o");
						break;
					default:
						write_user(user,"?");
						break;
				}
			}
		write_user(user,"+\n+");
	}
	for (x=0;x<D_MAPX+1;x++)
		write_user(user,"+");
	write_user(user,"\n");
}

extern void write_doom(UR_OBJECT user,int location)
{
	UR_OBJECT u;
	char n,e,s,w,buffer[D_COLMS+5],name[USER_NAME_LEN+3],weapon[20],rating[20];
	int vidiho;

	echo_off(user);
	if (user->doom_score<10)
		strcpy(rating,"Neskodny");
	else if (user->doom_score<20)
		strcpy(rating,"Slaby");
	else if (user->doom_score<40)
		strcpy(rating,"Priemerny");
	else if (user->doom_score<80)
		strcpy(rating,"Schopny");
	else if (user->doom_score<160)
		strcpy(rating,"Utociaci");
	else if (user->doom_score<320)
		strcpy(rating,"Smrtiaci");
	else strcpy(rating,"- ELITA -");
	switch (user->doom_weapon) {
		case 0:
			strcpy(weapon,"Mot. pila");
			break;
		case 1:
			strcpy(weapon,"Gulovnica");
			break;
		case 2:
			strcpy(weapon,"Raketomet");
			break;
	}
	switch (user->doom_heading) {
		case 0:
			n='N'; e='E'; s='S'; w='W';
			break;
		case 1:
			n='E'; e='S'; s='W'; w='N';
			break;
		case 2:
			n='S'; e='W'; s='N'; w='E';
			break;
		case 3:
			n='W'; e='N'; s='E'; w='S';
			break;
		default:
			n='?'; e='?'; s='?'; w='?';
			break;
	}
	if ((u=doom_check_view(user))!=NULL && user->doom_energy) {
		vidiho=u->doom_heading-user->doom_heading;
		if (vidiho<0)
			vidiho+=4;
		vidiho++;
		strcpy(name,u->name);
	}
	else {
		vidiho=0;
		strcpy(name,"");
	}
	write_user(user,"\0337\033[1;1H");
	sprintf(text,"~BT~FK   K O M P A S    ~FT[]~FW~BK%s~BT~FT[]~FKP A R A M E T R E ~BK\n",doom_grf[location][0]);
	write_user(user,text);
	sprintf(text,"~BB~OL~FY        %c         ~RS~BT~FT[]~FW~BK%s~BT~FT[]~OL~BBVidis:~FY%-12s\n",n,doom_grf[location][1],name);
	write_user(user,text);
	sprintf(buffer,"%s",doom_grf[location][2]);
	switch (vidiho)	{
		case 1:
			buffer[20]='O';
			break;
		case 2:
			buffer[20]='>';
			break;
		case 3:
			buffer[20]='0';
			break;
		case 4:
			buffer[20]='<';
			break;
		default:
			break;
	}
	sprintf(text,"~BB~OL~FT       / \\        ~RS~BT~FT[]~FW~BK%s~BT~FT[]~OL~BBZbran   :~FY%-9s~BK\n",buffer,weapon);
	write_user(user,text);
	sprintf(buffer,"%s",doom_grf[location][3]);
	if (vidiho) {
		buffer[19]='/';
		buffer[20]='|';
		buffer[21]='\\';
	}
	sprintf(text,"~BB~OL~FY    %c~FT< > < >~FY%c     ~RS~FT~BT[]~FW~BK%s~BT~FT[]~OL~BBStrelivo:~FY%3d      ~BK\n",w,e,buffer,user->doom_ammo[user->doom_weapon]);
	write_user(user,text);
	sprintf(buffer,"%s",doom_grf[location][4]);
	if (vidiho) {
		buffer[19]='/';
		buffer[20]='^';
		buffer[21]='\\';
	}
	sprintf(text,"~BB~OL~FT       \\ /        ~RS~BT~FT[]~FW~BK%s~BT~FT[]~OL~BBSkore   :~FY%3d      \n",buffer,user->doom_score);
	write_user(user,text);
	sprintf(text,"~BB~OL~FY        %c         ~RS~BT~FT[]~FW~BK%s~BT~FT[]~OL~BBHodnost :~FY%-9s~BK\n",s,doom_grf[location][5],rating);
	write_user(user,text);
	write_user(user,"\0338");
	doom_text(user,"",0);
	echo_on(user);
}

extern int doom_join(UR_OBJECT user)
{
	int x,y,x1,y1;

	x1=x=rand()%D_MAPX;
	y1=y=rand()%D_MAPY;
	if (doom_map[y][x][4]==1) {
		user->doom_x=x;
		user->doom_y=y;
		doom_map[user->doom_y][user->doom_x][4]=2;
		return 0;
	}
	x++;
	while (!((y==y1)&&(x==x1))) {
		if (x==D_MAPX) {
			x=0;
			y++;
		}
		if (y==D_MAPY)
			y=0;
		if (doom_map[y][x][4]==1) {
			user->doom_x=x;
			user->doom_y=y;
			doom_map[user->doom_y][user->doom_x][4]=2;
			return 0;
		}
		x++;
	}
	return 1;
}

extern void doom_checkround(int x,int y)
{
	UR_OBJECT u;

	if (y+1 < D_MAPY)
		if ((u=doom_get_user(x,y+1))!=NULL)
			write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
	if (x+1 < D_MAPX)
		if ((u=doom_get_user(x+1,y))!=NULL)
			write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
	if (y>0)
		if ((u=doom_get_user(x,y-1))!=NULL)
			write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
	if (x>0)
		if ((u=doom_get_user(x-1,y))!=NULL)
			write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
}

extern int doom_check(int x,int y,int smer)
{
	if (smer>3)
		smer-=4;
	switch (smer) {
		case 0:
			if ((y+1)==D_MAPY)
				return 0;
			break;
		case 1:
			if ((x+1)==D_MAPX)
				return 0;
			break;
		case 2:
			if (y<=0)
				return 0;
			break;
		case 3:
			if (x<=0)
				return 0;
			break;
	}
	switch (smer) {
		case 0:
			if (!doom_map[y+1][x][4])
				return 0;
			break;
		case 1:
			if (!doom_map[y][x+1][4])
				return 0;
			break;
		case 2:
			if (!doom_map[y-1][x][4])
				return 0;
			break;
		case 3:
			if (!doom_map[y][x-1][4])
				return 0;
			break;
	}
	switch (doom_map[y][x][smer]) {
		case  3:
			return 0;
			break;
		case  6:
			return 0;
			break;
		case  8:
			return 0;
			break;
		case 17:
			return 0;
			break;
		case 18:
			return 0;
			break;
		case 20:
			return 0;
			break;
		default:
			break;
	}
	switch (smer) {
		case 0:
			if (doom_map[y+1][x][4]==2)
				return 2;
			break;
		case 1:
			if (doom_map[y][x+1][4]==2)
				return 2;
			break;
		case 2:
			if (doom_map[y-1][x][4]==2)
				return 2;
			break;
		case 3:
			if (doom_map[y][x-1][4]==2)
				return 2;
			break;
	}
	return 1;
}

extern UR_OBJECT doom_check_view(UR_OBJECT user)
{
	UR_OBJECT u=NULL;

	if (!doom_check(user->doom_x,user->doom_y,user->doom_heading))
		return 0;
	switch (user->doom_heading) {
		case 0:
			if (!(u=doom_get_user(user->doom_x,user->doom_y+1)))
				return NULL;
			break;
		case 1:
			if (!(u=doom_get_user(user->doom_x+1,user->doom_y)))
				return NULL;
			break;
		case 2:
			if (!(u=doom_get_user(user->doom_x,user->doom_y-1)))
				return NULL;
			break;
		case 3:
			if (!(u=doom_get_user(user->doom_x-1,user->doom_y)))
				return NULL;
			break;
	}
	return u;
}

extern void doom_text(UR_OBJECT user,char *msg,int beep)
{
	int x;
	char energy[40],sprava[100],temp[300];

	echo_off(user);
	write_user(user,"\0337\033[7;1H");
	strcpy(energy,"");
	for (x=0;x<25;x++) {
		if (x==5)
			strcat(energy,"~FW");
		if ((user->doom_energy-4*x)>0)
			strcat(energy,"#");
		else
			strcat(energy," ");
	}
	if (strlen(msg)) {
		sstrncpy(sprava,msg,40);
		sprava[40]='\0';
		while (strlen(sprava)<40) {
			strcat(sprava," ");
			if (strlen(sprava)<40) {
				sprintf(temp," %s",sprava);
				sstrncpy(sprava,temp,49);
			}
		}
	}
	else
		strcpy(sprava,"");
	sprintf(temp,"~BT~FK[~FB%02d~FK] Energia:[~OL~FR%s~RS~BT~FK]%s",doom_players,energy,sprava);
	write_user(user,temp);
	if (beep)
		write_user(user,"~LB");
	write_user(user,"\0338");
	echo_on(user);
}

extern void doom_who(UR_OBJECT user)
{
	UR_OBJECT u;

	if (!doom_players) {
		write_user(user,"~FTMomentalne DOOM nehra nikto...~FW\n");
		return;
	}
	write_user(user,"~FK~OL+-+-+-+-+-+-+-+-+-+-+-+-+\n~OL~FK| ~FYDoom hraju:     ~FK|~FYBody:~FK|\n~OL~FK+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->type!=USER_TYPE || u->room==NULL)
			continue;
		if (u->doom && u->doom_energy) {
			sprintf(text,"~OL~FK| ~FW%-15s ~FK| ~FW%03d ~FK|\n",u->name,u->doom_score);
			write_user(user,text);
		}
	}
	write_user(user,"~FK~OL+-+-+-+-+-+-+-+-+-+-+-+-+~RS~FW\n");
}

extern void doom_wall(UR_OBJECT user,char *msg)
{
	UR_OBJECT u;
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->type!=USER_TYPE || u==user || u->room==NULL)
			continue;
		if (u->doom)
			doom_text(u,msg,0);
	}
}

extern void doom_quit(UR_OBJECT user)
{
	char tmpstr[300];

	if (user==NULL || !user->doom)
		return;
	if (user->doom_energy) {
		doom_players--;
		doom_map[user->doom_y][user->doom_x][4]=1;
		doom_checkround(user->doom_x,user->doom_y);
		sprintf(tmpstr,"%s opustil%s hru...",user->name,pohl(user,"","a"));
		doom_wall(user,tmpstr);
		if (user->doom_energy < 50)
			add_point(user,DB_DOOM,-1,0);
	}
	user->doom=0;
	user->doom_energy=0;
	user->lines+=D_LINES;
	if (user->statline==CHARMODE)
		init_statline(user);
	else {
		write_user(user,"\033c\033[?7h");
		write_user(user,"\033[2;1r");
	}
	cls(user,0);
}

extern int doom_init()
{
	FILE *subor;
	int x,y,z,h,check;
	char string[100];

	if (!(subor=ropen(D_GRAPH,"r")))
		return 1;
	for (y=0;y<D_LOCAT;y++)
		for (x=0;x<6;x++)
			if (fgets(doom_grf[y][x],D_COLMS,subor)==NULL) {
				fclose(subor);
				return 1;
			}
			else doom_grf[y][x][40]='\0';
	fclose(subor);
	if (!(subor=ropen(D_DATA,"r")))
		return 1;
	for (y=(D_MAPY-1);y>=0;y--)
		for (x=0;x<D_MAPX;x++)
			if (fscanf(subor,"%s",string)==1)
				if (strlen(string)==8) {
					check=0;
					for (z=0;z<4;z++) {
						h=(10*(string[(z*2)]-'0'))+(string[(z*2)+1]-'0');
						check+=h;
						doom_map[y][x][z]=h;
						if (check)
							doom_map[y][x][4]=1;
						else
							doom_map[y][x][4]=0;
					}
				}
				else {
					fclose(subor);
					return 1;
				}
			else {
				fclose(subor);
				return 1;
			}
	fclose(subor);
	return 0;
}

extern void doom_load_users()
{
	UR_OBJECT u;

	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->type!=USER_TYPE || u->room==NULL)
			continue;
		if (u->doom && u->doom_energy)
			if (u->doom_energy<100) {
				u->doom_energy++;
				if (!(u->doom_energy%4))
					doom_text(u,"",0);
			}
		if (u->doom && u->doom_loading) {
			u->doom_loading--;
			if (u->doom_loading==0) {
				doom_text(u,"Tvoja zbran je znovu NABITA",1);
				switch (u->doom_weapon) {
					case 1:
						u->doom_ammo[u->doom_weapon]=D_AMMO;
						break;
					case 2:
						u->doom_ammo[u->doom_weapon]=1;
						break;
				}
				write_doom(u,doom_map[u->doom_y][u->doom_x][u->doom_heading]);
			}
		}
	}
}

int lab_load()
{
	int mode=0,lines=0,x=0,a,b,c,d,e,f;
	char string[MAX_LINE+1],filename[255];
	FILE *data;

	lab_room.start=0;
	lab_room.end=0;
	sprintf(filename,"%s%c%s",DATAFILES,DIRSEP,LAB_FILE);
	if (!(data=ropen(filename,"rt"))) {
		printf("CHYBA: Datafile nenajdeny\n");
		return 1;
	}
	string[0]='\0';
	while (fgets(string,sizeof(string)-1,data)!=NULL) {
		lines++;
		if (string[0]=='[') {
			mode=1;
			nxtznak(string);
			x=atoi(string);
			if (nxtznak(string)!=':') {
				printf("CHYBA na riadku %d: Znacka ':' chyba\n",lines);
				fclose(data);
				return 1;
			}
			a=atoi(string);
			if (nxtznak(string)!=',') {
				printf("CHYBA na riadku %d: Zla deklaracia alebo chyba ','\n",lines);
				fclose(data);
				return 1;
			}
			b=atoi(string);
			if (nxtznak(string)!=',') {
				printf("CHYBA na riadku %d: Zla deklaracia alebo chyba ','\n",lines);
				fclose(data);
				return 1;
			}
			c=atoi(string);
			if (nxtznak(string)!=',') {
				printf("CHYBA na riadku %d: Zla deklaracia alebo ',' chyba\n",lines);
				fclose(data);
				return 1;
			}
			d=atoi(string);
			if (nxtznak(string)!=',') {
				printf("CHYBA na riadku %d: Zla deklaracia alebo ',' chyba\n",lines);
				fclose(data);
				return 1;
			}
			e=atoi(string);
			if (nxtznak(string)!=',') {
				printf("CHYBA na riadku %d: Zla deklaracia alebo ',' chyba\n",lines);
				fclose(data);
				return 1;
			}
			f=atoi(string);
			if (nxtznak(string)!=']') {
				printf("CHYBA na riadku %d: Znacka ']' nenajdena\n",lines);
				fclose(data);
				return 1;
			}
			if (tolower(*string)=='e')
				lab_room.end=x;
			if (tolower(*string)=='s')
				lab_room.start=x;
			lab_room.sever[x]=a;
			lab_room.juh[x]=b;
			lab_room.vychod[x]=c;
			lab_room.zapad[x]=d;
			lab_room.hore[x]=e;
			lab_room.dole[x]=f;
			lab_room.popiska[x][0]='\0';
		}
		else if (mode==0 || string[0]=='#' || string[0]=='\n')
			continue;
		else
			strcat(lab_room.popiska[x],string);
		string[0]='\0';
	}
	fclose(data);
	if (mode==0) {
		printf("CHYBA: Nenasli sa identifikatori miestnosti\n");
		return 1;
	}
	if (lab_room.start==0) {
		printf("CHYBA: Startovacia pozicia(..]START) nedefinovana\n");
		return 1;
	}
	if (lab_room.end==0) {
		printf("CHYBA: Zaverecna pozicia(..]END) nedefinovana\n");
		return 1;
	}
	return 0;
}

char *cards[53][5]={
	{ ".-----. ","|~OLA~RS    | ","|  s  | ","|    ~OLA~RS| ","`-----' " },
	{ ".-----. ","|~OL2~RS    | ","|  s  | ","|    ~OL2~RS| ","`-----' " },
	{ ".-----. ","|~OL3~RS    | ","|  s  | ","|    ~OL3~RS| ","`-----' " },
	{ ".-----. ","|~OL4~RS    | ","|  s  | ","|    ~OL4~RS| ","`-----' " },
	{ ".-----. ","|~OL5~RS    | ","|  s  | ","|    ~OL5~RS| ","`-----' " },
	{ ".-----. ","|~OL6~RS    | ","|  s  | ","|    ~OL6~RS| ","`-----' " },
	{ ".-----. ","|~OL7~RS    | ","|  s  | ","|    ~OL7~RS| ","`-----' " },
	{ ".-----. ","|~OL8~RS    | ","|  s  | ","|    ~OL8~RS| ","`-----' " },
	{ ".-----. ","|~OL9~RS    | ","|  s  | ","|    ~OL9~RS| ","`-----' " },
	{ ".-----. ","|~OL10~RS   | ","|  s  | ","|   ~OL10~RS| ","`-----' " },
	{ ".-----. ","|~OLJ~RS    | ","|  s  | ","|    ~OLJ~RS| ","`-----' " },
	{ ".-----. ","|~OLQ~RS    | ","|  s  | ","|    ~OLQ~RS| ","`-----' " },
	{ ".-----. ","|~OLK~RS    | ","|  s  | ","|    ~OLK~RS| ","`-----' " },
	{ ".-----. ","|~OLA~RS    | ","|  ~FR~OLd~RS  | ","|    ~OLA~RS| ","`-----' " },
	{ ".-----. ","|~OL2~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL2~RS| ","`-----' " },
	{ ".-----. ","|~OL3~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL3~RS| ","`-----' " },
	{ ".-----. ","|~OL4~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL4~RS| ","`-----' " },
	{ ".-----. ","|~OL5~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL5~RS| ","`-----' " },
	{ ".-----. ","|~OL6~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL6~RS| ","`-----' " },
	{ ".-----. ","|~OL7~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL7~RS| ","`-----' " },
	{ ".-----. ","|~OL8~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL8~RS| ","`-----' " },
	{ ".-----. ","|~OL9~RS    | ","|  ~FR~OLd~RS  | ","|    ~OL9~RS| ","`-----' " },
	{ ".-----. ","|~OL10~RS   | ","|  ~FR~OLd~RS  | ","|   ~OL10~RS| ","`-----' " },
	{ ".-----. ","|~OLJ~RS    | ","|  ~FR~OLd~RS  | ","|    ~OLJ~RS| ","`-----' " },
	{ ".-----. ","|~OLQ~RS    | ","|  ~FR~OLd~RS  | ","|    ~OLQ~RS| ","`-----' " },
	{ ".-----. ","|~OLK~RS    | ","|  ~FR~OLd~RS  | ","|    ~OLK~RS| ","`-----' " },
	{ ".-----. ","|~OLA~RS    | ","|  c  | ","|    ~OLA~RS| ","`-----' " },
	{ ".-----. ","|~OL2~RS    | ","|  c  | ","|    ~OL2~RS| ","`-----' " },
	{ ".-----. ","|~OL3~RS    | ","|  c  | ","|    ~OL3~RS| ","`-----' " },
	{ ".-----. ","|~OL4~RS    | ","|  c  | ","|    ~OL4~RS| ","`-----' " },
	{ ".-----. ","|~OL5~RS    | ","|  c  | ","|    ~OL5~RS| ","`-----' " },
	{ ".-----. ","|~OL6~RS    | ","|  c  | ","|    ~OL6~RS| ","`-----' " },
	{ ".-----. ","|~OL7~RS    | ","|  c  | ","|    ~OL7~RS| ","`-----' " },
	{ ".-----. ","|~OL8~RS    | ","|  c  | ","|    ~OL8~RS| ","`-----' " },
	{ ".-----. ","|~OL9~RS    | ","|  c  | ","|    ~OL9~RS| ","`-----' " },
	{ ".-----. ","|~OL10~RS   | ","|  c  | ","|   ~OL10~RS| ","`-----' " },
	{ ".-----. ","|~OLJ~RS    | ","|  c  | ","|    ~OLJ~RS| ","`-----' " },
	{ ".-----. ","|~OLQ~RS    | ","|  c  | ","|    ~OLQ~RS| ","`-----' " },
	{ ".-----. ","|~OLK~RS    | ","|  c  | ","|    ~OLK~RS| ","`-----' " },
	{ ".-----. ","|~OLA~RS    | ","|  ~FR~OLh~RS  | ","|    ~OLA~RS| ","`-----' " },
	{ ".-----. ","|~OL2~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL2~RS| ","`-----' " },
	{ ".-----. ","|~OL3~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL3~RS| ","`-----' " },
	{ ".-----. ","|~OL4~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL4~RS| ","`-----' " },
	{ ".-----. ","|~OL5~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL5~RS| ","`-----' " },
	{ ".-----. ","|~OL6~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL6~RS| ","`-----' " },
	{ ".-----. ","|~OL7~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL7~RS| ","`-----' " },
	{ ".-----. ","|~OL8~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL8~RS| ","`-----' " },
	{ ".-----. ","|~OL9~RS    | ","|  ~FR~OLh~RS  | ","|    ~OL9~RS| ","`-----' " },
	{ ".-----. ","|~OL10~RS   | ","|  ~FR~OLh~RS  | ","|   ~OL10~RS| ","`-----' " },
	{ ".-----. ","|~OLJ~RS    | ","|  ~FR~OLh~RS  | ","|    ~OLJ~RS| ","`-----' " },
	{ ".-----. ","|~OLQ~RS    | ","|  ~FR~OLh~RS  | ","|    ~OLQ~RS| ","`-----' " },
	{ ".-----. ","|~OLK~RS    | ","|  ~FR~OLh~RS  | ","|    ~OLK~RS| ","`-----' " },
	{ ".-----. ","|~FRO~FGX~FRO~FGX~FRO~RS| ","|~FGX~FRO~FGX~FRO~FGX~RS| ","|~FRO~FGX~FRO~FGX~FRO~RS| ","`-----' " }
};

BJ_GAME create_blackjack_game(void)
{
	BJ_GAME bj;
	int i,j,tmp;

	if ((bj=(BJ_GAME)malloc(sizeof(struct blackjack_game_struct)))==NULL) {
		write_syslog("GAMES: Memory allocation failure in create_blackjack_game().\n",1);
		return NULL;
	}
	for (i=0;i<52;i++)
		bj->deck[i]=i;
	for (i=0;i<5;i++) {
		bj->hand[i]=-1;
		bj->dealer_hand[i]=-1;
	}
	bj->bet=10;
	bj->cardpos=0;
	srand(time(0));
	i=j=tmp=0;
	for (i=0;i<52;i++) {
		j=i+(rand()%(52-i));
		tmp=bj->deck[i];
		bj->deck[i]=bj->deck[j];
		bj->deck[j]=tmp;
	}
	bj->hand[0]=bj->deck[bj->cardpos];
	++bj->cardpos;
	bj->dealer_hand[0]=bj->deck[bj->cardpos];
	++bj->cardpos;
	bj->hand[1]=bj->deck[bj->cardpos];
	++bj->cardpos;
	bj->dealer_hand[1]=bj->deck[bj->cardpos];
	++bj->cardpos;
	bj->bet=DEFAULT_BJ_BET;
	return bj;
}

void destruct_blackjack_game(UR_OBJECT user)
{
	if (user->bj_game==NULL)
		return;
	free(user->bj_game);
	user->bj_game=NULL;
}

void play_blackjack(UR_OBJECT user)
{
	int i,user_total,dealer_total,cnt,blank;

	if (word_count<2) {
#if USE_MONEY_SYSTEM
		write_user(user,"Usage: bjack deal [<ante>]/hit/stand/double/surrender/status\n");
#else
		write_user(user,"Usage: bjack deal/hit/stand/surrender/status\n");
#endif
		return;
	}
	if (!strcasecmp(word[1],"deal")) {
		if (user->bj_game!=NULL) {
			write_user(user,"You are already playing a game of Blackjack.\n");
			return;
		}
		if ((user->bj_game=create_blackjack_game())==NULL) {
			write_user(user,"You just can't find a pack of cards when ya need 'em!\n");
			return;
		}
#if USE_MONEY_SYSTEM
		if (word_count>2) {
			user->bj_game->bet=atoi(word[2]);
			if (!user->bj_game->bet) {
				write_user(user,"If you're going bet then ante a good amount!\n");
				destruct_blackjack_game(user);
				return;
			}
		}
		if (user->money<user->bj_game->bet) {
			sprintf(text,"You haven't got enough money for the $%d ante!\n",user->bj_game->bet);
			write_user(user,text);
			destruct_blackjack_game(user);
			return;
		}
		user->money-=user->bj_game->bet;
		vwrite_user(user,"~FT~OLThe dealer says:~RS Ante's up!  The table bet is $%d.\n\n",user->bj_game->bet);
#endif
		write_user(user,"~FY~OLYour current blackjack hand is...\n");
		show_blackjack_cards(user,0,1);
		write_user(user,"\n~FM~OLThe dealer's hand is...\n");
		show_blackjack_cards(user,1,1);
		if ((user_total=check_blackjack_total(user,0))==21) {
#if USE_MONEY_SYSTEM
			vwrite_user(user,"~FT~OLThe dealer says:~RS You've just got ~OLBlackjack~RS, so you win $%d!\n",user->bj_game->bet*3);
			user->money+=user->bj_game->bet*3;
#else
			write_user(user,"~FT~OLThe dealer says:~RS You've just got ~OLBlackjack~RS!\n");
#endif
			destruct_blackjack_game(user);
		}
		else {
			sprintf(text,"\n~FT~OLThe dealer says:~RS You can now hit, stand%s or surrender.\n\n",(USE_MONEY_SYSTEM)?", double":"");
			write_user(user,text);
		}
		return;
	}
	if (!strcasecmp(word[1],"status")) {
		if (user->bj_game==NULL) {
			write_user(user,"You aren't playing a game of Blackjack.\n");
			return;
		}
		write_user(user,"~FY~OLYour current blackjack hand is...\n");
		show_blackjack_cards(user,0,1);
		write_user(user,"\n~FM~OLThe dealer's hand is...\n");
		show_blackjack_cards(user,1,1);
		sprintf(text,"~FT~OLThe dealer says:~RS You can now hit, stand%s or surrender (or see the status).\n\n",(USE_MONEY_SYSTEM)?", double":"");
		write_user(user,text);
		return;
	}
	if (!strcmp(word[1],"surrender")) {
		if (user->bj_game==NULL) {
			write_user(user,"You aren't playing a game of Blackjack.\n");
			return;
		}
#if USE_MONEY_SYSTEM
		user->money+=(user->bj_game->bet/2);
		vwrite_user(user,"~FT~OLThe dealer says:~RS Sorry, but you have surrendered and lose $%d - half your bet.\n",user->bj_game->bet/2);
#else
		write_user(user,"~FT~OLThe dealer says:~RS You have surrendered your game.\n");
#endif
		destruct_blackjack_game(user);
		return;
	}
	if (!strcasecmp(word[1],"hit")) {
		if (user->bj_game==NULL) {
			write_user(user,"You aren't playing a game of Blackjack.\n");
			return;
		}
		cnt=blank=0;
		for (i=0;i<5;i++) {
			if (user->bj_game->hand[i]==-1)
				blank++;
			else
				cnt++;
		}
		if (!blank) {
#if USE_MONEY_SYSTEM
			vwrite_user(user,"~FT~OLThe dealer says:~RS Well done! You got a five card hand and win $%d.\n",((user->bj_game->bet*2)+(user->bj_game->bet/2)));
			user->money+=((user->bj_game->bet*2)+(user->bj_game->bet/2));
#else
			write_user(user,"~FT~OLThe dealer says:~RS Well done!  You got a five card hand.\n");
#endif
			destruct_blackjack_game(user);
			return;
		}
		user->bj_game->hand[cnt]=user->bj_game->deck[user->bj_game->cardpos];
		++user->bj_game->cardpos;
		write_user(user,"~FY~OLYour current blackjack hand is...\n");
		show_blackjack_cards(user,0,0);
		user_total=check_blackjack_total(user,0);
		if (user_total>21) {
#if USE_MONEY_SYSTEM
			vwrite_user(user,"~FT~OLThe dealer says:~RS Sorry, but you have busted and lose your bet of $%d.\n",user->bj_game->bet);
#else
			write_user(user,"~FT~OLThe dealer says:~RS Sorry, but you have busted.\n");
#endif
			destruct_blackjack_game(user);
			return;
		}
		if (++cnt>=5) {
#if USE_MONEY_SYSTEM
			vwrite_user(user,"~FT~OLThe dealer says:~RS Well done!  You got a five card hand and win $%d.\n",((user->bj_game->bet*2)+(user->bj_game->bet/2)));
			user->money+=((user->bj_game->bet*2)+(user->bj_game->bet/2));
#else
			write_user(user,"~FT~OLThe dealer says:~RS Well done!  You have got a five card hand.\n");
#endif
			destruct_blackjack_game(user);
			return;
		}
		vwrite_user(user,"~FT~OLThe dealer says:~RS You can now hit, stand%s or surrender (or see the status).\n\n",(USE_MONEY_SYSTEM)?", double":"");
		return;
	}
#if USE_MONEY_SYSTEM
	if (!strcasecmp(word[1],"double")) {
		if (user->bj_game==NULL) {
			write_user(user,"You aren't playing a game of Blackjack.\n");
			return;
		}
		if (user->money<user->bj_game->bet) {
			write_user(user,"You can't afford to double your bet.\n");
			return;
		}
		user->money-=user->bj_game->bet;
		user->bj_game->bet+=user->bj_game->bet;
		cnt=blank=0;
		for (i=0;i<5;i++) {
			if (user->bj_game->hand[i]==-1)
				blank++;
			else
				cnt++;
		}
		if (!blank) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS Well done!  You have got a five card hand and win $%d.\n",((user->bj_game->bet*2)+(user->bj_game->bet/2)));
			user->money+=((user->bj_game->bet*2)+(user->bj_game->bet/2));
			write_user(user,"~FT~OLThe dealer says:~RS Well done!  You have got a five card hand.\n");
			destruct_blackjack_game(user);
			return;
		}
		user->bj_game->hand[cnt]=user->bj_game->deck[user->bj_game->cardpos];
		++user->bj_game->cardpos;
		vwrite_user(user,"You double your bet to $%d and draw just one more card...\n",user->bj_game->bet);
		show_blackjack_cards(user,0,0);
		if ((user_total=check_blackjack_total(user,0))>21) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS Sorry, but you busted and lose your bet of $%d.\n",user->bj_game->bet);
			destruct_blackjack_game(user);
			return;
		}
		if (++cnt>=5) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS Well done!  You have got a five card hand and win $%d.\n",((user->bj_game->bet*2)+(user->bj_game->bet/2)));
			user->money+=((user->bj_game->bet*2)+(user->bj_game->bet/2));
			destruct_blackjack_game(user);
			return;
		}
		write_user(user,"The dealer now takes their turn...\n");
		goto CARD_SKIP;
	}
#endif
	if (!strcasecmp(word[1],"stand")) {
		if (user->bj_game==NULL) {
			write_user(user,"You aren't playing a game of Blackjack.\n");
			return;
		}
		write_user(user,"~FY~OLYou stand, and the dealer takes their turn...\n");
		user_total=check_blackjack_total(user,0);
#if USE_MONEY_SYSTEM
		CARD_SKIP:
#endif
		cnt=blank=0;
		for (i=0;i<5;i++) {
			if (user->bj_game->dealer_hand[i]==-1) blank++;
			else cnt++;
		}
		if (!blank) {
#if USE_MONEY_SYSTEM
			vwrite_user(user,"~FT~OLThe dealer says:~RS I have a five card hand, so you lose $%d.\n",user->bj_game->bet);
#else
			write_user(user,"~FT~OLThe dealer says:~RS I have a five card hand, so you lose.\n");
#endif
			destruct_blackjack_game(user);
			return;
		}
		dealer_total=check_blackjack_total(user,1);
		while (dealer_total<17) {
			user->bj_game->dealer_hand[cnt]=user->bj_game->deck[user->bj_game->cardpos];
			++user->bj_game->cardpos;
			++cnt;
			dealer_total=check_blackjack_total(user,1);
			if (cnt>=5)
				break;
		}
		write_user(user,"\n~FM~OLThe dealer's hand is...\n");
		show_blackjack_cards(user,1,0);
#if USE_MONEY_SYSTEM
		if (dealer_total>21) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS I've busted so you win $%d.\n",user->bj_game->bet*2);
			user->money+=user->bj_game->bet*2;
		}
		else if (cnt>=5) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS I've got a five card hand, so you lose $%d.\n",user->bj_game->bet);
		}
		else if (dealer_total>user_total) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS I beat your score so you lose $%d.\n",user->bj_game->bet);
		}
		else if (dealer_total<user_total) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS You've beaten me so you win $%d.\n",user->bj_game->bet*2);
			user->money+=user->bj_game->bet*2;
		}
		else {
			vwrite_user(user,"~FT~OLThe dealer says:~RS Push! We've both got the same score so you get back your $%d!\n",user->bj_game->bet);
			user->money+=user->bj_game->bet;
		}
#else
		if (dealer_total>21) {
			vwrite_user(user,"~FT~OLThe dealer says:~RS I've busted so you win!\n");
		}
		else if (cnt>=5) {
			write_user(user,"~FT~OLThe dealer says:~RS I've got a five card hand, so you lose!\n");
		}
		else if (dealer_total>user_total) {
			write_user(user,"~FT~OLThe dealer says:~RS I've beaten yer score so you lose.\n");
		}
		else if (dealer_total<user_total) {
			write_user(user,"~FT~OLThe dealer says:~RS You've beaten me!\n");
		}
		else {
			write_user(user,"~FT~OLThe dealer says:~RS Push! We've both got the same score so it's a draw.\n");
		}
#endif
		destruct_blackjack_game(user);
		return;
	}
#if USE_MONEY_SYSTEM
	write_user(user,"Usage: bjack deal [<ante>]/hit/stand/double/surrender/status\n");
#else
	write_user(user,"Usage: bjack deal/hit/stand/surrender/status\n");
#endif
}

void show_blackjack_cards(UR_OBJECT user,int dealer,int start)
{
	int h,d,hand[5];
	char buff[80];

	if (dealer && start) {
		for (d=0;d<5;d++) {
			buff[0]='\0';
			for (h=0;h<5;++h) {
				if (user->bj_game->dealer_hand[h]==-1)
					continue;
				if (h==0)
					strcat(buff,cards[52][d]);
				else
					strcat(buff,cards[user->bj_game->dealer_hand[h]][d]);
			}
			vwrite_user(user,"%s\n",buff);
		}
		return;
	}
	if (!dealer)
		for (h=0;h<5;h++)
			hand[h]=user->bj_game->hand[h];
	else
		for (h=0;h<5;h++)
			hand[h]=user->bj_game->dealer_hand[h];
	for (d=0;d<5;d++) {
		buff[0]='\0';
		for (h=0;h<5;++h) {
			if (hand[h]==-1)
				continue;
			strcat(buff,cards[hand[h]][d]);
		}
		vwrite_user(user,"%s\n",buff);
	}
}

int check_blackjack_total(UR_OBJECT user,int dealer)
{
	int h,total,i,has_ace,all_aces_one;

	has_ace=all_aces_one=0;
	while (1) {
		total=0;
		if (!dealer) {
			for (h=0;h<5;h++) {
				if (user->bj_game->hand[h]==-1)
					continue;
				i=user->bj_game->hand[h]%13;
				switch(i) {
					case 0:
						if (!has_ace) {
							has_ace=1;
							total+=11;
						}
						else
							total++;
						break;
					case 10:
					case 11:
					case 12:
						total+=10;
						break;
					default:
						total+=(i+1);
						break;
				}
			}
		}
		else {
			for (h=0;h<5;h++) {
				if (user->bj_game->dealer_hand[h]==-1)
					continue;
				i=user->bj_game->dealer_hand[h]%13;
				switch(i) {
					case 0:
						if (!has_ace) {
							has_ace=1;  total+=11;
						}
						else
							total++;
						break;
					case 10:
					case 11:
					case 12:
						total+=10;
						break;
					default:
						total+=(i+1);
						break;
				}
			}
		}
		if (total>21 && has_ace && !all_aces_one)
			all_aces_one=1;
		else
			return total;
	}
}

char reversi_znaky[5][15]={"~FB.","~OL~FGX~RS","~OL~FWO~RS","~OL~FG~BBX~RS","~OL~FW~BBO~RS"};
int reversi_smerX[8]={0,1,1,1,0,-1,-1,-1};
int reversi_smerY[8]={-1,-1,0,1,1,1,0,-1};

void reversi(user)
UR_OBJECT user;
{
	int k,l,pocetX,pocetO;
	UR_OBJECT user_sh,winner;

	if (
			(word_count==1)
			||
			(word[1][0]<'0')
			||
			(
			 (word[1][0]>'9')
			 &&
			 (word[1][0]<'A')
			)
			||
			(
			 (word[1][0]>'Z')
			 &&
			 (word[1][0]<'a')
			)
			||
			(word[1][0]>'z')
	   ) {
		reversi_pishelp(user);
		return;
	}
	if (!strcmp(word[1],"stop")) {
		if (!user->reversi_cislotahu) {
			write_user(user,"Nehras reversi.\n");
			return;
		}
		user_sh=user->reversi_sh;
		vwrite_user(user,"Prestal%s si hrat reversi.\n",pohl(user,"","a"));
		sprintf(text,"%s prestal%s s tebou hrat reversi.\n",user->name,pohl(user,"","a"));
		write_user(user_sh,text);
		if (user->reversi_cislotahu>20) {
			add_point(user_sh,DB_REVERSI,1,0);
			add_point(user,DB_REVERSI,-1,0);
		}
		reversi_koniechry(user);
		reversi_koniechry(user_sh);
		sprintf(text,"Reve: %s/%s stop\n",user_sh->name,user->name);
		log_game(text);
		return;
	}
	if (!strcmp(word[1],"show")) {
		if (word_count>2) {
			user_sh=get_user(word[2]);
			if (user_sh==NULL) {
				write_user(user,notloggedon);
				return;
			}
			if (!user_sh->reversi_cislotahu) {
				sprintf(text,"%s nehra reversi.\n",user_sh->name);
				write_user(user,text);
				return;
			}
			sprintf(text,"Toto je rozohrana hra %s:",sklonuj(user_sh,2));
			write_user(user,text);
			reversi_pisplan(user,user_sh);
			reversi_pisplan2(user,user_sh);
			return;
		}
		if (!user->reversi_cislotahu) {
			write_user(user,"Nehras reversi.\n");
			return;
		}
		write_user(user,"Toto je tvoja rozohrana hra:");
		reversi_pisplan(user,user);
		reversi_pisplan2(user,user);
		return;
	}
	if (!strcmp(word[1],"pass")) {
		if (!user->reversi_cislotahu) {
			write_user(user,"Nehras reversi.\n");
			return;
		}
		if (user->reversi_natahu!=1) {
			write_user(user,"Nie si na tahu.\n");
			return;
		}
		if (user->reversi_cislotahu!=1) {
			write_user(user,"Nie je to prvy tah.\n");
			return;
		}
		user_sh=user->reversi_sh;
		user->reversi_natahu=1-user->reversi_natahu;
		user_sh->reversi_natahu=1-user_sh->reversi_natahu;
		reversi_pisplan(user,user);
		reversi_pisplan2(user,user);
		reversi_pisplan(user_sh,user_sh);
		reversi_pisplan2(user_sh,user_sh);
		sprintf(text,"Vzdal%s si sa prveho tahu. Na tahu je %s.\n",pohl(user,"","a"),user_sh->name);
		write_user(user,text);
		sprintf(text,"%s sa vzdal%s prveho tahu. Si na tahu.\n",user->name,pohl(user,"","a"));
		write_user(user_sh,text);
		return;
	}
	if (!strcmp(word[1],"about")) {
		write_user(user,"Reversi 1.2  Made by Alfonz (13.3.2000-2.4.2000)\n");
		return;
	}
	if (!strcmp(word[1],"special")) {
		char meno[USER_NAME_LEN+1];
		UR_OBJECT user_;
		if (word_count>2) {
			user_=get_user(word[2]);
			if (user_==NULL) {
				user_=user;
				write_user(user,notloggedon);
			}
		}
		else
			user_=user;
		user_sh=user_->reversi_sh;
		if (user_sh!=NULL)
			strcpy(meno,user_sh->name);
		else
			strcpy(meno,"NONAME");
		sprintf(text,"%s's special:          plan:\n",user_->name);
		write_user(user,text);
		for (l=0; l<8 ;++l) {
			for (k=0; k<8 ;++k) {
				sprintf(text,"%d ",user_->reversi_plan[l][k]);
				write_user(user,text);
			}
			write_user(user,"\n");
		}
		sprintf(text,"Meno_sh:%s   Na_tahu:%d   Znak:%d   Cislo_tahu:%d   Je_pt:%d   ptY:%d   ptX:%d\n",meno,user_->reversi_natahu,user_->reversi_znak,user_->reversi_cislotahu,user_->reversi_jept,user_->reversi_ptY,user_->reversi_ptX);
		write_user(user,text);
		return;
	}
	if ((word[1][0]>='0') && (word[1][0]<='9')) {
		if (!user->reversi_cislotahu) {
			write_user(user,"Nehras reversi.\n");
			return;
		}
		user_sh=user->reversi_sh;
		if (user->reversi_natahu!=1) {
			write_user(user,"Nie si na tahu.\n");
			return;
		}
		if (
				(word_count<3)
				||
				(strlen(word[1])>1)
				||
				(strlen(word[2])>1)
				||
				(word[1][0]>'7')
				||
				(word[2][0]<'0')
				||
				(word[2][0]>'7')
		   ) {
			write_user(user,"Chybne parametre.\n");
			reversi_pishelp(user);
			return;
		}
		k=word[2][0]-'0';
		l=word[1][0]-'0';
		if (user->reversi_plan[l][k]) {
			write_user(user,"Toto pole je obsadene.\n");
			return;
		}
		if (reversi_testok(user,l,k)) {
			reversi_tah(user,l,k);
			reversi_pisplan(user,user);
			reversi_pisplan(user_sh,user_sh);
			reversi_pocitajznaky(user,&pocetX,&pocetO);
			if (user->reversi_cislotahu>60) {
				if (pocetX==pocetO) {
					user->reversi_natahu=user_sh->reversi_natahu=2;
					reversi_pisplan2(user,user);
					reversi_pisplan2(user_sh,user_sh);
					sprintf(text,"Reversi duel %s vs %s skoncil remizou.\n",user->name,user_sh->name);
					write_room(user->room,text);
					write_room(user_sh->room,text);
					reversi_koniechry(user);
					reversi_koniechry(user_sh);
					return;
				}
				if (
						(
						 (pocetX>pocetO)
						 &&
						 (user->reversi_znak==1)
						)
						||
						(
						 (pocetX<pocetO)
						 &&
						 (user->reversi_znak==2)
						)
				   )
					winner=user;
				else
					winner=user_sh;
				user->reversi_natahu=user_sh->reversi_natahu=2;
				reversi_pisplan2(user,user);
				reversi_pisplan2(user_sh,user_sh);
				sprintf(text,"Z reversi duelu %s vs %s vysiel ako vitaz %s.\n",user->name,user_sh->name,winner->name);
				write_room(user->room,text);
				write_room(user_sh->room,text);
				if (winner==user) {
					add_point(winner,DB_REVERSI,1,0);
					add_point(user_sh,DB_REVERSI,-1,0);
				}
				else {
					add_point(winner,DB_REVERSI,1,0);
					add_point(user,DB_REVERSI,-1,0);
				}
				reversi_koniechry(user);
				reversi_koniechry(user_sh);
				return;
			}
			if (reversi_mozetahat(user_sh)) {
				reversi_pisplan2(user,user);
				reversi_pisplan2(user_sh,user_sh);
				return;
			}
			if (reversi_mozetahat(user)) {
				user_sh->reversi_natahu=1-user_sh->reversi_natahu;
				user->reversi_natahu=1-user->reversi_natahu;
				reversi_pisplan2(user,user);
				reversi_pisplan2(user_sh,user_sh);
				write_user(user,"Spoluhrac nemoze tahat. Si na tahu.\n");
				write_user(user_sh,"Nemozes tahat. Na tahu je spoluhrac.\n");
				return;
			}
			if (pocetX==pocetO) {
				user->reversi_natahu=user_sh->reversi_natahu=2;
				reversi_pisplan2(user,user);
				reversi_pisplan2(user_sh,user_sh);
				sprintf(text,"Reversi duel %s vs %s skoncil remizou.\n",user->name,user_sh->name);
				write_room(user->room,text);
				write_room(user_sh->room,text);
				reversi_koniechry(user);
				reversi_koniechry(user_sh);
				sprintf(text,"Reve: %s-%s remiza\n",user_sh->name,user->name);
				log_game(text);
				return;
			}
			if (
					(
					 (pocetX>pocetO)
					 &&
					 (user->reversi_znak==1)
					)
					||
					(
					 (pocetX<pocetO)
					 &&
					 (user->reversi_znak==2)
					)
			   )
				winner=user;
			else
				winner=user_sh;
			user->reversi_natahu=user_sh->reversi_natahu=2;
			reversi_pisplan2(user,user);
			reversi_pisplan2(user_sh,user_sh);
			sprintf(text,"Z reversi duelu %s vs %s vysiel ako vitaz %s.\n",user->name,user_sh->name,winner->name);
			write_room(user->room,text);
			write_room(user_sh->room,text);
			if (winner==user) {
				add_point(winner,DB_REVERSI,1,0);
				add_point(user_sh,DB_REVERSI,-1,0);
			}
			else {
				add_point(winner,DB_REVERSI,1,0);
				add_point(user,DB_REVERSI,-1,0);
			}
			reversi_koniechry(user);
			reversi_koniechry(user_sh);
			sprintf(text,"Reve: %s:%s Win:%s\n",user_sh->name,user->name,winner->name);
			log_game(text);
			return;
		}
		write_user(user,"Podla pravidiel tu nemozes tahat.\n");
		return;
	}
	if (user->reversi_cislotahu) {
		sprintf(text,"Uz hras reversi s %s.\n",sklonuj(user->reversi_sh,7));
		write_user(user,text);
		return;
	}
	user_sh=get_user(word[1]);
	if (user_sh==user) {
		write_user(user,"Nemozes hrat so sebou.\n");
		return;
	}
	if (user_sh==NULL) {
		write_user(user,notloggedon);
		return;
	}
	if (user_sh->reversi_cislotahu) {
		sprintf(text,"%s uz hra reversi.\n",user_sh->name);
		write_user(user,text);
		return;
	}
	if (user_sh->igngames) {
		sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",user_sh->name);
		write_user(user,text);
		return;
	}
	if (word_count>2) {
		if (!strcmp(word[2],"o")) {
			user->reversi_znak=2;
			user_sh->reversi_znak=1;
		}
		else {
			user->reversi_znak=1;
			user_sh->reversi_znak=2;
		}
	}
	else {
		user->reversi_znak=1;
		user_sh->reversi_znak=2;
	}
	user->reversi_cislotahu=user_sh->reversi_cislotahu=1;
	user->reversi_natahu=1;
	user_sh->reversi_natahu=0;
	user->reversi_sh=user_sh;
	user_sh->reversi_sh=user;
	for (k=0;k<8;++k)
		for (l=0;l<8;++l)
			user->reversi_plan[k][l]=user_sh->reversi_plan[k][l]=0;
	user->reversi_plan[3][3]=user->reversi_plan[4][4]=user_sh->reversi_plan[3][3]=user_sh->reversi_plan[4][4]=1;
	user->reversi_plan[3][4]=user->reversi_plan[4][3]=user_sh->reversi_plan[3][4]=user_sh->reversi_plan[4][3]=2;
	sprintf(text,"%s si hrat reversi s %s.\n",pohl(user,"Zacal","Zacala"),sklonuj(user_sh,7));
	write_user(user,text);
	sprintf(text,"%s %s s tebou hrat reversi.\n",user->name,pohl(user,"zacal","zacala"));
	write_user(user_sh,text);
	reversi_pisplan(user,user);
	reversi_pisplan2(user,user);
	reversi_pisplan(user_sh,user_sh);
	reversi_pisplan2(user_sh,user_sh);
	return;
}

void reversi_pishelp(UR_OBJECT user)
{
	write_user(user,"Umozni ti zahrat si s niekym reversi.\n");
	write_user(user,"Pouzitie: reversi <user> [o]        - zacne novu hru s uzitatelom <user>\n");
	write_user(user,"                                        o - budes mat znak O\n");
	write_user(user,"          reversi show [<user>]     - ukaze ti rozohranu hru (tvoju/userovu)\n");
	write_user(user,"          reversi pass              - prenecha prvy tah hry superovi\n");
	write_user(user,"          reversi <riadok> <stlpec> - tah na urcene pole\n");
	write_user(user,"          reversi stop              - ukonci rozohranu hru\n");
}

void reversi_pisplan(UR_OBJECT pisuser,UR_OBJECT user)
{
	int k,l,pX,pO;
	UR_OBJECT userX,userO;

	pX=pO=0;
	write_user(pisuser,"\n                                 ~FG0 1 2 3 4 5 6 7\n");
	for (l=0; l<8 ;++l) {
		sprintf(text,"                               ~FG%-1.1d",l);
		write_user(pisuser,text);
		for (k=0;k<8;++k) {
			sprintf(text," %s",reversi_znaky[user->reversi_plan[l][k]+2*(user->reversi_jept)*(l==user->reversi_ptY)*(k==user->reversi_ptX)]);
			write_user(pisuser,text);
			switch (user->reversi_plan[l][k]) {
				case 0:
					break;
				case 1:
					pX+=1;
					break;
				case 2:
					pO+=1;
					break;
			}
		}
		sprintf(text," ~FG%-1.1d\n",l);
		write_user(pisuser,text);
	}
	write_user(pisuser,"                                 ~FG0 1 2 3 4 5 6 7\n");
	if (user->reversi_znak==1) {
		userX=user;
		userO=user->reversi_sh;
	}
	else {
		userX=user->reversi_sh;
		userO=user;
	}
	sprintf(text,"~FTStav: %s (~FWX~FT):~OL~FW%d~RS  ~FT%s (~FWO~FT):~OL~FW%d~RS\n",userX->name,pX,userO->name,pO);
	for (k=(57-strlen(text)/2); k>=0 ;--k)
		write_user(pisuser," ");
	write_user(pisuser,text);
}

void reversi_pisplan2(UR_OBJECT pisuser,UR_OBJECT user)
{
	UR_OBJECT userX,userO;
	char natahu[USER_NAME_LEN+28];
	int k,i=0;
	char posltah[50];

	if (user->reversi_znak==1) {
		userX=user;
		userO=user->reversi_sh;
	}
	else {
		userX=user->reversi_sh;
		userO=user;
	}
	switch (userX->reversi_natahu) {
		case 0:
			sprintf(natahu," Na tahu je %s (~FWO~FT).",userO->name);
			i=6;
			break;
		case 1:
			sprintf(natahu," Na tahu je %s (~FWX~FT).",userX->name);
			i=6;
			break;
		case 2:
			strcpy(natahu,"");
			i=0;
			break;
	}
	if (user->reversi_jept) {
		sprintf(posltah,", posledny tah na [~OL~FW%d~RS~FT,~OL~FW%d~RS~FT]",user->reversi_ptY,user->reversi_ptX);
		i+=24;
	}
	else
		strcpy(posltah,"");
	sprintf(text,"~FTTah c. ~OL~FW%d~RS~FT%s.%s\n",user->reversi_cislotahu,posltah,natahu);
	for (k=((80+16+i-strlen(text))/2); k>0 ;--k)
		write_user(pisuser," ");
	write_user(pisuser,text);
}

void reversi_pocitajznaky(UR_OBJECT user,int *pX,int *pO)
{
	int k,l;

	*pX=*pO=0;
	for (l=0;l<8;++l)
		for (k=0;k<8;++k)
			switch (user->reversi_plan[l][k]) {
				case 0:break;
				case 1:*pX+=1;  break;
				case 2:*pO+=1;  break;
			}
}

int reversi_testok(UR_OBJECT user,int y,int x)
{
	int k,l,kn,ln,i,j;
	int ok=0;

	if (user->reversi_plan[y][x])
		return 0;
	for (i=0; (!ok)&&(i<8) ;++i)
		for (k=x,l=y,j=0; !ok; k=kn,l=ln,++j) {
			kn=k+reversi_smerX[i];
			ln=l+reversi_smerY[i];
			if ((kn<0)||(kn>7)||(ln<0)||(ln>7))
				break;
			if (user->reversi_plan[ln][kn]==0)
				break;
			if ((user->reversi_plan[ln][kn])==(user->reversi_znak)) {
				if (j==0)
					break;
				ok=1;
				break;
			}
		}
	return ok;
}

int reversi_mozetahat(UR_OBJECT user)
{
	int k,l,ok;

	ok=0;
	for (l=0; (!ok)&&(l<8) ;++l)
		for (k=0; (!ok)&&(k<8) ;++k)
			ok=reversi_testok(user,l,k);
	return ok;
}

void reversi_tah(UR_OBJECT user,int y,int x)
{
	int k,l,kn,ln,i,j,ok;

	for (i=0; i<8 ;++i) {
		ok=0;
		for (k=x,l=y,j=0; !ok ;k=kn,l=ln,++j) {
			kn=k+reversi_smerX[i];
			ln=l+reversi_smerY[i];
			if ((kn<0)||(kn>7)||(ln<0)||(ln>7))
				break;
			if (user->reversi_plan[ln][kn]==0)
				break;
			if (user->reversi_plan[ln][kn]==user->reversi_znak) {
				if (j==0)
					break;
				ok=1;
				break;
			}
		}
		if (ok)
			for (k=x,l=y; ;k=kn,l=ln) {
				kn=k+reversi_smerX[i];
				ln=l+reversi_smerY[i];
				if (user->reversi_plan[ln][kn]==user->reversi_znak)
					break;
				user->reversi_plan[ln][kn]=3-user->reversi_plan[ln][kn];
				user->reversi_sh->reversi_plan[ln][kn]=3-user->reversi_sh->reversi_plan[ln][kn];
			}
	}
	user->reversi_jept=user->reversi_sh->reversi_jept=1;
	user->reversi_ptX=user->reversi_sh->reversi_ptX=x;
	user->reversi_ptY=user->reversi_sh->reversi_ptY=y;
	user->reversi_plan[y][x]=user->reversi_znak;
	user->reversi_sh->reversi_plan[y][x]=user->reversi_znak;
	user->reversi_natahu=1-user->reversi_natahu;
	user->reversi_sh->reversi_natahu=1-user->reversi_sh->reversi_natahu;
	user->reversi_cislotahu+=1;
	user->reversi_sh->reversi_cislotahu+=1;
}

void reversi_koniechry(user)
UR_OBJECT user;
{
	int k,l;

	for (l=0; l<8 ;++l) for (k=0; k<8 ;++k)
		user->reversi_plan[l][k]=0;
	user->reversi_sh=NULL;
	user->reversi_natahu=-1;
	user->reversi_znak=user->reversi_cislotahu=user->reversi_jept=user->reversi_ptX=user->reversi_ptY=0;
}

#define M_X user->miny_x
#define M_Y user->miny_y
#define HOW_MINES user->miny_h

#define M_HIDDEN  50
#define M_THERE   10
#define M_FLAG    100
#define M_NOTHING 0

#define M_TAB(a,b) (user->miny_tab[a][b])

void miny_help_msg(UR_OBJECT user)
{
	write_user(user,"Pouzi: .miny start <easy/medium/hard>  - zacne hru\n");
	write_user(user,"       .miny stop                      - zrusi hru \n");
	write_user(user,"       .miny <riadok> <stlpec>         - odkryje policko\n");
	write_user(user,"       .miny <riadok> <stlpec> !       - oznaci predpokladanu minu\n");
	write_user(user,"       .miny draw                      - vykresli hracie pole\n");
}

char *miny_alloc_msg = "~FRProblemy s pametou, kontaktuj spravcov!\n";

void miny_draw(UR_OBJECT user)
{
	unsigned int i,j;
	char *buff;
	int Hidden=0,Mines=0;

	for (j=0;j<M_Y;j++)
		for (i=0;i<M_X;i++) {
			if (M_TAB(j,i)>=M_HIDDEN && M_TAB(j,i)<M_FLAG)
				Hidden++;
			if (M_TAB(j,i)>=M_FLAG)
				Mines++;
		}
	if (!(buff=(char *)malloc((user->miny_x)*15+100))) {
		write_user(user,miny_alloc_msg);
		return;
	}
	sprintf(buff,"~FG  ");
	for (i = 0;i < M_X;i++)
		sprintf(buff+5+(i*2),"%2d",i);
	strcat(buff,"\n");
	write_user(user,buff);
	for (j=0;j<M_Y;j++) {
		sprintf(buff,"~FG%2d ",j);
		for (i = 0;i < M_X; i++) {
			if (M_TAB(j,i)>=M_FLAG)
				sprintf(buff+6+(i*8),"~OL~FMF ");
			else if (M_TAB(j,i)>=M_HIDDEN)
				sprintf(buff+6+(i*8),"~RS~FW# ");
			else if (M_TAB(j,i)==M_THERE)
				sprintf(buff+6+(i*8),"~OL~FY* ");
			else if (M_TAB(j,i)==0)
				sprintf(buff+6+(i*8),"~OL~FK. ");
			else if (M_TAB(j,i)==1)
				sprintf(buff+6+(i*8),"~OL~FB1 ");
			else if (M_TAB(j,i)==2)
				sprintf(buff+6+(i*8),"~RS~FG2 ");
			else if (M_TAB(j,i)==3)
				sprintf(buff+6+(i*8),"~OL~FR3 ");
			else if (M_TAB(j,i)==4)
				sprintf(buff+6+(i*8),"~RS~FY4 ");
			else if (M_TAB(j,i)==5)
				sprintf(buff+6+(i*8),"~RS~FT5 ");
			else
				sprintf(buff+6+(i*8),"~OL~FW%c ",M_TAB(j,i)+'0');
		}
		sprintf(text,"~RS~FG%-2d",j);
		strcat(buff,text);
		if (j==2) {
			sprintf(text,"  ~RS~FTOdkryte:   ~OL%d",M_X*M_Y-Hidden);
			strcat(buff,text);
		}
		if (j==3) {
			sprintf(text,"  ~RS~FTNeodkryte: ~OL%d",Hidden);
			strcat(buff,text);
		}
		if (j==5) {
			sprintf(text,"  ~RS~FTMiny: ~OL%d",user->miny_h-Mines);
			strcat(buff,text);
		}
		strcat(buff,"\n");
		write_user(user,buff);
	}
	sprintf(buff,"~RS~FG  ");
	for (i=0;i<M_X;i++)
		sprintf(buff+8+(i*2),"%2d",i);
	strcat(buff,"\n");
	write_user(user,buff);
	free(buff);
}

void miny_numbering(UR_OBJECT user,unsigned int j,unsigned int i)
{
	if (i+1<M_X) {
		M_TAB(j,i+1)+=(M_TAB(j,i+1)!=M_HIDDEN+M_THERE)?1:0;
		if (j-1!=0)
			M_TAB(j-1,i+1)+=(M_TAB(j-1,i+1)!=M_HIDDEN+M_THERE)?1:0;
		if (j+1<M_Y)
			M_TAB(j+1,i+1)+=(M_TAB(j+1,i+1)!=M_HIDDEN+M_THERE)?1:0;
	}
	if (i-1!=0) {
		M_TAB(j,i-1)+=(M_TAB(j,i-1)!=M_HIDDEN+M_THERE)?1:0;
		if (j-1!=0)
			M_TAB(j-1,i-1)+=(M_TAB(j-1,i-1)!=M_HIDDEN+M_THERE)?1:0;
		if (j+1<M_Y)
			M_TAB(j+1,i-1)+=(M_TAB(j+1,i-1)!=M_HIDDEN+M_THERE)?1:0;
	}
	if (j+1<M_Y)
		M_TAB(j+1,i)+=(M_TAB(j+1,i)!=M_HIDDEN+M_THERE)?1:0;
	if (j-1!=0)
		M_TAB(j-1,i)+=(M_TAB(j-1,i)!=M_HIDDEN+M_THERE)?1:0;
}

int miny_init(UR_OBJECT user)
{
	unsigned int i,j;

	user->miny_tab=(unsigned char **)malloc(sizeof(unsigned char *)*M_Y);
	if (user->miny_tab==0)
		return -1;
	for (j=0;j<M_Y;j++) {
		user->miny_tab[j]=(unsigned char *)malloc(sizeof(unsigned char)*M_X);
		if (!user->miny_tab[j])
			return -1;
	}
	for (j=0;j<M_Y;j++)
		for (i=0;i<M_X;i++)
			M_TAB(j,i)=M_HIDDEN;
	M_TAB(0,0)=(char)M_HIDDEN+9;
	return 0;
}

void miny_placing(UR_OBJECT user,unsigned int oj,unsigned int oi)
{
	unsigned int i,j,total;
	time_t t;

	srand((unsigned) time(&t));
	for (total=0;total<HOW_MINES;) {
		j=rand()%M_Y;
		i=rand()%M_X;
		if ((M_TAB(j,i)!=M_THERE+M_HIDDEN) && (j!=oj) && (i!=oi)) {
			M_TAB(j,i)=M_THERE + M_HIDDEN;
			total++;
			miny_numbering(user,j,i);
		}
	}
}

void miny_zero_show(UR_OBJECT user,unsigned int j,unsigned int i)
{
	M_TAB(j,i)=M_TAB(j,i)-M_HIDDEN;
	if (
			(i+1<M_X)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j,i+1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j,i+1)>=M_HIDDEN)
			  &&
			  (M_TAB(j,i+1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j,i+1);
	if (
			(i-1<M_X)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j,i-1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j,i-1)>=M_HIDDEN)
			  &&
			  (M_TAB(j,i-1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
	       	miny_zero_show(user,j,i-1);
	if (
			(j+1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j+1,i)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j+1,i)>=M_HIDDEN)
			  &&
			  (M_TAB(j+1,i)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j+1,i);
	if (
			(j-1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j-1,i)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j-1,i)>=M_HIDDEN)
			  &&
			  (M_TAB(j-1,i)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j-1,i);
	if (
			(i-1<M_X)
			&&
			(j-1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j-1,i-1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j-1,i-1)>=M_HIDDEN)
			  &&
			  (M_TAB(j-1,i-1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j-1,i-1);
	if (
			(i-1<M_X)
			&&
			(j+1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j+1,i-1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j+1,i-1)>=M_HIDDEN)
			  &&
			  (M_TAB(j+1,i-1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j+1,i-1);
	if (
			(i+1<M_X)
			&&
			(j+1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j+1,i+1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j+1,i+1)>=M_HIDDEN)
			  &&
			  (M_TAB(j+1,i+1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j+1,i+1);
	if (
			(i+1<M_X)
			&&
			(j-1<M_Y)
			&&
			(M_TAB(j,i)==0)
			&&
			(
			 (M_TAB(j-1,i+1)==M_HIDDEN)
			 ||
			 (
			  (M_TAB(j-1,i+1)>=M_HIDDEN)
			  &&
			  (M_TAB(j-1,i+1)<M_THERE+M_HIDDEN)
			 )
			)
	   )
		miny_zero_show(user,j-1,i+1);
}

void miny_done(UR_OBJECT user)
{
	unsigned int j;

	if (user->miny_tab) {
		for (j=0;j<M_Y;j++)
			if (user->miny_tab[j])
				free(user->miny_tab[j]);
		free(user->miny_tab);
		user->miny_tab=NULL;
	}
}


void miny(UR_OBJECT user)
{
	unsigned int i,j,b;
	int ri,rj;
	char buff[100];

	if (word_count<2 || word_count>4) {
		miny_help_msg(user);
		return;
	}

	if (word_count==2) {
		if (!strcasecmp(word[1],"draw") || !strcasecmp(word[1],"show")) {
			if (!user->miny_tab)
				write_user(user,"Este Nehras hru.\n");
			else
				miny_draw(user);
		}
		else if (!strcasecmp(word[1],"stop")) {
			if (!user->miny_tab)
				write_user(user,"Nehras miny.\n");
			else {
				vwrite_user(user,"%s si hru miny.\n",pohl(user,"Skoncil","Skoncila"));
				miny_done(user);
			}
		}
		else
			miny_help_msg(user);
	}
	else if (word_count>=3) {
		if (!strcasecmp(word[1],"start")) {
			if (user->miny_tab) {
				write_user(user,"Uz hras hru miny.\n");
				return;
			}
			if (!strcasecmp(word[2],"easy")) {
				user->miny_x = 8;
				user->miny_y = 8;
				user->miny_h = 10;
			}
			else if (!strcasecmp(word[2],"medium")) {
				user->miny_x = 16;
				user->miny_y = 16;
				user->miny_h = 40;
			}
			else if (!strcasecmp(word[2],"hard")) {
				user->miny_x = 22;
				user->miny_y = 16;
				user->miny_h = 70;
			}
			else {
				miny_help_msg(user);
				return;
			}
			if (miny_init(user)) {
				write_user(user,miny_alloc_msg);
				return;
			}
			miny_draw(user);
		}
		else {
			if (!user->miny_tab) {
				write_user(user,"Este si nezacal hrat hru.\n");
				return;
			}
			sscanf(word[1],"%d",&rj);
			sscanf(word[2],"%d",&ri);
			if (ri < 0 || rj < 0) {
				sprintf(buff,"Suradnice nemozu byt zaportne!\n");
				write_user(user,buff);
				return;
			}
			j=rj;
			i=ri;
			if (i > (M_X-1) || j > (M_Y-1)) {
				sprintf(buff,"Zadaj suradnice <0..%d> <0..%d> !\n",(user->miny_y-1),(user->miny_x-1));
				write_user(user,buff);
				return;
			}
			if (M_TAB(0,0) == (char)M_HIDDEN+9) {
				M_TAB(0,0) = M_HIDDEN;
				miny_placing(user,j,i);
			}
			if (strpbrk(word[2],"!")) {
				word_count=4;
				strcpy(word[3],"!");
			}
			if (word_count>3) {
				if (M_TAB(j,i)>=M_HIDDEN && M_TAB(j,i)<M_FLAG) {
					M_TAB(j,i)+=M_FLAG;
					miny_draw(user);
					return;
				}
				if (M_TAB(j,i)>=M_FLAG) {
					M_TAB(j,i)-=M_FLAG;
					miny_draw(user);
					return;
				}
			}
			if (M_TAB(j,i)>=M_FLAG)
				M_TAB(j,i)-=M_FLAG;
			if (M_TAB(j,i)>=M_HIDDEN) {
				if (M_TAB(j,i) == M_HIDDEN)
					miny_zero_show(user,j,i);
				else
					M_TAB(j,i) -= M_HIDDEN;
				miny_draw(user);
				for (j=b=0;j<M_Y;j++) {
					for (i=0;i<M_X;i++) {
						if (M_TAB(j,i)==M_THERE) {
							vwrite_user(user,"~FG~OL....Whooopz....\n~FG~OLNas%s si Minu.\n",pohl(user,"iel","la"));
							for (j=b=0;j<M_Y;j++)
								for (i=0; i<M_X;i++)
									if (M_TAB(j,i)<M_HIDDEN) b++;
							if (b>6) {
								if (M_X==8)
									add_point(user,DB_MINY,-2,0);
								if (M_X==16)
									add_point(user,DB_MINY,-10,0);
								if (M_X==22)
									add_point(user,DB_MINY,-20,0);
							}
							miny_done(user);
							return;
						}
						else if (M_TAB(j,i) < M_HIDDEN)
							b++;
					}
				}
				if (b==(M_X*M_Y)-HOW_MINES) {
					vwrite_user(user,"~FG~OLVyhral%s si, gratulujem.\n",pohl(user,"","a"));
					if (M_X==8)
						add_point(user,DB_MINY,5,0);
					if (M_X==16)
						add_point(user,DB_MINY,20,0);
					if (M_X==22)
						add_point(user,DB_MINY,40,0);
					miny_done(user);
				}
			}
			else
				write_user(user,"Uz odkryte policko\n");
		}
	}
}

DAMA_OBJECT create_dama()
{
	DAMA_OBJECT dama;
	int i,ii;
	if ((dama=(DAMA_OBJECT)malloc(sizeof(struct dama_struct)))==NULL) {
		write_syslog("HUH: Memory allocation failure in create_dama().\n",0);
		return NULL;
	}
	for (i=0;i<8;++i)
		for (ii=0;ii<8;++ii)
			dama->plocha[ii][i]=0;
	dama->natahu=0;
	dama->forcejump=0;
	dama->lastmove[0]=10;
	dama->lastmove[1]=10;
	dama->disptype[0]=1;
	dama->disptype[1]=1;
	dama->remiza[0]=0;
	dama->remiza[1]=0;
	dama->monochrom[0]=0;
	dama->monochrom[1]=0;
	dama->hrac[0]=NULL;
	dama->hrac[1]=NULL;
	return dama;
}

void destruct_dama(UR_OBJECT user)
{
	free(user->dama);
}

void dama_save(UR_OBJECT user)
{
	FILE *fp;
	char filename[80];
	int x,y;

	sprintf(filename,"%s/%s.sav",DAMAFILES,user->name);
	if (!(fp=fopen(filename,"w"))) {
		sprintf(text,"%s: problem so zapisovanim do suboru.\n",syserror);
		write_user(user,text);
		return;
	}
	fprintf(fp,"%s\n",user->dama->hrac[2-user->dhrac]->name);
	fprintf(fp,"%d %d %d %d\n",user->dhrac,user->dama->hrac[2-user->dhrac]->dhrac,user->dama->forcejump,user->dama->natahu);
	fprintf(fp,"%d %d\n",user->dama->monochrom[0],user->dama->monochrom[1]);
	fprintf(fp,"%d %d %d %d\n",user->dama->lastmove[0],user->dama->lastmove[1],user->dama->disptype[0],user->dama->disptype[1]);
	for (y=0;y<8;++y)
		for (x=0;x<8;++x)
			if (x<7)
				fprintf(fp,"%d ",user->dama->plocha[x][y]);
			else
				fprintf(fp,"%d\n",user->dama->plocha[x][y]);
	fclose(fp);
}

void dama_load(UR_OBJECT user)
{
	FILE *fp;
	char filename[80],tempusername[USER_NAME_LEN+1];
	int x,y;

	sprintf(filename,"%s/%s.sav",DAMAFILES,user->name);
	if (!(fp=fopen(filename,"r"))) {
		return;
	}
	fscanf(fp,"%s\n",tempusername);
	fscanf(fp,"%d",&user->dhrac);
	user->dama->hrac[user->dhrac-1]=user;
	user->dama->hrac[2-user->dhrac]=get_user(tempusername);
	fscanf(fp,"%d %d %d\n",&user->dama->hrac[2-user->dhrac]->dhrac,&user->dama->forcejump,&user->dama->natahu);
	fscanf(fp,"%d %d\n",&user->dama->monochrom[0],&user->dama->monochrom[1]);
	fscanf(fp,"%d %d %d %d\n",&user->dama->lastmove[0],&user->dama->lastmove[1],&user->dama->disptype[0],&user->dama->disptype[1]);
	for (y=0;y<8;++y)
		for (x=0;x<8;++x)
			if (x<7)
				fscanf(fp,"%d ",&user->dama->plocha[x][y]);
			else
				fscanf(fp,"%d\n",&user->dama->plocha[x][y]);
	fclose(fp);
}

char *dama_saved_opponent(UR_OBJECT user)
{
	FILE *fp;
	char filename[80];

	sprintf(filename,"%s/%s.sav",DAMAFILES,user->name);
	if (!(fp=fopen(filename,"r"))) {
		strcpy(texthb,"_none_");
		return texthb;
	}
	fscanf(fp,"%s\n",texthb);
	fclose(fp);
	return texthb;
}

int check_dama_jump(DAMA_OBJECT udama,int biely,int x,int y,int oldsmer)
{
	int count=0,smer,dx=0,dy=0,j,ok;

	for (smer=0;smer<4;++smer) {
		if ((abs(smer-oldsmer)==2) || (x>6 && smer<2) || (x<1 && smer>1))
			continue;
		if (((y>6) && (smer==0 || smer==3)) || ((y<1) && (smer==1 || smer==2)))
			continue;
		switch (smer) {
			case 0:
				dx=1;
				dy=1;
				break;
			case 1:
				dx=1;
				dy=-1;
				break;
			case 2:
				dx=-1;
				dy=-1;
				break;
			case 3:
				dx=-1;
				dy=1;
				break;
		}
		ok=0;
		for (j=1;ok<2;++j) {
			if ((x+j*dx)>7 || (x+j*dx)<0 || (y+j*dy)>7 || (y+j*dy)<0)
				break;
			if ((biely && udama->plocha[x+j*dx][y+j*dy]<0) || (!biely && udama->plocha[x+j*dx][y+j*dy]>0)) {
				ok++;
				continue;
			}
			if (udama->plocha[x+j*dx][y+j*dy]==0) {
				if (ok==1) {
					ok=40;
					break;
				}
				else
					continue;
			}
			ok=88;
			break;
		}
		if (ok==40)
			count++;
	}
	return count;
}

void dama(UR_OBJECT user)
{
	UR_OBJECT u;
	RM_OBJECT rm;
	unsigned int i,j;
	int biely,chr,dlx=0,dly=0,figurka[7],smer,ok,minx=0,miny=0,x=0,y=0,oldsmer=0;
	int tah[7][2];
	int vymaz[3][2];

	if (word_count==1) {
		write_user(user,"Pouzi: .dama <user>   vyzves niekoho na partiu damy\n");
		write_user(user,"       .dama stop     vzdas sa (prehral si)\n");
		write_user(user,"       .dama stav     (dama show) zobrazi aktualny stav hry\n");
		write_user(user,"       .dama typ      prepnes typ zobrazovania hracej plochy (su 2 typy)\n");
		write_user(user,"       .dama farby    prepnes vystup pre monochromaticky/farebny monitor\n");
		write_user(user,"       .dama remiza   navrhnes protihracovi remizu\n");
		write_user(user,"       .dama uloz     hra sa ulozi a skonci, da sa dohrat neskor\n");
		write_user(user,"       .dama pokracuj navrhnes dokoncenie ulozenej hry protihracovi\n");
		write_user(user,"       .dama <suradnice>  urobis tah, ak si na rade. (napr. dama 3c d)\n");
		strcpy(text,dama_saved_opponent(user));
		if (strcmp(text,"_none_")) {
			sprintf(texthb,"~FTMas ulozenu rozohratu hru s protihracom: ~OL%s\n",text);
			write_user(user,texthb);
		}
		return;
	}
	if (!strcmp(word[1],"stop")) {
		if (user->dhrac>0) {
			u=user->dama->hrac[2-user->dhrac];
			sprintf(text,"~FTVzdal%s si sa a ~OL%s~RS~FT je vitaz.\n",pohl(user,"","a"),u->name);
			write_user(user,text);
			sprintf(text,"~FT~OL%s~RS~FT sa vzdal%s, si vitaz.\n",user->name,pohl(user,"","a"));
			write_user(u,text);
			if (user->dama->tah>4) {
				add_point(u,DB_DAMA,1,0);
				add_point(user,DB_DAMA,-1,0);
			}
			sprintf(text,"~FT~OL%s~RS~FT v dame podlah%s ~OL%s~RS~FT.\n",user->name,pohl(user,"ol","la"),sklonuj(u,3));
			if (user->room!=u->room) {
				write_room_except(user->room,text,user);
				write_room_except(u->room,text,u);
			}
			else {
				rm=u->room;
				u->room=NULL;
				write_room_except(user->room,text,user);
				u->room=rm;
			}
			u->dama=NULL;
			destruct_dama(user);
			user->dhrac=0;
			u->dhrac=0;
			user->dama_opp=NULL;
			u->dama_opp=NULL;
			sprintf(text,"Dama: %s/%s stop\n",u->name,user->name);
			log_game(text);
			return;
		}
		else {
			write_user(user,"~FTVed nehras damu.\n");
			return;
		}
	}
	if (!strcmp(word[1],"stav") || !strcmp(word[1],"show")) {
		if (user->dhrac>0) {
			if (user->dama->disptype[user->dhrac-1]==1)
				dama_stav(user);
			else
				dama_stav2(user);
			return;
		}
		else {
			write_user(user,"~FTVed nehras damu.\n");
			return;
		}
	}
	if (!strcmp(word[1],"typ") || !strcmp(word[1],"type")) {
		if (user->dhrac>0) {
			if (user->dama->disptype[user->dhrac-1]==1)
				user->dama->disptype[user->dhrac-1]=2;
			else
				user->dama->disptype[user->dhrac-1]=1;
			if (user->dama->disptype[user->dhrac-1]==1)
				dama_stav(user);
			else
				dama_stav2(user);
			sprintf(text,"~FTNastavil%s si typ zobrazovania cislo: ~OL~FW%d~RS~FT.\n",pohl(user,"","a"),user->dama->disptype[user->dhrac-1]);
			write_user(user,text);
			return;
		}
		else {
			write_user(user,"~FTTyp zobrazenia hracej plochy si mozes nastavit len ked hras damu.\n");
			return;
		}
	}
	if (!strcmp(word[1],"colors") || !strcmp(word[1],"farby")) {
		if (user->dhrac>0) {
			if (user->dama->monochrom[user->dhrac-1]==0)
				user->dama->monochrom[user->dhrac-1]=1;
			else
				user->dama->monochrom[user->dhrac-1]=0;
			if (user->dama->disptype[user->dhrac-1]==1)
				dama_stav(user);
			else
				dama_stav2(user);
			if (user->dama->monochrom[user->dhrac-1]==1)
				sprintf(text,"~FTNastavil%s si vystup pre ciernobiely alebo monochromaticky monitor.\n",pohl(user,"","a"));
			else
				sprintf(text,"~FTNastavil%s si vystup pre farebny monitor.\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		else {
			write_user(user,"~FTTyp zobrazenia hracej plochy si mozes nastavit len ked hras damu.\n");
			return;
		}
	}
	if (!strcmp(word[1],"remiza") || !strcmp(word[1],"rem")) {
		if (user->dhrac>0) {
			if (user->dama->remiza[user->dhrac-1]==1) {
				write_user(user,"~FTRemizu mozes protihracovi navrhnut len raz.\n");
				return;
			}
			else {
				if (user->dama->remiza[2-user->dhrac]==0) {
					user->dama->remiza[user->dhrac-1]=1;
					sprintf(text,"~FTiNavrh%s si ~OL%s~RS~FT remizu.\n",pohl(user,"ol","la"),sklonuj(user->dama->hrac[2-user->dhrac],3));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT ti navrhuje remizu.\n",user->name);
					write_user(user->dama->hrac[2-user->dhrac],text);
				}
				else {
					u=user->dama->hrac[2-user->dhrac];
					sprintf(text,"~FTSuhlasil%s si s remizou, hra skoncila.\n",pohl(user,"","a"));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT suhlasil%s s remizou, hra skoncila.\n",user->name,pohl(user,"","a"));
					write_user(u,text);
					u->dama=NULL;
					destruct_dama(user);
					user->dhrac=0;
					u->dhrac=0;
					user->dama_opp=NULL;
					u->dama_opp=NULL;
				}
			}
			return;
		}
		else {
			write_user(user,"~FTVed nehras damu.\n");
			return;
		}
	}
	if (!strcmp(word[1],"save") || !strcmp(word[1],"uloz")) {
		if (user->dhrac>0) {
			sprintf(text,"~FT%s si rozohratu hru damy.\n",pohl(user,"Ulozil","Ulozila"));
			write_user(user,text);
			sprintf(text,"~FT~OL%s~RS~FT %s rozohratu hru damy.\n",user->name,pohl(user,"ulozil","ulozila"));
			write_user(user->dama->hrac[2-user->dhrac],text);
			dama_save(user);
			return;
		}
		else {
			write_user(user,"~FTVed nehras damu.\n");
			return;
		}
	}
	if (user->dhrac>0) {
		if (user->dhrac==1)
			biely=1;
		else
			biely=0;
		if (user->dama->natahu!=user->dhrac) {
			sprintf(text,"~FTNa tahu je teraz %s, nemontuj sa mu do toho.\n",user->dama->hrac[2-user->dhrac]->name);
			write_user(user,text);
			return;
		}
		if (strlen(word[1])==1) {
			if (word[1][0]>='a' && word[1][0]<='h') {
				ok=0;
				minx=word[1][0]-'a';
				for (j=0;j<8;++j)
					if ((biely && user->dama->plocha[7-minx][j]>0) || (!biely && user->dama->plocha[minx][j]<0)) {
						ok++;
						miny=j;
					}
				if (ok==1) {
					word[1][1]=word[1][0];
					if (!biely)
						word[1][0]='1'+7-miny;
					else
						word[1][0]='1'+miny;
					word[1][2]='\0';
				}
				if (ok==2) {
					write_user(user,"~FTV tom stlpci mas viac figuriek, musis zadat aj riadok.\n");
					return;
				}
			}
			else {
				if (word[1][0]>='1' && word[1][0]<='8') {
					ok=0;
					miny=word[1][0]-'1';
					for (j=0;j<8;++j)
						if ((biely && user->dama->plocha[j][miny]>0) || (!biely && user->dama->plocha[j][7-miny]<0)) {
							ok++;
							minx=j;
						}
					if (ok==1) {
						word[1][1]=word[1][0];
						if (biely)
							word[1][0]='a'+7-minx;
						else
							word[1][0]='a'+minx;
						word[1][2]='\0';
					}
				}
			}
		}
		for (i=1;i<word_count;++i) {
			word[i][0]=tolower(word[i][0]);
			word[i][1]=tolower(word[i][1]);
			if (strlen(word[i])==1) {
				if (i>1) {
					word[i][2]='\0';
					word[i][1]=word[i][0];
					word[i][0]=word[i-1][0]+1;
				}
			}
			if (word[i][0]>='a' && word[i][0]<='h') {
				chr=word[i][0];
				word[i][0]=word[i][1];
				word[i][1]=chr;
			}
			if (strlen(word[i])!=2 || word[i][0]<'1' || word[i][0]>'8' || word[i][1]<'a' || word[i][1]>'h') {
				sprintf(text,"~FT%s su neplatne suradnice.\n",word[i]);
				write_user(user,text);
				return;
			}
			if (biely) {
				tah[i-1][0]=7-(word[i][1]-'a');
				tah[i-1][1]=word[i][0]-'1';
			}
			else {
				tah[i-1][0]=word[i][1]-'a';
				tah[i-1][1]=7-(word[i][0]-'1');
			}
		}
		for (i=0;i<word_count-1;++i)
			figurka[i]=user->dama->plocha[tah[i][0]][tah[i][1]];
		if (figurka[0]==0) {
			write_user(user,"~FTTy tam vidis nejaku figurku ??\n");
			return;
		}
		if ((biely && figurka[0]<0) || (!biely && figurka[0]>0)) {
			sprintf(text,"~FTNa %s figurky radsej nesahaj.\n",sklonuj(user->dama->hrac[2-user->dhrac],2));
			write_user(user,text);
			return;
		}
		j=0;ok=0;
		if (abs(figurka[0])==1 && user->dama->forcejump==0 && word_count==2) {
			if (tah[0][0]>0 && user->dama->plocha[tah[0][0]-1][tah[0][1]-1+biely*2]==0) {
				ok++;
				j=-1;
			}
			if (tah[0][0]<7 && user->dama->plocha[tah[0][0]+1][tah[0][1]-1+biely*2]==0) {
				ok++;
				j=1;
			}
			switch(ok) {
				case 0:
					write_user(user,"~FTS tou figurkou asi nepohnes.\n");
					return;
					break;
				case 2:
					write_user(user,"~FTMusis upresnit tah, je tam viac moznosti.\n");
					return;
					break;
			}
			if (ok==1) {
				tah[1][0]=tah[0][0]+j;
				tah[1][1]=tah[0][1]+biely*abs(j*2)-abs(j);
				word_count=3;
				figurka[1]=0;
			}
		}
		if (user->dama->forcejump>0) {
			ok=1;
			for (i=word_count-2;ok==1 && i<7;++i) {
				if (i>0) {
					dlx=tah[i][0]-tah[i-1][0];
					dly=tah[i][1]-tah[i-1][1];
					if (dlx>0 && dly>0)
						oldsmer=0;
					if (dlx>0 && dly<0)
						oldsmer=1;
					if (dlx<0 && dly<0)
						oldsmer=2;
					if (dlx<0 && dly>0)
						oldsmer=3;
				}
				else
					oldsmer=10;
				ok=0;
				if (
						user->dama->plocha[tah[0][0]][tah[0][1]]==2*biely-1
						&&
						(
						 (
						  biely
						  &&
						  tah[i][1]<6
						 )
						 ||
						 (
						  !biely
						  &&
						  tah[i][1]>1
						 )
						)
				   ) {
					if (
							(
							 tah[i][0]>1
							 &&
							 user->dama->plocha[tah[i][0]-2][tah[i][1]-2+biely*4]==0
							)
							&&
							(
							 (
							  biely
							  &&
							  user->dama->plocha[tah[i][0]-1][tah[i][1]+biely*2-1]<0
							 )
							 ||
							 (
							  !biely
							  &&
							  user->dama->plocha[tah[i][0]-1][tah[i][1]+biely*2-1]>0
							 )
							)
					   ) {
						ok++;
						j=-2;
					}
					if (
							(
							 tah[i][0]<6
							 &&
							 user->dama->plocha[tah[i][0]+2][tah[i][1]-2+biely*4]==0
							)
							&&
							(
							 (
							  biely
							  &&
							  user->dama->plocha[tah[i][0]+1][tah[i][1]+biely*2-1]<0
							 )
							 ||
							 (
							  !biely
							  &&
							  user->dama->plocha[tah[i][0]+1][tah[i][1]+biely*2-1]>0
							 )
							)
					   ) {
						ok++;
						j=2;
					}
					if (ok==1) {
						tah[i+1][0]=tah[i][0]+j;
						tah[i+1][1]=tah[i][1]+biely*abs(j*2)-abs(j);
						word_count=i+3;
						figurka[i+1]=0;
					}
				}
				if (user->dama->plocha[tah[0][0]][tah[0][1]]==4*biely-2) {
					chr=0;
					for (smer=0;smer<4;++smer) {
						if ((abs(smer-oldsmer)==2) || (tah[i][0]>6 && smer<2) || (tah[i][0]<1 && smer>1))
							continue;
						if (((tah[i][1]>6) && (smer==0 || smer==3)) || ((tah[i][1]<1) && (smer==1 || smer==2)))
							continue;
						switch (smer) {
							case 0:
								dlx=1;
								dly=1;
								break;
							case 1:
								dlx=1;
								dly=-1;
								break;
							case 2:
								dlx=-1;
								dly=-1;
								break;
							case 3:
								dlx=-1;
								dly=1;
								break;
						}
						miny=-1;
						j=0;
						for (chr=1;ok<2;++chr) {
							if (
									tah[i][0]+chr*dlx>7
									||
									tah[i][0]+chr*dlx<0
									||
									tah[i][1]+chr*dly>7
									||
									tah[i][1]+chr*dly<0
							   )
								break;
							if (user->dama->plocha[tah[i][0]+chr*dlx][tah[i][1]+chr*dly]==0 && j==1) {
								minx=check_dama_jump(user->dama,biely,tah[i][0]+chr*dlx,tah[i][1]+chr*dly,smer);
								if (minx>=miny) {
									x=tah[i][0]+chr*dlx;
									y=tah[i][1]+chr*dly;
									if (miny!=0)
										ok++;
									miny=minx;
								}
								continue;
							}
							if (user->dama->plocha[tah[i][0]+chr*dlx][tah[i][1]+chr*dly]==0)
								continue;
							if (
									(
									 biely
									 &&
									 user->dama->plocha[tah[i][0]+chr*dlx][tah[i][1]+chr*dly]<0
									)
									||
									(
									 !biely
									 &&
									 user->dama->plocha[tah[i][0]+chr*dlx][tah[i][1]+chr*dly]>0
									)
							   ) {
								j++;
							}
							else
								break;
						}
					}
					if (ok==1) {
						tah[i+1][0]=x;
						tah[i+1][1]=y;
						word_count=i+3;
						figurka[i+1]=0;
					}
				}
			}
			if (ok==0 && i<1) {
				write_user(user,"~FTS tou figurkou nemozes skakat.\n");
				return;
			}
			if (ok==2) {
				write_user(user,"~FTMusis upresnit skok, je tam viac moznosti.\n");
				return;
			}
		}
		if (word_count<3) {
			write_user(user,"~FTZadaj dvoje suradnice.\n");
			return;
		}
		for (i=0;i<word_count-2;++i) {
			dlx=tah[i][0]-tah[i+1][0];
			dly=tah[i][1]-tah[i+1][1];
			if (tah[i][0]<tah[i+1][0])
				minx=1;
			else
				minx=-1;
			if (tah[i][1]<tah[i+1][1])
				miny=1;
			else
				miny=-1;
			if (dlx==0 && dly==0) {
				write_user(user,"~FTSuradnice nemozu byt rovnake.\n");
				return;
			}
			if (abs(dlx)!=abs(dly)) {
				write_user(user,"~FTSkus tahat po diagonalach.\n");
				return;
			}
			if (abs(dlx)>2 && abs(figurka[0])==1) {
				write_user(user,"~FTDlhe skoky mozes robit iba damou.\n");
				return;
			}
			if (abs(figurka[0])==1 && ((biely && tah[i][1]>tah[i+1][1]) || (!biely && tah[i][1]<tah[i+1][1]))) {
				write_user(user,"~FTVracat sa mozes iba s damou.\n");
				return;
			}
			if (figurka[i+1]!=0) {
				write_user(user,"~FTZavadzia ti tam ina figurka.\n");
				return;
			}
			if (abs(dlx)==1)
				break;
			if (abs(dlx)>1) {
				ok=0;
				for (j=1;j<(unsigned int)abs(dlx);++j) {
					if (
							(
							 biely
							 &&
							 user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]<0
							)
							||
							(
							 !biely
							 &&
							 user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]>0
							)
					   ) {
						ok++;
						vymaz[i][0]=tah[i][0]+j*minx;
						vymaz[i][1]=tah[i][1]+j*miny;
						continue;
					}
					else
						if (user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]!=0) {
							write_user(user,"~FTNemozes skakat cez svoje figurky.\n");
							return;
						}
				}
				if (ok==0 && (abs(figurka[0])==1 || (abs(figurka[0])==2 && user->dama->forcejump>0))) {
					write_user(user,"~FTMusis preskocit protihracovi aspon jednu figurku.\n");
					return;
				}
				if (ok>1) {
					write_user(user,"~FTNemozes mu preskocit viac naraz.\n");
					return;
				}
			}
			if (word_count>2 && i==word_count-3 && figurka[0]==4*biely-2) {
				if (minx>0 && miny>0) oldsmer=0;
				if (minx>0 && miny<0) oldsmer=1;
				if (minx<0 && miny<0) oldsmer=2;
				if (minx<0 && miny>0) oldsmer=3;
				ok=0;
				chr=check_dama_jump(user->dama,biely,tah[i+1][0],tah[i+1][1],oldsmer);
				for (j=1;j<8 && ok<2;++j) {
					if (tah[i][0]+j*minx>7 || tah[i][1]+j*miny>7)
						break;
					if (ok==1 && user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]!=0)
						break;
					if (ok==1 && check_dama_jump(user->dama,biely,tah[i][0]+j*minx,tah[i][1]+j*miny,oldsmer)>chr) {
						write_user(user,"~FTMusis spravit dlhsi skok ako si zadal.\n");
						return;
					}
					if (
							(
							 biely
							 &&
							 user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]<0
							)
							||
							(
							 !biely
							 &&
							 user->dama->plocha[tah[i][0]+j*minx][tah[i][1]+j*miny]>0
							)
					   )
						ok++;
				}
			}
		}
		if (user->dama->forcejump && (abs(dlx)<2 || ok!=1)) {
			write_user(user,"~FTMusis skakat, take su pravidla.\n");
			return;
		}
		if (user->dama->forcejump>0) {
			for (i=0;i<word_count-2;++i)
				for (j=i+1;j<word_count-2;++j)
					if (vymaz[i][0]==vymaz[j][0] && vymaz[i][1]==vymaz[j][1]) {
						write_user(user,"~FTNemozes skakat tu istu figurku dva krat.\n");
						return;
					}
			for (i=0;i<word_count-2;++i)
				user->dama->plocha[vymaz[i][0]][vymaz[i][1]]=0;
		}
		user->dama->plocha[tah[word_count-2][0]][tah[word_count-2][1]]=figurka[0];
		user->dama->plocha[tah[0][0]][tah[0][1]]=0;
		if (tah[word_count-2][1]==biely*7 && abs(figurka[0])==1)
			user->dama->plocha[tah[word_count-2][0]][tah[word_count-2][1]]=biely*4-2;
		user->dama->lastmove[0]=tah[0][0];
		user->dama->lastmove[1]=tah[0][1];
		if (user->dama->natahu==1)
			user->dama->natahu=2;
		else
			user->dama->natahu=1;
		user->dama->forcejump=0;
		biely=1-biely;
		for (y=0;y<8;++y) {
			for (x=0;x<8;++x) {
				if (user->dama->plocha[x][y]==2*biely-1)
					if ((!biely && y>1) || (biely && y<6)) {
						if (
								x>1
								&&
								user->dama->plocha[x-2][y-2+biely*4]==0
								&&
								(
								 (
								  biely
								  &&
								  user->dama->plocha[x-1][y-1+biely*2]<0
								 )
								 ||
								 (
								  !biely
								  &&
								  user->dama->plocha[x-1][y-1+biely*2]>0
								 )
								)
						   )
							user->dama->forcejump++;
						if (
								x<6
								&&
								user->dama->plocha[x+2][y-2+biely*4]==0
								&&
								(
								 (
								  biely
								  &&
								  user->dama->plocha[x+1][y-1+biely*2]<0
								 )
								 ||
								 (
								  !biely
								  &&
								  user->dama->plocha[x+1][y-1+biely*2]>0
								 )
								)
						   )
							user->dama->forcejump++;
					}
				if (user->dama->plocha[x][y]==4*biely-2) {
					user->dama->forcejump+=check_dama_jump(user->dama,biely,x,y,10);
				}
			}
		}
		if (user->dama->disptype[user->dhrac-1]==1)
			dama_stav(user);
		else
			dama_stav2(user);
		write_user(user->dama->hrac[2-user->dhrac],"\n");
		if (user->dama->disptype[2-user->dhrac]==1)
			dama_stav(user->dama->hrac[2-user->dhrac]);
		else
			dama_stav2(user->dama->hrac[2-user->dhrac]);
		user->dama->tah++;
		user->dama->hrac[2-user->dhrac]->dama->tah++;
		if (user->dama->natahu>2) {
			u=user->dama->hrac[user->dama->natahu-3];
			sprintf(text,"~FTPorazil%s si ~OL%s~RS~FT v dame. ~OLCongratz~RS~FT.\n",pohl(user,"","a"),sklonuj(u,2));
			write_user(user,text);
			sprintf(text,"~FTPodlah%s si ~OL%s~RS~FT v dame. Hra skoncila.\n",pohl(u,"ol","la"),sklonuj(user,3));
			write_user(u,text);
			add_point(user,DB_DAMA,1,0);
			add_point(u,DB_DAMA,-1,0);
			sprintf(text,"Dama: %s/%s\n",user->name,u->name);
			log_game(text);
			sprintf(text,"~FT~OL%s~RS~FT podlah%s ~OL%s~RS~FT v dame.\n",u->name,pohl(u,"ol","la"),sklonuj(user,3));
			if (user->room!=u->room) {
				write_room_except(user->room,text,user);
				write_room_except(u->room,text,u);
			}
			else {
				rm=u->room;
				u->room=NULL;
				write_room_except(user->room,text,user);
				u->room=rm;
			}
			u->dama=NULL;
			destruct_dama(user);
			user->dhrac=0;
			u->dhrac=0;
			u->dama_opp=NULL;
			return;
		}
		return;
	}
	if (!strcmp(word[1],"load") || !strcmp(word[1],"pokracuj")) {
		strcpy(word[1],dama_saved_opponent(user));
		if (strcmp(word[1],"_none_")) {
			sprintf(text,"~FTMas ulozenu rozohratu hru, protihrac: ~OL%s\n",word[1]);
			write_user(user,text);
		}
		else {
			write_user(user,"~FTNemas ulozenu ziadnu rozohratu hru.\n");
			return;
		}
		user->dhrac=-1;
	}
	if (!(u=get_user(word[1]))) {
		write_user(user,notloggedon);
		return;
	}
	if (user==u) {
		write_user(user,"~FTHmm, myslis ze by ta to bavilo ?\n");
		return;
	}
	if (u->dhrac>0) {
		sprintf(text,"~FT~OL%s~RS~FT uz s niekym hra.\n",u->name);
		write_user(user,text);
		return;
	}
	if (user->dama_opp==u) {
		sprintf(text,"~FTUz si %s navrhoval%s damu.\n",sklonuj(u,3),pohl(user,"","a"));
		write_user(user,text);
		return;
	}
	if (u->igngames) {
		sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",u->name);
		write_user(user,text);
		return;
	}
	if (user!=u->dama_opp) {
		sprintf(text,"~FTUkazujes ~OL%s~RS~FT sachovnicu a figurky, ci si nezahra damu.\n",sklonuj(u,3));
		write_user(user,text);
		if (user->dhrac==-1)
			sprintf(text,"\n~FT~OL%s~RS~FT ti ukazuje sachovnicu s rozohratou damou.. dohrame to?\n",user->name);
		else
			sprintf(text,"\n~FT~OL%s~RS~FT ti ukazuje sachovnicu a figurky, nezahras si damu?\n",user->name);
		write_user(u,text);
		user->dama_opp=u;
		return;
	}
	else {
		if ((user->dama=create_dama())==NULL) {
			write_user(user,"~FTBohuzial niekde sa stratila sachovnica, asi si nezahrate.\n");
			write_user(u,"~FTBohuzial niekde sa stratila sachovnica, asi si nezahrate.\n");
			return;
		}
		if (u->dhrac==-1) {
			if (get_user(dama_saved_opponent(u))==user) {
				u->dama=user->dama;
				dama_load(u);
			}
			if (user->dama->disptype[user->dhrac-1]==1)
				dama_stav(user);
			else
				dama_stav2(user);
			if (user->dama->disptype[2-user->dhrac]==1)
				dama_stav(u);
			else
				dama_stav2(u);
			write_user(user,"~FTHra pokracuje!\n");
			write_user(u,"~FTHra pokracuje!\n");
			return;
		}
		else {
			sprintf(text,"~FTChopil%s si sa bielych figuriek, zacinas. ~OL%s~RS~FT ma cierne.\n",pohl(user,"","a"),u->name);
			write_user(user,text);
			sprintf(text,"~FT~OL%s~RS~FT sa chopil%s bielych figuriek, zacina. Tebe ostali cierne.\n",user->name,pohl(user,"","a"));
			write_user(u,text);
			user->dama_opp=u;
			user->dhrac=1;
			u->dhrac=2;
			u->dama=user->dama;
			user->dama->hrac[0]=user;
			user->dama->hrac[1]=u;
			u->dama->tah=0; user->dama->tah=0;
			for (i=0;i<4;++i) {
				user->dama->plocha[i*2+1][0]=1;
				user->dama->plocha[i*2][1]=1;
				user->dama->plocha[i*2+1][2]=1;
				user->dama->plocha[i*2][5]=-1;
				user->dama->plocha[i*2+1][6]=-1;
				user->dama->plocha[i*2][7]=-1;
			}
			user->dama->natahu=1;
			if (user->dama->disptype[user->dhrac-1]==1)
				dama_stav(user);
			else
				dama_stav2(user);
			if (user->dama->disptype[2-user->dhrac]==1)
				dama_stav(u);
			else
				dama_stav2(u);
			return;
		}
	}
}

void dama_stav(UR_OBJECT user)
{
	int x,y,reverse,biely_count,cierny_count,figurkax,figurkay;
	char dispf[5][20];

	if (user->colour && user->dama->monochrom[user->dhrac-1]==0) {
		strcpy(dispf[0],"| ~OL~FKQ~RS ");
		strcpy(dispf[1],"| ~OL~FK0~RS ");
	}
	else {
		strcpy(dispf[0],"| Y ");
		strcpy(dispf[1],"| X ");
	}
	strcpy(dispf[2],"|   ");
	strcpy(dispf[3],"| ~OLO~RS ");
	strcpy(dispf[4],"| ~OLQ~RS ");
	biely_count=0;
	cierny_count=0;
	if (user->dama==NULL)
		return;
	if (user->dhrac==2)
		reverse=1;
	else
		reverse=-1;
	sprintf(text,"Dama: protihrac ~OL%s",user->dama->hrac[2-user->dhrac]->name);
	write_user(user,header(text));
	strcpy(texthb,"                          ~FTA   B   C   D   E   F   G   H~FW  \n");
	if (user->dama->natahu==user->dhrac) {
		if (user->dama->forcejump>0)
			sprintf(text,"~OL~FTMusis skakat:~RS~FW           ");
		else
			sprintf(text,"~OL~FTSi na tahu:~RS~FW             ");
	}
	else
		sprintf(text,"~FTNa tahu je ~OL~FT%-12s~RS~FW ",user->dama->hrac[2-user->dhrac]->name);
	strcat(texthb,text);
	strcat(texthb,".-------------------------------.\n                      ~FT8~FW ");
	write_user(user,texthb);
	texthb[0]='\0';
	for (y=0;y<8;++y) {
		for (x=0;x<8;++x) {
			if (x==0 && y>0)
				sprintf(texthb,"                        |---|---|---|---|---|---|---|---|\n                      ~FT%d~FW ",8-y);
			else
				texthb[0]='\0';
			figurkax=(int)(7*((-reverse+1)/2)+x*reverse);
			figurkay=(int)(7*((-reverse+1)/2)+y*reverse);
			if (user->dama->lastmove[0]==figurkax && user->dama->lastmove[1]==figurkay)
				strcat(texthb,"| ~FK~OL.~RS~FW ");
			else
				strcat(texthb,dispf[user->dama->plocha[figurkax][figurkay]+2]);
			if (user->dama->plocha[figurkax][figurkay]<0)
				cierny_count++;
			if (user->dama->plocha[figurkax][figurkay]>0)
				biely_count++;
			if (x==7) {
				sprintf(text,"| ~FT%d~FW\n",8-y);
				strcat(texthb,text);
			}
			write_user(user,texthb);
		}
	}
	if (reverse==1)
		sprintf(text,"Straty:                 `-------------------------------'\n%s(~OL~FK0~RS~FW): %-*d       ~FTA   B   C   D   E   F   G   H~FW      %12s(~OLO~RS): %-2d\n",user->name,(int)(14-strlen(user->name)),12-cierny_count,user->dama->hrac[2-user->dhrac]->name,12-biely_count);
	else
		sprintf(text,"Straty:                 `-------------------------------'\n%s(~OLO~RS): %-*d       ~FTA   B   C   D   E   F   G   H~FW      %12s(~OL~FK0~RS~FW): %-2d\n",user->name,(int)(14-strlen(user->name)),12-biely_count,user->dama->hrac[2-user->dhrac]->name,12-cierny_count);
	write_user(user,text);
	oline(user);
	if (biely_count==0)
		user->dama->natahu=3;
	if (cierny_count==0)
		user->dama->natahu=4;
}

void dama_stav2(UR_OBJECT user)
{
	int x,y,reverse,biely_count,cierny_count,figurkax,figurkay;
	char dispf[5][20];

	if (user->colour && user->dama->monochrom[user->dhrac-1]==0) {
		strcpy(dispf[0],"~OL~FKQ~RS");
		strcpy(dispf[1],"~OL~FK0~RS");
	}
	else {
		strcpy(dispf[0],"Y");
		strcpy(dispf[1],"X");
	}
	strcpy(dispf[2],"~OL~FK.~RS");
	strcpy(dispf[3],"~OLO~RS");
	strcpy(dispf[4],"~OLQ~RS");
	biely_count=0;
	cierny_count=0;
	if (user->dama==NULL)
		return;
	if (user->dhrac==2)
		reverse=1;
	else
		reverse=-1;
	sprintf(text,"Dama: protihrac ~OL%s",user->dama->hrac[2-user->dhrac]->name);
	write_user(user,header(text));
	strcpy(texthb,"                                 ~FTA B C D E F G H~FW  \n");
	if (user->dama->natahu==user->dhrac) {
		if (user->dama->forcejump>0)
			sprintf(text,"~OL~FTMusis skakat:~RS~FW                   ");
		else
			sprintf(text,"~OL~FTSi na tahu:~RS~FW                     ");
	}
	else
		sprintf(text,"~FTNa tahu je ~OL~FT%-12s~RS~FW         ",user->dama->hrac[2-user->dhrac]->name);
	strcat(texthb,text);
	strcat(texthb,".---------------.\n");
	write_user(user,texthb);
	texthb[0]='\0';
	for (y=0;y<8;++y) {
		for (x=0;x<8;++x) {
			if (x==0) {
				sprintf(text,"                              ~FT%d~FW |",8-y);
				strcat(texthb,text);
			}
			figurkax=(int)(7*((-reverse+1)/2)+x*reverse);
			figurkay=(int)(7*((-reverse+1)/2)+y*reverse);
			if (user->dama->lastmove[0]==figurkax && user->dama->lastmove[1]==figurkay) {
				if (user->colour)
					strcat(texthb,"~FG.~FW"); else strcat(texthb,",");
			}
			else
				strcat(texthb,dispf[user->dama->plocha[figurkax][figurkay]+2]);
			if (user->dama->plocha[figurkax][figurkay]<0)
				cierny_count++;
			if (user->dama->plocha[figurkax][figurkay]>0)
				biely_count++;
			if (x==7) {
				sprintf(text,"| ~FT%d~FW\n",8-y);
				strcat(texthb,text);
			}
			else
				strcat(texthb," ");
		}
	}
	write_user(user,texthb);
	if (reverse==1)
		sprintf(text,"Straty:                         `---------------'\n%s(~OL~FK0~RS~FW): %-*d              ~FTA B C D E F G H~FW             %12s(~OLO~RS): %-2d\n",user->name,(int)(14-strlen(user->name)),12-cierny_count,user->dama->hrac[2-user->dhrac]->name,12-biely_count);
	else
		sprintf(text,"Straty:                         `---------------'\n%s(~OLO~RS): %-*d              ~FTA B C D E F G H~FW             %12s(~OL~FK0~RS~FW): %-2d\n",user->name,(int)(14-strlen(user->name)),12-biely_count,user->dama->hrac[2-user->dhrac]->name,12-cierny_count);
	write_user(user,text);
	oline(user);
	if (biely_count==0)
		user->dama->natahu=3;
	if (cierny_count==0)
		user->dama->natahu=4;
}

char *header(char *str)
{
	char text2[ARR_SIZE*2];
	int i,len,spc,odd;
	strcpy(texthb,str);
	colour_com_strip(texthb);
	len=strlen(texthb);
	spc=(int)((33)-(len/2));
	odd=((spc+spc+len)-66);
	sprintf(text2,"%*.*s~OL~FB=[~OL~FY%s",spc,spc," ",texthb);
	for (i=0;i<33 && text2[i]==' ';++i)
		text2[i]='-';
	sprintf(texthb,"%s~OL~FB]=~RS~FB%*.*s",text2,spc-odd,spc-odd," ");
	for (i=strlen(texthb)-1;texthb[i]==' ';--i)
		texthb[i]='-';
	sprintf(text2,"~FB-----%s=|=--\n",texthb);
	strcpy(texthb,text2);
	return texthb;
}

void oline(UR_OBJECT u)
{
	write_user(u,"~FB----------------------------------------------------------------------------=---~FW\n");
}

UR_OBJECT hra(UR_OBJECT user,int n)
{
	UR_OBJECT u;

	if (n==1) {
		if (user->game!=2)
			return NULL;
		else if ((u=get_user(user->uname)))
			return u;
		return NULL;
	}
	if (n==2) {
		if (user->game!=3)
			return NULL;
		else if ((u=get_user(user->uname)))
			return u;
		return NULL;
	}
	if (n==3) {
		if (user->hang_stage==-1)
			return NULL;
		else
			return user;
	}
	if (n==4) {
		if (!user->doom_energy)
			return NULL;
		else
			return user;
	}
	if (n==5) {
		if (!user->reversi_cislotahu)
			return NULL;
		else if (user->reversi_sh!=NULL)
			return user->reversi_sh;
		return NULL;
	}
	if (n==6) {
		if (user->dhrac<=0)
			return NULL;
		else if (user->dama->hrac[2-user->dhrac]!=NULL)
			return user->dama->hrac[2-user->dhrac];
		return NULL;
	}
	if (n==7) {
		if (user->miny_tab==NULL)
			return NULL;
		else
			return user;
	}
	if (n==8) {
		if (user->game!=4)
			return NULL;
		else if ((u=get_user(user->uname)))
			return u;
		return NULL;
	}
	if (n==9) {
		if (!user->lab)
			return NULL;
		else
			return user;
	}
	return NULL;
}

void games(UR_OBJECT user)
{
	UR_OBJECT u;
	UR_OBJECT u2;

	write_user(user,"~FB[~OL~FYPiskvorky~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,1);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s vs. %s\n",u->name,u2->name);
	}
	write_user(user,"~FB[~OL~FYGEO~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,2);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s vs. %s\n",u->name,u2->name);
	}
	write_user(user,"~FB[~OL~FYHangman~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,3);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s\n",u->name);
	}
	write_user(user,"~FB[~OL~FYDOOM~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,4);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s\n",u->name);
	}
	write_user(user,"~FB[~OL~FYReversi~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,5);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s vs. %s\n",u->name,u2->name);
	}
	write_user(user,"~FB[~OL~FYDama~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,6);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s vs. %s\n",u->name,u2->name);
	}
	write_user(user,"~FB[~OL~FYMiny~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,7);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s\n",u->name);
	}
	write_user(user,"~FB[~OL~FYLodicky~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,8);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s vs. %s\n",u->name,u2->name);
	}
	write_user(user,"~FB[~OL~FYLabyrint~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login || u->room==NULL)
			continue;
		u2=hra(u,9);
		if (u2!=NULL)
			vwrite_user(user,"~RS~FG-> ~FW%s\n",u->name);
	}
	write_user(user,"~FB[~OL~FYClovece~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->chrac>0) {
			vwrite_user(user,"~RS~FG-> ~FW%s ~FG(hraju:~OL~FW",u->name);
			if (u->clovece->hrac[0]!=NULL)
				vwrite_user(user," %s",u->clovece->hrac[0]->name);
			if (u->clovece->hrac[1]!=NULL)
				vwrite_user(user," %s",u->clovece->hrac[1]->name);
			if (u->clovece->hrac[2]!=NULL)
				vwrite_user(user," %s",u->clovece->hrac[2]->name);
			if (u->clovece->hrac[3]!=NULL)
				vwrite_user(user," %s",u->clovece->hrac[3]->name);
			write_user(user,"~RS~FG)\n");
		}
	}
	write_user(user,"~FB[~OL~FYFaraon~RS~FB]\n");
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->fhrac>-1) {
			vwrite_user(user,"~RS~FG-> ~FW%s ~FG(hraju:~OL~FW",u->name);
			if (u->farar->hrac[0]!=NULL)
				vwrite_user(user," %s",u->farar->hrac[0]->name);
			if (u->farar->hrac[1]!=NULL)
				vwrite_user(user," %s",u->farar->hrac[1]->name);
			if (u->farar->hrac[2]!=NULL)
				vwrite_user(user," %s",u->farar->hrac[2]->name);
			if (u->farar->hrac[3]!=NULL)
				vwrite_user(user," %s",u->farar->hrac[3]->name);
			if (u->farar->hrac[4]!=NULL)
				vwrite_user(user," %s",u->farar->hrac[4]->name);
			write_user(user,"~RS~FG)\n");
		}
	}
}

void clovece_disp(UR_OBJECT user)
{
	int x,y,cp,i,j,cnt;
	char kockad[3][6];
	switch (user->clovece->kocka) {
		case 1:
			strcpy(kockad[0],"     ");
			strcpy(kockad[1],"  o  ");
			strcpy(kockad[2],"     ");
			break;
		case 2:
			strcpy(kockad[0],"o    ");
			strcpy(kockad[1],"     ");
			strcpy(kockad[2],"    o");
			break;
		case 3:
			strcpy(kockad[0],"    o");
			strcpy(kockad[1],"  o  ");
			strcpy(kockad[2],"o    ");
			break;
		case 4:
			strcpy(kockad[0],"o   o");
			strcpy(kockad[1],"     ");
			strcpy(kockad[2],"o   o");
			break;
		case 5:
			strcpy(kockad[0],"o   o");
			strcpy(kockad[1],"  o  ");
			strcpy(kockad[2],"o   o");
			break;
		case 6:
			strcpy(kockad[0],"o   o");
			strcpy(kockad[1],"o   o");
			strcpy(kockad[2],"o   o");
			break;
		default:
			strcpy(kockad[0],"     ");
			strcpy(kockad[1],"     ");
			strcpy(kockad[2],"     ");
			break;
	}
	write_user(user,"\n");
	write_user(user,header("Clovece nehnevaj sa !"));
	if (user->clovece->hrac[2]!=NULL)
		sprintf(text,"              ~OL~FY%12s",user->clovece->hrac[2]->name);
	else
		sprintf(text,"                          ");
	write_user(user,text);
	write_user(user,"  .-----------------------.");
	if (user->clovece->hrac[3]!=NULL)
		sprintf(text,"  ~OL~FR%-12s\n",user->clovece->hrac[3]->name);
	else
		sprintf(text,"\n");
	write_user(user,text);
	for (y=0;y<11;++y) {
		if (user->clovece->natahu>0)
			switch (y) {

				case 1:
					if (user->clovece->loadlock>0)
						sprintf(texthb,"~FTEste chyba ~OL%d~RS~FT ks hracov.~FW",user->clovece->loadlock);
					else {
						if (user->clovece->natahu==user->chrac)
							sprintf(texthb," ~OL%sHodil%s si~RS~FT kockou:~FW",clovece_col[user->clovece->natahu-1],pohl(user->clovece->hrac[user->clovece->natahu-1],"","a"));
						else
							sprintf(texthb,"~OL%s %s ~RS~FThodil%s:~FW",clovece_col[user->clovece->natahu-1],user->clovece->hrac[user->clovece->natahu-1]->name,pohl(user->clovece->hrac[user->clovece->natahu-1],"","a"));
					}
					sprintf(text," %-40s  ",texthb);
					break;
				case 3:
					strcpy(text,"~FT        ______              ~FW");
					break;
				case 4:
					strcpy(text,"~FT       /     /|             ~FW");
					break;
				case 5:
					strcpy(text,"~FT      '-----' |             ~FW");
					break;
				case 6:
					sprintf(text,"~FT      |~OL%5s~RS~FT| |             ~FW",kockad[0]);
					break;
				case 7:
					sprintf(text,"~FT      |~OL%5s~RS~FT| |             ~FW",kockad[1]);
					break;
				case 8:
					sprintf(text,"~FT      |~OL%5s~RS~FT|/              ~FW",kockad[2]);
					break;
				case 9:
					strcpy(text,"~FT      `-----'               ~FW");
					break;
				default:
					strcpy(text,"                            ");
			}
		else
			strcpy(text,"                            ");
		for (x=0;x<13;++x) {
			cp=clovece_plocha[x+y*13];
			cnt=0;
			for (i=0;i<4;++i)
				for (j=0;j<4;++j)
					if (user->clovece->figurka[i][j]==cp) {
						strcat(text,clovece_col[i]);
						if (user->chrac-1==i) {
							sprintf(texthb,"~OL%d ~RS~FW",j+1);
							strcat(text,texthb);
						}
						else {
							if ((user->colour==0 || user->clovece->monochrom[user->chrac-1]==1)) {
								strcat(text,"~OL  ");
								switch (i) {
									case 0:
										text[strlen(text)-2]='0';
										break;
									case 1:
										text[strlen(text)-2]='B';
										break;
									case 2:
										text[strlen(text)-2]='8';
										break;
									case 3:
										text[strlen(text)-2]='D';
										break;
								}
							}
							else
								strcat(text,"~OL8 ~RS~FW");
						}
						cnt++;
					}
			if (cnt==0) {
				if (cp>50) {
					strcat(text,clovece_col[(clovece_plocha[x+y*13]-50)/10]);
					if (
							clovece_plocha[x+y*13]-(((clovece_plocha[x+y*13]-50)/10)*10)>4
							&&
							(
							 user->colour==0
							 ||
							 user->clovece->monochrom[user->chrac-1]==1
							)
					   )
						strcat(text,"~OL, ");
					else
						strcat(text,"~OL. ");
				}
				else
					switch (cp) {
						case 44:
							strcat(text,"  ");
							break;
						case  1:
							strcat(text,"~FT~OL. ~RS~FW");
							break;
						case 11:
							strcat(text,"~FG~OL. ~RS~FW");
							break;
						case 21:
							strcat(text,"~FY~OL. ~RS~FW");
							break;
						case 31:
							strcat(text,"~FR~OL. ~RS~FW");
							break;
						case 49:
							strcat(text,"| ");
							break;
						default:
							strcat(text,". ");
							break;
					}
				if (clovece_plocha[x+y*13]>50)
					strcat(text,"~RS~FW");
			}
		}
		strcat(text,"\n");
		write_user(user,text);
	}
	if (user->clovece->hrac[1]!=NULL)
		sprintf(text,"              ~OL~FG%12s",user->clovece->hrac[1]->name);
	else
		sprintf(text,"                          ");
	write_user(user,text);
	write_user(user,"  `-----------------------'");
	if (user->clovece->hrac[0]!=NULL)
		sprintf(text,"  ~OL~FT%-12s\n",user->clovece->hrac[0]->name);
	else
		sprintf(text,"\n");
	write_user(user,text);
	oline(user);
}

CL_OBJECT create_clovece()
{
	CL_OBJECT clovece;
	int i,ii;

	if ((clovece=(CL_OBJECT)malloc(sizeof(struct cl_struct)))==NULL) {
		write_syslog("HUH: Memory allocation failure in create_clovece().\n",0);
		return NULL;
	}
	for (i=0;i<4;++i)
		for (ii=0;ii<4;++ii)
			clovece->figurka[ii][i]=0;
	for (i=0;i<4;++i)
		clovece->hrac[i]=NULL;
	for (i=0;i<4;++i)
		clovece->loadhrac[i]=NULL;
	for (i=0;i<4;++i)
		clovece->monochrom[i]=0;
	for (i=0;i<4;++i)
		clovece->usertahs[i]=0;
	for (i=0;i<4;++i)
		clovece->remiza[i]=0;
	clovece->natahu=0;
	clovece->tahov=0;
	clovece->locked=0;
	clovece->kocka=0;
	clovece->loadlock=0;
	clovece->revline=0;
	for (i=0;i<REVIEW_LINES;++i)
		clovece->revbuff[i][0]='\0';
	return clovece;
}

void record_clovece(CL_OBJECT clovece)
{
	sstrncpy(clovece->revbuff[clovece->revline],text,REVIEW_LEN);
	clovece->revbuff[clovece->revline][REVIEW_LEN]='\n';
	clovece->revbuff[clovece->revline][REVIEW_LEN+1]='\0';
	clovece->revline=(clovece->revline+1)%REVIEW_LINES;
}

void destruct_clovece(UR_OBJECT user)
{
	free(user->clovece);
}

void clovece_save(UR_OBJECT user)
{
	FILE *fp;
	char filename[80];
	int i;

	sprintf(filename,"%s/%s.sav",CLOVECEFILES,user->name);
	if (!(fp=fopen(filename,"w"))) {
		sprintf(text,"%s: problem so zapisovanim do suboru.\n",syserror);
		write_user(user,text);
		return;
	}
	for (i=0;i<4;++i)
		if (user->clovece->hrac[i]!=NULL)
			fprintf(fp,"%s\n",user->clovece->hrac[i]->name);
		else
			fprintf(fp,"N/A\n");
	for (i=0;i<4;++i)
		fprintf(fp,"%d %d %d %d\n",user->clovece->figurka[i][0],user->clovece->figurka[i][1],user->clovece->figurka[i][2],user->clovece->figurka[i][3]);
	fprintf(fp,"%d %d %d %d\n",user->clovece->monochrom[0],user->clovece->monochrom[1],user->clovece->monochrom[2],user->clovece->monochrom[3]);
	fprintf(fp,"%d %d %d %d\n",user->clovece->usertahs[0],user->clovece->usertahs[1],user->clovece->usertahs[2],user->clovece->usertahs[3]);
	fprintf(fp,"%d %d %d %d\n",user->clovece->natahu,user->clovece->tahov,user->clovece->locked,user->clovece->kocka);
	fclose(fp);
}

void clovece_load(UR_OBJECT user)
{
	FILE *fp;
	UR_OBJECT u;
	char filename[80],tempusername[USER_NAME_LEN+1];
	int cnt,i;

	sprintf(filename,"%s/%s.sav",CLOVECEFILES,user->name);
	if (!(fp=fopen(filename,"r"))) {
		write_user(user,"~FTNemas ulozenu hru clovece.\n");
		return;
	}
	if ((user->clovece=create_clovece())==NULL) {
		write_user(user,"~FTBohuzial niekde sa stratili figurky aj kocka, asi si nezahras.\n");
		return;
	}
	cnt=0;
	for (i=0;i<4;++i) {
		fscanf(fp,"%s\n",tempusername);
		if (strcmp(tempusername,"N/A")) {
			if (!(u=get_user(tempusername))) {
				sprintf(text,"~FT%s tu nie je. Musia byt prihlaseni vsetci hraci.\n",tempusername);
				write_user(user,text);
				return;
			}
			if (u->chrac>0) {
				sprintf(text,"~FT%s uz hra clovece, vsetci hraci musia byt volni.\n",u->name);
				write_user(user,text);
				return;
			}
			user->clovece->loadhrac[i]=u;
			cnt++;
		}
	}
	for (i=0;i<4;++i)
		fscanf(fp,"%d %d %d %d\n",&user->clovece->figurka[i][0],&user->clovece->figurka[i][1],&user->clovece->figurka[i][2],&user->clovece->figurka[i][3]);
	fscanf(fp,"%d %d %d %d\n",&user->clovece->monochrom[0],&user->clovece->monochrom[1],&user->clovece->monochrom[2],&user->clovece->monochrom[3]);
	fscanf(fp,"%d %d %d %d\n",&user->clovece->usertahs[0],&user->clovece->usertahs[1],&user->clovece->usertahs[2],&user->clovece->usertahs[3]);
	fscanf(fp,"%d %d %d %d\n",&user->clovece->natahu,&user->clovece->tahov,&user->clovece->locked,&user->clovece->kocka);
	fclose(fp);
	for (i=0;i<4;++i)
		if (user->clovece->loadhrac[i]==user) {
			user->chrac=i+1;
			user->clovece->hrac[i]=user;
		}
	user->clovece->loadlock=cnt-1;
	if (user->clovece->tahov>20)
		user->clovece->tahov=20;
	clovece_disp(user);
	for (i=0;i<4;++i)
		if (user->clovece->loadhrac[i]!=NULL && i!=user->chrac-1) {
			if (user->clovece->loadhrac[i]->clovece_starter==user) {
				sprintf(text,"~FTUz si %s navrhoval%s clovece.\n",sklonuj(user->clovece->loadhrac[i],3),pohl(user,"","a"));
				write_user(user,text);
			}
			else {
				sprintf(text,"~FTUkazujes ~OL%s~RS~FT hraciu plochu.. dohrame to?\n",sklonuj(user->clovece->loadhrac[i],3));
				write_user(user,text);
				sprintf(text,"\n~FT~OL%s~RS~FT ti ukazuje hraciu plochu s rozohratym clovece.. dohrame to ?\n",user->name);
				write_user(user->clovece->loadhrac[i],text);
			}
		}
}

char *clovece_saved_opponent(UR_OBJECT user)
{
	FILE *fp;
	char filename[80];
	int i,cnt;

	sprintf(filename,"%s/%s.sav",CLOVECEFILES,user->name);
	if (!(fp=fopen(filename,"r"))) {
		strcpy(text,"_none_");
		return text;
	}
	texthb[0]='\0';
	cnt=0;
	for (i=0;i<4;++i) {
		fscanf(fp,"%s\n",text);
		if (strcmp(text,"N/A")) {
			cnt++;
			if (cnt>1) strcat(texthb,"~RS~FT, ~OL");
			strcat(texthb,text);
		}
	}
	fclose(fp);
	return texthb;
}

extern void add_cloveceplayer(UR_OBJECT user,UR_OBJECT u)
{
	int i,j=0,cnt,o;

	cnt=0;
	for (i=0;i<4;++i)
		if (u->clovece->hrac[i]!=NULL) {
			j=i;
			cnt++;
		}
	if (cnt==1) {
		if (j==1)
			i=3;
		else
			i=abs(2-j);
		u->clovece->kocka=(int)(1+rand()%6);
		u->clovece->natahu=i+1;
	}
	if (cnt>1) {
		for (i=0;i<4 && u->clovece->hrac[i]!=NULL;++i);
		for (o=0;o<4;++o)
			for (j=0;j<4;++j)
				if (u->clovece->figurka[o][j]==i*10+1) {
					write_user(user,"~FTTeraz sa nemozes pripojit, vyrazil by si niekomu figurku.\n");
					sprintf(text,"~FT%s sa chcel%s pripojit ale vyrazil%s by niekomu figurku.\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
					write_user(u,text);
					return;
				}
	}
	user->clovece=u->clovece;
	user->chrac=i+1;
	user->clovece->hrac[i]=user;
	user->clovece->figurka[i][0]=1+i*10;
	for (j=1;j<4;++j)
		user->clovece->figurka[i][j]=i*10+51+j;
	for (j=0;j<4;++j)
		if (user->clovece->hrac[j]!=NULL)
			clovece_disp(user->clovece->hrac[j]);
	sprintf(text,"~FT%s si rozlozil%s ~OL%sfigurky~RS~FT a pridal%s sa do hry.\n",user->name,pohl(user,"","a"),clovece_col[user->chrac-1],pohl(user,"","a"));
	for (i=0;i<4;++i)
		if (user->clovece->hrac[i]!=NULL && i!=user->chrac-1)
			write_user(user->clovece->hrac[i],text);
	sprintf(text,"~FTRozlozil%s si si ~OL%sfigurky~RS~FT a pridal%s sa do hry.\n",pohl(user,"","a"),clovece_col[user->chrac-1],pohl(user,"","a"));
	write_user(user,text);
}

void hod_kockou(UR_OBJECT user,int rec)
{
	int cnt=0,i,j;

	for (i=0;i<4;++i)
		if (
				(user->clovece->figurka[user->clovece->natahu-1][i]-50)-10*(user->clovece->natahu-1)>0
				&&
				(user->clovece->figurka[user->clovece->natahu-1][i]-50)-10*(user->clovece->natahu-1)<5
		   )
			cnt++;
	if (cnt==4)
		user->clovece->kocka=(int)(1+rand()%7);
	else
		user->clovece->kocka=(int)(1+rand()%6);
	if (user->clovece->kocka==7)
		user->clovece->kocka=6;
	if (rec==0)
		return;
	i=0;
	for (j=0;j<4;++j) {
		cnt=user->clovece->figurka[user->clovece->natahu-1][j]-(user->clovece->natahu-1)*10-50;
		if (0<cnt && cnt<9)
			i++;
	}
	if (i==4 && user->clovece->kocka<6) {
		for (j=0;j<4;++j)
			if (user->clovece->hrac[j]!=NULL)
				clovece_disp(user->clovece->hrac[j]);
		write_user(user->clovece->hrac[user->clovece->natahu-1],"~FTCakas kym ti padne 6ka.\n");
		clovece_next(user);
		hod_kockou(user,1);
	}
}

void clovece_next(UR_OBJECT user)
{
	int i,j;

	i=user->clovece->natahu;
	j=i;
	while (j==user->clovece->natahu) {
		i++;
		if (i>4)
			i=1;
		if (user->clovece->hrac[i-1]!=NULL)
			user->clovece->natahu=i;
	}
}

void clovece(UR_OBJECT user,char *inpstr)
{
	UR_OBJECT u=NULL;
	int i,j=0,cnt,tah,tahable[5],x,y,pl,line;
	char *pom;

	if (!strcmp(word[1],"goto")) {
		if (user->chrac>0)
			user->clovece->figurka[user->clovece->natahu-1][atoi(word[2])]=atoi(word[3]);
		return;
	}
	if (!strcmp(word[1],"kocka")) {
		if (user->chrac>0)
			user->clovece->kocka=atoi(word[2]);
		return;
	}
	if (!strcmp(word[1],"natahu")) {
		if (user->chrac>0)
			user->clovece->natahu=atoi(word[2]);
		return;
	}
	if (!strcmp(word[1],"say") || !strncmp(word[1],"pov",3)) {
		if (user->chrac>0) {
			if (user->muzzled) {
				write_user(user,"Si umlcany.\n");
				return;
			}
			inpstr=remove_first(inpstr);
			if (strlen(inpstr)>0) {
				sprintf(text,"~OL%sHovoris~RS~FT spoluhracom:~FW %s\n",clovece_col[user->chrac-1],inpstr);
				write_user(user,text);
				sprintf(text,"~OL%s%s~RS~FT hovori:~FW %s\n",clovece_col[user->chrac-1],user->name,inpstr);
				for (i=0;i<4;++i)
					if (user->clovece->hrac[i]!=NULL && user!=user->clovece->hrac[i])
						write_user(user->clovece->hrac[i],text);
				record_clovece(user->clovece);
				return;
			}
			else {
				write_user(user,"~FTCo chces povedat spoluhracom?\n");
				return;
			}
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strncmp(word[1],"rev",3)) {
		if (user->chrac>0) {
			inpstr=remove_first(inpstr);
			inpstr=remove_first(inpstr);
			cnt=0;
			strtolower(inpstr);
			for (i=0;i<REVIEW_LINES;++i) {
				line=(user->clovece->revline+i)%REVIEW_LINES;
				if (user->clovece->revbuff[line][0]) {
					cnt++;
					if (cnt==1) {
						sprintf(text,"Hlasky spoluhracov");
						write_user(user,header(text));
					}
					pom=(char *) malloc ((strlen(user->clovece->revbuff[line])*sizeof(char))+1);
					if (pom!=NULL) {
						strcpy(pom,user->clovece->revbuff[line]);
						strtolower(pom);
						if (!strlen(inpstr) || strstr(pom,inpstr))
							write_user(user,user->clovece->revbuff[line]);
						free(pom);
					}
					else {
						sprintf(text,"~OL~FROUT OF MEMORY IN CLOVECE()\n");
						write_level(KIN,1,text,NULL);
						colour_com_strip(text);
						write_syslog(text,1);
					}
				}
			}
			if (!cnt) write_user(user,"Spoluhraci este nekecali.\n");
			oline(user);
			return;
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strcmp(word[1],"remiza")) {
		if (user->chrac>0) {
			cnt=0;j=0;
			for (i=0;i<4;++i)
				if (user->clovece->hrac[i]!=NULL && user!=user->clovece->hrac[i]) {
					cnt++;
					if (user->clovece->remiza[i]==1)
						j++;
				}
			if (cnt==j) {
				sprintf(text,"~FTVsetci suhlasili s remizou, hracia plocha vysublimovala.\n");
				for (i=0;i<4;++i)
					if (user->clovece->hrac[i]!=NULL) {
						write_user(user->clovece->hrac[i],text);
						user->clovece->hrac[i]->chrac=0;
						if (user!=user->clovece->hrac[i])
							user->clovece->hrac[i]->clovece=NULL;
					}
				destruct_clovece(user);
				return;
			}
			sprintf(text,"~FTNavrh%s si protihracom remizu.\n",pohl(user,"al","la"));
			write_user(user,text);
			sprintf(text,"~OL%s%s~RS~FT navrhuje remizu.\n",clovece_col[user->chrac-1],user->name);
			for (i=0;i<4;++i)
				if (user->clovece->hrac[i]!=NULL && user!=user->clovece->hrac[i])
					write_user(user->clovece->hrac[i],text);
			user->clovece->remiza[user->chrac-1]=1;
			return;
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strcmp(word[1],"otvor") || !strcmp(word[1],"open")) {
		if (user->chrac>0) {
			if (user->chrac!=3) {
				write_user(user,"~FTVed uz hras clovece, najprv musis ukoncit hru.\n");
				return;
			}
			else {
				user->clovece->locked=0;
				write_user(user,"~FTZase sa mozu pripajat hraci.\n");
				sprintf(text,"~FT%s odomk%s hru, zase sa mozu pripajat hraci.\n",user->name,pohl(user,"al","la"));
				for (i=0;i<4;++i)
					if (i!=2 && user->clovece->hrac[i]!=NULL)
						write_user(user->clovece->hrac[i],text);
				return;
			}
		}
		else {
			if ((user->clovece=create_clovece())==NULL) {
				write_user(user,"~FTBohuzial niekde sa stratili figurky aj kocka, asi si nezahras.\n");
				return;
			}
			user->clovece->hrac[2]=user;
			user->chrac=3;
			user->clovece->figurka[2][0]=21;
			for (j=1;j<4;++j) user->clovece->figurka[2][j]=71+j;
			clovece_disp(user);
			sprintf(text,"~FTRozlozil%s si hraciu plochu a figurky. Pozvi niekoho do hry. (clovece <meno>)\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
	}
	if (!strcmp(word[1],"uzavri")) {
		if (user->chrac==0) {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
		else {
			if (user->chrac!=3) {
				write_user(user,"~FTUzavriet hru moze len ten kto ju otvoril.\n");
				return;
			}
			user->clovece->locked=1;
			sprintf(text,"~FTUzavrel%s si hru, uz sa nikto nemoze pripojit.\n",pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"~FT%s uzavrel%s hru, uz sa nikto nemoze pripojit.\n",user->name,pohl(user,"","a"));
			for (i=0;i<4;++i)
				if (i!=2 && user->clovece->hrac[i]!=NULL)
					write_user(user->clovece->hrac[i],text);
			return;
		}
	}
	if (!strcmp(word[1],"koniec")) {
		if (user->chrac>0) {
			if (user->clovece->loadlock>0) {
				sprintf(text,"~FTPoskladal%s si hraciu plochu, nema to vyznam.\n",pohl(user,"","a"));
				write_user(user,text);
				sprintf(text,"~FT%s poskladal%s hraciu plochu, nema to vyznam.\n",user->name,pohl(user,"","a"));
				for (j=0;j<4;++j)
					if (user->clovece->hrac[j]!=NULL && user!=user->clovece->hrac[j]) {
						write_user(user->clovece->hrac[j],text);
						user->clovece->hrac[j]->clovece=NULL;
						user->clovece->hrac[j]->chrac=0;
					}
				user->chrac=0;
				destruct_clovece(user);
				return;
			}
			cnt=0;
			for (j=0;j<4;++j)
				user->clovece->figurka[user->chrac-1][j]=0;
			if (user->clovece->natahu==user->chrac) {
				i=user->clovece->natahu;
				j=i;
				while (j==user->clovece->natahu) {
					i++;
					if (i>4)
						i=1;
					if (user->clovece->hrac[i-1]!=NULL && user!=user->clovece->hrac[i-1])
						user->clovece->natahu=i;
				}
			}
			for (j=0;j<4;++j)
				if (user->clovece->hrac[j]!=NULL)
					clovece_disp(user->clovece->hrac[j]);
			for (i=0;i<4;++i)
				if (user->clovece->hrac[i]!=NULL)
					cnt++;
			if (cnt>1) {
				j=(int)(rand()%4);
				switch (j) {
					case 0:
						sprintf(text,"~FT%s sa nahneval%s a hodil%s svoje figurky o zem.\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 1:
						sprintf(text,"~FT%s sa nastval%s a porozhadzoval%s svoje figurky po okoli.\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 2:
						sprintf(text,"~FT%s hodil%s na zem svoje figurky a zurivo po nich zacal%s skakat.\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 3:
						sprintf(text,"~FT%s sa uz nechce hrat clovece, vzal%s svoje figurky a odlozil%s ich do krabice.\n",sklonuj(user,3),pohl(user,"","a"),pohl(user,"","a"));
						break;
				}
				for (i=0;i<4;++i)
					if (user->clovece->hrac[i]!=NULL && i!=user->chrac-1) {
						if (user->room!=user->clovece->hrac[i]->room)
							write_user(user->clovece->hrac[i],text);
						u=user->clovece->hrac[i];
					}
				user->clovece->hrac[user->chrac-1]=NULL;
				user->clovece=NULL;
				user->chrac=0;
				user->clovece_starter=NULL;
				write_room_except(user->room,text,user);
				switch (j) {
					case 0:
						sprintf(text,"~FTNahneval%s si sa a hodil%s si o zem svoje figurky z clovece.\n",pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 1:
						sprintf(text,"~FTNastval%s si sa a porozhadzoval%s si svoje figurky z clovece po okoli.\n",pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 2:
						sprintf(text,"~FTHodil%s si na zem svoje figurky z clovece a zurivo zacal%s po nich skakat.\n",pohl(user,"","a"),pohl(user,"","a"));
						break;
					case 3:
						sprintf(text,"~FTNechce sa ti uz hrat clovece, tak si vzal%s svoje figurky a odlozil%s ich do krabice.\n",pohl(user,"","a"),pohl(user,"","a"));
						break;
				}
				write_user(user,text);
			}
			if (cnt==2) {
				sprintf(text,"~FTPoskladal%s si hraciu plochu a figurky, si vitaz!\n",pohl(u,"","a"));
				write_user(u,text);
				sprintf(text,"~FT%s vyhral%s v clovece a od stastia sa rozplakal%s.\n",u->name,pohl(u,"","a"),pohl(u,"","a"));
				write_room_except(u->room,text,u);
				u->chrac=0;
				if (u->clovece->tahov>30)
					u->clovecewin++;
				destruct_clovece(u);
				u->clovece_starter=NULL;
			}
			if (cnt==1) {
				sprintf(text,"~FTPoskladal%s si hraciu plochu a figurky.\n",pohl(user,"","a"));
				write_user(user,text);
				user->chrac=0;
				destruct_clovece(user);
			}
			return;
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strcmp(word[1],"stav") || !strcmp(word[1],"show") || !strcmp(word[1],"ukaz")) {
		if (user->chrac>0) {
			clovece_disp(user);
			return;
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strcmp(word[1],"colors") || !strcmp(word[1],"farby")) {
		if (user->chrac>0) {
			if (user->clovece->monochrom[user->chrac-1]==0)
				user->clovece->monochrom[user->chrac-1]=1;
			else
				user->clovece->monochrom[user->chrac-1]=0;
			clovece_disp(user);
			if (user->clovece->monochrom[user->chrac-1]==1)
				sprintf(text,"~FTNastavil%s si vystup pre ciernobiely alebo monochromaticky monitor.\n",pohl(user,"","a"));
			else
				sprintf(text,"~FTNastavil%s si vystup pre farebny monitor.\n",pohl(user,"","a"));
			write_user(user,text);
			return;
		}
		else {
			write_user(user,"~FTPrepinat farby v clovece mozes az ked hras.\n");
			return;
		}
	}
	if (!strcmp(word[1],"save") || !strcmp(word[1],"uloz")) {
		if (user->chrac>0) {
			cnt=0;
			for (i=0;i<4;++i)
				if (user->clovece->hrac[i]!=NULL) cnt++;
			if (cnt==1) {
				write_user(user,"~FTCo chces ukladat ? Ved hra ani nezacala.\n");
				return;
			}
			clovece_save(user);
			sprintf(text,"~FTUlozil%s si rozohratu hru clovece.\n",pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"~FT~OL%s~RS~FT ulozil%s rozohratu hru clovece.\n",user->name,pohl(user,"","a"));
			for (i=0;i<4;++i)
				if (user->clovece->hrac[i]!=NULL && i!=user->chrac-1)
					write_user(user->clovece->hrac[i],text);
			return;
		}
		else {
			write_user(user,"~FTVed nehras clovece.\n");
			return;
		}
	}
	if (!strcmp(word[1],"load") || !strcmp(word[1],"pokracuj")) {
		if (user->chrac>0)
			write_user(user,"~FTVed uz hras clovece, najprv musis ukoncit hru.\n");
		else
			clovece_load(user);
		return;
	}
	if (word_count==2) {
		if (strlen(word[1])>1 && strcmp(word[1],"cakaj")) {
			if (!(u=get_user(word[1]))) {
				write_user(user,notloggedon);
				return;
			}
			if (user==u) {
				write_user(user,"~FTMyslis ze by ta to bavilo?\n");
				return;
			}
			if (user->chrac>0) {
				if (user->clovece->loadlock>0) {
					j=0;
					for (i=0;i<4;++i)
						if (user->clovece->loadhrac[i]==u)
							j=i+1;
					if (j==0) {
						write_user(user,"~FTMozes vyzvat len hracov ktori hrali pred tym.\n");
						return;
					}
				}
				if (u->chrac>0) {
					sprintf(text,"~FT~OL%s~RS~FT uz hra clovece.\n",u->name);
					write_user(user,text);
					return;
				}
				cnt=0;
				for (i=0;i<4;++i)
					if (user->clovece->hrac[i]!=NULL)
						cnt++;
				if (cnt>3) {
					sprintf(text,"~FTVed uz hrate styria, nemoze s vami hrat.\n");
					write_user(user,text);
					return;
				}
				if (user->clovece->locked==1) {
					sprintf(text,"~FTHra je uzavreta, nemozes nikoho pozvat.\n");
					write_user(user,text);
					return;
				}
				if (user->clovece_starter==u) {
					if (user->clovece->loadlock>0) {
						u->chrac=j;
						u->clovece=user->clovece;
						u->clovece->hrac[j-1]=user;
						u->clovece->loadlock--;
						for (i=0;i<4;++i)
							if (user->clovece->hrac[i]!=NULL)
								clovece_disp(user->clovece->hrac[i]);
						return;
					}
					else
						add_cloveceplayer(u,user);
					return;
				}
				if (u->clovece_starter==user) {
					sprintf(text,"~FTUz si %s navrhoval%s clovece.\n",pohl(u,"mu","jej"),pohl(user,"","a"));
					write_user(user,text);
					return;
				}
				else {
					if (u->igngames) {
						sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",u->name);
						write_user(user,text);
						return;
					}
					sprintf(text,"~FTUkazujes ~OL%s~RS~FT figurky a kocku, ci si nezahra clovece.\n",sklonuj(u,3));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT ti ukazuje figurky a kocku.. nezahras si clovece?\n",user->name);
					write_user(u,text);
					u->clovece_starter=user;
					return;
				}
			}
			else {
				if (u->chrac==0) {
					sprintf(text,"~FT~OL%s~RS~FT nehra clovece.\n",u->name);
					write_user(user,text);
					return;
				}
				j=10;
				if (u->clovece->loadlock>0) {
					j=0;
					for (i=0;i<4;++i) if (u->clovece->loadhrac[i]==user) j=i+1;
					if (j>0)
						user->clovece_starter=u;
				}
				cnt=0;
				for (i=0;i<4;++i)
					if (u->clovece->hrac[i]!=NULL)
						cnt++;
				if (cnt>3) {
					sprintf(text,"~FTUz hraju styria, uz nemozes s nimi hrat.\n");
					write_user(user,text);
					return;
				}
				if ((u->clovece->locked==1 && j==10) || j==0) {
					sprintf(text,"~FTUz sa k nim nemozes pripojit.\n");
					write_user(user,text);
					return;
				}
				if (u->clovece->loadlock>0) {
					user->chrac=j;
					user->clovece=u->clovece;
					user->clovece->hrac[j-1]=user;
					user->clovece->loadlock--;
					for (i=0;i<4;++i)
						if (user->clovece->hrac[i]!=NULL)
							clovece_disp(user->clovece->hrac[i]);
					return;
				}
				if (user->clovece_starter==u) {
					add_cloveceplayer(user,u);
					return;
				}
				if (u->clovece_starter==user) {
					sprintf(text,"~FTUz si sa %s pytal%s ci mozes hrat.\n",pohl(u,"ho","jej"),pohl(user,"","a"));
					write_user(user,text);
					return;
				}
				else {
					sprintf(text,"~FTPytas sa ~OL%s~RS~FT ci sa mozes pridat do hry clovece.\n",sklonuj(u,2));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT sa ta pyta ci sa moze pridat do hry clovece.\n",user->name);
					write_user(u,text);
					u->clovece_starter=user;
					return;
				}
			}
		}
		else {
			if (user->chrac>0) {
				if (user->clovece->loadlock>0) 	{
					write_user(user,"~FTAby hra mohla pokracovat, musia sa vsetci hraci chopit figurok.\n");
					return;
				}
				cnt=0;
				for (i=0;i<4;++i)
					if (user->clovece->hrac[i]!=NULL)
						cnt++;
				if (cnt==1) {
					write_user(user,"~FTSkus si najst najprv aspon jedneho spoluhraca.\n");
					return;
				}
				if (user->clovece->natahu!=user->chrac) {
					sprintf(text,"~FTNa tahu je teraz %s, nemontuj sa %s do toho.\n",user->clovece->hrac[user->clovece->natahu-1]->name,pohl(user->clovece->hrac[user->clovece->natahu-1],"mu","jej"));
					write_user(user,text);
					return;
				}
				if (!strcmp(word[1],"cakaj"))
					tah=10;
				else {
					tah=atoi(word[1]);
					if (tah<1 || 4<tah) {
						write_user(user,"~FTMusis zadat cislo figurky ktorou chces tahat (od 1 do 4).\n");
						return;
					}
					tah--;
				}
				if (user->chrac==1)
					pl=1;
				else
					pl=0;
				cnt=0;
				for (j=0;j<5;++j)
					tahable[j]=0;
				for (i=0;i<4;++i) {
					if (
							user->clovece->figurka[user->chrac-1][i]+user->clovece->kocka < (user->chrac-1)*10+1+pl*40
							||
							(
							 user->clovece->figurka[user->chrac-1][i]>(user->chrac-1)*10
							 &&
							 user->clovece->figurka[user->chrac-1][i]<41-pl*6
							)
					   ) {
						tahable[4]++;
						tahable[i]=1;
						for (j=0;j<4;++j)
							if (user->clovece->figurka[user->chrac-1][j]==user->clovece->figurka[user->chrac-1][i]+user->clovece->kocka)
								tahable[i]=0;
					}
					else
						for (x=1;x<3;++x) {
							if (x==1) {
								if (
										(user->clovece->figurka[user->chrac-1][i]-50)-10*(user->chrac-1)>0
										&&
										(user->clovece->figurka[user->chrac-1][i]-50)-10*(user->chrac-1)<5
								   ) {
									if (user->clovece->kocka==6)
										tahable[i]=1;
									else
										tahable[i]=0;
									cnt++;
									continue;
								}
								if (user->clovece->kocka>3)
									continue;
								y=user->clovece->figurka[user->chrac-1][i]-54-10*(user->chrac-1)+user->clovece->kocka;
							}
							else {
								y=user->clovece->figurka[user->chrac-1][i]+user->clovece->kocka-(user->chrac-1)*10-pl*40;
							}
							if (0<y && y<5 && tahable[i]==0) {
								tahable[i]=1;
								for (j=0;j<4;++j)
									if (user->clovece->figurka[user->chrac-1][j]==y+54+10*(user->chrac-1))
										tahable[i]=0;
							}
						}
				}
				x=0;
				for (j=0;j<4;++j)
					if (tahable[j]==1)
						x++;
				if ((cnt>3 && user->clovece->kocka!=6) || (x==0)) {
					if (user->clovece->kocka<6)
						clovece_next(user);
					hod_kockou(user,1);
					if (cnt==4)
						write_user(user,"~FTCakas kym ti padne 6ka..\n");
					else
						write_user(user,"~FTNemozes tahat.. cakas.\n");
					user->clovece->usertahs[user->chrac-1]++;
					user->clovece->tahov++;
					for (j=0;j<4;++j)
						if (user->clovece->hrac[j]!=NULL)
							clovece_disp(user->clovece->hrac[j]);
					return;
				}
				if (tah==10) {
					write_user(user,"~FTMusis tahat figurkou.\n");
					return;
				}
				i=0;
				for (j=0;j<4;++j)
					if (user->clovece->figurka[user->chrac-1][j]==(user->chrac-1)*10+1)
						i++;
				y=user->clovece->figurka[user->chrac-1][tah]-50-10*(user->chrac-1);
				if (i==0 && cnt>0 && user->clovece->kocka==6 && 0<y && y<5) {
					user->clovece->figurka[user->chrac-1][tah]=(user->chrac-1)*10+1;
					user->clovece->kocka=(int)(1+rand()%6);
				}
				else {
					for (j=0;j<4;++j)
						if (user->clovece->figurka[user->chrac-1][j]==(user->chrac-1)*10+1 && (tah!=j)) {
							sprintf(text,"~FTMusis tahat figurkou ktoru si vylozil%s aj keby si sa zasamovrazdil%s.\n",pohl(user,"","a"),pohl(user,"","a"));
							write_user(user,text);
							return;
						}
					if (
							(user->clovece->figurka[user->chrac-1][tah]-50)-10*(user->chrac-1)>0
							&&
							(user->clovece->figurka[user->chrac-1][tah]-50)-10*(user->chrac-1)<5
					   ) {
						write_user(user,"~FTNemozes vytiahnut figurku z garaze, najprv ti musi padnut 6 alebo viac.\n");
						return;
					}
					else {
						cnt=0;
						cnt=(user->clovece->figurka[user->chrac-1][tah]-54-(user->chrac-1)*10);
						if (0<cnt && cnt<5)
							cnt+=user->clovece->kocka;
						else
							cnt=0;
						if (
								(user->clovece->figurka[user->chrac-1][tah]<(user->chrac-1)*10+1+pl*40)
								&&
								(user->clovece->figurka[user->chrac-1][tah]+user->clovece->kocka>(user->chrac-1)*10+pl*40)
						   )
							cnt=user->clovece->figurka[user->chrac-1][tah]+user->clovece->kocka-(user->chrac-1)*10-pl*40;
						if (cnt>0) {
							if (cnt>4) {
								if ((int)(rand()%2)==0)
									sprintf(text,"~FTTou figurkou nemozes tahat, prebural%s by si zadnu stenu domceka.\n",pohl(user,"","a"));
								else
									sprintf(text,"~FTTou figurkou nemozes tahat, vypad%s by si dierkou v strede hracej plochy.\n",pohl(user,"al","la"));
								write_user(user,text);
								return;
							}
							for (j=0;j<4;++j)
								if ((user->clovece->figurka[user->chrac-1][j]-54)-10*(user->chrac-1)==cnt) {
									sprintf(text,"~FTTou figurkou nemozes tahat, zasamovrazdil%s by si si figurku v domceku.\n",pohl(user,"","a"));
									write_user(user,text);
									return;
								}
							user->clovece->figurka[user->chrac-1][tah]=54+10*(user->chrac-1)+cnt;
						}
						else {
							user->clovece->figurka[user->chrac-1][tah]+=user->clovece->kocka;
							if (
									user->clovece->figurka[user->chrac-1][tah]>40
									&&
									user->clovece->figurka[user->chrac-1][tah]<50
							   )
								user->clovece->figurka[user->chrac-1][tah]-=40;
						}
						if (user->clovece->kocka<6)
							clovece_next(user);
						y=0;
						for (j=0;j<4;++j) {
							x=user->clovece->figurka[user->chrac-1][j]-54-10*(user->chrac-1);
							if (0<x && x<5)
								y++;
						}
						if (y==4)
							hod_kockou(user,0);
						else
							hod_kockou(user,1);
					}
				}
				for (i=0;i<4;++i)
					for (j=0;j<4;++j)
						if (user->clovece->figurka[i][j]==user->clovece->figurka[user->chrac-1][tah] && (j!=tah || i!=user->chrac-1)) {
							if (user->chrac-1==i) {
								if ((int)(rand()%2)==0)
									sprintf(text,"~FT~OLVynikajuco! Zasamovrazdil%s si svoju figurku, teraz skus protihracovu.\n",pohl(user,"","a"));
								else
									sprintf(text,"~FT~OLVyrazil%s si si figurku! Skusal%s si uz vyrazit superovu ?\n",pohl(user,"","a"),pohl(user,"","a"));
							}
							else {
								if ((int)(rand()%2)==0)
									sprintf(text,"~OL%s%s ti odpratal%s figurku na start!\n",clovece_col[user->chrac-1],user->name,pohl(user,"","a"));
								else
									sprintf(text,"~OL%s%s ti odpazil%s figurku do garaze!\n",clovece_col[user->chrac-1],user->name,pohl(user,"","a"));
								write_user(user->clovece->hrac[i],text);
								if ((int)(rand()%2)==0)
									sprintf(text,"~FTVyborne, odpratal%s si ~OL%s%s~RS~FT figurku na start!\n",pohl(user,"","a"),clovece_col[i],sklonuj(user->clovece->hrac[i],3));
								else
									sprintf(text,"~FTVyborne, odpazil%s si ~OL%s%s~RS~FT do garaze!\n",pohl(user,"","a"),clovece_col[i],sklonuj(user->clovece->hrac[i],3));
							}
							write_user(user,text);
							for (cnt=4;cnt>0;--cnt) {
								y=0;
								for (x=0;x<4;++x)
									if (user->clovece->figurka[i][x]==i*10+50+cnt)
										y++;
								if (y==0) {
									user->clovece->figurka[i][j]=i*10+50+cnt;
									cnt=0;
									break;
								}
							}
						}
				user->clovece->usertahs[user->chrac-1]++;
				user->clovece->tahov++;
				for (j=0;j<4;++j)
					if (user->clovece->hrac[j]!=NULL)
						clovece_disp(user->clovece->hrac[j]);
				y=0;
				for (j=0;j<4;++j) {
					x=user->clovece->figurka[user->chrac-1][j]-54-10*(user->chrac-1);
					if (0<x && x<5)
						y++;
				}
				if (y==4) {
					if (user->clovece->natahu==user->chrac)
						clovece_next(user);
					cnt=0;
					for (i=0;i<4;++i)
						if (user->clovece->hrac[i]!=NULL && user->clovece->usertahs[i]>10)
							cnt++;
					if (user->clovece->tahov>40)
						user->clovecewin+=cnt-1;
					if (cnt==2) {
						sprintf(text,"~FT~OLVyborne! Vyhral%s si v clovece a od stastia si sa rozplakal%s.\n",pohl(user,"","a"),pohl(user,"","a"));
						write_user(user,text);
						sprintf(text,"~FT%s vyhral%s v clovece a od stastia sa rozplakal%s.\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
						write_room_except(user->room,text,user);
						sprintf(text,"~OL%s%s~RS~FT vyhral%s v clovece a od stastia sa rozplakal%s.\n",clovece_col[user->chrac-1],user->name,pohl(user,"","a"),pohl(user,"","a"));
					}
					else {
						sprintf(text,"~FT~OLVyborne, zaparkoval%s si vsetky figurky do domceka!\n",pohl(user,"","a"));
						write_user(user,text);
						sprintf(text,"~FT%s vitazoslavne vyhodil%s do vzduchu figurky z clovece.\n",user->name,pohl(user,"","a"));
						write_room_except(user->room,text,user);
						sprintf(text,"~OL%s%s~RS~FT vitazoslavne vyhodil%s do vzduchu figurky z clovece.\n",clovece_col[user->chrac-1],user->name,pohl(user,"","a"));
					}
					for (j=0;j<4;++j)
						if (user->clovece->hrac[j]!=NULL && j!=user->chrac-1 && user->clovece->hrac[j]->room!=user->room)
							write_user(user->clovece->hrac[j],text);
					for (j=0;j<4;++j)
						user->clovece->figurka[user->chrac-1][j]=0;
					cnt=0;
					for (i=0;i<4;++i)
						if (user->clovece->hrac[i]!=NULL)
							cnt++;
					user->clovece->hrac[user->chrac-1]=NULL;
					if (cnt==2) {
						u=user->clovece->hrac[user->clovece->natahu-1];
						if ((int)(rand()%2)==0) {
							sprintf(text,"~FT~OLPrehral%s si a rozplakal%s si sa..\n",pohl(u,"","a"),pohl(u,"","a"));
							write_user(u,text);
							sprintf(text,"~FT%s prehral%s v clovece a rozplakal%s sa.\n",u->name,pohl(u,"","a"),pohl(u,"","a"));
						}
						else {
							sprintf(text,"~FT~OLPrehral%s si a od zlosti si roztrhal%s hraciu plochu.\n",pohl(u,"","a"),pohl(u,"","a"));
							write_user(u,text);
							sprintf(text,"~FT%s prehral%s v clovece a od zlosti roztrhal%s hraciu plochu.\n",u->name,pohl(u,"","a"),pohl(u,"","a"));
						}
						write_room_except(u->room,text,u);
						u->chrac=0;
						u->clovece=NULL;
						destruct_clovece(user);
					}
					else
						user->clovece=NULL;
					user->chrac=0;
					user->clovece_starter=NULL;
				}
				return;
			}
			else {
				write_user(user,"~FTNemozes tahat, ved nehras clovece.\n");
				return;
			}
		}
	}
	write_user(user,"~FT       clovece <meno>  vyzves niekoho aby sa pripojil do hry\n");
	write_user(user,"~FT       clovece otvor   otvoris hru clovece (pre 2-4 hracov)\n");
	write_user(user,"~FT       clovece uzavri  zabrani pripajaniu sa dalsich hracov do hry\n");
	write_user(user,"~FT       clovece pokracuj pokracovanie ulozenej hry so vsetkymi protihracmi.\n");
	write_user(user,"~FT       clovece uloz    ulozis aktualny stav hry, da sa dokoncit neskor\n");
	write_user(user,"~FT       clovece koniec  vzdas sa a odpojis sa z hry\n");
	write_user(user,"~FT       clovece stav    zobrazi aktualny stav hry\n");
	write_user(user,"~FT       clovece farby   prepnes vystup pre monochromaticky/farebny monitor\n");
	write_user(user,"~FT       clovece povedz  <text> povies text spoluhracom (clovece say <text>)\n");
	write_user(user,"~FT       clovece rev     zobrazi posledne kecy hracov\n");
	write_user(user,"~FT       clovece cakaj   cakas ak nemozes tahat ziadnou figurkou\n");
	write_user(user,"~FT       clovece <cislo> urobis tah figurkou s cislom ktore si zadal\n");
	strcpy(text,clovece_saved_opponent(user));
	if (strcmp(text,"_none_")) {
		sprintf(texthb,"~FTMas ulozenu rozohratu hru, hraci: ~OL%s\n",text);
		write_user(user,texthb);
	}
}

FR_OBJECT create_farar(void)
{
	FR_OBJECT farar;
	int i;

	if ((farar=(FR_OBJECT)malloc(sizeof(struct fr_struct)))==NULL) {
		write_syslog("HUH: Memory allocation failure in create_farar().\n",0);
		return NULL;
	}
	for (i=0;i<32;++i)
		farar->karty[i]=8;
	for (i=0;i<5;++i)
		farar->hrac[i]=NULL;
	for (i=0;i<5;++i)
		farar->active[i]=0;
	farar->natahu=-1;
	farar->action=0;
	farar->revline=0;
	for (i=0;i<REVIEW_LINES;++i)
		farar->revbuff[i][0]='\0';
	return farar;
}

void record_farar(FR_OBJECT farar)
{
	sstrncpy(farar->revbuff[farar->revline],text,REVIEW_LEN);
	farar->revbuff[farar->revline][REVIEW_LEN]='\n';
	farar->revbuff[farar->revline][REVIEW_LEN+1]='\0';
	farar->revline=(farar->revline+1)%REVIEW_LINES;
}

void destruct_farar(UR_OBJECT user)
{
	free(user->farar);
}

void add_fararplayer(UR_OBJECT user,UR_OBJECT u)
{
	int i=-1,cnt=0,o;
	char players[80];

	for (o=0;o<5;++o)
		if (u->farar->hrac[o]==NULL) {
			if (i==-1)
				i=o;
			cnt++;
		}
	user->farar=u->farar;
	user->fhrac=i;
	user->farar->active[i]=0;
	user->farar->hrac[i]=user;
	if (user->farar->natahu==-1) {
		sprintf(text,"~FTPrisad%s si si a cakas kym sa zacne hra.\n",pohl(user,"ol","la"));
		write_user(user,text);
		sprintf(text,"~FT%s si k vam prisad%s a caka kym zacne hra.\n",user->name,pohl(user,"ol","la"));
	}
	else {
		sprintf(text,"~FTPrisad%s si si a cakas kym dohraju rundu.\n",pohl(user,"ol","la"));
		write_user(user,text);
		sprintf(text,"~FT%s si k vam prisad%s, dalsiu rundu sa prida do hry.\n",user->name,pohl(user,"ol","la"));
	}
	strcpy(players,"~FTOkrem teba hraju:~FW ");
	o=0;
	for (i=0;i<5;++i)
		if (user->farar->hrac[i]!=NULL && i!=user->fhrac) {
			o++;
			if (o>1)
				strcat(players,"~FT,~FW ");
			write_user(user->farar->hrac[i],text);
			strcat(players,user->farar->hrac[i]->name);
		}
	strcat(players,".\n");
	write_user(user,players);
}

void farar(UR_OBJECT user,char *inpstr)
{
	UR_OBJECT u=NULL;
	unsigned int cnt,pocet;
	int i,j=0,o=0,y,line;
	int tah[5];
	int farba,oznac,zmen;
	int hold[32];
	char *pom,vtext[100];

	if (!strcmp(word[1],"huh!")) {
		if (user->fhrac>-1) {
			cnt=0;
			for (i=0;i<5;i++)
				if (user->farar->hrac[i]!=NULL) {
					sprintf(text,"%d.%s/%d,  ",i,user->farar->hrac[i]->name,user->farar->active[i]);
					write_user(user,text);
				}
			sprintf(text,"%d\n",user->farar->action);
			write_user(user,text);
			for (i=0;i<32;i++) {
				sprintf(text,"%4d ",user->farar->karty[i]);
				write_user(user,text);
			}
			write_user(user,"\n");
		}
		return;
	}
	if (!strcmp(word[1],"gogogo!")) {
		if (user->fhrac>-1)
			user->farar->natahu=user->fhrac;
		return;
	}
	if (!strcmp(word[1],"say") || !strncmp(word[1],"pov",3)) {
		if (user->fhrac>-1) {
			if (user->muzzled) {
				write_user(user,"Si umlcany.\n");
				return;
			}
			cnt=0;
			for (i=0;i<5;i++)
				if (user->farar->hrac[i]!=NULL)
					cnt++;
			if (cnt<2) {
				write_user(user,"~FTSamomluva je prvy priznak sialenstva.\n");
				return;
			}
			inpstr=remove_first(inpstr);
			if (strlen(inpstr)>0) {
				sprintf(text,"~FTHovoris spoluhracom:~FW %s\n",inpstr);
				write_user(user,text);
				sprintf(text,"~OL~FT%s~RS~FT hovori:~FW %s\n",user->name,inpstr);
				for (i=0;i<5;++i)
					if (user->farar->hrac[i]!=NULL && user!=user->farar->hrac[i])
						write_user(user->farar->hrac[i],text);
				record_farar(user->farar);
				return;
			}
			else {
				write_user(user,"~FTCo chces povedat spoluhracom ?\n");
				return;
			}
		}
		else {
			write_user(user,"~FTVed nehras faraona.\n");
			return;
		}
	}
	if (!strncmp(word[1],"rev",3)) {
		if (user->fhrac>-1) {
			inpstr=remove_first(inpstr);
			inpstr=remove_first(inpstr);
			cnt=0;
			strtolower(inpstr);
			for (i=0;i<REVIEW_LINES;++i) {
				line=(user->farar->revline+i)%REVIEW_LINES;
				if (user->farar->revbuff[line][0]) {
					cnt++;
					if (cnt==1) {
						sprintf(text,"Hlasky spoluhracov");
						write_user(user,header(text));
					}
					pom=(char *) malloc ((strlen(user->farar->revbuff[line])*sizeof(char))+1);
					if (pom!=NULL) {
						strcpy(pom,user->farar->revbuff[line]);
						strtolower(pom);
						if (!strlen(inpstr) || strstr(pom,inpstr))
							write_user(user,user->farar->revbuff[line]);
						free(pom);
					}
					else {
						sprintf(text,"~OL~FROUT OF MEMORY IN FARAR()\n");
						write_level(KIN,1,text,NULL);
						colour_com_strip(text);
						write_syslog(text,1);
					}
				}
			}
			if (!cnt)
				write_user(user,"Spoluhraci este nekecali..\n");
			oline(user);
			return;
		}
		else {
			write_user(user,"~FTVed nehras faraona.\n");
			return;
		}
	}
	if (!strncmp(word[1],"otv",3) || !strcmp(word[1],"open")) {
		if (user->fhrac>-1) {
			write_user(user,"~FTVed uz hras faraona, najprv musis ukoncit hru.\n");
			return;
		}
		else {
			if ((user->farar=create_farar())==NULL) {
				write_user(user,"~FTBohuzial niekde sa stratili karty, asi si nezahras.\n");
				return;
			}
			user->farar->hrac[0]=user;
			user->fhrac=0;
			user->farar->natahu=-1;
			sprintf(text,"~FTPozical%s si si karty od Poseidona. Pozvi niekoho do hry. (faraon <meno>)\n",pohl(user,"","a"));
			write_user(user,text);
			write_user(user,"~FTKed sa pripoja hraci, karty rozdas prikazom 'faraon zacni'\n");
			farar_disp(user,-1);
			return;
		}
	}
	if (!strncmp(word[1],"kon",3)) {
		if (user->fhrac>-1) {
			cnt=0;
			for (i=0;i<5;++i)
				if (user->farar->hrac[i]!=NULL)
					cnt++;
			if (cnt==1) {
				sprintf(text,"~FTZbali%s si karty a vratil%s si ich Poseidonovi.\n",pohl(user,"","a"),pohl(user,"","a"));
				write_user(user,text);
				user->fhrac=-1;
				destruct_farar(user);
				user->farar_starter=NULL;
				return;
			}
			o=0;
			for (i=0;i<32;++i)
				if (user->farar->karty[i]>10 && (user->farar->karty[i]<user->farar->karty[o] || user->farar->karty[o]<10))
					o=i;
			y=user->farar->karty[o];
			for (j=0;j<32;++j)
				if (user->farar->karty[j]==user->fhrac) {
					y--;
					user->farar->karty[j]=y;
				}
			if (user->farar->natahu==user->fhrac) {
				i=user->farar->natahu;
				j=i;
				while (j==user->farar->natahu) {
					i++;
					if (i>4)
						i=0;
					if (user->farar->hrac[i]!=NULL && user!=user->farar->hrac[i] && user->farar->active[i]==1)
						user->farar->natahu=i;
				}
			}
			if (cnt>1) {
				sprintf(text,"~FT%s to prestalo bavit.. kasle na faraona.\n",sklonuj(user,4));
				for (i=0;i<5;++i)
					if (user->farar->hrac[i]!=NULL && i!=user->fhrac) {
						write_user(user->farar->hrac[i],text);
						u=user->farar->hrac[i];
					}
				user->farar->hrac[user->fhrac]=NULL;
				user->farar=NULL;
				user->fhrac=-1;
				user->farar_starter=NULL;
				sprintf(text,"~FTUz ta to nebavi.. prestal%s si hrat faraona.\n",pohl(user,"","a"));
				write_user(user,text);
			}
			if (cnt==2) {
				sprintf(text,"~FTNemas uz ziadneho spoluhraca.. vratil%s si Poseidonovi karty.\n",pohl(u,"","a"));
				write_user(u,text);
				destruct_farar(u);
				u->fhrac=-1;
				u->farar_starter=NULL;
				return;
			}
			cnt=0;
			for (i=0;i<5;++i)
				if (u->farar->hrac[i]!=NULL && u->farar->active[i]==1)
					cnt++;
			if (cnt==1) {
				for (i=0;i<5;++i)
					u->farar->active[i]=0;
				u->farar->natahu=-1;
				u->farar->action=u->fhrac;
				sprintf(text,"~FTRunda skoncila. Dalsiu rundu zacnete prikazom .faraon zacni.\n");
				for (i=0;i<5;++i)
					if (u->farar->hrac[i]!=NULL)
						write_user(u->farar->hrac[i],text);
			}
			return;
		}
		else {
			write_user(user,"~FTVed nehras faraona.\n");
			return;
		}
	}
	if (!strcmp(word[1],"stav") || !strcmp(word[1],"show") || !strcmp(word[1],"ukaz")) {
		if (user->fhrac>-1) {
			farar_disp(user,-1);
			return;
		}
		else {
			write_user(user,"~FTVed nehras faraona.\n");
			return;
		}
	}
	if (!strcmp(word[1],"start") || !strncmp(word[1],"zac",3)) {
		if (user->fhrac>-1) {
			cnt=0;
			for (i=0;i<5;++i)
				if (user->farar->hrac[i]!=NULL)
					cnt++;
			if (cnt==1) {
				write_user(user,"~FTSkus si najst najprv aspon jedneho spoluhraca.\n");
				return;
			}
			cnt=0;
			for (i=0;i<5;i++)
				if (user->farar->active[i]==1)
					cnt++;
			if (cnt>1 || user->farar->natahu>-1) {
				write_user(user,"~FTEste nedohrali rundu, nemozes zacat dalsiu.\n");
				return;
			}
			if (user->farar->hrac[user->farar->action]!=NULL)
				user->farar->natahu=user->farar->hrac[user->farar->action]->fhrac;
			else
				user->farar->natahu=0;
			user->farar->action=0;
			cnt=0;
			for (i=0;i<32;i++)
				user->farar->karty[i]=-1;
			for (i=0;i<5;++i) {
				if (user->farar->hrac[i]!=NULL) {
					user->farar->active[i]=1;
					for (o=0;o<5;o++) {
						cnt++;
						j=(int)(rand()%32);
						while (user->farar->karty[j]!=-1) {
							j++;
							if (j>31)
								j=0;
						}
						user->farar->karty[j]=i;
					}
				}
			}
			y=100;
			for (i=0;i<(int)(32-cnt);i++) {
				j=(int)(rand()%32);
				while (user->farar->karty[j]!=-1) {
					j++;
					if (j>31)
						j=0;
				}
				user->farar->karty[j]=y;
				y++;
			}
			sprintf(text,"~FTPomiesal%s a rozdal%s si karty..\n",pohl(user,"","a"),pohl(user,"","a"));
			write_user(user,text);
			sprintf(text,"~FT%s pomiesal%s a rozdal%s karty..\n",user->name,pohl(user,"","a"),pohl(user,"","a"));
			for (i=0;i<5;++i)
				if (user->farar->hrac[i]!=NULL && user!=user->farar->hrac[i])
					write_user(user->farar->hrac[i],text);
			for (i=0;i<5;++i)
				if (user->farar->hrac[i]!=NULL)
					farar_disp(user->farar->hrac[i],-1);
			return;
		}
		else {
			write_user(user,"~FTVed nehras faraona.\n");
			return;
		}
	}
	if (word_count>1) {
		if (strlen(word[1])>2 && strncmp(word[1],"sto",3) && strncmp(word[1],"tah",3)) {
			if (!(u=get_user(word[1]))) {
				write_user(user,notloggedon);
				return;
			}
			if (user==u) {
				write_user(user,"~FTMyslis ze by ta to bavilo?\n");
				return;
			}
			if (user->fhrac>-1) {
				if (u->fhrac>-1) {
					sprintf(text,"~FT~OL%s~RS~FT uz hra faraona.\n",u->name);
					write_user(user,text);
					return;
				}
				cnt=0;
				for (i=0;i<5;++i)
					if (user->farar->hrac[i]!=NULL)
						cnt++;
				if (cnt>4) {
					sprintf(text,"~FTVed uz hrate piati, nemoze s vami hrat.\n");
					write_user(user,text);
					return;
				}
				if (user->farar_starter==u) {
					sprintf(text,"~FT%s ta pozval do hry.\n",user->name);
					write_user(u,text);
					add_fararplayer(u,user);
					return;
				}
				if (u->farar_starter==user) {
					sprintf(text,"~FTUz si %s navrhoval%s faraona.\n",pohl(u,"mu","jej"),pohl(user,"","a"));
					write_user(user,text);
					return;
				}
				else {
					if (u->igngames) {
						sprintf(text,"%s si tu visi a vsetky hry ma na haku.\n",u->name);
						write_user(user,text);
						return;
					}
					sprintf(text,"~FTUkazujes ~OL%s~RS~FT karty, ci si nezahra faraona.\n",sklonuj(u,3));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT ti ukazuje karty.. nezahras si faraona ?\n",user->name);
					write_user(u,text);
					u->farar_starter=user;
					return;
				}
			}
			else {
				if (u->fhrac==-1) {
					sprintf(text,"~FT~OL%s~RS~FT nehra faraona.\n",u->name);
					write_user(user,text);
					return;
				}
				j=10;
				cnt=0;
				for (i=0;i<5;++i)
					if (u->farar->hrac[i]!=NULL)
						cnt++;
				if (cnt>4) {
					sprintf(text,"~FTUz hraju piati, uz nemozes s nimi hrat.\n");
					write_user(user,text);
					return;
				}
				if (user->farar_starter==u) {
					add_fararplayer(user,u);
					return;
				}
				if (u->farar_starter==user) {
					sprintf(text,"~FTUz si sa %s pytal%s ci mozes hrat.\n",pohl(u,"ho","jej"),pohl(user,"","a"));
					write_user(user,text);
					return;
				}
				else {
					sprintf(text,"~FTPytas sa ~OL%s~RS~FT ci sa mozes pridat do hry faraona.\n",sklonuj(u,2));
					write_user(user,text);
					sprintf(text,"~FT~OL%s~RS~FT sa ta pyta ci sa moze pridat do hry faraona.\n",user->name);
					write_user(u,text);
					u->farar_starter=user;
					return;
				}
			}
		}
		else {
			if (user->fhrac>-1) {
				cnt=0;
				for (i=0;i<5;++i)
					if (user->farar->hrac[i]!=NULL)
						cnt++;
				if (cnt==1) {
					write_user(user,"~FTSkus si najst najprv aspon jedneho spoluhraca.\n");
					return;
				}
				if (user->farar->natahu==-1) {
					sprintf(text,"~FTKarty este nie su rozdane, daj im pokoj !\n");
					write_user(user,text);
					return;
				}
				if (user->farar->active[user->fhrac]==0) {
					sprintf(text,"~FTUz si sa zbavil%s vsetkych kariet, pockaj kym zacne dalsie kolo.\n",pohl(user,"","a"));
					write_user(user,text);
					return;
				}
				if (user->farar->natahu!=user->fhrac) {
					if (user->farar->hrac[user->farar->natahu]==NULL) {
						sprintf(text,"~FTNepchaj tam tie karty, nie si na rade..\n");
						write_user(user,text);
						return;
					}
					sprintf(text,"~FTNa rade je teraz %s, nemontuj sa %s do toho.\n",user->farar->hrac[user->farar->natahu]->name,pohl(user->farar->hrac[user->farar->natahu],"mu","jej"));
					write_user(user,text);
					return;
				}
				if (!strncmp(word[1],"tah",3)) {
					if (100<user->farar->action && user->farar->action<105) {
						sprintf(text,"~FTNemozes si tahat, ak mas eso, mozes ho vyhodit, inak musis stat (.faraon stoj).\n");
						write_user(user,text);
						return;
					}
					else if (0==user->farar->action || 200<user->farar->action) {
						i=user->fhrac;
						j=i;
						while (i==user->farar->natahu) {
							j++;
							if (j>4)
								j=0;
							if (user->farar->hrac[j]!=NULL && user->farar->active[j]==1)
								user->farar->natahu=j;
						}
						j=0;
						for (i=0;i<32;++i)
							if (user->farar->karty[i]>10)
								j++;
						if (1<j)
							j=1;
						else
							j=0;
						if (j==0) {
							write_user(user,"~FTKeby ste hrali normalne a nerobili kreposti tak by v kope este boli kraty.\n");
							for (i=0;i<5;++i)
								if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1) {
									if (user->farar->natahu==i)
										sprintf(text,"~FT%s si chcel%s tahat ale dosli vam karty. ~OL~FYSi na rade.\n",user->name,pohl(user,"","a"));
									else
										sprintf(text,"~FT%s si chcel%s tahat ale dosli vam karty.\n",user->name,pohl(user,"","a"));
									write_user(user->farar->hrac[i],text);
								}
							return;
						}
						else {
							sprintf(text,"~FTPotiah%s si kartu z kopky.\n",pohl(user,"ol","la"));
							write_user(user,text);
							for (i=0;i<5;++i)
								if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1) {
									if (user->farar->natahu==i)
										sprintf(text,"~FT%s potiah%s kartu z kopky. ~OL~FYSi na rade.\n",user->name,pohl(user,"ol","la"));
									else
										sprintf(text,"~FT%s potiah%s kartu z kopky.\n",user->name,pohl(user,"ol","la"));
									write_user(user->farar->hrac[i],text);
								}
						}
					}
					else if (0<user->farar->action && user->farar->action<100) {
						i=user->fhrac;
						j=i;
						while (i==user->farar->natahu) {
							j++;
							if (j>4)
								j=0;
							if (user->farar->hrac[j]!=NULL && user->farar->active[j]==1)
								user->farar->natahu=j;
						}
						j=0;
						for (i=0;i<32;++i)
							if (user->farar->karty[i]>10)
								j++;
						if (user->farar->action<j)
							j=user->farar->action;
						else
							j--;
						sprintf(text,"~FTPotiah%s si ~OL%d~RS~FT kar%s z kopky.\n",pohl(user,"ol","la"),j,skloncislo(j,"tu","ty","iet"));
						write_user(user,text);
						for (i=0;i<5;++i)
							if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1) {
								if (user->farar->natahu==i)
									sprintf(text,"~FT%s si potiah%s ~OL%d~RS~FT kar%s z kopky. ~OL~FYSi na rade.\n",user->name,pohl(user,"ol","la"),j,skloncislo(j,"tu","ty","iet"));
								else
									sprintf(text,"~FT%s si potiah%s ~OL%d~RS~FT kar%s z kopky.\n",user->name,pohl(user,"ol","la"),j,skloncislo(j,"tu","ty","iet"));
								write_user(user->farar->hrac[i],text);
							}
						user->farar->action=0;
					}
					for (y=0;y<j;y++) {
						o=0;
						for (i=0;i<32;++i)
							if (user->farar->karty[i]>10 && (user->farar->karty[i]<user->farar->karty[o] || user->farar->karty[o]<10))
								o=i;
						user->farar->karty[o]=user->fhrac;
					}
					if (j==1)
						farar_disp(user,o);
					else
						farar_disp(user,-1);
					return;
				}
				if (!strncmp(word[1],"sto",3)) {
					if (100<user->farar->action && user->farar->action<105) {
						i=user->fhrac;
						j=i;
						while (i==user->farar->natahu) {
							j++;
							if (j>4)
								j=0;
							if (user->farar->hrac[j]!=NULL && user->farar->active[j]==1)
								user->farar->natahu=j;
						}
						sprintf(text,"~FTTak ti treba, teraz musis stat.\n");
						write_user(user,text);
						for (i=0;i<5;++i)
							if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1) {
								if (user->farar->natahu==i)
									sprintf(text,"~FT%s nemal%s eso, musi stat. ~OL~FYSi na rade.\n",user->name,pohl(user,"","a"));
								else
									sprintf(text,"~FT%s nemal%s eso, musi stat.\n",user->name,pohl(user,"","a"));
								write_user(user->farar->hrac[i],text);
							}
						user->farar->action--;
						if (user->farar->action==100)
							user->farar->action=0;
						return;
					}
					else {
						write_user(user,"~FTTy uz do toho radsej nekukaj, ved uz ani nevidis co je na kope. Eso to nie je!\n");
						return;
					}
				}
				else {
					if (word_count>5) {
						write_user(user,"~FTToto nie je stolny tenis! Naraz mozes vyhodit maximalne 4 karty.\n");
						return;
					}
					o=0;
					for (i=1;i<32;++i)
						if (user->farar->karty[i]>user->farar->karty[o])
							o=i;
					y=o;
					oznac=o/4;
					farba=o%4;
					pocet=0;
					for (i=0;i<32;++i)
						if (user->farar->karty[i]==user->fhrac) {
							hold[pocet]=i; pocet++;
						}
					if (
							word_count-1>pocet
							&&
							strncmp(word[word_count-1],"zel",3)
							&&
							strncmp(word[word_count-1],"cer",3)
							&&
							strncmp(word[word_count-1],"zal",3)
							&&
							strncmp(word[word_count-1],"gul",3)
					   ) {
						write_user(user,"~FTNepodvadzaj! Nemas tolko kariet.\n");
						return;
					}
					zmen=-1;
					for (i=0;i<4;++i)
						tah[i]=-1;
					for (i=0;i<(int)(word_count-1);i++) {
						if (i==(int)(word_count-2) && hold[tah[0]]/4==5) {
							if (!strncmp(word[i+1],"zel",3)) {
								zmen=0;
								word_count--;
								continue;
							}
							if (!strncmp(word[i+1],"cer",3)) {
								zmen=1;
								word_count--;
								continue;
							}
							if (!strncmp(word[i+1],"zal",3)) {
								zmen=2;
								word_count--;
								continue;
							}
							if (!strncmp(word[i+1],"gul",3)) {
								zmen=3;
								word_count--;
								continue;
							}
						}
						tah[i]=atoi(word[i+1]);
						if (tah[i]<1 || (int)pocet<tah[i]) {
							write_user(user,"~FTKartu s takym cislom nemas, musis si tam nejaku namalovat.\n");
							return;
						}
						tah[i]--;
					}
					if (0<user->farar->action && user->farar->action<100 && oznac==0 && (hold[tah[0]]/4!=0 && hold[tah[0]]!=16)) {
						write_user(user,"~FTAk nemas sedmicku alebo zeleneho dolnika, pekne si tahaj z kvopky..\n");
						return;
					}
					for (i=0;i<(int)(word_count-1);i++) {
						for (j=0;j<i;j++) {
							if (tah[j]==tah[i]) {
								write_user(user,"~FTNemooooozes^ vyhodit jednu kartu dva krat.\n");
								return;
							}
							if (hold[tah[j]]/4!=hold[tah[i]]/4) {
								write_user(user,"~FTMusis hodit karty rovnakeho typu, nemozes hadzat na kopu od buka do buka.\n");
								return;
							}
						}
						if (100<user->farar->action && user->farar->action<105 && hold[tah[i]]/4!=7) {
							write_user(user,"~FTMozes hodit len eso, ak nemas, musis stat (.faraon stoj).\n");
							return;
						}
					}
					y=0;
					if (hold[tah[0]]/4==5)
						y=1;
					if (hold[tah[0]]==16)
						y=1;
					if (oznac==4 && farba==0)
						y=1;
					if (hold[tah[0]]/4==oznac)
						y=1;
					if (user->farar->action>200) {
						if (hold[tah[0]]%4==user->farar->action-201)
							y=1;
					}
					else if (hold[tah[0]]%4==farba)
						y=1;
					if (y==0) {
						sprintf(text,"~FTNemozes hadzat na kopu co sa ti zachce, musi sediet farba alebo znak.\n");
						write_user(user,text);
						return;
					}
					for (i=0;i<(int)(word_count-1);i++) {
						user->farar->karty[hold[tah[i]]]=user->farar->karty[o]+1+i;
						if (hold[tah[i]]/4==7) {
							if (user->farar->action<100 || 105<user->farar->action)
								user->farar->action=101;
							else
								user->farar->action++;
						}
						if (hold[tah[i]]/4==0) {
							if (0==user->farar->action || 10<user->farar->action)
								user->farar->action=3;
							else
								user->farar->action+=3;
						}
						if (hold[tah[i]]==16) {
							if (0<user->farar->action && user->farar->action<10)
								user->farar->action=0;
						}
					}
					if (user->farar->action>200 && zmen==-1)
						user->farar->action=0;
					i=user->fhrac;
					j=i;
					while (i==user->farar->natahu) {
						j++;
						if (j>4)
							j=0;
						if (user->farar->hrac[j]!=NULL && user->farar->active[j]==1)
							user->farar->natahu=j;
					}
					if (word_count-1==1) {
						sprintf(text,"~FTVyhodil%s si kartu s cislom %d na kopu",pohl(user,"","a"),(tah[0]+1));
						write_user(user,text);
						sprintf(text,"~FT%s vyhodil%s kartu na kopu",user->name,pohl(user,"","a"));
					}
					else {
						sprintf(text,"~FTVyhodil%s si na kopu %d karty",pohl(user,"","a"),(word_count-1));
						write_user(user,text);
						sprintf(text,"~FT%s vyhodil%s na kopu %d karty",user->name,pohl(user,"","a"),(word_count-1));
					}
					if (zmen>-1) {
						sprintf(vtext,"~FT a zmenil%s si na %s (%s~RS~FT).\n",pohl(user,"","a"),farba_meno[zmen],karta_farba[zmen]);
						write_user(user,vtext);
						sprintf(vtext," a zmenil%s na %s (%s~RS~FT).\n",pohl(user,"","a"),farba_meno[zmen],karta_farba[zmen]);
						strcat(text,vtext);
						user->farar->action=201+zmen;
					}
					else {
						write_user(user,"~FT.\n");
						strcat(text,".\n");
					}
					for (i=0;i<5;++i)
						if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1)
							write_user(user->farar->hrac[i],text);
					if (word_count-1==4) {
						if (user->farar->action==104)
							user->farar->action=0;
						if (user->farar->action==12)
							user->farar->action=0;
						sprintf(text,"~FTMas spalenu, vyhadzujes dalej..\n");
						write_user(user,text);
						sprintf(text,"~FT%s mal%s spalenu, vyhadzuje dalej..\n",user->name,pohl(user,"","a"));
						for (i=0;i<5;++i)
							if (user->farar->hrac[i]!=NULL && i!=user->fhrac && user->farar->active[i]==1)
								write_user(user->farar->hrac[i],text);
						user->farar->natahu=user->fhrac;
					}
				}
				y=0;
				for (j=0;j<32;++j) {
					if (user->farar->karty[j]==user->fhrac) y++;
				}
				for (j=0;j<5;++j)
					if (user->farar->hrac[j]!=NULL && (y>0 || user!=user->farar->hrac[j]) && user->farar->active[j]==1)
						farar_disp(user->farar->hrac[j],-1);
				if (y==0) {
					if (user->farar->natahu==user->fhrac) {
						i=user->fhrac;
						j=i;
						while (i==user->farar->natahu) {
							j++;
							if (j>4)
								j=0;
							if (user->farar->hrac[j]!=NULL && user->farar->active[j]==1)
								user->farar->natahu=j;
						}
					}
					sprintf(text,"~FT~OLYeehaw! Zbavil%s si sa vsetkych kariet..\n",pohl(user,"","a"));
					write_user(user,text);
					sprintf(text,"~FT%s skoncil%s vo faraonovi.\n",user->name,pohl(user,"","a"));
					for (j=0;j<5;++j)
						if (user->farar->hrac[j]!=NULL && j!=user->fhrac && user->farar->hrac[j]->room!=user->room)
							write_user(user->farar->hrac[j],text);
					write_room_except(user->room,text,user);
					cnt=0;
					for (i=0;i<5;++i)
						if (user->farar->hrac[i]!=NULL && user->farar->active[i]==1)
							cnt++;
					if (cnt==2) {
						u=user->farar->hrac[user->farar->natahu];
						if ((int)(rand()%2)==0) {
							sprintf(text,"~FT~OLPrehral%s si a rozplakal%s si sa..\n",pohl(u,"","a"),pohl(u,"","a"));
							write_user(u,text);
							sprintf(text,"~FT%s prehral%s vo faraonovi a rozplakal%s sa.\n",u->name,pohl(u,"","a"),pohl(u,"","a"));
						}
						else {
							sprintf(text,"~FT~OLUh-oh! Prehral%s si rundu vo faraonovi.. trieskas si hlavu o zem.\n",pohl(u,"","a"));
							write_user(u,text);
							sprintf(text,"~FT%s prehral%s rundu vo faraonovi a trieska si hlavu o zem.\n",u->name,pohl(u,"","a"));
						}
						for (j=0;j<5;++j)
							if (u->farar->hrac[j]!=NULL && j!=u->fhrac && u->farar->hrac[j]->room!=u->room)
								write_user(u->farar->hrac[j],text);
						write_room_except(u->room,text,u);
						user->farar->natahu=-1;
						user->farar->action=u->fhrac;
						u->farar->active[u->fhrac]=0;
					}
					user->fararwin+=cnt-1;
					user->farar->active[user->fhrac]=0;
				}
				return;
			}
			else {
				write_user(user,"~FTNemozes, ved nehras faraona.\n");
				return;
			}
		}
	}
	write_user(user,"~FTfaraon otvor   otvoris hru faraona (pre 2-5 hracov)\n");
	write_user(user,"~FTfaraon <meno>  vyzves niekoho aby sa pripojil do hry\n");
	write_user(user,"~FTfaraon zacni   rozdas spoluhracom karty a zacnete hrat\n");
	write_user(user,"~FTfaraon koniec  vzdas sa a odpojis sa z hry\n");
	write_user(user,"~FTfaraon stav    zobrazi aktualny stav hry\n");
	write_user(user,"~FTfaraon stoj    ak si zastaveny a nemas eso, posuvas poradie\n");
	write_user(user,"~FTfaraon tahaj   tahas si kartu z kopky\n");
	write_user(user,"~FTfaraon povedz  <text> povies text spoluhracom (faraon say <text>)\n");
	write_user(user,"~FTfaraon rev     zobrazi posledne kecy hracov\n");
	write_user(user,"~FTfaraon <cislo> [<cislo>] [<cislo>] [<cislo>] vylozis kartu(y)\n");
	write_user(user,"~FTfaraon <cislo> <zelen | cerven | zalud | gula> zmena hornikom\n");
}

void farar_disp(UR_OBJECT user,int tahal)
{
	int cnt,i,o,farba,oznac,pos;
	unsigned int len;
	int hold[32],pocet[5],vkope;
	char num[40],kopa[5][40];

	if (user->fhrac==-1) {
		user->farar=NULL;
		return;
	}
	for (i=0;i<5;++i)
		pocet[i]=0;
	vkope=0;
	for (i=0;i<32;++i)
		if (user->farar->karty[i]>10)
			vkope++;
	for (i=0;i<32;++i)
		if (-1<user->farar->karty[i] && user->farar->karty[i]<5)
			pocet[user->farar->karty[i]]++;

	strcpy(kopa[0],".-----...");
	for (i=1;i<4;i++)
		strcpy(kopa[i],"|~OL~FK/\\/\\/~RS~FW|||");
	strcpy(kopa[4],"`-----'''");

	if (user->farar->natahu==-1) {
		for (i=0;i<5;i++) {
			write_user(user,"        ");
			write_user(user,kopa[i]);
			write_user(user,"\n");
		}
	}
	else {
		if (vkope==1)
			for (i=0;i<5;i++)
				strcpy(kopa[i],"         ");
		if (vkope==2)
			for (i=0;i<5;i++) {
				kopa[i][strlen(kopa[i])-2]=' ';
				kopa[i][strlen(kopa[i])-1]=' ';
			}
		if (vkope==3)
			for (i=0;i<5;i++)
				kopa[i][strlen(kopa[i])-1]=' ';
		o=0;
		for (i=1;i<32;++i)
			if (user->farar->karty[i]>user->farar->karty[o])
				o=i;
		pos=0;
		for (i=0;i<32;++i)
			if (user->farar->karty[i]==user->fhrac) {
				hold[pos]=i;
				pos++;
			}
		oznac=o/4;
		farba=o%4;
		if (pocet[user->fhrac]>0)
			len=60/pocet[user->fhrac];
		else
			len=0;
		if (len>8)
			len=8;
		sprintf(text,".-----. %s  ",kopa[0]);
		for (i=0;i<pos;i++) {
			sprintf(num,".-----. ");
			strncat(text,num,len);
		}
		write_user(user,text);write_user(user,"\n");
		sprintf(text,"|~OL%s~RS  %8s~RS~FW| %s  ",karta_znak[oznac],karta_farba[farba],kopa[1]);
		for (i=0;i<pos;i++) {
			sprintf(num,"|~OL%s~RS  %8s~RS~FW| ",karta_znak[hold[i]/4],karta_farba[hold[i]%4]);
			while (strlen(colour_com_strip2(num,0))>len) num[strlen(num)-1]='\0';
			strcat(text,"~RS");
			strcat(text,num);
		}
		write_user(user,text);write_user(user,"\n");
		sprintf(text,"|~FB~OL%s~RS~FW| %s  ",karta_pict[oznac],kopa[2]);
		for (i=0;i<pos;i++) {
			sprintf(num,"|~FB~OL%s~RS~FW| ",karta_pict[hold[i]/4]);
			while (strlen(colour_com_strip2(num,0))>len) num[strlen(num)-1]='\0';
			strcat(text,"~RS");
			strcat(text,num);
		}
		write_user(user,text);write_user(user,"\n");
		sprintf(text,"|%-8s~FW  ~OL%s~RS| %s  ",karta_farba[farba],karta_znak[oznac],kopa[3]);
		for (i=0;i<pos;i++) {
			sprintf(num,"|%-8s~FW  ~OL%s~RS| ",karta_farba[hold[i]%4],karta_znak[hold[i]/4]);
			while (strlen(colour_com_strip2(num,0))>len)
				num[strlen(num)-1]='\0';
			strcat(text,"~RS");
			strcat(text,num);
		}
		write_user(user,text);write_user(user,"\n");
		sprintf(text,"`-----' %s  ",kopa[4]);
		for (i=0;i<pos;i++) {
			sprintf(num,"`-----' ");
			strncat(text,num,len);
		}
		write_user(user,text);write_user(user,"\n");
		sprintf(text,"~FTKariet v kope: ~OL%-2d~RS~FT  ",vkope);
		for (i=0;i<pos;i++) {
			if (tahal==hold[i])
				strcat(text,"~OL");
			sprintf(num,"%*s%2d%*s",(len-2)/2-len/8,"",i+1,(len-2)%2+(len-2)/2+len/8,"");
			strncat(text,num,len);
			if (tahal==hold[i])
				strcat(text,"~RS~FT");
		}
		write_user(user,text);write_user(user,"\n");
	}
	if (user->farar->action>200) {
		sprintf(text,"~FTZmenene na %s (%s~RS~FT).\n",farba_meno[user->farar->action-201],karta_farba[user->farar->action-201]);
		write_user(user,text);
	}
	if (100<user->farar->action && user->farar->action<105) {
		sprintf(text,"~FTZastavenie este ~OL%d~RS~FT krat.\n",user->farar->action-101);
		write_user(user,text);
	}
	cnt=0;
	for (i=0;i<5;++i)
		if (user->farar->hrac[i]!=NULL)
			cnt++;
	if (cnt>1) {
		strcpy(text,"~FTHrac:pocet kariet>~FW ");
		o=0;
		for (i=0;i<5;++i)
			if (user->farar->hrac[i]!=NULL) {
				o++;
				if (o>1)
					strcat(text,"~FT,~FW");
				if (user->farar->active[i]==1) 	{
					if (user->farar->natahu==i)
						sprintf(num,"~FY*~OL~FY%s~RS:~OL%d~RS",user->farar->hrac[i]->name,pocet[i]);
					else
						sprintf(num," ~OL~FT%s~RS:~OL%d~RS",user->farar->hrac[i]->name,pocet[i]);
					strcat(text,num);
				}
				else
					strcat(text,user->farar->hrac[i]->name);
			}
		if (user->fhrac==user->farar->natahu)
			strcat(text,". ~OL~FYSi na rade.\n");
		else
			strcat(text,".\n");
		write_user(user,text);
	}
	else
		write_user(user,"~FTZatial s tebou nikto nehra.\n");
}
