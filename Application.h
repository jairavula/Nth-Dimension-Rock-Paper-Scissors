/*
 * Application.h
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>

typedef enum { titleScreen, settingsScreen, instructionsScreen, nameSelectScreen, gameScreen, gameOverScreen, softResetScreen } _screenState;

struct _Application {
  // Put your application members and FSM state variables here!
  // =========================================================================
  UART_Baudrate baudChoice;
  bool firstCall;
};

struct _Gamesettings {
    _screenState screenState;
    int numRounds;
    int numPlayers;

    bool loadTitleScreen;
    bool loadSettingsScreen;
    bool loadInstructionsScreen;
    bool loadNameSelectScreen;

    bool startGame;
    bool endGame;

    char playerNames[6][4];

    bool player1Turn;
    bool player2Turn;
    bool player3Turn;
    bool player4Turn;
    bool player5Turn;
    bool player6Turn;

    char playerMoves[6];

    int playerScores[6];

    char roundWinners[6];
    char gameWinner;

};

typedef struct _Gamesettings Gamesettings;
typedef struct _Application Application;

// Called only a single time - inside of main(), where the application is
// constructed
Application Application_construct();

// Called once per super-loop of the main application.
void Application_loop(Application* app, HAL* hal);

// Called whenever the UART module needs to be updated
void Application_updateCommunications(Application* app, HAL* hal);

// Interprets an incoming character and echoes back to terminal what kind of
// character was received (number, letter, or other)
char Application_interpretIncomingChar(char);

// Generic circular increment function
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

#endif /* APPLICATION_H_ */
