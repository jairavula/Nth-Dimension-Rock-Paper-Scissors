/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED()
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}
void Screen_manager(HAL *hal_p);
void Title_screen(HAL *hal_p, Gamesettings *game);
void Settings_screen(HAL *hal_p, Gamesettings *game);
void Instructions_screen(HAL *hal_p, Gamesettings *game);
void NameSelect_screen(HAL *hal_p, Gamesettings *game);
void Game_screen(HAL *hal_p, Gamesettings *game);
void GameOver_screen(HAL *hal_p, Gamesettings *game);
void Round_logic(HAL *hal_p, Gamesettings *game, int *roundCount);
void Round_winLogic(HAL *hal_p, Gamesettings *game, int *roundCount);
void Game_logic(HAL *hal_p, Gamesettings *game);
void getPlayerInput(HAL *hal_p, bool *currentPlayerTurn, bool *nextPlayerTurn,
                    char *name, Gamesettings *game, int playerNumber,
                    int *playersMoved, bool *MSG);
void NameSelect_screenDialogue(HAL *hal_p, Gamesettings *game);
void NameEntry(HAL *hal_p, Gamesettings *game, int *yNamePos, int *xNamePos,
               int *nameIndex);
void Settings_screenLogic(HAL* hal_p, Gamesettings* game, int *settingsScreenCursorPos, char* antiCursor, char* cursor, char* numRoundsChar, char* numPlayersChar, int *numRounds, int *numPlayers);

/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The Application constructor
 * should be responsible for initializing all hardware components as well as all
 * other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to temporarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void)
{
    // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
    WDT_A_holdTimer();

    // Initialize the system clock and background hardware timer, used to enable
    // software timers to time their measurements properly.
    InitSystemTiming();

    // Initialize the main Application object and HAL object
    HAL hal = HAL_construct();
    Application app = Application_construct();

    // Do not remove this line. This is your non-blocking check.
    InitNonBlockingLED();

    // Main super-loop! In a polling architecture, this function should call
    // your main FSM function over and over.
    while (true)
    {
        // Do not remove this line. This is your non-blocking check.
        PollNonBlockingLED();
        HAL_refresh(&hal);
        Application_loop(&app, &hal);
    }
}

/**
 * A helper function which increments a value with a maximum. If incrementing
 * the number causes the value to hit its maximum, the number wraps around
 * to 0.
 */
uint32_t CircularIncrement(uint32_t value, uint32_t maximum)
{
    return (value + 1) % maximum;
}

/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */
Application Application_construct()
{
    Application app;
    // Initialize local application state variables here!
    app.baudChoice = BAUD_9600;
    app.firstCall = true;

    return app;
}

/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time,
 * followed by refreshing all inputs to the system through a convenient
 * [HAL_refresh()] call.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */

//  static int x = 5;
// static int y = 5;
void Application_loop(Application *app_p, HAL *hal_p)
{
    // Restart/Update communications if either this is the first time the
    // application is run or if BoosterPack S2 is pressed (which means a new
    // baudrate is being set up)
    if (Button_isTapped(&hal_p->boosterpackS2) || app_p->firstCall)
    {
        Application_updateCommunications(app_p, hal_p);
    }

    Screen_manager(hal_p);

    if (UART_hasChar(&hal_p->uart))
    {
        // The character received from your serial terminal
        char rxChar = UART_getChar(&hal_p->uart);

        char txChar = Application_interpretIncomingChar(rxChar);

        if ((rxChar == 'r') || (rxChar == 'p') || (rxChar == 's'))
        {
            LED_turnOn(&hal_p->boosterpackBlue);
        }
        /*
         char str[2];
         str[0] = rxChar;
         str[1] = '\0';

         Graphics_drawString(&hal_p->g_sContext, (int8_t*)str, -1, x, y, true);
         x +=5;
         if (x >= 120){
         y +=10;
         x = 0;
         }
         */

    }
}

