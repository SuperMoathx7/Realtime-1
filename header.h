#ifndef HEADER_H
#define HEADER_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <GL/glut.h>
#include <string.h>

#define MAX_PLAYERS 4
#define PIPE_READ 0
#define PIPE_WRITE 1
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
    int team_players;
    int energy_min;
    int energy_max;
    int decrease_min;
    int decrease_max;
    int recovery_min;
    int recovery_max;
    int threshold;
    int game_duration;
    int score_limit;
} Config;

typedef struct {
    pid_t pid;
    int energy;
    int position;
    int pipe_fd[2];
    bool active;
    int recovery_time;
} PlayerInfo;

extern Config config;
extern PlayerInfo team1[MAX_PLAYERS];
extern PlayerInfo team2[MAX_PLAYERS];
extern int score_team1;
extern int score_team2;
extern float rope_offset;
extern int current_round;
extern int time_remaining;
extern bool game_over;

void parse_config(const char *filename);
void create_pipes();
void spawn_players();
void sort_players(PlayerInfo *team);
void update_game_state();
void reset_round();
void draw_text(float x, float y, char *string);
void draw_players();
void draw_rope();
void render();
void init_graphics(int argc, char **argv);

#endif