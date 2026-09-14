/*
    NAME : RIZWAN UL KARIM
    ID   : 24-55963-1

    Project Title : View of a Small Village (Extended / Animated Version)

    -------------------------------------------------------------------
    PROJECT DESCRIPTION
    -------------------------------------------------------------------

    This project presents an extended and animated view of a small
    village environment using OpenGL. The scene is created with
    reusable drawing functions such as drawCoconutTree, drawMangoTree,
    drawBanyanTree, drawBoat, drawFish, and other helper functions.

    The use of reusable functions makes the program easier to
    understand, modify, and extend with new features. The village
    scene contains a house/hut, fence, grass, river, boat, trees,
    birds, sun, moon, stars, and clouds.

    Several animated elements have been added to make the village
    environment more lively and realistic. These include moving
    clouds, flying birds, waving trees, moving water waves, swimming
    fish, boat movement, sail movement, rainfall, and a day-night
    transition.

    -------------------------------------------------------------------
    FEATURES IMPLEMENTED
    -------------------------------------------------------------------

     [x] River with a wavy top edge
     [x] Animated water waves
     [x] Boat on the river
     [x] Keyboard-controlled boat movement
     [x] Boat sail swaying with wind
     [x] Multiple tree types: coconut, mango, and banyan
     [x] Animated tree leaves
     [x] Flying birds with flapping wings
     [x] Bird flight path animation
     [x] Moving clouds
     [x] Sun with day/sunset transition
     [x] Moon and stars for the night scene
     [x] Twinkling stars
     [x] Toggleable rainfall animation
     [x] Swimming fish under the water
     [x] Background hills and horizon
     [x] Day-night transition
     [x] Camera zoom in/out

    -------------------------------------------------------------------
    CONTROLS
    -------------------------------------------------------------------

      D / d          -> Switch to Day
      N / n          -> Switch to Night
      Up ARROW     -> Move boat Up
      Down ARROW    -> Move boat Down
      R / r          -> Toggle rain on/off
      + / =          -> Zoom in
      - / _          -> Zoom out
      Mouse left Click    -> Move boat left
      Mouse Right Click    -> Move boat right
      ESC            -> Exit the program

    -------------------------------------------------------------------
    BUILD COMMAND
    -------------------------------------------------------------------

      g++ village_scenario.cpp -o village.exe -lfreeglut -lopengl32 -lglu32 -lwinmm

*/


#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
#endif

#include <GL/glut.h>

#define PI 3.14159265358979323846f
const float BOAT_MIN_Y = -0.5f;
const float BOAT_MAX_Y =  0.5f;

// ======================================================================
//  Small helpers
// ======================================================================
static float frand(float lo, float hi) {
    return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
}

static float lerp(float a, float b, float t) { return a + (b - a) * t; }
static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void drawFilledCircle(float cx, float cy, float r, float rr, float gg, float bb, int segs = 24) {
    glColor3f(rr, gg, bb);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; i++) {
        float a = (2.0f * PI * i) / segs;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawFilledEllipse(float cx, float cy, float rx, float ry, float rr, float gg, float bb, int segs = 24) {
    glColor3f(rr, gg, bb);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; i++) {
        float a = (2.0f * PI * i) / segs;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }
    glEnd();
}
// Function Prototypes
void drawBanyanTree(float x, float y, float scale);
void drawMangoTree(float x, float y, float scale);
void drawCoconutTree(float x, float y, float scale);
void drawHut();
void drawFence();
// ======================================================================
//  Boat/MOUSE
// ======================================================================
float boatX = 0.0f;
float boatYOffset = 0.0f;
float mouseSpeedFactor = 1.0f;
float targetYDirection = 0.0f;
bool isMouseDown = false;

// ======================================================================
//  Window / camera state
// ======================================================================
int   winW = 1240, winH = 750;
float zoomLevel = 1.0f;   // smaller = zoomed in

// ======================================================================
//  Day / Night state
// ======================================================================
bool  isDay     = true;
float dayNightT = 0.0f;   // eased 0 = full day .. 1 = full night

// ======================================================================
//  Global animation clocks
// ======================================================================
float globalTime  = 0.0f;
float windPhase   = 0.0f;   // drives sail + leaf sway
float wavePhase    = 0.0f;   // drives river waves
float birdOffset   = 0.0f;   // drives bird flight path

// ======================================================================
//  Sun / Moon (share one track across the sky)
// ======================================================================
float celestialX     = -1.1f;
float celestialSpeed =  0.03f;

// ======================================================================
//  Boat
// ======================================================================

float boatBob    = 0.0f;
float sailPhase  = 0.0f;
const float BOAT_MIN_X = -0.95f;
const float BOAT_MAX_X =  0.95f;
const float BOAT_STEP  =  0.03f;




// ======================================================================
//  Scene data
// ======================================================================
struct Cloud  { float x, y, speed, scale; };
struct Bird   { float baseX, y, amp, freq, phase, flap; };
struct Star   { float x, y, phase; };
struct Fish   { float x, y, speed, dir, tailPhase; };
struct Raindrop { float x, y, speed; };
struct TreeSpot { float x, y, scale; };


std::vector<Cloud>    clouds;
std::vector<Bird>     birds;
std::vector<Star>     stars;
std::vector<Fish>     fishes;
std::vector<Raindrop> raindrops;
std::vector<TreeSpot> coconutTrees, mangoTrees, banyanTrees;

bool raining = false;

// River band (top edge is wavy)
const float RIVER_TOP    = -0.18f;
const float RIVER_BOTTOM = -0.55f;

