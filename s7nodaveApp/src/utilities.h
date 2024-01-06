#ifndef s7nodaveUtilities_h
#define s7nodaveUtilities_h

#include <string>
#include <vector>

#include "s7nodaveAPI.h"

namespace s7nodave {

/**
 * Splits a string into substrings that are separated by a delimiter.
 */
S7NODAVE_API std::vector<std::string> splitString(const std::string& str, char delimiter);

/**
 * Converts a string to lower case (in place).
 */
S7NODAVE_API void toLower(std::string& str);

/**
 * Converts a string to upper case (in place).
 */
S7NODAVE_API void toUpper(std::string& str);

/**
 * Trims all whitespace on both sides (left and right) of a string (in place).
 */
S7NODAVE_API void trim(std::string& str);

/**
 * Trims all whitespace on the left side of a string (in place).
 */
S7NODAVE_API void trimLeft(std::string& str);

/**
 * Trims all whitespace on the right side of a string (in place).
 */
S7NODAVE_API void trimRight(std::string& str);

} // namespace s7nodave

#endif // s7nodaveUtilities_h
