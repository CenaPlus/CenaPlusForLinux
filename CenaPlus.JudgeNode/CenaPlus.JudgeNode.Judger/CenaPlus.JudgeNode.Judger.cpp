#include <CenaPlus.Sandbox.h>

/*
	argv[1]: filename and arguments.
	argv[2]: input file path.
	argv[3]: output file path.
	argv[4]: errput file path.
	argv[5]: time limit(ms).
	argv[6]: memory limit(KB).
	argv[7]: high priority run time(ms). [markd as unused]
	argv[8]: APIHook dll path. [markd as unused]
	argv[9]: XML create path.
*/
int main(int argc, const char* argv[]){
	const int nSuccess = 0;
	const int nSystemError = 1;
	const int nArgcError = 2;
	if (argc != 10){
		return nArgcError;
	}
	CenaPlus::Linux::Cgroup *cgroup = new CenaPlus::Linux::Cgroup(std::string(argv[1]));
	cgroup->SetTimeLimit(atoi(argv[5]));
	cgroup->SetMemoryLimit(atoi(argv[6]) * 1048576LL);
	cgroup->SetStandardInput(std::(argv[2]));
	cgroup->SetStandardOutput(std::(argv[3]));
	cgroup->SetStandardErrput(std::(argv[4]));

}