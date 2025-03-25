#include "header.h"


void draw_text(float x, float y, char *string) {
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);
    for(char *c = string; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

void draw_player(float x, float y, int team, int energy, bool active) {
    glColor3f(team == 1 ? 0.0 : 1.0, 0.0, team == 1 ? 1.0 : 0.0);
    if(!active) glColor4f(0.5, 0.5, 0.5, 0.5);
    
    if(team == 1) {
        glBegin(GL_QUADS);
        glVertex2f(x-15, y-15);
        glVertex2f(x+15, y-15);
        glVertex2f(x+15, y+15);
        glVertex2f(x-15, y+15);
    } else {
        glBegin(GL_TRIANGLES);
        glVertex2f(x-15, y-15);
        glVertex2f(x+15, y-15);
        glVertex2f(x, y+15);
    }
    glEnd();

    char buf[10];
    snprintf(buf, 10, "%d", energy);
    draw_text(x-10, y+20, buf);
}

void draw_players() {
    float base_y = WINDOW_HEIGHT/2.0f;
    float spacing = WINDOW_WIDTH/(config.team_players+1.0f);
    
    // Team 1 (Squares - Left side)
    for(int i = 0; i < config.team_players; i++) {
        draw_player(100 + i*spacing, base_y, 1, team1[i].energy, team1[i].recovery_time == 0);
    }
    
    // Team 2 (Triangles - Right side)
    for(int i = 0; i < config.team_players; i++) {
        draw_player(WINDOW_WIDTH - 100 - i*spacing, base_y, 2, team2[i].energy, team2[i].recovery_time == 0);
    }
}

void draw_rope() {
    glColor3f(0.5, 0.35, 0.05);
    glLineWidth(5.0);
    glBegin(GL_LINES);
    float center = WINDOW_WIDTH/2 + (rope_offset * WINDOW_WIDTH/2);
    glVertex2f(center - 200, WINDOW_HEIGHT/2);
    glVertex2f(center + 200, WINDOW_HEIGHT/2);
    glEnd();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Game status
    char status[100];
    snprintf(status, 100, "Round: %d  Time: %d  Score: %d-%d", 
            current_round, time_remaining, score_team1, score_team2);
    draw_text(10, WINDOW_HEIGHT-30, status);
    
    if(game_over) {
        char result[50];
        snprintf(result, 50, "Winner: %s", 
                score_team1 > score_team2 ? "Team 1 (Squares)" : "Team 2 (Triangles)");
        draw_text(WINDOW_WIDTH/2-100, WINDOW_HEIGHT/2, result);
    }
    else {
        update_game_state();
        draw_players();
        draw_rope();
    }
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void init_graphics(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Rope Pulling Game");
    glClearColor(0.1, 0.1, 0.1, 1.0);
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glutDisplayFunc(render);
    glutMainLoop();
}