// ======================================================================
//  Scene initialisation
// ======================================================================
void InitScene() {
    srand((unsigned int)time(0));

    // clouds
    clouds.push_back(Cloud{ -0.9f, 0.80f, 0.02f, 1.0f });
    clouds.push_back(Cloud{ -0.2f, 0.88f, 0.015f, 1.3f });
    clouds.push_back(Cloud{  0.5f, 0.82f, 0.025f, 0.8f });

    // birds
    for (int i = 0; i < 5; i++) {
        Bird b;
        b.baseX = frand(-1.4f, 1.4f);
        b.y     = frand(0.55f, 0.78f);
        b.amp   = frand(0.03f, 0.07f);
        b.freq  = frand(2.0f, 4.0f);
        b.phase = frand(0.0f, 2.0f * PI);
        b.flap  = frand(0.0f, 2.0f * PI);
        birds.push_back(b);
    }

    // stars
    for (int i = 0; i < 60; i++) {
        Star s;
        s.x = frand(-1.4f, 1.4f);
        s.y = frand(0.15f, 0.98f);
        s.phase = frand(0.0f, 2.0f * PI);
        stars.push_back(s);
    }

    // fish
    for (int i = 0; i < 6; i++) {
        Fish f;
        f.x = frand(-1.2f, 1.2f);
        f.y = frand(RIVER_BOTTOM + 0.06f, RIVER_TOP - 0.06f);
        f.speed = frand(0.15f, 0.35f);
        f.dir = (rand() % 2 == 0) ? 1.0f : -1.0f;
        f.tailPhase = frand(0.0f, 2.0f * PI);
        fishes.push_back(f);
    }

    // rain drops (pre-allocated, only drawn/updated while raining == true)
    for (int i = 0; i < 150; i++) {
        Raindrop r;
        r.x = frand(-1.4f, 1.4f);
        r.y = frand(-1.0f, 1.0f);
        r.speed = frand(1.2f, 2.2f);
        raindrops.push_back(r);
    }

    // tree placements
    banyanTrees.push_back(TreeSpot{ -1.15f, -0.14f, 1.15f });
    banyanTrees.push_back(TreeSpot{  1.10f, -0.14f, 0.95f });

    mangoTrees.push_back(TreeSpot{ -0.62f, -0.60f, 0.9f });
    mangoTrees.push_back(TreeSpot{  0.72f, -0.62f, 1.0f });

    coconutTrees.push_back(TreeSpot{ -0.30f, -0.62f, 1.0f });
    coconutTrees.push_back(TreeSpot{ -0.05f, -0.65f, 0.85f });
    coconutTrees.push_back(TreeSpot{  0.35f, -0.63f, 0.95f });
}

// ======================================================================
//  Sky, hills, sun, moon, stars
// ======================================================================
// [OBJ-01] Sky
void drawSky() {
    // colour lerp between day / night
    float topR = lerp(0.35f, 0.02f, dayNightT);
    float topG = lerp(0.65f, 0.02f, dayNightT);
    float topB = lerp(0.95f, 0.10f, dayNightT);

    float horR = lerp(0.95f, 0.06f, dayNightT);
    float horG = lerp(0.80f, 0.06f, dayNightT);
    float horB = lerp(0.55f, 0.20f, dayNightT);

    glBegin(GL_QUADS);
        glColor3f(topR, topG, topB);
        glVertex2f(-1.6f, 1.0f);
        glVertex2f( 1.6f, 1.0f);
        glColor3f(horR, horG, horB);
        glVertex2f( 1.6f, -0.05f);
        glVertex2f(-1.6f, -0.05f);
    glEnd();
}

// [OBJ-02] Background Hills
void drawHills() {
    float shade = lerp(0.35f, 0.08f, dayNightT);
    glColor3f(shade * 0.7f, shade, shade * 0.55f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-1.6f, -0.18f);
        glVertex2f(-1.6f, -0.05f);
        glVertex2f(-1.0f,  0.10f);
        glVertex2f(-0.5f, -0.02f);
        glVertex2f( 0.0f,  0.13f);
        glVertex2f( 0.6f, -0.03f);
        glVertex2f( 1.1f,  0.09f);
        glVertex2f( 1.6f, -0.05f);
        glVertex2f( 1.6f, -0.18f);
    glEnd();
}

// [OBJ-03] Sun
void drawSun() {
    float alpha = clampf(1.0f - dayNightT * 1.6f, 0.0f, 1.0f);
    if (alpha <= 0.0f) return;
    float sy = 0.55f + 0.25f * sinf((celestialX + 1.1f) / 2.2f * PI);
    // sunrise/sunset colour: yellow at noon, orange/red near horizon
    float edgeFactor = 1.0f - fabsf(sinf((celestialX + 1.1f) / 2.2f * PI));
    float r = 1.0f;
    float g = lerp(0.95f, 0.45f, edgeFactor);
    float b = lerp(0.35f, 0.05f, edgeFactor);
    glColor4f(r, g, b, alpha);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawFilledCircle(celestialX, sy, 0.09f, r, g, b);
    // soft glow rays
    glColor4f(r, g, b, alpha * 0.35f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(celestialX, sy);
    for (int i = 0; i <= 24; i++) {
        float a = (2.0f * PI * i) / 24;
        glVertex2f(celestialX + 0.14f * cosf(a), sy + 0.14f * sinf(a));
    }
    glEnd();
    glDisable(GL_BLEND);
}

// [OBJ-04] Moon
void drawMoon() {
    float alpha = clampf((dayNightT - 0.35f) * 1.6f, 0.0f, 1.0f);
    if (alpha <= 0.0f) return;
    float my = 0.55f + 0.25f * sinf((celestialX + 1.1f) / 2.2f * PI);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.92f, 0.92f, 0.85f, alpha);
    drawFilledCircle(celestialX, my, 0.075f, 0.92f, 0.92f, 0.85f);
    // crescent shadow
    glColor4f(0.08f, 0.10f, 0.20f, alpha);
    drawFilledCircle(celestialX + 0.03f, my + 0.015f, 0.065f, 0.08f, 0.10f, 0.20f);
    glDisable(GL_BLEND);
}

