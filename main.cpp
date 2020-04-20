#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#if defined(__APPLE__)
#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

const unsigned int windowWidth = 600, windowHeight = 600;

int majorVersion = 3, minorVersion = 3;

struct vec2
{
    float x;
    float y;
    float Ydeg;

public:
    vec2(float a = 0, float b = 0)
    {
        x = a;
        y = b;
        Ydeg = 0.0;
    }

    vec2 operator*(float a)
    {
        return vec2(x * a, y * a);
    }

    vec2 operator/(float a)
    {
        return vec2(x / a, y / a);
    }

    vec2 operator*(vec2 a)
    {
        return vec2(x * a.x, y * a.y);
    }

    void RotateZ(float a, float b, float deg)
    {
        float phi = (deg / 180.0f) * M_PI;
        float x_ = (x - a) * cosf(phi) - (y - b) * sinf(phi);
        float y_ = (x - a) * sinf(phi) + (y - b) * cosf(phi);
        x = x_ + a;
        y = y_ + b;
    }

    void RotateZ(vec2 a, float deg)
    {
        RotateZ(a.x, a.y, deg);
    }

    void RotateY(float a, float b, float deg)
    {
        float phi = (Ydeg / 180.0f) * M_PI;
        x = (x - a) / cosf(phi) + a;

        Ydeg = deg;
        phi = (deg / 180.0f) * M_PI;
        x = (x - a) * cosf(phi) + a;
    }

    void RotateY(vec2 a, float deg)
    {
        RotateY(a.x, a.y, deg);
    }

    void Translate(float a, float b)
    {
        x = x + a;
        y = y + b;
    }

    void Translate(vec2 a)
    {
        Translate(a.x, a.y);
    }

    vec2 GetTranslated(float a, float b)
    {
        x = x + a;
        y = y + b;
        return vec2(x, y);
    }

    vec2 GetTranslated(vec2 a)
    {
        GetTranslated(a.x, a.y);
    }
};

struct vec3
{
    float x;
    float y;
    float z;

public:
    vec3(float a = 0, float b = 0, float c = 0)
    {
        x = a;
        y = b;
        z = c;
    }

    vec3(vec2 a, float c = 0)
    {
        x = a.x;
        y = a.y;
        z = c;
    }
};

struct vec4
{
    float x;
    float y;
    float z;
    float w;

public:
    vec4(float a = 0, float b = 0, float c = 0, float d = 1)
    {
        x = a;
        y = b;
        z = c;
        w = d;
    }

    vec4(vec3 a, float c = 1)
    {
        x = a.x;
        y = a.y;
        z = a.z;
        w = c;
    }
};

unsigned int shaderProgram;
unsigned int texshaderProgram;

float MVPtransf[4][4] = {1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1};

struct GlEllipse
{
    unsigned int vao;
    unsigned int vbo;
    vec2 start;
    vec2 center;
    vec2 f1;
    vec2 f2;
    float len;
    float wid;
    float rot;
    std::vector<vec2> points;
    float vertexCoords[124];

    int location;

public:
    GlEllipse(float x = 0, float y = 0, float elen = 0, float ewid = 0, float deg = 0)
    {
        start.x = x;
        start.y = y;
        len = elen;
        wid = ewid;
        rot = deg;
        center.x = x;
        center.y = y + elen;
        f1 = vec2(x, y + elen - sqrtf(len * len - wid * wid));
        f2 = vec2(x, y + elen + sqrtf(len * len - wid * wid));
    }

    GlEllipse Circle(float a, float b, float r)
    {
        return GlEllipse(a, b - r, r, r);
    }

    void Create()
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        center.RotateZ(start, rot);
        vertexCoords[0] = center.x;
        vertexCoords[1] = center.y;
        points.push_back(center);
        for (int i = 0; i < 61; i++)
        {
            vec2 point(center.x + wid * cosf((i / 60.0f) * 2.0f * M_PI), center.y + len * sinf((i / 60.0f) * 2.0f * M_PI));
            point.RotateZ(center.x, center.y, rot);
            vertexCoords[2 + 2 * i] = point.x;
            vertexCoords[3 + 2 * i] = point.y;
            points.push_back(point);
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void Draw(float a, float b, float c)
    {
        location = glGetUniformLocation(shaderProgram, "color");
        glUniform3f(location, a, b, c);

        location = glGetUniformLocation(shaderProgram, "MVP");
        glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 62);
    }

