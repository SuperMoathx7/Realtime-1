//remaining things:
//done  1-There is a bug When starting a new round. Doneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//  2-Accidentally falling.
//done  3-Stop when reaching game Duration.  Doneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//done  4-Stop when reaching Win Streak. Doneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//  5-Parent must tell Childs that their team won or lost.
//  6-Bug: if all energies reach zero in a pulling phase in a round, the game will stuck till the end of the timer. Must we fix it? or it is realistic case?
//done   7-Energy in new round didn't generated again, it contiunues with the last values.#include <stdio.h> doneeeeeeeeeeeee
//----------------------------------------------------------------------------------------------
#include "headers.h"


int main(){
    readFile("inputs.txt");
    srand(time(NULL));

    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }
    pipe_read_fd = pipefd[0]; // read end of the pipe.

    // Set the read end of the pipe to non-blocking mode
    int flags = fcntl(pipe_read_fd, F_GETFL, 0);
    fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK); // changing the pipe to be non-blocking, - when reading the pipe with no data, it won't stuck.
    

  

    int idx = 0;

    // creating the players
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
