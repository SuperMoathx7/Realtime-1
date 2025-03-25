#ifndef COMMON_H
#define COMMON_H

/* System Libraries */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <GL/glut.h>
#include <math.h>

/* Constants */
#define MAX_PLAYERS 8
#define PIPE_READ 0
#define PIPE_WRITE 1
#define TEAM1_ID 1
#define TEAM2_ID 2

/* Game Configuration Structure */
typedef struct {
    int team_players;      // Number of players per team (configurable)
    int energy_min;        // Minimum initial energy (10-100)
    int energy_max;        // Maximum initial energy
    int decrease_min;      // Minimum energy decrease rate (1-5)
    int decrease_max;      // Maximum decrease rate
    int recovery_min;      // Minimum recovery time after fall (2-5 sec)
    int recovery_max;      // Maximum recovery time
    int threshold;         // Effort threshold to win round (e.g., 500)
    int game_duration;     // Total game time in seconds
    int score_limit;       // Score needed to win game (e.g., 3)
} Config;

/* Process Communication */
typedef struct {
    pid_t pid;
    int energy;
    int position;
    bool active;
} PlayerInfo;

/* Global Variables */
extern Config config;
extern PlayerInfo *team1;
extern PlayerInfo *team2;
extern int **pipes_team1;
extern int **pipes_team2;

/* Function Prototypes */

// Configuration
void parse_config(const char *filename);
void validate_config();

// Process Management
void create_pipes();
void spawn_players();
void cleanup_resources();

// Game Logic
void align_players(int team_id);
void calculate_effort();
void check_round_winner(int effort1, int effort2);
void handle_player_fall(int team_id, int player_id);

// Graphics
void init_graphics(int *argc, char **argv);
void update_display(int effort1, int effort2);
void render_rope_position(float offset);
void draw_player(int x, int y, int team, bool active);

// Utilities
int random_range(int min, int max);
double get_timestamp();
void log_message(const char *message);

#endif /* COMMON_H */