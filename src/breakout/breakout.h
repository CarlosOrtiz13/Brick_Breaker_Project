/*
 * breakout.h - Main header file (SIMPLIFIED)
 */

#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <stdint.h>
#include <stdbool.h>

/* CONSTANTS */
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

#define MAX_BALLS 10
#define MAX_POWERUPS 10
#define MAX_PARTICLES 100
#define MAX_LASERS 20
#define MAX_LEVELS 4
#define MAX_PLAYERS 2

#define BRICK_ROWS 5
#define BRICK_COLS 12
#define BRICK_WIDTH 25
#define BRICK_HEIGHT 10
#define BRICK_OFFSET_X 10
#define BRICK_OFFSET_Y 30

#define PADDLE_WIDTH 40
#define PADDLE_HEIGHT 8
#define PADDLE_SPEED 4
#define PADDLE_Y (VGA_HEIGHT - 20)

#define BALL_SIZE 4
#define BALL_TRAIL_LENGTH 10

#define POWERUP_SIZE 10
#define POWERUP_FALL_SPEED 2

/* ENUMS */
typedef enum {
    POWERUP_NONE = 0,
    POWERUP_MULTI_BALL,
    POWERUP_EXPAND,
    POWERUP_SHRINK,
    POWERUP_LASER,
    POWERUP_SLOW,
    POWERUP_EXTRA_LIFE,
    POWERUP_FAST
} powerup_type_t;

/* STRUCTURES */
typedef struct {
    int x, y;
    int dx, dy;
    bool active;
    int trail_x[BALL_TRAIL_LENGTH];
    int trail_y[BALL_TRAIL_LENGTH];
    int trail_index;
} ball_t;

typedef struct {
    int health;
    uint8_t color;
} brick_t;

typedef struct {
    int x, y;
    powerup_type_t type;
    bool active;
} powerup_t;

typedef struct {
    int x, y;
    int dx, dy;
    uint8_t color;
    int lifetime;
    bool active;
} particle_t;

typedef struct {
    int x, y;
    bool active;
} laser_t;

typedef struct {
    int score;
    int lives;
    int paddle_x;
    int paddle_width;
    bool has_laser;
    int laser_cooldown;
    bool turn_complete;
    float ball_speed_multiplier;
} player_t;

typedef struct {
    int pattern[BRICK_ROWS][BRICK_COLS];
    uint8_t colors[BRICK_ROWS];
    int ball_speed;
    char* name;
} level_t;

typedef struct {
    // Player management
    int num_players;
    int current_player;
    player_t players[MAX_PLAYERS];
    bool all_players_done;
    
    // Level and gameplay
    int level;
    brick_t bricks[BRICK_ROWS][BRICK_COLS];
    
    // Active game objects
    ball_t balls[MAX_BALLS];
    powerup_t powerups[MAX_POWERUPS];
    particle_t particles[MAX_PARTICLES];
    laser_t lasers[MAX_LASERS];
    
    // Visual effects
    int screen_shake_timer;
    int screen_shake_x;
    int screen_shake_y;
    
    // Audio (disabled but kept for compatibility)
    int music_timer;
    int music_note;
} game_state_t;

/* FUNCTION PROTOTYPES */
void breakout_init(int num_players);
void breakout_run();

void play_sound(int frequency, int duration_ms);
void stop_sound();
void update_music();

void draw_bricks();
void draw_balls();
void draw_paddle();
void draw_powerups();
void draw_lasers();
void draw_particles();

void init_bricks();
void init_balls();
void update_bricks();
void update_balls();
bool check_level_complete();

void spawn_powerup(int x, int y);
void update_powerups();

void spawn_explosion(int x, int y, uint8_t color);
void update_particles();

void draw_hud();
void draw_level_start_screen();
void draw_turn_transition();
void draw_winner_screen();
void draw_countdown(int number);

#endif // BREAKOUT_H