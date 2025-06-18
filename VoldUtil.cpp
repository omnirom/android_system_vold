/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "VoldUtil.h"
#include "Utils.h"

#include <libdm/dm.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>

using android::base::Basename;
using android::base::Realpath;
using namespace android::dm;

android::fs_mgr::Fstab fstab_default;

static std::string GetUfsHostControllerSysfsPathOnce() {
    android::fs_mgr::FstabEntry* entry =
            android::fs_mgr::GetEntryForMountPoint(&fstab_default, DATA_MNT_POINT);
    if (entry == nullptr) {
        LOG(ERROR) << "No mount point entry for " << DATA_MNT_POINT;
        return "";
    }

    // Handle symlinks.
    std::string real_path;
    if (!Realpath(entry->blk_device, &real_path)) {
        real_path = entry->blk_device;
    }

    // Handle logical volumes.
    android::dm::DeviceMapper& dm = android::dm::DeviceMapper::Instance();
    for (;;) {
        std::optional<std::string> parent = dm.GetParentBlockDeviceByPath(real_path);
        if (!parent.has_value()) break;
        real_path = *parent;
    }

    std::string path = "/sys/class/block/" + Basename(real_path);

    // Walk up the sysfs directory tree from the partition (e.g, /sys/class/block/sda34)
    // or from the disk (e.g., /sys/class/block/sda) to reach the UFS host controller's directory
    // (e.g., /sys/class/block/sda34/../device/../../.. --> /sys/devices/platform/00000000.ufs).
    if (android::vold::pathExists(path + "/../device")) {
        path += "/../device/../../..";
    } else if (android::vold::pathExists(path + "/device")) {
        path += "/device/../../..";
    } else {
        LOG(WARNING) << "Failed to get sysfs_path for user data partition";
        return "";
    }

    // Verify the block device is UFS by checking for the presence of "uic_link_state",
    // which is UFS interconnect layer link state.
    // If not UFS, return an empty path.
    if (!android::vold::pathExists(path + "/uic_link_state")) {
        LOG(ERROR) << "The block device (" << Basename(real_path) << ") of " << DATA_MNT_POINT
                   << " is not UFS.";
        return "";
    }

    LOG(DEBUG) << "The sysfs directory for the ufs host controller is found at " << path;
    return path;
}

// get sysfs directory for the ufs host controller containing userdata
// returns an empty string on failures.
std::string GetUfsHostControllerSysfsPath() {
    static std::string ufshc_sysfs_path = GetUfsHostControllerSysfsPathOnce();
    return ufshc_sysfs_path;
}
