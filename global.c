#include "headers.h"


PlayerInfo players[TOTAL_PLAYERS]; //must be changed if total number of players were changed.


int INIT_ENERGY_MIN;
int INIT_ENERGY_MAX;
int DECAY_RATE_MIN;
int DECAY_RATE_MAX;
int RECOVERY_TIME_MIN;
int RECOVERY_TIME_MAX;
int GAME_DURATION;
int STREAK_TO_WIN;
int THRESHOLD_TO_WIN;
int COPY_GAME_DURATION;

int pipefd[2];
int pipe_read_fd;

int child_team, child_member;
volatile int child_energy, child_decay, child_return_after;
volatile int pulling_flag = 0;
int child_pipe_fd;

int game_phase = 0;
int score_team1 = 0, score_team2 = 0;
int global_totalEffort = 0;

int total_rounds = 0;
int current_round = 1;
int prev_round = 1;
int rounds_won_team1 = 0;
int rounds_won_team2 = 0;
int user_defined_distance = 500;
int totalEffort_threshold = 20;
int timercaller = 0;
int consecutiveWinsTeam1 = 0;
int consecutiveWinsTeam2 = 0;
