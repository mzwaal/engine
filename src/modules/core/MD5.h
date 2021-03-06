/**
 * @file
 */

#pragma once

#include <stdint.h>
#include <string>

namespace core {

extern std::string md5sum(const uint8_t *buf, uint32_t len);

}
