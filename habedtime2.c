//remaining things:
//  1-There is a bug When starting a new round.
//  2-Accidentally falling.
//  3-Stop when reaching game Duration.  Doneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//  4-Stop when reaching Win Streak.
//  5-Parent must tell Childs that their team won or lost.
//  6-Bug: if all energies reach zero in a pulling phase in a round, the game will stuck till the end of the timer. Must we fix it? or it is realistic case?
//  7-Energy in new round didn't generated again, it contiunues with the last values.#include <stdio.h> doneeeeeeeee

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

int INIT_ENERGY_MIN;//
int INIT_ENERGY_MAX; //
int DECAY_RATE_MIN;//
int DECAY_RATE_MAX;//
int RECOVERY_TIME_MIN;
int RECOVERY_TIME_MAX;
int GAME_DURATION;
int STREAK_TO_WIN;
int THRESHOLD_TO_WIN;
int COPY_GAME_DURATION;


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

PlayerInfo players[TOTAL_PLAYERS]; //must be changed if total number of players were changed.

// ---------------- Global IPC Descriptors ----------------
int pipefd[2];     // pipefd[0] is used by the parent for reading; pipefd[1] is inherited by children.
int pipe_read_fd;  // alias for parent's read end

// ---------------- Globals in Child Processes ----------------
// (Each child)
int child_team, child_member;
volatile int child_energy, child_decay, child_return_after;
// Before pulling phase, energy remains constant.
// Once pulling_flag is set (via SIGUSR2), energy decay begins.
volatile int pulling_flag = 0;
int child_pipe_fd; // inherited write end

// ---------------- Global for Parent ----------------
int game_phase = 0; // 0 = waiting for alignment, 1 = aligned, 2 = pulling active
int score_team1 = 0, score_team2 = 0;
// Global totalEffort computed from effective efforts (updated in updateScoreTimer).
int global_totalEffort = 0;

int total_rounds;         // Total number of rounds in the game
int current_round = 1;        // Current round number
int prev_round = 1;
int rounds_won_team1 = 0;     // Rounds won by Team 1
int rounds_won_team2 = 0;     // Rounds won by Team 2
int user_defined_distance = 500;  // Distance needed to win a round (user-defined)
int totalEffort_threshold = 20;     // totalEffort threshold to compute distance
int timercaller = 0;
int consecutiveWinsTeam1 = 0;
int consecutiveWinsTeam2 = 0;


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


void countdownTimer(int value) {
    if (game_phase == 2 && COPY_GAME_DURATION > 0) {  // Pulling phase
        COPY_GAME_DURATION--;
        glutPostRedisplay(); // Update display
    }

    // Keep calling every second regardless, but only decrement if in phase 2
    if (COPY_GAME_DURATION > 0) {
        glutTimerFunc(1000, countdownTimer, 0);
    } else if (COPY_GAME_DURATION <= 0) {
        printf("Sumulation ends: Game Duration ends!\n");


        //current_round++;
        for (int k = 0; k < TOTAL_PLAYERS; k++) kill(players[k].pid, SIGPWR);
        exit(0);
        
        //COPY_GAME_DURATION = GAME_DURATION;

        //glutTimerFunc(1000, timer, 0);
    }
}

void handleClose() {  //this kills all related processes when the user closes the GUI.
    printf("Window closed by user. Cleaning up...\n");
    
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        kill(players[i].pid, SIGKILL);
    }

    exit(0);
}

void readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
if (!file) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
}
//********************** */
    if(fscanf(file, "energy_min=%d\n", &INIT_ENERGY_MIN) !=1){
        printf("enegy_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "energy_max=%d\n", &INIT_ENERGY_MAX) !=1){
        printf("enegy_max is non-integer! Retry again.\n");
        exit(10);
    }

    if(INIT_ENERGY_MIN > INIT_ENERGY_MAX){
        printf("Energy_min must be smaller than Energy_max! Retry again.\n");
        exit(5);
    }
    else if(INIT_ENERGY_MAX == INIT_ENERGY_MIN){
        printf("Energy_min must differ from Energy_max! Retry again.\n");
        exit(5);
    }
    else if(INIT_ENERGY_MIN < 0 || INIT_ENERGY_MAX < 0){
        printf("Energies must be positive! Retry again.\n");
        exit(5);
    }
//********************** */
    if(fscanf(file, "decay_min=%d\n", &DECAY_RATE_MIN) !=1){
        printf("decay_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "decay_max=%d\n", &DECAY_RATE_MAX) !=1){
        printf("decay_max is non-integer! Retry again.\n");
        exit(10);
    }
    if(DECAY_RATE_MIN > DECAY_RATE_MAX){
        printf("Decay_min must be smaller than Decay_max! Retry again.\n");
        exit(6);
    }
    else if(DECAY_RATE_MAX == DECAY_RATE_MIN){
        printf("Decay_min must differ from Decay_max! Retry again.\n");
        exit(6);
    }
    else if(DECAY_RATE_MIN < 0 || DECAY_RATE_MAX < 0){
        printf("Decay time must be positive! Retry again.\n");
        exit(6);
    }
//********************** */
    if(fscanf(file, "recovery_min=%d\n", &RECOVERY_TIME_MIN) !=1){
        printf("recovery_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "recovery_max=%d\n", &RECOVERY_TIME_MAX) !=1){
        printf("recovery_max is non-integer! Retry again.\n");
        exit(10);
    }
    if(RECOVERY_TIME_MIN > RECOVERY_TIME_MAX){
        printf("Recovery_min must be smaller than Recovery_max! Retry again.\n");
        exit(7);
    }
    else if(RECOVERY_TIME_MAX == RECOVERY_TIME_MIN){
        printf("Recovery_min must differ from Recovery_max! Retry again.\n");
        exit(7);
    }
    else if(RECOVERY_TIME_MIN < 0 || RECOVERY_TIME_MAX < 0){
        printf("Recovery time must be positive! Retry again.\n");
        exit(7);
    }
    //********************** */
    if(fscanf(file, "threshold=%d\n", &THRESHOLD_TO_WIN) !=1){
        printf("threshold is non-integer! Retry again.\n");
        exit(10);
    }
    if(THRESHOLD_TO_WIN < 0 ){
        printf("Threshold to win must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "game_duration=%d\n", &GAME_DURATION) !=1){
        printf("game_duration is non-integer! Retry again.\n");
        exit(10);
    }
    if(GAME_DURATION < 0 ){
        printf("GAME_DURATION must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "score_limit=%d\n", &STREAK_TO_WIN) !=1){
        printf("score_limit is non-integer! Retry again.\n");
        exit(10);
    }
    if(STREAK_TO_WIN < 0 ){
        printf("STREAK_TO_WIN must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "total_rounds=%d\n", &total_rounds) !=1){
        printf("total_rounds is non-integer! Retry again.\n");
        exit(10);
    }
    if(total_rounds < 0 ){
        printf("total_rounds must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    COPY_GAME_DURATION = GAME_DURATION;
    fclose(file);
}


void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
      glVertex2f(cx, cy);
      for (int angle = 0; angle <= 360; angle += 10) {
         float rad = angle * (M_PI / 180.0);
         glVertex2f(cx + r * cos(rad), cy + r * sin(rad));
      }
    glEnd();
}

void renderBitmapString(float x, float y, void *font, char *string) {
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen(string); i++) {
        glutBitmapCharacter(font, string[i]);
    }
}

void initGL(void) {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw the rope fixed in the middle.
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_LINES);
       glVertex2f(0, WINDOW_HEIGHT / 2);
       glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT / 2);
    glEnd();
    // Draw dashed vertical midpoint line
    

glColor3f(0.7f, 0.0f, 0.0f); // Slightly darker red for visibility
glLineWidth(2.0f);

float dashLength = 10.0f;
float gapLength = 1.0f;

for (float y = WINDOW_HEIGHT /4; y < (3 * WINDOW_HEIGHT) /4; y += dashLength + gapLength) {
    glBegin(GL_LINES);
        glVertex2f(WINDOW_WIDTH / 2, y);
        glVertex2f(WINDOW_WIDTH / 2, y + dashLength);
    glEnd();
}

glLineWidth(1.0f); // Reset to default
    // Draw each player.
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        if (players[i].team == 1)
            glColor3f(1.0, 0.0, 0.0);  // red for Team 1
        else
            glColor3f(0.0, 0.0, 1.0);  // blue for Team 2

        drawCircle(players[i].x, players[i].y, players[i].radius);
        
        char energyText[20];
        sprintf(energyText, "E:%d", players[i].energy);
        glColor3f(0.0, 0.0, 0.0);
        renderBitmapString(players[i].x - players[i].radius,players[i].y - players[i].radius - 15,GLUT_BITMAP_HELVETICA_12,energyText);
        char decatText[20];
        sprintf(decatText, "D:%d", players[i].decay);
        glColor3f(0.0, 0.0, 0.0);
        renderBitmapString(players[i].x - players[i].radius,players[i].y - players[i].radius - 30,GLUT_BITMAP_HELVETICA_12,decatText);
    }
    
    // // Draw score information.
    // char scoreLabel[50] = "Score:";
    // glColor3f(0.0, 0.0, 0.0);
    // renderBitmapString(10, WINDOW_HEIGHT - 20, GLUT_BITMAP_HELVETICA_18, scoreLabel);
    
    // char score1Text[50], score2Text[50];
    // sprintf(score1Text, "Team 1 (Red): %d", score_team1);
    // sprintf(score2Text, "Team 2 (Blue): %d", score_team2);
    // glColor3f(1.0, 0.0, 0.0);
    // renderBitmapString(10, WINDOW_HEIGHT - 40, GLUT_BITMAP_HELVETICA_18, score1Text);
    // glColor3f(0.0, 0.0, 1.0);
    // renderBitmapString(10, WINDOW_HEIGHT - 60, GLUT_BITMAP_HELVETICA_18, score2Text);
    
    // Display totalEffort.
    if (game_phase == 2 || game_phase == 1) {
    char totalEffortText[50];
    sprintf(totalEffortText, "Total Effort: %d", global_totalEffort);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(WINDOW_WIDTH / 2 -50, 60, GLUT_BITMAP_HELVETICA_18, totalEffortText);
    }


    // Display heading information.
    char headingText[50];
    if (global_totalEffort > 0)
        sprintf(headingText, "Heading: Team 1 is winning");
    else if (global_totalEffort < 0)
        sprintf(headingText, "Heading: Team 2 is winning");
    else
        sprintf(headingText, "Heading: Tie");
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 100, GLUT_BITMAP_HELVETICA_18, headingText);

    // Display current round
    char roundText[50];
    sprintf(roundText, "Round: %d/%d", current_round, total_rounds);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 120, GLUT_BITMAP_HELVETICA_18, roundText);
    
    // Display rounds won by each team
    char roundsWonText1[50], roundsWonText2[50];
    sprintf(roundsWonText1, "Rounds Won by Team 1: %d", rounds_won_team1);
    sprintf(roundsWonText2, "Rounds Won by Team 2: %d", rounds_won_team2);
    glColor3f(1.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 140, GLUT_BITMAP_HELVETICA_18, roundsWonText1);
    glColor3f(0.0, 0.0, 1.0);
    renderBitmapString(10, WINDOW_HEIGHT - 160, GLUT_BITMAP_HELVETICA_18, roundsWonText2);
    



    //for Timer
    char timerText[50];
    sprintf(timerText, "Timer: %d", COPY_GAME_DURATION);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(WINDOW_WIDTH/2 - 40, WINDOW_HEIGHT -100 , GLUT_BITMAP_HELVETICA_18, timerText);




    // Display computed distance
    // char distanceText[50];
    // sprintf(distanceText, "Distance: %d (Threshold: %d, Needed: %d)", abs(global_totalEffort), totalEffort_threshold, user_defined_distance);
    // glColor3f(0.0, 0.0, 0.0);
    // renderBitmapString(10, WINDOW_HEIGHT - 180, GLUT_BITMAP_HELVETICA_18, distanceText);
  
    if (game_phase == 2 || game_phase == 1) {
        // --------- Display Team Efforts in Field (bottom center) ---------
    
        int team1_effort = 0;
        int team2_effort = 0;
        int t1 = 0, t2 = 0;
        int team1_idx[MEMBERS_PER_TEAM], team2_idx[MEMBERS_PER_TEAM];
    
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            if (players[i].team == 1)
                team1_idx[t1++] = i;
            else if (players[i].team == 2)
                team2_idx[t2++] = i;
        }
    
        for (int i = 0; i < MEMBERS_PER_TEAM - 1; i++) {
            for (int j = i + 1; j < MEMBERS_PER_TEAM; j++) {
                if (players[team1_idx[i]].energy > players[team1_idx[j]].energy) {
                    int tmp = team1_idx[i];
                    team1_idx[i] = team1_idx[j];
                    team1_idx[j] = tmp;
                }
                if (players[team2_idx[i]].energy > players[team2_idx[j]].energy) {
                    int tmp = team2_idx[i];
                    team2_idx[i] = team2_idx[j];
                    team2_idx[j] = tmp;
                }
            }
        }
    
        for (int i = 0; i < MEMBERS_PER_TEAM; i++) {
            team1_effort += players[team1_idx[i]].energy * (i + 1);
            team2_effort += players[team2_idx[i]].energy * (i + 1);
        }
    
        // Display Team 1 effort
        char effortT1[50];
        sprintf(effortT1, "Team 1 Effort: %d", team1_effort);
        glColor3f(1.0, 0.0, 0.0);
        renderBitmapString(WINDOW_WIDTH / 2 - 170, 100, GLUT_BITMAP_HELVETICA_18, effortT1);
    
        // Display Team 2 effort
        char effortT2[50];
        sprintf(effortT2, "Team 2 Effort: %d", team2_effort);
        glColor3f(0.0, 0.0, 1.0);
        renderBitmapString(WINDOW_WIDTH / 2 + 20, 100, GLUT_BITMAP_HELVETICA_18, effortT2);
    
        // ---------------------------------------------------------------
    }
    
    glutSwapBuffers();
}

