#pragma once

#ifndef OTT_FASTCALL
#define OTT_FASTCALL
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t ott_entity_id;
const ott_entity_id OTT_ENTITY_INVALID = 0;

// entity_system
void ott_entity_system_init(void);
void ott_entity_system_end_frame(void);
ott_entity_id ott_entity_system_create_entity(void);
void OTT_FASTCALL ott_entity_system_destroy_entity(ott_entity_id const ent);
ott_entity_id OTT_FASTCALL ott_entity_system_validate_entity(ott_entity_id const ent);

#ifdef __cplusplus
}
#endif
