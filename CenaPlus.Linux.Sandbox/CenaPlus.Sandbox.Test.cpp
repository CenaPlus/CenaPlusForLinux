#include "CenaPlus.Sandbox.h"
#include <cstdio>
int main(){
	// TLE
	printf("Test TLE1\n");

	CenaPlus::Linux::Sandbox sandbox1("./TLE1", {"TLE1"});
	sandbox1.SetTimeLimit(1200);
	sandbox1.Start();

	printf("Time Used: %d\n", sandbox1.GetReport().Runtime);
	printf("ReturnValue: %d\n", sandbox1.GetReport().ExitCode);
	printf("Signal: %d\n", sandbox1.GetReport().Signal);
	printf("Test TLE2\n");

	CenaPlus::Linux::Sandbox sandbox2("./TLE2", {"TLE2"});
	sandbox2.SetTimeLimit(1200);
	if (sandbox2.Start()){
		printf("TLE2 Failed!");
		return 0;
	};;
	printf("Time Used: %d\n", sandbox2.GetReport().Runtime);
	printf("ReturnValue: %d\n", sandbox2.GetReport().ExitCode);
	printf("Signal: %d\n", sandbox2.GetReport().Signal);
	// MLE
	printf("Test MLE\n");

	CenaPlus::Linux::Sandbox sandbox3("./MLE", {"MLE"});
	sandbox3.SetMemoryLimit(100000000);
	if (sandbox3.Start()){
		printf("MLE Failed!");
		return 0;
	};;
	printf("Memery Used: %d\n", sandbox3.GetReport().MemoryUsed);
	printf("ReturnValue: %d\n", sandbox3.GetReport().ExitCode);
	printf("Signal: %d\n", sandbox3.GetReport().Signal);

	// Return Value
	printf("Return Value\n");

	CenaPlus::Linux::Sandbox sandbox4("./ReturnValue", {"ReturnValue"});
	if (sandbox4.Start()){
		printf("ret Failed!\n");
		return 0;
	};
	printf("ReturnValue: %d\n", sandbox4.GetReport().ExitCode);
	printf("Signal: %d\n", sandbox4.GetReport().Signal);
	//surprise
	printf("surprise\n");
	FILE *f = fopen("surprise.txt", "w");
	fprintf(f, "aaa");
	fclose(f);
	CenaPlus::Linux::Sandbox sandbox5("./Danger", {"Danger"});
	if (sandbox5.Start()){
		printf("surprise Failed!\n");
		return 0;
	};
	printf("ReturnValue: %d\n", sandbox5.GetReport().ExitCode);
	printf("Signal: %d\n", sandbox5.GetReport().Signal);
//	for (;;);
	return 0;

}