/**
 * Updates which LEDs are lit and what baud rate the UART module communicates
 * with, based on what the application's baud choice is at the time this
 * function is called.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_updateCommunications(Application *app_p, HAL *hal_p)
{
    // When this application first loops, the proper LEDs aren't lit. The
    // firstCall flag is used to ensure that the
    if (app_p->firstCall)
    {
        app_p->firstCall = false;
    }

    // When BoosterPack S2 is tapped, circularly increment which baud rate is
    // used.
    else
    {
        uint32_t newBaudNumber = CircularIncrement((uint32_t) app_p->baudChoice,
                                                   NUM_BAUD_CHOICES);
        app_p->baudChoice = (UART_Baudrate) newBaudNumber;
    }

    // Start/update the baud rate according to the one set above.
    UART_SetBaud_Enable(&hal_p->uart, app_p->baudChoice);

    // Based on the new application choice, turn on the correct LED.
    // To make your life easier, we recommend turning off all LEDs before
    // selectively turning back on only the LEDs that need to be relit.
    // -------------------------------------------------------------------------
    LED_turnOff(&hal_p->launchpadLED2Red);
    LED_turnOff(&hal_p->launchpadLED2Green);
    LED_turnOff(&hal_p->launchpadLED2Blue);

    // TODO: Turn on all appropriate LEDs according to the tasks below.
    switch (app_p->baudChoice)
    {
    // When the baud rate is 9600, turn on Launchpad LED Red
    case BAUD_9600:
        LED_turnOn(&hal_p->launchpadLED2Red);
        break;

        // TODO: When the baud rate is 19200, turn on Launchpad LED Green
    case BAUD_19200:
        LED_turnOn(&hal_p->launchpadLED2Green);
        break;

        // TODO: When the baud rate is 38400, turn on Launchpad LED Blue
    case BAUD_38400:
        LED_turnOn(&hal_p->launchpadLED2Blue);
        break;

        // TODO: When the baud rate is 57600, turn on all Launchpad LEDs
        // (illuminates white)
    case BAUD_57600:
        LED_turnOn(&hal_p->launchpadLED2Red);
        LED_turnOn(&hal_p->launchpadLED2Green);
        LED_turnOn(&hal_p->launchpadLED2Blue);
        break;

        // In the default case, this program will do nothing.
    default:
        break;
    }
}

/**
 * Interprets a character which was incoming and returns an interpretation of
 * that character. If the input character is a letter, it return L for Letter,
 * if a number return N for Number, and if something else, it return O for
 * Other.
 *
 * @param rxChar: Input character
 * @return :  Output character
 */
char Application_interpretIncomingChar(char rxChar)
{
    // The character to return back to sender. By default, we assume the letter
    // to send back is an 'O' (assume the character is an "other" character)
    char txChar = 'O';

    // Numbers - if the character entered was a number, transfer back an 'N'
    if (rxChar >= '0' && rxChar <= '9')
    {
        txChar = 'N';
    }

    // Letters - if the character entered was a letter, transfer back an 'L'
    if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z'))
    {
        txChar = 'L';
    }

    return (txChar);
}






void Screen_manager(HAL *hal_p)
{
    static Gamesettings game = { titleScreen, 3, 2, true, true, true, true, false, false, { 0 }, true, false, false, false, { 0 },{0}, { 0 }, { 0 } };
    switch (game.screenState)
    {
    case titleScreen:
        Title_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->boosterpackS1))
        {
            game.screenState = settingsScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        if (Button_isTapped(&hal_p->launchpadS2))
        {
            game.loadInstructionsScreen = true;
            game.screenState = instructionsScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case settingsScreen:
        Settings_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->boosterpackS1))
        {
            game.screenState = nameSelectScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case instructionsScreen:
        Instructions_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->launchpadS2))
        {
            game.screenState = titleScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case nameSelectScreen:
        NameSelect_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->boosterpackS1) && game.startGame)
        {
            game.screenState = gameScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case gameScreen:
        Game_screen(hal_p, &game);
        if (game.endGame)
        {
            game.screenState = gameOverScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case gameOverScreen:
        GameOver_screen(hal_p, &game);
    }
}





