// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FILE_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_FILE_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/File.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

// 性能警告：FileOutputStream 不提供 I/O 缓冲。

class FileOutputStream : public AbstractOutputStream {
private:
	File x_vFile;
	std::uint64_t x_u64Offset;

public:
	constexpr FileOutputStream() noexcept
		: x_vFile(), x_u64Offset(0)
	{
	}
	explicit FileOutputStream(File vFile, std::uint64_t u64Offset = 0) noexcept
		: x_vFile(std::move(vFile)), x_u64Offset(u64Offset)
	{
	}
	~FileOutputStream() override;

	FileOutputStream(FileOutputStream &&) noexcept = default;
	FileOutputStream& operator=(FileOutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override {
		Put(&byData, 1);
	}

	void Put(const void *pData, std::size_t uSize) override {
		std::size_t uBytesWritten = 0;
		while(uBytesWritten < uSize){
			const auto uBytesWrittenThisTime = x_vFile.Write(x_u64Offset + uBytesWritten, static_cast<const unsigned char *>(pData) + uBytesWritten, uSize - uBytesWritten);
			if(uBytesWrittenThisTime == 0){
				DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileStreamBase: Partial contents written"_rcs);
			}
			uBytesWritten += uBytesWrittenThisTime;
		}
		x_u64Offset += uBytesWritten;
	}

	const File &GetFile() const noexcept {
		return x_vFile;
	}
	File &GetFile() noexcept {
		return x_vFile;
	}
	void SetFile(File vFile, std::uint64_t u64Offset = 0) noexcept {
		x_vFile     = std::move(vFile);
		x_u64Offset = u64Offset;
	}

	std::uint64_t GetOffset() const noexcept {
		return x_u64Offset;
	}
	void SetOffset(std::uint64_t u64Offset) noexcept {
		x_u64Offset = u64Offset;
	}

	void Swap(FileOutputStream &rhs) noexcept {
		using std::swap;
		swap(x_vFile,     rhs.x_vFile);
		swap(x_u64Offset, rhs.x_u64Offset);
	}

	friend void swap(FileOutputStream &lhs, FileOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