// [OBJ-05] Stars
void drawStars() {
    float alpha = clampf((dayNightT - 0.25f) * 1.4f, 0.0f, 1.0f);
    if (alpha <= 0.0f) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < stars.size(); i++) {
        float twinkle = 0.35f + 0.65f * fabsf(sinf(globalTime * 2.2f + stars[i].phase));
        glColor4f(1.0f, 1.0f, 0.95f, alpha * twinkle);
        glVertex2f(stars[i].x, stars[i].y);
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ======================================================================
//  Cow
// ======================================================================



// [OBJ-18] Cow with Feeder
void drawCowWithFeeder(float x, float y, float scale) {
    float shade = lerp(1.0f, 0.45f, dayNightT);

    // [AN-14] Dynamic tail motion using global windPhase
    float tailAngle = sinf(windPhase * 2.5f) * 12.0f; // Tail sways back and forth

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // ------------------------------------------------------------------
    // 1. COW BODY
    // ------------------------------------------------------------------
    // Main Body
    drawFilledEllipse(0.0f, 0.11f, 0.16f, 0.095f, 0.96f * shade, 0.96f * shade, 0.94f * shade);
    drawFilledEllipse(-0.07f, 0.12f, 0.08f, 0.07f, 0.96f * shade, 0.96f * shade, 0.94f * shade);

    // Udder
    drawFilledEllipse(-0.05f, 0.02f, 0.035f, 0.025f, 0.98f * shade, 0.78f * shade, 0.78f * shade);

    // Realistic Black Spots
    drawFilledEllipse(-0.06f, 0.13f, 0.045f, 0.038f, 0.12f * shade, 0.12f * shade, 0.12f * shade);
    drawFilledEllipse( 0.04f, 0.09f, 0.038f, 0.032f, 0.12f * shade, 0.12f * shade, 0.12f * shade);
    drawFilledEllipse(-0.01f, 0.05f, 0.025f, 0.020f, 0.12f * shade, 0.12f * shade, 0.12f * shade);

    // Slender Legs with Joint Shading
    glColor3f(0.88f * shade, 0.88f * shade, 0.86f * shade);
    glLineWidth(4.2f);
    glBegin(GL_LINES);
        glVertex2f(-0.10f, 0.04f); glVertex2f(-0.11f, -0.09f);
        glVertex2f(-0.05f, 0.04f); glVertex2f(-0.05f, -0.09f);
        glVertex2f( 0.06f, 0.04f); glVertex2f( 0.05f, -0.09f);
        glVertex2f( 0.10f, 0.04f); glVertex2f( 0.10f, -0.09f);
    glEnd();

    // Dark Hooves
    glColor3f(0.18f * shade, 0.18f * shade, 0.18f * shade);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.11f, -0.09f); glVertex2f(-0.11f, -0.11f);
        glVertex2f(-0.05f, -0.09f); glVertex2f(-0.05f, -0.11f);
        glVertex2f( 0.05f, -0.09f); glVertex2f( 0.05f, -0.11f);
        glVertex2f( 0.10f, -0.09f); glVertex2f( 0.10f, -0.11f);
    glEnd();

    // ------------------------------------------------------------------
    // 2. TAIL WITH MOTION
    // ------------------------------------------------------------------
    glPushMatrix();
        glTranslatef(-0.15f, 0.14f, 0.0f);
        glRotatef(tailAngle, 0.0f, 0.0f, 1.0f); // Rotation for tail sway

        glColor3f(0.85f * shade, 0.85f * shade, 0.83f * shade);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(-0.04f, -0.13f);
        glEnd();

        // Tail Switch
        drawFilledEllipse(-0.045f, -0.14f, 0.012f, 0.022f, 0.12f * shade, 0.12f * shade, 0.12f * shade);
    glPopMatrix();

    // ------------------------------------------------------------------
    // 3. ENHANCED HEAD & FACIAL FEATURES
    // ------------------------------------------------------------------
    // Neck
    drawFilledEllipse(0.13f, 0.11f, 0.048f, 0.058f, 0.94f * shade, 0.94f * shade, 0.92f * shade);

    // Head
    drawFilledEllipse(0.18f, 0.09f, 0.040f, 0.035f, 0.96f * shade, 0.96f * shade, 0.94f * shade);

    // Pink Muzzle
    drawFilledEllipse(0.21f, 0.055f, 0.024f, 0.019f, 0.98f * shade, 0.72f * shade, 0.72f * shade);

    // Curved Horns
    glColor3f(0.22f * shade, 0.22f * shade, 0.22f * shade);
    glLineWidth(2.8f);
    glBegin(GL_LINES);
        glVertex2f(0.17f, 0.12f); glVertex2f(0.19f, 0.17f);
        glVertex2f(0.15f, 0.12f); glVertex2f(0.16f, 0.16f);
    glEnd();

    // Ears
    drawFilledEllipse(0.13f, 0.11f, 0.022f, 0.010f, 0.90f * shade, 0.85f * shade, 0.83f * shade);

    // Eye Detail
    drawFilledEllipse(0.18f, 0.10f, 0.006f, 0.006f, 0.05f, 0.05f, 0.05f);

    // ------------------------------------------------------------------
    // 4. DETAILED STRAW FEEDER
    // ------------------------------------------------------------------
    // Legs
    glColor3f(0.35f * shade, 0.20f * shade, 0.08f * shade);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2f(0.21f, -0.01f); glVertex2f(0.21f, -0.10f);
        glVertex2f(0.33f, -0.01f); glVertex2f(0.33f, -0.10f);
    glEnd();

    // Wooden Container Outer Box
    glColor3f(0.48f * shade, 0.28f * shade, 0.10f * shade);
    glBegin(GL_QUADS);
        glVertex2f(0.19f, -0.03f);
        glVertex2f(0.35f, -0.03f);
        glVertex2f(0.37f,  0.045f);
        glVertex2f(0.17f,  0.045f);
    glEnd();

    // Straw / Kher Layer
    drawFilledEllipse(0.27f, 0.055f, 0.080f, 0.028f, 0.98f * shade, 0.86f * shade, 0.20f * shade);

    glPopMatrix();
}

// ======================================================================
//  Clouds
// ======================================================================



// [OBJ-06] Clouds
void drawOneCloud(float x, float y, float scale) {
    glColor3ub(255, 255, 255);
    drawFilledCircle(x,            y,           0.05f * scale, 1.0f, 1.0f, 1.0f);
    drawFilledCircle(x + 0.05f,    y + 0.025f,  0.05f * scale, 1.0f, 1.0f, 1.0f);
    drawFilledCircle(x - 0.05f,    y + 0.02f,   0.045f * scale, 1.0f, 1.0f, 1.0f);
    drawFilledCircle(x + 0.02f,    y + 0.045f,  0.045f * scale, 1.0f, 1.0f, 1.0f);
    drawFilledCircle(x - 0.02f,    y - 0.015f,  0.04f * scale, 0.92f, 0.92f, 0.95f);
}

