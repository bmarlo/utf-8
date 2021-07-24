#include "marlo/utf8.hpp"

namespace marlo {
namespace utf8 {

static std::pair<std::uint8_t, std::uint8_t> multibytes(std::uint32_t code)
{
    auto shifts = (code > 0x07ff && code < 0x010000) | (code > 0xffff) << 1;
    return {static_cast<std::uint8_t>(shifts), static_cast<std::uint8_t>(shifts + 2)};
}

static std::uint8_t eval_bytes(std::uint32_t code)
{
    return multibytes(code).second;
}

static std::pair<std::uint8_t, std::uint8_t> mkmask(std::uint8_t shifts)
{
    constexpr auto tmp = static_cast<std::int8_t>(0xc0);
    return {static_cast<std::uint8_t>(tmp >> shifts), static_cast<std::uint8_t>(0x1f >> shifts)};
}

static void reshift(std::uint8_t& shifts)
{
    shifts = static_cast<std::uint8_t>((shifts + 1) * 6);
}

static void unshift(std::uint8_t& shifts)
{
    shifts = static_cast<std::uint8_t>(shifts - 6);
}

static bool is_ascii(std::uint32_t code)
{
    return code <= utf8::max_ascii;
}

static bool is_surrogate(std::uint32_t code)
{
    return code >= utf8::surrogate_beg && code < utf8::surrogate_end;
}

static bool is_valid(std::uint32_t code)
{
    return code <= utf8::max_unicode && !is_surrogate(code);
}

/**
 * [0x00, 0x7f]: 0xxxxxxx, 7 bits -> 0xxxxxxx
 * [0x0080, 0x07ff]: 00000xxx xxxxxxxx, 11 bits -> 110xxxxx 10xxxxxx
 * [0x0800, 0xffff]: xxxxxxxx xxxxxxxx, 16 bits -> 1110xxxx 10xxxxxx 10xxxxxx
 * [0x010000, 0x10ffff]: 000xxxxx xxxxxxxx xxxxxxxx, 21 bits -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
static void encode_impl(std::uint32_t code, std::string& dst)
{
    if (is_ascii(code)) {
        dst.push_back(static_cast<char>(code));
        return;
    }

    auto [shifts, bytes] = multibytes(code);    // [0, 1, 2], [2, 3, 4]
    auto [high, low] = mkmask(shifts);          // leading byte mask
    reshift(shifts);        // [6, 12, 18]
    dst.push_back(static_cast<char>(high | (low & (code >> shifts))));
    while (--bytes) {       // trailing bytes
        unshift(shifts);
        dst.push_back(static_cast<char>(0x80 | (0x3f & (code >> shifts))));   // 10xxxxxx
    }
}

bool encode(std::uint32_t code, std::string& dst)
{
    if (!is_valid(code)) {
        return false;
    }

    encode_impl(code, dst);
    return true;
}

/**
 * Takes the unsigned leading byte of a multibyte utf-8 sequence right-shifted
 * by three, gives the number of bytes that make up the sequence.
 * 
 * 110xxxxx >> 3 -> 000110xx, [24, 25, 26, 27], 2 bytes
 * 1110xxxx >> 3 -> 0001110x, [28, 29], 3 bytes
 * 11110xxx >> 3 -> 00011110, [30], 4 bytes
 */
constexpr std::uint8_t mbytes[32] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 3, 3, 4, 0
};

static std::pair<std::uint8_t, std::uint8_t> multibytes(unsigned char c)
{
    auto bytes = mbytes[c >> 3];
    return {static_cast<std::uint8_t>(bytes - 2), bytes};
}

static std::uint32_t decode_impl(std::string_view u8, std::size_t& i)
{
    unsigned char c = u8[i++];
    if (is_ascii(c)) {
        return c;
    }

    constexpr std::size_t max_index = -1;
    static_assert(u8.max_size() < max_index - 2);
    auto [shifts, bytes] = multibytes(c);   // [0, 1, 2], [2, 3, 4]
    if (!bytes || u8.size() < i + bytes - 1) {
        return utf8::bad_char;
    }

    auto mask = mkmask(shifts).second;
    reshift(shifts);        // [6, 12, 18]
    std::uint32_t code = c & mask;          // leading byte
    code <<= shifts;

    while (shifts) {        // trailing bytes
        c = u8[i++];
        unshift(shifts);
        code |= (c & 0x3f) << shifts;       // 10xxxxxx
        code |= utf8::bad_char * ((c & 0xc0) != 0x80);
    }

    if (!is_valid(code) || is_ascii(code) || bytes != eval_bytes(code)) {
        return utf8::bad_char;
    }

    return code;
}

template<typename cb_t>
bool decode_impl(std::string_view u8, cb_t on_decoded)
{
    std::size_t i = 0;
    while (i < u8.size()) {
        std::uint32_t code = decode_impl(u8, i);
        if (code == utf8::bad_char) {
            return false;
        }
        on_decoded(code);
    }
    return true;
}

bool decode(std::string_view u8, std::vector<std::uint32_t>& dst)
{
    auto push = [&](std::uint32_t code) {
        dst.push_back(code);
    };
    return decode_impl(u8, std::move(push));
}

bool validate(std::string_view u8) noexcept
{
    return decode_impl(u8, [](std::uint32_t) {});
}

bool is_ascii(std::string_view s) noexcept
{
    for (unsigned char c : s) {
        if (c > utf8::max_ascii) {
            return false;
        }
    }
    return true;
}

bool char_at(std::string_view u8, std::size_t i, std::uint32_t& dst) noexcept
{
    std::size_t count = 0, byte = 0;
    while (next_char(u8, byte, dst)) {
        if (count++ == i) {
            return true;
        }
    }
    return false;
}

bool char_count(std::string_view u8, std::size_t& dst) noexcept
{
    dst = 0;
    auto count = [&](std::uint32_t) {
        ++dst;
    };
    return decode_impl(u8, std::move(count));
}

bool next_char(std::string_view u8, std::size_t& i, std::uint32_t& dst) noexcept
{
    if (i < u8.size()) {
        dst = decode_impl(u8, i);
        return dst != utf8::bad_char;
    }
    return false;
}

void ascii_lower(std::string& s) noexcept
{
    for (char& c : s) {
        auto delta = c >= 'A' && c <= 'Z' ? 32 : 0;
        c = static_cast<char>(c + delta);
    }
}

void ascii_upper(std::string& s) noexcept
{
    for (char& c : s) {
        auto delta = c >= 'a' && c <= 'z' ? -32 : 0;
        c = static_cast<char>(c + delta);
    }
}

}}
