#include "caves.h"



int
main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    Game   *game;
    Player *player;
    Map    *test_map;
    Sprite *map_sprites;

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
    map_sprites = init_map_sprites();

    test_map = gen_test_map();

    last_update_ms = SDL_GetTicks();

    while (game->running) {
        uint64_t start_ms = SDL_GetTicks(),
                 current_time_ms,
                 elapsed_time_ms;
        SDL_Event event;
        
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

        player_update(player, elapsed_time_ms, test_map);

        last_update_ms = current_time_ms;
        
        draw_player(game, player);

        draw_map(game, map_sprites, test_map);

        SDL_RenderPresent(game->renderer);

        force_fps(50, start_ms);
    }

    free_map(map_sprites, test_map);
    free_player_struct(player);
    free_game_struct(game);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

/*main functions*/
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

/*player functions*/
Player*
load_player_struct(void)
{
    int i;
    Player *p; 

    p = malloc(sizeof(Player));
    if (p == NULL) return NULL;

    init_player_sprites(p);

    p->state     = IDLE;
    p->dir       = LEFT;
    p->looking   = HORIZONTAL;
    p->curr_sprite = &p->sprites[L_IDLE_H];
    p->pos.x       = (MAP_COLS / 2) * TILE_SIZE /*G_WIDTH  / 2*/;
    p->pos.y       = 0 /*G_HEIGHT / 2*/;
    p->physics     = (Physics) {
                .interacting  = false,
                .on_ground    = false,
                .jump_active  = false,
                .acc_x        = 0,
                .walking_acc  = 0.00083007812,
                .max_speed_x  = 0.15859375 / 2,
                .vel_x        = 0.0,
                .friction     = 0.00049804687,
                .vel_y        = 0.0,
                .max_speed_y  = 0.2998046875,
                .jump_speed   = 0.25 / 2,
                .air_acc      = 0.0003125,
                .jump_gravity = 0.0003125 / 3,  
                .gravity      = 0.00078125,
                .collisionX   = (SDL_FRect) {.x = 3, .y = 5, .w = 10, .h = 6},
                .collisionY   = (SDL_FRect) {.x = 5, .y = 1, .w = 6, .h = 15},
            };

    for (i = 0; i < NUM_MOVES; i++) {
        p->move_buffer[i] = false;
    }

    return p;
}

void
init_player_sprites(Player *p)
{
    int s_idx = 0;
    for (int i = 0; i < NUM_DIRS; i++) {
        for (int j = 0; j < NUM_STATES; j++) {
            for (int k = 0; k < NUM_LOOKS; k++) {
                p->sprites[s_idx] = load_player_sprite(i, j, k);
                s_idx++;
            }
        }
    }
}

Sprite
load_player_sprite(int dir, int state, int looking)
{
    int base_x = 0, base_y = 0, y_factor = 1;
    int fps = 10, num_walking_frames = 3;
    int look_offset = 3, jump_offset = 2, fall_offset = 1;
    int back_frame = 7, down_fall = 6;


    Sprite s = (Sprite) {
        .source = (SDL_FRect) {.x = base_x, .y = base_y, .w = TILE_SIZE, .h = TILE_SIZE},
        .anim   = (Animation) {1, fps, 0, 0},
    };

    s.source.y = (dir == LEFT) ? 0 : y_factor * TILE_SIZE;

    switch (state) {
        case IDLE:
            s.source.x += (looking * look_offset) * TILE_SIZE;
            break;
        case WALKING:
            s.source.x += (looking * look_offset) * TILE_SIZE;
            s.anim.num_frames = num_walking_frames;
            break;
        case JUMPING:
            s.source.x += (looking * look_offset + jump_offset) * TILE_SIZE;
            break;
        case FALLING:
            s.source.x += (looking * look_offset + fall_offset) * TILE_SIZE;
            break;
        case INTERACTING:
            s.source.x += back_frame * TILE_SIZE;
            break;
    }

    if (looking == DOWN) {
        if (state == WALKING) {
            s.source.x = base_x;
        } else if (state == IDLE) {
            s.source.x = back_frame * TILE_SIZE;
        } else if (state == FALLING) {
            s.source.x = down_fall * TILE_SIZE;
        }
    }

    return s;
}

