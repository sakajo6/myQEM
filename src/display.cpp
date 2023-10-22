
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//  Display Meshes on Window                                            //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <GL/freeglut.h>

#include "models.h"
#include "display.h"

extern Model* model;

extern int HEIGHT;
extern int WIDTH;

int step;
int zoom;
double rotation_horizontal;
double rotation_vertical;

void idle(void) {
    glutPostRedisplay();
}

void resize(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(2.0, (double)w / (double)h, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

void display_string(std::string str, float x, float y) {
    glRasterPos2f(x, y);
    for (int i = 0; i < str.size(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glLoadIdentity();
    gluLookAt(5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotated(rotation_vertical, 1.0, 0.0, 0.0);
    glRotated(rotation_horizontal, 0.0, 1.0, 0.0);

    // zoom in and out
    glScalef(zoom, zoom, zoom);

    // Faces
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    // std::cout << model->snapshots[step].size() << std::endl;
    for (auto face: model->snapshots[step]) {
        HalfEdge *half_edge = face->half_edge;
        glNormal3d(
            face->normal[0],
            face->normal[1],
            face->normal[2]
        );

        for (int i = 0; i < 3; i++) {
            Vertex *vertex = half_edge->vertex;
            glVertex3d(
                vertex->v[0],
                vertex->v[1],
                vertex->v[2]
            );
            half_edge = half_edge->next;
        }
    }
    glEnd();

    // description
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor3f(0.5, 0.5, 0.5);
    float line_height = 0.04;
    float x = -1.0, y = 1.0 - line_height;
    display_string("zoom in: i", x, y);         y -= line_height;
    display_string("zoom out: k", x, y);        y -= line_height;
    display_string("rotate left: j", x, y);     y -= line_height;
    display_string("rotate right: l", x, y);    y -= line_height;
    y -= line_height;
    display_string("decrease faces: d", x, y);  y -= line_height;
    display_string("increase faces: f", x, y);  y -= line_height;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW) ;
    glPopMatrix();

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    switch (button) {

    default:
        break;
    }
}
  
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'i':
        if (zoom < 6) zoom++;
        glutPostRedisplay();
        break;
    case 'k':
        if (zoom > 1) zoom--;
        glutPostRedisplay();
        break;
    case 'l':
        rotation_horizontal += 1.0;
        if (rotation_horizontal >= 360) rotation_horizontal = 0;
        glutPostRedisplay();
        break;
    case 'j':
        rotation_horizontal -= 1.0;
        if (rotation_horizontal < 0) rotation_horizontal = 360;
        glutPostRedisplay();
        break;

    case 'd':
        // increment step
        if (step < model->snapshots.size() - 1) step++;
        glutIdleFunc(idle);
        break;
    case 'f':
        // decrement step
        if (step > 0) step--;
        glutPostRedisplay();
        break;

    default:
        break;
    }
}

void display_start(int argc, char** argv) {
    // parameter initialization
    step = 0;
    zoom = 1;
    rotation_horizontal = 0.0;
    rotation_vertical = 0.0;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(HEIGHT, WIDTH);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("QEM DEMO");

    // callback
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
}