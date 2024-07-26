#pragma once

#include "./carrier.hpp"

namespace chx::log {
struct color {
  private:
    template <std::size_t ColorCode>
    constexpr static inline decltype(string<'\e', '['>() +
                                     from_number<std::size_t, ColorCode>() +
                                     string<'m'>()) escape = {};

  public:
    friend struct fg;
    friend struct bg;

    struct fg {
        constexpr static auto black = color::escape<30>;
        constexpr static auto red = color::escape<31>;
        constexpr static auto green = color::escape<32>;
        constexpr static auto yellow = color::escape<33>;
        constexpr static auto blue = color::escape<34>;
        constexpr static auto magenta = color::escape<35>;
        constexpr static auto cyan = color::escape<36>;
        constexpr static auto white = color::escape<37>;

        constexpr static auto bright_black = color::escape<90>;
        constexpr static auto bright_red = color::escape<91>;
        constexpr static auto bright_green = color::escape<92>;
        constexpr static auto bright_yellow = color::escape<93>;
        constexpr static auto bright_blue = color::escape<94>;
        constexpr static auto bright_magenta = color::escape<95>;
        constexpr static auto bright_cyan = color::escape<96>;
        constexpr static auto bright_white = color::escape<97>;

        constexpr static auto end = color::escape<0>;
    };

    struct bg {
        constexpr static auto black = color::escape<40>;
        constexpr static auto red = color::escape<41>;
        constexpr static auto green = color::escape<42>;
        constexpr static auto yellow = color::escape<43>;
        constexpr static auto blue = color::escape<44>;
        constexpr static auto magenta = color::escape<45>;
        constexpr static auto cyan = color::escape<46>;
        constexpr static auto white = color::escape<47>;

        constexpr static auto bright_black = color::escape<100>;
        constexpr static auto bright_red = color::escape<101>;
        constexpr static auto bright_green = color::escape<102>;
        constexpr static auto bright_yellow = color::escape<103>;
        constexpr static auto bright_blue = color::escape<104>;
        constexpr static auto bright_magenta = color::escape<105>;
        constexpr static auto bright_cyan = color::escape<106>;
        constexpr static auto bright_white = color::escape<107>;

        constexpr static auto end = color::escape<0>;
    };
};

struct fg : color::fg {};
struct bg : color::bg {};
}  // namespace chx::log