void
read_player_input(Player *p, SDL_Event e, SDL_EventType t)
{
    if (t == SDL_EVENT_KEY_UP) {
        switch (e.key.key) {
            case SDLK_LEFT:
                p->move_buffer[M_LEFT]  = false;
                break;
            case SDLK_RIGHT:
                p->move_buffer[M_RIGHT] = false;
                break;
            case SDLK_Z:
                p->move_buffer[JUMP] = false;
                break;
            case SDLK_UP:
                p->move_buffer[LOOK_UP] = false;
                break;
            case SDLK_DOWN:
                p->move_buffer[LOOK_DOWN] = false;
                break;
        }
    } else if (t == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
            case SDLK_LEFT:
                p->move_buffer[M_LEFT]  = true;
                break;
            case SDLK_RIGHT:
                p->move_buffer[M_RIGHT] = true;
                break;
            case SDLK_Z:
                p->move_buffer[JUMP] = true;
                break;
            case SDLK_UP:
                p->move_buffer[LOOK_UP] = true;
                break;
            case SDLK_DOWN:
                p->move_buffer[LOOK_DOWN] = true;
                break;
        }
    }
    //printf("MoveBuffer[L=%d][R=%d]\n", p->move_buffer[LEFT], p->move_buffer[RIGHT]);
}

void
player_update(Player *p, uint64_t e_t, Map *m)
{
    handle_player_input(p);
    update_player_pos(p, e_t, m);
    set_state(p);
    change_sprite(p);
    tick_animation(p, e_t);
    return;
}

void
handle_player_input(Player *p)
{
    if (p->move_buffer[M_LEFT] && p->move_buffer[M_RIGHT]) {
        stop_moving(p);
    } else if (p->move_buffer[M_LEFT]) {
        start_moving_left(p);
    } else if (p->move_buffer[M_RIGHT]) {
        start_moving_right(p);
    } else {
        stop_moving(p);
    }

    if (p->move_buffer[LOOK_UP] && p->move_buffer[LOOK_DOWN]) {
        look_horizontal(p);
    } else if (p->move_buffer[LOOK_UP]) {
        look_up(p);
    } else if (p->move_buffer[LOOK_DOWN]) {
        look_down(p);
    } else {
        look_horizontal(p);
    }

    if (p->move_buffer[JUMP]) {
        start_jump(p);
    } else if (!p->move_buffer[JUMP]) {
        stop_jump(p);
    }

    return;
}

void
set_state(Player *p)
{
    if (p->physics.interacting) {
        p->state = INTERACTING;
    } else if (p->physics.on_ground) {
        if (p->physics.acc_x < 0 || p->physics.acc_x > 0) {
            p->state = WALKING;
        } else {
            p->state = IDLE;
        }
    } else {
        if (p->physics.vel_y < 0.0f) {
            p->state = JUMPING;
        } else if (p->physics.vel_y > 0.0f) {
            p->state = FALLING;
        }
    }
}

void
change_sprite(Player *p)
{
    int dir_offset =  15, state_offset = 3;

    int id = (p->dir * dir_offset) + (p->state * state_offset) + (p->looking);  

    p->curr_sprite = &p->sprites[id];
}

void
start_moving_left(Player *p)
{
    p->dir                 = LEFT;
    p->physics.acc_x       = -1;
    p->physics.interacting = false;
}

void
start_moving_right(Player *p)
{
    p->dir                 = RIGHT;
    p->physics.acc_x       = 1;
    p->physics.interacting = false;
}

void
stop_moving(Player *p)
{
    reset_animation(p);
    p->physics.acc_x = 0;
}

void
look_up(Player *p)
{
    p->looking = UP;
    p->physics.interacting = false;
}

void 
look_down(Player *p)
{
    if (p->looking == DOWN) return;

    p->looking = DOWN;
    p->physics.interacting = p->physics.on_ground;        
    
}

void
look_horizontal(Player *p)
{
    p->looking = HORIZONTAL;
}

void
reset_animation(Player *p)
{
    p->curr_sprite->anim.elapsed_time = 0;
}

void
start_jump(Player *p)
{
    p->physics.interacting = false;
    p->physics.jump_active = true;
    if (p->physics.on_ground) {
        p->physics.vel_y = (-1 * p->physics.jump_speed);
    }
}

void
stop_jump(Player *p)
{
    p->physics.jump_active = false;
}

void
update_player_pos(Player *p, uint64_t e_t, Map* m)
{
    update_player_X(p, e_t, m);
    update_player_Y(p, e_t, m);
}

