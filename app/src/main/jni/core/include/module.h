//
// Created by ASUS on 2018/6/12.
//

#ifndef MEDIAENGINE_APP_H
#define MEDIAENGINE_APP_H

#include <typedef.h>

#ifndef MAX_MODULE_NAME_LEN
#define MAX_MODULE_NAME_LEN 8
#endif

#ifndef OnHandleMessageCallback

typedef bool (*OnMessageCallback)(const paomiantv::CMessage *);

#endif

namespace paomiantv {
    class IModule {
    public:
        IModule(const s8 *name = NULL);

        virtual ~IModule();

        virtual bool handleMessage(CMessage *message);

        void setMessageCallback(OnMessageCallback callback);

        void setName(const s8 *name);

        inline u32 getId();

        inline s8 *getName();

    private:
        void init(const s8 *name);

        void uninit();

    private:
        static u32 m_sCount;
        ILock *m_pLockHandler;

        const u32 m_uId;// module id

        s8 m_achName[MAX_MODULE_NAME_LEN];

        OnMessageCallback m_cbHandler;
    };

    inline u32 IModule::getId() {
        return m_uId;
    }

    inline s8 *IModule::getName() {
        return m_achName;
    }
}

#endif //MEDIAENGINE_APP_H
