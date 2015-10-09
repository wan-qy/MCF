// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CLONE_HPP_
#define MCF_UTILITIES_CLONE_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

template<typename T>
inline std::decay_t<T> Clone(T &&vSrc) noexcept(std::is_nothrow_constructible<std::decay_t<T>, T &&>::value) {
	auto vTemp(std::forward<T>(vSrc));
	return vTemp;
}

}

#endif
