#/** * * * BME-VIK-MI_2018_02_08 * * *|1.HF| * * * * * * * * * *\
#*    _ _____   _____        __ _                              *
#*   (_)  __ \ / ____|      / _| |                             *
#*   |_| |__)| (___    ___ | |_| |___      ____ _ _ __ ___     *
#*   | |  _  / \___ \ / _ \|  _| __\ \ /\ / / _` | '__/ _ \    *
#*   | | | \ \ ____) | (_) | | | |_ \ V  V / (_| | | |  __/    *
#*   |_|_|  \_\_____/ \___/|_|  \__| \_/\_/ \__,_|_|  \___|    *
#*                                                             *
#*                   http://irsoftware.net                     *
#*                                                             *
#*              contact_adress: sk8Geri@gmail.com               *
#*                                                               *
#*       This file is a part of the work done by aFagylaltos.     *
#*         You are free to use the code in any way you like,      *
#*         modified, unmodified or copied into your own work.     *
#*        However, I would like you to consider the following:    *
#*                                                               *
#*  -If you use this file and its contents unmodified,         *
#*              or use a major part of this file,               *
#*     please credit the author and leave this note untouched.   *
#*  -If you want to use anything in this file commercially,      *
#*                please request my approval.                    *
#*                                                              *
#\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif // Win32 platform

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define PI 3.141592653589793238462643383
enum {
    CR = 0,
    CRI = 1
};

const int detail = 15;
const float radius = 5;
const int screenWidth = 600;
const int screenHeight = 600;
float zoom = 1;
float panX = 0;
float panY = 0;

struct Color {
    float r, g, b;

    Color() {
        r = g = b = 0;
    }

    Color(float r0, float g0, float b0) {
        r = r0;
        g = g0;
        b = b0;
    }
};

struct vec2 {
    float x, y;

    vec2() {
        x = y = 0;
    }

    vec2(float x0, float y0) {
        x = x0;
        y = y0;
    }

    vec2 operator*(float a) {
        return vec2(x * a, y * a);
    }

    vec2 operator/(float a) {
        return vec2(x / a, y / a);
    }

    vec2 operator+(const vec2 &v) {
        return vec2(x + v.x, y + v.y);
    }

    vec2 operator-(const vec2 &v) {
        return vec2(x - v.x, y - v.y);
    }

    float operator*(const vec2 &v) {
        return (x * v.x + y * v.y);
    }

    float Length() { return sqrt(x * x + y * y); }
};

void drawCircle(float x, float y, float radius, Color color = Color(1, 1, 1)) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= detail; i++) {
        float tmp = PI / 180.0f * 360.0f * ((float) i / (float) detail);
        glVertex2f(x + radius * (float) cos(tmp), y + radius * (float) sin(tmp));
    }
    glEnd();
}

class Spline {
public:
    Spline(uint type, Color color) {
        this->type = type;
        this->color = color;
        numPoints = 0;
        segment = 0;
    }

    void addPoint(vec2 p) {
        if (numPoints == 0)
            points[numPoints++] = p;
        points[numPoints++] = p;
    }

    uint getCount() {
        return numPoints;
    }

    void draw() {
        if (numPoints == 0) return;

        addPoint(vec2(points[numPoints - 1].x, points[numPoints - 1].y));

        glColor3f(color.r, color.g, color.b);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < numPoints - 3; i++) {
            float dt = limit(i + 2) - limit(i + 1);
            for (float t = 0; t < dt; t += 0.01f) {
                vec2 vertex = getPoint(i, t);
                glVertex2f(vertex.x, vertex.y);
            }
        }
        glEnd();

        for (int i = 0; i < numPoints - 1; i++)
            drawCircle(points[i].x, points[i].y, radius);

