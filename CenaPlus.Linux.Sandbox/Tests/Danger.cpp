#include <cstdio>
#include <unistd.h>
int main(){
	unlink("surprise.txt");
	FILE *f1 = fopen("surprise.txt", "w");
	fprintf(f1, "I'm joking!");
	fclose(f1);
	return 0;
}
