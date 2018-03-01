//
//  src/mach-o/utils/tbd-write.cc
//  tbd
//
//  Created by inoahdev on 1/7/18.
//  Copyright © 2018 inoahdev. All rights reserved.
//

#include "tbd.h"

namespace macho::utils {
    // write_to helpers

    template <typename T>
    struct stream_helper;

    template <>
    struct stream_helper<int> {
        int descriptor = -1;

        explicit inline stream_helper(int descriptor) : descriptor(descriptor) {}

        inline bool print(const char *str) const noexcept {
            return dprintf(this->descriptor, "%s", str) != -1;
        }

        __attribute__((format(printf, 2, 3))) inline auto printf(const char *str, ...) const noexcept {
            va_list list;

            va_start(list, str);
            int result = vdprintf(this->descriptor, str, list);
            va_end(list);

            return result != -1;
        }

        inline bool print(const char ch) const noexcept {
            return dprintf(this->descriptor, "%c", ch) != -1;
        }
    };

    template <>
    struct stream_helper<FILE *> {
        FILE *file = nullptr;

        explicit inline stream_helper(FILE *file) : file(file) {}

        inline bool print(const char *str) const noexcept {
            return fputs(str, this->file) != -1;
        }

        __attribute__((format(printf, 2, 3))) inline bool printf(const char *str, ...) const noexcept {
            va_list list;

            va_start(list, str);
            int result = vfprintf(this->file, str, list);
            va_end(list);

            return result != -1;
        }

        inline bool print(const char ch) const noexcept {
            return fputc(ch, this->file) != -1;
        }
    };

    template <typename T>
    tbd::write_result tbd_write_with_export_groups_to(const tbd &tbd, const stream_helper<T> &stream, const tbd::write_options &options, const std::vector<tbd::export_group> &groups) noexcept;

    template <typename T>
    bool write_architectures_to_stream(const stream_helper<T> &stream, uint64_t architectures, bool dash) noexcept;

    template <typename T>
    bool write_compatibility_version_to_stream(const stream_helper<T> &stream, const tbd::packed_version &version) noexcept;

    template <typename T>
    bool write_current_version_to_stream(const stream_helper<T> &file, const tbd::packed_version &version) noexcept;

    template <typename T>
    tbd::write_result write_exports_to_stream(const tbd &tbd, const stream_helper<T> &stream, const tbd::write_options &options, const std::vector<tbd::export_group> &groups) noexcept;

    template <typename T>
    bool write_flags_to_stream(const stream_helper<T> &stream, const struct tbd::flags &flags);

    template <typename T>
    bool write_footer_to_stream(const stream_helper<T> &stream) noexcept;

    template <typename T>
    bool write_header_to_stream(const stream_helper<T> &stream, const enum tbd::version &version) noexcept;

    template <typename T>
    bool write_install_name_to_stream(const stream_helper<T> &stream, const std::string &install_name) noexcept;

    template <typename T>
    tbd::write_result write_group_to_stream(const stream_helper<T> &stream, FILE *file, const tbd::export_group &group, const tbd::write_options &options) noexcept;

    template <typename T>
    bool write_packed_version_to_stream(const stream_helper<T> &stream, const tbd::packed_version &version) noexcept;

    template <typename T>
    bool write_parent_umbrella_to_stream(const stream_helper<T> &stream, const std::string &parent_umbrella) noexcept;

    template <typename T>
    bool write_platform_to_stream(const stream_helper<T> &stream, const enum tbd::platform &platform) noexcept;

    template <typename T>
    bool write_objc_constraint_to_stream(const stream_helper<T> &stream, const enum tbd::objc_constraint &constraint) noexcept;

    template <typename T>
    bool write_swift_version_to_stream(const stream_helper<T> &stream, const uint32_t &version) noexcept;

    static inline constexpr const auto line_length_max = 105ul;

    template <typename T>
    bool write_string_for_array_to_stream(const stream_helper<T> &stream, const std::string &string, size_t &line_length) noexcept;

    template <typename T>
    bool write_uuids_to_stream(const stream_helper<T> &stream, const std::vector<tbd::uuid_pair> &uuids, const tbd::write_options &options) noexcept;