    void Redraw()
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        for (size_t i = 0; i < points.size(); i++)
        {
            vertexCoords[2 * i] = points.at(i).x;
            vertexCoords[2 * i + 1] = points.at(i).y;
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void Rotate(float a, float b, float deg)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).RotateZ(a, b, deg);
        }
        start.RotateZ(a, b, deg);
        center.RotateZ(a, b, deg);
        rot = GetRot();
    }

    void Rotate(vec2 a, float deg)
    {
        Rotate(a.x, a.y, deg);
    }

    float GetRot()
    {
        vec2 dir(center.x - start.x, center.y - start.y);
        float dirrad = atan2f(dir.y, dir.x);
        dirrad = dirrad / M_PI * 180.0f;
        if (dirrad < 0.0f)
            dirrad += 360.0f;
        return dirrad - 90.0f;
    }

    void Roll(float deg)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).RotateZ(start, -rot);
            points.at(i).RotateY(start, deg);
            points.at(i).RotateZ(start, rot);
        }
    }

    void Translate(float a, float b)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).Translate(a, b);
        }
        start.Translate(a, b);
        center.Translate(a, b);
    }

    bool InEllipse(float a, float b)
    {
        float d1 = sqrtf((f1.x - a) * (f1.x - a) + (f1.y - b) * (f1.y - b));
        float d2 = sqrtf((f2.x - a) * (f2.x - a) + (f2.y - b) * (f2.y - b));
        if (d1 + d2 > 2 * len)
            return false;
        return true;
    }

    bool InEllipse(vec2 a)
    {
        return InEllipse(a.x, a.y);
    }
};

struct Bezier
{
    std::vector<vec2> cps;

public:
    Bezier(){};

    float Bz(int i, float f)
    {
        int n = cps.size() - 1;
        float choose = 1.0;
        for (int j = 1; j <= i; j++)
            choose = choose * (float)(n - j + 1) / j;
        return choose * pow(f, i) * pow(1 - f, n - i);
    }

    void AddCp(float a, float b)
    {
        cps.push_back(vec2(a, b));
    }

    void AddCp(vec2 v)
    {
        AddCp(v.x, v.y);
    }

    vec2 GetPoint(float f)
    {
        float a = 0.0;
        float b = 0.0;
        for (size_t i = 0; i < cps.size(); i++)
        {
            a += cps.at(i).x * Bz(i, f);
            b += cps.at(i).y * Bz(i, f);
        };
        return vec2(a, b);
    }
};

struct GlWing
{
    unsigned int vao;
    unsigned int vbo[2];
    vec2 start;
    vec2 p1;
    vec2 p2;
    float len;
    float wid;
    Bezier bz;
    std::vector<vec2> points;
    float vertexCoords[102];
    float texCoords[102];
    float rot;
    bool mirrored;

    unsigned int texid;
    std::vector<vec3> pixels;
    float *image;
    int location;
    int sampler;

public:
    GlWing(float a = 0, float b = 0, float c = 0, float d = 0, bool p = false)
    {
        start.x = a;
        start.y = b;
        len = c;
        wid = d;
        mirrored = p;
        rot = 0.0;
        p1 = vec2(0.0, 0.0);
        p2 = vec2(0.0, 1.0);
    }

    void Create()
    {
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);

        GlEllipse black[6];
        black[0] = GlEllipse(0.0f, -0.65f, 0.15f, 0.15f);
        black[1] = GlEllipse(0.0f, -0.15f, 0.15f, 0.15f);
        black[2] = GlEllipse(0.0f, 0.35f, 0.15f, 0.15f);
        black[3] = GlEllipse(0.5f, -0.65f, 0.15f, 0.15f);
        black[4] = GlEllipse(0.5f, -0.15f, 0.15f, 0.15f);
        black[5] = GlEllipse(0.5f, 0.35f, 0.15f, 0.15f);

