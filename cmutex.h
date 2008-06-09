#ifndef CMUTEX_H
#define CMUTEX_H

#include <SDL_mutex.h>
#include <stdexcept>


class cMutex
{
	public:
		cMutex() : mutex_(SDL_CreateMutex())
		{
			if (!mutex_) throw std::runtime_error("Failed to create mutex");
		}

		~cMutex() { SDL_DestroyMutex(mutex_); }

		class Lock
		{
			public:
				Lock(cMutex& m) : mutex_(m.mutex_) { SDL_mutexP(mutex_); }

				~Lock() { SDL_mutexV(mutex_); }

			private:
				SDL_mutex* const mutex_;
		};

	private:
		SDL_mutex* const mutex_;
};

#endif