// [OBJ-06] Clouds
void drawClouds() {
    for (size_t i = 0; i < clouds.size(); i++) {
        drawOneCloud(clouds[i].x, clouds[i].y, clouds[i].scale);
    }
}



// ======================================================================
//  Birds (flight path + wing flap)
// ======================================================================
// [OBJ-07] Birds
void drawOneBird(float x, float y, float flap) {
    float wing = 0.02f + 0.015f * fabsf(sinf(flap));
    glColor3ub(30, 30, 30);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(x - 0.025f, y);
        glVertex2f(x - 0.008f, y + wing);
        glVertex2f(x,          y);
        glVertex2f(x + 0.008f, y + wing);
        glVertex2f(x + 0.025f, y);
    glEnd();
}

// [OBJ-07] Birds  [AN-09] uses birdOffset for flight path movement
void drawBirds() {
    for (size_t i = 0; i < birds.size(); i++) {
        Bird &b = birds[i];
        float x = b.baseX + birdOffset;
        // wrap into visible range
        while (x > 1.5f)  x -= 3.0f;
        while (x < -1.5f) x += 3.0f;
        float y = b.y + b.amp * sinf(b.freq * x + b.phase);
        drawOneBird(x, y, b.flap);
    }
}

// ======================================================================
//  River (polygon model with wave animation) + fish
// ======================================================================
// [OBJ-08] River  [AN-08] uses wavePhase for wave animation
void drawRiver() {
    float shade = lerp(1.0f, 0.35f, dayNightT);
    const int segments = 48;
    float xStart = -1.6f, xEnd = 1.6f;
    float dx = (xEnd - xStart) / segments;

    // Fixed Top Gap: Added height offset (+0.04f) to overlap with ground layer seamlessly
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float x = xStart + i * dx;
        float waveY = (RIVER_TOP + 0.04f) + 0.015f * sinf(x * 6.0f + wavePhase);

        // Top river gradient (Day-Night shaded)
        glColor3f(0.15f * shade, 0.45f * shade, 0.80f * shade);
        glVertex2f(x, waveY);

        // Bottom river gradient (Day-Night shaded)
        glColor3f(0.02f * shade, 0.16f * shade, 0.45f * shade);
        glVertex2f(x, RIVER_BOTTOM);
    }
    glEnd();

    // Shimmering wave-crest lines
    glLineWidth(1.5f);
    for (int row = 0; row < 3; row++) {
        float baseY = RIVER_TOP - 0.07f - row * 0.11f;
        glColor3f((0.55f + 0.1f * row) * shade, 0.80f * shade, 1.0f * shade);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= segments; i++) {
            float x = xStart + i * dx;
            float y = baseY + 0.015f * sinf(x * 8.0f + wavePhase * 1.5f + row * 1.3f);
            glVertex2f(x, y);
        }
        glEnd();
    }
}
//FISH

// [OBJ-09] Fish
void drawOneFish(float x, float y, float dir, float tailPhase, float r, float g, float b) {
    float shade = lerp(1.0f, 0.45f, dayNightT);
    float wag = 0.012f * sinf(tailPhase);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(dir, 1.0f, 1.0f); // Dir handles facing direction (1.0f or -1.0f)

    // 1. Animated Tail
    glColor3f(r * 0.85f * shade, g * 0.85f * shade, b * 0.85f * shade);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.025f, 0.0f);
        glVertex2f(-0.052f,  0.018f + wag);
        glVertex2f(-0.052f, -0.018f + wag);
    glEnd();

    // 2. Main Body
    drawFilledEllipse(0.0f, 0.0f, 0.032f, 0.016f, r * shade, g * shade, b * shade);

    // 3. Top/Dorsal Fin
    glColor3f(r * 0.9f * shade, g * 0.9f * shade, b * 0.9f * shade);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.01f, 0.014f);
        glVertex2f( 0.01f, 0.014f);
        glVertex2f( 0.00f, 0.026f);
    glEnd();

    // 4. Pectoral Fin
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(0.00f, -0.003f);
        glVertex2f(-0.012f, -0.012f);
    glEnd();

    // 5. Eye
    drawFilledEllipse(0.018f, 0.004f, 0.004f, 0.004f, 1.0f, 1.0f, 1.0f); // White pupil
    drawFilledEllipse(0.019f, 0.004f, 0.002f, 0.002f, 0.0f, 0.0f, 0.0f); // Black center

    glPopMatrix();
}

// [OBJ-09] Fish
void drawFish() {
    // Limits rendering to exactly 3 fishes directly from code
    size_t count = fishes.size() < 3 ? fishes.size() : 3;

    for (size_t i = 0; i < count; i++) {
        Fish &f = fishes[i];
        if (i == 0)
            drawOneFish(f.x, f.y, f.dir, f.tailPhase, 0.98f, 0.50f, 0.10f); // Orange Fish
        else if (i == 1)
            drawOneFish(f.x, f.y, f.dir, f.tailPhase, 0.95f, 0.25f, 0.25f); // Red Fish
        else
            drawOneFish(f.x, f.y, f.dir, f.tailPhase, 0.90f, 0.75f, 0.15f); // Yellow Fish
    }
}
// ======================================================================
//  Boat (translation + swaying sail)
// ======================================================================