void
update_player_X(Player *p, uint64_t e_t, Map* m)
{
    SDL_FRect r;
    Collision_Info info;

    float x_accel = 0.0f;
    if (p->physics.acc_x < 0) 
        x_accel = p->physics.on_ground ? -1 * p->physics.walking_acc : -1 * p->physics.air_acc;
    else if (p->physics.acc_x > 0) 
        x_accel = p->physics.on_ground ? p->physics.walking_acc : p->physics.air_acc;

    p->physics.vel_x += x_accel * e_t;

    if (p->physics.acc_x < 0) {
        p->physics.vel_x = fmaxf(p->physics.vel_x, (-1 * p->physics.max_speed_x));
    } else if (p->physics.acc_x > 0) {
        p->physics.vel_x = fminf(p->physics.vel_x, p->physics.max_speed_x);
    } else if (p->physics.on_ground) {
        p->physics.vel_x = p->physics.vel_x > 0.0f ? 
            fmaxf(0.0f, p->physics.vel_x - p->physics.friction * e_t) : 
            fminf(0.0f, p->physics.vel_x + p->physics.friction * e_t);
    }

    int delta = p->physics.vel_x * e_t;

    if (delta > 0) {
        r = right_collision(p, delta);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.x = info.col * TILE_SIZE - rect_right(p->physics.collisionX);
            p->physics.vel_x = 0.0f;
        } else {
            p->pos.x += delta;
        }

        r = left_collision(p, 0);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.x = info.col * TILE_SIZE + rect_right(p->physics.collisionX);
        }
    } else {
        r = left_collision(p, delta);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.x = info.col * TILE_SIZE + rect_right(p->physics.collisionX);
            p->physics.vel_x = 0.0f;
        } else {
            p->pos.x += delta;
        }

        r = right_collision(p, 0);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.x = info.col * TILE_SIZE - rect_right(p->physics.collisionX);
        }        
    }
}

void
update_player_Y(Player *p, uint64_t e_t, Map* m)
{
    SDL_FRect r;
    Collision_Info info;

    float gravity = p->physics.jump_active && p->physics.vel_y < 0.0f ? 
            p->physics.jump_gravity : p->physics.gravity;

    p->physics.vel_y = fminf(p->physics.vel_y + gravity * e_t, p->physics.max_speed_y);

    int delta = p->physics.vel_y * e_t;

    if (delta > 0) {
        r = bot_collision(p, delta);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.y = info.row * TILE_SIZE - rect_bot(p->physics.collisionY);
            p->physics.vel_y = 0.0f;
            p->physics.on_ground = true;
        } else {
            p->pos.y += delta;
            p->physics.on_ground = false;
        }

        r = top_collision(p, 0);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.y = info.row * TILE_SIZE + p->physics.collisionY.h;
        }
    } else {
        r = top_collision(p, delta);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.y = info.row * TILE_SIZE + p->physics.collisionY.h;
            p->physics.vel_y = 0.0f;
        } else {
            p->pos.y += delta;
            p->physics.on_ground = false;
        }

        r = bot_collision(p, 0);
        info = get_wall_collision_coords(m, r);

        if (info.collided) {
            p->pos.y = info.row * TILE_SIZE - rect_bot(p->physics.collisionY);
            p->physics.on_ground = true;
        }
    } 
}

SDL_FRect
left_collision(Player *p, int delta)
{
    SDL_FRect col_x = p->physics.collisionX;
    SDL_FRect r = (SDL_FRect) {
        .x = p->pos.x + rect_left(col_x) + delta,
        .y = p->pos.y + rect_top(col_x),
        .w = col_x.w / 2 - delta,
        .h = col_x.h,
    };
    return r;
}

SDL_FRect
right_collision(Player *p, int delta)
{
    SDL_FRect col_x = p->physics.collisionX;
    SDL_FRect r = (SDL_FRect) {
        .x = p->pos.x + rect_left(col_x) + col_x.w / 2 ,
        .y = p->pos.y + rect_top(col_x),
        .w = col_x.w / 2 + delta,
        .h = col_x.h,
    };

    return r;
}

SDL_FRect
top_collision(Player *p, int delta)
{
    SDL_FRect col_y = p->physics.collisionY;
    SDL_FRect r = (SDL_FRect) {
        .x = p->pos.x + rect_left(col_y),
        .y = p->pos.y + rect_top(col_y) + delta,
        .w = col_y.w,
        .h = col_y.h / 2 - delta,
    };

    return r;
}

