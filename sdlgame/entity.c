#include "ott.h"

#include <assert.h>

enum
{
	MAX_ENTITIES = 128
};

struct ott_entity_system
{
	uint8_t in_use[MAX_ENTITIES / 8];
	uint8_t destroyed[MAX_ENTITIES / 8];
} s_entity_sys;

void ott_entity_system_init(void)
{
	s_entity_sys.in_use[0] = 1;
}

void ott_entity_system_end_frame(void)
{
	uint8_t i = 0;
	for (; i < sizeof(s_entity_sys.in_use); ++i)
	{
		uint8_t j = 1;

		while (j)
		{
			if (s_entity_sys.destroyed[i] & j)
			{
				if (s_entity_sys.in_use[i] & j)
				{
					s_entity_sys.in_use[i] &= (uint8_t)~j;
				}
				else
				{
					s_entity_sys.destroyed[i] &= (uint8_t)~j;
				}
			}

			j <<= 1;
		}
	}
}

ott_entity_id ott_entity_system_create_entity(void)
{
	uint8_t i = 0;
	for (; i < sizeof(s_entity_sys.in_use); ++i)
	{
		uint8_t j = 1, k = 0;

		while (j)
		{
			if (!(s_entity_sys.in_use[i] & j))
			{
				if (!(s_entity_sys.destroyed[i] & j))
				{
					s_entity_sys.in_use[i] |= j;
					return (i << 3) + k;
				}
			}

			j <<= 1; k++;
		}
	}

	return OTT_ENTITY_INVALID;
}

void OTT_FASTCALL ott_entity_system_destroy_entity(ott_entity_id const ent)
{
	uint8_t const base = ent >> 3;
	uint8_t const mask = 1 << (ent & 0x7);

	assert(ent != OTT_ENTITY_INVALID);
	assert((s_entity_sys.in_use[base] & mask) && !(s_entity_sys.destroyed[base] & mask));
	s_entity_sys.destroyed[base] |= mask;
}

ott_entity_id OTT_FASTCALL ott_entity_system_validate_entity(ott_entity_id const ent)
{
	uint8_t const base = ent >> 3;
	uint8_t const mask = 1 << (ent & 0x7);

	return ((s_entity_sys.in_use[base] & mask) && !(s_entity_sys.destroyed[base] & mask)) ? ent : OTT_ENTITY_INVALID;
}
