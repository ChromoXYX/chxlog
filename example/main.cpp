#include "../include/chx/log.hpp"
#include "../include/chx/log/chrono.hpp"

#include <fmt/core.h>
#include <fmt/compile.h>
#include <fmt/chrono.h>

#include <iostream>
#include <chrono>
#include <random>
#include <unistd.h>

using namespace std::literals;
using namespace fmt::literals;
using namespace chx::log::literals;

constexpr static inline char wday_name[7][4] = {"Sun", "Mon", "Tue", "Wed",
                                                "Thu", "Fri", "Sat"};
constexpr static inline char mon_name[12][4] = {"Jan", "Feb", "Mar", "Apr",
                                                "May", "Jun", "Jul", "Aug",
                                                "Sep", "Oct", "Nov", "Dec"};

int main(void) {
    auto now = std::chrono::system_clock::now();
    struct tm _tm = {};
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    localtime_r(&time, &_tm);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr('a', 'z');
    char a = distr(gen), b = distr(gen), c = distr(gen);
    std::uniform_int_distribution<long> distr2(
        std::numeric_limits<int>::max(), std::numeric_limits<long>::max());
    long integer_value = distr2(gen);

    std::string output;

    std::chrono::high_resolution_clock::time_point begin, end;

    output.resize(
        chx::log::formatted_size("hel%-+30ldlo, and to the%cw%cor%cld"_str,
                                 integer_value, a, b, c) +
        5);
    std::fill(output.begin(), output.end(), '#');
    begin = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < 100000000; ++i) {
        chx::log::format_to(output.data(), "%:%A %B %C:C"_str, _tm);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "chxlog: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       begin)
                     .count()
              << "ms\n"
              << output << "\nnow sleep for 1s\n";

    sleep(1);

    std::fill(output.begin(), output.end(), '#');
    begin = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < 100000000; ++i) {
        fmt::format_to(output.data(), FMT_COMPILE("{:%A %B %C}"), _tm);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "fmt: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       begin)
                     .count()
              << "ms\n"
              << output << "\n";
}
