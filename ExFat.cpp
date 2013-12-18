/*
 *  Copyright (C) 2013 The OmniROM Project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/wait.h>

#include <linux/kdev_t.h>

#define LOG_TAG "Vold"

#include <cutils/log.h>
#include <cutils/properties.h>

#include <logwrap/logwrap.h>

#include "ExFat.h"
#include "VoldUtil.h"

int ExFat::check(const char *fsPath) {
    //TODO
    SLOGW("Skipping exfat fs checks.\n");
    return 0;
}

int ExFat::format(const char *fsPath, const char *mountpoint) {
    //TODO
    SLOGW("Skipping exfat formating .\n");
    return 0;
}

int ExFat::doMount(const char *fsPath, const char *mountPoint, bool ro, bool remount,
        bool executable, bool sdcard,
        int ownerUid, int ownerGid, int permMask,
        bool createLost) {
#ifdef EXFAT_KMOD
    int rc;
    unsigned long flags;
    const char *data = NULL;
    char mountData[255];

    flags = MS_NOATIME | MS_NODEV | MS_NOSUID | MS_DIRSYNC;

    flags |= (executable ? 0 : MS_NOEXEC);
    flags |= (ro ? MS_RDONLY : 0);
    flags |= (remount ? MS_REMOUNT : 0);

    if (sdcard) {
        // Mount external volumes with forced context
        data = "context=u:object_r:sdcard_external:s0";
    }
    sprintf(mountData,
                "%s,uid=%d,gid=%d,fmask=%o,dmask=%o",
                data,ownerUid, ownerGid, permMask, permMask);
    rc = mount(fsPath, mountPoint, "exfat", flags, mountData);

    if (rc && errno == EROFS) {
        SLOGE("%s appears to be a read only filesystem - retrying mount RO", fsPath);
        flags |= MS_RDONLY;
        rc = mount(fsPath, mountPoint, "exfat", flags, mountData);
    }
    return rc;
#else
    //TODO exfat fuse mount
    return -1;
#endif
}


