/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "thread.h"
#include "Except.h"
#include "wwdebug.h"

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_timer.h>
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
#include <sched.h>
#include <time.h>
#endif

#include <cassert>


ThreadClass::ThreadClass(const char *thread_name) :
#if defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	mHandle(0),
#else
	mHandle(nullptr),
#endif
	mRunning(false)
{
	if (thread_name) {
		assert(strlen(thread_name) < sizeof(ThreadName) - 1);
		strcpy(ThreadName, thread_name);
	} else {
		strcpy(ThreadName, "No name");
	}
}

ThreadClass::~ThreadClass()
{
	Stop();
}

ThreadClass::InternalThreadFunctionReturnType INTERNAL_THREAD_FUNCTION_CALL_CONVENTION ThreadClass::Internal_Thread_Function(void *params)
{
	ThreadClass *tc = static_cast<ThreadClass *>(params);
	tc->mRunning = true;

	tc->mThreadID = Get_Current_Thread_ID();
#if defined(OPENW3D_WIN32)
#elif defined(OPENW3D_SDL3)
	SDL_ThreadPriority sdl_thread_priority;
	if (tc->mThread_priority <= -1) {
		sdl_thread_priority = SDL_THREAD_PRIORITY_LOW;
	} else if (tc->mThread_priority <= 1) {
		sdl_thread_priority = SDL_THREAD_PRIORITY_NORMAL;
	} else if (tc->mThread_priority <= 3) {
		sdl_thread_priority = SDL_THREAD_PRIORITY_HIGH;
	} else {
		sdl_thread_priority = SDL_THREAD_PRIORITY_TIME_CRITICAL;
	}
	SDL_SetCurrentThreadPriority(sdl_thread_priority);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	// Priority changes are best-effort and usually require privileges on POSIX.
#else
	assert(0);
#endif

	Register_Thread_ID(tc->mThreadID, tc->ThreadName);

#ifdef _MSC_VER
	__try {
		tc->Thread_Function();
	} __except(Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {
	}
#else
	tc->Thread_Function();
#endif

	Unregister_Thread_ID(tc->mThreadID, tc->ThreadName);
	tc->mThreadID = 0;
	tc->mRunning = false;
	return 0;
}

void ThreadClass::Execute()
{
	WWASSERT(!mHandle);	// Only one thread at a time!

#if defined(OPENW3D_WIN32)
	DWORD threadId;
	mHandle = CreateThread(NULL, 0, Internal_Thread_Function, this, CREATE_SUSPENDED, &threadId);
	mThreadID = threadId;
	SetThreadPriority(mHandle, THREAD_PRIORITY_NORMAL + mThread_priority);
	wchar_t wideThreadName[64];
	std::mbstowcs(wideThreadName, ThreadName, sizeof(wideThreadName));
	wideThreadName[ARRAYSIZE(wideThreadName) - 1] = L'\0';
	SetThreadDescription(mHandle, wideThreadName);
	ResumeThread(mHandle);
#elif defined(OPENW3D_SDL3)
	mHandle = SDL_CreateThread(Internal_Thread_Function, ThreadName, this);

	mThreadID = SDL_GetThreadID(mHandle);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	if (pthread_create(&mHandle, nullptr, Internal_Thread_Function, this) != 0) {
		mHandle = 0;
		return;
	}
	mThreadID = static_cast<unsigned>(reinterpret_cast<uintptr_t>(mHandle));
#else
	assert(0);
#endif
	WWDEBUG_SAY(("ThreadClass::Execute: Started thread \"%s\", thread ID is %X\n", ThreadName, mThreadID));
}

void ThreadClass::Set_Priority(int priority)
{
	mThread_priority = priority;
	if (mHandle) {
#if defined(OPENW3D_WIN32)
		SetThreadPriority(mHandle, THREAD_PRIORITY_NORMAL + mThread_priority);
#elif defined(OPENW3D_SDL3)
		assert(!mHandle);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
		// Not implemented; most POSIX targets require privileges for this.
#else
		assert(0);
#endif
	}
}

void ThreadClass::Stop()
{
	mRunning = false;
	if (mHandle) {
#if defined(OPENW3D_WIN32)
		WaitForSingleObjectEx(mHandle, INFINITE, FALSE);
		CloseHandle(mHandle);
#elif defined(OPENW3D_SDL3)
		SDL_WaitThread(mHandle, NULL);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
		pthread_join(mHandle, nullptr);
#else
		assert(0);
#endif
	}
	mHandle = NULL;
	mThreadID = 0;
}

void ThreadClass::Sleep_Ms(unsigned ms)
{
#if defined(OPENW3D_WIN32)
	Sleep(ms);
#elif defined(OPENW3D_SDL3)
	SDL_Delay(ms);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&ts, nullptr);
#else
	assert(0);
#endif
}

void ThreadClass::Switch_Thread()
{
#if defined(OPENW3D_WIN32)
	SwitchToThread();
#elif defined(OPENW3D_SDL3)
	SDL_Delay(0);
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	sched_yield();
#else
	assert(0);
#endif
}

// Return calling thread's unique thread id
unsigned ThreadClass::Get_Current_Thread_ID()
{
#if defined(OPENW3D_WIN32)
	return GetCurrentThreadId();
#elif defined(OPENW3D_SDL3)
	return SDL_GetCurrentThreadID();
#elif defined(OPENW3D_POSIX) || defined(OPENW3D_ANDROID)
	return static_cast<unsigned>(reinterpret_cast<uintptr_t>(pthread_self()));
#else
	assert(0);
#endif
}

bool ThreadClass::Is_Running()
{
	return mRunning;
}
