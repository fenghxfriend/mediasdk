/// 智能内存块模块头文件
/// 本模块旨在实现数组的自动资源管理，使用者无需主动调用释放函数
/// 作者：杨方华
/// 创建日期：2017-8-5
/// 注意：特意使用中文注释，请勿修改

#ifndef __PAOMIANTV_SHARED_BUFFER_H__
#define __PAOMIANTV_SHARED_BUFFER_H__

#include <memory>

namespace paomiantv {
    
    typedef std::shared_ptr<char> CharPtr;
    typedef std::shared_ptr<unsigned char> UCharPtr;
    
    /// 分配指定类型的智能数组对象
    /// 建议以此方式替代new char[count], new unsigned char[count]等分配内存块方式
    /// 例：
    ///     std::shared_ptr<char> curBuffer = paomiantv::allocSharedArray<char>(1024);
    ///     std::shared_ptr<unsigned char> curBuffer = paomiantv::allocSharedArray<unsigned char>(1024);
    template <typename T>
    static inline std::shared_ptr<T> allocSharedArray(size_t count)
    {
        T *nativeArray = nullptr;
        if (0 < count) {
            nativeArray = new T[count];
        }
        return std::shared_ptr<T>(nativeArray, [](T *buf) {
            if (buf) {
                delete []buf;
            }
        });
    }

}

#endif
