/*
 * breakout_main.c - Main game logic (SIMPLIFIED)
 */

#include "keyboard/keyboard.h"
#include "graphics/vga.h"
#include "timer/timer.h"
#include "breakout.h"

/* GLOBAL GAME STATE */
game_state_t game;

/* LEVEL DEFINITIONS */
level_t levels[MAX_LEVELS] = {
    // LEVEL 1: CLASSIC
    {
        .pattern = {
            {1,1,1,1,1,1,1,1,1,1,1,1},
            {1,1,1,1,1,1,1,1,1,1,1,1},
            {1,1,1,1,1,1,1,1,1,1,1,1},
            {1,1,1,1,1,1,1,1,1,1,1,1},
            {1,1,1,1,1,1,1,1,1,1,1,1}
        },
        .colors = {4, 12, 14, 2, 1},
        .ball_speed = 2,
        .name = "CLASSIC"
    },
    
    // LEVEL 2: CHECKERBOARD
    {
        .pattern = {
            {2,0,2,0,2,0,2,0,2,0,2,0},
            {0,2,0,2,0,2,0,2,0,2,0,2},
            {2,0,2,0,2,0,2,0,2,0,2,0},
            {0,2,0,2,0,2,0,2,0,2,0,2},
            {2,0,2,0,2,0,2,0,2,0,2,0}
        },
        .colors = {12, 12, 14, 14, 2},
        .ball_speed = 3,
        .name = "CHECKERBOARD"
    },
    
    // LEVEL 3: PYRAMID
    {
        .pattern = {
            {0,0,0,0,0,3,3,0,0,0,0,0},
            {0,0,0,0,2,2,2,2,0,0,0,0},
            {0,0,0,2,2,2,2,2,2,0,0,0},
            {0,0,2,2,2,2,2,2,2,2,0,0},
            {0,2,2,2,2,2,2,2,2,2,2,0}
        },
        .colors = {4, 12, 14, 2, 1},
        .ball_speed = 3,
        .name = "PYRAMID"
    },
    
    // LEVEL 4: BOSS
    {
        .pattern = {
            {3,3,3,3,3,3,3,3,3,3,3,3},
            {3,0,0,3,3,3,3,3,3,0,0,3},
            {3,3,3,3,3,3,3,3,3,3,3,3},
            {3,0,0,3,3,3,3,3,3,0,0,3},
            {3,3,3,3,3,3,3,3,3,3,3,3}
        },
        .colors = {4, 4, 12, 12, 14},
        .ball_speed = 4,
        .name = "BOSS"
    }
};

/* EXTERNAL FUNCTION DECLARATIONS */
extern void init_bricks();
extern void init_balls();
extern void update_balls();
extern void update_bricks();
extern bool check_level_complete();
extern void update_powerups();
extern void update_particles();
extern void draw_rect(int x, int y, int width, int height, uint8_t color);
extern void draw_bricks();
extern void draw_balls();
extern void draw_paddle();
extern void draw_powerups();
extern void draw_lasers();
extern void draw_particles();
extern void draw_hud();
extern void draw_level_start_screen();
extern void draw_turn_transition();
extern void draw_winner_screen();
extern void draw_countdown(int number);

void shoot_laser(player_t* player);
void update_lasers();

/* LASER SYSTEM */
void shoot_laser(player_t* player)
{
    if (!player->has_laser || player->laser_cooldown > 0)
    {
        return;
    }
    
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (!game.lasers[i].active)
        {
            game.lasers[i].x = player->paddle_x + player->paddle_width / 2;
            game.lasers[i].y = PADDLE_Y - 5;
            game.lasers[i].active = true;
            player->laser_cooldown = 10;
            return;
        }
    }
}

