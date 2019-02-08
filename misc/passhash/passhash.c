#include "../../atl-cryp.c"

char *expand_pass(char *pass) {
	static char out[20];
	sprintf(out,"&%-10s*2=h",pass);
	return out;
}

int main(int argc,char *argv[]) {
	if (argc!=3) {
		printf ("Usage: %s <user> <pass>\n",argv[0]);
		printf ("Returns hash of <pass> for <user>\n");
		return 0;
	}
	printf("%s",(char *)md5_crypt(expand_pass(argv[2]),argv[1]));
	return 0;
}
