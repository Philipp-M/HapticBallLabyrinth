#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <thread>
#include <cmath>
#include "Physics.hpp"
#include "HandleInterface.hpp"
#include "GLMain.hpp"

#define PROGRAM_NAME "Ball Labyrinth"

#define OBJ_FILE_PATH_LABYRINTH "scenes/labyrinths/Labyrinth"
#define OBJ_FILE_PATH_BALL "scenes/Ballrough.obj"
#define MATERIAL_FOLDER "scenes/"
#define COLLISION_GEOMETRY_PATH "scenes/labyrinths/walloutput"

#define MAX_ROTATION 5     /**< Maximum tilt degree for labyrinth. */
#define ROTATION_STEP 0.15 /**< Rotation step in degree to tilt labyrinth */

#define DELTA_TIME 0.001 /**< Loop time for physics thread. */
#define BALL_RADIUS 1.0  /**< Ball radius in centimeters. */
#define BALL_MASS                                                                                  \
    (7.82 * 4.0 / 3.0 * M_PI * BALL_RADIUS * BALL_RADIUS                                           \
     * BALL_RADIUS)              /**< Ball mass for density of steel in gramm. */
#define BALL_EPSILON 0.5         /**< Ball refraction material constant. */
#define BALL_ROLL_FRICTION 0.001 /**< Ball roll friction constant. */


int
showMessageBox(float elapsedTime)
{
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "start" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "quit" },
    };
    const SDL_MessageBoxColorScheme colorScheme = { { /* .colors (.r, .g, .b) */
                                                      /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
                                                      { 255, 0, 0 },
                                                      /* [SDL_MESSAGEBOX_COLOR_TEXT] */
                                                      { 0, 255, 0 },
                                                      /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
                                                      { 255, 255, 0 },
                                                      /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
                                                      { 0, 0, 255 },
                                                      /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
                                                      { 255, 0, 255 } } };

    std::string message;

    if (elapsedTime > 0.0)
    {
        message = "It took you " + std::to_string(elapsedTime) + " seconds to complete.";
    }
    else
    {
        message = "Press start to begin!";
    }

    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        nullptr,                    /* .window */
        "Ball Labyrinth",           /* .title */
        message.c_str(),            /* .message */
        SDL_arraysize(buttons),     /* .numbuttons */
        buttons,
        nullptr /* .buttons */
    };
    int buttonid;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0)
    {
        std::cout << "error displaying message box!" << std::endl;
        return -1;
    }

    if (buttonid == -1)
    {
        std::cout << "no button selected!" << std::endl;
        return -1;
    }
    else
    {
        return buttonid;
    }
}


