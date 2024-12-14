#ifndef STR_UTILS_HPP
#define STR_UTILS_HPP

#include <cassert>
#include <cstdlib>
#include <cwctype>
#include <string>

namespace utils {

std::u16string make_u16string(const std::wstring &ws);
/* Creates a UTF-16 string from a wide-character string.  Any wide
 * characters outside the allowed range of UTF-16 are mapped to the sentinel
 * value U+FFFD, per the Unicode documentation.
 * (http://www.unicode.org/faq/private_use.html retrieved 12 March 2017.)
 * Unpaired surrogates in ws are also converted to sentinel values.
 * Noncharacters, however, are left intact.  As a fallback, if wide
 * characters are the same size as char16_t, this does a more trivial
 * construction using that implicit conversion.
 */
}  // namespace utils
#endif
