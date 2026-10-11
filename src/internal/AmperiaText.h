#ifndef AMPERIA_INTERNAL_TEXT_H
#define AMPERIA_INTERNAL_TEXT_H

#include <stddef.h>
#include <string.h>

namespace amperia
{
namespace detail
{

// Copy text into a fixed-size C-string field used by protocol descriptors and
// metadata. The destination is always null-terminated when its size is
// non-zero; overlong text is truncated and a null source becomes an empty
// string.
inline void copyText(char* destination,
                     size_t destinationSize,
                     const char* source)
{
    if (destination == nullptr || destinationSize == 0) {
        return;
    }

    destination[0] = '\0';
    if (source == nullptr) {
        return;
    }

    strncpy(destination, source, destinationSize - 1);
    destination[destinationSize - 1] = '\0';
}

} // namespace detail
} // namespace amperia

#endif // AMPERIA_INTERNAL_TEXT_H
