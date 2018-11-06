//
// LICENSE:
//
// Copyright (c) 2016 -- 2018 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

//
// Compare two images are returns either 0 or 1.
//

#include "../yocto/ygl.h"
#include "../yocto/yglio.h"
using namespace ygl;

template <typename T>
void compute_diff_image(image<vec<T, 4>>& diff,
    const image<vec<T, 4>>& a, const image<vec<T, 4>>& b) {
    init_image(diff, a.size);
    for (auto i = 0; i < a.size.x * a.size.y; i++) {
        diff.pixels[i] = {(T)abs(a.pixels[i].x - b.pixels[i].x),
            (T)abs(a.pixels[i].y - b.pixels[i].y),
            (T)abs(a.pixels[i].z - b.pixels[i].z),
            (T)abs(a.pixels[i].w - b.pixels[i].w)};
    }
}

template <typename T>
vec<T, 4> max_diff_value(const image<vec<T, 4>>& diff) {
    auto max_value = vec<T, 4>{0, 0, 0, 0};
    for (auto c : diff.pixels) {
        max_value = {max(c.x, max_value.x), max(c.y, max_value.y),
            max(c.z, max_value.z), max(c.w, max_value.w)};
    }
    return max_value;
}

template <typename T>
void display_diff(image<vec<T, 4>>& display, const image<vec<T, 4>>& diff, T alpha) {
    init_image(display, diff.size);
    for (auto i = 0; i < diff.pixels.size(); i++) {
        auto diff_value   = max(diff.pixels[i]);
        display.pixels[i] = {diff_value, diff_value, diff_value, alpha};
    }
}

int main(int argc, char* argv[]) {
    // parse command line
    auto parser = make_cmdline_parser(
        argc, argv, "Compares two images", "yimdiff");
    auto threshold = parse_arg(parser, "--threshold,-t", 0.1f, "Thhhreshold");
    auto output    = parse_arg(
        parser, "--output,-o", ""s, "output image filename", false);
    auto filename1 = parse_arg(
        parser, "filename1", "in1.png"s, "input image filename", true);
    auto filename2 = parse_arg(
        parser, "filename2", "in2.png"s, "input image filename", true);
    check_cmdline(parser);

    // check image type
    if (is_hdr_filename(filename1) && is_hdr_filename(filename2)) {
        auto img1 = image<vec4f>{}, img2 = image<vec4f>{};
        if (!load_image(filename1, img1))
            log_fatal("cannot open image {}", filename1);
        if (!load_image(filename2, img2))
            log_fatal("cannot open image {}", filename2);
        if (img1.size != img2.size) log_fatal("image size differs");
        auto diff     = image<vec4f>{};
        auto display     = image<vec4f>{};
        compute_diff_image(diff, img1, img2);
        auto max_diff = max_diff_value(diff);
        if (!output.empty()) {
            display_diff(display, diff, 1.0f);
            if (!save_image(output, display))
                log_fatal("cannot save image {}", output);
        }
        if (max(max_diff) > threshold) {
            log_info("image max difference: {}", max_diff);
            log_fatal("image content differs");
        }
    } else if (!is_hdr_filename(filename1) && !is_hdr_filename(filename2)) {
        auto img1 = image<vec4b>{}, img2 = image<vec4b>{};
        if (!load_image(filename1, img1))
            log_fatal("cannot open image {}", filename1);
        if (!load_image(filename2, img2))
            log_fatal("cannot open image {}", filename2);
        if (img1.size != img2.size) log_fatal("image size differs");
        auto diff     = image<vec4b>{};
        auto display     = image<vec4b>{};
        compute_diff_image(diff, img1, img2);
        auto max_diff = max_diff_value(diff);
        if (!output.empty()) {
            display_diff(display, diff, (byte)255);
            if (!save_image(output, display))
                log_fatal("cannot save image {}", output);
        }
        if (max(max_diff) > threshold) {
            log_info("image max difference: {}", max_diff);
            log_fatal("image content differs");
        }
    } else {
        log_fatal("different image types");
    }

    // done
    return 0;
}