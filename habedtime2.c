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

// ---------------- Configuration Macros ----------------
#define WINDOW_WIDTH   1500
#define WINDOW_HEIGHT  800

#define NUM_TEAMS       2
#define MEMBERS_PER_TEAM 4
#define TOTAL_PLAYERS   (NUM_TEAMS * MEMBERS_PER_TEAM)

#define INIT_ENERGY_MIN 80
#define INIT_ENERGY_MAX 100
#define DECAY_RATE_MIN   1
#define DECAY_RATE_MAX   5

// ---------------- Message Structure ----------------
// type: 0 = periodic update, 1 = alignment update, 2 = pulling update.
typedef struct {
    pid_t pid;
    int team;       // 1 or 2
    int member;     // e.g., 1 or 2
    int effort;     // computed as energy/decay_rate (child-side computation)
    int energy;     // current energy (child-side)
    int type;       // message type
} EffortMsg;

// ---------------- Player Display Structure ----------------
typedef struct {
    int team;
    int member;
    int energy;            // updated from child's messages
    int effort;            // reported effort (without parent's multiplier)
    float x, y;            // current display position
    float targetX, targetY;// target positions (set during alignment and pulling)
    float radius;
    pid_t pid;             // associated child's PID
} PlayerInfo;

PlayerInfo players[TOTAL_PLAYERS];

#define SIGUSR3 (SIGRTMIN + 0)

// ---------------- Global IPC Descriptors ----------------
int pipefd[2];     // pipefd[0] is used by the parent for reading; pipefd[1] is inherited by children.
int pipe_read_fd;  // alias for parent's read end

// ---------------- Globals in Child Processes ----------------
// (Each child)
int child_team, child_member;
volatile int child_energy, child_decay;
// Before pulling phase, energy remains constant.
// Once pulling_flag is set (via SIGUSR2), energy decay begins.
volatile int pulling_flag = 0;
int child_pipe_fd; // inherited write end

// ---------------- Global for Parent ----------------
int game_phase = 0; // 0 = waiting for alignment, 1 = aligned, 2 = pulling active
int score_team1 = 0, score_team2 = 0;
// Global delta computed from effective efforts (updated in updateScoreTimer).
int global_delta = 0;

int total_rounds = 5;         // Total number of rounds in the game
int current_round = 1;        // Current round number
int prev_round = 1;
int rounds_won_team1 = 0;     // Rounds won by Team 1
int rounds_won_team2 = 0;     // Rounds won by Team 2
int user_defined_distance = 50;  // Distance needed to win a round (user-defined)
int delta_threshold = 20;     // Delta threshold to compute distance
int timercaller = 0;
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
    }
    
    // Draw score information.
    char scoreLabel[50] = "Score:";
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 20, GLUT_BITMAP_HELVETICA_18, scoreLabel);
    
    char score1Text[50], score2Text[50];
    sprintf(score1Text, "Team 1 (Red): %d", score_team1);
    sprintf(score2Text, "Team 2 (Blue): %d", score_team2);
    glColor3f(1.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 40, GLUT_BITMAP_HELVETICA_18, score1Text);
    glColor3f(0.0, 0.0, 1.0);
    renderBitmapString(10, WINDOW_HEIGHT - 60, GLUT_BITMAP_HELVETICA_18, score2Text);
    
    // Display delta.
    char deltaText[50];
    sprintf(deltaText, "Delta: %d", global_delta);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 80, GLUT_BITMAP_HELVETICA_18, deltaText);
    
    // Display heading information.
    char headingText[50];
    if (global_delta > 0)
        sprintf(headingText, "Heading: Team 1 is winning");
    else if (global_delta < 0)
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
    
    // Display computed distance
    char distanceText[50];
    sprintf(distanceText, "Distance: %d (Threshold: %d, Needed: %d)", abs(global_delta), delta_threshold, user_defined_distance);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(10, WINDOW_HEIGHT - 180, GLUT_BITMAP_HELVETICA_18, distanceText);
  
    
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

           int align = 20;
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
    printf("fucking phase is: %d\n", game_phase);
    //printf("i am here: current round: %d, game phase: %d\n", current_round, game_phase);
    if (game_phase == 2 && current_round <= total_rounds) {
        int team1_effort = 0;
        int team2_effort = 0;
        int team1_idx[4], team2_idx[4];
        int t1 = 0, t2 = 0;

        // Collect indices per team
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
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


        // Calculate effective efforts
        team1_effort = players[team1_idx[0]].energy * 1 + players[team1_idx[1]].energy * 2 + players[team1_idx[2]].energy * 3 + players[team1_idx[3]].energy * 4;
        team2_effort = players[team2_idx[0]].energy * 1 + players[team2_idx[1]].energy * 2 + players[team2_idx[2]].energy * 3 + players[team2_idx[3]].energy * 4;

        // Compute delta
        int delta = team1_effort - team2_effort;
        global_delta = delta;

        // Compute distance based on delta exceeding the threshold
        int distance = (abs(delta) > delta_threshold) ? abs(delta) - delta_threshold : 0;
        printf("distance is : %d\n", distance);
        // Check if a team has won the round
        if (distance >= user_defined_distance) {
           // printf("i am entering here fella\n");
            if (delta > 0) {
                rounds_won_team1++;
                printf("Team 1 wins Round %d!\n", current_round);
            } else if (delta < 0) {
                rounds_won_team2++;
                printf("Team 2 wins Round %d!\n", current_round);
            }
            current_round++;
            for (int i = 0; i < TOTAL_PLAYERS; i++)
                kill(players[i].pid, SIGUSR3);
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

        // Smaller displacement for clearer pulling effect
        int displacement = (delta > 10) ? -10 : (delta < -10) ? 10 : 0;
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
            players[i].targetX += displacement;
        }
    }
    glutTimerFunc(1000, updateScoreTimer, 0);
}