        vec2 op(-1.0, 1.0);
        unsigned int cnt = 0;
        for (int i = 0; i < 128; i++)
        {
            for (int j = 0; j < 128; j++)
            {
                cnt = pixels.size();
                for (int k = 0; k < 6; k++)
                    if (black[k].InEllipse(op))
                        pixels.push_back(vec3(0.0, 0.0, 0.0));
                if (cnt == pixels.size())
                    pixels.push_back(vec3(1.0, 0.5, 0.0));
                op.Translate(0.015625, 0.0);
            };
            op.Translate(-2.0, -0.015625);
        }
        image = new float[49152];
        for (int i = 0; i < 128 * 128; i++)
        {
            image[3 * i] = pixels.at(i).x;
            image[3 * i + 1] = pixels.at(i).y;
            image[3 * i + 2] = pixels.at(i).z;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_FLOAT, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        delete[] image;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

        bz.AddCp(start);
        bz.AddCp(start.GetTranslated(0.0f, len / 2.0f));
        bz.AddCp(start.GetTranslated(0.0f, len / 2.0f));
        bz.AddCp(start.GetTranslated(wid / 2.0f, wid / 4.0f));
        bz.AddCp(start.GetTranslated(wid / 2.0f, wid / 4.0f));
        bz.AddCp(start.GetTranslated(len / 2.0f, 0.0f) * 3);
        bz.AddCp(start.GetTranslated(0.0f, wid / -2.0f) * 3);
        bz.AddCp(start.GetTranslated(len / -4.0f, len / -2.0f) * 0.5);
        bz.AddCp(start.GetTranslated(len / -4.0f, len / -2.0f) * -1.5);
        bz.AddCp(start.GetTranslated(len / 4.0f, len / -2.0f) * 0.5);
        bz.AddCp(start.GetTranslated(len / 4.0f, len / -2.0f) * 3);
        bz.AddCp(start.GetTranslated(0.0f, wid / -2.0f) * 2);
        bz.AddCp(start.GetTranslated(len / -2.0f, 0.0f));
        bz.AddCp(start.GetTranslated(wid / -2.0f, wid / 4.0f));
        bz.AddCp(start.GetTranslated(wid / -2.0f, wid / 4.0f));
        bz.AddCp(start.GetTranslated(0.0f, len / 2.0f));
        bz.AddCp(start.GetTranslated(0.0f, len / 2.0f));

        for (float i = 0.0f; i <= 1.0f; i += 0.02f)
        {
            vec2 point = bz.GetPoint(i);
            if (mirrored)
                point.x *= -1;
            points.push_back(point);
        }
        for (size_t i = 0; i < points.size(); i++)
        {
            vertexCoords[2 * i] = points.at(i).x;
            vertexCoords[2 * i + 1] = points.at(i).y;
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

        texCoords[0] = 0.0f;
        texCoords[1] = 0.5f;
        int i = 2;
        float x = 0.0f;
        float y = 1.0f;
        while (x < 1.0f)
        {
            texCoords[2 * i] = x;
            texCoords[2 * i + 1] = y;
            x += 3.0f / 50.0f;
            i++;
        }
        while (y > 0.0f)
        {
            texCoords[2 * i] = x;
            texCoords[2 * i + 1] = y;
            y -= 3.0f / 50.0f;
            i++;
        }
        while (x > 0.0f)
        {
            texCoords[2 * i] = x;
            texCoords[2 * i + 1] = y;
            x -= 3.0f / 50.0f;
            i++;
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void Draw()
    {
        glUseProgram(texshaderProgram);

        location = glGetUniformLocation(texshaderProgram, "MVP");
        glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

        sampler = 0;
        location = glGetUniformLocation(texshaderProgram, "samplerUnit");
        glUniform1i(location, sampler);
        glActiveTexture(GL_TEXTURE0 + sampler);
        glBindTexture(GL_TEXTURE_2D, texid);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 51);

        glUseProgram(shaderProgram);
    }

    void Redraw()
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

        for (size_t i = 0; i < points.size(); i++)
        {
            vertexCoords[2 * i] = points.at(i).x;
            vertexCoords[2 * i + 1] = points.at(i).y;
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    float GetRot()
    {
        vec2 dir(p2.x - p1.x, p2.y - p1.y);
        float dirrad = atan2f(dir.y, dir.x);
        dirrad = dirrad / M_PI * 180.0f;
        if (dirrad < 0.0f)
            dirrad += 360.0f;
        return dirrad - 90.0f;
    }

    void Flip(float deg)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).RotateZ(p1, -rot);
            points.at(i).RotateY(p1, deg);
            points.at(i).RotateZ(p1, rot);
        };
    }

