// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UTF8_TEXT_FILE_HPP_
#define MCF_CORE_UTF8_TEXT_FILE_HPP_

#include "File.hpp"
#include "String.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class Utf8TextFileReader : Noncopyable {
private:
	const std::unique_ptr<File> xm_pFile;

	std::uint64_t xm_u64Offset;
	StreamBuffer xm_sbufCache;

public:
	explicit Utf8TextFileReader(std::unique_ptr<File> pFile);

public:
	bool IsAtEndOfFile() const noexcept;
	int Read();
	bool Read(Utf8String &u8sData, std::size_t uCount);
	bool ReadLine(Utf8String &u8sData);
	bool ReadTillEof(Utf8String &u8sData);
};

class Utf8TextFileWriter : Noncopyable {
public:
	enum : std::uint32_t {
		BOM_NONE		= 0x00000000,
		BOM_USE			= 0x00000001,

		LES_LF			= 0x00000000,
		LES_CRLF		= 0x00000002,

		BUF_LINE		= 0x00000000,
		BUF_NONE		= 0x00000004,
		BUF_FULL		= 0x00000008,
	};

private:
	const std::unique_ptr<File> xm_pFile;
	const std::uint32_t xm_u32Flags;

	std::uint64_t xm_u64Offset;
	Utf8String xm_u8sLine;

public:
	explicit Utf8TextFileWriter(std::unique_ptr<File> pFile, std::uint32_t u32Flags = 0);
	~Utf8TextFileWriter();

public:
	void Write(char ch);
	void Write(const Utf8StringObserver &u8soData);
	void WriteLine(const Utf8StringObserver &u8soData);
	void Flush();
};

}

#endif
