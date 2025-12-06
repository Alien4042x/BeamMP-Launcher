/*
 Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.
 Licensed under AGPL-3.0 (or later), see <https://www.gnu.org/licenses/>.
 SPDX-License-Identifier: AGPL-3.0-or-later
*/


#include "Logger.h"
#include <span>
#include <vector>
#include <zconf.h>
#include <zlib.h>
#ifdef __linux__
#include <cstring>
#endif

std::vector<char> Comp(std::span<const char> input) {
    auto max_size = compressBound(input.size());
    std::vector<char> output(max_size);
    uLongf output_size = output.size();
    int res = compress(
        reinterpret_cast<Bytef*>(output.data()),
        &output_size,
        reinterpret_cast<const Bytef*>(input.data()),
        static_cast<uLongf>(input.size()));
    if (res != Z_OK) {
        error("zlib compress() failed (code: " + std::to_string(res) + ", message: " + zError(res) + ")");
        throw std::runtime_error("zlib compress() failed");
    }
    debug("zlib compressed " + std::to_string(input.size()) + " B to " + std::to_string(output_size) + " B");
    output.resize(output_size);
    return output;
}

std::vector<char> DeComp(std::span<const char> input) {
    z_stream strm{};
    strm.next_in = (Bytef*)input.data();
    strm.avail_in = input.size();

    // Start with a large buffer - 256KB
    std::vector<char> output(256 * 1024);

    strm.next_out = (Bytef*)output.data();
    strm.avail_out = output.size();

    // 15 = zlib header (zlib stream)
    inflateInit2(&strm, 15);

    int ret;
    while ((ret = inflate(&strm, Z_NO_FLUSH)) == Z_OK) {
        if (strm.avail_out == 0) {
            size_t oldSize = output.size();
            output.resize(oldSize * 2);
            strm.next_out = (Bytef*)(output.data() + oldSize);
            strm.avail_out = oldSize;
        }
    }

    inflateEnd(&strm);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("inflate failed");
    }

    output.resize(strm.total_out);
    return output;
}
