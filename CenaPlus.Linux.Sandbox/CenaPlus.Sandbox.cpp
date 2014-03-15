#include "CenaPlus.Sandbox.h"
#include <sys/resource.h> // setrlimit
#include <sys/time.h>
#include <sys/wait.h>
#include <cstring>
using namespace std;
using namespace boost;
using namespace CenaPlus;
using namespace CenaPlus::Linux;

// working here

namespace CenaPlus{
	namespace Linux{
		// invke functions
		static int sandbox_main(SandboxSettings *args){
			// setup environment
			/*
				TODO:
					Prepare STDIO by freopen
					Setup seccomp (prctl(PR_SET_SECCOMP, 2, filters))
					Exec Target Code

			*/

			// Set Limit
			#define SetRLimit(name, lmt) {rlimit limit; limit.rlim_cur = limit.rlim_max = (lmt); setrlimit(name, &limit);}
			if (args->doTimeLimit)
				SetRLimit(RLIMIT_CPU, (args->TimeLimit/1000) + 1); // some function will not take cpu time!
			else
				SetRLimit(RLIMIT_CPU, (Sandbox::MaxRuningTime/1000) + 1);
			if (args->doStackLimit)
				SetRLimit(RLIMIT_STACK, args->StackLimit);
			if (args->doMemoryLimit)
				SetRLimit(RLIMIT_DATA, args->MemoryLimit);
			if (args->doOutputLimit)
				SetRLimit(RLIMIT_FSIZE, args->OutputLimit);

			// open file

			freopen(args->StandardInput.c_str(), "r", stdin);
			freopen(args->StandardOutput.c_str(), "w", stdout);
			freopen(args->StandardErrput.c_str(), "w", stderr);

			// Set seccomp

			// execv
		};

		static void TimerTick(sigval v){
			Sandbox * _this = static_cast<Sandbox*>(v.sival_ptr);
			if (pthread_mutex_lock(&_this->mutex) == 0){
				if (_this->RunState == Runing){
					kill(_this->pid, SIGKILL);
				}
			}
		};
	};
};


// Class Sandbox

Sandbox::Sandbox(const std::string cmd, const std::list<std::string> args){
	m_Settings.Args = args;
	m_Settings.cmd = cmd;
	RunState = Nothing; Exitcode = 0; memset(m_Settings, 0, sizeof(m_Settings));
};

int Sandbox::Start(){
	pid = 0;
	if ((pid = fork()) == 0) {
		sandbox_main(&m_Settings);
		exit(0);
	}
	if (pid < 0){
		return pid; // failed to create child process
	}
	if (m_Settings.doTimeLimit)	{
		sigevent evp;
		memset(&evp, 0, sizeof(evp));
		evp.sigev_value.sival_ptr = this;
		evp.sigev_notify = SIGEV_THREAD;
		pthread_mutex_init(&mutex, NULL);

	}


	// TODO: Async Socket for Start Timer.

	if (m_Settings.doTimeLimit){
		pthread_mutex_lock(&mutex);
		RunState = Done;
		timer_delete()
		pthread_mutex_unlock(&mutex)
	}
};

void Sandbox::SetTimeLimit(unsigned int millisecond){
	m_Settings.TimeLimit = millisecond;
	m_Settings.doTimeLimit = true;
};

void Sandbox::SetMemoryLimit(size_t bytes){
	m_Settings.MemoryLimit = bytes;
	m_Settings.doMemoryLimit = true;
};

void Sandbox::SetStandardInput(const std::string input){
	m_Settings.SetStandardInput = input;
};

void Sandbox::SetStandardOutput(const std::string output){
	m_Settings.SetStandardOutput = output;
};

void Sandbox::SetStandardErrput(const std::string errput){
	m_Settings.StandardErrput = errput;
};

void Sandbox::SetStackLimit(size_t bytes){
	m_Settings.SetStackLimit = bytes;
	m_Settings.doStackLimit = true;
};

void Sandbox::SetOutputLimit(size_t size){
	m_Settings.OutputLimit = size;
	m_Settings.doOutputLimit = true;
};

void Sandbox::SetCPUCore(int num){
	m_Settings.CPUCore = num;
};

int Sandbox::GetExitCode(){
	return Exitcode;
};
