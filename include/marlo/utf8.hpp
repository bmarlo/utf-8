#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace marlo {
namespace utf8 {

inline constexpr std::uint32_t max_ascii = 0x7f;
inline constexpr std::uint32_t max_bmp = 0xffff;
inline constexpr std::uint32_t max_unicode = 0x10ffff;
inline constexpr std::uint32_t surrogate_beg = 0xd800;
inline constexpr std::uint32_t surrogate_end = 0xe000;
inline constexpr std::uint32_t bad_char = -1;

/**
 * Encodes the given code point into a utf-8 string.
 * Returns an empty string if the code point is invalid, i.e., greater than
 * 0x10ffff or in the surrogate range [0xd800, 0xdfff].
 */
std::string encode(std::uint32_t unicode);

/**
 * Encodes the given code point into `dst`.
 * Returns `false` if the code point is invalid, `true` otherwise.
 */
bool encode(std::uint32_t unicode, std::string& dst);

/**
 * Decodes the given string, returning all of its unicode code points.
 * Returns an empty vector if the string is empty or contains invalid utf-8
 * bytes.
 */
std::vector<std::uint32_t> decode(std::string_view s);

/**
 * Decodes all the code points in the given string, appending them into `dst`.
 * Returns `false` if the string contains invalid utf-8 bytes, `true` otherwise.
 * An empty string is also considered valid.
 */
bool decode(std::string_view s, std::vector<std::uint32_t>& dst);

/**
 * Whether the given string contains only valid utf-8-encoded unicode code
 * points. An empty string is also considered valid.
 */
bool validate(std::string_view s) noexcept;

/**
 * Whether the given string contains only ascii chars.
 * An empty string is also considered valid.
 */
bool is_ascii(std::string_view s) noexcept;

/**
 * Returns `true` and sets `dst` to the `ith` unicode code point in the given
 * string or returns `false` if eof or invalid utf-8 bytes are detected while
 * decoding.
 * At most `i + 1` code points are decoded starting from the beginning of the
 * string, so the remainder of the string may still contain invalid utf-8 bytes.
 */
bool char_at(std::string_view s, std::size_t i, std::uint32_t& dst) noexcept;

/**
 * Returns the `ith` unicode code point in the given string, or an empty
 * optional if eof or invalid utf-8 bytes are detected while decoding.
 * At most `i + 1` code points are decoded starting from the beginning of the
 * string, so the remainder of the string may still contain invalid utf-8 bytes.
 */
std::optional<std::uint32_t> char_at(std::string_view s, std::size_t i) noexcept;

/**
 * Returns `true` and sets `dst` to the number of code points in the given
 * string if it's valid utf-8, otherwise returns `false`.
 */
bool char_count(std::string_view s, std::size_t& dst) noexcept;

/**
 * Returns the number of code points in the given string, or an empty optional
 * if the string is not valid utf-8.
 */
std::optional<std::size_t> char_count(std::string_view s) noexcept;

/**
 * Returns `true` and sets `dst` to the code point whose encoding starts at the
 * `ith` byte of the given string, or returns `false` if eof or invalid utf-8
 * bytes are encountered.
 * On success, the index parameter is updated to one byte past the code point.
 */
bool next_char(std::string_view s, std::size_t& i, std::uint32_t& dst) noexcept;

/**
 * Returns the code point whose encoding starts at the `ith` byte of the given
 * string, or an empty optional if eof or invalid utf-8 bytes are encountered.
 * On success, the index parameter is updated to one byte past the code point.
 */
std::optional<std::uint32_t> next_char(std::string_view s, std::size_t& i) noexcept;

/**
 * Transforms all ascii chars in the given string to lowercase.
 */
void ascii_lower(std::string& s) noexcept;

/**
 * Transforms all ascii chars in the given string to uppercase.
 */
void ascii_upper(std::string& s) noexcept;

#ifdef _WIN32
/**
 * Encodes the given unicode code point, appending it to the utf-16 string.
 * Returns `false` if the code point is invalid, `true` otherwise.
 */
bool encode(std::uint32_t code, std::wstring& dst);

/**
 * Converts the given utf-8 string into a utf-16 string, returning whether it
 * succeeded.
 */
bool convert(std::string_view s, std::wstring& dst);

/**
 * Converts the given utf-16 string into a utf-8 string, returning whether it
 * succeeded.
 */
bool convert(std::wstring_view ws, std::string& dst);
#endif

inline std::string encode(std::uint32_t code)
{
    std::string s;
    encode(code, s);
    return s;
}

inline std::vector<std::uint32_t> decode(std::string_view s)
{
    std::vector<std::uint32_t> codes;
    if (decode(s, codes)) {
        return codes;
    }
    return {};
}

inline std::optional<std::uint32_t> char_at(std::string_view s, std::size_t i) noexcept
{
    std::uint32_t code;
    if (char_at(s, i, code)) {
        return code;
    }
    return std::nullopt;
}

inline std::optional<std::size_t> char_count(std::string_view s) noexcept
{
    std::size_t count;
    if (char_count(s, count)) {
        return count;
    }
    return std::nullopt;
}

inline std::optional<std::uint32_t> next_char(std::string_view s, std::size_t& i) noexcept
{
    std::uint32_t code;
    if (next_char(s, i, code)) {
        return code;
    }
    return std::nullopt;
}

}}