void alignPlayers(void) {
    // Send SIGUSR1 to all player processes to trigger alignment
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        kill(players[i].pid, SIGUSR1);
    }

    // Wait for all players to send their alignment messages
    int alignment_count = 0;
    while (alignment_count < TOTAL_PLAYERS) {
        EffortMsg msg;
        ssize_t bytes = read(pipe_read_fd, &msg, sizeof(msg));
        if (bytes == sizeof(msg)) {
            for (int i = 0; i < TOTAL_PLAYERS; i++) {
                if (players[i].pid == msg.pid) {
                    players[i].energy = msg.energy;
                    players[i].effort = msg.effort;
                    players[i].decay = msg.decay;
                    players[i].returnAfter=msg.returnAfter;
                    break;
                }
            }
            if (msg.type == 1) {
                alignment_count++;
            }
        }
    }

    // Assign default target positions based on team and energy levels
    for (int team = 1; team <= NUM_TEAMS; team++) {
        int idx[MEMBERS_PER_TEAM], k = 0;

        // Collect indices for the players in the current team
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            if (players[i].team == team) {
                idx[k++] = i;
            }
        }

        if (k == MEMBERS_PER_TEAM) { // Ensure we process exactly 2 players per team
            // Sort the two players by energy in ascending order
            // (lower energy comes closer to the rope)
            for(int i = 0; i < MEMBERS_PER_TEAM; i++){
                for(int j =0; j < MEMBERS_PER_TEAM; j++){
                    if (players[idx[i]].energy < players[idx[j]].energy) {
                        int temp = idx[i];
                        idx[i] = idx[j];
                        idx[j] = temp;
                    }

                } 
            }

           int align = 60;
           for (int m = 0; m < 4; m++) {
                if (team == 1) { // Left team
                    players[idx[m]].targetX = WINDOW_WIDTH/2 - align;
                } else { // Right team
                    players[idx[m]].targetX = WINDOW_WIDTH/2 + align;
                }
                align+=40;
                players[idx[m]].targetY = WINDOW_HEIGHT / 2; // Align vertically
            }           

        }
    }

    // Update player positions to their target positions
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        players[i].x = players[i].targetX;
        players[i].y = players[i].targetY;
    }

    // Trigger OpenGL to redraw the display with updated positions
    glutPostRedisplay();
}

