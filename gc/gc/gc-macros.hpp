#pragma once


#ifdef _MSC_VER // Microsoft compilers

#   define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#   define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define INTERNAL_EXPAND(x) x
#   define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#else // Non-Microsoft compilers

#   define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_0(WHAT, X)
#define FOR_EACH_1(WHAT, X)       WHAT(X)
#define FOR_EACH_2(WHAT, X, ...)  WHAT(X) FOR_EACH_1 (WHAT, __VA_ARGS__)
#define FOR_EACH_3(WHAT, X, ...)  WHAT(X) FOR_EACH_2 (WHAT, __VA_ARGS__)
#define FOR_EACH_4(WHAT, X, ...)  WHAT(X) FOR_EACH_3 (WHAT, __VA_ARGS__)
#define FOR_EACH_5(WHAT, X, ...)  WHAT(X) FOR_EACH_4 (WHAT, __VA_ARGS__)
#define FOR_EACH_6(WHAT, X, ...)  WHAT(X) FOR_EACH_5 (WHAT, __VA_ARGS__)
#define FOR_EACH_7(WHAT, X, ...)  WHAT(X) FOR_EACH_6 (WHAT, __VA_ARGS__)
#define FOR_EACH_8(WHAT, X, ...)  WHAT(X) FOR_EACH_7 (WHAT, __VA_ARGS__)
#define FOR_EACH_9(WHAT, X, ...)  WHAT(X) FOR_EACH_8 (WHAT, __VA_ARGS__)
#define FOR_EACH_10(WHAT, X, ...) WHAT(X) FOR_EACH_9 (WHAT, __VA_ARGS__)
#define FOR_EACH_11(WHAT, X, ...) WHAT(X) FOR_EACH_10(WHAT, __VA_ARGS__)
#define FOR_EACH_12(WHAT, X, ...) WHAT(X) FOR_EACH_11(WHAT, __VA_ARGS__)
#define FOR_EACH_13(WHAT, X, ...) WHAT(X) FOR_EACH_12(WHAT, __VA_ARGS__)
#define FOR_EACH_14(WHAT, X, ...) WHAT(X) FOR_EACH_13(WHAT, __VA_ARGS__)
#define FOR_EACH_15(WHAT, X, ...) WHAT(X) FOR_EACH_14(WHAT, __VA_ARGS__)
#define FOR_EACH_16(WHAT, X, ...) WHAT(X) FOR_EACH_15(WHAT, __VA_ARGS__)

#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, ...) FOR_EACH_(GET_ARG_COUNT(__VA_ARGS__), what, __VA_ARGS__)

#define VISIT_GC_BASE(BASE) \
	BASE::_gc_trace(visitor__);

#define VISIT_GC_BASES(...) \
	FOR_EACH(VISIT_GC_BASE, __VA_ARGS__)

#define VISIT_GC_BASES_IND(...) \
	VISIT_GC_BASES __VA_ARGS__

#define VISIT_GC_MEMBER(PROTECTION, TYPE, MEMBER) \
	visitor__->trace(MEMBER);

#define VISIT_GC_MEMBER_IND(...) \
	VISIT_GC_MEMBER __VA_ARGS__

#define VISIT_GC_MEMBERS(...) \
	FOR_EACH(VISIT_GC_MEMBER_IND, __VA_ARGS__)

#define VISIT_GC_MEMBERS_IND(...) \
	VISIT_GC_MEMBERS __VA_ARGS__

#define DECLARE_GC_MEMBER(PROTECTION, TYPE, NAME) \
	PROTECTION: garbage_collection::member<TYPE> NAME;

#define DECLARE_GC_MEMBER_IND(...) \
	DECLARE_GC_MEMBER __VA_ARGS__

#define DECLARE_GC_MEMBERS(...) \
	FOR_EACH(DECLARE_GC_MEMBER_IND, __VA_ARGS__)

#define DECLARE_GC_MEMBERS_IND(...) \
	DECLARE_GC_MEMBERS __VA_ARGS__

#define DEFINE_GC_MEMBERS(BASES, MEMBERS) \
	DECLARE_GC_MEMBERS_IND(MEMBERS) \
	public: \
	virtual void _gc_trace(gsl::not_null<garbage_collection::visitor*> visitor__) const noexcept override { \
		VISIT_GC_MEMBERS_IND(MEMBERS) \
		VISIT_GC_BASES_IND(BASES) \
	}
