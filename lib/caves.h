#ifndef CAVES_H
#define CAVES_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


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

typedef enum {
	LEFT = 0,
	RIGHT,
	NUM_MOVES
} Moves;

typedef struct {
	Moves      moves;
	SDL_FRect  sprite;
	SDL_FPoint pos;
	Animation  anim;
	float      acceleration_x;
	float      max_acceleration_x;
	float      max_speed_x;
	float      velocity_x;
	float      slowdown_x;
	bool       move_buffer[NUM_MOVES];
} Player;


/*main functions*/
Game*			init_game_struct(char* name, int w, int h);
void			set_game_resolution(Game *g, int r_w, int r_h, SDL_RendererLogicalPresentation lp);
SDL_Texture*	load_spritesheet_png(Game *g, char* filepath);
void			free_game_struct(Game *g);
void			force_fps(uint8_t fps, uint64_t start_ms);

/*player functions*/
Player*			load_player_struct(void);
void			handle_player_input(Player *p);
void			read_player_input(Player *p, SDL_Event e, SDL_EventType t);
void			start_moving_left(Player *p);
void			start_moving_right(Player *p);
void			stop_moving(Player *p);
void			update_player_pos(Player *p, uint64_t e_t);
void			update_animation(Player *p, uint64_t e_t);
void			free_player_struct(Player *p);


#endif
