#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <stdio.h>
#include <string.h>

Game::Game() {
    screen = NULL;
    running = false;
    gameState = 0;
    selectedPart = 0;
    partCount = 0;
    buildCursorY = 0;
    rocketPos = Vec2(640, 500);
    rocketVel = Vec2(0, 0);
    rocketAngle = 0;
    throttle = 0;
    totalFuel = 0;
    maxFuelTotal = 1;
    altitude = 0;
    speed = 0;
    cameraY = 0;
    timeFlight = 0;
    stage = 0;
    exploded = false;
    explodeTimer = 0;
    thrustPower = 0;
}

Game::~Game() {
    shutdown();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        return false;
    }
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) return false;
    SDL_ShowCursor(SDL_DISABLE);
    ioPadInit(7);
    srand(12345);
    initWorld();
    running = true;
    return true;
}

void Game::shutdown() {
    ioPadEnd();
    SDL_Quit();
    screen = NULL;
}

void Game::initWorld() {
    parts.clear();
    // Foguete inicial igual do print: 2 boosters + tanque + capsula
    addPart(3);
    addPart(3);
    addPart(2);
    addPart(1);
    addPart(1);
    addPart(0);
    resetRocket();
    gameState = 0;
}

void Game::resetRocket() {
    rocketPos = Vec2(640, 500);
    rocketVel = Vec2(0, 0);
    rocketAngle = 0;
    throttle = 0;
    timeFlight = 0;
    stage = 0;
    exploded = false;
    explodeTimer = 0;
    cameraY = 0;
    altitude = 0;
    speed = 0;
    totalFuel = 0;
    maxFuelTotal = 1;
    for (unsigned int i = 0; i < parts.size(); i++) {
        parts[i].active = true;
        parts[i].fuel = parts[i].maxFuel;
        totalFuel += parts[i].fuel;
    }
    if (totalFuel < 1) totalFuel = 1;
    maxFuelTotal = totalFuel;
}

void Game::addPart(int type) {
    if ((int)parts.size() >= 30) return;
    PartInfo p;
    p.type = type;
    p.x = 0; p.y = 0;
    p.w = 70; p.h = 60;
    p.fuel = 0; p.maxFuel = 0;
    p.isBooster = false;
    p.active = true;
    if (type == 0) { p.w = 60; p.h = 55; }
    if (type == 1) { p.w = 70; p.h = 70; p.fuel = 100; p.maxFuel = 100; }
    if (type == 2) { p.w = 70; p.h = 50; }
    if (type == 3) { p.w = 38; p.h = 110; p.fuel = 80; p.maxFuel = 80; p.isBooster = true; }
    if (type == 4) { p.w = 70; p.h = 110; p.fuel = 180; p.maxFuel = 180; }
    if (type == 5) { p.w = 30; p.h = 40; }
    parts.push_back(p);
    partCount = parts.size();
}

void Game::removeLastPart() {
    if (!parts.empty()) {
        parts.pop_back();
        partCount = parts.size();
    }
}

void Game::run() {
    padInfo padinfo;
    padData paddata;
    Uint32 lastTime = SDL_GetTicks();
    bool startBefore = false;
    bool upBefore = false;
    bool downBefore = false;
    bool xBefore = false;
    bool triBefore = false;
    bool oBefore = false;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 16.666f;
        if (dt > 3) dt = 3;
        if (dt < 0.1f) dt = 0.1f;
        lastTime = now;

        sysUtilCheckCallback();

        float analogX = 0, analogY = 0;
        bool btnX = false, btnO = false, btnStart = false;
        bool btnUp = false, btnDown = false, btnTri = false;
        bool btnSquare = false;

        ioPadGetInfo(&padinfo);
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            int ax = paddata.ANA_L_H - 128;
            int ay = paddata.ANA_L_V - 128;
            if (ax < -10 || ax > 10 || ay < -10 || ay > 10) {
                analogX = ax / 128.0f;
                analogY = ay / 128.0f;
            }
            if (paddata.BTN_CROSS) btnX = true;
            if (paddata.BTN_CIRCLE) btnO = true;
            if (paddata.BTN_START) btnStart = true;
            if (paddata.BTN_UP) btnUp = true;
            if (paddata.BTN_DOWN) btnDown = true;
            if (paddata.BTN_TRIANGLE) btnTri = true;
            if (paddata.BTN_SQUARE) btnSquare = true;
        }

        // Troca de peca com 1 toque
        if (btnUp && !upBefore) { selectedPart--; if (selectedPart < 0) selectedPart = 5; }
        if (btnDown && !downBefore) { selectedPart++; if (selectedPart > 5) selectedPart = 0; }
        if (btnX && !xBefore) {
            if (gameState == 0) addPart(selectedPart);
        }
        if (btnTri && !triBefore) {
            if (gameState == 0) removeLastPart();
        }
        if (btnSquare && !oBefore) {
            if (gameState == 2 || gameState == 3) { initWorld(); }
        }
        if (btnStart && !startBefore) {
            if (gameState == 0) { resetRocket(); gameState = 1; }
            else if (gameState == 2 || gameState == 3) { initWorld(); }
        }
        // O no voo solta estagio
        if (btnO && !oBefore) {
            if (gameState == 1) checkStage();
        }

        update(dt, analogX, analogY, btnX, btnO, btnStart, btnUp, btnDown, btnTri);

        upBefore = btnUp;
        downBefore = btnDown;
        xBefore = btnX;
        triBefore = btnTri;
        oBefore = btnO;
        startBefore = btnStart;

        render();
        Uint32 elapsed = SDL_GetTicks() - now;
        if (elapsed < 16) SDL_Delay(16 - elapsed);
    }
}