void startPullingPhase(void) {
    for (int i = 0; i < TOTAL_PLAYERS; i++)
        kill(players[i].pid, SIGUSR2);
    glutPostRedisplay();
}

void updateFromPipe(void) {
    EffortMsg msg;
    ssize_t bytes;
    while ((bytes = read(pipe_read_fd, &msg, sizeof(msg))) > 0) {
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            if (players[i].pid == msg.pid) {
                players[i].energy = msg.energy;
                players[i].effort = msg.effort;
                players[i].decay = msg.decay;
                players[i].returnAfter=msg.returnAfter;
                break;
            }
        }
    }
    if (game_phase == 2) {
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            players[i].x += 0.1f * (players[i].targetX - players[i].x);
            players[i].y += 0.1f * (players[i].targetY - players[i].y);
        }
    }
    glutPostRedisplay();
}

// void timer(int value) {
//     timercaller++;
//     if(current_round > total_rounds ){
//         printf("Terminating...\n");
//         glutLeaveMainLoop();
//         return;
//         //exit(0);
//     }
//     static int local_phase = 0;
//     printf("prev is: %d  curr is: %d\n", prev_round, current_round);
//     if(prev_round != current_round){
//         game_phase = 0;
//         local_phase = 0;
//         prev_round++;
//     }
//     //local_phase = 0;
//     printf("local phase is: %d\n", local_phase);
//     if (local_phase == 0) {
//         printf("\nReferee: Triggering alignment phase...\n");
//         if(current_round > total_rounds +1 ){
//             printf("Terminating...\n");
//             glutLeaveMainLoop();
//             return;
//             //exit(0);
//         }
//         glutIdleFunc(NULL);
//         alignPlayers();
//         local_phase = 1;
//         game_phase = 1;
//         glutIdleFunc(updateFromPipe);
//         //sleep(5);
//         //timer(0);
//     } else if (local_phase == 1) {
//         printf("\nReferee: Triggering pulling phase...\n");
//         startPullingPhase();
//         local_phase = 2;
//         game_phase = 2;
//     }