void Title_screen(HAL *hal_p, Gamesettings *game)
{
    char tSLine1[] = "Rock Paper Scissors";
    char tSLine2[] = "Multiplayer Game";
    char tSLine3[] = "[Name]";
    char tSLine4[] = "BB1: Play Game";
    char tSLine5[] = "LB2: Instructions";

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine1, -1, 5, 30, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine2, -1, 5, 40, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine3, -1, 5, 60, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine4, -1, 5, 90, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine5, -1, 5, 100,
    true);
}







void Settings_screen(HAL *hal_p, Gamesettings *game)
{
    static int settingsScreenCursorPos = 61;
    static int numRounds = 3;
    static int numPlayers = 2;
    static char numRoundsChar[4];
    static char numPlayersChar[4];
    char sSLine1[] = "Choose Settings";
    char sSLine2[] = "Press JSB to change #";
    char sSLine3[] = "Press LB2 to switch";
    char sSLine4[] = "between Players";
    char sSLine5[] = "and # of Rounds";
    char sSLine6[] = "#  of  Rounds:  ";
    char sSLine7[] = "#  of  Players:  ";
    char sSLine8[] = "BB1: Confirm";
    char sSLine9[] = "LB1: Reset Settings";
    char cursor[] = "*";
    char antiCursor[] = " ";
    if (game->loadSettingsScreen)
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine1, -1, 0, 5,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine2, -1, 0, 21,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine3, -1, 0, 29,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine4, -1, 0, 37,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine5, -1, 0, 45,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine6, -1, 0, 61,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine7, -1, 0, 76,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine8, -1, 0, 103,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) sSLine9, -1, 0, 111,
        true);
        sprintf(numRoundsChar, "%d", numRounds);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                            105, settingsScreenCursorPos, true);
        sprintf(numPlayersChar, "%d", numPlayers);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar, -1,
                            105, settingsScreenCursorPos + 15, true);
        game->loadSettingsScreen = false;
    }
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                        settingsScreenCursorPos, true);
    Settings_screenLogic(hal_p, game, &settingsScreenCursorPos, antiCursor, cursor, numRoundsChar, numPlayersChar, &numRounds, &numPlayers);
    game->numPlayers = numPlayers;
    game->numRounds = numRounds;
}







void Settings_screenLogic(HAL* hal_p, Gamesettings* game, int *settingsScreenCursorPos, char* antiCursor, char* cursor, char* numRoundsChar, char* numPlayersChar, int *numRounds, int *numPlayers){
    if (Button_isTapped(&hal_p->launchpadS2) && *settingsScreenCursorPos == 61)
        {
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                                *settingsScreenCursorPos, true);
            *settingsScreenCursorPos += 15;
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                                *settingsScreenCursorPos, true);
        }
        else if (Button_isTapped(&hal_p->launchpadS2)
                && *settingsScreenCursorPos == 76)
        {
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                                *settingsScreenCursorPos, true);
            *settingsScreenCursorPos -= 15;
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                                *settingsScreenCursorPos, true);
        }
        if (Button_isTapped(&hal_p->boosterpackJS) && *settingsScreenCursorPos == 61)
        {
            if (*numRounds == 6)
            {
                *numRounds = 1;
                sprintf(numRoundsChar, "%d", *numRounds);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                    105, *settingsScreenCursorPos, true);
            }
            else
            {
                (*numRounds)++;
                sprintf(numRoundsChar, "%d", *numRounds);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                    105, *settingsScreenCursorPos, true);
            }

        }
        else if (Button_isTapped(&hal_p->boosterpackJS)
                && *settingsScreenCursorPos == 76)
        {
            if (*numPlayers == 4)
            {
                *numPlayers = 2;
                sprintf(numPlayersChar, "%d", *numPlayers);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                    -1, 105, *settingsScreenCursorPos, true);
            }
            else
            {
                (*numPlayers)++;
                sprintf(numPlayersChar, "%d", *numPlayers);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                    -1, 105, *settingsScreenCursorPos, true);
            }
        }
}








