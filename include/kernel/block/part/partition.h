#pragma once
#include <common.h>

namespace blk {

    struct partition {
        addr start_sector;
        addr end_sector;

        u8 type;
    };

}