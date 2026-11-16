#include "game.h"
#include <stdio.h>

void Game::drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            setPixel(xx, yy, r, g, b);
        }
    }
}

void Game::drawCircle(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) setPixel(cx + x, cy + y, r, g, b);
        }
    }
}

void Game::drawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b) {
    int dx = x2 - x1; if (dx < 0) dx = -dx;
    int dy = y2 - y1; if (dy < 0) dy = -dy;
    int steps = dx > dy ? dx : dy;
    if (steps == 0) steps = 1;
    for (int i = 0; i <= steps; i++) {
        int x = x1 + (x2 - x1) * i / steps;
        int y = y1 + (y2 - y1) * i / steps;
        setPixel(x, y, r, g, b);
        setPixel(x + 1, y, r, g, b);
        setPixel(x, y + 1, r, g, b);
    }
}

void Game::drawPartIcon(int x, int y, int type, int w, int h) {
    if (type == 0) {
        drawRect(x, y + 15, w, h - 15, 220, 220, 220);
        drawCircle(x + w / 2, y + 15, w / 2, 30, 30, 30);
        drawCircle(x + w / 2, y + 18, 8, 120, 200, 255);
    } else if (type == 1) {
        drawRect(x, y, w, h, 220, 220, 220);
        drawRect(x + 5, y + 10, w - 10, h - 20, 180, 180, 180);
    } else if (type == 2) {
        drawRect(x, y, w, h, 60, 60, 60);
        drawRect(x + 10, y + h - 10, w - 20, 12, 30, 30, 30);
    } else if (type == 3) {
        drawRect(x, y, w, h, 230, 230, 230);
        drawRect(x, y + h - 15, w, 15, 50, 50, 50);
    } else if (type == 4) {
        drawRect(x, y, w, h, 200, 200, 200);
        drawRect(x, y + 20, w, 10, 150, 150, 150);
    } else {
        drawRect(x, y, w, h, 200, 50, 50);
    }
}

void Game::drawRocketAt(int cx, int cy, float scale) {
    int y = cy;
    // Desenha de baixo pra cima na ordem que montou
    for (int i = 0; i < (int)parts.size(); i++) {
        int t = parts[i].type;
        int w = (int)(70 * scale);
        int h = (int)(55 * scale);
        if (t == 0) h = (int)(55 * scale);
        if (t == 1) h = (int)(70 * scale);
        if (t == 2) h = (int)(50 * scale);
        if (t == 3) { w = (int)(38 * scale); h = (int)(110 * scale); }
        if (t == 4) h = (int)(110 * scale);
        if (t == 5) { w = (int)(30 * scale); h = (int)(40 * scale); }
        y -= h;
        if (t == 3) {
            // Booster do lado
            if (i % 2 == 0) drawPartIcon(cx - w - 40, y + 20, t, w, h);
            else drawPartIcon(cx + 40, y + 20, t, w, h);
        } else {
            drawPartIcon(cx - w / 2, y, t, w, h);
        }
        // Fogo se estiver voando e com acelerador
        if (gameState == 1 && throttle > 0.1f && i == 0) {
            int fx = cx;
            int fy = cy + 5;
            drawCircle(fx, fy + 15, (int)(18 * throttle), 255, 150, 0);
            drawCircle(fx, fy + 10, (int)(10 * throttle), 255, 255, 0);
        }
    }
}

void Game::renderBuild() {
    // Fundo grade azul igual do print
    drawRect(0, 0, SCREEN_W, SCREEN_H, 70, 110, 180);
    for (int x = 0; x < SCREEN_W; x += 40) drawLine(x, 0, x, SCREEN_H, 60, 95, 160);
    for (int y = 0; y < SCREEN_H; y += 40) drawLine(0, y, SCREEN_W, y, 60, 95, 160);

    // Painel da esquerda
    drawRect(0, 0, 190, SCREEN_H, 20, 40, 70);
    drawRect(0, 0, 190, 40, 30, 60, 100);
    // Lista de pecas
    const char* names[6] = {"CAPSULA", "TANQUE", "MOTOR", "BOOSTER", "TANQUE GDE", "ALETA"};
    for (int i = 0; i < 6; i++) {
        int y = 60 + i * 100;
        if (i == selectedPart) drawRect(10, y - 5, 170, 90, 255, 200, 0);
        else drawRect(10, y - 5, 170, 90, 50, 80, 120);
        drawPartIcon(25, y + 5, i, 50, 45);
        // Barra do nome - retangulo colorido
        drawRect(85, y + 20, 80, 18, 255, 255, 255);
        (void)names;
    }

    // Foguete no centro
    drawRocketAt(640, 620, 1.0f);

    // Botao LAUNCH verde em cima
    drawRect(1050, 20, 180, 45, 0, 200, 80);
    drawRect(1060, 30, 160, 25, 255, 255, 255);
    // Chão
    drawRect(0, 620, SCREEN_W, 100, 100, 100, 100);
    // Instrucao embaixo - barra branca
    drawRect(200, 670, 880, 30, 255, 255, 255);
}

void Game::renderFlight() {
    // Ceu vai escurecendo com altitude
    int skyB = 255 - (int)(altitude);
    if (skyB < 20) skyB = 20;
    drawRect(0, 0, SCREEN_W, SCREEN_H, 30, 120, skyB);
    // Estrelas se alto
    if (altitude > 100) {
        for (int i = 0; i < 60; i++) {
            int sx = (i * 173) % SCREEN_W;
            int sy = (i * 97 + (int)cameraY) % SCREEN_H;
            setPixel(sx, sy, 255, 255, 255);
        }
    }
    // Chão
    int groundY = (int)(500 - cameraY);
    if (groundY < SCREEN_H) {
        drawRect(0, groundY, SCREEN_W, SCREEN_H - groundY, 80, 160, 60);
        drawRect(0, groundY, SCREEN_W, 10, 120, 120, 120);
    }
    // Foguete
    int rx = (int)rocketPos.x;
    int ry = (int)(rocketPos.y - cameraY);
    drawRocketAt(rx, ry + 150, 1.0f);

    // HUD
    drawRect(10, 10, 320, 130, 0, 0, 0);
    drawRect(20, 20, 300, 20, 255, 255, 255);
    drawRect(20, 50, 300, 20, 255, 200, 0);
    drawRect(20, 80, 300, 20, 0, 200, 255);
    drawRect(20, 110, 300, 18, 255, 255, 255);
    // Barra de combustivel
    int fuelW = (int)(280 * (totalFuel / maxFuelTotal));
    if (fuelW < 0) fuelW = 0;
    drawRect(20, 145, 300, 18, 80, 80, 80);
    drawRect(20, 145, fuelW, 18, 255, 100, 0);
    // Barra de acelerador
    drawRect(950, 10, 320, 40, 0, 0, 0);
    drawRect(960, 20, (int)(300 * throttle), 20, 0, 255, 0);

    if (gameState == 2) {
        drawRect(440, 300, 400, 120, 200, 0, 0);
        drawRect(460, 320, 360, 30, 255, 255, 255);
        drawRect(460, 360, 360, 30, 255, 255, 255);
    }
    if (gameState == 3) {
        drawRect(440, 300, 400, 120, 0, 180, 0);
        drawRect(460, 320, 360, 30, 255, 255, 255);
        drawRect(460, 360, 360, 30, 255, 255, 255);
    }
}

void Game::render() {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    if (gameState == 0) renderBuild();
    else renderFlight();
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
}