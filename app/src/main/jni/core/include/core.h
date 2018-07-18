//
// Created by ASUS on 2018/6/12.
//

#ifndef MEDIAENGINE_CORE_H
#define MEDIAENGINE_CORE_H

#include <safequeue.h>
#include <thread.h>
#include <safelist.h>
#include "message.h"
#include "module.h"

#define MESSAGE_BUS_CAPACITY 512
namespace paomiantv {

    typedef void* (*PTRCreateObject)(void);

    class CCore {
    private:
        CCore();

        ~CCore();

        static CCore *m_pInstance;

        class Garbo {
        public:
            ~Garbo() {
                if (CCore::m_pInstance) {
                    delete CCore::m_pInstance;
                }
            }
        };

        static Garbo garbo;
    public:
        static CCore *getInstance();

        void setup();

        void shutdown();

        bool registerModule(IModule *module);

        void unregisterModule(IModule *module);

        bool sendMessage(CMessage *message);

        bool checkModule(u32 id);

    private:
        bool m_bShutDown;
        ILock *m_pLockModule;
        CThread *m_pThread;
        CSafeQueue<CMessage> *m_pMsgBus;
        CSafeList<IModule> *m_pModuleList;
    private:
        static bool compareByIdCB(u64 id, IModule *module);

        static void *ThreadWrapper(void *pData);

        long run();

        void handleMessage(CMessage *message);
    };
}

#endif //MEDIAENGINE_CORE_H
