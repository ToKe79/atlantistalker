#include "../../atl-cryp.c"

char *expand_pass(char *pass) {
	static char out[20];
	sprintf(out,"&%-10s*2=h",pass);
	return out;
}

int main(int argc,char *argv[]) {
	int i;
	if (argc!=3) {
		printf ("argc=%d\n",argc);
		for(i=0;i<argc;i++) {
			printf("argv[%d]=%s\n",i,argv[i]);
		}
		printf ("Usage: %s <user> <pass>\n",argv[0]);
		return 1;
	}
	printf("%s",(char *)md5_crypt(expand_pass(argv[2]),argv[1]));
	return 0;
}
