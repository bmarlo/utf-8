## Usage

```C++
#include <marlo/utf8.hpp>
#include <iostream>

int main()
{
    std::string s = "ça marche 😎 🚀";
    std::size_t i = 0;
    std::size_t oldi = 0;
    std::uint32_t code = 0;
    while (marlo::utf8::next_char(s, i, code)) {
        auto bytes = i - oldi;      // 1, 2, 3 or 4
        std::cout << "U+" << std::hex << code << ", " << std::dec << bytes;
        std::cout << (bytes > 1 ? " bytes" : " byte") << " long" << '\n';
        oldi = i;
    }

    bool valid = code != marlo::utf8::bad_char;
    std::cout << "that is " << (valid ? "a valid" : "an invalid") << " utf-8 string" << '\n';
    if (!valid) {
        std::cout << "error starts at index " << oldi << '\n';
    }
    return 0;
}
```
