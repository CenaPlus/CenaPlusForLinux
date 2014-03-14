#pragma once
/*
	CenaPlus::Linux::Cgroup
	* Decs: Cgroup for c++
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
namespace CenaPlus{
	namespace Linux{
		class Cgroup {
		public:
			Cgroup(const std::string cmd);
			// Behavior Control
			int Kill();
			int Start();
			// Resource Control
			void SetTimeLimit(unsigned int millisecond);
			void SetMemoryLimit(unsigned long long bytes)
			void SetStandardInput(const std::string input);
			void SetStandardOutput(const std::string output);
			void SetStandardError(const std::string output);
			void SetNetwork(bool flag);
			void SetStackLimit(unsigned int bytes);
			void SetLimitDevices(bool flag);
			void SetCPUCore(int num);
			// Exit Code
			int GetExitCode();

		private:
			bool Set(const std::string &subsystem, const std::string &property, const std::string &value);
			bool Get(const std::string &subsystem, const std::string &property, const std::string &value);

			enum RunState{
				Nothing, Runing, Frozen, Killed
			};
			RunState m_RunState;

			struct CgroupStartup{
				int sockets[2];
				int StandardInput, StandardOutput;
			} m_Startup;

			struct CgroupSettings{
				CgroupStartup startup;
				int TimeLimit;
				int MemoryLimit;
				int StackLimit;
				int CPUCore;
				bool LimitDevices;
				bool Network;
			} m_Settings;


		}; // class
	}; // namespace
}; // namespace
