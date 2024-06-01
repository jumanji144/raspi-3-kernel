#pragma once
#include <common.h>

#include <kernel/block/part/partition.h>
#include <kernel/device/block.h>

#include <common/list.h>

namespace blk {

    constexpr inline size_t max_partitions = 16;

    struct partition_table {
        util::list<partition, 16> partitions;
    };

    using partition_table_parser = bool(*) (partition_table& table, dev::blk_device* device);

    // parse routines
    bool parse_mbr(partition_table& table, dev::blk_device* device);

}