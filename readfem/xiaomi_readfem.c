/*
 * Copyright (C) 2017 The LineageOS Project
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

#define LOG_TAG "xiaomi_readfem"
#define LOG_NDEBUG 0

#include <cutils/log.h>
#include <cutils/properties.h>

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define ISMATCH(a, b) (!strncmp(a, b, PROP_VALUE_MAX))

#define BDWLAN_FILE "/sys/module/cnss_common/parameters/bdwlan_file"
#define FF_FLAG_FILE "/persist/ff_flag"

#define SCORPIO_BDWLAN "bd30_a4.bin"
#define CAPRICORN_BDWLAN "bd30_a7.bin"
#define CAPRICORN_BDWLAN_FEM "bd30_a7.b02"
#define LITHIUM_BDWLAN "bd30_a8.bin"
#define NATRIUM_BDWLAN "bd30_b7.bin"
#define NATRIUM_BDWLAN_FEM "bd30_b7.b02"

#define LIB_QMINVAPI "libqminvapi.so"

typedef int (*qmi_nv_read_oem_item_3_t)(unsigned int** ff_flag);

static int read_ff_flag_file() {
    int ff_flag;
    FILE* fp;

    fp = fopen(FF_FLAG_FILE, "r");
    if (fp == NULL) return -1;

    fscanf(fp, "%d", &ff_flag);
    fclose(fp);

    if (ff_flag == 0 || ff_flag == 1)
        return ff_flag;
    else
        return -1;
}

static int write_ff_flag_file(int need_fem_fix) {
    FILE* fp;

    fp = fopen(FF_FLAG_FILE, "w");
    if (fp == NULL) return 0;

    fprintf(fp, "%d\n", need_fem_fix);
    fclose(fp);

    return 1;
}

static int need_fem_fix() {
    unsigned int* ff_flag;
    int need_fem_fix = 0;  // Disabled by default

    int ret = read_ff_flag_file();
    if (ret >= 0) {
        need_fem_fix = ret;
        ALOGV("%s already exists and is valid", FF_FLAG_FILE);
        goto out;
    }

    void* handle = dlopen(LIB_QMINVAPI, RTLD_NOW);
    if (!handle) {
        ALOGE("%s", dlerror());
        goto out;
    }

    qmi_nv_read_oem_item_3_t qmi_nv_read_oem_item_3 =
        (qmi_nv_read_oem_item_3_t)dlsym(handle, "qmi_nv_read_oem_item_3");

    if (!qmi_nv_read_oem_item_3) {
        ALOGE("%s", dlerror());
        goto out_dlclose;
    }

    // Read ff_flag from modem NV
    qmi_nv_read_oem_item_3(&ff_flag);
    need_fem_fix = *ff_flag;

    if (!write_ff_flag_file(need_fem_fix)) {
        ALOGE("Failed to write %s", FF_FLAG_FILE);
    } else {
        ALOGV("%s was successfully generated", FF_FLAG_FILE);
    }

out_dlclose:
    if (handle) dlclose(handle);

out:
    return need_fem_fix;
}

int main() {
    char product_device[PROPERTY_VALUE_MAX];
    FILE* fp;
    int ret = 0;

    fp = fopen(BDWLAN_FILE, "w");
    if (fp == NULL) return 1;

    property_get("ro.product.device", product_device, "");

    if (ISMATCH(product_device, "gemini")) {
        goto out;
    } else if (ISMATCH(product_device, "scorpio")) {
        fprintf(fp, "%s", SCORPIO_BDWLAN);
    } else if (ISMATCH(product_device, "capricorn")) {
        if (need_fem_fix()) {
            fprintf(fp, "%s", CAPRICORN_BDWLAN_FEM);
        } else {
            fprintf(fp, "%s", CAPRICORN_BDWLAN);
        }
    } else if (ISMATCH(product_device, "lithium")) {
        fprintf(fp, "%s", LITHIUM_BDWLAN);
    } else if (ISMATCH(product_device, "natrium")) {
        if (need_fem_fix()) {
            fprintf(fp, "%s", NATRIUM_BDWLAN_FEM);
        } else {
            fprintf(fp, "%s", NATRIUM_BDWLAN);
        }
    } else {
        ALOGE("Unknown product device %s", product_device);
        ret = 1;
    }

    if (ret) {
        ALOGE("Failed to write %s", BDWLAN_FILE);
    } else {
        ALOGV("%s was successfully written", BDWLAN_FILE);
    }

out:
    fclose(fp);
    return ret;
}
