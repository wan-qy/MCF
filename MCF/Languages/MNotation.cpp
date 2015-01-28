// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "MNotation.hpp"
#include "../Containers/Vector.hpp"
using namespace MCF;

namespace {

WideString Unescape(const WideStringObserver &wsoSrc){
	WideString wsRet;
	wsRet.Reserve(wsoSrc.GetSize());

	enum {
		NORMAL,
		ESCAPED,
		UCS_CODE
	} eState = NORMAL;

	char32_t c32CodePoint = 0;
	std::size_t uHexExpecting = 0;

	const auto pwcEnd = wsoSrc.GetEnd();
	for(auto pwcCur = wsoSrc.GetBegin(); pwcCur != pwcEnd; ++pwcCur){
		const auto wc = *pwcCur;

		switch(eState){
		case NORMAL:
			if(wc == L'\\'){
				eState = ESCAPED;
			} else {
				// eState = NORMAL;
				wsRet.Push(wc);
			}
			break;

		case ESCAPED:
			switch(wc){
			case L'n':
				wsRet += L'\n';
				eState = NORMAL;
				break;

			case L'b':
				wsRet += L'\b';
				eState = NORMAL;
				break;

			case L'r':
				wsRet += L'\r';
				eState = NORMAL;
				break;

			case L't':
				wsRet += L'\t';
				eState = NORMAL;
				break;

			case L'\n':
				eState = NORMAL;
				break;

			case L'x':
				c32CodePoint = 0;
				uHexExpecting = 2;
				eState = UCS_CODE;
				break;

			case L'u':
				c32CodePoint = 0;
				uHexExpecting = 4;
				eState = UCS_CODE;
				break;

			case L'U':
				c32CodePoint = 0;
				uHexExpecting = 8;
				eState = UCS_CODE;
				break;

			default:
				wsRet += wc;
				eState = NORMAL;
				break;
			}
			break;

		case UCS_CODE:
			{
				auto uHex = (unsigned)(wc - L'0');
				do {
					if(uHex <= 9){
						break;
					}
					uHex += (unsigned)(L'0' - L'A');
					if(uHex <= 5){
						uHex += 0x0A;
						break;
					}
					uHex += (unsigned)(L'A' - L'a');
					if(uHex <= 5){
						uHex += 0x0A;
						break;
					}
					uHex = 0x10;
				} while(false);

				if(uHex <= 0x0F){
					c32CodePoint = (c32CodePoint << 4) | uHex;
					--uHexExpecting;
				} else {
					uHexExpecting = 0;
				}
				if(uHexExpecting != 0){
					// eState = UCS_CODE;
				} else {
					wsRet += Utf32StringObserver(&c32CodePoint, 1);
					eState = NORMAL;
				}
			}
			break;
		}
	}
	if(eState == UCS_CODE){
		wsRet += Utf32StringObserver(&c32CodePoint, 1);
	}

	return wsRet;
}
void Escape(WideString &wsAppendTo, const WideStringObserver &wsoSrc){
	const auto uSrcLength = wsoSrc.GetLength();
	wsAppendTo.ReserveMore(uSrcLength);

	for(std::size_t i = 0; i < uSrcLength; ++i){
		const auto wc = wsoSrc[i];

		switch(wc){
		case L'\\':
		case L'=':
		case L'{':
		case L'}':
		case L';':
			wsAppendTo += L'\\';
			wsAppendTo += wc;
			break;

		case L' ':
			if((i == 0) || (i == uSrcLength - 1)){
				wsAppendTo += L'\\';
			}
			wsAppendTo += L' ';
			break;

		case L'\n':
			wsAppendTo += L'\\';
			wsAppendTo += L'n';
			break;

		case L'\r':
			wsAppendTo += L'\\';
			wsAppendTo += L'r';
			break;

		case L'\t':
			wsAppendTo += L'\\';
			wsAppendTo += L't';
			break;

		default:
			wsAppendTo += wc;
			break;
		}
	}
}

}

