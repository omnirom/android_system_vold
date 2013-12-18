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

#ifndef _EXFAT_H
#define _EXFAT_H

#include <unistd.h>

class ExFat {
public:
    static int doMount(const char *fsPath, const char *mountPoint, bool ro, bool remount,
            bool executable, bool sdcard,
            int ownerUid, int ownerGid, int permMask,
            bool createLost);
    static int format(const char *fsPath, const char *mountpoint);
    static int check(const char *fsPath);
};

#endif
