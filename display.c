#define _CRT_SECURE_NO_WARNINGS
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "read.h"
#include "write.h"

static Image* g_imgs = NULL;
static int g_nimgs = 0;
static int g_cur = 0;

static const int WIN_W = 512, WIN_H = 512;
static float g_scale = 1.0f;  // �????�???��??fit ??? 512??512�?
static float g_offx = 0.0f;   // 置中???�?
static float g_offy = 0.0f;
static char* DEFAULT_TITLE = "Display 512x512  (LEFT/RIGHT to switch, ESC to quit)";

static void die(const char* m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}

static void compute_fit_params(const Image* im) {
    // �????�???��??�?????????��????? 512??512�?並置�?
    float sx = (float)WIN_W / (float)im->width;
    float sy = (float)WIN_H / (float)im->height;
    g_scale = (sx < sy) ? sx : sy;
    // g_offx = 0.5f * (WIN_W - im->width);
    // g_offy = 0.5f * (WIN_H - im->height);
}

static void draw_current_image(void) {
    const Image* im = &g_imgs[g_cur];

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    for (int y = 0; y < im->height; ++y) {
        for (int x = 0; x < im->width; ++x) {
            float g = im->data[y][x];
            if (g < 0) g = 0;
            if (g > 1) g = 1;
            float x0 = x * g_scale;
            float y0 = y * g_scale;
            float x1 = x0 + g_scale;
            float y1 = y0 + g_scale;
            glColor3f(g, g, g);
            glVertex2f(x0, y0);
            glVertex2f(x1, y0);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
        }
    }
    glEnd();

    glutSwapBuffers();
}

static void display_cb(void) {
    draw_current_image();
}

static void reshape_cb(int w, int h) {
    // ??��?? 512??512
    if (w != WIN_W || h != WIN_H) {
        glutReshapeWindow(WIN_W, WIN_H);
        return;
    }
    glViewport(0, 0, WIN_W, WIN_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, (GLdouble)WIN_W, 0.0, (GLdouble)WIN_H, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void keyboard_cb(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    if (key == 27 || key == 'q' || key == 'Q') {  // ESC / q
        glutLeaveMainLoop();
    }
}

static void special_cb(int key, int x, int y) {
    (void)x;
    (void)y;
    if (g_nimgs <= 0) return;

    int changed = 0;
    if (key == GLUT_KEY_RIGHT) {
        g_cur = (g_cur + 1) % g_nimgs;
        changed = 1;
    } else if (key == GLUT_KEY_LEFT) {
        g_cur = (g_cur - 1 + g_nimgs) % g_nimgs;
        changed = 1;
    }

    if (changed) {
        compute_fit_params(&g_imgs[g_cur]);
        char* title = DEFAULT_TITLE;
        if (g_imgs[g_cur].name) {
            title = (char*)malloc((strlen(g_imgs[g_cur].name) + 38) * sizeof(char));
            sprintf(title, "%s  (LEFT/RIGHT to switch, ESC to quit)", g_imgs[g_cur].name);
        }
        glutSetWindowTitle(title);
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <img1.txt> <img2.txt> < ... >\n", argv[0]);
        return 1;
    }
    g_nimgs = argc - 1;
    g_imgs = (Image*)malloc(sizeof(Image) * g_nimgs);
    for (int i = 0; i < g_nimgs; i++) {
        g_imgs[i] = readImage(argv[i + 1]);
    }
    compute_fit_params(&g_imgs[g_cur]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - WIN_W / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - WIN_H / 1.5);

    char* title = DEFAULT_TITLE;
    if (g_imgs[g_cur].name) {
        title = (char*)malloc((strlen(g_imgs[g_cur].name) + 38) * sizeof(char));
        sprintf(title, "%s  (LEFT/RIGHT to switch, ESC to quit)", g_imgs[g_cur].name);
    }
    glutCreateWindow(title);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glClearColor(0.f, 0.f, 0.f, 1.f);

    glShadeModel(GL_FLAT);

    reshape_cb(WIN_W, WIN_H);

    glutDisplayFunc(display_cb);
    glutReshapeFunc(reshape_cb);
    glutKeyboardFunc(keyboard_cb);
    glutSpecialFunc(special_cb);

    glutMainLoop();

    for (int i = 0; i < g_nimgs; i++) {
        // free(g_imgs[i].data);
        // free(g_imgs[i].name);
        freeImage(g_imgs[i]);
    }
    free(g_imgs);
    return 0;
}
