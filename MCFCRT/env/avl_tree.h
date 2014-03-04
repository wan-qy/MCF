// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_AVL_H__
#define __MCF_CRT_AVL_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

typedef struct tagAVLNodeHeader {
	__MCF_STD intptr_t nKey;
	struct tagAVLNodeHeader *pParent;
	struct tagAVLNodeHeader **ppRefl;
	struct tagAVLNodeHeader *pLeft;
	struct tagAVLNodeHeader *pRight;
	__MCF_STD size_t uHeight;
	struct tagAVLNodeHeader *pPrev;
	struct tagAVLNodeHeader *pNext;
} __MCF_AVL_NODE_HEADER;

typedef __MCF_AVL_NODE_HEADER *__MCF_AVL_PROOT;

static inline __MCF_AVL_NODE_HEADER *__MCF_AvlPrev(const __MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pPrev;
}
static inline __MCF_AVL_NODE_HEADER *__MCF_AvlNext(const __MCF_AVL_NODE_HEADER *pNode) __MCF_NOEXCEPT {
	return pNode->pNext;
}

extern void __MCF_AvlSwap(__MCF_AVL_PROOT *ppRoot1, __MCF_AVL_PROOT *ppRoot2) __MCF_NOEXCEPT;

// 若 arg0 < arg1 应返回非零值，否则应返回零。
typedef int (*__MCF_AVL_KEY_COMPARER)(__MCF_STD intptr_t, __MCF_STD intptr_t) __MCF_NOEXCEPT;

extern void __MCF_AvlAttach(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;
extern void __MCF_AvlAttachCustomComp(
	__MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_NODE_HEADER *pNode,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;
extern void __MCF_AvlDetach(
	const __MCF_AVL_NODE_HEADER *pNode
) __MCF_NOEXCEPT;

// Q: 为什么这里是 const __MCF_AVL_NODE_HEADER * 而不是 __MCF_AVL_NODE_HEADER * 呢？
// A: 参考 strchr 函数。
extern __MCF_AVL_NODE_HEADER *__MCF_AvlLowerBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;
extern __MCF_AVL_NODE_HEADER *__MCF_AvlLowerBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;
extern __MCF_AVL_NODE_HEADER *__MCF_AvlUpperBound(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;
extern __MCF_AVL_NODE_HEADER *__MCF_AvlUpperBoundCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;
extern __MCF_AVL_NODE_HEADER *__MCF_AvlFind(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;
extern __MCF_AVL_NODE_HEADER *__MCF_AvlFindCustomComp(
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;
extern void __MCF_AvlEqualRange(
	__MCF_AVL_NODE_HEADER **ppFrom,
	__MCF_AVL_NODE_HEADER **ppTo,
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey
) __MCF_NOEXCEPT;
extern void __MCF_AvlEqualRangeCustomComp(
	__MCF_AVL_NODE_HEADER **ppFrom,
	__MCF_AVL_NODE_HEADER **ppTo,
	const __MCF_AVL_PROOT *ppRoot,
	__MCF_STD intptr_t nKey,
	__MCF_AVL_KEY_COMPARER pfnKeyComparer
) __MCF_NOEXCEPT;

__MCF_EXTERN_C_END

#endif