// [OBJ-10] Boat (hull, boatman, oar, canopy, mast & sail)  [AN-07] uses windPhase for sail/oar sway
void drawBoat() {
    float shade = lerp(1.0f, 0.45f, dayNightT);
    float oarAngle = 15.0f * sinf(windPhase * 2.0f); // Swaying motion for rowing

    glPushMatrix();
    // Positioned in the middle of the river
    glTranslatef(boatX, (RIVER_TOP + RIVER_BOTTOM) * 0.5f + boatBob + boatYOffset, 0.0f);
    // ------------------------------------------------------------------
    // 1. BOAT HULL
    // ------------------------------------------------------------------
    // Dark Wooden Base
    glColor3ub(75 * shade, 45 * shade, 18 * shade);
    glBegin(GL_POLYGON);
        glVertex2f(-0.16f,  0.0f);
        glVertex2f( 0.16f,  0.0f);
        glVertex2f( 0.11f, -0.045f);
        glVertex2f(-0.11f, -0.045f);
    glEnd();

    // Hull Top Rim Highlight
    glColor3ub(110 * shade, 70 * shade, 30 * shade);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        glVertex2f(-0.16f, 0.0f);
        glVertex2f( 0.16f, 0.0f);
    glEnd();

    // ------------------------------------------------------------------
    // 2.BOATMAN
    // ------------------------------------------------------------------
    // Body / Shirt
    glColor3ub(40 * shade, 90 * shade, 160 * shade);
    drawFilledEllipse(-0.10f, 0.035f, 0.022f, 0.028f, 40*shade/255.0f, 90*shade/255.0f, 160*shade/255.0f);

    // Head
    glColor3ub(210 * shade, 150 * shade, 110 * shade);
    drawFilledEllipse(-0.10f, 0.075f, 0.014f, 0.016f, 210*shade/255.0f, 150*shade/255.0f, 110*shade/255.0f);

    // Mathal / Traditional Bamboo Hat
    glColor3ub(210 * shade, 170 * shade, 70 * shade);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.14f, 0.08f);
        glVertex2f(-0.06f, 0.08f);
        glVertex2f(-0.10f, 0.115f);
    glEnd();

    // ------------------------------------------------------------------
    // 3. ROWING OAR
    // ------------------------------------------------------------------
    glPushMatrix();
        glTranslatef(-0.08f, 0.01f, 0.0f);
        glRotatef(oarAngle, 0.0f, 0.0f, 1.0f);

        // Oar Handle
        glColor3ub(100 * shade, 65 * shade, 30 * shade);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
            glVertex2f(0.02f, 0.04f);
            glVertex2f(-0.07f, -0.07f);
        glEnd();

        // Oar Blade
        glColor3ub(130 * shade, 85 * shade, 40 * shade);
        glBegin(GL_POLYGON);
            glVertex2f(-0.07f, -0.07f);
            glVertex2f(-0.09f, -0.085f);
            glVertex2f(-0.075f, -0.095f);
            glVertex2f(-0.055f, -0.08f);
        glEnd();
    glPopMatrix();

    // ------------------------------------------------------------------
    // 4. BAMBOO CANOPY
    // ------------------------------------------------------------------
    // Canopy Roof Cover
    glColor3ub(160 * shade, 120 * shade, 50 * shade);
    drawFilledEllipse(0.02f, 0.035f, 0.075f, 0.045f, 160*shade/255.0f, 120*shade/255.0f, 50*shade/255.0f);

    // Inner Shadow of Canopy
    glColor3ub(80 * shade, 55 * shade, 25 * shade);
    drawFilledEllipse(0.02f, 0.02f, 0.065f, 0.025f, 80*shade/255.0f, 55*shade/255.0f, 25*shade/255.0f);

    // ------------------------------------------------------------------
    // 5. MAST & SAIL
    // ------------------------------------------------------------------
    // Mast Pole
    glColor3ub(60 * shade, 40 * shade, 20 * shade);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        glVertex2f(0.03f, 0.0f);
        glVertex2f(0.03f, 0.17f);
    glEnd();

    // Swaying Sail
    float swayAngle = 8.0f * sinf(windPhase);
    glPushMatrix();
        glTranslatef(0.03f, 0.17f, 0.0f);
        glRotatef(swayAngle, 0.0f, 0.0f, 1.0f);

        // Sail Cloth
        glColor3ub(240 * shade, 235 * shade, 215 * shade);
        glBegin(GL_TRIANGLES);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(0.09f, -0.13f);
            glVertex2f(0.0f, -0.14f);
        glEnd();

        // Red Stripe on Sail
        glColor3ub(190 * shade, 40 * shade, 40 * shade);
        glBegin(GL_TRIANGLES);
            glVertex2f(0.0f, -0.05f);
            glVertex2f(0.055f, -0.09f);
            glVertex2f(0.0f, -0.10f);
        glEnd();
    glPopMatrix();

    glPopMatrix();
}

// ======================================================================
//  Ground / hut / fence (kept simple, this is the front river bank)
// ======================================================================
// [OBJ-11] Ground / River Bank
void drawGround() {
    float shade = lerp(1.0f, 0.35f, dayNightT);

    // Main ground below the river
    glColor3f(0.25f * shade, 0.55f * shade, 0.20f * shade);
    glBegin(GL_QUADS);
        glVertex2f(-1.6f, -1.0f);
        glVertex2f( 1.6f, -1.0f);
        glVertex2f( 1.6f, RIVER_BOTTOM);
        glVertex2f(-1.6f, RIVER_BOTTOM);
    glEnd();

    // Thin bank strip behind the river (Extended down to fix black gap)
    glColor3f(0.22f * shade, 0.5f * shade, 0.18f * shade);
    glBegin(GL_QUADS);
        glVertex2f(-1.6f, -0.08f);
        glVertex2f( 1.6f, -0.08f);
        glVertex2f( 1.6f, RIVER_TOP + 0.05f);
        glVertex2f(-1.6f, RIVER_TOP + 0.05f);
    glEnd();
}

// [OBJ-12] Hut (walls, roof, door, windows)
void drawSingleHut(float xOffset) {
    float shade = lerp(1.0f, 0.45f, dayNightT);

    // Walls (Increased width)
    glColor3f(0.85f * shade, 0.75f * shade, 0.55f * shade);
    glBegin(GL_QUADS);
        glVertex2f(xOffset - 0.24f, -0.60f);
        glVertex2f(xOffset + 0.24f, -0.60f);
        glVertex2f(xOffset + 0.24f, -0.92f);
        glVertex2f(xOffset - 0.24f, -0.92f);
    glEnd();

    // Roof (Adjusted to match wall width)
    glColor3f(0.55f * shade, 0.25f * shade, 0.1f * shade);
    glBegin(GL_TRIANGLES);
        glVertex2f(xOffset - 0.28f, -0.60f);
        glVertex2f(xOffset + 0.28f, -0.60f);
        glVertex2f(xOffset,        -0.35f);
    glEnd();

    // Door
    glColor3f(0.3f * shade, 0.18f * shade, 0.05f * shade);
    glBegin(GL_QUADS);
        glVertex2f(xOffset - 0.05f, -0.92f);
        glVertex2f(xOffset + 0.05f, -0.92f);
        glVertex2f(xOffset + 0.05f, -0.73f);
        glVertex2f(xOffset - 0.05f, -0.73f);
    glEnd();

    // Left Window (Original Dark Brown Color)
    glColor3f(0.3f * shade, 0.18f * shade, 0.05f * shade);
    glBegin(GL_QUADS);
        glVertex2f(xOffset - 0.17f, -0.75f);
        glVertex2f(xOffset - 0.09f, -0.75f);
        glVertex2f(xOffset - 0.09f, -0.67f);
        glVertex2f(xOffset - 0.17f, -0.67f);
    glEnd();

    // Right Window (Original Dark Brown Color)
    glColor3f(0.3f * shade, 0.18f * shade, 0.05f * shade);
    glBegin(GL_QUADS);
        glVertex2f(xOffset + 0.09f, -0.75f);
        glVertex2f(xOffset + 0.17f, -0.75f);
        glVertex2f(xOffset + 0.17f, -0.67f);
        glVertex2f(xOffset + 0.09f, -0.67f);
    glEnd();
}

