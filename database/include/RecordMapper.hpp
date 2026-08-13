#pragma once

#include "DBConnection.hpp"

#include <charconv>
#include <cstdint>
#include <string>

namespace arcane::database::detail {

inline std::string value(const QueryRow& row, const char* column)
{
    const auto iterator = row.find(column);
    return iterator == row.end() ? std::string{} : iterator->second;
}

template <typename Integer>
Integer integer(const QueryRow& row, const char* column)
{
    Integer parsed{};
    const auto text = value(row, column);
    const auto [pointer, error] = std::from_chars(text.data(), text.data() + text.size(), parsed);
    return error == std::errc{} && pointer == text.data() + text.size() ? parsed : Integer{};
}

inline bool boolean(const QueryRow& row, const char* column)
{
    const auto text = value(row, column);
    return text == "1" || text == "true";
}

} // namespace arcane::database::detail
