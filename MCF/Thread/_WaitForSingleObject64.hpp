// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_WAIT_FOR_SINGLE_OBJECT_64_HPP_
#define MCF_THREAD_WAIT_FOR_SINGLE_OBJECT_64_HPP_

namespace MCF {

// 如果 pullMilliSeconds 不为空，则其指定一个出入参，表示需等待的时间；函数返回 false 时，其值为剩余的时间。
// 如果 pullMilliSeconds 为空则无限等待，此时不可能返回 false。
extern bool WaitForSingleObject64(void *hObject, unsigned long long *pullMilliSeconds) noexcept;

}

#endif
