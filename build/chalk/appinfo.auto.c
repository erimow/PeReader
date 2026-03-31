#include "pebble_process_info.h"
#include "src/resource_ids.auto.h"

const PebbleProcessInfo __pbl_app_info __attribute__ ((section (".pbl_header"))) = {
  .header = "PBLAPP",
  .struct_version = { PROCESS_INFO_CURRENT_STRUCT_VERSION_MAJOR, PROCESS_INFO_CURRENT_STRUCT_VERSION_MINOR },
  .sdk_version = { PROCESS_INFO_CURRENT_SDK_VERSION_MAJOR, PROCESS_INFO_CURRENT_SDK_VERSION_MINOR },
  .process_version = { 1, 0 },
  .load_size = 0xb6b6,
  .offset = 0xb6b6b6b6,
  .crc = 0xb6b6b6b6,
  .name = "PeReader",
  .company = "erimow",
  .icon_resource_id = RESOURCE_ID_IMAGE_ICON,
  .sym_table_addr = 0xA7A7A7A7,
  .flags = PROCESS_INFO_PLATFORM_CHALK,
  .num_reloc_entries = 0xdeadcafe,
  .uuid = { 0x85, 0x1D, 0x53, 0xA9, 0xA5, 0x0F, 0x42, 0x9A, 0x9E, 0xD5, 0x56, 0xEB, 0x3F, 0xFA, 0xD2, 0x58 },
  .virtual_size = 0xb6b6
};