//     if(timercaller != 2){
//         glutTimerFunc(5000, timer, 0);
//     }
//     else{
//         timercaller = 0;
//     } 
//     /*if (current_round <= total_rounds) {
//         //timer(0);
//         //
//     }*/
// }
void timer(int value) {
    timercaller++;
    if(current_round > total_rounds){
        printf("Terminating...\n");
        glutLeaveMainLoop();
        return;
        //exit(0);
    }
    static int local_phase = 0;
    printf("prev is: %d  curr is: %d\n", prev_round, current_round);
    if(prev_round != current_round){
        game_phase = 0;
        local_phase = 0;
        prev_round++;
    }
    //local_phase = 0;
    printf("local phase is: %d\n", local_phase);
    if (local_phase == 0) {
        printf("\nReferee: Triggering alignment phase...\n");
        glutIdleFunc(NULL);
        alignPlayers();
        local_phase = 1;
        game_phase = 1;
        glutIdleFunc(updateFromPipe);
        //sleep(5);
        //timer(0);
    } else if (local_phase == 1) {
        printf("\nReferee: Triggering pulling phase...\n");
        
        startPullingPhase();
        printf("current jjjj\n");
        local_phase = 2;
        game_phase = 2;
    }


    if(timercaller != 2){
        glutTimerFunc(5000, timer, 0);
    }
    else{
        timercaller = 0;
    } 
    /*if (current_round <= total_rounds) {
        //timer(0);
        //
    }*/
}
void updateScoreTimer(int value) {
    printf("current phase is: %d\n", game_phase);
    printf("Current round is: %d *************\n", current_round);
    if (game_phase == 2 && current_round <= total_rounds ) { // if in pulling phase.
        printf("Current round is: %d *************\n", current_round);
        int team1_effort = 0;
        int team2_effort = 0;
        int team1_idx[4], team2_idx[4];
        int t1 = 0, t2 = 0;

        // Collect indices per team
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            printf("Player %d targetX: %f\n", i, players[i].targetX);
            if (players[i].team == 1)
                team1_idx[t1++] = i;
            else if (players[i].team == 2)
                team2_idx[t2++] = i;
        }


        for(int i =0; i < MEMBERS_PER_TEAM; i++){
            for(int j =0; j < MEMBERS_PER_TEAM; j++){
                if (players[team1_idx[i]].energy < players[team1_idx[j]].energy) {
                    int temp = team1_idx[i];
                    team1_idx[i] = team1_idx[j];
                    team1_idx[j] = temp;
                }
            }
        }

        for(int i =0; i < MEMBERS_PER_TEAM; i++){
            for(int j =0; j < MEMBERS_PER_TEAM; j++){
                if (players[team2_idx[i]].energy < players[team2_idx[j]].energy) {
                    int temp = team2_idx[i];
                    team2_idx[i] = team2_idx[j];
                    team2_idx[j] = temp;
                }
            } 
        }

        //******************************************* */
// Midpoint defeat rule: if any player crosses the center line, their team loses
for (int i = 0; i < TOTAL_PLAYERS; i++) {
    if (players[i].team == 1 && players[i].x >= (WINDOW_WIDTH / 2) -15) {
        // Team 1 crossed the center - lose the round
        rounds_won_team2++;
        printf("Team 1 crossed the midpoint! Team 2 wins Round %d by rule.\n", current_round);
        current_round++;
        for (int k = 0; k < TOTAL_PLAYERS; k++) kill(players[k].pid, SIGPWR);
        glutTimerFunc(1000, timer, 0);
        // return;
    }
    if (players[i].team == 2 && players[i].x <= (WINDOW_WIDTH / 2) +15) {
        // Team 2 crossed the center - lose the round
        rounds_won_team1++;
        printf("Team 2 crossed the midpoint! Team 1 wins Round %d by rule.\n", current_round);
        current_round++;
        for (int k = 0; k < TOTAL_PLAYERS; k++) kill(players[k].pid, SIGPWR);
        glutTimerFunc(1000, timer, 0);
        // return;
    }
}
//******************************************************************* */
        // Calculate effective efforts
        team1_effort = players[team1_idx[0]].energy * 1 + players[team1_idx[1]].energy * 2 + players[team1_idx[2]].energy * 3 + players[team1_idx[3]].energy * 4;
        team2_effort = players[team2_idx[0]].energy * 1 + players[team2_idx[1]].energy * 2 + players[team2_idx[2]].energy * 3 + players[team2_idx[3]].energy * 4;

        // Compute totalEffort
        int totalEffort = team1_effort - team2_effort;
        global_totalEffort = totalEffort;


        //i changed the win criteria, the team wins if the another team reached the midpoint line.

        // Compute distance based on totalEffort exceeding the threshold
        //int distance = (abs(totalEffort) > totalEffort_threshold) ? abs(totalEffort) - totalEffort_threshold : 0;
        //printf("distance is : %d\n", distance);
        // Check if a team has won the round
       /* if (distance >= user_defined_distance) {
           // printf("i am entering here fella\n");
            if (totalEffort > 0) {
                rounds_won_team1++;
                printf("Team 1 wins Round %d!\n", current_round);
            } else if (totalEffort < 0) {
                rounds_won_team2++;
                printf("Team 2 wins Round %d!\n", current_round);
            }
            current_round++; 
            for (int i = 0; i < TOTAL_PLAYERS; i++)
                kill(players[i].pid, SIGPWR);
            glutTimerFunc(1000, timer, 0);
            printf("called the timer\n");
            // Reset everything for the new round
            if (current_round <= total_rounds) {
                //sleep(1)
                alignPlayers(); // Re-align players to start fresh
                printf("Starting Round %d...\n", current_round);  
                glutTimerFunc(1000, updateScoreTimer, 0);
                return; // Restart loop for the new round
            } else {
                // End the game
                printf("Game Over! Final Score:\n");
                printf("Team 1: %d rounds\n", rounds_won_team1);
                printf("Team 2: %d rounds\n", rounds_won_team2);
                return; // No more rounds
            }
        }
*/  
        // Smaller displacement for clearer pulling effect
       /* int displacement = (totalEffort > 10) ? -10 : (totalEffort < -10) ? 10 : 0;
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            players[i].targetX += displacement;
        }*/

       // make the movement adjustable according to total effort.
        float maxEffort = 100.0f; 
        float effortFactor = (float)totalEffort / maxEffort;

        // to prevent large steps.
        if (effortFactor > 1.0f) effortFactor = 1.0f;
        if (effortFactor < -1.0f) effortFactor = -1.0f;

       
        float displacement = -effortFactor * 15.0f;

        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            players[i].targetX += displacement;
        }
    }
    printf("cccccccccccccccccccccccccccccc\n");
    glutTimerFunc(1000, updateScoreTimer, 0);
}

