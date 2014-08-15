// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STATIC_ASSERT_NOEXCEPT_HPP_
#define MCF_STATIC_ASSERT_NOEXCEPT_HPP_

namespace MCF {

namespace Impl {
	__attribute__((__error__("noexcept static assertion failed."), __noreturn__))
	void StaticAssertNoexcept() noexcept;
}

}

#define STATIC_ASSERT_NOEXCEPT_BEGIN	\
	try {

#define STATIC_ASSERT_NOEXCEPT_END	\
	} catch(...){	\
		::MCF::Impl::StaticAssertNoexcept();	\
		__builtin_unreachable();	\
	}

#define STATIC_ASSERT_NOEXCEPT_END_COND(cond)	\
	} catch(...){	\
		if(cond){	\
			::MCF::Impl::StaticAssertNoexcept();	\
		}	\
		throw;	\
	}

#define FORCE_NOEXCEPT_BEGIN	\
	[&]() noexcept {

#define FORCE_NOEXCEPT_END	\
	}();

#define FORCE_NOEXCEPT(expr)	\
	FORCE_NOEXCEPT_BEGIN	\
	{	\
		return (expr);	\
	}	\
	FORCE_NOEXCEPT_END	\

#endif