// [OBJ-12] Hut
void drawHut() {
    // Drawing two wider huts shifted further to the right
    drawSingleHut(0.18f); // First hut (Middle-Right)
    drawSingleHut(0.74f); // Second hut (Far-Right)
}

// [OBJ-13] Fence
void drawFence() {
    float shade = lerp(1.0f, 0.45f, dayNightT);
    glColor3f(0.5f * shade, 0.35f * shade, 0.15f * shade);
    glLineWidth(3.0f);

    // Vertical posts for the fence on the left side of the first hut
    for (int i = 0; i < 5; i++) {
        float x = -0.22f + i * 0.04f;
        glBegin(GL_LINES);
            glVertex2f(x, -0.92f);
            glVertex2f(x, -0.78f);
        glEnd();
    }

    // Horizontal rails for the fence
    glBegin(GL_LINES);
        glVertex2f(-0.22f, -0.82f);
        glVertex2f(-0.06f, -0.82f);

        glVertex2f(-0.22f, -0.88f);
        glVertex2f(-0.06f, -0.88f);
    glEnd();
}

// ======================================================================
//  Trees (coconut / mango / banyan) with wind-swayed leaves
// ======================================================================

// Banyan Tree Implementation
// Enhanced Banyan Tree Implementation
// [OBJ-14] Banyan Tree  [AN-07] uses windPhase for leaf sway
void drawBanyanTree(float x, float y, float scale) {
    float sway = 3.0f * sinf(windPhase * 0.7f + x * 1.5f);

    // Main Trunk
    glColor3ub(80, 50, 25);
    glBegin(GL_QUADS);
        glVertex2f(x - 0.055f * scale, y);
        glVertex2f(x + 0.055f * scale, y);
        glVertex2f(x + 0.040f * scale, y + 0.18f * scale);
        glVertex2f(x - 0.040f * scale, y + 0.18f * scale);
    glEnd();

    // Multiple Prop Roots (ঝুড়ি মূল)
    glColor3ub(100, 65, 35);
    glLineWidth(2.5f * scale);
    glBegin(GL_LINES);
        // Left side roots
        glVertex2f(x - 0.12f * scale, y + 0.14f * scale);
        glVertex2f(x - 0.12f * scale, y);
        glVertex2f(x - 0.07f * scale, y + 0.12f * scale);
        glVertex2f(x - 0.07f * scale, y);

        // Right side roots
        glVertex2f(x + 0.08f * scale, y + 0.13f * scale);
        glVertex2f(x + 0.08f * scale, y);
        glVertex2f(x + 0.14f * scale, y + 0.15f * scale);
        glVertex2f(x + 0.14f * scale, y);
    glEnd();

    // Multi-layered Broad Leaves Canopy (ঘন ও বড় ছাতা)
    float topY = y + 0.18f * scale;
    glPushMatrix();
        glTranslatef(x, topY, 0.0f);
        glRotatef(sway, 0.0f, 0.0f, 1.0f);

        // Dark Base Layer
        drawFilledEllipse(0.0f,  0.07f * scale, 0.24f * scale, 0.14f * scale, 0.08f, 0.38f, 0.10f);
        drawFilledEllipse(-0.16f * scale, 0.01f * scale, 0.15f * scale, 0.11f * scale, 0.09f, 0.40f, 0.11f);
        drawFilledEllipse( 0.17f * scale, 0.02f * scale, 0.15f * scale, 0.11f * scale, 0.07f, 0.35f, 0.09f);

        // Lighter Highlight Top Layer
        drawFilledEllipse(0.0f,  0.10f * scale, 0.20f * scale, 0.11f * scale, 0.12f, 0.50f, 0.15f);
        drawFilledEllipse(-0.12f * scale, 0.05f * scale, 0.12f * scale, 0.08f * scale, 0.14f, 0.55f, 0.16f);
        drawFilledEllipse( 0.13f * scale, 0.06f * scale, 0.12f * scale, 0.08f * scale, 0.11f, 0.48f, 0.13f);
    glPopMatrix();
}

// Beautiful Mango Tree Implementation with Ripe Mangoes
// [OBJ-15] Mango Tree  [AN-07] uses windPhase for leaf sway
void drawMangoTree(float x, float y, float scale) {
    float sway = 4.0f * sinf(windPhase * 0.9f + x * 2.0f);

    // Trunk
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
        glVertex2f(x - 0.025f * scale, y);
        glVertex2f(x + 0.025f * scale, y);
        glVertex2f(x + 0.018f * scale, y + 0.18f * scale);
        glVertex2f(x - 0.018f * scale, y + 0.18f * scale);
    glEnd();

    // Layered Canopy
    float topY = y + 0.18f * scale;
    glPushMatrix();
        glTranslatef(x, topY, 0.0f);
        glRotatef(sway, 0.0f, 0.0f, 1.0f);

        drawFilledEllipse(0.0f, 0.05f * scale, 0.15f * scale, 0.11f * scale, 0.10f, 0.45f, 0.12f);
        drawFilledEllipse(-0.08f * scale, 0.0f, 0.10f * scale, 0.08f * scale, 0.15f, 0.55f, 0.15f);
        drawFilledEllipse( 0.09f * scale, 0.01f * scale, 0.10f * scale, 0.08f * scale, 0.12f, 0.50f, 0.12f);
        drawFilledEllipse( 0.0f, 0.09f * scale, 0.11f * scale, 0.08f * scale, 0.20f, 0.65f, 0.20f);

        // Yellow Ripe Mangoes
        drawFilledEllipse(-0.06f * scale, 0.03f * scale, 0.015f * scale, 0.022f * scale, 1.0f, 0.7f, 0.0f);
        drawFilledEllipse( 0.05f * scale, 0.04f * scale, 0.015f * scale, 0.022f * scale, 1.0f, 0.7f, 0.0f);
        drawFilledEllipse( 0.01f * scale, -0.01f * scale, 0.014f * scale, 0.020f * scale, 1.0f, 0.7f, 0.0f);
        drawFilledEllipse(-0.03f * scale, 0.08f * scale, 0.014f * scale, 0.020f * scale, 1.0f, 0.7f, 0.0f);
        drawFilledEllipse( 0.07f * scale, -0.02f * scale, 0.015f * scale, 0.022f * scale, 1.0f, 0.7f, 0.0f);
    glPopMatrix();
}

