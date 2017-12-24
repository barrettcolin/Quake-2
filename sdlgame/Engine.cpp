#include "Engine.h"

#include <cassert>

namespace EntityImpl
{
    union Impl
    {
        struct
        {
            uint32_t m_index : 24;
            uint32_t m_generation : 8;
        } m_bits;
        EntityId m_id;
    };

    unsigned GetIndex(EntityId ent)
    {
        Impl entId;
        entId.m_id = ent;

        return entId.m_bits.m_index;
    }

    unsigned GetGeneration(EntityId ent)
    {
        Impl entId;
        entId.m_id = ent;

        return entId.m_bits.m_generation;
    }

    EntityId Make(unsigned idx, unsigned gen)
    {
        Impl entId;
        entId.m_bits.m_index = idx;
        entId.m_bits.m_generation = gen;

        return entId.m_id;
    }
}

EntitySystem::EntitySystem(Context *ctx)
    : m_context(ctx)
{

}

EntityId EntitySystem::Create()
{
    uint32_t idx;
    if (m_freeEntityIndices.size())
    {
        idx = m_freeEntityIndices.back();
        m_freeEntityIndices.pop_back();
    }
    else
    {
        m_entityGenerations.push_back(0);
        idx = m_entityGenerations.size() - 1;
    }

    return EntityImpl::Make(idx, m_entityGenerations[idx]);
}

void EntitySystem::Destroy(EntityId ent)
{
    const unsigned idx = EntityImpl::GetIndex(ent);
    ++m_entityGenerations[idx];
    m_freeEntityIndices.push_back(idx);
}

bool EntitySystem::IsAlive(EntityId ent) const
{
    const unsigned gen = EntityImpl::GetGeneration(ent);
    const unsigned idx = EntityImpl::GetIndex(ent);
    return m_entityGenerations[idx] == gen;
}

SpriteSystem::SpriteSystem(Context *ctx)
    : m_context(ctx)
{

}

SpriteInstance SpriteSystem::Create(EntityId ent)
{
    assert(m_indexFromEntity.find(ent) == m_indexFromEntity.end());
    const unsigned idx = m_data.size();

    InstanceData data; data.m_entity = ent;
    m_data.push_back(data);

    m_indexFromEntity[ent] = idx;
    return 0;
}

void SpriteSystem::Destroy(SpriteInstance inst)
{
    const EntityId ent = m_data[inst].m_entity;
    assert(m_indexFromEntity.find(ent) != m_indexFromEntity.end());

    const unsigned last = m_data.size() - 1;
    if (inst < last)
    {
        m_data[inst] = m_data[last];
    }
    m_data.pop_back();
    m_indexFromEntity.erase(ent);
}

SpriteInstance SpriteSystem::Lookup(EntityId ent)
{
    std::unordered_map<EntityId, unsigned>::const_iterator it = m_indexFromEntity.find(ent);
    return (it != m_indexFromEntity.end()) ? it->second : 0;
}

void SpriteSystem::SetTexture(SpriteInstance inst, std::shared_ptr<Texture> tex)
{
    m_data[inst].m_texture = tex;
}

void SpriteSystem::SetSrcRect(SpriteInstance inst, const SDL_Rect& rect)
{
    m_data[inst].m_srcRect = rect;
}

void SpriteSystem::SetDstRect(SpriteInstance inst, const SDL_Rect& rect)
{
    m_data[inst].m_dstRect = rect;
}

void SpriteSystem::Render(SDL_Renderer *rend)
{
    int numSprites = m_data.size();

    if (numSprites)
    {
        for (int i = numSprites - 1; i >= 0; --i)
        {
            InstanceData& data = m_data[i];
            if (m_context->GetEntitySystem().IsAlive(data.m_entity))
            {
                SDL_RenderCopy(rend, data.m_texture->m_sdlTexture, &data.m_srcRect, &data.m_dstRect);
            }
            else
            {
                if (i < numSprites - 1)
                {
                    data = m_data[numSprites - 1];
                }
                m_data.pop_back();
                numSprites--;
            }
        }
    }
}

Context::Context()
    : m_entitySystem(this),
    m_spriteSystem(this)
{

}
