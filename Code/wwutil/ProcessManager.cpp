#include "ProcessManager.h"

#if defined(OPENW3D_WIN32)
#include "wwstring.h"
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_timer.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

#include "debug.h"

Process::~Process()
{
	if (mHandle) {
#if defined(OPENW3D_WIN32)
		CloseHandle(mHandle);
#elif defined(OPENW3D_SDL3)
		SDL_DestroyProcess(mHandle);
#else
		// POSIX: reap child if not already waited
		int status;
		waitpid(mHandle, &status, WNOHANG);
#endif
	}
}

bool Process::Wait(bool block)
{
	if (!mHandle) {
		return true;
	}
#if defined(OPENW3D_WIN32)
	DWORD win32_result;

	win32_result = WaitForSingleObject(mHandle, block ? INFINITE : 0);

	if (win32_result == WAIT_OBJECT_0) {
		bool result;
		DWORD return_code;
		if (GetExitCodeProcess(mHandle, &return_code)) {
			mReturnCode = return_code;
			result = true;
		} else {
			result = false;
		}
		CloseHandle(mHandle);
		mHandle = nullptr;
		return result;
	} else if (win32_result == WAIT_FAILED) {
		return false;
	} else {
		return false;
	}
#elif defined(OPENW3D_SDL3)
	int exit_code;
	while (!SDL_WaitProcess(mHandle, block, &exit_code)) {
		if (!block) {
			return false;
		}
		SDL_Delay(100);
	}
	SDL_DestroyProcess(mHandle);
	mReturnCode = exit_code;
	mHandle = nullptr;
	return true;
#else
	// POSIX
	int status;
	pid_t result = waitpid(mHandle, &status, block ? 0 : WNOHANG);
	if (result == -1) {
		return false;
	}
	if (result == 0 && !block) {
		return false; // still running
	}
	if (WIFEXITED(status)) {
		mReturnCode = WEXITSTATUS(status);
	} else {
		mReturnCode = -1;
	}
	mHandle = 0;
	return true;
#endif
}

bool Process::Kill()
{
	if (!mHandle) {
		return true;
	}

#if defined(OPENW3D_WIN32)
	if (!TerminateProcess(mHandle, 1)) {
		return false;
	}
	CloseHandle(mHandle);
	mHandle = nullptr;
	mReturnCode = -1;
	return true;
#elif defined(OPENW3D_SDL3)
	if (!SDL_KillProcess(mHandle, true)) {
		return false;
	}
	SDL_DestroyProcess(mHandle);
	mHandle = nullptr;
	mReturnCode = -1;
	return true;
#else
	// POSIX
	if (kill(mHandle, SIGTERM) == -1) {
		return false;
	}
	mReturnCode = -1;
	mHandle = 0;
	return true;
#endif
}

Process *ProcessManager::Create_Process(const char * const *args)
{
	if (!args || !args[0] || !args[0][0]) {
		return nullptr;
	}
#if defined(OPENW3D_WIN32)
	StringClass command_line;
	for (size_t i = 0; args[i]; i++) {
		bool escape_arg = strchr(args[i], ' ') != nullptr;
		if (escape_arg) {
			command_line += "\"";
		}
		command_line += args[i];
		if (escape_arg) {
			command_line += "\"";
		}
		if (args[i + 1]) {
			command_line += " ";
		}
	}
	STARTUPINFOA startup_info = { 0 };
	startup_info.cb = sizeof(startup_info);
	PROCESS_INFORMATION process_information;
	if (!CreateProcessA(args[0], command_line.Peek_Buffer(), NULL, NULL, false, 0, nullptr, nullptr, &startup_info, &process_information)) {
		return nullptr;
	}
	CloseHandle(process_information.hThread);
	return new Process(process_information.hProcess, process_information.dwProcessId);
#elif defined(OPENW3D_SDL3)
	SDL_Process *sdl_process = SDL_CreateProcess(args, false);
	if (!sdl_process) {
		return nullptr;
	}
	auto props = SDL_GetProcessProperties(sdl_process);
	auto pid = SDL_GetNumberProperty(props, SDL_PROP_PROCESS_PID_NUMBER, -1);
	return new Process(sdl_process, pid);
#else
	// POSIX
	pid_t pid = fork();
	if (pid == -1) {
		return nullptr;
	}
	if (pid == 0) {
		// Child process
		execvp(args[0], const_cast<char * const *>(args));
		_exit(127); // exec failed
	}
	return new Process(pid, pid);
#endif
}
