#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

using namespace std;
int t[10000000 / 4 * 3];
int main(){
	for (int i = 0; i < 10000000; i++)
		t[i] = i;
	printf("%d\n", sizeof(t));
	for (;;) {
		int *x = new int[10000];
		*x = 9;
	}
	return 0;
}