SDL_FRect
bot_collision(Player *p, int delta)
{
    SDL_FRect col_y = p->physics.collisionY;
    SDL_FRect r = (SDL_FRect) {
        .x = p->pos.x + rect_left(col_y),
        .y = p->pos.y + rect_top(col_y) + col_y.h / 2,
        .w = col_y.w,
        .h = col_y.h / 2 + delta,
    };

    return r;
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
draw_player(Game *g, Player *p)
{
    SDL_FRect dest = (SDL_FRect) {
        .x = round(p->pos.x),
        .y = round(p->pos.y),
        .w = 16.0,
        .h = 16.0
    };

    //DEBUG: - DELETE WHEN COLLISIONS WORKING

    // int tile_x, tile_y;
    // tile_x = (int) p->pos.x / TILE_SIZE;
    // tile_y = (int) p->pos.y / TILE_SIZE;

    // int top = (int) dest.y / TILE_SIZE;
    // int bot = (int) (dest.y + dest.h) / TILE_SIZE;
    // int left = (int) dest.x / TILE_SIZE;
    // int right = (int) (dest.x + dest.w) / TILE_SIZE;


    // printf("TILE_X: %d, TILE_Y: %d --> ", tile_x, tile_y);
    // printf("TOP: %d, BOT: %d, LEFT: %d, RIGHT: %d\n", top, bot, left, right);

    SDL_RenderTexture(g->renderer,
        g->spritesheet,
        &p->curr_sprite->source,
        &dest
    );

    return;
}

void
free_player_struct(Player *p)
{
    if (p != NULL) {
        printf("...freeing Player struct\n");
        free(p);
    }
}

/*map functions*/
Sprite*
init_map_sprites(void)
{
    Sprite* s_a = malloc(sizeof(Sprite) * NUM_MAP_SPRITES);
    if (s_a == NULL) return NULL;

    s_a[NO_TILE] = load_map_sprite(NO_TILE);
    s_a[WALL]    = load_map_sprite(WALL);

    return s_a;
}

Sprite
load_map_sprite(int id)
{
    Sprite s = (Sprite) {
        .source = (SDL_FRect) {.w = TILE_SIZE, .h = TILE_SIZE},
    };

    switch (id) {
        case NO_TILE:
            break;
        case WALL:
            s.source.x = 0;
            s.source.y = 2 * TILE_SIZE;
            break;
        default:
            break;   
    }
    return s;
}

Map*
gen_test_map(void)
{
    int rows, cols;
    Map* m = malloc(sizeof(Map));
    if (m == NULL) return NULL;

    for (rows = 0; rows < MAP_ROWS; rows++) {
        for (cols = 0; cols < MAP_COLS; cols++) {
            m->tile_id[rows][cols] = 0;
        }
    }

    rows = (MAP_ROWS / 2) + 1;

    for (cols = 0; cols < MAP_COLS; cols++) {
        m->tile_id[rows][cols] = WALL;
    }

    m->tile_id[7][7] = WALL;
    m->tile_id[6][6] = WALL;
    m->tile_id[5][5] = WALL;
    m->tile_id[5][7] = WALL;

    return m;
}

void
draw_map(Game* g, Sprite* m_s, Map* m)
{
    SDL_FRect dest = (SDL_FRect) {.w = 16.0, .h = 16.0};
    int rows, cols;
    for (rows = 0; rows < MAP_ROWS; rows++) {
        for (cols = 0; cols < MAP_COLS; cols++) {
            if (m->tile_id[rows][cols] != NO_TILE) {
                Sprite to_draw = m_s[m->tile_id[rows][cols]];
                dest.x = cols * TILE_SIZE;
                dest.y = rows * TILE_SIZE;
                SDL_RenderTexture(g->renderer,
                    g->spritesheet,
                    &to_draw.source,
                    &dest);
            }
        }
    }
}

int
rect_top(SDL_FRect r)
{
    return (int) r.y;
}

int
rect_bot(SDL_FRect r)
{
    return (int) (r.y + r.h);
}

int
rect_left(SDL_FRect r)
{
    return (int) (r.x);
}

int
rect_right(SDL_FRect r)
{
    return (int) (r.x + r.w);
}

Collision_Info
get_wall_collision_coords(Map *m, SDL_FRect r)
{
    Collision_Info info = (Collision_Info) {false, 0, 0};

    if (!m) return info;

    Colliding_Tiles c = {0};
    get_colliding_tiles(&c, r);

    int i, x, y;
    for (i = 0; i < c.index; i++) {
        y = c.col_tiles[i].y;
        x = c.col_tiles[i].x;
        if (m->tile_id[y][x] == WALL) {
            info = (Collision_Info) {true, y, x};
            return info;
        }
    }

    return info;
}


Colliding_Tiles
get_colliding_tiles(Colliding_Tiles *c, SDL_FRect r)
{
    int row, col, index;
    int top   = (int) (rect_top(r)   / TILE_SIZE);
    int bot   = (int) (rect_bot(r)   / TILE_SIZE);
    int left  = (int) (rect_left(r)  / TILE_SIZE);
    int right = (int) (rect_right(r) / TILE_SIZE);

    index = 0;
    for (row = top; row <= bot; row++) {
        for (col = left; col <= right; col++) {
            c->col_tiles[index] = (SDL_Point) {.x = col, .y = row};
            index++;
        }
    }
    c->index = index;

    return *c;
}

void
free_map(Sprite* s_a, Map* m)
{
    if (s_a != NULL) {
        printf("...freeing Map Sprites\n");
        free(s_a);
    }
    if (m != NULL) {
        printf("...freeing Map\n");
        free(m);
    }
}