void Game::update(float dt, float analogX, float analogY, bool btnX, bool btnO, bool btnStart, bool btnUp, bool btnDown, bool btnTri) {
    (void)analogY; (void)btnStart; (void)btnUp; (void)btnDown; (void)btnTri;
    if (gameState == 0) {
        // Na construcao nao faz nada, so espera
        return;
    }
    if (gameState == 1) {
        updateFlight(dt, btnX, btnO);
        // Inclina com analogico
        if (analogX < -0.2f) rocketAngle -= 0.02f * dt;
        if (analogX > 0.2f) rocketAngle += 0.02f * dt;
        if (rocketAngle > 0.9f) rocketAngle = 0.9f;
        if (rocketAngle < -0.9f) rocketAngle = -0.9f;
    }
    if (gameState == 2) {
        explodeTimer += dt;
        if (explodeTimer > 120) {
            // fica na tela de explodiu ate apertar START
        }
    }
}

void Game::updateFlight(float dt, bool btnX, bool btnO) {
    (void)btnO;
    timeFlight += dt / 60.0f;

    // Acelerador: segura X = 100%
    if (btnX) {
        if (throttle < 1.0f) throttle += 0.03f * dt;
        if (throttle > 1.0f) throttle = 1.0f;
    } else {
        if (throttle > 0) throttle -= 0.02f * dt;
        if (throttle < 0) throttle = 0;
    }

    float thrust = 0;
    if (throttle > 0 && totalFuel > 0) {
        thrust = throttle * 9.5f;
        float use = throttle * 1.2f * dt;
        totalFuel -= use;
        if (totalFuel < 0) totalFuel = 0;
    }

    // Gravidade
    float gravity = 3.2f;
    rocketVel.y += gravity * dt * 0.35f;
    // Empuxo pra cima + inclinacao
    rocketVel.y -= thrust * dt * 0.35f * cosf(rocketAngle);
    rocketVel.x += thrust * dt * 0.35f * sinf(rocketAngle);

    rocketPos.x += rocketVel.x * dt * 0.8f;
    rocketPos.y -= rocketVel.y * dt * 0.8f;

    // Camera segue o foguete
    float targetCam = rocketPos.y - 400;
    cameraY += (targetCam - cameraY) * 0.08f * dt;

    altitude = (500 - rocketPos.y) / 10.0f;
    if (altitude < 0) altitude = 0;
    speed = dist(0, 0, rocketVel.x, rocketVel.y) * 12.0f;

    // Bateu no chao forte = explode
    if (rocketPos.y >= 500 && timeFlight > 1.0f && rocketVel.y > 2.0f) {
        gameState = 2;
        exploded = true;
        explodeTimer = 0;
        return;
    }
    if (rocketPos.y >= 500) {
        rocketPos.y = 500;
        if (rocketVel.y > 0) rocketVel.y = 0;
    }
    // Chegou na orbita
    if (altitude > 250) {
        gameState = 3;
    }
}

void Game::checkStage() {
    // Solta os boosters / tanque vazio e ganha impulso
    stage++;
    if (totalFuel < 20) {
        // Recarrega um pouco simulando proximo estagio
        totalFuel = maxFuelTotal * 0.4f;
        if (totalFuel < 1) totalFuel = 1;
    }
    rocketVel.y += 1.5f;
}

float Game::dist(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

Uint32 Game::getColor(Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(screen->format, r, g, b);
}

void Game::setPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    Uint32* pixels = (Uint32*)screen->pixels;
    pixels[y * SCREEN_W + x] = getColor(r, g, b);
}