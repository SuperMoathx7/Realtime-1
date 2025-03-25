#include "header.h"

void init_graphics(int *argc, char **argv) {
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rope Pulling Game");
    glClearColor(1.0, 1.0, 1.0, 1.0);
    gluOrtho2D(0, 800, 0, 600);
    glutDisplayFunc(update_display);
}

void update_display(int effort1, int effort2) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw teams
    glColor3f(0.0, 0.0, 1.0);
    glRecti(100, 200, 200, 300);  // Team 1
    
    glColor3f(1.0, 0.0, 0.0);
    glRecti(600, 200, 700, 300);  // Team 2
    
    // Draw rope
    glColor3f(0.5, 0.35, 0.05);
    glLineWidth(5.0);
    glBegin(GL_LINES);
    glVertex2i(250, 250);
    glVertex2i(550, 250);
    glEnd();
    
    glFlush();
}