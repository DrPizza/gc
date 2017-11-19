#pragma once

#include "targetver.h"

#define STRICT
#define NOMINMAX

#pragma warning(disable: 4668) // warning C4668: '%s' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable: 4710) // warning C4710: '%s': function not inlined
#pragma warning(disable: 4711) // warning C4711: function '%s' selected for automatic inline expansion
#pragma warning(disable: 4820) // warning C4820: '%s': '%d' bytes padding added after data member '%s'

#include <Windows.h>

#if !defined(_STL_EXTRA_DISABLED_WARNINGS)
#define _STL_EXTRA_DISABLED_WARNINGS 4061 4324 4365 4571 4582 4583 4623 4625 4626 4710 4774 4820 4987 5026 5027
#endif

#if !defined(_SCL_SECURE_NO_WARNINGS)
#define _SCL_SECURE_NO_WARNINGS 1
#endif

#include <iostream>
