/*******************************************************************************************
*
*   GameOverWindow v1.0.0 - Tool Description
*
*   MODULE USAGE:
*       #define GUI_GAMEOVERWINDOW_IMPLEMENTATION
*       #include "gui_GameOverWindow.h"
*
*       INIT: GuiGameOverWindowState state = InitGuiGameOverWindow();
*       DRAW: GuiGameOverWindow(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2022 Huasushis & dzr. All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string.h>     // Required for: strcpy()

#ifndef GUI_GAMEOVERWINDOW_H
#define GUI_GAMEOVERWINDOW_H

typedef struct {
    // Define anchors
    Vector2 anchor01;            // ANCHOR ID:1
    
    // Define controls variables
    bool WindowBox000Active;            // WindowBox: WindowBox000

    // Define rectangles
    Rectangle layoutRecs[4];

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required

} GuiGameOverWindowState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiGameOverWindowState InitGuiGameOverWindow(void);
int GuiGameOverWindow(GuiGameOverWindowState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_GAMEOVERWINDOW_H

/***********************************************************************************
*
*   GUI_GAMEOVERWINDOW IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_GAMEOVERWINDOW_IMPLEMENTATION)

#include "raygui.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Internal Module Functions Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
GuiGameOverWindowState InitGuiGameOverWindow(void)
{
    GuiGameOverWindowState state = { 0 };

    // Init anchors
    state.anchor01 = (Vector2){ (ScreenWidth - 288) / 2.0, (ScreenHeight - 288) / 2.0 };            // ANCHOR ID:1
    
    // Initilize controls variables
    state.WindowBox000Active = true;            // WindowBox: WindowBox000

    // Init controls rectangles
    state.layoutRecs[0] = (Rectangle){ state.anchor01.x + 0, state.anchor01.y + 0, 288, 288 };// WindowBox: WindowBox000
    state.layoutRecs[1] = (Rectangle){ state.anchor01.x + 72, state.anchor01.y + 56, 136, 56 };// Label: Label001
    state.layoutRecs[2] = (Rectangle){ state.anchor01.x + 72, state.anchor01.y + 144, 136, 40 };// Button: Restart
    state.layoutRecs[3] = (Rectangle){ state.anchor01.x + 72, state.anchor01.y + 200, 136, 40 };// Button: Home

    // Custom variables initialization

    return state;
}


int GuiGameOverWindow(GuiGameOverWindowState *state)
{
    // Const text
    const char *WindowBox000Text = "You Died!";    // WINDOWBOX: WindowBox000
    const char *Label001Text = "Game Over";    // LABEL: Label001
    const char *RestartText = "Restart";    // BUTTON: Restart
    const char *HomeText = "Home";    // BUTTON: Home
    
    // Draw controls
    if (state->WindowBox000Active)
    {
        state->WindowBox000Active = !GuiWindowBox(state->layoutRecs[0], WindowBox000Text);
        GuiLabel(state->layoutRecs[1], Label001Text);
        if (GuiButton(state->layoutRecs[2], RestartText)) return 1;
        if (GuiButton(state->layoutRecs[3], HomeText)) return -1;
    } else {
        return -1;
    }
    return 0;
}

#endif // GUI_GAMEOVERWINDOW_IMPLEMENTATION
