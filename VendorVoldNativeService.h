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

#ifndef _VENDOR_VOLD_NATIVE_SERVICE_H_
#define _VENDOR_VOLD_NATIVE_SERVICE_H_

#include <android/system/vold/BnVold.h>
#include <android/system/vold/CheckpointingState.h>
#include <android/system/vold/IVoldCheckpointListener.h>

namespace android::vold {

class VendorVoldNativeService : public android::system::vold::BnVold {
  public:
    /** Start the service, but if it's not declared, give up and return OK. */
    static status_t try_start();

    binder::Status registerCheckpointListener(
            const sp<android::system::vold::IVoldCheckpointListener>& listener,
            android::system::vold::CheckpointingState* _aidl_return) final;
};

}  // namespace android::vold

#endif  // _VENDOR_VOLD_NATIVE_SERVICE_H_