void Instructions_screen(HAL *hal_p, Gamesettings *game)
{
    char iSLine1[] = "Instructions";
    char iSLine2[] = "Select the number of ";
    char iSLine3[] = "rounds, players, and";
    char iSLine4[] = "player's names. Every";
    char iSLine5[] = "round all player's ";
    char iSLine6[] = "enter their choice. ";
    char iSLine7[] = "Whoever wins the ";
    char iSLine8[] = "round gets a point ";
    char iSLine9[] = "After all rounds are ";
    char iSLine10[] = "played, the scores";
    char iSLine11[] = "and winners are shown. ";
    char iSLine12[] = "LB2: Go Back";

    if (game->loadInstructionsScreen)
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine1, -1, 0, 5,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine2, -1, 0, 20,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine3, -1, 0, 28,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine4, -1, 0, 36,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine5, -1, 0, 44,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine6, -1, 0, 52,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine7, -1, 0, 60,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine8, -1, 0, 68,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine9, -1, 0, 76,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine10, -1, 0, 84,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine11, -1, 0, 92,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) iSLine12, -1, 0, 108,
        true);
        game->loadInstructionsScreen = false;
    }

}








void NameSelect_screenDialogue(HAL *hal_p, Gamesettings *game)
{
    char nSSLine1[] = "Name Select Screen";
    char nSSLine2[] = "Type 3 letters into ";
    char nSSLine3[] = "the UART terminal, ";
    char nSSLine4[] = "then press BB1 to";
    char nSSLine5[] = "move to the next ";
    char nSSLine6[] = "player or start the ";
    char nSSLine7[] = "game.";

    if (game->loadNameSelectScreen)
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine1, -1, 0, 5,
        true);

        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine2, -1, 5, 60,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine3, -1, 5, 68,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine4, -1, 5, 76,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine5, -1, 5, 84,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine6, -1, 5, 92,
        true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nSSLine7, -1, 5, 100,
        true);
        game->loadNameSelectScreen = false;
    }
}







void NameEntry(HAL *hal_p, Gamesettings *game, int *yNamePos, int *xNamePos,
               int *nameIndex)
{

    if (*yNamePos == 12)
    {
        game->playerNames[0][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[0],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[0][*nameIndex]);
        (*nameIndex)++;
    }
    if (*yNamePos == 20)
    {
        game->playerNames[1][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[1],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[1][*nameIndex]);
        (*nameIndex)++;
    }
    if (*yNamePos == 28)
    {
        game->playerNames[2][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[2],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[2][*nameIndex]);
        (*nameIndex)++;
    }
    if (*yNamePos == 36)
    {
        game->playerNames[3][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[3],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[3][*nameIndex]);
        (*nameIndex)++;
    }
}







void NameSelect_screen(HAL *hal_p, Gamesettings *game)
{
    static int i = 1;
    static int yPosition = 13;
    char nameField[4];
    int xNamePos = 40;
    static int yNamePos = 12;
    static int playersEntered = 0;
    static int nameIndex = 0;

    NameSelect_screenDialogue(hal_p, game);

    while (i <= game->numPlayers)
    {
        sprintf(nameField, "%d)", i);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nameField, -1, 15,
                            yPosition, true);
        i++;
        yPosition += 8;
    }

    if (nameIndex < 3)
    {
        NameEntry(hal_p, game, &yNamePos, &xNamePos, &nameIndex);
    }

    if (Button_isTapped(&hal_p->boosterpackS1))
    {
        nameIndex = 0;
        yNamePos += 8;
        playersEntered++;
        if (Button_isTapped(&hal_p->boosterpackS1)
                && playersEntered == game->numPlayers)
        {
            game->startGame = true;
        }
    }
}






