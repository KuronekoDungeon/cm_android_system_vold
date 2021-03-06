/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "Hfsplus.h"
#include "Utils.h"
#include "fsexist.h"

#include <base/logging.h>
#include <base/stringprintf.h>

#include <vector>
#include <string>

#include <sys/mount.h>

using android::base::StringPrintf;

namespace android {
namespace vold {
namespace hfsplus {

static const char* kMkfsPath = "/system/bin/mkfs.hfsplus";
static const char* kFsckPath = "/system/bin/fsck.hfsplus";
static const char* kMountPath = "/system/bin/mount";

status_t Check(const std::string& source) {
    std::vector<std::string> cmd;
    cmd.push_back(kFsckPath);
    cmd.push_back(source);

    // Hfsplus devices are currently always untrusted
    return ForkExecvp(cmd, sFsckUntrustedContext);
}

status_t Mount(const std::string& source, const std::string& target, bool ro,
        bool remount, int ownerUid, int ownerGid, int permMask) {
    char mountData[255];

    const char* c_source = source.c_str();
    const char* c_target = target.c_str();
    std::vector<std::string> cmd;
    if ( has_filesystem("hfsplus") ) {
      sprintf(mountData,
            "nls=utf8,nodev,nosuid,dirsync,uid=%d,gid=%d,fmask=%o,dmask=%o",
            ownerUid, ownerGid, permMask, permMask);
      cmd.push_back(kMountPath);
      cmd.push_back("-t");
      cmd.push_back("hfsplus");
    } else {
      sprintf(mountData, "nls=utf8,force,uid=%d,gid=%d,fmask=%o,dmask=%o",
            ownerUid, ownerGid, permMask, permMask);
      cmd.push_back(kMountPath);
      cmd.push_back("-t");
      cmd.push_back("ufsd");
    }
    
    if (ro)
        strcat(mountData, ",ro");
    if (remount)
        strcat(mountData, ",remount");

    cmd.push_back("-o");
    cmd.push_back(mountData);
    cmd.push_back(c_source);
    cmd.push_back(c_target);
    
    return ForkExecvp(cmd);
}

status_t Format(const std::string& source) {
    std::vector<std::string> cmd;
    cmd.push_back(kMkfsPath);
    cmd.push_back(source);

    return ForkExecvp(cmd);
}

}  // namespace hfsplus
}  // namespace vold
}  // namespace android
