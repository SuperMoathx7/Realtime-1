#include "headers.h"


// Signal handler for alignment phase   
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

// Signal handler for pulling phase
void pulling_handler(int sig) {
    printf("calling the pulling handler\n");
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

// Signal handler for new round
void newrnd(int sig){
    current_round++;//for child
    //check if the new round larger than the total round, then terminate.
    if(current_round >= total_rounds +1){
        //terminate
        for(int k=0;k<TOTAL_PLAYERS; k++) 
            kill(players[k].pid,SIGKILL);
        exit(0);
    }
    // Regenerate the child's parameters for the new round.
    child_energy = INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);
    child_decay = DECAY_RATE_MIN + rand() % (DECAY_RATE_MAX - DECAY_RATE_MIN + 1);
    child_return_after = RECOVERY_TIME_MIN + rand() % (RECOVERY_TIME_MAX - RECOVERY_TIME_MIN + 1);


    pulling_flag = 0;
}