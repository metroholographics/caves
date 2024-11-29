#include "caves.h"

#define G_WIDTH   320
#define G_HEIGHT  240
#define W_WIDTH   G_WIDTH * 2
#define W_HEIGHT  G_HEIGHT * 2
#define TILE_SIZE 16

int
main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL couldn't init: %s\n", SDL_GetError());
        return 1;
    }

    Game* game    = init_game_struct("caves", W_WIDTH, W_HEIGHT);
    game->running = true;

    if (!SDL_CreateWindowAndRenderer(
                game->name, 
                game->width, 
                game->height, 
                0, 
                &game->window,
                &game->renderer
            )
        ) {
            printf("Window and Renderer couldn't init: %s\n", SDL_GetError());
            game->running = false;
    }
    set_game_resolution(game, G_WIDTH, G_HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    game->spritesheet = load_spritesheet_png(game, "./assets/tilesheet.png");
    if (game->spritesheet == NULL) {
        printf("Couldn't load spritesheet\n");
        game->running = false;
    }

    SDL_SetTextureScaleMode(game->spritesheet, SDL_SCALEMODE_NEAREST);

    Player *player = load_player_struct();

    uint64_t last_update_ms = SDL_GetTicks();

    while (game->running) {
        uint64_t start_ms = SDL_GetTicks();
        SDL_Event event;

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderClear(game->renderer);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    game->running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        game->running = false;
                    } else {
                        read_player_input(player, event, SDL_EVENT_KEY_DOWN);
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    read_player_input(player, event, SDL_EVENT_KEY_UP);
                    break;
                default:
                    break;
            }
        }

        uint64_t current_time_ms = SDL_GetTicks();
        uint64_t elapsed_time_ms = current_time_ms - last_update_ms;

        handle_player_input(player);
        update_player_pos(player, elapsed_time_ms);
        update_animation(player, elapsed_time_ms);

        last_update_ms = current_time_ms;

        SDL_FRect dest = (SDL_FRect) {
            .x = player->pos.x,
            .y = player->pos.y,
            .w = 16.0,
            .h = 16.0
        };

        SDL_RenderTexture(game->renderer, game->spritesheet, &player->sprite, &dest);

        SDL_RenderPresent(game->renderer);

        force_fps(60, start_ms);
    }

    free_player_struct(player);
    free_game_struct(game);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

Game*
init_game_struct(char* name, int w, int h)
{
    Game* g;

    g = malloc(sizeof(Game));
    if (g == NULL) return NULL;

    g->name        = name;
    g->window      = NULL;
    g->renderer    = NULL;
    g->spritesheet = NULL;
    g->running     = false;
    g->width       = w;
    g->height      = h;

    return g;
}

void
set_game_resolution(Game* g, int r_w, int r_h, SDL_RendererLogicalPresentation lp)
{
    SDL_SetRenderLogicalPresentation(
        g->renderer,
        r_w,
        r_h,
        lp
    );
}

SDL_Texture*
load_spritesheet_png(Game *g, char* filepath)
{
    SDL_Texture *s;

    if (g->renderer == NULL) return NULL;
    s = IMG_LoadTexture(g->renderer, filepath);

    return s;
}

Player*
load_player_struct(void)
{
    Player* p; 

    p = malloc(sizeof(Player));
    if (p == NULL) return NULL;

    p->sprite.x = 0;
    p->sprite.y = 0;
    p->sprite.w = TILE_SIZE;
    p->sprite.h = TILE_SIZE;
    p->pos.x    = G_WIDTH / 2;
    p->pos.y    = G_HEIGHT / 2;
    p->anim     = (Animation) {
                    .frame_time = 8, 
                    .num_frames = 3,
                    .curr_frame = 0,
                    .elapsed_time = 0,
                };
    p->acceleration_x       = 0.0f;
    p->max_acceleration_x   = 0.0003;
    p->max_speed_x          = 0.08f;
    p->velocity_x           = 0.0f;
    p->slowdown_x           = 0.8f;

    for (int i = 0; i < NUM_MOVES; i++) {
        p->move_buffer[i] = false;
    }

    return p;
}