    void Rotate(float a, float b, float deg)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).RotateZ(a, b, deg);
        }
        p1.RotateZ(a, b, deg);
        p2.RotateZ(a, b, deg);
        rot = GetRot();
    }

    void Rotate(vec2 a, float deg)
    {
        Rotate(a.x, a.y, deg);
    }

    void Translate(float a, float b)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            points.at(i).Translate(a, b);
        }
        p1.Translate(a, b);
        p2.Translate(a, b);
    }
};

void getErrorInfo(unsigned int handle)
{
    int logLen;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0)
    {
        char *log = new char[logLen];
        int written;
        glGetShaderInfoLog(handle, logLen, &written, log);
        printf("Shader log:\n%s", log);
        delete log;
    }
}

struct Flower
{
    vec2 center;
    float petals;
    float rad;

    GlEllipse cent;
    std::vector<GlEllipse> pets;

public:
    Flower(float a = 0, float b = 0, float c = 0, float d = 0)
    {
        center.x = a;
        center.y = b;
        rad = c;
        petals = d;

        cent = GlEllipse(center.x, center.y - rad, rad, rad, 0);
        for (int i = 0; i < petals; i++)
        {
            pets.push_back(GlEllipse(center.x, center.y, rad, rad / (petals * 0.4f), (i / petals) * 360.0f));
        };
    }

    void Create()
    {
        cent.Create();
        for (size_t i = 0; i < pets.size(); i++)
        {
            pets.at(i).Create();
        };
    }

    void Draw(float a, float b, float c, float d, float e, float f)
    {
        for (size_t i = 0; i < pets.size(); i++)
        {
            pets.at(i).Draw(d, e, f);
        };
        cent.Draw(a, b, c);
    }
};

struct Meadow
{
    Flower fl1;
    Flower fl2;
    Flower fl3;
    Flower fl4;

public:
    Meadow()
    {
        fl1 = Flower(-0.6f, 0.6f, 0.15f, 3.0f);
        fl2 = Flower(0.6f, 0.6f, 0.15f, 5.0f);
        fl3 = Flower(0.6f, -0.6f, 0.15f, 8.0f);
        fl4 = Flower(-0.6f, -0.6f, 0.15f, 13.0f);
    }

    void Create()
    {
        fl1.Create();
        fl2.Create();
        fl3.Create();
        fl4.Create();
    }
    void Draw()
    {
        fl1.Draw(1, 0, 0, 1, 1, 0);
        fl2.Draw(1, 1, 0, 0, 0, 1);
        fl3.Draw(0, 0, 1, 1, 1, 1);
        fl4.Draw(0, 0, 0, 1, 0.5, 0);
    }
};

struct Butterfly
{
    GlEllipse body;
    GlEllipse head;
    GlWing rightwing;
    GlWing leftwing;

