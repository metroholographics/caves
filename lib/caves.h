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
	float acc_x;
	float max_acc_x;
	float max_speed_x;
	float vel_x;
	float slowdown_x;
	float vel_y;
	float max_speed_y;
	float jump_speed;
	int   jump_time;
	int   max_jump;
	bool  jump_active;
	float gravity;
} Physics;

typedef enum {
	LEFT = 0,
	RIGHT,
	JUMP,
	NUM_MOVES
} Moves;

typedef enum {
	IDLE_LEFT = 0,
	WALK_LEFT,
	IDLE_RIGHT,
	WALK_RIGHT,
	NUM_SPRITES
} SpriteID;

typedef struct {
	SpriteID  id;
	SDL_FRect source;
	Animation anim;
} Sprite;

typedef enum {
	FACING_LEFT,
	FACING_RIGHT,
} P_Dir;

typedef struct {
	P_Dir      dir;
	bool       move_buffer[NUM_MOVES];
	Sprite     sprites[NUM_SPRITES];
	Sprite*    curr_sprite;
	SDL_FPoint pos;
	Physics    physics;
} Player;

/*main functions*/
Game*			init_game_struct(char* name, int w, int h);
void			set_game_resolution(Game *g, int r_w, int r_h, SDL_RendererLogicalPresentation lp);
SDL_Texture*	load_spritesheet_png(Game *g, char* filepath);
void			free_game_struct(Game *g);
void			force_fps(uint8_t fps, uint64_t start_ms);

/*player functions*/
Player*			load_player_struct(void);
void			load_player_sprites(Player *p);
void			read_player_input(Player *p, SDL_Event e, SDL_EventType t);
void			handle_player_input(Player *p);
void			start_moving_left(Player *p);
void			start_moving_right(Player *p);
void			stop_moving(Player *p);
void			reset_animation(Player *p);
void			start_jump(Player *p);
bool			on_ground(Player *p);
void			reset_jump(Player *p);
void			reactivate_jump(Player *p);
void			stop_jump(Player *p);
void			update_player_pos(Player *p, uint64_t e_t);
void			update_jump(Player *p, uint64_t e_t);
void			tick_animation(Player *p, uint64_t e_t);
void			free_player_struct(Player *p);




#endif
