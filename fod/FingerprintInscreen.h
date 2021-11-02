/*
 * Copyright (C) 2019-2020 The LineageOS Project
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

#pragma once

#include <vendor/lineage/biometrics/fingerprint/inscreen/1.1/IFingerprintInscreen.h>
#include <vendor/goodix/extend/service/2.0/IGoodixFPExtendService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>


namespace vendor::lineage::biometrics::fingerprint::inscreen::implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::vendor::goodix::extend::service::V2_0::IGoodixFPExtendService;

struct FingerprintInscreen : public V1_1::IFingerprintInscreen {
    FingerprintInscreen();
    Return<int32_t> getPositionX() override;
    Return<int32_t> getPositionY() override;
    Return<int32_t> getSize() override;
    Return<void> onStartEnroll() override;
    Return<void> onFinishEnroll() override;
    Return<void> onPress() override;
    Return<void> onRelease() override;
    Return<void> onShowFODView() override;
    Return<void> onHideFODView() override;
    Return<bool> handleAcquired(int32_t acquiredInfo, int32_t vendorCode) override;
    Return<bool> handleError(int32_t error, int32_t vendorCode) override;
    Return<void> setLongPressEnabled(bool enabled) override;
    Return<int32_t> getDimAmount(int32_t brightness) override;
    Return<bool> shouldBoostBrightness() override;
    Return<void> setCallback(const sp<::vendor::lineage::biometrics::fingerprint::inscreen::V1_0::IFingerprintInscreenCallback>& callback) override;
	
    Return<int32_t> getHbmOffDelay() override;
    Return<int32_t> getHbmOnDelay() override;
    Return<bool> supportsAlwaysOnHBM() override;
    Return<void> switchHbm(bool enabled) override;
    Return<void> setIsInKeyguard(bool enabled) override;

    bool mFodCircleVisible;
    sp<IGoodixFPExtendService> mVendorFpService;

    std::mutex mCallbackLock;
    sp<V1_0::IFingerprintInscreenCallback> mCallback;
    bool mFingerPressed;
    bool mIsInKeyguard;
};

}  // namespace vendor::lineage::biometrics::fingerprint::inscreen::implementation