    vec2 direction;
    float vel;
    float flipdeg;
    float rolldeg;
    float turn;

public:
    Butterfly()
    {
        body = GlEllipse(0.0f, -0.25f, 0.25f, 0.06f, 0.0f);
        head = GlEllipse(0.0f, 0.22f, 0.03f, 0.03f, 0.0f);
        rightwing = GlWing(0.0f, 0.0f, 0.2f, 0.1f);
        leftwing = GlWing(0.0f, 0.0f, 0.2f, 0.1f, true);
        vel = 0.0;
        flipdeg = 0.0;
        rolldeg = 0.0;
        turn = 0.0;
        direction = vec2(head.center.x - body.start.x, head.center.y - body.start.y);
    }

    void Create()
    {
        rightwing.Create();
        leftwing.Create();
        body.Create();
        head.Create();
    }

    void Draw()
    {
        rightwing.Draw();
        leftwing.Draw();
        body.Draw(0, 0, 0);
        head.Draw(1, 0, 0);
    }

    void Redraw()
    {
        rightwing.Redraw();
        leftwing.Redraw();
        body.Redraw();
        head.Redraw();
    }

    float GetRot(vec2 a)
    {
        float dirrad = atan2f(a.y, a.x);
        dirrad = dirrad / M_PI * 180.0f;
        if (dirrad < 0.0f)
            dirrad += 360.0f;
        return dirrad - 90.0f;
    }

    void Turn(float a, float b, float deg)
    {
        direction.x = head.center.x - body.start.x;
        direction.y = head.center.y - body.start.y;
        vec2 mouse(a, b);
        float mousedeg = GetRot(mouse);
        float dirdeg = GetRot(direction);
        if (mousedeg > dirdeg)
        {
            if (mousedeg - dirdeg < 180.0)
                TurnLeft(deg);
            else
                TurnRight(deg);
        }
        if (mousedeg < dirdeg)
        {
            if (dirdeg - mousedeg < 180.0)
                TurnRight(deg);
            else
                TurnLeft(deg);
        }
        if (turn >= 360.0 || turn <= -360.0)
        {
            body.Rotate(head.center, -turn);
            rolldeg = 45.0;
            head.Rotate(head.center, -turn);
            leftwing.Rotate(head.center, -turn);
            rightwing.Rotate(head.center, -turn);
            turn = 0.0;
        }
        Redraw();
    }

    void TurnRight(float deg)
    {
        turn -= deg;
        body.Rotate(head.center, -deg);
        rolldeg = 45.0;
        head.Rotate(head.center, -deg);
        leftwing.Rotate(head.center, -deg);
        rightwing.Rotate(head.center, -deg);
    }

    void TurnLeft(float deg)
    {
        turn += deg;
        body.Rotate(head.center, deg);
        rolldeg = 45.0;
        head.Rotate(head.center, deg);
        leftwing.Rotate(head.center, deg);
        rightwing.Rotate(head.center, deg);
    }

    void Translate(float a, float b)
    {
        leftwing.Translate(a, b);
        rightwing.Translate(a, b);
        body.Translate(a, b);
        head.Translate(a, b);
    }

    void Accelerate(float a, float b, float v = 1.0)
    {
        float dist = sqrtf((head.center.x - a) * (head.center.x - a) + (head.center.y - b) * (head.center.y - b));
        vel = vel + dist * 0.00001f * v;
    }

    void Move(float deg)
    {
        direction.x = head.center.x - body.start.x;
        direction.y = head.center.y - body.start.y;
        if (vel > 0)
        {
            vec2 dir = direction * vel;
            Translate(dir.x, dir.y);
            vel = vel - 0.001f * vel;
        };
        body.Roll(rolldeg);
        if (rolldeg > 0.0f)
            rolldeg -= 0.1f;
        if (rolldeg < 0.0f)
            rolldeg += 0.1f;
        Flip(deg);
        Redraw();
    }

    void Flip(float deg)
    {
        flipdeg += deg;
        if (flipdeg > 75.0)
            flipdeg = -75.0;
        leftwing.Flip(flipdeg);
        rightwing.Flip(flipdeg);
    }
};

void checkShader(unsigned int shader, const char *message)
{
    int OK;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
    if (!OK)
    {
        printf("%s!\n", message);
        getErrorInfo(shader);
    }
}