// Coconut Tree Implementation
// [OBJ-16] Coconut Tree  [AN-07] uses windPhase for leaf sway
void drawCoconutTree(float x, float y, float scale) {
    float sway = 6.0f * sinf(windPhase + x * 3.0f);

    // Trunk
    glColor3ub(110, 75, 35);
    glLineWidth(7.0f * scale);
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 12; i++) {
            float t = i / 12.0f;
            float px = x + 0.04f * scale * sinf(t * 2.5f);
            float py = y + t * 0.35f * scale;
            glVertex2f(px, py);
        }
    glEnd();

    float topY = y + 0.35f * scale;
    float topX = x + 0.04f * scale * sinf(2.5f);

    glPushMatrix();
        glTranslatef(topX, topY, 0.0f);
        glRotatef(sway, 0.0f, 0.0f, 1.0f);

        // Coconuts
        glColor3ub(100, 140, 30);
        glBegin(GL_TRIANGLE_FAN);
            for(int i = 0; i <= 20; i++) {
                float angle = i * 2.0f * 3.14159f / 20;
                glVertex2f(0.015f * scale * cosf(angle), 0.015f * scale * sinf(angle) - 0.01f * scale);
            }
        glEnd();

        // Leaves
        for (int i = 0; i < 8; i++) {
            float a = (2.0f * 3.14159f * i) / 8.0f;

            glColor3ub(20, 140, 45);
            glBegin(GL_TRIANGLES);
                glVertex2f(0.0f, 0.0f);
                glVertex2f(0.14f * scale * cosf(a - 0.15f), 0.14f * scale * sinf(a - 0.15f) - 0.02f * scale);
                glVertex2f(0.14f * scale * cosf(a + 0.15f), 0.14f * scale * sinf(a + 0.15f) - 0.02f * scale);
            glEnd();

            glColor3ub(40, 175, 60);
            glBegin(GL_TRIANGLES);
                glVertex2f(0.0f, 0.0f);
                glVertex2f(0.10f * scale * cosf(a - 0.08f), 0.10f * scale * sinf(a - 0.08f));
                glVertex2f(0.10f * scale * cosf(a + 0.08f), 0.10f * scale * sinf(a + 0.08f));
            glEnd();
        }
    glPopMatrix();
}
// ======================================================================
//  Rain
// ======================================================================
// [OBJ-17] Rain  [AN-12] raindrops loop drives falling animation (see Update())
void drawRain() {
    if (!raining) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.65f, 0.8f, 1.0f, 0.6f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for (size_t i = 0; i < raindrops.size(); i++) {
        Raindrop &r = raindrops[i];
        glVertex2f(r.x, r.y);
        glVertex2f(r.x - 0.006f, r.y - 0.04f);
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ======================================================================
//  On-screen instructions
// ======================================================================
// [OBJ-19] On-screen HUD Text
void drawText(float x, float y, void *font, const char *text) {
    glRasterPos2f(x, y);
    for (const char *c = text; *c; c++) glutBitmapCharacter(font, *c);
}

// [OBJ-19] On-screen HUD Text
void drawHUD() {
    float shade = lerp(0.0f, 1.0f, dayNightT);
    glColor3f(shade, shade, shade);
    drawText(-1.55f, 0.95f, GLUT_BITMAP_HELVETICA_12, "D/N: Day-Night   Arrows: Move boat   R: Rain   +/- or scroll: Zoom");
}

// ======================================================================
//  Timer-driven update loop
// ======================================================================
// [AN-01] Update() - Global animation clock driving all time-based motion
void Update(int) {
    const float dt = 0.016f;
    globalTime += dt;
    windPhase  += dt * 1.5f;   // [AN-07] drives sail / leaf / cow-tail sway
    wavePhase   += dt * 2.0f;  // [AN-08] drives river wave animation
    sailPhase  += dt * 2.0f;

    // [AN-10] celestialX movement - Sun and Moon travel across the sky
    celestialX += celestialSpeed * dt * 10.0f;
    if (celestialX > 1.3f) celestialX = -1.3f;

    boatBob = 0.01f * sinf(globalTime * 2.0f);

    // [AN-11] day/night easing - colour transition of sky, ground, river, huts, trees, boat, cow
    float target = isDay ? 0.0f : 1.0f;
    dayNightT += (target - dayNightT) * 0.02f;

    // clouds drift
    for (size_t i = 0; i < clouds.size(); i++) {
        clouds[i].x += clouds[i].speed;
        if (clouds[i].x > 1.7f) clouds[i].x = -1.7f;
    }

    // [AN-09] birdOffset - bird flight path movement
    birdOffset += dt * 0.35f;
    // [AN-15] flap increment - bird wing flapping motion
    for (size_t i = 0; i < birds.size(); i++) birds[i].flap += dt * 10.0f;

    // [AN-13] fish position loop - fish swimming back and forth
    for (size_t i = 0; i < fishes.size(); i++) {
        Fish &f = fishes[i];
        f.x += f.dir * f.speed * dt;
        f.tailPhase += dt * 6.0f;
        if (f.x > 1.3f || f.x < -1.3f) f.dir *= -1.0f;
    }

    // [AN-12] raindrops loop - falling rain animation
    if (raining) {
        for (size_t i = 0; i < raindrops.size(); i++) {
            Raindrop &r = raindrops[i];
            r.y -= r.speed * dt * 4.0f;
            if (r.y < -1.0f) {
                r.y = 1.0f;
                r.x = frand(-1.4f, 1.4f);
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Update, 0);
}

   void update(int value) {

    if (boatX > 0.95f) boatX = 0.95f;
    if (boatX < -0.95f) boatX = -0.95f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


// ======================================================================
//  Projection / camera zoom
// ======================================================================
// [AN-06] ApplyProjection() - Camera / viewport projection setup
void ApplyProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)winW / (float)winH;
    float halfH = 1.0f * zoomLevel;
    float halfW = halfH * aspect;
    glOrtho(-halfW, halfW, -halfH, halfH, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// [AN-06] reshape() - Camera / viewport update on window resize
void reshape(int w, int h) {
    winW = w;
    winH = (h == 0) ? 1 : h;
    glViewport(0, 0, w, h);
    ApplyProjection();
}

// ======================================================================
//  Display
// ======================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawSky();
    drawHills();
    drawSun();
    drawMoon();
    drawStars();
   drawClouds();

    drawBirds();

    drawRiver();
    drawFish();
    drawBoat();

    drawGround();

    // Loop through vector data to draw all trees on the left side
    for (size_t i = 0; i < banyanTrees.size(); i++)
        drawBanyanTree(banyanTrees[i].x, banyanTrees[i].y, banyanTrees[i].scale);

    for (size_t i = 0; i < mangoTrees.size(); i++)
        drawMangoTree(mangoTrees[i].x, mangoTrees[i].y, mangoTrees[i].scale);

    for (size_t i = 0; i < coconutTrees.size(); i++)
        drawCoconutTree(coconutTrees[i].x, coconutTrees[i].y, coconutTrees[i].scale);





    drawRain();
    drawHUD();
  // Sized up (1.20f) and positioned cleanly so huts stay fully visible
   drawCowWithFeeder(-0.42f, -0.80f, 0.95f);
    drawFence();
    drawHut();

    glutSwapBuffers();
}

// ======================================================================
//  Input
// ======================================================================
// [AN-02] keyboard() - Day/Night switch, Rain toggle, Keyboard zoom
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'd': case 'D':
            isDay = true;
#ifdef _WIN32
            PlaySound(TEXT("bird-2.wav"), NULL, SND_ASYNC);
#endif
            break;
        case 'n': case 'N':
            isDay = false;
#ifdef _WIN32
            PlaySound(TEXT("cricket-2.wav"), NULL, SND_ASYNC);
#endif
            break;
        case 'r': case 'R':
            raining = !raining;
            break;
        case '+': case '=':
            zoomLevel = clampf(zoomLevel - 0.1f, 0.5f, 2.5f);
            ApplyProjection();
            break;
        case '-': case '_':
            zoomLevel = clampf(zoomLevel + 0.1f, 0.5f, 2.5f);
            ApplyProjection();
            break;
        case 27: // Esc
            exit(0);
            break;
    }
    glutPostRedisplay();
}

// [AN-03] specialKeys() - Boat vertical movement (Up / Down arrow keys)
void specialKeys(int key, int, int) {
    if (key == GLUT_KEY_UP) {
        boatYOffset = clampf(boatYOffset + BOAT_STEP, BOAT_MIN_Y, BOAT_MAX_Y);
    } else if (key == GLUT_KEY_DOWN) {
        boatYOffset = clampf(boatYOffset - BOAT_STEP, BOAT_MIN_Y, BOAT_MAX_Y);
    }
    glutPostRedisplay();
}

// [AN-05] mouseWheel() - Camera zoom (mouse scroll)
void mouseWheel(int, int direction, int, int) {
    zoomLevel = clampf(zoomLevel - direction * 0.1f, 0.5f, 2.5f);
    ApplyProjection();
    glutPostRedisplay();
}

// ======================================================================
//  Init / main
// ======================================================================
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    InitScene();


// ------------------------------------------------------------------
    // Trees Setup (Banyan shifted far left & Coconut near the hut)
    // ------------------------------------------------------------------

    // 1. Banyan Trees (Pushed further left & Across the River)
    banyanTrees.clear();
    // Pushed far left (-1.08f) to clear space
    banyanTrees.push_back({-1.08f, -0.92f, 1.40f});

    // Banyan Trees across the river
    banyanTrees.push_back({-0.25f, -0.08f, 0.95f});
    banyanTrees.push_back({ 0.35f, -0.08f, 0.90f});

    // 2. Mango Trees (Positioned naturally in between)
    mangoTrees.clear();
    mangoTrees.push_back({-0.82f, -0.92f, 1.25f});

    // Mango trees across the river
    mangoTrees.push_back({-0.65f, -0.08f, 0.90f});
    mangoTrees.push_back({ 0.75f, -0.08f, 0.85f});

    // 3. Coconut Trees (Placed right next to the hut / fence)
    coconutTrees.clear();
    // Shifted right (-0.32f, -0.22f) so they stand right beside the hut
    coconutTrees.push_back({-0.60f, -0.92f, 1.20f});
    // 3. Coconut Trees (Slightly reduced for optimal proportion)
    coconutTrees.clear();
    coconutTrees.push_back({ 0.22f, -0.92f, 1.50f}); // Between huts (Medium-Large)
    coconutTrees.push_back({ 0.35f, -0.92f, 1.40f}); // Between huts (Medium-Large)
    boatX = -0.1f; // Placed near the middle of the river

    // Reduced fish count to only 2 for a minimal and clean river view

}
//Mouse//
// [AN-04] handleMouse() - Boat horizontal movement (mouse click)
void handleMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {

            boatX += 0.08f;
        }
        else if (button == GLUT_RIGHT_BUTTON) {

            boatX -= 0.08f;
        }
    }
}



int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH) - winW) / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - winH) / 2);
    glutCreateWindow("Village Scenario - Extended");

    init();
    ApplyProjection();
    glutMouseFunc(handleMouse);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);



#ifdef FREEGLUT
    glutMouseWheelFunc(mouseWheel);
#endif
    glutTimerFunc(16, Update, 0);

    glutMainLoop();
    return 0;
}
