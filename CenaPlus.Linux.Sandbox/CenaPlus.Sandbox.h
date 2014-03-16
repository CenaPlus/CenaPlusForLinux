#pragma once
/*
	CenaPlus::Linux::Sandbox
	* Decs: Sandbox for c++
	* Author: Yangff
									*/
// 我放弃cgroup了！

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
		public:
			static const int MaxRuningTime = 20000; // 20s
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

		public:
			enum RunState{
				Nothing, Runing, Done
			};
			RunState RunState;

		public:
			struct SandboxSettings{
				unsigned int TimeLimit = 0; bool doTimeLimit = 0;
				size_t MemoryLimit = 0; bool doMemoryLimit = 0;
				size_t StackLimit = 0; bool doStackLimit = 0;
				size_t OutputLimit = 0; bool doOutputLimit = 0;
				int CPUCore = 0; // unsupported
				std::string StandardInput, StandardOutput, StandardErrput;
				std::list<std::string> Args; std::string cmd;

				int sockets[2]; // for async timer.
			};

			int pid; pthread_mutex_t mutex;

			struct RunReport {
				int Runtime;
				int MemoryUsed;
				int ExitCode;
				int Signal;
			};

			RunReport GetReport();
		private:
			SandboxSettings m_Settings;
		}; // class
	}; // namespace
}; // namespace