void update_lasers()
{
    player_t* player = &game.players[game.current_player];
    
    if (player->laser_cooldown > 0)
    {
        player->laser_cooldown--;
    }
    
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (!game.lasers[i].active)
        {
            continue;
        }
        
        game.lasers[i].y -= 5;
        
        for (int row = 0; row < BRICK_ROWS; row++)
        {
            for (int col = 0; col < BRICK_COLS; col++)
            {
                if (game.bricks[row][col].health == 0)
                {
                    continue;
                }
                
                int brick_x = col * (BRICK_WIDTH + 2) + BRICK_OFFSET_X;
                int brick_y = row * (BRICK_HEIGHT + 2) + BRICK_OFFSET_Y;
                
                if (game.lasers[i].x >= brick_x && 
                    game.lasers[i].x < brick_x + BRICK_WIDTH &&
                    game.lasers[i].y >= brick_y && 
                    game.lasers[i].y < brick_y + BRICK_HEIGHT)
                {
                    game.bricks[row][col].health--;
                    game.lasers[i].active = false;
                    
                    if (game.bricks[row][col].health == 0)
                    {
                        player->score += 10;
                        extern void spawn_explosion(int x, int y, uint8_t color);
                        extern void spawn_powerup(int x, int y);
                        spawn_explosion(brick_x + BRICK_WIDTH/2, brick_y + BRICK_HEIGHT/2,
                                      levels[game.level].colors[row]);
                        spawn_powerup(brick_x, brick_y);
                        game.screen_shake_timer = 3;
                    }
                    
                    goto next_laser;
                }
            }
        }
        
        next_laser:
        
        if (game.lasers[i].y < 0)
        {
            game.lasers[i].active = false;
        }
    }
}

/* INPUT HANDLING */
static void handle_input(key_event_t* event)
{
    if (!event->pressed)
    {
        return;
    }
    
    if (game.all_players_done)
    {
        if (event->scancode == 0x39)  // Space
        {
            breakout_init(game.num_players);
        }
        return;
    }
    
    player_t* player = &game.players[game.current_player];
    
    // Left
    if (event->scancode == 0x4B || event->scancode == 0x1E)
    {
        player->paddle_x -= PADDLE_SPEED * 2;
        if (player->paddle_x < 0)
        {
            player->paddle_x = 0;
        }
    }
    
    // Right
    if (event->scancode == 0x4D || event->scancode == 0x20)
    {
        player->paddle_x += PADDLE_SPEED * 2;
        if (player->paddle_x > VGA_WIDTH - player->paddle_width)
        {
            player->paddle_x = VGA_WIDTH - player->paddle_width;
        }
    }
    
    // Shoot laser
    if (event->scancode == 0x1D)
    {
        shoot_laser(player);
    }
}

/* GAME INITIALIZATION - SIMPLIFIED */
void breakout_init(int num_players)
{
    game.num_players = (num_players <= MAX_PLAYERS) ? num_players : 1;
    game.current_player = 0;
    game.level = 0;
    game.all_players_done = false;
    
    game.music_note = 0;
    game.music_timer = 0;
    game.screen_shake_timer = 0;
    game.screen_shake_x = 0;
    game.screen_shake_y = 0;
    
    // Initialize both players - FIXED VALUES
    for (int p = 0; p < MAX_PLAYERS; p++)
    {
        game.players[p].lives = 3;  // Fixed: 3 lives
        game.players[p].score = 0;
        game.players[p].paddle_width = PADDLE_WIDTH;  // Fixed: 40px
        game.players[p].paddle_x = VGA_WIDTH / 2 - game.players[p].paddle_width / 2;
        game.players[p].has_laser = false;
        game.players[p].laser_cooldown = 0;
        game.players[p].turn_complete = false;
        game.players[p].ball_speed_multiplier = 1.0f;
    }
    
    // Deactivate all lasers
    for (int i = 0; i < MAX_LASERS; i++)
    {
        game.lasers[i].active = false;
    }
    
    init_bricks();
    init_balls();
    
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        game.powerups[i].active = false;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        game.particles[i].active = false;
    }
}