void alignment_handler(int sig) {
    EffortMsg msg;
    msg.pid = getpid();
    msg.team = child_team;
    msg.member = child_member;
    msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
    msg.energy = child_energy;
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
    msg.type = 2;  // pulling update
    write(child_pipe_fd, &msg, sizeof(msg));
}

void newrnd(int sig){
    current_round++;
    pulling_flag = 0;
}

void child_process(int team, int member) {
    // Initialize player attributes.
    srand(time(NULL) ^ getpid());
    child_team = team;
    child_member = member;
    child_energy = INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);
    child_decay = DECAY_RATE_MIN + rand() % (DECAY_RATE_MAX - DECAY_RATE_MIN + 1);
    
    printf("Player created: Team %d Member %d, PID %d, Initial Energy = %d, Decay = %d\n",team, member, getpid(), child_energy, child_decay);
    
    // Set signal handlers using sigset().
    sigset(SIGUSR1, alignment_handler);  // Handle alignment signal.
    sigset(SIGUSR2, pulling_handler);   // Handle pulling signal.
    sigset(SIGUSR3, newrnd);
    // Assign the inherited write end of the pipe.
    child_pipe_fd = pipefd[1];
    
    // Main simulation loop.
    while (1) {
        //printf("current rnd from child: %d\n", current_round);
        if(current_round < total_rounds){
            //printf("current rnd from child: %d\n", current_round);
        }
        else if(current_round == 5){
           //printf("current rnd from childdd: %d\n", current_round);
        }
        else{
            printf("i am here exiting\n");
            exit(0);
        }
        
        sleep(1);  // Wait for one second per iteration.
        
        if (pulling_flag) {
            // Decrement energy during the pulling phase.
            child_energy -= child_decay;
            if(child_energy <= 0){ child_energy =  INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);}
            if (child_energy <= 0) {
                // Set energy to 0, send the exhaustion update, and exit.
                child_energy = 0;
                EffortMsg msg;
                msg.pid = getpid();
                msg.team = child_team;
                msg.member = child_member;
                msg.effort = 0;
                msg.energy = child_energy;
                msg.type = 0;  // periodic update
                write(child_pipe_fd, &msg, sizeof(msg));
                printf("Player (Team %d Member %d, PID %d) is exhausted.\n",child_team, child_member, getpid());
                exit(0);
            }
        }
        
        // Send periodic updates to the parent process.
        EffortMsg msg;
        msg.pid = getpid();
        msg.team = child_team;
        msg.member = child_member;
        msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
        msg.energy = child_energy;
        msg.type = 0;  // periodic update
        write(child_pipe_fd, &msg, sizeof(msg));
    }
}

int main(){

    srand(time(NULL));
    
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }
    pipe_read_fd = pipefd[0];
    
    int flags = fcntl(pipe_read_fd, F_GETFL, 0);
    fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK);
    
    mkfifo("effort_fifo", 0666);

    int idx = 0;

    for (int i = 0; i < NUM_TEAMS; i++) {
        for (int j = 0; j < MEMBERS_PER_TEAM; j++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                //printf("Hello child %d %d  %d\n", idx ,getpid(), pid);
                child_process(i + 1, j + 1);
                exit(EXIT_SUCCESS);
            } else {
                //printf("parent %d %d\n", getpid(),pid);
                players[idx].team = i + 1;
                players[idx].member = j + 1;
                players[idx].pid = pid;
                players[idx].radius = 15.0f;
                players[idx].x = (float)(rand() % (WINDOW_WIDTH - 50) + 25);
                players[idx].y = (float)(rand() % (WINDOW_HEIGHT - 50) + 25);
                players[idx].targetX = players[idx].x;
                players[idx].targetY = players[idx].y;
                players[idx].energy = 0;
                players[idx].effort = 0;
                idx++;
            }
        }
    }

    sleep(1);
    int argc = 1;
    char *argv[1] = { "RopeGame" };
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Rope Pulling Game Simulation");
    initGL();
    glutDisplayFunc(display);
    glutIdleFunc(updateFromPipe);
    //timer(0);
    glutTimerFunc(5000, timer, 0);          // Phase transition timer.
    glutTimerFunc(1000, updateScoreTimer, 0); // Score, delta, and uniform pulling update timer.
    glutMainLoop();
    return 0;
}