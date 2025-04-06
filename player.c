#include "headers.h"

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
    sigset(SIGPWR, newrnd);                // Handle new round signal.
    
    // Assign the inherited write end of the pipe.
    child_pipe_fd = pipefd[1];
    
    // Main simulation loop.
    while (1) {
        //printf("current rnd from child: %d\n", current_round);
        if(current_round <= total_rounds){
            //printf("current rnd from child: %d\n", current_round);
            printf("Round %d in progress...\n", current_round);
            //enters this always.
        }
        else{
           // printf("i am here exiting\n");
           // printf("moazzzzz\n");
            exit(0);
        }
        
        sleep(1);  // Wait for one second per iteration.
       
        if (pulling_flag) { //enters in pulling phase.
         //printf("i am here insider\n");
        // Generate a random number between 0 and 99
        int chance = rand() % 100;
        if (chance < FALL_PROBABILITY) {
            // Save the current energy before falling
        printf("Player (Team %d Member %d, PID %d) fell!\n", child_team, child_member, getpid());
        int saved_energy = child_energy;
        child_energy = 0;  // Set effort to zero
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

        printf("Player (Team %d Member %d, PID %d) accidentally fell. Recovering for %ds.\n",
               child_team, child_member, getpid(), child_return_after);

        // Sleep for the recovery period
        sleep(child_return_after);

        // Restore the saved energy
        child_energy = saved_energy;
        
        // Optionally, send an update after recovery
        msg.effort = child_energy / ((child_decay == 0) ? 1 : child_decay);
        msg.energy = child_energy;
        write(child_pipe_fd, &msg, sizeof(msg));

        } else {
            // printf("inside the if");
            // Decrement energy during the pulling phase.
            child_energy -= child_decay;
            //if(child_energy <= 0){ child_energy=0;child_energy =  INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);}
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
                printf("Player (Team %d Member %d, PID %d) is exhausted. they will return the next round.\n",child_team, child_member, getpid());
                //exit(0); //if they got exhasted they will die, but if they fall, they will return back.
                sleep(1000);

                //sleep(child_return_after);child_energy =  INIT_ENERGY_MIN + rand() % (INIT_ENERGY_MAX - INIT_ENERGY_MIN + 1);
            }


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