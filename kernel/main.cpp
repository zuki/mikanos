/**
 * @file main.cpp
 *
 * カーネル本体
 */
#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"

void* operator new(size_t size, void *buf) {
    return buf;
}

void operator delete(void *obj) noexcept {
}

char pixel_writer_buffer[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

char console_buf[sizeof(Console)];
Console *console;

int printk(const char *format, ...) {
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}

//extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config)
//extern "C" void KernelMain(int a, int b, int c)
extern "C" void KernelMain(uint64_t* dummy1, const FrameBufferConfig &dummy2, const FrameBufferConfig &frame_buffer_config)
{
    switch (frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new(pixel_writer_buffer)
                RGBResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new(pixel_writer_buffer)
                BGRResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
    }

    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
            pixel_writer->Write(x, y, {255, 255, 255});
        }
    }

    console = new(console_buf) Console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

    for (int i = 0; i < 27; ++i) {
        printk("printk: %d\n", i);
    }

    while (1) __asm__("hlt");
}
