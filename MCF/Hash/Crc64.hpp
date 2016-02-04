// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_HASH_CRC64_HPP_
#define MCF_HASH_CRC64_HPP_

#include <cstddef>
#include <cstdint>

namespace MCF {

// 按照 ECMA-182 描述的算法，除数为 0xEDB88640。

class Crc64 {
private:
	bool x_bInited;
	std::uint64_t x_u64Reg;

public:
	Crc64() noexcept;

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize) noexcept;
	std::uint64_t Finalize() noexcept;
};

}

#endif