void Game_screen(HAL *hal_p, Gamesettings *game)
{

    char gSLine1[] = "Game Screen";
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) gSLine1, -1, 5, 5, true);
    Game_logic(hal_p, game);

}








void Round_logic(HAL *hal_p, Gamesettings *game, int *roundCount)
{

    static int playersMoved = 0;
    bool lastPlayer = false;
    static bool MSG = true;
    static bool endOfRoundMSG = true;
    char playRoundMSG [] = "Press BB1 to play the round";

    if (playersMoved < game->numPlayers)
    {
        getPlayerInput(hal_p, &game->player1Turn, &game->player2Turn,
                       game->playerNames[0], game, 0, &playersMoved, &MSG);
        getPlayerInput(hal_p, &game->player2Turn, &game->player3Turn,
                       game->playerNames[1], game, 1, &playersMoved, &MSG);
        getPlayerInput(hal_p, &game->player3Turn, &game->player4Turn,
                       game->playerNames[2], game, 2, &playersMoved, &MSG);
        getPlayerInput(hal_p, &game->player4Turn, &lastPlayer,
                       game->playerNames[3], game, 3, &playersMoved, &MSG);

    }
    else
    {
        if (endOfRoundMSG){
            UART_sendString(&hal_p->uart, playRoundMSG);
            endOfRoundMSG = false;
        }

        if (UART_canSend(&hal_p->uart) && Button_isTapped(&hal_p->boosterpackS1))
        {
            Round_winLogic(hal_p, game, roundCount);
            (*roundCount)++;
                   playersMoved = 0;
                   MSG = true;
                   endOfRoundMSG = true;
                   game->player1Turn = true;
        }
    }
}


void Round_endLogic(HAL *hal_p, Gamesettings *game, int *roundCount) {
    int r = 0;
    int p = 0;
    int s = 0;
    int i=0;

    for (i = 0; i < game->numPlayers; i++){
        if (game->playerMoves[i] == 'r' || game->playerMoves[i] == 'R') r++;
        else if (game->playerMoves[i] == 'p' || game->playerMoves[i] == 'P') p++;
        else if (game->playerMoves[i] == 's' || game->playerMoves[i] == 'S') s++;
    }
    if ((r > 0 && p > 0 && s > 0) || (r == game->numPlayers || p == game->numPlayers || s == game->numPlayers)){
        if(r == game->numPlayers || p == game->numPlayers || s == game->numPlayers){
            for (i=0; i< game->numPlayers; i++) game->playerScores[i]++;
        }
    } else{
        for (i=0; i< game->numPlayers; i++){
            if(((game->playerMoves[i] == 'r' || game->playerMoves[i] == 'R') && s > 0) ||
               ((game->playerMoves[i] == 'p' || game->playerMoves[i] == 'P') && r > 0) ||
               ((game->playerMoves[i] == 's' || game->playerMoves[i] == 'S') && p > 0)){
                game->playerScores[i]++;
            }
        }
    }
}

void Round_winLogic(HAL *hal_p, Gamesettings *game, int *roundCount) {

Round_endLogic(hal_p, game, roundCount);
            int i = 0;
            int yPos = 30;
            for (i = 0; i < game->numPlayers; i++)
            {
                char displayMove[2];
                char displayScore[2];
                char displayRound[2];
                char wins [] = "Wins: ";
                char round [] = "Round ";
                displayMove[0] = game->playerMoves[i];
                displayMove[1] = '\0';
                snprintf(displayScore, sizeof(displayScore), "%d", game->playerScores[i]);
                snprintf(displayRound, sizeof(displayRound), "%d", *roundCount + 1);

                Graphics_drawString(&hal_p->g_sContext,
                                    (int8_t*) game->playerNames[i], -1, 5, yPos,
                                    true);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) displayMove,
                                    -1, 30, yPos,
                                    true);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) wins,
                                                    -1, 70, yPos,
                                                    true);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) displayScore,
                                                    -1, 110, yPos,
                                                    true);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) round,
                                                                                    -1, 30, 90,
                                                                                    true);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) displayRound,
                                                                    -1, 70, 90,
                                                                    true);
                yPos += 8;
            }


}




