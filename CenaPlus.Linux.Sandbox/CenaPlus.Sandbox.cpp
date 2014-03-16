#include "CenaPlus.Sandbox.h"
#include <sys/resource.h> // setrlimit
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <boost/foreach.hpp>
using namespace std;
using namespace boost;
using namespace CenaPlus;
using namespace CenaPlus::Linux;

// working here

namespace CenaPlus{
	namespace Linux{
		// invke functions
		static int sandbox_main(Sandbox::SandboxSettings *args){
			// setup environment
			/*
				TODO:
					Prepare STDIO by freopen
					Setup seccomp (prctl(PR_SET_SECCOMP, 2, filters))
					Exec Target Code

			*/

			// Set Limit
			#define ERROR {}
			#define SetRLimit(name, lmt) {rlimit limit; limit.rlim_cur = limit.rlim_max = (lmt); setrlimit(name, &limit);}
			if (args->doTimeLimit){
				SetRLimit(RLIMIT_CPU, (args->TimeLimit/1000) + 1); // some function will not take cpu time!
			} else
				SetRLimit(RLIMIT_CPU, (Sandbox::MaxRuningTime/1000) + 1);
			if (args->doStackLimit)
				SetRLimit(RLIMIT_STACK, args->StackLimit);
			if (args->doMemoryLimit)
				SetRLimit(RLIMIT_AS, args->MemoryLimit + 1);
			if (args->doOutputLimit)
				SetRLimit(RLIMIT_FSIZE, args->OutputLimit);
			// fprintf(stderr, "Setup RLimit.");
			// open file
			// fprintf(stderr, "%d\n", args->StandardInput.length());
			if (args->StandardInput.length() && !freopen(args->StandardInput.c_str(), "r", stdin))
				return 0;
			if (args->StandardOutput.length() && !freopen(args->StandardOutput.c_str(), "w", stdout))
				return 0;
			if (args->StandardErrput.length() && !freopen(args->StandardErrput.c_str(), "w", stderr))
				return 0;

			// Set seccomp
/*
			const char filters[] =
				"sys_exit: 1\n"
				"exit_group: 1\n"
				"sys_open: 1\n"
				"sys_read: 1\n"
				"sys_write: (fd == 1) || (fd == 2)\n"
				"sys_close: 1\n"
				"sys_creat: 1\n"
				"sys_stat: 1\n"
				"sys_lseek: 1\n"
				"sys_fstat: 1\n"
				"sys_access: 1\n"
				"sys_sync: 1\n"
				"sys_brk: 1\n"
				"sys_ioctl: 1\n"
				"sys_fcntl: 1\n"
				"sys_olduname: 1\n"
				"sys_getrlimit: 1\n"
				"old_select: 1\n"
				"sys_lstat: 1\n"
				"sys_readlink: 1\n"
				"old_readdir: 1\n"
				"old_mmap: 1\n"
				"sys_munmap: 1\n"
				"sys_uname: 1\n"
				"sys_idle: 1\n"
				"sys_vm: 1\n"
				"sys_newuname: 1\n"
				"sys_mprotect: 1\n"
				"sys_rt_sigaction: 1\n"
				"on_next_syscall: 1";*/

			if (prctl(PR_SET_SECCOMP, 2, filters) == -1){
				return -1;
			};
			char opt[2] = {'O','\0'};

			// send prepared sign

			write(args->sockets[0], opt, sizeof(opt));  // 1
			// waiting timer
			read(args->sockets[0], opt, sizeof(opt));   // 2
			if (opt[0] == 'S'){
				char ** pass_args = new char*[args->Args.size() + 1];
				int cnt = 0;
				BOOST_FOREACH(std:: string arg, args->Args){
					pass_args[cnt] = new char[arg.length() + 1];
					strcpy(pass_args[cnt], arg.c_str()); cnt++;
				}
				execvp(args->cmd.c_str(), NULL);  // 3
			}
			// socket send "O"
			// waiting socket send "S"
			// if not, return
			// execv

			return -1;
		};