        numPoints--;
    }

    void reset() {
        if (type == CR) {
            segment = 0;
            timeD = 0;
        } else if (type == CRI) {
            segment = numPoints - 3;
            timeD = limit(segment + 1) - limit(segment);
        }
    }

    float limit(uint i) {
        return (float) i + sin(((float) i) / 5.0f) * 0.4f;
    }

    void update(float dt) {
        if (type == CR) {
            timeD += dt;
            if (timeD > (limit(segment + 1) - limit(segment))) {
                segment++;
                if (segment == numPoints - 2)
                    segment = 0;

                timeD = 0;
            }
        } else if (type == CRI) {
            timeD -= dt;
            if (timeD < 0) {
                segment--;
                if (segment == -1)
                    segment = numPoints - 3;

                timeD = limit(segment + 1) - limit(segment);
            }
        }
    }

    vec2 getPoint(uint i, float T, bool differential = false) {
        if (numPoints < 3) return vec2(0, 0);

        vec2 P0 = points[i + 0];
        vec2 P1 = points[i + 1];
        vec2 P2 = points[i + 2];
        vec2 P3 = points[i + 3];

        float T0 = limit(i);
        float T1 = limit(i + 1);
        float T2 = limit(i + 2);
        float T3 = limit(i + 3);

        vec2 v0, v1;
        if (type == CR) {
            v0 = ((P2 - P1) / (T2 - T1) + (P1 - P0) / (T1 - T0)) * 0.5f;
            v1 = ((P3 - P2) / (T3 - T2) + (P2 - P1) / (T2 - T1)) * 0.5f;
        } else if (type == CRI) {
            v0 = (P2 - P0) / (T2 - T0);
            v1 = (P3 - P1) / (T3 - T1);
        }

        vec2 a, b, c, d;
        float dt;
        dt = T2 - T1;

        a = (v1 + v0) / dt / dt - (P2 - P1) * 2.0f / dt / dt / dt;
        b = (P2 - P1) * 3.0f / dt / dt - (v1 + v0 * 2.0f) / dt;
        c = v0;
        d = P1;

        float qX, qY;
        if (differential) {
            qX = (3.0f * a.x * T * T + 2.0f * b.x * T + c.x) / 3.0f;
            qY = (3.0f * a.y * T * T + 2.0f * b.y * T + c.y) / 3.0f;
        } else {
            qX = a.x * T * T * T + b.x * T * T + c.x * T + d.x;
            qY = a.y * T * T * T + b.y * T * T + c.y * T + d.y;
        }

        return vec2(qX, qY);
    }

    void drawArrow() {
        float x0 = getPoint(segment, timeD).x;
        float y0 = getPoint(segment, timeD).y;
        float x = x0 + (getPoint(segment, timeD, true).x * ((type == CR) ? +1 : -1));
        float y = y0 + (getPoint(segment, timeD, true).y * ((type == CR) ? +1 : -1));

        vec2 arrowLine = vec2((type == CR) ? x0 - x : x - x0, (type == CR) ? y0 - y : y - y0);
        arrowLine = arrowLine / arrowLine.Length() * 10.0f;

        vec2 arrowLineNormal1 = vec2(arrowLine.y, -arrowLine.x) / 2.0f;
        vec2 arrowLineNormal2 = vec2(-arrowLine.y, arrowLine.x) / 2.0f;

        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        if (type == CR) {
            glVertex2f(x0, y0);
            glVertex2f(x, y);
            glVertex2f(x + arrowLine.x + arrowLineNormal1.x, y + arrowLine.y + arrowLineNormal1.y);
            glVertex2f(x, y);
            glVertex2f(x + arrowLine.x + arrowLineNormal2.x, y + arrowLine.y + arrowLineNormal2.y);
        } else if (type == CRI) {
            glVertex2f(x0, y0);
            glVertex2f(x, y);
            glVertex2f(x - arrowLine.x - arrowLineNormal1.x, y - arrowLine.y - arrowLineNormal1.y);
            glVertex2f(x, y);
            glVertex2f(x - arrowLine.x - arrowLineNormal2.x, y - arrowLine.y - arrowLineNormal2.y);
        }
        glEnd();
    }

private:
    uint type;
    Color color;
    vec2 points[100];
    int numPoints;
    float timeD;
    int segment;
} splineCR(CR, Color(1, 0, 0)), splineCRI(CRI, Color(0, 1, 0));

void onInitialization() {
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-screenWidth / 2, screenWidth / 2, -screenHeight / 2, screenHeight / 2);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    splineCR.draw();
    splineCRI.draw();
    splineCR.drawArrow();
    splineCRI.drawArrow();

    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int x, int y) {
    if (key == 'Z') {
        zoom /= 2;
    } else if (key == 'P') {
        panX -= (float) screenWidth * 0.1f * zoom;
        panY -= (float) screenHeight * 0.2f * zoom;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-screenWidth / 2 * zoom + panX / 2.0f, screenWidth / 2 * zoom + panX / 2.0f,
               -screenHeight / 2 * zoom - panY / 2.0f, screenHeight / 2 * zoom - panY / 2.0f);
}

void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT && state == GLUT_DOWN) {
        splineCR.addPoint(vec2(x * zoom - (float) screenWidth / 2.0f * zoom + panX / 2.0f,
                               ((float) screenHeight / 2.0f * zoom - y * zoom - panY / 2.0f)));
        splineCRI.addPoint(vec2(x * zoom - (float) screenWidth / 2.0f * zoom + panX / 2.0f,
                                ((float) screenHeight / 2.0f * zoom - y * zoom - panY / 2.0f)));
        splineCR.reset();
        splineCRI.reset();
    }
}

void onIdle() {
    static float time = 0;
    const float Dt = 0.01f;
    float newTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    for (; time < newTime; time += Dt) {
        if (time + Dt > newTime) {
            splineCR.update(newTime - time);
            splineCRI.update(newTime - time);
            time = newTime;
            break;
        } else {
            splineCR.update(Dt);
            splineCRI.update(Dt);
        }
    }
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Spline2D - lesson 02");
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    onInitialization();
    glutDisplayFunc(onDisplay);
    glutMouseFunc(onMouse);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);
    glutMainLoop();
    return 0;
}