void checkLinking(unsigned int program)
{
    int OK;
    glGetProgramiv(program, GL_LINK_STATUS, &OK);
    if (!OK)
    {
        printf("Failed to link shader program!\n");
        getErrorInfo(program);
    }
}

const char *vertexSource = R"(
    #version 330
    precision highp float;
 
    uniform mat4 MVP;
    layout(location = 0) in vec2 vtxPos;
    layout(location = 1) in vec2 vtxUV;
 
    out vec2 texcoord;
 
    void main() {
        gl_Position = vec4(vtxPos, 0, 1) * MVP;
        texcoord = vtxUV;
    }
)";

const char *fragmentSource = R"(
    #version 330
    precision highp float;
    
    uniform vec3 color;
    out vec4 outColor;
 
    void main() {
        outColor = vec4(color,1);
    }
)";

const char *textureSource = R"(
    #version 330
    precision highp float;
    
    uniform sampler2D samplerUnit;
    in vec2 texcoord;
    out vec4 outColor;
 
    void main() {
        outColor = texture(samplerUnit, texcoord);
    }
)";

Meadow world;
Butterfly but;

void onInitialization()
{
    glViewport(0, 0, windowWidth, windowHeight);

    world.Create();
    but.Create();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!vertexShader)
    {
        printf("Error in vertex shader creation\n");
        exit(1);
    }
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    checkShader(vertexShader, "Vertex shader error");

    unsigned int texfragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!texfragmentShader)
    {
        printf("Error in fragment shader creation\n");
        exit(1);
    }
    glShaderSource(texfragmentShader, 1, &textureSource, NULL);
    glCompileShader(texfragmentShader);
    checkShader(texfragmentShader, "Fragment shader error");

    texshaderProgram = glCreateProgram();
    if (!texshaderProgram)
    {
        printf("Error in shader program creation\n");
        exit(1);
    }
    glAttachShader(texshaderProgram, vertexShader);
    glAttachShader(texshaderProgram, texfragmentShader);

    glBindFragDataLocation(texshaderProgram, 0, "outColor");

    glLinkProgram(texshaderProgram);
    checkLinking(texshaderProgram);
    glUseProgram(texshaderProgram);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!fragmentShader)
    {
        printf("Error in fragment shader creation\n");
        exit(1);
    }
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    checkShader(fragmentShader, "Fragment shader error");

    shaderProgram = glCreateProgram();
    if (!shaderProgram)
    {
        printf("Error in shader program creation\n");
        exit(1);
    }
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindFragDataLocation(shaderProgram, 0, "outColor");

    glLinkProgram(shaderProgram);
    checkLinking(shaderProgram);
    glUseProgram(shaderProgram);
}

void onDisplay()
{
    glClearColor(0.0f, 0.7f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    world.Draw();
    but.Draw();
    but.Move(0.5);

    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY)
{
    if (key == 'r')
        but.Translate(-but.body.center.x, -but.body.center.y);
}

void onKeyboardUp(unsigned char key, int pX, int pY)
{
}

void onMouseMotion(int pX, int pY)
{
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;
    but.Turn(cX, cY, 1.0);
    but.Accelerate(cX, cY, 1.0);
    glutPostRedisplay();
}

void onMouse(int button, int state, int pX, int pY)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        onMouseMotion(pX, pY);
    }
}

void onIdle()
{
    glutPostRedisplay();
}

void onExit()
{
    glDeleteProgram(shaderProgram);
    glDeleteProgram(texshaderProgram);
    printf("exit");
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
#if !defined(__APPLE__)
    glutInitContextVersion(majorVersion, minorVersion);
#endif
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
#if defined(__APPLE__)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_3_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutCreateWindow(argv[0]);

#if !defined(__APPLE__)
    glewExperimental = true;
    glewInit();
#endif

    printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
    printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    onInitialization();

    glutDisplayFunc(onDisplay);
    glutMouseFunc(onMouse);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutMotionFunc(onMouseMotion);

    glutMainLoop();
    onExit();
    return 1;
}