void alignment_handler(int sig) {
    EffortMsg msg;
    msg.pid = getpid();
    msg.team = child_team;
    msg.member = child_member;
    msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
    msg.energy = child_energy;
    msg.decay = child_decay;
    msg.returnAfter = child_return_after;

    msg.type = 1;  // alignment update
    write(child_pipe_fd, &msg, sizeof(msg));
}

void pulling_handler(int sig) {
    pulling_flag = 1;
    EffortMsg msg;
    msg.pid = getpid();
    msg.team = child_team;
    msg.member = child_member;
    msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
    msg.energy = child_energy;
    msg.decay = child_decay;
    msg.returnAfter = child_return_after;
    msg.type = 2;  // pulling update
    write(child_pipe_fd, &msg, sizeof(msg));
}

void newrnd(int sig){
    current_round++;
    //check if the new round larger than the total round, then terminate.
    if(current_round >= total_rounds +1){
        //terminate
       
        for(int k=0;k<TOTAL_PLAYERS; k++) kill(players[k].pid,SIGKILL);
        exit(0);
    }
    pulling_flag = 0;
}

void child_process(int team, int member) {
    // Initialize player attributes.
    srand(time(NULL) ^ getpid());
    child_team = team;
    child_member = member;
    child_energy = INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);
    child_decay = DECAY_RATE_MIN + rand() % (DECAY_RATE_MAX - DECAY_RATE_MIN + 1);
    child_return_after = RECOVERY_TIME_MIN + rand() % (RECOVERY_TIME_MAX - RECOVERY_TIME_MIN + 1);

    
    printf("Player created: Team %d Member %d, PID %d, Initial Energy = %d, Decay = %d\n",team, member, getpid(), child_energy, child_decay);
    
    // Set signal handlers using sigset(). if the child gets these signals, go to a custom function, not doing the default behavior.
    sigset(SIGUSR1, alignment_handler);     // Handle alignment signal.
    sigset(SIGUSR2, pulling_handler);       // Handle pulling signal.
    sigset(SIGPWR, newrnd);                //
    // Assign the inherited write end of the pipe.
    child_pipe_fd = pipefd[1];
    
    // Main simulation loop.
    while (1) {
        //printf("current rnd from child: %d\n", current_round);
        if(current_round < total_rounds +1){
            //printf("current rnd from child: %d\n", current_round);
            //printf("Cat\n");
            //enters this always.
        }
        else if(current_round == total_rounds){
           //printf("current rnd from childdd: %d\n", current_round);
           printf("my name\n");
        }
        else{
            printf("i am here exiting\n");
            printf("moazzzzz\n");
            exit(0);
        }
        
        sleep(1);  // Wait for one second per iteration.
        
        if (pulling_flag) { //enters in pulling phase.
            
           // printf("inside the if");
            // Decrement energy during the pulling phase.
            child_energy -= child_decay;
            if(child_energy <= 0){ child_energy=0;child_energy =  INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);}
            if (child_energy <= 0) { // it won't enter this!!
                // Set energy to 0, send the exhaustion update, and exit.
                child_energy = 0;
                EffortMsg msg;
                msg.pid = getpid();
                msg.team = child_team;
                msg.member = child_member;
                msg.effort = 0;
                msg.energy = child_energy;
                msg.decay = child_decay;
                msg.returnAfter = child_return_after;
                
                msg.type = 0;  // periodic update
                write(child_pipe_fd, &msg, sizeof(msg));
                printf("Player (Team %d Member %d, PID %d) is exhausted. they will return after %ds.\n",child_team, child_member, getpid(),child_return_after);
                exit(0); //if they got exhasted they will die, but if they fall, they will return back.


                //sleep(child_return_after);child_energy =  INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);
            }
        }
        
        // Send periodic updates to the parent process.
        EffortMsg msg;
        msg.pid = getpid();
        msg.team = child_team;
        msg.member = child_member;
        msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
        msg.energy = child_energy;
        msg.decay = child_decay;
        msg.returnAfter = child_return_after;
        msg.type = 0;  // periodic update
        write(child_pipe_fd, &msg, sizeof(msg));
    }
}

