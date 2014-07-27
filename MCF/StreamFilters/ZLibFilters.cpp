// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ZLibFilters.hpp"
#include "../Core/Exception.hpp"

#define ZLIB_CONST
#define Z_PREFIX
#include "../../External/zlib/zlib.h"
using namespace MCF;

namespace {

constexpr std::size_t STEP_SIZE = 0x4000;

unsigned long ZLibErrorToWin32Error(int nZLibError) noexcept {
	switch(nZLibError){
	case Z_OK:
		return ERROR_SUCCESS;

	case Z_STREAM_END:
		return ERROR_HANDLE_EOF;

	case Z_NEED_DICT:
		return ERROR_INVALID_PARAMETER;

	case Z_ERRNO:
		return ERROR_OPEN_FAILED;

	case Z_STREAM_ERROR:
		return ERROR_INVALID_PARAMETER;

	case Z_DATA_ERROR:
		return ERROR_INVALID_DATA;

	case Z_MEM_ERROR:
		return ERROR_NOT_ENOUGH_MEMORY;

	case Z_BUF_ERROR:
		return ERROR_SUCCESS;

	case Z_VERSION_ERROR:
		return ERROR_NOT_SUPPORTED;

	default:
		return ERROR_INVALID_FUNCTION;
	}
}

class ZLibEncoderDelegate : CONCRETE(ZLibEncoder) {
private:
	z_stream xm_vStream;
	bool xm_bInited;

public:
	ZLibEncoderDelegate(bool bRaw, unsigned int uLevel){
		xm_vStream.zalloc	= Z_NULL;
		xm_vStream.zfree	= Z_NULL;
		xm_vStream.opaque	= Z_NULL;

		const auto ulErrorCode = ZLibErrorToWin32Error(::deflateInit2(
			&xm_vStream, (int)uLevel, Z_DEFLATED, bRaw ? -15 : 15, 9, Z_DEFAULT_STRATEGY
		));
		if(ulErrorCode != ERROR_SUCCESS){
			MCF_THROW(ulErrorCode, L"::deflateInit2() 失败。"_wso);
		}

		xm_bInited = false;
	}
	~ZLibEncoderDelegate() noexcept {
		::deflateEnd(&xm_vStream);
	}

public:
	void Abort() noexcept {
		xm_bInited = false;
		StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::deflateReset(&xm_vStream);
			xm_bInited = true;
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_vStream.next_out = abyTemp;
		xm_vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_vStream.next_in = pbyRead;
			xm_vStream.avail_in = uToProcess;
			do {
				const auto ulErrorCode = ZLibErrorToWin32Error(::deflate(
					&xm_vStream, Z_NO_FLUSH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::deflate() 失败。"_wso);
				}
				if(xm_vStream.avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			} while(xm_vStream.avail_in != 0);

			StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_vStream.avail_out != 0){
			xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned char abyTemp[STEP_SIZE];
			xm_vStream.next_out = abyTemp;
			xm_vStream.avail_out = sizeof(abyTemp);

			xm_vStream.next_in = nullptr;
			xm_vStream.avail_in = 0;
			for(;;){
				const auto ulErrorCode = ZLibErrorToWin32Error(::deflate(
					&xm_vStream, Z_FINISH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::deflate() 失败。"_wso);
				}
				if(xm_vStream.avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			}
			if(xm_vStream.avail_out != 0){
				xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
			}

			xm_bInited = false;
		}
		StreamFilterBase::Finalize();
	}
};

class ZLibDecoderDelegate : CONCRETE(ZLibDecoder) {
private:
	z_stream xm_vStream;
	bool xm_bInited;

public:
	explicit ZLibDecoderDelegate(bool bRaw){
		xm_vStream.zalloc	= Z_NULL;
		xm_vStream.zfree	= Z_NULL;
		xm_vStream.opaque	= Z_NULL;

		xm_vStream.next_in = nullptr;
		xm_vStream.avail_in = 0;

		const auto ulErrorCode = ZLibErrorToWin32Error(::inflateInit2(
			&xm_vStream, bRaw ? -15 : 15
		));
		if(ulErrorCode != ERROR_SUCCESS){
			MCF_THROW(ulErrorCode, L"::inflateInit2() 失败。"_wso);
		}

		xm_bInited = false;
	}
	~ZLibDecoderDelegate() noexcept {
		::inflateEnd(&xm_vStream);
	}

public:
	void Abort() noexcept {
		xm_bInited = false;
		StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::inflateReset(&xm_vStream);
			xm_bInited = true;
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_vStream.next_out = abyTemp;
		xm_vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_vStream.next_in = pbyRead;
			xm_vStream.avail_in = uToProcess;
			do {
				const auto ulErrorCode = ZLibErrorToWin32Error(::inflate(
					&xm_vStream, Z_NO_FLUSH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::inflate() 失败。"_wso);
				}
				if(xm_vStream.avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			} while(xm_vStream.avail_in != 0);

			StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_vStream.avail_out != 0){
			xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned char abyTemp[STEP_SIZE];
			xm_vStream.next_out = abyTemp;
			xm_vStream.avail_out = sizeof(abyTemp);

			xm_vStream.next_in = nullptr;
			xm_vStream.avail_in = 0;
			for(;;){
				const auto ulErrorCode = ZLibErrorToWin32Error(::inflate(
					&xm_vStream, Z_FINISH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::inflate() 失败。"_wso);
				}
				if(xm_vStream.avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			}
			if(xm_vStream.avail_out != 0){
				xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
			}

			xm_bInited = false;
		}
		StreamFilterBase::Finalize();
	}
};

}

// ========== ZLibEncoder ==========
// 静态成员函数。
std::unique_ptr<ZLibEncoder> ZLibEncoder::Create(bool bRaw, unsigned int uLevel){
	return std::make_unique<ZLibEncoderDelegate>(bRaw, uLevel);
}

// ========== ZLibDecoder ==========
// 静态成员函数。
std::unique_ptr<ZLibDecoder> ZLibDecoder::Create(bool bRaw){
	return std::make_unique<ZLibDecoderDelegate>(bRaw);
}
