#include "headers.h"

void countdownTimer(int value) {
    if (game_phase == 2 && GAME_DURATION > 0) {  // Pulling phase
        GAME_DURATION--;
        glutPostRedisplay(); // Update display
    }

    // Keep calling every second regardless, but only decrement if in phase 2
    if (GAME_DURATION > 0) {
        glutTimerFunc(1000, countdownTimer, 0);
    } else if (GAME_DURATION <= 0) {
        printf("Sumulation ends: Game Duration ends!\n");
        printFinalSummary();

        //current_round++;
        for(int k=0;k<TOTAL_PLAYERS; k++) 
            kill(players[k].pid,SIGKILL);
        
        exit(0);
        //GAME_DURATION = GAME_DURATION;

        //glutTimerFunc(1000, timer, 0);
    }
}

// Close the GUI and kill all child processes
void handleClose() {  
    printf("Window closed by user. Cleaning up...\n");
    
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        kill(players[i].pid, SIGKILL);
    }

    exit(0);
}


// draw a circle
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
      glVertex2f(cx, cy);
      for (int angle = 0; angle <= 360; angle += 10) {
         float rad = angle * (M_PI / 180.0);
         glVertex2f(cx + r * cos(rad), cy + r * sin(rad));
      }
    glEnd();
}

// Draw a string at a given position
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

// This function is called whenever glotPostRedisplay() is called to
// update the display
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
        
        // Draw the player as a circle.
        drawCircle(players[i].x, players[i].y, players[i].radius);
        
        // shows the energy of each player.
        char energyText[20];
        sprintf(energyText, "E:%d", players[i].energy);
        glColor3f(0.0, 0.0, 0.0);
        renderBitmapString(players[i].x - players[i].radius,players[i].y - players[i].radius - 15,GLUT_BITMAP_HELVETICA_12,energyText);
        
        // shows the decay of each player.        
        char decatText[20];
        sprintf(decatText, "D:%d", players[i].decay);
        glColor3f(0.0, 0.0, 0.0);
        renderBitmapString(players[i].x - players[i].radius,players[i].y - players[i].radius - 30,GLUT_BITMAP_HELVETICA_12,decatText);
    }
    

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
    sprintf(timerText, "Timer: %d", GAME_DURATION);
    glColor3f(0.0, 0.0, 0.0);
    renderBitmapString(WINDOW_WIDTH/2 - 40, WINDOW_HEIGHT -100 , GLUT_BITMAP_HELVETICA_18, timerText);


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

        // Sort players in each team by energy in descending order
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