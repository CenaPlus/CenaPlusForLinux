#include <cstdio>
#include <unistd.h>
int main(){
	unlink("surprise.txt");
	FILE *f1 = fopen("hi.txt", "w");
	fprintf(f1, "I'm joking!");
	return 0;
}