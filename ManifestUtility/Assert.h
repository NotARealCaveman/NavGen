#include <cstdlib>

namespace Manifest_Utility
{
#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "Assertion failed: " << #expr << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            std::abort();\
        } \
    } while (0)
}