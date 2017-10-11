//
//  src/mach-o/container.h
//  tbd
//
//  Created by inoahdev on 4/24/17.
//  Copyright © 2017 inoahdev. All rights reserved.
//

#pragma once

#include <cstdint>
#include <functional>

#include "swap.h"

namespace macho {
    class container {
    public:
        explicit container() = default;

        explicit container(const container &) = delete;
        explicit container(container &&) noexcept;

        container &operator=(const container &) = delete;
        container &operator=(container &&) noexcept;

        ~container();

        FILE *stream = nullptr;

        long base = 0;
        size_t size = 0;

        struct header header = {};

        enum class open_result {
            ok,
            invalid_range,

            stream_seek_error,
            stream_read_error,

            fat_container,

            not_a_macho,
            invalid_macho,

            not_a_library,
            not_a_dynamic_library
        };

        open_result open(FILE *stream, long base = 0, size_t size = 0) noexcept;
        open_result open_from_library(FILE *stream, long base = 0, size_t size = 0) noexcept;
        open_result open_from_dynamic_library(FILE *stream, long base = 0, size_t size = 0) noexcept;

        open_result open_copy(const container &container) noexcept;

        enum class load_command_iteration_result {
            ok,
            no_load_commands,
            stream_seek_error,
            stream_read_error,
            load_command_is_too_small,
            load_command_is_too_large
        };

        struct load_command *find_first_of_load_command(load_commands cmd, load_command_iteration_result *result = nullptr);
        load_command_iteration_result iterate_load_commands(const std::function<bool(long, const struct load_command *, const struct load_command *)> &callback) noexcept;

        enum class symbols_iteration_result {
            ok,

            stream_seek_error,
            stream_read_error,

            no_symbol_table_load_command,
            invalid_symbol_table_load_command,

            no_symbols,
            invalid_symbol_table_entry
        };

        symbols_iteration_result iterate_symbols(const std::function<bool(const struct nlist_64 &, const char *)> &callback) noexcept;

        inline const bool is_big_endian() const noexcept { return header.magic == magic::big_endian || header.magic == magic::bits64_big_endian; }

        inline const bool is_32_bit() const noexcept { return magic_is_32_bit(header.magic); }
        inline const bool is_64_bit() const noexcept { return magic_is_64_bit(header.magic); }

    private:
        uint8_t *cached_load_commands_ = nullptr;
        uint8_t *cached_symbol_table_ = nullptr;

        char *cached_string_table_ = nullptr;
        size_t file_size(open_result &result) noexcept;

        enum class validation_type : uint64_t {
            none,
            library,
            dynamic_library
        };

        open_result validate_and_load_data(validation_type type = validation_type::none) noexcept;
    };
}
