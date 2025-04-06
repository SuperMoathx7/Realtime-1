#include "headers.h"


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

void timer(int value) {
    timercaller++;
    if(current_round > total_rounds){
        printf("Terminating...\n");
        printFinalSummary();
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
        //printf("current jjjj\n");
        local_phase = 2;
        game_phase = 2;
    }


    if(timercaller != 2){
        //printf("entered here\n");
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
    //printf("Current round is: %d *************\n", current_round);
    if (game_phase == 2 && current_round <= total_rounds ) { // if in pulling phase.
        //printf("Current round is: %d *************\n", current_round);
        int team1_effort = 0;
        int team2_effort = 0;
        int team1_idx[4], team2_idx[4];
        int t1 = 0, t2 = 0;

        // Collect indices per team
        for (int i = 0; i < TOTAL_PLAYERS; i++) {
           // printf("Player %d targetX: %f\n", i, players[i].targetX);
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

        // Sort players in team 2 by energy in descending order
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
        consecutiveWinsTeam2++;      // increment team2 consecutive win counter
        printf("Team 1 crossed the midpoint! Team 2 wins Round %d by rule.\n", current_round);
        consecutiveWinsTeam1 = 0;      // reset team1 consecutive wins

        if (consecutiveWinsTeam2 >= STREAK_TO_WIN) {
            printf("Team 2 wins the game with a streak of %d times!\n", consecutiveWinsTeam2);
           printFinalSummary();
            for (int i = 0; i < TOTAL_PLAYERS; i++) {
                kill(players[i].pid, SIGPWR);
            }
        
            exit(0);
            
        }

        current_round++;
        if(current_round >= total_rounds +1) printFinalSummary();
        for (int k = 0; k < TOTAL_PLAYERS; k++) kill(players[k].pid, SIGPWR);
        glutTimerFunc(1000, timer, 0);
       // printf("i am erroring here1\n");
        // return;
    }
    if (players[i].team == 2 && players[i].x <= (WINDOW_WIDTH / 2) +15) {
        // Team 2 crossed the center - lose the round
        rounds_won_team1++;
        consecutiveWinsTeam1++;      // increment team1 consecutive win counter
        consecutiveWinsTeam2 = 0;      // reset team2 consecutive wins
        printf("Team 2 crossed the midpoint! Team 1 wins Round %d by rule.\n", current_round);
        if (consecutiveWinsTeam1 >= STREAK_TO_WIN) {
            printf("Team 1 wins the game with a streak of %d times!\n", consecutiveWinsTeam1);
           printFinalSummary();
            for (int i = 0; i < TOTAL_PLAYERS; i++) {
                kill(players[i].pid, SIGPWR);
            }
        
            exit(0);
            
        }


        current_round++;
        if(current_round >= total_rounds +1) printFinalSummary();
        for (int k = 0; k < TOTAL_PLAYERS; k++) kill(players[k].pid, SIGPWR);
        glutTimerFunc(1000, timer, 0);
        //printf("i am erroring here2\n");
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

        int counter=0;
        for(int k =0;k<TOTAL_PLAYERS;k++){
            if(players[k].energy == 0) counter++;
            
            if(counter == 8){//Tie.........
                //printf("helllo from hell\n");
                if(current_round+1 < total_rounds){current_round++;//the round will be counted, but no one wins.}
                }
                
                printf("All players get tried! No one wins! Tieeeeeeeeeeeeeeeeee.\n");
                for(int f=0;f<TOTAL_PLAYERS;f++)kill(players[f].pid,SIGPWR);//new round.
                glutTimerFunc(1000, timer, 0);


            }
        }


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
    //printf("cccccccccccccccccccccccccccccc\n");
    glutTimerFunc(1000, updateScoreTimer, 0);
}

// Function to print the final game summary.
void printFinalSummary(void) {
    printf("\n========== Game Summary ==========\n");
    printf("Rounds Won by Team 1: %d\n", rounds_won_team1);
    printf("Rounds Won by Team 2: %d\n", rounds_won_team2);
    if (rounds_won_team1 > rounds_won_team2)
        printf("Team 1 wins the game!\n");
    else if (rounds_won_team2 > rounds_won_team1)
        printf("Team 2 wins the game!\n");
    else
        printf("The game is a tie!\n");
    printf("========================================\n");
}