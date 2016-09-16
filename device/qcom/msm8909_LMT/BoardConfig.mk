# Copyright (C) 2011 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# config.mk
#
# Product-specific compile-time definitions.
#

include device/qcom/msm8909/BoardConfig.mk

#Disable IMS on lean & mean targets
TARGET_USES_IMS := false

#Disable NFC
TARGET_USES_QCA_NFC := false
TARGET_ENABLE_SMARTCARD_SERVICE := false

#Disable SVA listen app
AUDIO_FEATURE_ENABLED_LISTEN := false
BOARD_SUPPORTS_SOUND_TRIGGER := false
