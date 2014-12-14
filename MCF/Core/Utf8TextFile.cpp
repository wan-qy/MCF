// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utf8TextFile.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Containers/Vector.hpp"
using namespace MCF;

namespace {

constexpr unsigned char UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };

}

// ========== Utf8TextFileReader ==========
// 其他非静态成员函数。
void Utf8TextFileReader::Reset() noexcept {
	xm_vFile.Close();
	xm_u64Offset = 0;
	xm_sbufCache.Clear();
}
void Utf8TextFileReader::Reset(File &&vFile){
	ASSERT(&xm_vFile != &vFile);

	Reset();

	if(!vFile){
		return;
	}
	xm_vFile = std::move(vFile);

	unsigned char abyTemp[sizeof(UTF8_BOM)];
	if((xm_vFile.Read(abyTemp, sizeof(abyTemp), 0) == sizeof(abyTemp)) && !BComp(abyTemp, UTF8_BOM)){
		xm_u64Offset += sizeof(UTF8_BOM);
	}
}

bool Utf8TextFileReader::IsAtEndOfFile() const {
	return xm_sbufCache.IsEmpty() && (xm_u64Offset == xm_vFile.GetSize());
}
int Utf8TextFileReader::Read(){
	if(xm_sbufCache.GetSize() <= 1){
		unsigned char abyTemp[0x1000];
		const auto uBytesRead = xm_vFile.Read(abyTemp, sizeof(abyTemp), xm_u64Offset);
		if(uBytesRead != 0){
			xm_sbufCache.Put(abyTemp, uBytesRead);
			xm_u64Offset += uBytesRead;
		}
	}
	int nRet = xm_sbufCache.Get();
	if((nRet == '\r') && (xm_sbufCache.Peek() == '\n')){
		nRet = xm_sbufCache.Get();
	}
	return nRet;
}
bool Utf8TextFileReader::Read(Utf8String &u8sData, std::size_t uCount){
	u8sData.Clear();
	u8sData.Reserve(uCount);

	if(uCount == 0){
		return !IsAtEndOfFile();
	}

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		u8sData.UncheckedPush((char)nChar);
		if(u8sData.GetSize() == uCount){
			break;
		}
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}
bool Utf8TextFileReader::ReadLine(Utf8String &u8sData){
	u8sData.Clear();

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		if(nChar == '\n'){
			break;
		}
		u8sData.Push((char)nChar);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}
bool Utf8TextFileReader::ReadTillEof(Utf8String &u8sData){
	u8sData.Clear();

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		u8sData.Push((char)nChar);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}

// ========== Utf8TextFileWriter ==========
// 其他非静态成员函数。
void Utf8TextFileWriter::Reset() noexcept {
	try {
		Flush();
	} catch(...){
	}
	xm_vFile.Close();
	xm_u32Flags = 0;
	xm_u64Offset = 0;
	xm_u8sLine.Clear();
}
void Utf8TextFileWriter::Reset(File &&vFile, std::uint32_t u32Flags){
	ASSERT(&xm_vFile != &vFile);

	xm_vFile = std::move(vFile);
	xm_u32Flags = u32Flags;
	xm_u64Offset = xm_vFile.GetSize();

	if((xm_u32Flags & BOM_USE) && (xm_u64Offset == 0)){
		xm_vFile.Write(0, UTF8_BOM, sizeof(UTF8_BOM));
		xm_vFile.Flush();

		xm_u64Offset += sizeof(UTF8_BOM);
	}

}

std::uint32_t Utf8TextFileWriter::GetFlags() const noexcept {
	return xm_u32Flags;
}
std::uint32_t Utf8TextFileWriter::SetFlags(std::uint32_t u32Flags) noexcept {
	auto u32OldFlags = xm_u32Flags;
	if(xm_u32Flags != u32Flags){
		Flush();
		xm_u32Flags = u32Flags;
	}
	return u32OldFlags;
}

void Utf8TextFileWriter::Write(char ch){
	if(xm_u32Flags & BUF_NONE){
		xm_vFile.Write(xm_u64Offset, &ch, 1);
	} else {
		if((ch == '\n') && (xm_u32Flags & LES_CRLF)){
			xm_u8sLine += '\r';
		}
		xm_u8sLine += ch;

		bool bFlushNow;
		if(xm_u32Flags & BUF_FULL){
			bFlushNow = xm_u8sLine.GetSize() >= 0x1000;
		} else {
			bFlushNow = (ch == '\n');
		}
		if(bFlushNow){
			Flush();
		}
	}
}
void Utf8TextFileWriter::Write(const Utf8StringObserver &u8soData){
	for(auto ch : u8soData){
		Write(ch);
	}
}
void Utf8TextFileWriter::WriteLine(const Utf8StringObserver &u8soData){
	Write(u8soData);
	Write('\n');
}
void Utf8TextFileWriter::Flush(){
	xm_vFile.Write(xm_u64Offset, xm_u8sLine.GetData(), xm_u8sLine.GetSize());
	xm_vFile.Flush();

	xm_u64Offset += xm_u8sLine.GetSize();
	xm_u8sLine.Clear();
}