    // write_group_to_stream helpers

    template <typename T>
    bool write_reexports_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::reexport>::const_iterator &begin, const std::vector<tbd::reexport>::const_iterator &end, uint64_t architectures) noexcept;

    template <typename T>
    bool write_normal_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept;

    template <typename T>
    bool write_objc_class_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept;

    template <typename T>
    bool write_objc_ivar_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept;

    template <typename T>
    bool write_weak_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept;

    std::vector<tbd::symbol>::const_iterator next_iterator_for_symbol(const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures, enum tbd::symbol::type type) noexcept;

    std::vector<tbd::export_group> tbd::export_groups() const noexcept {
        // Symbols created using tbd::create are kept
        // track of by a bitset of indexs into the
        // containers that hold them, but this differs
        // from the tbd-format, where symbols are tracked
        // with the *architecture* of where they're found

        // To generate export-groups, we need to convert the
        // container-index bitset to an architecture-info indexs
        // bitset but that presents an issue which may be a problem,
        // namely multiple containers with the same architecture.

        // Multiple containers with the same architecture may not
        // even be supported officially by llvm, either they don't,
        // they choose a "best" fitting architecture with some sort
        // or measurement, or combine all containers with the same
        // architecture into a single architecture export-group, which,
        // for now, is what we're doing

        // XXX: We might want to move to a different behavior for this,
        // or allow the caller to provide this different behavior, keeping
        // a default if the caller does not provide one

        auto groups = std::vector<export_group>();

        auto groups_begin = groups.begin();
        auto groups_end = groups.end();

        for (const auto &reexport : this->reexports) {
            auto groups_iter = std::find(groups_begin, groups_end, reexport);
            if (groups_iter != groups_end) {
                if (!groups_iter->reexport) {
                    groups_iter->reexport = &reexport;
                }

                continue;
            }

            groups.emplace_back(&reexport);

            groups_begin = groups.begin();
            groups_end = groups.end();
        }


        for (const auto &symbol : this->symbols) {
            auto groups_iter = std::find(groups_begin, groups_end, symbol);
            if (groups_iter != groups_end) {
                if (!groups_iter->symbol) {
                    groups_iter->symbol = &symbol;
                }

                continue;
            }

            groups.emplace_back(&symbol);

            groups_begin = groups.begin();
            groups_end = groups.end();
        }

        return groups;
    }

    tbd::tbd::write_result tbd::write_to(const char *path, const tbd::write_options &options) const noexcept {
        const auto file = fopen(path, "w");
        if (!file) {
            return write_result::failed_to_open_stream;
        }

        const auto result = this->write_to(file, options);
        fclose(file);

        return result;
    }

    tbd::write_result tbd::write_to(int descriptor, const tbd::write_options &options) const noexcept {
        return this->write_with_export_groups_to(descriptor, options, this->export_groups());
    }

    tbd::write_result tbd::write_to(FILE *file, const tbd::write_options &options) const noexcept {
        return this->write_with_export_groups_to(file, options, this->export_groups());
    }

    tbd::write_result tbd::write_with_export_groups_to(int descriptor, const write_options &options, const std::vector<export_group> &groups) const noexcept {
        return tbd_write_with_export_groups_to(*this, stream_helper<int>(descriptor), options, groups);
    }

    tbd::write_result tbd::write_with_export_groups_to(FILE *file, const write_options &options, const std::vector<export_group> &groups) const noexcept {
        return tbd_write_with_export_groups_to(*this, stream_helper<FILE *>(file), options, groups);
    }

    template <typename T>
    tbd::write_result tbd_write_with_export_groups_to(const tbd &tbd, const stream_helper<T> &stream, const tbd::write_options &options, const std::vector<tbd::export_group> &groups) noexcept {
        if (!options.ignore_header) {
            if (!write_header_to_stream(stream, tbd.version)) {
                return tbd::write_result::failed_to_write_header;
            }
        }

        if (!options.ignore_architectures) {
            if (!write_architectures_to_stream(stream, tbd.architectures, false)) {
                return tbd::write_result::failed_to_write_architectures;
            }
        }

        if (!options.ignore_uuids) {
            if (!write_uuids_to_stream(stream, tbd.uuids, options)) {
                return tbd::write_result::failed_to_write_uuids;
            }
        }

        if (!options.ignore_platform) {
            if (!write_platform_to_stream(stream, tbd.platform)) {
                return tbd::write_result::failed_to_write_platform;
            }
        }

        if (!options.ignore_flags) {
            if (!write_flags_to_stream(stream, tbd.flags)) {
                return tbd::write_result::failed_to_write_flags;
            }
        }

        if (!options.ignore_install_name) {
            if (!write_install_name_to_stream(stream, tbd.install_name)) {
                return tbd::write_result::failed_to_write_install_name;
            }
        }

        if (!options.ignore_current_version) {
            if (!write_current_version_to_stream(stream, tbd.current_version)) {
                return tbd::write_result::failed_to_write_current_version;
            }
        }

        if (!options.ignore_compatibility_version) {
            if (!write_compatibility_version_to_stream(stream, tbd.compatibility_version)) {
                return tbd::write_result::failed_to_write_compatibility_version;
            }
        }

        if (!options.ignore_swift_version) {
            if (tbd.version == tbd::version::v2 || !options.ignore_unneeded_fields_for_version) {
                if (!write_swift_version_to_stream(stream, tbd.swift_version)) {
                    return tbd::write_result::failed_to_write_swift_version;
                }
            }
        }

        if (!options.ignore_objc_constraint) {
            if (tbd.version == tbd::version::v2 || !options.ignore_unneeded_fields_for_version) {
                if (!write_objc_constraint_to_stream(stream, tbd.objc_constraint)) {
                    return tbd::write_result::failed_to_write_objc_constraint;
                }
            }
        }

        if (!options.ignore_parent_umbrella) {
            if (tbd.version == tbd::version::v2 || !options.ignore_unneeded_fields_for_version) {
                if (!write_parent_umbrella_to_stream(stream, tbd.parent_umbrella)) {
                    return tbd::write_result::failed_to_write_parent_umbrella;
                }
            }
        }

        if (!options.ignore_exports) {
            const auto result = write_exports_to_stream(tbd, stream, options, tbd.export_groups());
            if (result != tbd::write_result::ok) {
                return result;
            }
        }

        if (!options.ignore_footer) {
            if (!write_footer_to_stream(stream)) {
                return tbd::write_result::failed_to_write_footer;
            }
        }

        return tbd::write_result::ok;
    }

    template <typename T>
    bool write_architectures_to_stream(const stream_helper<T> &stream, uint64_t architectures, bool dash) noexcept {
        // Check if the relevant bits of architectures,
        // the bits 0..get_architecture_info_table_size(),
        // are empty

        const auto bit_size = sizeof(uint64_t) * 8;
        const auto architecture_info_size = get_architecture_info_table_size();

        if (!(architectures << (bit_size - architecture_info_size))) {
            return false;
        }

        // Find first architecture to write out, then
        // write out the rest with a leading comma

        auto first_architecture_index = uint64_t();
        for (; first_architecture_index < architecture_info_size; first_architecture_index++) {
            if (!(architectures & (1ull << first_architecture_index))) {
                continue;
            }

            break;
        }

        if (first_architecture_index == architecture_info_size) {
            return false;
        }

        const auto architecture_info = architecture_info_from_index(first_architecture_index);
        if (dash) {
            if (!stream.printf("  - archs:%-14s[ %s", "", architecture_info->name)) {
                return false;
            }
        } else if (!stream.printf("archs:%-17s[ %s", "", architecture_info->name)) {
            return false;
        }

        for (auto index = first_architecture_index + 1; index < architecture_info_size; index++) {
            if (!(architectures & (1ull << index))) {
                continue;
            }

            const auto architecture_info = architecture_info_from_index(index);
            if (!stream.print(architecture_info->name)) {
                return false;
            }
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_compatibility_version_to_stream(const stream_helper<T> &stream, const tbd::packed_version &version) noexcept {
        if (!stream.print("compatibility-version: ")) {
            return false;
        }

        return write_packed_version_to_stream(stream, version);
    }

    template <typename T>
    bool write_current_version_to_stream(const stream_helper<T> &stream, const tbd::packed_version &version) noexcept {
        if (!stream.printf("current-version:%-7s", "")) {
            return false;
        }

        return write_packed_version_to_stream(stream, version);
    }

    template <typename T>
    tbd::write_result write_exports_to_stream(const tbd &tbd, const stream_helper<T> &stream, const tbd::write_options &options, const std::vector<tbd::export_group> &groups) noexcept {
        // Don't check if options.ignore_exports() as
        // caller is supposed to check if it's configured

        if (options.ignore_reexports && options.ignore_normal_symbols && options.ignore_weak_symbols &&
            options.ignore_objc_class_symbols && options.ignore_objc_ivar_symbols) {
            return tbd::write_result::ok;
        }

        if (tbd.reexports.empty() && tbd.symbols.empty()) {
            if (options.enforce_has_exports) {
                return tbd::write_result::has_no_exports;
            }

            return tbd::write_result::ok;
        }

        if (groups.empty()) {
            if (options.enforce_has_exports) {
                return tbd::write_result::has_no_exports;
            }

            return tbd::write_result::ok;
        }

        if (!stream.print("exports:\n")) {
            return tbd::write_result::failed_to_write_exports;
        }

        for (const auto &group : groups) {
            const auto result = write_group_to_stream(tbd, stream, group, options);
            if (result == tbd::write_result::ok) {
                continue;
            }

            return result;
        }

        return tbd::write_result::ok;
    }

    template <typename T>
    bool write_flags_to_stream(const stream_helper<T> &stream, const struct tbd::flags &flags) {
        if (flags.has_none()) {
            return true;
        }

        if (!stream.printf("flags:%-17s[ ", "")) {
            return false;
        }

        if (flags.flat_namespace) {
            if (!stream.print("flat_namespace")) {
                return false;
            }
        }

        if (flags.not_app_extension_safe) {
            if (flags.flat_namespace) {
                if (!stream.print(", ")) {
                    return false;
                }
            }

            if (!stream.print("not_app_extension_safe")) {
                return false;
            }
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_footer_to_stream(const stream_helper<T> &stream) noexcept {
        return stream.print("...\n");
    }

    template <typename T>
    bool write_header_to_stream(const stream_helper<T> &stream, const enum tbd::version &version) noexcept {
        if (!stream.print("---")) {
            return false;
        }

        if (version == tbd::version::v2) {
            if (!stream.print(" !tapi-tbd-v2")) {
                return false;
            }
        }

        if (!stream.print('\n')) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_install_name_to_stream(const stream_helper<T> &stream, const std::string &install_name) noexcept {
        if (install_name.empty()) {
            return true;
        }

        return stream.printf("install-name:\'%-10s%s\'\n", "", install_name.c_str());
    }

    template <typename T>
    tbd::write_result write_group_to_stream(const tbd &tbd, const stream_helper<T> &stream, const tbd::export_group &group, const tbd::write_options &options) noexcept {
        // We have to check that atleast one category is written because
        // we need to write the architectures list of this group first
        // and we can't just write architectures and nothing else

        if (options.ignore_allowable_clients && options.ignore_reexports && options.ignore_normal_symbols &&
            options.ignore_weak_symbols && options.ignore_objc_class_symbols && options.ignore_objc_ivar_symbols) {
            return tbd::write_result::ok;
        }

        const auto architectures = group.architectures();
        const auto symbols_end = tbd.symbols.cend();

        auto group_symbols_begin = symbols_end;
        if (group.symbol != nullptr) {
            group_symbols_begin = tbd.symbols.cbegin() + std::distance(tbd.symbols.data(), group.symbol);
        } else {
            group_symbols_begin = std::find(tbd.symbols.cbegin(), symbols_end, group.architectures());
        }

        auto normal_symbols_iter = symbols_end;
        auto objc_class_symbols_iter = symbols_end;
        auto objc_ivar_symbols_iter = symbols_end;
        auto weak_symbols_iter = symbols_end;

        if (group_symbols_begin != symbols_end) {
            const auto symbols_begin = group_symbols_begin + 1;

            if (group.symbol != nullptr) {
                switch (group.symbol->type) {
                    case tbd::symbol::type::normal:
                        normal_symbols_iter = group_symbols_begin;
                        break;

                    case tbd::symbol::type::objc_class:
                        objc_class_symbols_iter = group_symbols_begin;
                        break;

                    case tbd::symbol::type::objc_ivar:
                        objc_ivar_symbols_iter = group_symbols_begin;
                        break;

                    case tbd::symbol::type::weak:
                        weak_symbols_iter = group_symbols_begin;
                        break;
                }
            }

            const auto architectures = group.architectures();
            if (normal_symbols_iter != group_symbols_begin) {
                normal_symbols_iter = next_iterator_for_symbol(symbols_begin, symbols_end, architectures, tbd::symbol::type::normal);
            }

            if (objc_class_symbols_iter != group_symbols_begin) {
                objc_class_symbols_iter = next_iterator_for_symbol(symbols_begin, symbols_end, architectures, tbd::symbol::type::objc_class);
            }

            if (objc_ivar_symbols_iter != group_symbols_begin) {
                objc_ivar_symbols_iter = next_iterator_for_symbol(symbols_begin, symbols_end, architectures, tbd::symbol::type::objc_ivar);
            }

            if (weak_symbols_iter != group_symbols_begin) {
                weak_symbols_iter = next_iterator_for_symbol(symbols_begin, symbols_end, architectures, tbd::symbol::type::weak);
            }
        }

        // Like above, we have to check that atleast one category is written
        // because we need to write the architectures list of this group first
        // and we can't just write architectures and nothing else

        // We need this check because although group is guaranteed to have one
        // symbol, it may have been ignored by its type and options, so we need
        // to check each

        const auto reexports_begin = tbd.reexports.cbegin();
        const auto reexports_end = tbd.reexports.cend();

        auto reexports_iter = reexports_end;
        if (group.reexport != nullptr) {
            reexports_iter = reexports_begin + std::distance(tbd.reexports.data(), group.reexport);
        } else {
            reexports_iter = std::find(reexports_begin, reexports_end, architectures);
        }

        if ((!options.ignore_reexports && reexports_iter == reexports_end) &&
            (!options.ignore_normal_symbols && normal_symbols_iter == symbols_end) &&
            (!options.ignore_objc_class_symbols && objc_class_symbols_iter == symbols_end) &&
            (!options.ignore_objc_ivar_symbols && objc_ivar_symbols_iter == symbols_end) &&
            (!options.ignore_weak_symbols && weak_symbols_iter == symbols_end)) {
            return tbd::write_result::ok;
        }

        if (!write_architectures_to_stream(stream, architectures, true)) {
            return tbd::write_result::failed_to_write_architectures;
        }

        if (reexports_iter != reexports_end) {
            if (!write_reexports_array_to_stream(stream, reexports_iter, reexports_end, architectures)) {
                return tbd::write_result::failed_to_write_reexports;
            }
        }

        if (normal_symbols_iter != symbols_end) {
            if (!write_normal_symbols_array_to_stream(stream, normal_symbols_iter, symbols_end, architectures)) {
                return tbd::write_result::failed_to_write_normal_symbols;
            }
        }

        if (objc_class_symbols_iter != symbols_end) {
            if (!write_objc_class_symbols_array_to_stream(stream, objc_class_symbols_iter, symbols_end, architectures)) {
                return tbd::write_result::failed_to_write_objc_class_symbols;
            }
        }

        if (objc_ivar_symbols_iter != symbols_end) {
            if (!write_objc_ivar_symbols_array_to_stream(stream, objc_ivar_symbols_iter, symbols_end, architectures)) {
                return tbd::write_result::failed_to_write_objc_ivar_symbols;
            }
        }

        if (weak_symbols_iter != symbols_end) {
            if (!write_weak_symbols_array_to_stream(stream, weak_symbols_iter, symbols_end, architectures)) {
                return tbd::write_result::failed_to_write_weak_symbols;
            }
        }

        return tbd::write_result::ok;
    }

    template <typename T>
    bool write_packed_version_to_stream(const stream_helper<T> &stream, const tbd::packed_version &version) noexcept {
        if (!stream.printf("%u", version.components.major)) {
            return false;
        }

        if (version.components.minor != 0) {
            if (!stream.printf(".%u", version.components.minor)) {
                return false;
            }
        }

        if (version.components.revision != 0) {
            if (version.components.minor == 0) {
                if (!stream.print(".0")) {
                    return false;
                }
            }

            if (!stream.printf(".%u", version.components.revision)) {
                return false;
            }
        }

        if (!stream.print('\n')) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_parent_umbrella_to_stream(const stream_helper<T> &stream, const std::string &parent_umbrella) noexcept {
        if (parent_umbrella.empty()) {
            return true;
        }

        return stream.printf("parent-umbrella:%-7s%s\n", "", parent_umbrella.c_str());
    }

    template <typename T>
    bool write_platform_to_stream(const stream_helper<T> &stream, const enum tbd::platform &platform) noexcept {
        if (platform == tbd::platform::none) {
            return true;
        }

        return stream.printf("platform:%-14s%s\n", "", tbd::platform_to_string(platform));
    }

    template <typename T>
    bool write_objc_constraint_to_stream(const stream_helper<T> &stream, const enum tbd::objc_constraint &constraint) noexcept {
        if (constraint == tbd::objc_constraint::none) {
            return true;
        }

        return stream.printf("objc-constraint:%-7s%s\n", "", tbd::objc_constraint_to_string(constraint));
    }

    template <typename T>
    bool write_swift_version_to_stream(const stream_helper<T> &stream, const uint32_t &version) noexcept {
        if (version == 0) {
            return true;
        }

        if (!stream.printf("swift-version:%-9s", "")) {
            return false;
        }

        switch (version) {
            case 1:
                if (!stream.print("1\n")) {
                    return false;
                }

                break;

            case 2:
                if (!stream.print("1.2\n")) {
                    return false;
                }

                break;

            default:
                if (!stream.printf("%d\n", version - 1)) {
                    return false;
                }

                break;
        }

        return true;
    }

    template <typename T>
    bool write_string_for_array_to_stream(const stream_helper<T> &stream, const std::string &string, size_t &line_length) noexcept {
        const auto total_string_length = string.length() + 2; // comma + trailing space

        // Go to a new-line either if the current-line is or is about to be
        // used up, or if the current-string itself is larger than line-length
        // max

        // For strings that larger than line-length-max, they are to be on a separate line
        // by themselves, serving as the only exception to the rule that lines can go upto
        // a certain length

        auto new_line_length = line_length + total_string_length;
        if (line_length != 0) {
            if (new_line_length >= line_length_max) {
                if (!stream.printf(",\n%-26s", "")) {
                    return false;
                }

                new_line_length = total_string_length;
            } else {
                if (!stream.print(", ")) {
                    return false;
                }
            }
        }

        // Strings are printed with quotations
        // if they start out with $ld in Apple's
        // official, not sure if needed though

        if (strncmp(string.c_str(), "$ld", 3) == 0) {
            if (!stream.printf("\'%s\'", string.c_str())) {
                return false;
            }
        } else {
            if (!stream.printf("%s", string.c_str())) {
                return false;
            }
        }

        line_length = new_line_length;
        return true;
    }

    template <typename T>
    bool write_uuids_to_stream(const stream_helper<T> &stream, const std::vector<tbd::uuid_pair> &uuids, const tbd::write_options &options) noexcept {
        if (uuids.empty()) {
            return true;
        }

        if (!stream.printf("uuids:%-17s[ ", "")) {
            return false;
        }

        auto tracker = uint64_t();

        const auto uuids_begin = uuids.cbegin();
        const auto uuids_end = uuids.cend();

        const auto uuids_size = uuids.size();

        if (options.order_by_architecture_info_table) {
            auto null_architectures_pair = tbd::uuid_pair();
            null_architectures_pair.architecture = nullptr;

            auto last_pair = const_cast<const tbd::uuid_pair *>(&null_architectures_pair);
            auto pair = uuids_begin;

            do {
                for (auto iter = uuids_begin; iter != uuids_end; iter++) {
                    if (pair->architecture < iter->architecture) {
                        continue;
                    }

                    if (last_pair->architecture >= iter->architecture) {
                        continue;
                    }

                    pair = iter;
                }

                const auto &uuid = pair->uuid();
                const auto result = stream.printf("'%s: %.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X'", pair->architecture->name,
                                                    uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8],
                                                    uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
                if (!result) {
                    return false;
                }

                tracker++;
                if (tracker != uuids_size) {
                    if (!stream.print(", ")) {
                        return false;
                    }

                    if (!(tracker & 1)) {
                        if (!stream.printf("%-26s", "\n")) {
                            return false;
                        }
                    }
                }

                last_pair = pair.base();
                pair = uuids_begin;
            } while (tracker != uuids_size);
        } else {
            for (auto pair = uuids.cbegin(); pair != uuids_end; pair++) {
                const auto &uuid = pair->uuid();
                const auto result = stream.printf("'%s: %.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X'", pair->architecture->name,
                                                    uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8],
                                                    uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
                if (result) {
                    return false;
                }

                tracker++;
                if (tracker != uuids_size) {
                    if (!stream.print(", ")) {
                        return false;
                    }

                    if (!(tracker & 1)) {
                        if (!stream.printf("%-26s", "\n")) {
                            return false;
                        }
                    }
                }
            }
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_reexports_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::reexport>::const_iterator &begin, const std::vector<tbd::reexport>::const_iterator &end, uint64_t architectures) noexcept {
        if (!stream.printf("%-4sre-exports:%9s[ ", "", "")) {
            return false;
        }

        auto line_length = size_t();
        for (auto iter = begin; iter != end; ) {
            if (!write_string_for_array_to_stream(stream, iter->string, line_length)) {
                return false;
            }

            iter++;
            iter = std::find(iter, end, architectures);
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_normal_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept {
        if (!stream.printf("%-4ssymbols:%12s[ ", "", "")) {
            return false;
        }

        auto line_length = size_t();
        for (auto iter = begin; iter != end; ) {
            if (!write_string_for_array_to_stream(stream, iter->string, line_length)) {
                return false;
            }

            iter++;
            iter = next_iterator_for_symbol(iter, end, architectures, tbd::symbol::type::normal);
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_objc_class_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept {
        if (!stream.printf("%-4sobjc-classes:%7s[ ", "", "")) {
            return false;
        }

        auto line_length = size_t();
        for (auto iter = begin; iter != end; ) {
            if (!write_string_for_array_to_stream(stream, iter->string, line_length)) {
                return false;
            }

            iter++;
            iter = next_iterator_for_symbol(iter, end, architectures, tbd::symbol::type::objc_class);
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_objc_ivar_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept {
        if (!stream.printf("%-4sobjc-ivars:%9s[ ", "", "")) {
            return false;
        }

        auto line_length = size_t();
        for (auto iter = begin; iter != end; ) {
            if (!write_string_for_array_to_stream(stream, iter->string, line_length)) {
                return false;
            }

            iter++;
            iter = next_iterator_for_symbol(iter, end, architectures, tbd::symbol::type::objc_ivar);
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    template <typename T>
    bool write_weak_symbols_array_to_stream(const stream_helper<T> &stream, const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures) noexcept {
        if (!stream.printf("%-4sweak-def-symbols:%3s[ ", "", "")) {
            return false;
        }

        auto line_length = size_t();
        for (auto iter = begin; iter != end; ) {
            if (!write_string_for_array_to_stream(stream, iter->string, line_length)) {
                return false;
            }

            iter++;
            iter = next_iterator_for_symbol(iter, end, architectures, tbd::symbol::type::weak);
        }

        if (!stream.print(" ]\n")) {
            return false;
        }

        return true;
    }

    std::vector<tbd::symbol>::const_iterator next_iterator_for_symbol(const std::vector<tbd::symbol>::const_iterator &begin, const std::vector<tbd::symbol>::const_iterator &end, uint64_t architectures, enum tbd::symbol::type type) noexcept {
        auto iter = begin;
        for (; iter != end; iter++) {
            if (iter->type != type) {
                continue;
            }

            if (iter->architectures != architectures) {
                continue;
            }

            break;
        }

        return iter;
    }
}
