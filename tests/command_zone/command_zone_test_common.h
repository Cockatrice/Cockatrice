/**
 * @file command_zone_test_common.h
 * @brief Shared types and utilities for command zone unit tests.
 *
 * Provides access to production CommandZoneType and ZoneVisibility enums
 * without pulling in full Qt graphics dependencies. Tests remain isolated
 * while using the canonical enum definitions.
 */

#ifndef COMMAND_ZONE_TEST_COMMON_H
#define COMMAND_ZONE_TEST_COMMON_H

#include "cockatrice/src/game/zones/command_zone_types.h"

// Re-export the minimized height ratio using the canonical constant name
// that tests were already using (ZoneSizes vs CommandZoneConstants)
namespace ZoneSizes
{
using namespace CommandZoneConstants;
}

#endif // COMMAND_ZONE_TEST_COMMON_H