void
handle_player_input(Player *p)
{
    if (p->move_buffer[LEFT] && p->move_buffer[RIGHT]) {
        stop_moving(p);
    } else if (p->move_buffer[LEFT]) {
        start_moving_left(p);
    } else if (p->move_buffer[RIGHT]) {
        start_moving_right(p);
    } else {
        stop_moving(p);
    }

    return;
}

void
read_player_input(Player *p, SDL_Event e, SDL_EventType t)
{
    if (t == SDL_EVENT_KEY_UP) {
        switch (e.key.key) {
            case SDLK_LEFT:
                p->move_buffer[LEFT]  = false;
                break;
            case SDLK_RIGHT:
                p->move_buffer[RIGHT] = false;
                break;
        }
    } else if (t == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
            case SDLK_LEFT:
                p->move_buffer[LEFT]  = true;
                break;
            case SDLK_RIGHT:
                p->move_buffer[RIGHT] = true;
                break;
        }
    }
    //printf("MoveBuffer[L=%d][R=%d]\n", p->move_buffer[LEFT], p->move_buffer[RIGHT]);
}


void
start_moving_left(Player *p)
{
    p->acceleration_x = -1 * p->max_acceleration_x;
}

void
start_moving_right(Player *p)
{
    p->acceleration_x = p->max_acceleration_x;
}

void
stop_moving(Player *p)
{
    p->acceleration_x = 0;
    

}

void
update_player_pos(Player *p, uint64_t e_t)
{
    p->pos.x      += p->velocity_x * e_t;
    p->velocity_x += p->acceleration_x * e_t;

    if (p->acceleration_x < 0.0f && p->move_buffer[LEFT]) {
        p->velocity_x = fmaxf(p->velocity_x, (-1 * p->max_speed_x));
    } else if (p->acceleration_x > 0.0f && p->move_buffer[RIGHT]) {
        p->velocity_x = fminf(p->velocity_x, p->max_speed_x);
    } else {
        p->velocity_x *= p->slowdown_x;
    }

    //printf("Vel: %f, Acc: %f\n", p->velocity_x, p->acceleration_x);
    //printf("Pos_x %f\n", p->pos.x);
}


void
update_animation(Player* p, uint64_t e_t)
{
    p->anim.elapsed_time += e_t;

    if (p->anim.elapsed_time > 1000 / p->anim.frame_time) {
        p->anim.curr_frame++;
        p->anim.elapsed_time = 0;

        if (p->anim.curr_frame < p->anim.num_frames) {
            p->sprite.x += TILE_SIZE;
        } else {
            p->sprite.x -= TILE_SIZE * (p->anim.num_frames - 1);
            p->anim.curr_frame = 0;
        }
    }
}

void
force_fps(uint8_t fps, uint64_t start_ms)
{
    uint64_t elapsed_ms = SDL_GetTicks() - start_ms;
        if (elapsed_ms < 1000 / fps) {
            SDL_Delay(1000 / fps - elapsed_ms);
        }
    // float spf = (SDL_GetTicks() - start_ms) / 1000.0;
    // float f = 1 / spf;
    // printf("FPS: %f\n", f);
}

void
free_player_struct(Player *p)
{
    if (p != NULL) {
        printf("...freeing Player struct\n");
        free(p);
    }
}

void
free_game_struct(Game *g)
{
    if (g->window != NULL) {
        printf("...freeing Window\n");
        SDL_DestroyWindow(g->window);
    }
    if (g->renderer != NULL) {
        printf("...freeing Renderer\n");
        SDL_DestroyRenderer(g->renderer);
    }
    if (g->spritesheet != NULL) {
        printf("...freeing spritesheet Texture\n");
        SDL_DestroyTexture(g->spritesheet);
    }
    if (g != NULL) {
        printf("...freeing Game struct\n");
        free(g);
    }
}
