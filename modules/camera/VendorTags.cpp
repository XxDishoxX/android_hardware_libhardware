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

#include <system/camera_metadata.h>
#include "Metadata.h"

//#define LOG_NDEBUG 0
#define LOG_TAG "VendorTags"
#include <cutils/log.h>

#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)
#include <utils/Trace.h>

#include "VendorTags.h"

namespace default_camera_hal {

// Internal representations of vendor tags for convenience.
// Other classes must access this data via public interfaces.
// Structured to be easy to extend and contain complexity.
namespace {
// Describes a single vendor tag entry
struct Entry {
    const char* name;
    uint8_t     type;
};
// Describes a vendor tag section
struct Section {
    const char* name;
    uint32_t start;
    uint32_t end;
    const Entry* tags;
};

// Entry arrays for each section
const Entry DemoWizardry[DEMO_WIZARDRY_END - DEMO_WIZARDRY_START] = {
    [DEMO_WIZARDRY_DIMENSION_SIZE - DEMO_WIZARDRY_START] =
        {"dimensionSize",   TYPE_INT32},
    [DEMO_WIZARDRY_DIMENSIONS - DEMO_WIZARDRY_START] =
        {"dimensions",      TYPE_INT32},
    [DEMO_WIZARDRY_FAMILIAR - DEMO_WIZARDRY_START] =
        {"familiar",        TYPE_BYTE},
    [DEMO_WIZARDRY_FIRE - DEMO_WIZARDRY_START] =
        {"fire",            TYPE_RATIONAL}
};

const Entry DemoSorcery[DEMO_SORCERY_END - DEMO_SORCERY_START] = {
    [DEMO_SORCERY_DIFFICULTY - DEMO_SORCERY_START] =
        {"difficulty",      TYPE_INT64},
    [DEMO_SORCERY_LIGHT - DEMO_SORCERY_START] =
        {"light",           TYPE_BYTE}
};

const Entry DemoMagic[DEMO_MAGIC_END - DEMO_MAGIC_START] = {
    [DEMO_MAGIC_CARD_TRICK - DEMO_MAGIC_START] =
        {"cardTrick",       TYPE_DOUBLE},
    [DEMO_MAGIC_LEVITATION - DEMO_MAGIC_START] =
        {"levitation",      TYPE_FLOAT}
};

// Array of all sections
const Section DemoSections[DEMO_SECTION_COUNT] = {
    [DEMO_WIZARDRY] = { "demo.wizardry",
                        DEMO_WIZARDRY_START,
                        DEMO_WIZARDRY_END,
                        DemoWizardry },
    [DEMO_SORCERY]  = { "demo.sorcery",
                        DEMO_SORCERY_START,
                        DEMO_SORCERY_END,
                        DemoSorcery },
    [DEMO_MAGIC]    = { "demo.magic",
                        DEMO_MAGIC_START,
                        DEMO_MAGIC_END,
                        DemoMagic }
};

// Get a static handle to a specific vendor tag section
const Section* getSection(uint32_t tag)
{
    uint32_t section = (tag - VENDOR_SECTION_START) >> 16;

    if (tag < VENDOR_SECTION_START) {
        ALOGE("%s: Tag 0x%x before vendor section", __func__, tag);
        return NULL;
    }

    if (section >= DEMO_SECTION_COUNT) {
        ALOGE("%s: Tag 0x%x after vendor section", __func__, tag);
        return NULL;
    }

    return &DemoSections[section];
}

// Get a static handle to a specific vendor tag entry
const Entry* getEntry(uint32_t tag)
{
    const Section* section = getSection(tag);
    int index;

    if (section == NULL)
        return NULL;

    if (tag >= section->end) {
        ALOGE("%s: Tag 0x%x outside section", __func__, tag);
        return NULL;
    }

    index = tag - section->start;
    return &section->tags[index];
}
} // namespace

VendorTags::VendorTags()
  : mTagCount(0)
{
    for (int i = 0; i < DEMO_SECTION_COUNT; i++) {
        mTagCount += DemoSections[i].end - DemoSections[i].start;
    }
}

VendorTags::~VendorTags()
{
}

int VendorTags::getTagCount(const vendor_tag_ops_t* ops)
{
    return mTagCount;
}

void VendorTags::getAllTags(const vendor_tag_ops_t* ops, uint32_t* tag_array)
{
    if (tag_array == NULL) {
        ALOGE("%s: NULL tag_array", __func__);
        return;
    }

    for (int i = 0; i < DEMO_SECTION_COUNT; i++) {
        for (uint32_t tag = DemoSections[i].start;
                tag < DemoSections[i].end; tag++) {
            *tag_array++ = tag;
        }
    }
}

const char* VendorTags::getSectionName(const vendor_tag_ops_t* ops, uint32_t tag)
{
    const Section* section = getSection(tag);

    if (section == NULL)
        return NULL;

    return section->name;
}

const char* VendorTags::getTagName(const vendor_tag_ops_t* ops, uint32_t tag)
{
    const Entry* entry = getEntry(tag);

    if (entry == NULL)
        return NULL;

    return entry->name;
}

int VendorTags::getTagType(const vendor_tag_ops_t* ops, uint32_t tag)
{
    const Entry* entry = getEntry(tag);

    if (entry == NULL)
        return -1;

    return entry->type;
}
} // namespace default_camera_hal
