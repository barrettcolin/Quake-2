#include "SDL.h"

#include "mini.h"
#include "ott.h"

#include "Engine.h"

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("sdlgame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	ott_entity_system_init();
	ott_entity_id ent0 = ott_entity_system_create_entity();

    Context ctx;
    EntityId ent = ctx.GetEntitySystem().Create();

    SpriteInstance inst = ctx.GetSpriteSystem().Create(ent);

    std::shared_ptr<Texture> texture = std::make_shared<Texture>();
    {
        SDL_Texture *tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);

        unsigned char* bytes = nullptr;
        int pitch = 0;
        SDL_LockTexture(tex, nullptr, reinterpret_cast<void**>(&bytes), &pitch);
        unsigned char rgba[4] = { 255, 0, 0, 255 };
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                memcpy(&bytes[(y * 8 + x) * sizeof(rgba)], rgba, sizeof(rgba));
            }
        }
        SDL_UnlockTexture(tex);

        texture->m_sdlTexture = tex;
    }

    ctx.GetSpriteSystem().SetTexture(inst, texture);
    ctx.GetSpriteSystem().SetSrcRect(inst, { 0, 0, 8, 8 });
    ctx.GetSpriteSystem().SetDstRect(inst, { 8, 8, 16, 16 });

    while (1)
    {
        SDL_Event e;
        if (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                break;
            }
        }

        SDL_RenderClear(rend);

        ctx.GetSpriteSystem().Render(rend);

        SDL_RenderPresent(rend);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
