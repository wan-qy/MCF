// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "heap_dbg.h"

#ifdef __MCF_CRT_HEAPDBG_ON

#include "bail.h"
#include "../c/ext/tchar.h"
#include <wchar.h>
#include <windows.h>

#define GUARD_BAND_SIZE		0x20ul

static HANDLE							g_hMapAllocator;
static __MCF_AVL_PROOT					g_mapBlocks;
static const __MCF_HEAPDBG_BLOCK_INFO	*g_pBlockHead;

unsigned long __MCF_CRT_HeapDbgInitContext(){
	g_hMapAllocator = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if(!g_hMapAllocator){
		return GetLastError();
	}

	return ERROR_SUCCESS;
}
void __MCF_CRT_HeapDbgUninitContext(){
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = g_pBlockHead;
	if(pBlockInfo){
		__MCF_Bail(
			L"__MCF_CRT_HeapDbgUninitContext() 失败：侦测到内存泄漏。\n\n"
			"如果您选择调试应用程序，MCF CRT 将尝试使用 OutputDebugString() 导出内存泄漏的详细信息。"
		);

		wchar_t awchBuffer[256];
		do {
			const BYTE *pbyDump = __MCF_CRT_HeapDbgGetContents(pBlockInfo);
			wchar_t *pwchWrite = awchBuffer + __mingw_snwprintf(
				awchBuffer,
				sizeof(awchBuffer) / sizeof(wchar_t),
				L"地址 %p  大小 %p  调用返回地址 %p  首字节 ",
				(void *)pbyDump,
				(void *)pBlockInfo->uSize,
				(void *)pBlockInfo->pRetAddr
			);
			for(size_t i = 0; i < 16; ++i){
				*(pwchWrite++) = L' ';
				if(IsBadReadPtr(pbyDump, 1)){
					*(pwchWrite++) = L'?';
					*(pwchWrite++) = L'?';
				} else {
					static const wchar_t HEX_TABLE[] = L"0123456789ABCDEF";
					*(pwchWrite++) = HEX_TABLE[(*pbyDump >> 4) & 0xF0];
					*(pwchWrite++) = HEX_TABLE[*pbyDump & 0x0F];
				}
				++pbyDump;
			}
			*pwchWrite = 0;

			OutputDebugStringW(awchBuffer);
		} while(!!(pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLNext((const __MCF_AVL_NODE_HEADER *)pBlockInfo)));
		__asm__ __volatile__("int 3 \n");
	}

	HeapDestroy(g_hMapAllocator);

	g_pBlockHead	= NULL;
	g_mapBlocks		= NULL;
	g_hMapAllocator	= NULL;
}

size_t __MCF_CRT_HeapDbgGetRawSize(
	size_t uContentSize
){
	return uContentSize + GUARD_BAND_SIZE * 2;
}
void __MCF_CRT_HeapDbgAddGuardsAndRegister(
	unsigned char **ppContents,
	unsigned char *pRaw,
	size_t uContentSize,
	const void *pRetAddr
){
	unsigned char *const pContents = pRaw + GUARD_BAND_SIZE;
	*ppContents = pContents;

	void **ppGuard1 = (void **)pContents;
	void **ppGuard2 = (void **)(pContents + uContentSize);
	for(unsigned int i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		*ppGuard1 = EncodePointer(ppGuard2);
		*ppGuard2 = EncodePointer(ppGuard1);

		++ppGuard2;
	}

	__MCF_HEAPDBG_BLOCK_INFO *const pBlockInfo = (__MCF_HEAPDBG_BLOCK_INFO *)HeapAlloc(g_hMapAllocator, 0, sizeof(__MCF_HEAPDBG_BLOCK_INFO));
	if(!pBlockInfo){
		__MCF_BailF(L"__MCF_CRT_HeapDbgAddGuardsAndRegister() 失败：内存不足。\n调用返回地址：%p", pRetAddr);
	}
	pBlockInfo->uSize = uContentSize;
	pBlockInfo->pRetAddr = pRetAddr;

	if(__MCF_AVLAttach(&g_mapBlocks, (intptr_t)pContents, (__MCF_AVL_NODE_HEADER *)pBlockInfo)){
		__MCF_BailF(L"__MCF_CRT_HeapDbgAddGuardsAndRegister() 失败：传入的指针的记录已存在。\n调用返回地址：%p", pRetAddr);
	}
	if(!__MCF_AVLPrev((__MCF_AVL_NODE_HEADER *)pBlockInfo)){
		g_pBlockHead = pBlockInfo;
	}
}
const __MCF_HEAPDBG_BLOCK_INFO *__MCF_CRT_HeapDbgValidate(
	unsigned char **ppRaw,
	unsigned char *pContents,
	const void *pRetAddr
){
	unsigned char *const pRaw = pContents - GUARD_BAND_SIZE;
	*ppRaw = pRaw;

	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo = (const __MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLFind(&g_mapBlocks, (intptr_t)pContents);
	if(!pBlockInfo){
		__MCF_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：传入的指针无效。\n调用返回地址：%p", pRetAddr);
	}

	void *const *ppGuard1 = (void *const *)pContents;
	void *const *ppGuard2 = (void *const *)(pContents + pBlockInfo->uSize);
	for(unsigned int i = 0; i < GUARD_BAND_SIZE; i += sizeof(void *)){
		--ppGuard1;

		if((DecodePointer(*ppGuard1) != ppGuard2) || (DecodePointer(*ppGuard2) != ppGuard1)){
			__MCF_BailF(L"__MCF_CRT_HeapDbgValidate() 失败：侦测到堆损坏。\n调用返回地址：%p", pRetAddr);
		}

		++ppGuard2;
	}

	return pBlockInfo;
}
const unsigned char *__MCF_CRT_HeapDbgGetContents(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	return (const unsigned char *)pBlockInfo->__MCF_AVLNodeHeader.nKey;
}
void __MCF_CRT_HeapDbgUnregister(
	const __MCF_HEAPDBG_BLOCK_INFO *pBlockInfo
){
	if(g_pBlockHead == pBlockInfo){
		g_pBlockHead = (__MCF_HEAPDBG_BLOCK_INFO *)__MCF_AVLNext((__MCF_AVL_NODE_HEADER *)pBlockInfo);
	}
	__MCF_AVLDetach((const __MCF_AVL_NODE_HEADER *)pBlockInfo);

	HeapFree(g_hMapAllocator, 0, (void *)pBlockInfo);
}

#endif
