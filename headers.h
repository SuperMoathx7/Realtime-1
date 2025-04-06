#ifndef HEADER_H
#define HEADER_H

//------------------------Includes-------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
// ---------------- Configuration Macros ----------------
#define WINDOW_WIDTH   1500
#define WINDOW_HEIGHT  800
#define M_PI 3.14159265358979323846

#define NUM_TEAMS 2//
#define MEMBERS_PER_TEAM 4//
#define TOTAL_PLAYERS (NUM_TEAMS * MEMBERS_PER_TEAM)
#define FALL_PROBABILITY 5   // 5% chance to fall each iteration


extern int INIT_ENERGY_MIN;//
extern int INIT_ENERGY_MAX; //
extern int DECAY_RATE_MIN;//
extern int DECAY_RATE_MAX;//
extern int RECOVERY_TIME_MIN;
extern int RECOVERY_TIME_MAX;
extern int GAME_DURATION;
extern int STREAK_TO_WIN;
extern int THRESHOLD_TO_WIN;
extern int COPY_GAME_DURATION;



// ---------------- Message Structure ----------------
// type: 0 = periodic update, 1 = alignment update, 2 = pulling update.
typedef struct {
    pid_t pid;
    int team;       // 1 or 2
    int member;     // e.g., 1 or 2
    int effort;     // computed as energy/decay_rate (child-side computation)
    int energy,decay,returnAfter;     // current energy (child-side)
    int type;       // message type
} EffortMsg;

// ---------------- Player Display Structure ----------------
typedef struct {
    int team;
    int member;
    int energy,decay,returnAfter;            // updated from child's messages
    int effort;            // reported effort (without parent's multiplier)
    float x, y;            // current display position
    float targetX, targetY;// target positions (set during alignment and pulling)
    float radius;
    pid_t pid;             // associated child's PID
} PlayerInfo;

extern PlayerInfo players[TOTAL_PLAYERS]; //must be changed if total number of players were changed.

// ---------------- Global IPC Descriptors ----------------
extern int pipefd[2];     // pipefd[0] is used by the parent for reading; pipefd[1] is inherited by children.
extern int pipe_read_fd;  // alias for parent's read end

// ---------------- Globals in Child Processes ----------------
// (Each child)
extern int child_team, child_member;
extern volatile int child_energy, child_decay, child_return_after;
// Before pulling phase, energy remains constant.
// Once pulling_flag is set (via SIGUSR2), energy decay begins.
extern volatile int pulling_flag;
extern int child_pipe_fd; // inherited write end

// ---------------- Global for Parent ----------------
extern int game_phase; // 0 = waiting for alignment, 1 = aligned, 2 = pulling active
extern int score_team1, score_team2;
// Global totalEffort computed from effective efforts (updated in updateScoreTimer).
extern int global_totalEffort;

extern int total_rounds;         // Total number of rounds in the game
extern int current_round;        // Current round number
extern int prev_round;
extern int rounds_won_team1;     // Rounds won by Team 1
extern int rounds_won_team2;     // Rounds won by Team 2
extern int user_defined_distance;  // Distance needed to win a round (user-defined)
extern int totalEffort_threshold;     // totalEffort threshold to compute distance
extern int timercaller;
extern int consecutiveWinsTeam1;
extern int consecutiveWinsTeam2;


// ---------------- Function Declarations ----------------
void initGL(void);
void display(void);
void timer(int value);
void alignPlayers(void);
void startPullingPhase(void);
void updateFromPipe(void); 
void updateScoreTimer(int value); 
void child_process(int team, int member);
void alignment_handler(int sig);
void pulling_handler(int sig);
void renderBitmapString(float x, float y, void *font, char *string);
void readFile(const char *filename);
void handleClose();
void countdownTimer(int value);
void newrnd(int sig);
void printFinalSummary();


#endif