#include <GL/glew.h>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include "Physics.hpp"
#include "ShaderProgram.hpp"
#include "GLMain.hpp"

#define PROGRAM_NAME "Ball Labyrinth"

#define OBJFILEPATH "scenes/LabyrinthandBallblend.obj"
#define MATERIALFOLDER "scenes/"
#define COLLISIONGEOMETRYPATH "scenes/walloutput.txt"

#define MAXROTATION 10

int main(int argc, char *argv[]) {

    SDL_Window *mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */

    if (SDL_Init(SDL_INIT_VIDEO) < 0) { /* Initialize SDL's Video subsystem */
        std::cout << "Unable to initializeGraphics SDL";
        return 1;
    }

    /* Request opengl 4.4 context. */
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); // | SDL_WINDOW_RESIZABLE
    if (!mainwindow) { /* Die if creation failed */
        std::cout << "SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    GLMain glMain(mainwindow, maincontext, OBJFILEPATH, MATERIALFOLDER);
    SDL_Event event;

    Physics physics(0.0, 0.001);
    physics.addBall(glMain.getScene()->getModelByName("Ball"), 0.01, 1.0, 1.0);
    physics.addWalls(COLLISIONGEOMETRYPATH);

    double xAxisRotation = 0.0;
    double yAxisRotation = 0.0;

    bool xAxisChanged = false;
    bool yAxisChanged = false;


    unsigned int startTime = SDL_GetTicks();
    unsigned int frames = 0;

    while (!quit) {
        glMain.display();
        SDL_GL_SwapWindow(mainwindow);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            if(xAxisRotation < MAXROTATION) {
                                glMain.rotateModelAroundAxis(0, 0, 1.0);
                                glMain.rotateModelAroundAxis(1, 0, 1.0);
                                xAxisRotation += 1.0;
                                physics.rotateEarthAccelerationX(1.0);
                                xAxisChanged = true;
                            }
                            break;
                        case SDLK_DOWN:
                            if(xAxisRotation > -MAXROTATION) {
                                glMain.rotateModelAroundAxis(0, 0, -1.0);
                                glMain.rotateModelAroundAxis(1, 0, -1.0);
                                xAxisRotation -= 1.0;
                                physics.rotateEarthAccelerationX(-1.0);
                                xAxisChanged = true;
                            }
                            break;
                        case SDLK_LEFT:
                            if(yAxisRotation > -MAXROTATION) {
                                glMain.rotateModelAroundAxis(0, 1, -1.0);
                                glMain.rotateModelAroundAxis(1, 1, -1.0);
                                yAxisRotation -= 1.0;
                                physics.rotateEarthAccelerationY(-1.0);
                                yAxisChanged = true;
                            }
                            break;
                        case SDLK_RIGHT:
                            if(yAxisRotation < MAXROTATION) {
                                glMain.rotateModelAroundAxis(0, 1, 1.0);
                                glMain.rotateModelAroundAxis(1, 1, 1.0);
                                yAxisRotation += 1.0;
                                physics.rotateEarthAccelerationY(1.0);
                                yAxisChanged = true;
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            xAxisChanged = false;
                            break;
                        case SDLK_DOWN:
                            xAxisChanged = false;
                            break;
                        case SDLK_LEFT:
                            yAxisChanged = false;
                            break;
                        case SDLK_RIGHT:
                            yAxisChanged = false;
                            break;
                    }
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                        glMain.reshape(event.window.data1, event.window.data2);
                    break;
            }
        }
        if(!xAxisChanged) {
            if(xAxisRotation > 0.0) {
                glMain.rotateModelAroundAxis(0, 0, -1.0);
                glMain.rotateModelAroundAxis(1, 0, -1.0);
                xAxisRotation -= 1.0;
                physics.rotateEarthAccelerationX(-1.0);
            } else if(xAxisRotation < 0.0) {
                glMain.rotateModelAroundAxis(0, 0, 1.0);
                glMain.rotateModelAroundAxis(1, 0, 1.0);
                xAxisRotation += 1.0;
                physics.rotateEarthAccelerationX(1.0);
            }
        }
        if(!yAxisChanged) {
            if(yAxisRotation > 0.0) {
                glMain.rotateModelAroundAxis(0, 1, -1.0);
                glMain.rotateModelAroundAxis(1, 1, -1.0);
                yAxisRotation -= 1.0;
                physics.rotateEarthAccelerationY(-1.0);
            } else if(yAxisRotation < 0.0) {
                glMain.rotateModelAroundAxis(0, 1, 1.0);
                glMain.rotateModelAroundAxis(1, 1, 1.0);
                yAxisRotation += 1.0;
                physics.rotateEarthAccelerationY(1.0);
            }
        }

        if(SDL_GetTicks() - startTime >= 1000) {
            std::cout << "fps: " << frames + 1 << std::endl;
            startTime = SDL_GetTicks();
            frames = 0;
        } else {
            frames++;
        }

        physics.update();
    }
    /* Delete our opengl context, destroy our window, and shutdown SDL */
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();

    return 0;
}