// 其他非静态成员函数。
std::pair<MNotation::ErrorType, const wchar_t *> MNotation::Parse(const WideStringObserver &wsoData){
	MNotation vTemp;

	auto pwcRead = wsoData.GetBegin();
	const auto pwcEnd = wsoData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(ERR_NONE, pwcRead);
	}

	Vector<MNotationNode *> vecPackageStack(1, &vTemp);

	const wchar_t *pwcNameBegin = nullptr;
	const wchar_t *pwcNameEnd = nullptr;
	const wchar_t *pwcValueBegin = nullptr;
	const wchar_t *pwcValueEnd = nullptr;

	enum {
		NAME_INDENT,
		NAME_BODY,
		NAME_PADDING,
		VAL_INDENT,
		VAL_BODY,
		VAL_PADDING,
		COMMENT,
		NAME_ESCAPED,
		VAL_ESCAPED,
		COMMENT_ESCAPED
	} eState = NAME_INDENT;

	ErrorType eError;
	const auto PushPackage = [&]{
		ASSERT(!vecPackageStack.IsEmpty());

		MNotationNode *ppkgSource = nullptr;
		if(pwcValueBegin){
			const auto wsSourceName = Unescape(WideStringObserver(pwcValueBegin, pwcValueEnd));
			const auto pSourceNode = vecPackageStack.GetEnd()[-1]->Get(wsSourceName);
			if(!pSourceNode){
				eError = ERR_SOURCE_PACKAGE_NOT_FOUND;
				return false;
			}
			ppkgSource = &(pSourceNode->Get().second);
		}

		const auto vResult = vecPackageStack.GetEnd()[-1]->Insert(Unescape(WideStringObserver(pwcNameBegin, pwcNameEnd)));
		if(!vResult.second){
//			eError = ERR_DUPLICATE_PACKAGE;
//			return false;
		}
		if(ppkgSource){
			vResult.first->Get().second = *ppkgSource;
		}

		vecPackageStack.Push(&(vResult.first->Get().second));
		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};
	const auto PopPackage = [&]{
		if(vecPackageStack.GetSize() <= 1){
			eError = ERR_UNEXCEPTED_PACKAGE_CLOSE;
			return false;
		}

		vecPackageStack.Pop();
		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};
	const auto AcceptValue = [&]{
		ASSERT(!vecPackageStack.IsEmpty());

		const auto vResult = vecPackageStack.GetEnd()[-1]->Insert(Unescape(WideStringObserver(pwcNameBegin, pwcNameEnd)));
		if(!vResult.second){
//			eError = ERR_DUPLICATE_VALUE;
//			return false;
		}
		vResult.first->Get().second.Insert(Unescape(WideStringObserver(pwcValueBegin, pwcValueEnd)));

		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};

	do {
		const wchar_t wc = *pwcRead;

		switch(wc){
		case L'=':
			switch(eState){
			case NAME_INDENT:
			case NAME_BODY:
			case NAME_PADDING:
				eState = VAL_INDENT;
				continue;

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'{':
			switch(eState){
			case NAME_INDENT:
			case NAME_BODY:
			case NAME_PADDING:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'}':
			switch(eState){
			case NAME_INDENT:
				if(!PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				// eState = NAME_INDENT;
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!AcceptValue() || !PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L';':
			switch(eState){
			case NAME_INDENT:
				eState = COMMENT;
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!AcceptValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = COMMENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'\n':
			switch(eState){
			case NAME_INDENT:
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!AcceptValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
				eState = NAME_INDENT;
				continue;

			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;
		}

		switch(eState){
		case NAME_INDENT:
			switch(wc){
			case L' ':
			case L'\t':
				pwcNameBegin = pwcRead;
				pwcNameEnd = pwcRead;
				// eState = NAME_INDENT;
				break;

			default:
				pwcNameBegin = pwcRead;
				pwcNameEnd = pwcRead + 1;
				eState = (wc == L'\\') ? NAME_ESCAPED : NAME_BODY;
				break;
			}
			break;

		case NAME_BODY:
			switch(wc){
			case L' ':
			case L'\t':
				eState = NAME_PADDING;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = NAME_ESCAPED;
				}
				break;
			}
			break;

		case NAME_PADDING:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = NAME_PADDING;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				eState = (wc == L'\\') ? NAME_ESCAPED : NAME_BODY;
				break;
			}
			break;

		case VAL_INDENT:
			switch(wc){
			case L' ':
			case L'\t':
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead;
				// eState = VAL_INDENT;
				break;

			default:
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? VAL_ESCAPED : VAL_BODY;
				break;
			}
			break;

		case VAL_BODY:
			switch(wc){
			case L' ':
			case L'\t':
				eState = VAL_PADDING;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = VAL_ESCAPED;
				}
				break;
			}
			break;

		case VAL_PADDING:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = VAL_PADDING;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? VAL_ESCAPED : VAL_BODY;
				break;
			}
			break;

		case COMMENT:
			if(wc == L'\\'){
				eState = COMMENT_ESCAPED;
			}
			break;

		case NAME_ESCAPED:
			pwcNameEnd = pwcRead + 1;
			eState = NAME_BODY;
			break;

		case VAL_ESCAPED:
			pwcValueEnd = pwcRead + 1;
			eState = VAL_BODY;
			break;

		case COMMENT_ESCAPED:
			eState = COMMENT;
			break;
		}
	} while(++pwcRead != pwcEnd);

	switch(eState){
	case NAME_INDENT:
		break;

	case NAME_BODY:
	case NAME_PADDING:
	case NAME_ESCAPED:
		return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

	case VAL_INDENT:
	case VAL_BODY:
	case VAL_PADDING:
	case VAL_ESCAPED:
		if(!AcceptValue()){
			return std::make_pair(eError, pwcRead);
		}
		break;

	case COMMENT:
	case COMMENT_ESCAPED:
		break;
	};
	if(vecPackageStack.GetSize() > 1){
		return std::make_pair(ERR_UNCLOSED_PACKAGE, pwcRead);
	}

	Swap(vTemp);
	return std::make_pair(ERR_NONE, pwcRead);
}
WideString MNotation::Export(const WideStringObserver &wsoIndent) const {
	WideString wsRet;

	Vector<std::pair<const MNotationNode *, const ChildNode *>> vecPackageStack;
	vecPackageStack.Push(this, xm_mapChildren.GetFirst<1>());

	WideString wsIndent;
	for(;;){
		auto &vTop = vecPackageStack.GetEnd()[-1];

	jNextChild:
		if(vTop.second){
			const auto &wsName = vTop.second->Get().first;
			const auto &vNode = vTop.second->Get().second;
			vTop.second = vTop.second->GetNext<1>();

			wsRet += wsIndent;
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				wsRet += L' ';
			}
			if(vNode.xm_mapChildren.GetSize() == 1){
				const auto pNode = vNode.xm_mapChildren.GetFirst<0>();
				if(pNode->Get().second.xm_mapChildren.IsEmpty()){
					wsRet += L'=';
					wsRet += L' ';
					Escape(wsRet, pNode->Get().first);
					wsRet += L'\n';
					goto jNextChild;
				}
			}
			wsRet += L'{';
			wsRet += L'\n';
			wsIndent += wsoIndent;
			vecPackageStack.Push(&vNode, vNode.xm_mapChildren.GetFirst<1>());
			continue;
		}
/*		if(vTop.second){
			wsRet += wsIndent;
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				wsRet += L' ';
			}
			wsRet += L'{';
			wsRet += L'\n';
			wsIndent += wsoIndent;
			vecPackageStack.Push(&(vTop.second->Get().second), vTop.second->Get().second.xm_mapChildren.GetFirst<1>());
			continue;
		}*/
/*
		for(auto pNode = vTop.first->xm_mapValues.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
			wsRet += wsIndent;
			if(!pNode->first.IsEmpty()){
				Escape(wsRet, pNode->first);
				wsRet += L' ';
			}
		}
*/
		vecPackageStack.Pop();
		if(vecPackageStack.IsEmpty()){
			break;
		}

		wsIndent.Truncate(wsoIndent.GetLength());
		wsRet += wsIndent;
		wsRet += L'}';
		wsRet += L'\n';
	}

	ASSERT(wsIndent.IsEmpty());

	return wsRet;
}
