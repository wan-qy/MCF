// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_LANGUAGES_T_EXPRESSION_HPP_
#define MCF_LANGUAGES_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include "../Containers/List.hpp"
#include <utility>

namespace MCF {

class TExpressionNode {
	friend class TExpression;

public:
	using Child = std::pair<WideString, TExpressionNode>;

	using ChildList = List<Child>;
	using ChildNode = typename ChildList::Node;
	using ConstCursor = typename ChildList::ConstCursor;
	using Cursor = typename ChildList::Cursor;

private:
	ChildList xm_lstChildren;

public:
	bool IsEmpty() const noexcept {
		return xm_lstChildren.IsEmpty();
	}
	void Clear() noexcept {
		xm_lstChildren.Clear();
	}

	const ChildNode *GetFirstChild() const noexcept {
		return xm_lstChildren.GetFirst();
	}
	ChildNode *GetFirstChild() noexcept {
		return xm_lstChildren.GetFirst();
	}
	const ChildNode *GetLastChild() const noexcept {
		return xm_lstChildren.GetLast();
	}
	ChildNode *GetLastChild() noexcept {
		return xm_lstChildren.GetLast();
	}

	ConstCursor GetFirstCursor() const noexcept {
		return xm_lstChildren.GetFirstCursor();
	}
	Cursor GetFirstCursor() noexcept {
		return xm_lstChildren.GetFirstCursor();
	}
	ConstCursor GetLastCursor() const noexcept {
		return xm_lstChildren.GetLastCursor();
	}
	Cursor GetLastCursor() noexcept {
		return xm_lstChildren.GetLastCursor();
	}

	ChildNode *Insert(ChildNode *pPos, WideString wsName, TExpressionNode vNode){
		return Insert(pPos, Child(std::move(wsName), std::move(vNode)));
	}
	ChildNode *Insert(ChildNode *pPos, Child vChild){
		return xm_lstChildren.Insert(pPos, std::move(vChild));
	}
	ChildNode *Erase(ChildNode *pPos) noexcept {
		return xm_lstChildren.Erase(pPos);
	}

	void Swap(TExpressionNode &rhs) noexcept {
		xm_lstChildren.Swap(rhs.xm_lstChildren);
	}
};

inline void swap(TExpressionNode &lhs, TExpressionNode &rhs) noexcept {
	lhs.Swap(rhs);
}

class TExpression : public TExpressionNode {
public:
	enum ErrorType {
		ERR_NONE					= 0,
		ERR_UNEXCEPTED_NODE_CLOSE	= 1,
		ERR_UNCLOSED_QUOTE			= 2,
		ERR_UNCLOSED_NODE			= 3,
	};

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
