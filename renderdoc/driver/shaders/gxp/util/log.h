// Vita3K emulator project
// Copyright (C) 2018 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#pragma once

#include <type_traits>

#include <iomanip>
#include <iostream>
#include <vector>

namespace logging {

#define LOG_TRACE printf
#define LOG_DEBUG printf
#define LOG_INFO printf
#define LOG_WARN printf
#define LOG_ERROR printf
#define LOG_CRITICAL printf

#define LOG_TRACE_IF(flag, ...) \
    if (flag)                   \
    LOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG_IF(flag, ...) \
    if (flag)                   \
    LOG_DEBUG(__VA_ARGS__)
#define LOG_INFO_IF(flag, ...) \
    if (flag)                  \
    LOG_INFO(__VA_ARGS__)
#define LOG_WARN_IF(flag, ...) \
    if (flag)                  \
    LOG_WARN(__VA_ARGS__)
#define LOG_ERROR_IF(flag, ...) \
    if (flag)                   \
    LOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL_IF(flag, ...) \
    if (flag)                      \
    LOG_CRITICAL(__VA_ARGS__)

int ret_error_impl(const char *name, const char *error_str, std::uint32_t error_val);
} // namespace logging

#define RET_ERROR(error) logging::ret_error_impl(export_name, #error, error)

template <typename T>
std::string log_hex(T val) {
    using unsigned_type = typename std::make_unsigned<T>::type;
    //return fmt::format("0x{:0X}", static_cast<unsigned_type>(val));
    return std::to_string(static_cast<unsigned_type>(val));
}
