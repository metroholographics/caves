#include "caves.h"

#define G_WIDTH   320
#define G_HEIGHT  240
#define W_WIDTH   G_WIDTH * 2
#define W_HEIGHT  G_HEIGHT * 2
#define TILE_SIZE 16

int
main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    Game   *game;
    Player *player;
    uint64_t last_update_ms;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL couldn't init: %s\n", SDL_GetError());
        return 1;
    }

    game          = init_game_struct("caves", W_WIDTH, W_HEIGHT);
    game->running = true;

    if (!SDL_CreateWindowAndRenderer(game->name,
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

    player = load_player_struct();

    last_update_ms = SDL_GetTicks();

    while (game->running) {
        uint64_t start_ms = SDL_GetTicks(),
                 current_time_ms,
                 elapsed_time_ms;
        SDL_Event event;
        SDL_FRect dest;
        
        SDL_SetRenderDrawColor(game->renderer, 5, 5, 5, 255);
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

        current_time_ms = SDL_GetTicks();
        elapsed_time_ms = current_time_ms - last_update_ms;

        handle_player_input(player);
        tick_animation(player, elapsed_time_ms);
        update_player_pos(player, elapsed_time_ms);

        last_update_ms = current_time_ms;

        dest = (SDL_FRect) {
            .x = round(player->pos.x),
            .y = round(player->pos.y),
            .w = 16.0,
            .h = 16.0
        };

        SDL_RenderTexture(game->renderer,
            game->spritesheet,
            &player->curr_sprite->source,
            &dest
        );

        SDL_RenderPresent(game->renderer);

        force_fps(50, start_ms);
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
    Game *g;

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
set_game_resolution(Game *g, int r_w, int r_h, SDL_RendererLogicalPresentation lp)
{
    SDL_SetRenderLogicalPresentation(
        g->renderer,
        r_w,
        r_h,
        lp
    );
}

SDL_Texture*
load_spritesheet_png(Game *g, char *filepath)
{
    SDL_Texture *s;

    if (g->renderer == NULL) return NULL;
    s = IMG_LoadTexture(g->renderer, filepath);

    return s;
}

Player*
load_player_struct(void)
{
    int i;
    Player *p; 

    p = malloc(sizeof(Player));
    if (p == NULL) return NULL;

    load_player_sprites(p);
    p->dir       = FACING_LEFT;
    p->curr_sprite = &p->sprites[IDLE_LEFT];
    p->pos.x       = G_WIDTH  / 2;
    p->pos.y       = G_HEIGHT / 2;
    p->physics     = (Physics) {
                .acc_x       = 0.0f,
                .max_acc_x   = 0.000625,
                .max_speed_x = 0.1625f,
                .vel_x       = 0.0f,
                .slowdown_x  = 0.4f,
                .vel_y       = 0.0f,
                .max_speed_y = 0.1625f,
                .jump_speed  = 0.1625f,
                .jump_time   = 0,
                .max_jump    = 275,
                .jump_active = false,
                .gravity     = 0.000625f,
            };

    for (i = 0; i < NUM_MOVES; i++) {
        p->move_buffer[i] = false;
    }

    return p;
}

void
load_player_sprites(Player *p)
{
    p->sprites[IDLE_LEFT]  = (Sprite) {
                            .id     = IDLE_LEFT,
                            .source = (SDL_FRect) {.x = 16, .y = 0, .w = 16, .h = 16},
                            .anim   = (Animation) {
                            .num_frames   = 1,
                            .frame_time   = 10,
                            .curr_frame   = 0,
                            .elapsed_time = 0
                            }
                        };
    p->sprites[WALK_LEFT]  = (Sprite) {
                            .id     = WALK_LEFT,
                            .source = (SDL_FRect) {16, 0 , 16, 16},
                            .anim   = (Animation) {3 , 10, 0 , 0 }
                        };
    p->sprites[IDLE_RIGHT] = (Sprite) {
                            .id     = IDLE_RIGHT,
                            .source = (SDL_FRect) {64, 0 , 16, 16},
                            .anim   = (Animation) {1 , 10, 0 , 0 }
                        };
    p->sprites[WALK_RIGHT] = (Sprite) {
                            .id     = WALK_RIGHT,
                            .source = (SDL_FRect) {64, 0 , 16, 16},
                            .anim   = (Animation) {3 , 10, 0 , 0 }
                        };
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
            case SDLK_Z:
                p->move_buffer[JUMP] = false;
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
            case SDLK_Z:
                p->move_buffer[JUMP] = true;
                break;
        }
    }
    //printf("MoveBuffer[L=%d][R=%d]\n", p->move_buffer[LEFT], p->move_buffer[RIGHT]);
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

    if (p->move_buffer[JUMP]) {
        start_jump(p);
    } else if (!p->move_buffer[JUMP]) {
        stop_jump(p);
    }

    return;
}

void
start_moving_left(Player *p)
{
    p->dir         = FACING_LEFT;
    p->curr_sprite   = &p->sprites[WALK_LEFT];
    p->physics.acc_x = -1 * p->physics.max_acc_x;
}

void
start_moving_right(Player *p)
{
    p->dir           = FACING_RIGHT;
    p->curr_sprite   = &p->sprites[WALK_RIGHT];
    p->physics.acc_x = p->physics.max_acc_x;
}

void
stop_moving(Player *p)
{
    reset_animation(p);
    if (p->dir == FACING_LEFT) {
        p->curr_sprite = &p->sprites[IDLE_LEFT];
    } else if (p->dir == FACING_RIGHT) {
        p->curr_sprite = &p->sprites[IDLE_RIGHT];
    }
    p->physics.acc_x = 0;
}

void
reset_animation(Player *p)
{
    p->curr_sprite->anim.elapsed_time = 0;
}

void
start_jump(Player *p)
{
    if (on_ground(p)) {
        reset_jump(p);
        p->physics.vel_y = (-1 * p->physics.jump_speed);
    } else if (p->physics.vel_y < 0.0f) {
        reactivate_jump(p);
    } else {
        stop_jump(p);
    }
}

bool
on_ground(Player *p)
{
    return (p->pos.y == G_HEIGHT / 2);
}

void
reset_jump(Player *p)
{
    p->physics.jump_time = p->physics.max_jump;
    reactivate_jump(p);
}

void
reactivate_jump(Player *p)
{
    if (p->physics.jump_time > 0) {
        p->physics.jump_active = true;
    } else {
        p->physics.jump_active = false;
    }
}

void
stop_jump(Player *p)
{
    p->physics.jump_active = false;
}

void
update_player_pos(Player *p, uint64_t e_t)
{
    p->pos.x         += p->physics.vel_x * e_t;
    p->physics.vel_x += p->physics.acc_x * e_t;

    if (p->physics.acc_x < 0.0f) {
        p->physics.vel_x = fmaxf(p->physics.vel_x, (-1 * p->physics.max_speed_x));
    } else if (p->physics.acc_x > 0.0f) {
        p->physics.vel_x = fminf(p->physics.vel_x, p->physics.max_speed_x);
    } else if (on_ground(p)){
        p->physics.vel_x *= p->physics.slowdown_x;
    }

    update_jump(p, e_t);

    p->pos.y += p->physics.vel_y * e_t;

    if (!p->physics.jump_active) {
        p->physics.vel_y = fminf(p->physics.vel_y + p->physics.gravity * e_t,
                            p->physics.max_speed_y);
    }

    if (p->pos.y > G_HEIGHT / 2) {
        p->pos.y = G_HEIGHT / 2;
        p->physics.vel_y = 0;
    }

    //printf("Vel: %f, Acc: %d\n", p->physics.vel_y, p->physics.jump_time);
}

void
update_jump(Player *p, uint64_t e_t)
{
    if (p->physics.jump_active) {
        p->physics.jump_time -= e_t;
        if (p->physics.jump_time <= 0) {
            p->physics.jump_active = false;
        }
    }
}

void
tick_animation(Player *p, uint64_t e_t)
{
    p->curr_sprite->anim.elapsed_time += e_t;

    if (p->curr_sprite->anim.elapsed_time > 1000 / p->curr_sprite->anim.frame_time) {
        p->curr_sprite->anim.curr_frame++;
        p->curr_sprite->anim.elapsed_time = 0;

        if (p->curr_sprite->anim.curr_frame < p->curr_sprite->anim.num_frames) {
            p->curr_sprite->source.x += TILE_SIZE;
        } else {
            p->curr_sprite->source.x -= TILE_SIZE * (p->curr_sprite->anim.num_frames - 1);
            p->curr_sprite->anim.curr_frame = 0;
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