		static void TimerTick(sigval v){
			Sandbox * _this = static_cast<Sandbox*>(v.sival_ptr);

			if (pthread_mutex_lock(&_this->mutex) == 0){
				// fprintf(stderr, "Tick!");
				if (_this->RunState == Sandbox::Runing){
					kill(_this->pid, SIGKILL);
				}
				pthread_mutex_unlock(&_this->mutex);
			}
		};
		static Sandbox::RunReport report;
		static void SignalHandle(int sign){
			if (sign == SIGCHLD){
				rusage usage;
				getrusage(RUSAGE_CHILDREN, &usage);
				report.MemoryUsed = usage.ru_maxrss;
			}
		};
	};
};


// Class Sandbox

Sandbox::Sandbox(const std::string cmd, const std::list<std::string> args){
	m_Settings.Args = args;
	m_Settings.cmd = cmd;
	RunState = Nothing;
	if (!socketpair(AF_LOCAL, SOCK_STREAM, 0, m_Settings.sockets)){
		fcntl(m_Settings.sockets[0], F_SETFD, FD_CLOEXEC);
		fcntl(m_Settings.sockets[1], F_SETFD, FD_CLOEXEC);
	}
	m_Settings.StandardInput = ""; m_Settings.StandardOutput = ""; m_Settings.StandardErrput = "";
};

int Sandbox::Start(){
	pid = 0;

	if ((pid = fork()) == 0) {
		sandbox_main(&m_Settings);
		char opt[20]; opt[0] = 'E'; opt[1] = '\0';
		write(m_Settings.sockets[0], opt, sizeof(opt));
		exit(-1);
	}
	if (pid < 0){
		return 1; // failed to create child process
	}
	memset(&report, 0, sizeof(report));
	timer_t timer;
	if (m_Settings.doTimeLimit)	{
		sigevent evp;
		memset(&evp, 0, sizeof(evp));
		evp.sigev_value.sival_ptr = this;
		evp.sigev_notify = SIGEV_THREAD;
		evp.sigev_notify_function = TimerTick;
		if (timer_create(CLOCK_REALTIME, &evp, &timer)){
			return 1;
		}
		pthread_mutex_init(&mutex, NULL);
	}

	// SIGCHLD

	signal(SIGCHLD, SignalHandle);

	char buf[2];
	// waiting for child
	read(m_Settings.sockets[1], buf, sizeof(buf));

	if (buf[0] != 'O'){
		close(m_Settings.sockets[1]);
		return 1;
	}

	buf[0] = 'S'; buf[1] = '\0';

	// start timer
	if (m_Settings.doTimeLimit)	{
		itimerspec it;
		memset(&it, 0, sizeof(it));
		it.it_value.tv_nsec = (m_Settings.TimeLimit % 1000) * 1000000;
		// printf("%d\n", it.it_value.tv_nsec);
		it.it_value.tv_sec = m_Settings.TimeLimit / 1000;
		timer_settime(timer,0 , &it, NULL);
	}
	timeval StartTime;
	gettimeofday(&StartTime, NULL);
	// send start sign
	RunState = Runing;
	write(m_Settings.sockets[1], buf, sizeof(buf));
	int status = 0;
	wait(&status);
	timeval EndTime;
	gettimeofday(&EndTime, NULL);
	#define GetMs(time) (((long long)time.tv_usec / 1000LL + (long long)time.tv_sec * 1000LL))
	report.Runtime = GetMs(EndTime) - GetMs(StartTime);
	timer_delete(timer);
	if (m_Settings.doTimeLimit){
		pthread_mutex_lock(&mutex);
		RunState = Done;
		pthread_mutex_destroy(&mutex);
	} else RunState = Done;
	report.ExitCode = WEXITSTATUS(status);
	report.Signal = WSTOPSIG(status);
	return 0;
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
	m_Settings.StandardInput = input;
};

void Sandbox::SetStandardOutput(const std::string output){
	m_Settings.StandardOutput = output;
};

void Sandbox::SetStandardErrput(const std::string errput){
	m_Settings.StandardErrput = errput;
};

void Sandbox::SetStackLimit(size_t bytes){
	m_Settings.StackLimit = bytes;
	m_Settings.doStackLimit = true;
};

void Sandbox::SetOutputLimit(size_t size){
	m_Settings.OutputLimit = size;
	m_Settings.doOutputLimit = true;
};

void Sandbox::SetCPUCore(int num){
	m_Settings.CPUCore = num;
};

Sandbox::RunReport Sandbox::GetReport(){
	return report;
};
