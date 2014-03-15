#pragma once
/*
	CenaPlus::Linux::Sandbox
	* Decs: Sandbox for c++
	* Author: Yangff
									*/

/*
	简介
	====
	cgroup是一个系统级别的虚拟化方案，可以通过他限制进程组的资源使用。
	简单地说就是一个轻量级的虚拟机啦～
	因为是Linux底层直接支持的，所以效率自然是满满！
																*/

#include <boost/algorithm/string.hpp>
#include <boost/fusion/container/list.hpp>
#include <boost/fusion/include/list.hpp>
#include <boost/fusion/container/list/list_fwd.hpp>
#include <boost/fusion/include/list_fwd.hpp>
#include <pthread.h>
// working here
namespace CenaPlus{
	namespace Linux{
		class Sandbox {
			const int MaxRuningTime = 20000; // 20s
		public:
			Sandbox(const std::string cmd, const std::list<std::string> args);
			// Behavior Control
			int Start();
			// Resource Control
			void SetTimeLimit(unsigned int millisecond);
			void SetMemoryLimit(size_t bytes);
			void SetStandardInput(const std::string input);
			void SetStandardOutput(const std::string output);
			void SetStandardErrput(const std::string errput);
			void SetStackLimit(size_t bytes);

			void SetCPUCore(int num); // unsupported
			void SetOutputLimit(size_t size);
			// Exit Code
			int GetExitCode();

		public:
			enum RunState{
				Nothing, Runing, Done
			};
			RunState RunState;

			int Exitcode;

		public:
			struct SandboxSettings{
				unsigned int TimeLimit; bool doTimeLimit;
				size_t MemoryLimit; bool doMemoryLimit;
				size_t StackLimit; bool doStackLimit;
				size_t OutputLimit; bool doOutputLimit;
				int CPUCore; // unsupported
				std::string StandardInput, StandardOutput, StandardErrput;
				std::list<std::string> Args; std::string cmd;

				int sockets[2]; // for async timer.
			};

			int pid; pthread_mutex_t mutex;
		private:
			SandboxSettings m_Settings;
		}; // class
	}; // namespace
}; // namespace
