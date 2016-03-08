// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64InputStreamFilter.hpp"
#include "../Utilities/Endian.hpp"

namespace MCF {

namespace {
	constexpr signed char kBase64ReverseTable[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};
}

Base64InputStreamFilter::~Base64InputStreamFilter(){
}

void Base64InputStreamFilter::X_PopulatePlainBuffer(std::size_t uExpected){
	for(;;){
		if(x_sbufPlain.GetSize() >= uExpected){
			break;
		}

		unsigned char abyTemp[4];
		y_vStream.PopulateBuffer(4);
		const auto uBytesRead = y_vStream.BufferedPeek(abyTemp, 4);
		if(uBytesRead < 4){
			break;
		}

		const auto ChecedRead = [&](unsigned char by){
			const int nPart = kBase64ReverseTable[by];
			if(nPart < 0){
				DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "Invalid Base64 encoded byte"_rcs);
			}
			ASSERT(nPart < 64);
			return static_cast<std::uint_fast32_t>(nPart);
		};

		unsigned uBytes;
		std::uint_fast32_t u32Word = 0;
		if(abyTemp[2] == '='){
			uBytes = 1;
			u32Word |= (ChecedRead(abyTemp[0]) << 26);
			u32Word |= (ChecedRead(abyTemp[1]) << 20);
		} else if(abyTemp[3] == '='){
			uBytes = 2;
			u32Word |= (ChecedRead(abyTemp[0]) << 26);
			u32Word |= (ChecedRead(abyTemp[1]) << 20);
			u32Word |= (ChecedRead(abyTemp[2]) << 14);
		} else {
			uBytes = 3;
			u32Word |= (ChecedRead(abyTemp[0]) << 26);
			u32Word |= (ChecedRead(abyTemp[1]) << 20);
			u32Word |= (ChecedRead(abyTemp[2]) << 14);
			u32Word |= (ChecedRead(abyTemp[3]) <<  8);
		}

		std::uint32_t u32WordInBigEndian;
		StoreBe(u32WordInBigEndian, u32Word);
		x_sbufPlain.Put(&u32WordInBigEndian, uBytes);
		y_vStream.BufferedDiscard(4);
	}
}

int Base64InputStreamFilter::Peek(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Peek();
}
int Base64InputStreamFilter::Get(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Get();
}
bool Base64InputStreamFilter::Discard(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Discard();
}

std::size_t Base64InputStreamFilter::Peek(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Peek(pData, uSize);
}
std::size_t Base64InputStreamFilter::Get(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Get(pData, uSize);
}
std::size_t Base64InputStreamFilter::Discard(std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Discard(uSize);
}

}