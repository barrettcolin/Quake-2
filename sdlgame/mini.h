#pragma once

#include <cstddef>
#include <cstdint>

namespace ott
{
	typedef uint8_t Entity;
	const Entity INVALID_ENTITY = 0;
	const size_t MAX_ENTITIES = 128;

	class EntitySystem
	{
	public:
		inline void Init();

		Entity Create();

		void Destroy(Entity ent);

		Entity Validate(Entity ent) const;

	private:
		static inline uint8_t EntityByte(Entity ent)
		{
			return ent >> 3;
		}

		static inline uint8_t EntityBit(Entity ent)
		{
			return ent & 0x7;
		}

		uint8_t m_inUse[MAX_ENTITIES / 8];
		uint8_t m_destroyed[MAX_ENTITIES / 8];
	};

	class Engine
	{
	public:
		static inline void Init();
	};
}

#if defined (OTT_IMPL)

#include <cassert>

namespace ott
{
	static EntitySystem s_entitySystem;
}

inline void ott::EntitySystem::Init()
{
	m_inUse[0] = 1;
}

ott::Entity ott::EntitySystem::Create()
{
	uint8_t e = 0;
	for (uint8_t i = 0; i < sizeof(m_inUse); ++i)
	{
		uint8_t& inUse8 = m_inUse[i];
		uint8_t const destroyed8 = m_destroyed[i];
		for (uint8_t j = 1; j; j <<= 1, ++e)
		{
			if (!(destroyed8 & j) && !(inUse8 & j))
			{
				inUse8 |= j;
				return e;
			}
		}
	}
	return INVALID_ENTITY;
}

void ott::EntitySystem::Destroy(Entity ent)
{
	m_destroyed[EntityByte(ent)] |= EntityBit(ent);
}

ott::Entity ott::EntitySystem::Validate(Entity ent) const
{
	assert(ent >= 0 && ent < MAX_ENTITIES);
	uint8_t const entityByte = EntityByte(ent);
	uint8_t const entityBit = EntityBit(ent);
	uint8_t const destroyed = m_destroyed[entityByte] & entityBit;
	uint8_t const inUse = m_inUse[entityByte] & entityBit;
	return (!destroyed && inUse) ? ent : INVALID_ENTITY;
}

inline void ott::Engine::Init()
{
	s_entitySystem.Init();
}

#endif // defined (OTT_IMPL)

#if defined (OTT_TEST_MAIN)

#include <cassert>

int main(int argc, char *argv[])
{
	ott::Engine::Init();

	ott::Entity ent = ott::s_entitySystem.Create();
	assert(ent != ott::INVALID_ENTITY);

	ott::s_entitySystem.Destroy(ent);
	ent = ott::s_entitySystem.Validate(ent);
	assert(ent == ott::INVALID_ENTITY);

	ent = ott::s_entitySystem.Validate(ott::INVALID_ENTITY);
	assert(ent == ott::INVALID_ENTITY);

	return 0;
}
#endif // defined (OTT_TEST_MAIN)
