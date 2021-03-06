#ifndef _USAGEENVIRONMENT_H_
#define _USAGEENVIRONMENT_H_
#include "rtsp/net/EventScheduler.h"
#include "rtsp/base/ThreadPool.h"

class UsageEnvironment
{
public:
    static UsageEnvironment* createNew(EventScheduler* scheduler, ThreadPool* threadPool);
    ~UsageEnvironment();

    EventScheduler* scheduler();
    ThreadPool* threadPool();

private:
    UsageEnvironment(EventScheduler* scheduler, ThreadPool* threadPool);

private:
    EventScheduler* mScheduler;
    ThreadPool* mThreadPool;
};

#endif //_USAGEENVIRONMENT_H_
