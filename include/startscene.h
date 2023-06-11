/*******************************************************************************************
*
*   Startscene v1.0.0 - Tool Description
*
*   MODULE USAGE:
*       #define GUI_STARTSCENE_IMPLEMENTATION
*       #include "gui_startscene.h"
*
*       INIT: GuiStartsceneState state = InitGuiStartscene();
*       DRAW: GuiStartscene(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2022 huasushis & dzr. All Rights Reserved.
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

#ifndef GUI_STARTSCENE_H
#define GUI_STARTSCENE_H

typedef struct {
    // Define anchors
    Vector2 anchor01;            // ANCHOR ID:1
    
    // Define controls variables
    bool WindowBox000Active;            // WindowBox: WindowBox000
    bool IPboxEditMode;
    char IPboxText[256];            // TextBox: IPbox
    bool PortboxEditMode;
    int PortboxValue;            // TextBox: Portbox
    bool NameboxEditMode;
    char NameboxText[128];            // TextBox: Namebox

    // Define rectangles
    Rectangle layoutRecs[9];

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required

} GuiStartsceneState;

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
GuiStartsceneState InitGuiStartscene(void);
void GuiStartscene(GuiStartsceneState *state);
static void StartButton();                // Button: StartButton logic
extern bool isButtonDown;

#ifdef __cplusplus
}
#endif

#endif // GUI_STARTSCENE_H

/***********************************************************************************
*
*   GUI_STARTSCENE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_STARTSCENE_IMPLEMENTATION)

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
bool isButtonDown;
GuiStartsceneState InitGuiStartscene(void)
{
    GuiStartsceneState state = { 0 };

    // Init anchors
    state.anchor01 = (Vector2){ 0, 0 };            // ANCHOR ID:1
    
    // Initilize controls variables
    state.WindowBox000Active = true;            // WindowBox: WindowBox000
    state.IPboxEditMode = false;
    strcpy(state.IPboxText, "192.168");            // TextBox: IPbox
    state.PortboxEditMode = false;
    state.PortboxValue = 11451;           // TextBox: Portbox
    state.NameboxEditMode = false;
    strcpy(state.NameboxText, "sblhz");            // TextBox: Namebox

    // Init controls rectangles
    state.layoutRecs[0] = (Rectangle){ state.anchor01.x + 0, state.anchor01.y + 0, 336, 288 };// WindowBox: WindowBox000
    state.layoutRecs[1] = (Rectangle){ state.anchor01.x + 120, state.anchor01.y + 40, 88, 24 };// Label: Label001
    state.layoutRecs[2] = (Rectangle){ state.anchor01.x + 72, state.anchor01.y + 88, 120, 24 };// TextBox: IPbox
    state.layoutRecs[3] = (Rectangle){ state.anchor01.x + 40, state.anchor01.y + 88, 24, 24 };// Label: Label003
    state.layoutRecs[4] = (Rectangle){ state.anchor01.x + 200, state.anchor01.y + 88, 16, 24 };// Label: Label004
    state.layoutRecs[5] = (Rectangle){ state.anchor01.x + 208, state.anchor01.y + 88, 80, 24 };// TextBox: Portbox
    state.layoutRecs[6] = (Rectangle){ state.anchor01.x + 40, state.anchor01.y + 144, 32, 24 };// Label: Label006
    state.layoutRecs[7] = (Rectangle){ state.anchor01.x + 72, state.anchor01.y + 144, 216, 24 };// TextBox: Namebox
    state.layoutRecs[8] = (Rectangle){ state.anchor01.x + 104, state.anchor01.y + 208, 120, 24 };// Button: StartButton

    // Custom variables initialization
    isButtonDown = false;

    return state;
}
// Button: StartButton logic
static void StartButton()
{
    isButtonDown = true;
}


void GuiStartscene(GuiStartsceneState *state)
{
    // Const text
    const char *WindowBox000Text = "Battle of Balls";    // WINDOWBOX: WindowBox000
    const char *Label001Text = "Battle of Balls";    // LABEL: Label001
    const char *Label003Text = "IP:";    // LABEL: Label003
    const char *Label004Text = ":";    // LABEL: Label004
    const char *Label006Text = "Name:";    // LABEL: Label006
    const char *StartButtonText = "Start Game";    // BUTTON: StartButton
    
    // Draw controls
    if (state->WindowBox000Active)
    {
        state->WindowBox000Active = !GuiWindowBox(state->layoutRecs[0], WindowBox000Text);
        GuiLabel(state->layoutRecs[1], Label001Text);
        if (GuiTextBox(state->layoutRecs[2], state->IPboxText, 256, state->IPboxEditMode)) state->IPboxEditMode = !state->IPboxEditMode;
        GuiLabel(state->layoutRecs[3], Label003Text);
        GuiLabel(state->layoutRecs[4], Label004Text);
        if (GuiValueBox(state->layoutRecs[5], "", &state->PortboxValue, 0, 65535, state->PortboxEditMode)) state->PortboxEditMode = !state->PortboxEditMode;
        GuiLabel(state->layoutRecs[6], Label006Text);
        if (GuiTextBox(state->layoutRecs[7], state->NameboxText, 128, state->NameboxEditMode)) state->NameboxEditMode = !state->NameboxEditMode;
        if (GuiButton(state->layoutRecs[8], StartButtonText)) StartButton(); 
    }
}

#endif // GUI_STARTSCENE_IMPLEMENTATION
