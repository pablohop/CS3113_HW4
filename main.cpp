#define GL_SILENCE_DEPRECATION

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>

SDL_Window* displayWindow;
bool gameIsRunning = true;

GLuint backgroundTexture;
GLuint winTexture;
GLuint loseTexture;
GLuint playerTexture;
GLuint squareTexture;
bool gameOver = false;
bool gameWon = false;


/**
* Author: [Pablo O'Hop]
* Assignment: Rise of the AI
* Date due: 2024-03-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
 Ok so i was having a problem with drawing the ground, as for some reason my ground kept NOT showing up on screen
 and would only show up on occasion. I promise you Enemy 2 is in a pit and Enemy 3 is on a platform. Also for some
 reason my textures are showing up all discolored so idk why that is. Also I could not get music to work on time.
 */

float playerX = -0.8f;
float playerY = -0.6f;
float playerSpeed = 0.005f;
float playerVelocityX = 0.0f;
float playerJumpPower = 0.1f;
float gravity = -0.01f;
float playerVelocityY = 0.0f;
bool playerIsJumping = false;


float playerSize = 0.03f;
float enemySize = 0.05f;
float bulletSize = 0.01f;


float groundLevel = -0.6f;
float pitLevel = -0.8f;
float platformLevel = -0.4f;


struct Bullet {
    float x, y;
    bool active;
};

std::vector<Bullet> bullets;


struct Enemy {
    float x, y;
    bool active;
    int type;
    float velocityX;
    float velocityY;
    bool isJumping;
};

std::vector<Enemy> enemies;


GLuint LoadTexture(const char* filepath) {
    SDL_Surface* surface = IMG_Load(filepath);
    if (surface == NULL) {
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_PixelFormat* format = surface->format;
    GLenum textureFormat = GL_RGB;
    if (format->BytesPerPixel == 4) {
        textureFormat = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, surface->w, surface->h, 0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    SDL_FreeSurface(surface);

    return textureID;
}


void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    bullets.push_back({playerX + playerSize, playerY, true});
                    break;
                case SDLK_LEFT:
                    playerVelocityX = -playerSpeed;
                    break;
                case SDLK_RIGHT:
                    playerVelocityX = playerSpeed;
                    break;
                case SDLK_UP:
                    if (!playerIsJumping) {
                        playerVelocityY = playerJumpPower;
                        playerIsJumping = true;
                    }
                    break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    playerVelocityX = 0.0f;
                    break;
            }
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
            bullets.push_back({playerX + playerSize, playerY, true});}
    }
}

float square2JumpPower = 0.045f;
float square2Gravity = -0.005f;

void update() {
    playerX += playerVelocityX;
    playerY += playerVelocityY;
    playerVelocityY += gravity;

    if ((playerY <= groundLevel && (playerX < 0.3f || playerX > 0.7f)) ||
        (playerX >= 0.5f && playerX <= 1.0f && playerY <= platformLevel)) {
        playerY = (playerX >= 0.5f && playerX <= 1.0f) ? platformLevel : groundLevel;
        playerIsJumping = false;
        playerVelocityY = 0.0f;}
    
    if (playerX >= 0.3f && playerX <= 0.7f && playerY <= pitLevel) {
           playerY = pitLevel;
           playerIsJumping = false;
           playerVelocityY = 0.0f;
        }

    for (auto& enemy : enemies) {
        switch (enemy.type) {
            case 1:
                enemy.x += enemy.velocityX;
                if (enemy.x < -0.3f || enemy.x > 0.3f) {
                    enemy.velocityX *= -1;
                }
                break;
            case 2:
                enemy.y += enemy.velocityY;
                enemy.velocityY += square2Gravity;
                if (enemy.y <= pitLevel) {
                    enemy.y = pitLevel;
                    enemy.isJumping = false;
                    enemy.velocityY = 0.0f;
                    }
                if (!enemy.isJumping) {
                enemy.velocityY = square2JumpPower;
                enemy.isJumping = true;
                }
                break;
            case 3:
                if (enemy.type == 3 && rand() % 60 == 0) {
                    bullets.push_back({enemy.x - enemySize, enemy.y, true});
                }
                break;
        }

        if (enemy.active && std::abs(playerX - enemy.x) < enemySize && playerY > enemy.y && playerY - enemy.y < playerSize + enemySize) {
                enemy.active = false;
                playerVelocityY = playerJumpPower * 0.5;
            }
    }

    float bulletSpeed = 0.005f;
    for (auto& bullet : bullets) {
        if (bullet.active) {
            if (bullet.y == playerY) {
                bullet.x += bulletSpeed;
               } else {
                   bullet.x -= bulletSpeed;
               }
            if (bullet.x < -1.0f || bullet.x > 1.0f) {
                bullet.active = false;
               }
           }
       }

    for (auto& bullet : bullets) {
        for (auto& enemy : enemies) {
            if (bullet.active && enemy.active && std::abs(bullet.x - enemy.x) < enemySize && std::abs(bullet.y - enemy.y) < enemySize) {
                bullet.active = false;
                enemy.active = false;
            }
        }
    }
    
    if (!gameOver && std::all_of(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.active; })) {
            gameOver = true;
            gameWon = true;
        }

    for (const auto& enemy : enemies) {
        if (enemy.active && std::abs(playerX - enemy.x) < enemySize && std::abs(playerY - enemy.y) < enemySize) {
            gameOver = true;
            gameWon = false;
            break; } }

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());
}

