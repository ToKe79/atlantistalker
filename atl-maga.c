#include "atl-head.h"
#include "atl-mydb.h"
#include "atl-maga.h"
#define ISSUEFILE "issues/list.txt"

extern void magazin(UR_OBJECT user, char *inpstr)
{
	int issue,err;
	FILE *subor;
	char filename[1024];

	if (word_count < 2) {
		write_user(user,"Pouzitie: .magazin <list|cislo vydania> [obsah|cislo clanku|hladany retazec]\n");
		write_user(user," Priklad: .magazin 12      - zobrazi 12. cislo casopisu\n");
		write_user(user,"          .magazin 3 obsah - zobrazi obsah 3. cisla\n");
		write_user(user,"          .magazin list    - zoznam vydani\n");
		return;
	}
	sprintf(filename, "%s%c%s%s",TMPFOLDER,DIRSEP,user->name,TMPSUFFIX);
	if ((subor=ropen(filename,"w"))==NULL) { /*APPROVED*/
		write_user(user,"~OL~FRNepodarilo sa zapisat docasny subor pre citanie casopisu!~RS~FW\n");
		return;
	}
	if (!strcmp(word[1],"list")) {
		fclose(subor);
		switch(more(user,user->socket,ISSUEFILE)) {
			case 0: write_user(user,"~FRChyba pri citani!~FW\n"); break;
			case 1: user->misc_op=2;
		}
		return;
	}
	issue=atoi(word[1]);
	if (word_count<3)
		inpstr=NULL;
	else
		inpstr=remove_first(inpstr);
	err=parse_ezin(subor,issue,inpstr);
	if (err == 0)
		fprintf(subor,"\n\n                ~BB~FW~OL   Dakujeme vam, ze citate Stratene dialogy!   ~BK~FW~RS\n");
	fclose(subor);
	switch(more(user,user->socket,filename)) {
		case 0:
			write_user(user,"~FRChyba pri citani suboru!~FW\n");
			break;
		case 1:
			user->misc_op=2;
	}
}