void Game_logic(HAL *hal_p, Gamesettings *game)
{
    static int roundsPlayed = 0;
    char endMSG [] = "Press BB1 to end ";

    if (roundsPlayed < game->numRounds)
    {
        Round_logic(hal_p, game, &roundsPlayed);
    }
    else
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) endMSG,
                                                            -1, 15, 80,
                                                            true);
        if (Button_isTapped(&hal_p->boosterpackS1)){
            game->endGame = true;
        }
    }
}

void GameOver_screen(HAL *hal_p, Gamesettings *game)
{
    int yPosPlayers = 25;
    int yPosWinners = 70;
    int i=0;
    char displayScore[2];
    int winners[4];
    int winnerIndex;
    int highestValue = 0;
    int numWinners = 0;

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Game Over", -1, 5, 10,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Winners:", -1, 5, 90,
       true);
    for (i=0; i < game->numPlayers; i++){
        if (game->playerScores[i] > highestValue){
            highestValue = game->playerScores[i];
            winners[0] = i;
            numWinners = 1;
        } else if (game->playerScores[i] == highestValue){
            winners[numWinners] = i;
            numWinners++;
        }
    }

    for (i=0; i < game->numPlayers; i++){
        snprintf(displayScore, sizeof(displayScore), "%d", game->playerScores[i]);

        Graphics_drawString(&hal_p->g_sContext,
                                             (int8_t*) game->playerNames[i], -1, 5, yPosPlayers,
                                             true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Wins: ",
                                                           -1, 70, yPosPlayers,
                                                           true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) displayScore,
                                                           -1, 110, yPosPlayers,
                                                           true);
        yPosPlayers +=8;

    }

    for (winnerIndex =0; winnerIndex < numWinners; winnerIndex++){
        Graphics_drawString(&hal_p->g_sContext,
                            (int8_t*) game->playerNames[winners[winnerIndex]], -1, 90, yPosWinners ,
                                                     true);
        yPosWinners +=8;
    }
}









void getPlayerInput(HAL *hal_p, bool *currentPlayerTurn, bool *nextPlayerTurn,
                    char *name, Gamesettings *game, int playerNumber,
                    int *playersMoved, bool *MSG)
{

    char pName[32];
    char gSstr1[] = ", please enter:";
    char gSstr2[] = " R or r for Rock";
    char gSstr3[] = " P or p for Paper";
    char gSstr4[] = " S or s for Scissors";

    strncpy(pName, game->playerNames[playerNumber], sizeof(pName) - 1);
    pName[sizeof(pName) - 1] = '\0';

    if (*currentPlayerTurn)

    {
        strcat(pName, gSstr1);
        if (UART_canSend(&hal_p->uart) && *MSG)
        {
            UART_sendString(&hal_p->uart, pName);
            UART_sendString(&hal_p->uart, gSstr2);
            UART_sendString(&hal_p->uart, gSstr3);
            UART_sendString(&hal_p->uart, gSstr4);
            *(MSG) = false;
        }

        if (UART_hasChar(&hal_p->uart))
        {
            char playerMove = UART_getChar(&hal_p->uart);
            if (playerMove == 'R' || playerMove == 'r' || playerMove == 'P'
                    || playerMove == 'p' || playerMove == 'S'
                    || playerMove == 's')
            {
                game->playerMoves[playerNumber] = playerMove;
                (*playersMoved)++;
                *currentPlayerTurn = false;
                *nextPlayerTurn = true;

                if (*playersMoved < game->numPlayers)
                {
                    *(MSG) = true;
                }
            }
        }
    }
}

