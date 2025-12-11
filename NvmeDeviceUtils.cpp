/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "Utils.h"
#include "VoldUtil.h"
#include "WriteBooster.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/result.h>
#include <android-base/strings.h>
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

using android::base::ReadFileToString;
using android::base::WriteStringToFile;

namespace android {
namespace vold {

// Define basic NVMe data structure needed for ioctl.
#define NVME_NSID_ALL 0xffffffff

#define NVME_ADMIN_GET_LOG_PAGE 0x2
#define LID_SMART_HEALTH_INFO 0x2

int32_t GetNvmeStorageLifeTime(const std::string& blk_device) {
    uint8_t smart_log[512];
    struct nvme_admin_cmd cmd = {
            .opcode = NVME_ADMIN_GET_LOG_PAGE,
            .nsid = NVME_NSID_ALL,
            .addr = (uintptr_t)&smart_log,
            .data_len = sizeof(smart_log),
            .cdw10 = (((sizeof(smart_log) / sizeof(uint32_t)) - 1) << 16) | LID_SMART_HEALTH_INFO,
    };

    android::base::unique_fd fd(open(blk_device.c_str(), O_RDONLY | O_CLOEXEC));
    if (fd < 0) {
        PLOG(ERROR) << "Failed to open " << blk_device;
        return -1;
    }
    if (ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd)) {
        PLOG(ERROR) << "Failed to issue NVME_IOCTL_ADMIN_CMD";
        return -1;
    }

    return smart_log[5];
}

}  // namespace vold
}  // namespace android
