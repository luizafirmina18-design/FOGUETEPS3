#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <vector>
#include <string>
#include <math.h>

#define SCREEN_W 1280
#define SCREEN_H 720

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct PartInfo {
    int type; // 0=Capsula 1=Tanque 2=Motor 3=Booster 4=Tanque Grande 5=Ale ta
    float x, y, w, h;
    float fuel;
    float maxFuel;
    bool isBooster;
    bool active;
};

class Game {
public:
    Game();
    ~Game();
    bool init();
    void shutdown();
    void run();

private:
    SDL_Surface* screen;
    bool running;
    int gameState; // 0=CONSTRUIR 1=VOANDO 2=EXPLODIU 3=ORBITA
    int selectedPart;
    int partCount;
    float buildCursorY;

    // Foguete / voo
    std::vector<PartInfo> parts;
    Vec2 rocketPos;
    Vec2 rocketVel;
    float rocketAngle;
    float throttle;
    float totalFuel;
    float maxFuelTotal;
    float altitude;
    float speed;
    float cameraY;
    float timeFlight;
    int stage;
    bool exploded;
    float explodeTimer;
    float thrustPower;

    void initWorld();
    void resetRocket();
    void addPart(int type);
    void removeLastPart();
    void update(float dt, float analogX, float analogY, bool btnX, bool btnO, bool btnStart, bool btnUp, bool btnDown, bool btnTriangle);
    void updateFlight(float dt, bool btnX, bool btnO);
    void checkStage();

    // Render
    void render();
    void renderBuild();
    void renderFlight();
    void drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
    void drawCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b);
    void drawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b);
    void drawRocketAt(int cx, int cy, float scale);
    void drawPartIcon(int x, int y, int type, int w, int h);

    Uint32 getColor(Uint8 r, Uint8 g, Uint8 b);
    void setPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);
    float dist(float x1, float y1, float x2, float y2);
};

#endif