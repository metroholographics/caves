#ifndef CAVES_H
#define CAVES_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define G_WIDTH   320
#define G_HEIGHT  240
#define SCALING   2
#define W_WIDTH   G_WIDTH * SCALING
#define W_HEIGHT  G_HEIGHT * SCALING
#define TILE_SIZE 16
#define MAP_ROWS  (int) (G_HEIGHT / TILE_SIZE)
#define MAP_COLS  (int) (G_WIDTH / TILE_SIZE)

typedef struct {
	char         *name;
	SDL_Window   *window;
	SDL_Renderer *renderer;
	SDL_Texture  *spritesheet;
	bool         running;
	int          width;
	int          height;
} Game;

typedef struct {
	int num_frames;
	int frame_time;
	int curr_frame;
	int elapsed_time;
} Animation;

typedef struct {
	bool  interacting;
	bool  on_ground;
	bool  jump_active;
	int   acc_x;
	float walking_acc;
	float max_speed_x;
	float vel_x;
	float friction;
	float vel_y;
	float max_speed_y;
	float jump_speed;
	float air_acc;
	float jump_gravity;
	float gravity;
	SDL_FRect collisionX;
	SDL_FRect collisionY;
} Physics;

typedef enum {
	M_LEFT = 0,
	M_RIGHT,
	JUMP,
	LOOK_UP,
	LOOK_DOWN,
	NUM_MOVES
} Moves;

typedef enum {
	L_IDLE_H=0,
	L_IDLE_U,
	L_IDLE_D,
	L_WALK_H,
	L_WALK_U,
	L_WALK_D,
	L_JUMP_H,
	L_JUMP_U,
	L_JUMP_D,
	L_FALL_H,
	L_FALL_U,
	L_FALL_D,
	L_INT_U,
	L_INT_H,
	L_INT_D,
	R_IDLE_H,
	R_IDLE_U,
	R_IDLE_D,
	R_WALK_H,
	R_WALK_U,
	R_WALK_D,
	R_JUMP_H,
	R_JUMP_U,
	R_JUMP_D,
	R_FALL_H,
	R_FALL_U,
	R_FALL_D,
	R_INT_U,
	R_INT_H,
	R_INT_D,
	NUM_SPRITES
} SpriteID;

typedef struct {
	SDL_FRect source;
	Animation anim;
} Sprite;

typedef enum {
	IDLE=0,
	WALKING,
	JUMPING,
	FALLING,
	INTERACTING,
	NUM_STATES
} P_State;

typedef enum {
	LEFT=0,
	RIGHT,
	NUM_DIRS
} P_Dir;

typedef enum {
	HORIZONTAL=0,
	UP,
	DOWN,
	NUM_LOOKS
} P_Look;

typedef struct {
	P_State    state;
	P_Dir      dir;
	P_Look     looking;
	bool       move_buffer[NUM_MOVES];
	Sprite     sprites[NUM_SPRITES];
	Sprite*    curr_sprite;
	SDL_FPoint pos;
	Physics    physics;
} Player;

typedef enum {
	NO_TILE=0,
	WALL,
	NUM_MAP_SPRITES,
} Map_Sprites;

typedef struct {
	/*at 320x240, 16x16 tiles, the map is 20x15 tiles*/
	int    tile_id[MAP_ROWS][MAP_COLS];
} Map;

typedef struct {
	int index;
	SDL_Point col_tiles[10];
} Colliding_Tiles;

typedef struct {
	bool collided;
	int row;
	int col;
} Collision_Info;

/*main functions*/
Game*			init_game_struct(char* name, int w, int h);
void			set_game_resolution(Game *g, int r_w, int r_h, SDL_RendererLogicalPresentation lp);
SDL_Texture*	load_spritesheet_png(Game *g, char* filepath);
void			free_game_struct(Game *g);
void			force_fps(uint8_t fps, uint64_t start_ms);

/*player functions*/
Player*			load_player_struct(void);
void			init_player_sprites(Player *p);
Sprite			load_player_sprite(int dir, int state, int looking);
void			read_player_input(Player *p, SDL_Event e, SDL_EventType t);
void			player_update(Player *p, uint64_t e_t, Map *m);
void			handle_player_input(Player *p);
void			set_state(Player *p);
void			change_sprite(Player *p);
void			start_moving_left(Player *p);
void			start_moving_right(Player *p);
void			stop_moving(Player *p);
void			look_up(Player *p);
void			look_down(Player *p);
void			look_horizontal(Player *p);
void			reset_animation(Player *p);
void			start_jump(Player *p);
void			reset_jump(Player *p);
void			reactivate_jump(Player *p);
void			stop_jump(Player *p);
void			update_player_pos(Player *p, uint64_t e_t, Map *m);
void			update_player_X(Player *p, uint64_t e_t, Map *m);
void			update_player_Y(Player *p, uint64_t e_t, Map *m);
void			update_jump(Player *p, uint64_t e_t);
void			tick_animation(Player *p, uint64_t e_t);
void			draw_player(Game *g, Player *p);
void			free_player_struct(Player *p);
SDL_FRect		left_collision(Player *p, int delta);
SDL_FRect		right_collision(Player *p, int delta);
SDL_FRect		top_collision(Player *p, int delta);
SDL_FRect		bot_collision(Player *p, int delta);


/*map functions*/
Sprite*			init_map_sprites(void);
Sprite			load_map_sprite(int id);
Map*			gen_test_map(void);
void			draw_map(Game* g, Sprite* m_s, Map* m);
void			free_map(Sprite* s_a, Map* m);
int				rect_top(SDL_FRect r);
int				rect_bot(SDL_FRect r);
int				rect_left(SDL_FRect r);
int				rect_right(SDL_FRect r);
Collision_Info  get_wall_collision_coords(Map *m, SDL_FRect r);
Colliding_Tiles	get_colliding_tiles(Colliding_Tiles *c, SDL_FRect r);



#endif
