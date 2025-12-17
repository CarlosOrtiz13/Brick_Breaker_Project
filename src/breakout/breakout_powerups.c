/*
 * breakout_powerups.c - Power-up system
 * 
 * This file handles the power-up system. When bricks are destroyed, there's
 * a 30% chance a power-up will drop. The player must catch it with their
 * paddle to get the effect.
 * 
 * Power-ups include:
 * - Multi-ball: 3 balls at once!
 * - Expand/Shrink paddle: Changes difficulty
 * - Laser: Shoot bricks
 * - Slow/Fast ball: Changes speed
 * - Extra life: One more chance
 * 
 * Author: CS Student
 * Date: December 2024
 */

#include "keyboard/keyboard.h"
#include "graphics/vga.h"
#include "timer/timer.h"
#include "breakout.h"

// External references
extern game_state_t game;
extern int random_range(int min, int max);
extern void spawn_explosion(int x, int y, uint8_t color);

/*
 * spawn_powerup - Maybe create a power-up at given position
 * 
 * Called when a brick is destroyed. There's only a 30% chance of actually
 * spawning a power-up (otherwise the game would be too easy!).
 * 
 * Parameters:
 *   x, y - Position where brick was destroyed
 */
void spawn_powerup(int x, int y)
{
    // Only 30% chance to spawn - check random number 0-100
    if (random_range(0, 100) > 30)
    {
        return;  // No power-up this time
    }
    
    // Look for an empty power-up slot
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (!game.powerups[i].active)
        {
            // Found a slot! Initialize the power-up
            game.powerups[i].x = x;
            game.powerups[i].y = y;
            
            // Choose random power-up type (skip POWERUP_NONE)
            // Types: 1=MULTI_BALL, 2=EXPAND, 3=SHRINK, 4=LASER, 5=SLOW, 6=EXTRA_LIFE, 7=FAST
            game.powerups[i].type = random_range(1, 7);
            game.powerups[i].active = true;
            
            // Color is calculated at draw time based on type
            
            return;  // Done!
        }
    }
    
    // All power-up slots full - oh well, no power-up this time
}

/*
 * check_collision - Simple rectangle collision detection
 * 
 * Checks if two rectangles overlap. Used to detect when the paddle
 * catches a power-up.
 * 
 * Returns:
 *   true if rectangles overlap, false otherwise
 */
static bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return (x1 + w1 > x2 && x1 < x2 + w2 && y1 + h1 > y2 && y1 < y2 + h2);
}

/*
 * update_powerups - Update all active power-ups
 * 
 * This makes power-ups fall down and checks if the player caught them.
 * Called every frame from the main game loop.
 */
void update_powerups()
{
    player_t* player = &game.players[game.current_player];
    
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        // Skip inactive power-ups
        if (!game.powerups[i].active)
        {
            continue;
        }
        
        // Make power-up fall
        game.powerups[i].y += POWERUP_FALL_SPEED;
        
        // Check if paddle caught the power-up
        if (check_collision(game.powerups[i].x, game.powerups[i].y, 
                          POWERUP_SIZE, POWERUP_SIZE,
                          player->paddle_x, PADDLE_Y, 
                          player->paddle_width, PADDLE_HEIGHT))
        {
            // Power-up caught! Apply the effect
            switch (game.powerups[i].type)
            {
                case POWERUP_MULTI_BALL:
                    // Activate additional balls
                    for (int b = 1; b < MAX_BALLS; b++)
                    {
                        if (!game.balls[b].active)
                        {
                            // Copy first ball's state
                            game.balls[b] = game.balls[0];
                            game.balls[b].active = true;
                            
                            // Give it a random horizontal direction
                            game.balls[b].dx = random_range(-3, 3);
                            if (game.balls[b].dx == 0)
                            {
                                game.balls[b].dx = 2;  // Make sure it moves
                            }
                        }
                    }
                    
                    break;
                    
                case POWERUP_EXPAND:
                    // Make paddle bigger (easier to hit ball)
                    player->paddle_width += 20;
                    if (player->paddle_width > 80)
                    {
                        player->paddle_width = 80;  // Cap maximum size
                    }
                    
                    break;
                    
                case POWERUP_SHRINK:
                    // Make paddle smaller (harder to hit ball)
                    player->paddle_width -= 10;
                    if (player->paddle_width < 20)
                    {
                        player->paddle_width = 20;  // Minimum size
                    }
                    
                    break;
                    
                case POWERUP_LASER:
                    // Enable laser shooting
                    player->has_laser = true;
                   
                    break;
                    
                case POWERUP_SLOW:
                    // Slow down the ball (makes game easier)
                    player->ball_speed_multiplier = 0.5f;
                    
                    break;
                    
                case POWERUP_FAST:
                    // Speed up the ball (makes game harder)
                    player->ball_speed_multiplier = 2.0f;
                   
                    break;
                    
                case POWERUP_EXTRA_LIFE:
                    // Give player an extra life!
                    player->lives++;
                    
                    break;
                    
                default:
                    break;
            }
            
            // Deactivate the power-up and show explosion effect
            game.powerups[i].active = false;
            
            // Calculate color for explosion based on type
            uint8_t explosion_color = 15;  // White default
            switch (game.powerups[i].type)
            {
                case POWERUP_MULTI_BALL: explosion_color = 14; break;  // Yellow
                case POWERUP_EXPAND: explosion_color = 10; break;      // Green
                case POWERUP_SHRINK: explosion_color = 4; break;       // Red
                case POWERUP_LASER: explosion_color = 11; break;       // Light Blue
                case POWERUP_SLOW: explosion_color = 9; break;         // Cyan
                case POWERUP_EXTRA_LIFE: explosion_color = 13; break;  // Pink
                case POWERUP_FAST: explosion_color = 12; break;        // Light Red
            }
            
            spawn_explosion(game.powerups[i].x, game.powerups[i].y, explosion_color);
        }
        
        // Remove power-up if it fell off the bottom of the screen
        if (game.powerups[i].y > VGA_HEIGHT)
        {
            game.powerups[i].active = false;
        }
    }
}