void render() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(displayWindow, &windowWidth, &windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    if (!gameOver){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBegin(GL_QUADS);
        float aspectRatio = (float)windowWidth / (float)windowHeight;
        glTexCoord2f(0, 1); glVertex2f(-aspectRatio - 0.35, -1);
        glTexCoord2f(1, 1); glVertex2f(aspectRatio, -1);
        glTexCoord2f(1, 0); glVertex2f(aspectRatio, 1.5);
        glTexCoord2f(1, 0); glVertex2f(-aspectRatio - 0.35, 1.5);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glColor3f(0.5f, 0.35f, 0.05f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, groundLevel);
        glVertex2f(1.0f, groundLevel);
        glVertex2f(1.0f, groundLevel - 0.1f);
        glVertex2f(-1.0f, groundLevel - 0.1f);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);

        glLoadIdentity();
        glTranslatef(playerX, playerY, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-playerSize, -playerSize);
        glVertex2f(-playerSize, playerSize);
        glVertex2f(playerSize, 0.0f);
        glEnd();
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, playerTexture);
        glLoadIdentity();
        glTranslatef(playerX, playerY, 0.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(-playerSize, -playerSize);
        glTexCoord2f(1, 1); glVertex2f(playerSize, -playerSize);
        glTexCoord2f(1, 0); glVertex2f(playerSize, playerSize);
        glTexCoord2f(0, 0); glVertex2f(-playerSize, playerSize);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, squareTexture);
        for (const auto& enemy : enemies) {
            if (enemy.active) {
                glLoadIdentity();
                glTranslatef(enemy.x, enemy.y, 0.0f);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 1); glVertex2f(-enemySize, -enemySize);
                glTexCoord2f(1, 1); glVertex2f(enemySize, -enemySize);
                glTexCoord2f(1, 0); glVertex2f(enemySize, enemySize);
                glTexCoord2f(0, 0); glVertex2f(-enemySize, enemySize);
                glEnd();
            }
        }

        for (const auto& bullet : bullets) {
            glLoadIdentity();
            glTranslatef(bullet.x, bullet.y, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(-bulletSize, -bulletSize);
            glVertex2f(bulletSize, -bulletSize);
            glVertex2f(bulletSize, bulletSize);
            glVertex2f(-bulletSize, bulletSize);
            glEnd();
        }

        for (const auto& enemy : enemies) {
            if (enemy.active) {
                glLoadIdentity();
                glTranslatef(enemy.x, enemy.y, 0.0f);
                glBegin(GL_QUADS);
                glVertex2f(-enemySize, -enemySize);
                glVertex2f(enemySize, -enemySize);
                glVertex2f(enemySize, enemySize);
                glVertex2f(-enemySize, enemySize);
                glEnd();
            }
        }
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gameWon ? winTexture : loseTexture);
        glBegin(GL_QUADS);
        float aspectRatio = (float)windowWidth / (float)windowHeight;
        float screenHalfWidth = 1.0f;
        float screenHalfHeight = screenHalfWidth / aspectRatio;
        float offsetX = -0.7f;
        float offsetY = 0.5f;
        glTexCoord2f(0, 1); glVertex2f(-screenHalfWidth + offsetX, -screenHalfHeight + offsetY);
        glTexCoord2f(1, 1); glVertex2f(screenHalfWidth + offsetX, -screenHalfHeight + offsetY);
        glTexCoord2f(1, 0); glVertex2f(screenHalfWidth + offsetX, screenHalfHeight + offsetY);
        glTexCoord2f(0, 0); glVertex2f(-screenHalfWidth + offsetX, screenHalfHeight + offsetY);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    SDL_GL_SwapWindow(displayWindow);
}



int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Homework 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    enemies.push_back({0.2f, groundLevel, true, 1, -0.005f, 0.0f, false});
    enemies.push_back({0.4f, pitLevel, true, 2, 0.0f, 0.0f, false});
    enemies.push_back({0.65f, platformLevel, true, 3, 0.0f, 0.0f, false});
    
    backgroundTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/Background_2.png");
    winTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/maccombanner.jpg");
    loseTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/mfailed.png");
    playerTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/pureleaf.png");
    squareTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/coca.png");
    
    while (gameIsRunning) {
        processInput();
        update();
        render();
    }
    glDeleteTextures(1, &backgroundTexture);
    glDeleteTextures(1, &winTexture);
    glDeleteTextures(1, &loseTexture);
    glDeleteTextures(1, &playerTexture);
    glDeleteTextures(1, &squareTexture);
    SDL_Quit();
    return 0;
}