int main(){
    readFile("inputs.txt");
    srand(time(NULL));
    
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }
    pipe_read_fd = pipefd[0];
    
    int flags = fcntl(pipe_read_fd, F_GETFL, 0);
    fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK); // changing the pipe to be non-blocking, - when reading the pipe with no data, it won't stuck.
    
    //mkfifo("effort_fifo", 0666); //makes named pipe -fifo- to enable communication between unrelated processes, but it is not used till now!

    int idx = 0;

    for (int i = 0; i < NUM_TEAMS; i++) {
        for (int j = 0; j < MEMBERS_PER_TEAM; j++) {
            pid_t pid = fork();
            sleep(0.3);
            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) { // the same child.
                //printf("Hello child %d %d  %d\n", idx ,getpid(), pid);
                child_process(i + 1, j + 1);
                exit(EXIT_SUCCESS);
            } else { //parent of them all.
                //printf("parent %d %d\n", getpid(),pid);
                players[idx].team = i + 1;
                players[idx].member = j + 1;
                players[idx].pid = pid;
                players[idx].radius = 15.0f;
                players[idx].x = (float)(rand() % (WINDOW_WIDTH - 50) + 25);
                players[idx].y = (float)(rand() % (WINDOW_HEIGHT - 50) + 25);
                players[idx].targetX = players[idx].x;
                players[idx].targetY = players[idx].y;
                players[idx].energy = 0; // why zero??
                players[idx].effort = 0; // why zero??
                players[idx].decay = 0;
                players[idx].returnAfter=0;
                idx++;
            }
        }
    }


    //sleep(1);
    int argc = 1;
    char *argv[1] = { "RopeGame" };
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutTimerFunc(1000, countdownTimer, 0);
    glutCreateWindow("Rope Pulling Game Simulation");
    glutWMCloseFunc(handleClose); //this kills all related processes when the user closes the GUI.
    initGL();
    glutDisplayFunc(display);
    glutIdleFunc(updateFromPipe);
    //timer(0);
    glutTimerFunc(5000, timer, 0);          // Phase transition timer.
    glutTimerFunc(1000, updateScoreTimer, 0); // Score, totalEffort, and uniform pulling update timer.
    glutMainLoop();
    return 0;
}


