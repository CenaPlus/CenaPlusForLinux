#include "CenaPlus.Sandbox.h"
#include <sys/resource.h> // setrlimit
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/prctl.h>
#include <seccomp.h>
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
			// if (args->doMemoryLimit)
			// 	SetRLimit(RLIMIT_AS, args->MemoryLimit + 1);
			if (args->doOutputLimit)
				SetRLimit(RLIMIT_FSIZE, args->OutputLimit);
			// // fprintf(stderr, "Setup RLimit.");
			// open file
			// // fprintf(stderr, "%d\n", args->StandardInput.length());
			if (args->StandardInput.length() && !freopen(args->StandardInput.c_str(), "r", stdin))
				return 0;
			if (args->StandardOutput.length() && !freopen(args->StandardOutput.c_str(), "w", stdout))
				return 0;
			if (args->StandardErrput.length() && !freopen(args->StandardErrput.c_str(), "w", stderr))
				return 0;

			// Set seccomp
			scmp_filter_ctx ctx;

			if ((ctx = seccomp_init(SCMP_ACT_ERRNO(5))) == 0)
				return 0;
			//if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 1,
 			//      SCMP_A0(SCMP_CMP_EQ, STDIN_FILENO)) < 0)
			//	return 0;
			//if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 1,
 			//      SCMP_A0(SCMP_CMP_EQ, args->sockets[0])) < 0)
			//	return 0;
			if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
 			      SCMP_A0(SCMP_CMP_EQ, args->sockets[0])) < 0)
				return 0;
			if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
 			      SCMP_A0(SCMP_CMP_EQ, STDOUT_FILENO)) < 0)
				return 0;
			if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
 			      SCMP_A0(SCMP_CMP_EQ, STDERR_FILENO)) < 0)
				return 0;
			// fprintf(stderr, "b");
			int priority = 255;

			#define makeAllow(x) { if (seccomp_syscall_priority(ctx, SCMP_SYS(x), priority--)<0){return 0;};if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(x), 0) < 0) return 0; }
			//makeAllow(open);
			if (seccomp_rule_add_exact(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
 			      SCMP_A1(SCMP_CMP_MASKED_EQ, O_WRONLY, 0)) < 0)
				return 0;
			//if (seccomp_rule_add_exact(ctx, SCMP_ACT_ERRNO(5), SCMP_SYS(open), 1,
 			  //    SCMP_A1(SCMP_CMP_MASKED_EQ, O_CREAT, 1)) < 0)
				//return 0;

			makeAllow(read);
			//makeAllow(write);
			makeAllow(close);
			// brk mmap access open fstat close read fstat mprotect arch_prctl munmap write!!

			makeAllow(exit);
			makeAllow(fstat64);
			makeAllow(stat64);
			makeAllow(getdents);
			makeAllow(getdents64);
			makeAllow(mmap2);
			makeAllow(mremap);
			makeAllow(gettimeofday);
			makeAllow(time);
			makeAllow(exit_group);
			makeAllow(creat);
			makeAllow(stat);
			makeAllow(lseek);
			makeAllow(fstat);
			makeAllow(access);
			makeAllow(sync);
			makeAllow(brk);
			makeAllow(ioctl);
			makeAllow(fcntl);
			makeAllow(olduname);
			makeAllow(getrlimit);
			makeAllow(select);
			makeAllow(lstat);
			makeAllow(readlink);
			makeAllow(readdir);
			makeAllow(mmap);
			makeAllow(munmap);
			makeAllow(uname);
			makeAllow(idle);
			makeAllow(mprotect);
			makeAllow(rt_sigaction);
			makeAllow(uselib);
			makeAllow(execve);
			makeAllow(ptrace);
			makeAllow(arch_prctl);
			makeAllow(prctl);
			//if (seccomp_rule_add(SCMP_ACT_TRAP, SCMP_SYS(execve), 0) < 0)
			//	return 0;
			if (seccomp_load(ctx) != 0)
				return 0;
			// seccomp_release(ctx);
			// fprintf(stderr, "c");
			char opt[2] = {'O','\0'};


			// send prepared sign

			write(args->sockets[0], opt, sizeof(opt));  // 1
			// waiting timer
			read(args->sockets[0], opt, sizeof(opt));   // 2
			// fprintf(stderr, "d");
			if (opt[0] == 'S'){
				char ** pass_args = new char*[args->Args.size() + 1];
				int cnt = 0;
				BOOST_FOREACH(std:: string arg, args->Args){
					pass_args[cnt] = new char[arg.length() + 1];
					strcpy(pass_args[cnt], arg.c_str()); cnt++;
				}
				pass_args[cnt] = 0;
				// ptrace(PTRACE_TRACEME, 0, NULL, NULL); next feature?
			//	fprintf(stderr, "e");
				execvp(args->cmd.c_str(), pass_args);  // 3
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
				// // fprintf(stderr, "Tick!");
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
	// fprintf(stderr, "X");
	if ((pid = fork()) == 0) {
		// fprintf(stderr, "S");
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
	if (m_Settings.doTimeLimit)
		timer_delete(timer);
	if (m_Settings.doTimeLimit){
		pthread_mutex_lock(&mutex);
		RunState = Done;
		pthread_mutex_destroy(&mutex);
	} else RunState = Done;
	if (WIFEXITED(status)){
		report.ExitCode = WEXITSTATUS(status);
		report.Signal = 0;
	} else if (WIFSIGNALED(status)) {
		report.Signal = WTERMSIG(status);
		report.ExitCode = -1;
	}

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
