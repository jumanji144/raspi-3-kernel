#include <common/format.h>

void str::fmt::common_integer_format(str::string& buffer, str::view &fmt, u64 value) {
    int base = 10;
    int width = 0;
    bool upper = false;
    if (fmt.size != 0) {
        char base_char = fmt.data[fmt.size - 1];
        switch (base_char) {
            case 'x': base = 16; break;
            case 'o': base = 8; break;
            case 'b': base = 2; break;
            case 'X': base = 16; upper = true; break;
            case 'O': base = 8; upper = true; break;
            case 'B': base = 2; upper = true; break;
            default: break;
        }
        if (fmt.size > 1) {
            // parse width
            view width_view = { fmt.data, fmt.size - 1 };
            width = str::stou(width_view, 10);
        }
    }
    char buffer_data[64];
    size_t buffer_size = str::itos(value, buffer_data, base, width, upper);

    size_t iteration_size = math::min(buffer.size, buffer_size);
    for (size_t i = 0; i < iteration_size; i++) {
        buffer.data[i] = buffer_data[i];
    }

    buffer.data += iteration_size;
    buffer.size -= iteration_size;
}