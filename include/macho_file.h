//
//  include/macho_file.h
//  tbd
//
//  Created by inoahdev on 11/18/18.
//  Copyright © 2018 inoahdev. All rights reserved.
//

#ifndef MACHO_FILE_H
#define MACHO_FILE_H

#include <stdio.h>
#include "mach-o/loader.h"

#include "array.h"
#include "tbd.h"

enum macho_file_options {
    /*
     * Ignore any errors where an invalid field was produced.
     */

    O_MACHO_FILE_IGNORE_INVALID_FIELDS     = 1 << 0,
    O_MACHO_FILE_IGNORE_CONFLICTING_FIELDS = 1 << 1
};

struct macho_arch_group_specific_info {
    uint64_t archs;
    const char *info;  
};

/*
 * Keep a "cache" to store data necessary for parsing a mach-o file.
 */

struct macho_file_cache {
    uint8_t *load_command_buffer;
    char *string_table;
};

enum macho_file_parse_result {
    E_MACHO_FILE_PARSE_OK,

    E_MACHO_FILE_PARSE_SEEK_FAIL,
    E_MACHO_FILE_PARSE_READ_FAIL,

    E_MACHO_FILE_PARSE_NOT_A_MACHO,
    E_MACHO_FILE_PARSE_SIZE_TOO_SMALL,

    E_MACHO_FILE_PARSE_UNSUPPORTED_CPUTYPE,

    E_MACHO_FILE_PARSE_NO_ARCHITECTURES,
    E_MACHO_FILE_PARSE_TOO_MANY_ARCHITECTURES,

    E_MACHO_FILE_PARSE_INVALID_ARCHITECTURE,
    E_MACHO_FILE_PARSE_OVERLAPPING_ARCHITECTURES,

    E_MACHO_FILE_PARSE_MULTIPLE_ARCHS_FOR_CPUTYPE,

    E_MACHO_FILE_PARSE_ALLOC_FAIL,
    E_MACHO_FILE_PARSE_ARRAY_FAIL,

    E_MACHO_FILE_PARSE_NO_LOAD_COMMANDS,
    E_MACHO_FILE_PARSE_TOO_MANY_LOAD_COMMANDS,

    E_MACHO_FILE_PARSE_LOAD_COMMANDS_AREA_TOO_SMALL,
    E_MACHO_FILE_PARSE_INVALID_LOAD_COMMAND,

    E_MACHO_FILE_PARSE_TOO_MANY_SECTIONS,
    E_MACHO_FILE_PARSE_INVALID_SECTION,

    E_MACHO_FILE_PARSE_INVALID_CLIENT,
    E_MACHO_FILE_PARSE_INVALID_INSTALL_NAME,
    E_MACHO_FILE_PARSE_INVALID_PARENT_UMBRELLA,
    E_MACHO_FILE_PARSE_INVALID_PLATFORM,
    E_MACHO_FILE_PARSE_INVALID_REEXPORT,
    E_MACHO_FILE_PARSE_INVALID_SYMBOL_TABLE,
    E_MACHO_FILE_PARSE_INVALID_UUID,

    E_MACHO_FILE_PARSE_CONFLICTING_ARCH_INFO,
    E_MACHO_FILE_PARSE_CONFLICTING_FLAGS,
    E_MACHO_FILE_PARSE_CONFLICTING_IDENTIFICATION,
    E_MACHO_FILE_PARSE_CONFLICTING_OBJC_CONSTRAINT,
    E_MACHO_FILE_PARSE_CONFLICTING_PARENT_UMBRELLA,
    E_MACHO_FILE_PARSE_CONFLICTING_PLATFORM,
    E_MACHO_FILE_PARSE_CONFLICTING_SWIFT_VERSION,
    E_MACHO_FILE_PARSE_CONFLICTING_UUID,

    E_MACHO_FILE_PARSE_NO_IDENTIFICATION,
    E_MACHO_FILE_PARSE_NO_SYMBOL_TABLE,
    E_MACHO_FILE_PARSE_NO_UUID,
    
    E_MACHO_FILE_PARSE_NO_EXPORTS
};

enum macho_file_parse_result
macho_file_parse_from_file(struct tbd_create_info *info,
                           int fd,
                           uint64_t parse_options,
                           uint64_t options);

enum macho_file_parse_result
macho_file_parse_from_range(struct tbd_create_info *info,
                            int fd,
                            uint64_t start,
                            uint64_t end,
                            uint64_t parse_options,
                            uint64_t options);

void macho_file_print_archs(int fd);

#endif /* MACHO_FILE_H */
