
#ifndef __LTSERV_LOCK_H__
#define __LTSERV_LOCK_H__
#include <pthread.h>
#include <cstdlib>

class Lock
{
public:

	Lock()
	{
		if (pthread_mutex_init(&_mutex, NULL) != 0)
		{
			abort();
		}
	}

	~Lock()
	{
		pthread_mutex_destroy(&_mutex);
	}

	void get()
	{
		pthread_mutex_lock(&_mutex);
	}

	void free()
	{
		pthread_mutex_unlock(&_mutex);
	}

private:
	pthread_mutex_t _mutex;
};

#endif
