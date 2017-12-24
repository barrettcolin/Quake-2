#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "SDL.h"

class Context;

typedef uint32_t EntityId;

namespace EntityImpl
{
    unsigned GetIndex(EntityId ent);
    unsigned GetGeneration(EntityId ent);
}

class EntitySystem
{
public:
    EntitySystem(Context *ctx);

    EntityId Create();
    void Destroy(EntityId e);

    bool IsAlive(EntityId e) const;

private:
    Context *m_context;

    std::vector<uint8_t> m_entityGenerations;
    std::vector<uint32_t> m_freeEntityIndices;
};

typedef uint32_t SpriteInstance;

struct Texture
{
    SDL_Texture* m_sdlTexture;

    inline Texture();
};

inline Texture::Texture()
    : m_sdlTexture()
{
}

class SpriteSystem
{
public:
    SpriteSystem(Context *ctx);

    SpriteInstance Create(EntityId ent);
    void Destroy(SpriteInstance inst);

    SpriteInstance Lookup(EntityId ent);
    void SetTexture(SpriteInstance inst, std::shared_ptr<Texture> tex);
    void SetSrcRect(SpriteInstance inst, const SDL_Rect& rect);
    void SetDstRect(SpriteInstance inst, const SDL_Rect& rect);

    void Render(SDL_Renderer *rend);

private:
    Context *m_context;

    struct InstanceData
    {
        EntityId m_entity;
        std::shared_ptr<Texture> m_texture;
        SDL_Rect m_srcRect;
        SDL_Rect m_dstRect;
    };

    std::vector<InstanceData> m_data;
    std::unordered_map<EntityId, unsigned> m_indexFromEntity;
};

class Context
{
public:
    Context();

    EntitySystem& GetEntitySystem() { return m_entitySystem; }
    SpriteSystem& GetSpriteSystem() { return m_spriteSystem; }

private:
    EntitySystem m_entitySystem;
    SpriteSystem m_spriteSystem;
};