int
main(int argc, char* argv[])
{
    assert(argc == 3);
    SDL_Window*   mainwindow;  /**< Window handle. */
    SDL_GLContext maincontext; /**< Opengl context handle. */

    /** Initialize SDL's Video subsystem. */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Unable to initializeGraphics SDL";
        return 1;
    }

    /** Request opengl 3.3 context. */
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /** Get size of display. */
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return 1;
    }

    /** Create our window centered at with window size and height similar to display. */
    mainwindow = SDL_CreateWindow(PROGRAM_NAME,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  dm.w,
                                  dm.h,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    /** Die if creation failed. */
    if (!mainwindow)
    {
        std::cout << "SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    bool quit           = false; /**< Variable used to check if program should be exited. */
    int  labyrinthIndex = 1;     /**< Index, which labyrinth is loaded. */

    std::string labyrinthObjFilePath;             /**< Path to labyrinth obj */
    std::string materialFolder = MATERIAL_FOLDER; /**< Path to material files */
    std::string collisionGeometryFilePath;        /**< Path to labyrinth collision geometry */

    labyrinthObjFilePath      = OBJ_FILE_PATH_LABYRINTH + std::to_string(labyrinthIndex) + ".obj";
    collisionGeometryFilePath = COLLISION_GEOMETRY_PATH + std::to_string(labyrinthIndex) + ".txt";
    labyrinthIndex++;

    /** Create graphics object with .obj files. */
    std::vector<std::string> objFilePaths
        = { OBJ_FILE_PATH_BALL, labyrinthObjFilePath }; /**< Path for obj files. */
    GLMain glMain(mainwindow, maincontext, objFilePaths, materialFolder);

    showMessageBox(0.0);


    /** Start Handle communication **/

    HandleInterface handleInterface(500000, std::string(argv[1]), std::string(argv[2]));

    while (!quit)
    {

        SDL_Event event;

        /** Create physics object and add collision models. */
        Physics physics(DELTA_TIME);
        physics.addBall(glMain.getScene()->getModelByName("Ball"),
                        BALL_MASS,
                        BALL_RADIUS,
                        BALL_EPSILON,
                        BALL_ROLL_FRICTION);

        physics.addWalls(collisionGeometryFilePath);

        float xAxisRotation = 0.0; /**< x rotation of labyrinth. */
        float yAxisRotation = 0.0; /**< y rotation of labyrinth. */

        bool upKeyPressed, downKeyPressed, leftKeyPressed,
            rightKeyPressed; /**< State variables for key pressed check. */
        upKeyPressed = downKeyPressed = leftKeyPressed = rightKeyPressed = false;


        unsigned int gameStartTime = SDL_GetTicks();
        float        endTime;
        //    unsigned int startTime = SDL_GetTicks();
        //    unsigned int frames = 0;

        std::thread physicsThread(&Physics::update,
                                  std::ref(physics)); /**< Physics thread, calling the update
                                                         function of the physics object. */

        /** Game loop */
        while (!quit && physics.inGame())
        {
            std::cout << "handle1: "<< handleInterface.getPos1() << ", handle2: "<< handleInterface.getPos2() << std::endl;
            /** Update graphics model according to calculated positions and rotations of physics. */
            physics.updateGraphicsModel();
            /** Draw graphics object. */
            glMain.display();
            SDL_GL_SwapWindow(mainwindow);

            /** Check for events. */
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        quit = true;
                        break;
                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_UP:
                                upKeyPressed = true;
                                break;
                            case SDLK_DOWN:
                                downKeyPressed = true;
                                break;
                            case SDLK_LEFT:
                                leftKeyPressed = true;
                                break;
                            case SDLK_RIGHT:
                                rightKeyPressed = true;
                                break;
                            case SDLK_q:
                                quit = true;
                        }
                        break;
                    case SDL_KEYUP:
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_UP:
                                upKeyPressed = false;
                                break;
                            case SDLK_DOWN:
                                downKeyPressed = false;
                                break;
                            case SDLK_LEFT:
                                leftKeyPressed = false;
                                break;
                            case SDLK_RIGHT:
                                rightKeyPressed = false;
                                break;
                        }
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                            glMain.reshape(event.window.data1, event.window.data2);
                        break;
                }
            }

            xAxisRotation = handleInterface.getPos1() / 10.0;
            yAxisRotation = -handleInterface.getPos2() / 10.0;

            /* if (upKeyPressed) */
            /* { */
            /*     if (xAxisRotation > -MAX_ROTATION) */
            /*     { */
            /*         xAxisRotation -= ROTATION_STEP; */
            /*         std::cout << "xaxis: " << xAxisRotation << std::endl; */
            /*     } */
            /* } */

            /* if (downKeyPressed) */
            /* { */
            /*     if (xAxisRotation < MAX_ROTATION) */
            /*     { */
            /*         xAxisRotation += ROTATION_STEP; */
            /*         std::cout << "xaxis: " << xAxisRotation << std::endl; */
            /*     } */
            /* } */

            /* if (leftKeyPressed) */
            /* { */
            /*     if (yAxisRotation < MAX_ROTATION) */
            /*     { */
            /*         yAxisRotation += ROTATION_STEP; */
            /*         std::cout << "yaxis: " << yAxisRotation << std::endl; */
            /*     } */
            /* } */

            /* if (rightKeyPressed) */
            /* { */
            /*     if (yAxisRotation > -MAX_ROTATION) */
            /*     { */
            /*         yAxisRotation -= ROTATION_STEP; */
            /*         std::cout << "yaxis: " << yAxisRotation << std::endl; */
            /*     } */
            /* } */

            glMain.resetModelRotationAroundAxis(0);
            glMain.resetModelRotationAroundAxis(1);
            glMain.rotateModelAroundAxis(0, 0, xAxisRotation);
            glMain.rotateModelAroundAxis(1, 0, xAxisRotation);
            glMain.rotateModelAroundAxis(0, 1, yAxisRotation);
            glMain.rotateModelAroundAxis(1, 1, yAxisRotation);
            physics.rotateEarthAccelerationXY(xAxisRotation, yAxisRotation);

            //        if (SDL_GetTicks() - startTime >= 1000) {
            //            std::cout << "fps: " << frames + 1 << std::endl;
            //            startTime = SDL_GetTicks();
            //            frames = 0;
            //        } else {
            //            frames++;
            //        }
        }

        endTime = ((float) (SDL_GetTicks() - gameStartTime)) / 1000.0f;

        physics.quitPhysics();

        physicsThread.join();

        if (!quit)
        {
            if (showMessageBox(endTime) != 0)
            {
                break;
            }
        }

        labyrinthObjFilePath
            = OBJ_FILE_PATH_LABYRINTH + std::to_string(labyrinthIndex % 10 + 1) + ".obj";
        collisionGeometryFilePath
            = COLLISION_GEOMETRY_PATH + std::to_string(labyrinthIndex % 10 + 1) + ".txt";
        labyrinthIndex++;


        glMain.initializeNewLabyrinth(labyrinthObjFilePath, materialFolder);
    }
    handleInterface.quit = true;

    /** Clear glMain */
    glMain.clearOpenGLSceneAndShaderProgram();

    /** Delete our opengl context, destroy our window, and shutdown SDL */
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();

    return 0;
}
