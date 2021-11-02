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

#define LOG_TAG "FingerprintInscreenService"

#include "FingerprintInscreen.h"

#include <unistd.h>
#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <fstream>
#include <cmath>
#include <thread>

#include <sys/stat.h>

#define FINGERPRINT_ACQUIRED_VENDOR 6

#define CMD_FINGERPRINT_EVENT 10

#define HBM_ENABLE_PATH "/sys/class/backlight/panel0-hbm/brightness"

#define FOD_SENSOR_X 450
#define FOD_SENSOR_Y 1916
#define FOD_SENSOR_SIZE 178

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
namespace {

} // anonymous namespace

namespace vendor::lineage::biometrics::fingerprint::inscreen::implementation {

struct ba {
	uint32_t brightness;
	uint32_t alpha;
};

struct ba brightness_alpha_lut[] = {
	{0, 0xff},
	{1, 0xee},
	{2, 0xe8},
	{3, 0xe6},
	{4, 0xe5},
	{6, 0xe4},
	{10, 0xe0},
	{20, 0xd5},
	{30, 0xce},
	{45, 0xc6},
	{70, 0xb7},
	{100, 0xad},
	{150, 0xa0},
	{227, 0x8a},
	{300, 0x80},
	{400, 0x6e},
	{500, 0x5b},
	{600, 0x50},
	{800, 0x38},
	{1023, 0x18},
};
static int interpolate(int x, int xa, int xb, int ya, int yb)
{
	int bf, factor, plus;
	int sub = 0;

	bf = 2 * (yb - ya) * (x - xa) / (xb - xa);
	factor = bf / 2;
	plus = bf % 2;
	if ((xa - xb) && (yb - ya))
		sub = 2 * (x - xa) * (x - xb) / (yb - ya) / (xa - xb);

	return ya + factor + plus + sub;
}

int bl_to_alpha(int brightness)
{
	int level = ARRAY_SIZE(brightness_alpha_lut);
	int i = 0;
	int alpha;

	for (i = 0; i < ARRAY_SIZE(brightness_alpha_lut); i++){
		if (brightness_alpha_lut[i].brightness >= brightness)
			break;
	}

	if (i == 0)
		alpha = brightness_alpha_lut[0].alpha;
	else if (i == level)
		alpha = brightness_alpha_lut[level - 1].alpha;
	else
		alpha = interpolate(brightness,
			brightness_alpha_lut[i-1].brightness,
			brightness_alpha_lut[i].brightness,
			brightness_alpha_lut[i-1].alpha,
			brightness_alpha_lut[i].alpha);
	return alpha;
}

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
    LOG(INFO) << "wrote path: " << path << ", value: " << value << "\n";
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}


FingerprintInscreen::FingerprintInscreen() {
    this->mFodCircleVisible = false;
	this->mFingerPressed = false;
    this->mVendorFpService = IGoodixFPExtendService::getService();
    this->mIsInKeyguard = false;
}

// Methods from ::vendor::lineage::biometrics::fingerprint::inscreen::V1_0::IFingerprintInscreen follow.

Return<int32_t> FingerprintInscreen::getPositionX() {
    return FOD_SENSOR_X;
}

Return<int32_t> FingerprintInscreen::getPositionY() {
    return FOD_SENSOR_Y;
}

Return<int32_t> FingerprintInscreen::getSize() {
    return FOD_SENSOR_SIZE;
}

Return<void> FingerprintInscreen::onStartEnroll() {
    this->mVendorFpService->goodixExtendCommand(CMD_FINGERPRINT_EVENT, 1);
    return Void();
}

Return<void> FingerprintInscreen::onFinishEnroll() {
    return Void();
}

Return<void> FingerprintInscreen::onPress() {
    this->mVendorFpService->goodixExtendCommand(CMD_FINGERPRINT_EVENT, 1);
    return Void();
}

Return<void> FingerprintInscreen::onRelease() {
    this->mVendorFpService->goodixExtendCommand(CMD_FINGERPRINT_EVENT, 0);
    return Void();
}

Return<void> FingerprintInscreen::onShowFODView() {
    return Void();
}

Return<void> FingerprintInscreen::onHideFODView() {
    return Void();
}

Return<bool> FingerprintInscreen::handleAcquired(int32_t acquiredInfo, int32_t vendorCode) {
    LOG(ERROR) << "acquiredInfo: " << acquiredInfo << ", vendorCode: " << vendorCode << "\n";
    return false;
}

Return<bool> FingerprintInscreen::handleError(int32_t error, int32_t vendorCode) {
    LOG(ERROR) << "error: " << error << ", vendorCode: " << vendorCode;
    return false;
}

Return<void> FingerprintInscreen::setLongPressEnabled(bool) {
    return Void();
}

Return<int32_t> FingerprintInscreen::getDimAmount(int32_t brightness) {
//    float alpha;
//    int realBrightness = brightness * 2047 / 255;
//
//    if (realBrightness > 500) {
//        alpha = 1.0 - pow(realBrightness / 2047.0 * 430.0 / 600.0, 0.455);
//    } else {
//        alpha = 1.0 - pow(realBrightness / 1680.0, 0.455);
//    }
//
//    return 255 * alpha;
    LOG(ERROR) << "getDimAmount: brightness " << brightness << ", dimAmount: " << bl_to_alpha(brightness * 1023 / 255) << "\n";
    return bl_to_alpha(brightness * 1023 / 255);
}

Return<bool> FingerprintInscreen::shouldBoostBrightness() {
    return false;
}

Return<void> FingerprintInscreen::setCallback(const sp<V1_0::IFingerprintInscreenCallback>& callback) {
    {
        std::lock_guard<std::mutex> _lock(mCallbackLock);
        mCallback = callback;
    }

    return Void();
}


// Methods from ::vendor::lineage::biometrics::fingerprint::inscreen::V1_1::IFingerprintInscreen follow.
Return<int32_t> FingerprintInscreen::getHbmOffDelay() {
    // TODO implement
    return 0;
}

Return<int32_t> FingerprintInscreen::getHbmOnDelay() {
    // TODO implement
    return 0;
}

Return<bool> FingerprintInscreen::supportsAlwaysOnHBM() {
    return true;
}

Return<void> FingerprintInscreen::switchHbm(bool enabled) {
    set(HBM_ENABLE_PATH, enabled ? 1 : 0);

    if (enabled) {
        usleep(20 * 1000);
    }
    
    return Void();
}

Return<void> FingerprintInscreen::setIsInKeyguard(bool isKeyguard) {

    mIsInKeyguard = isKeyguard;
    return Void();
}

}  // namespace vendor::lineage::biometrics::fingerprint::inscreen::implementation
