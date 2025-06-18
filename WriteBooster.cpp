/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WriteBooster.h"
#include "Utils.h"
#include "VoldUtil.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/result.h>
#include <android-base/strings.h>

using android::base::ReadFileToString;
using android::base::WriteStringToFile;

namespace android {
namespace vold {

template <typename T>
static android::base::Result<T> readHexValue(const std::string_view path) {
    std::string sysfs_path = GetUfsHostControllerSysfsPath();
    if (sysfs_path.empty()) {
        return android::base::Error();
    }

    std::string fullpath = sysfs_path + "/" + std::string(path);
    std::string s;

    if (!ReadFileToString(fullpath, &s)) {
        PLOG(WARNING) << "Reading failed for " << fullpath;
        return android::base::Error();
    }

    s = android::base::Trim(s);
    T out;
    if (!android::base::ParseUint(s, &out)) {
        PLOG(WARNING) << "Parsing of " << fullpath << " failed. Content: " << s;
        return android::base::Error();
    }

    return out;
}

int32_t GetWriteBoosterBufferSize() {
    /* wb_cur_buf: in unit of allocation_unit_size (field width is 4 bytes)
     * allocation_unit_size: in unit of segments (field width is 1 bytes)
     * segment_size: in unit of 512 bytes (field width is 4 bytes)
     * raw_device_capacity: in unit of 512 bytes (field width is 8 bytes)
     */
    auto allocation_unit_size = readHexValue<uint8_t>("geometry_descriptor/allocation_unit_size");
    if (!allocation_unit_size.ok()) {
        return -1;
    }

    auto segment_size = readHexValue<uint32_t>("geometry_descriptor/segment_size");
    if (!segment_size.ok()) {
        return -1;
    }

    auto wb_cur_buf = readHexValue<uint32_t>("attributes/wb_cur_buf");
    if (!wb_cur_buf.ok()) {
        return -1;
    }

    auto raw_device_capacity = readHexValue<uint64_t>("geometry_descriptor/raw_device_capacity");
    if (!raw_device_capacity.ok()) {
        return -1;
    }

    if (allocation_unit_size.value() == 0) {
        LOG(DEBUG) << "Zero allocation_unit_size is invalid.";
        return -1;
    }

    if (segment_size.value() == 0) {
        LOG(DEBUG) << "Zero segment_size is invalid.";
        return -1;
    }

    uint64_t wb_cur_buf_allocation_units =
            static_cast<uint64_t>(wb_cur_buf.value()) * segment_size.value();

    if (wb_cur_buf_allocation_units >
        (raw_device_capacity.value() / allocation_unit_size.value())) {
        LOG(DEBUG) << "invalid wb_cur_buff > raw_device_capacity ";
        return -1;
    }

    /* The allocation_unit_size is represented in the number of sectors.
     * Since the sector size is 512 bytes, the allocation_unit_size in MiB can be calculated as
     * follows: allocation_unit_size in MiB = (allocation_unit_size * 512) / 1024 / 1024 =
     * allocation_unit_size / 2048
     */
    uint64_t wb_cur_buf_mib = wb_cur_buf_allocation_units * allocation_unit_size.value() / 2048ULL;

    if (wb_cur_buf_mib > INT32_MAX) {
        LOG(DEBUG) << "wb_cur_buff overflow";
        return -1;
    }

    /* return in unit of MiB */
    return static_cast<int32_t>(wb_cur_buf_mib);
}

/*
 * Returns the WriteBooster buffer's remaining capacity as a percentage (0-100).
 */
int32_t GetWriteBoosterBufferAvailablePercent() {
    /*
     * wb_avail_buf is in unit of 10% granularity.
     * 00h: 0% buffer remains.
     * 01h~09h: 10%~90% buffer remains
     * 0Ah: 100% buffer remains
     * Others : Reserved
     */
    auto out = readHexValue<uint8_t>("attributes/wb_avail_buf");
    if (!out.ok()) {
        return -1;
    }

    if (out.value() > 10) {
        PLOG(WARNING) << "Invalid wb_avail_buf (" << out.value() << ")";
        return -1;
    }

    return static_cast<int32_t>(out.value() * 10);
}

bool SetWriteBoosterBufferFlush(bool enable) {
    std::string path = GetUfsHostControllerSysfsPath();
    if (path.empty()) {
        return false;
    }

    path += "/enable_wb_buf_flush";

    std::string s = enable ? "1" : "0";

    LOG(DEBUG) << "Toggle WriteBoosterBufferFlush to " << s;
    if (!WriteStringToFile(s, std::string(path))) {
        PLOG(WARNING) << "Failed to set WriteBoosterBufferFlush to " << s << " on " << path;
        return false;
    }
    return true;
}

bool SetWriteBoosterBufferOn(bool enable) {
    std::string path = GetUfsHostControllerSysfsPath();
    if (path.empty()) {
        return false;
    }

    path += "/wb_on";

    std::string s = enable ? "1" : "0";

    LOG(DEBUG) << "Toggle WriteBoosterBufferOn to " << s;
    if (!WriteStringToFile(s, std::string(path))) {
        PLOG(WARNING) << "Failed to set WriteBoosterBufferOn to " << s << " on " << path;
        return false;
    }
    return true;
}

/**
 * Returns WriteBooster buffer lifetime as a percentage (0-100).
 *
 */
int32_t GetWriteBoosterLifeTimeEstimate() {
    /*
     * wb_life_time_est returns as follows:
     * 00h: Information not available (WriteBooster Buffer is disabled)
     * 01h: 0% - 10% WriteBooster Buffer life time used
     * 02h-09h: 10% - 90% WriteBooster Buffer life time used
     * 0Ah: 90% - 100% WriteBooster Buffer life time used
     * 0Bh: Exceeded its maximum estimated WriteBooster Buffer life time
     *      (write commands are processed as if WriteBooster feature was
     *       disabled)
     * Others: Reserved
     */
    auto out = readHexValue<uint8_t>("attributes/wb_life_time_est");
    if (!out.ok()) {
        return -1;
    }

    if (out.value() == 0) {
        PLOG(WARNING) << "WriteBooster is disabled.";
        return -1;
    }

    if (out.value() > 11) {
        PLOG(WARNING) << "Invalid wb_life_time_est (" << out.value() << ")";
        return -1;
    }

    return static_cast<int32_t>(10 * (out.value() - 1));
}

}  // namespace vold
}  // namespace android
