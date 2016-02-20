// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_BUFFER_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class BufferOutputStream : public AbstractOutputStream {
private:
	StreamBuffer x_vBuffer;

public:
	constexpr BufferOutputStream() noexcept
		: x_vBuffer()
	{
	}
	explicit BufferOutputStream(StreamBuffer vBuffer) noexcept
		: x_vBuffer(std::move(vBuffer))
	{
	}
	~BufferOutputStream() override;

	BufferOutputStream(BufferOutputStream &&) noexcept = default;
	BufferOutputStream& operator=(BufferOutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override {
		x_vBuffer.Put(byData);
	}

	void Put(const void *pData, std::size_t uSize) override {
		x_vBuffer.Put(pData, uSize);
	}

	const StreamBuffer &GetBuffer() const noexcept {
		return x_vBuffer;
	}
	StreamBuffer &GetBuffer() noexcept {
		return x_vBuffer;
	}
	void SetBuffer(StreamBuffer vBuffer) noexcept {
		x_vBuffer = std::move(vBuffer);
	}

	void Swap(BufferOutputStream &rhs) noexcept {
		using std::swap;
		swap(x_vBuffer, rhs.x_vBuffer);
	}

	friend void swap(BufferOutputStream &lhs, BufferOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