/* MAIN GAME LOOP */
void breakout_run()
{
    uint32_t last_update = timer_get_ticks();
    
    bool showing_transition = false;
    uint32_t transition_start = 0;
    bool transition_drawn = false;
    
    bool winner_drawn = false;
    
    bool showing_level_start = true;
    bool level_start_drawn = false;
    uint32_t level_start_time = 0;
    
    bool showing_countdown = false;
    int countdown_number = 3;
    uint32_t countdown_start = 0;
    bool countdown_drawn = false;
    
    level_start_time = timer_get_ticks();
    
    while (1)
    {
        // INPUT
        key_event_t event;
        while (keyboard_get_event(&event))
        {
            if (event.pressed && event.scancode == 0x01)  // ESC
            {
                return;
            }
            
            if (event.pressed && showing_level_start)
            {
                showing_level_start = false;
                continue;
            }
            
            if (event.pressed && event.scancode == 0x39 && game.all_players_done)
            {
                breakout_init(game.num_players);
                winner_drawn = false;
                transition_drawn = false;
                showing_transition = false;
                showing_level_start = true;
                level_start_drawn = false;
                showing_countdown = false;
                countdown_number = 3;
                continue;
            }
            
            handle_input(&event);
        }
        
        uint32_t current_ticks = timer_get_ticks();
        
        // LEVEL START SCREEN
        if (showing_level_start)
        {
            if (!level_start_drawn)
            {
                draw_level_start_screen();
                level_start_drawn = true;
                level_start_time = current_ticks;
            }
            
            if (current_ticks - level_start_time >= 3000)
            {
                showing_level_start = false;
                showing_countdown = true;
                countdown_number = 3;
                countdown_start = current_ticks;
                countdown_drawn = false;
            }
            continue;
        }
        
        // COUNTDOWN
        if (showing_countdown)
        {
            uint32_t elapsed = current_ticks - countdown_start;
            uint32_t seconds = elapsed / 1000;
            
            if (seconds == 0 && countdown_number == 3)
            {
                if (!countdown_drawn)
                {
                    draw_countdown(3);
                    countdown_drawn = true;
                }
            }
            else if (seconds == 1 && countdown_number == 3)
            {
                countdown_number = 2;
                countdown_drawn = false;
            }
            else if (seconds == 1 && countdown_number == 2)
            {
                if (!countdown_drawn)
                {
                    draw_countdown(2);
                    countdown_drawn = true;
                }
            }
            else if (seconds == 2 && countdown_number == 2)
            {
                countdown_number = 1;
                countdown_drawn = false;
            }
            else if (seconds == 2 && countdown_number == 1)
            {
                if (!countdown_drawn)
                {
                    draw_countdown(1);
                    countdown_drawn = true;
                }
            }
            else if (seconds == 3 && countdown_number == 1)
            {
                countdown_number = 0;
                countdown_drawn = false;
            }
            else if (seconds == 3 && countdown_number == 0)
            {
                if (!countdown_drawn)
                {
                    draw_countdown(0);
                    countdown_drawn = true;
                }
            }
            else if (seconds >= 4)
            {
                showing_countdown = false;
                last_update = current_ticks;
            }
            continue;
        }
        
        // TURN SWITCHING
        if (game.players[game.current_player].turn_complete && !showing_transition)
        {
            if (game.current_player < game.num_players - 1)
            {
                game.current_player++;
                showing_transition = true;
                transition_start = current_ticks;
                transition_drawn = false;
                
                init_bricks();
                init_balls();
                game.level = 0;
                
                for (int i = 0; i < MAX_POWERUPS; i++)
                {
                    game.powerups[i].active = false;
                }
            }
            else
            {
                game.all_players_done = true;
                winner_drawn = false;
            }
        }
        
        // TURN TRANSITION SCREEN
        if (showing_transition)
        {
            if (!transition_drawn)
            {
                draw_turn_transition();
                transition_drawn = true;
            }
            
            if (current_ticks - transition_start >= 2000)
            {
                showing_transition = false;
                showing_level_start = true;
                level_start_drawn = false;
                showing_countdown = false;
                countdown_number = 3;
            }
            continue;
        }
        
        // WINNER SCREEN
        if (game.all_players_done)
        {
            if (!winner_drawn)
            {
                draw_winner_screen();
                winner_drawn = true;
            }
            continue;
        }
        
        // GAME UPDATE (60 FPS)
        if (current_ticks - last_update >= 16)
        {
            last_update = current_ticks;
            
            update_balls();
            update_bricks();
            update_powerups();
            update_particles();
            update_lasers();
            
            if (check_level_complete())
            {
                game.level++;
                
                if (game.level >= MAX_LEVELS)
                {
                    game.players[game.current_player].turn_complete = true;
                }
                else
                {
                    init_bricks();
                    init_balls();
                    showing_level_start = true;
                    level_start_drawn = false;
                    showing_countdown = false;
                    countdown_number = 3;
                }
            }
            
            // Screen shake
            if (game.screen_shake_timer > 0)
            {
                game.screen_shake_timer--;
                extern int random_range(int min, int max);
                game.screen_shake_x = random_range(-2, 2);
                game.screen_shake_y = random_range(-2, 2);
            }
            else
            {
                game.screen_shake_x = 0;
                game.screen_shake_y = 0;
            }
            
            // RENDER
            vga_clear(0);
            draw_bricks();
            draw_paddle();
            draw_balls();
            draw_powerups();
            draw_lasers();
            draw_particles();
            draw_hud();
        }
    }
}