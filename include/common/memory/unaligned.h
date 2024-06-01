#pragma once
#include <common.h>

namespace mem {

    static inline u32 __get_unaligned_le32(const u8 *p)
    {
        return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
    }

    static inline u32 get_unaligned_le32(const u32 *p)
    {
        return __get_unaligned_le32((const u8 *)p);
    }

    static inline u16 __get_unaligned_le16(const u8 *p)
    {
        return p[0] | p[1] << 8;
    }

    static inline u16 get_unaligned_le16(const u16 *p)
    {
        return __get_unaligned_le16((const u8 *)p);
    }

    static inline u64 __get_unaligned_le64(const u8 *p)
    {
        return (u64)__get_unaligned_le32(p) | (u64)__get_unaligned_le32(p + 4) << 32;
    }

    static inline u64 get_unaligned_le64(const u64 *p)
    {
        return __get_unaligned_le64((const u8 *)p);
    }
}