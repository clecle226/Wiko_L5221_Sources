/*
 *
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *  Not a Contribution, Apache license notifications and license are retained
 *  for attribution purposes only.
 *
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

// BWJ for FCCELMXA-138 START
#if defined(CONFIG_PROJECT_L5221_LAX_MX) || defined(CONFIG_PROJECT_L5221_LAX_CO)
#define BTM_DEF_LOCAL_NAME   "LANIX LT500" //halezhang FCCELMXA-349 line
#elif defined(CONFIG_PROJECT_L5221_BLU_US)
#define BTM_DEF_LOCAL_NAME   "BLU STUDIO C 5+5 LTE"
#else
#define BTM_DEF_LOCAL_NAME   "L5221"
#endif
// BWJ for FCCELMXA-138 END

// Disables read remote device feature
#define BTA_SKIP_BLE_READ_REMOTE_FEAT FALSE
#define MAX_ACL_CONNECTIONS    7
#define MAX_L2CAP_CHANNELS    16
#define BLE_VND_INCLUDED   TRUE
// skips conn update at conn completion
#define BTA_BLE_SKIP_CONN_UPD  FALSE
#define BLE_PERIPHERAL_ADV_NAME  TRUE
#endif
