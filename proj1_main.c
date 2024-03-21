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

// Declaration of all functions I made
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
// Entire game is handled by the screen manager
    Screen_manager(hal_p);

/*
    if (UART_hasChar(&hal_p->uart))
    {
        // The character received from your serial terminal
        char rxChar = UART_getChar(&hal_p->uart);

        char txChar = Application_interpretIncomingChar(rxChar);

        if ((rxChar == 'r') || (rxChar == 'p') || (rxChar == 's'))
        {
            LED_turnOn(&hal_p->boosterpackBlue);
        }

         char str[2];
         str[0] = rxChar;
         str[1] = '\0';

         Graphics_drawString(&hal_p->g_sContext, (int8_t*)str, -1, x, y, true);
         x +=5;
         if (x >= 120){
         y +=10;'g
         x = 0;
         }
         */

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
 */ /*
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



*/

// Main activity of the application, all screens and functions are called/ wrapped through this higher level function
void Screen_manager(HAL *hal_p)
{ // Initializes the game settings for a new game
    static Gamesettings game = { titleScreen, 3, 2, true, true, true, true, false, false, { 0 }, true, false, false, false, { 0 },{0}, { 0 }, { 0 } };
    switch (game.screenState) // FSM logic for each screen setting
    {
    // entry point of game, leads to game settings or instructions screen through button taps
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
        // Settings screen, leads to nameSelect Screen through button tap
    case settingsScreen:
        Settings_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->boosterpackS1))
        {
            game.screenState = nameSelectScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
        // Instructions screen providing info on how to play the game
    case instructionsScreen:
        Instructions_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->launchpadS2))
        {
            game.screenState = titleScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
        // name select screen state, leads to game screen when names are inputted
    case nameSelectScreen:
        NameSelect_screen(hal_p, &game);
        if (Button_isTapped(&hal_p->boosterpackS1) && game.startGame)
        {
            game.screenState = gameScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
        // game screen, leads to game over screen when game ends through boolean variable
    case gameScreen:
        Game_screen(hal_p, &game);
        if (game.endGame)
        {
            game.screenState = gameOverScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
        // End of game screen state
    case gameOverScreen:
        GameOver_screen(hal_p, &game);
    }
}




// Title screen, only for display purposes, logic is handled in the FSM for transitions
void Title_screen(HAL *hal_p, Gamesettings *game)
{ // Text Initialization
    char tSLine1[] = "Rock Paper Scissors";
    char tSLine2[] = "Multiplayer Game";
    char tSLine3[] = "Jai";
    char tSLine4[] = "BB1: Play Game";
    char tSLine5[] = "LB2: Instructions";
// Draws the text on screen
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine1, -1, 5, 30, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine2, -1, 5, 40, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine3, -1, 5, 60, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine4, -1, 5, 90, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) tSLine5, -1, 5, 100,
    true);
}







void Settings_screen(HAL *hal_p, Gamesettings *game)
{ // Initializes data variables for settings screen
    static int settingsScreenCursorPos = 61; // Cursor positions used to determine edits/ visual updates on round number or player number
    static int numRounds = 3; // default round setting
    static int numPlayers = 2; // default player setting
    static char numRoundsChar[4]; // Holds number of rounds in a string for drawString use
    static char numPlayersChar[4];// Holds number of players in a string for drawString use
    // Initializes text for screen
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
    // If statement allows text to only be drawn once, then not reloaded
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
        // Casts int values into a string to display, and displays them
        sprintf(numRoundsChar, "%d", numRounds);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                            105, settingsScreenCursorPos, true);
        sprintf(numPlayersChar, "%d", numPlayers);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar, -1,
                            105, settingsScreenCursorPos + 15, true);
        game->loadSettingsScreen = false;
    }
    // Draws the cursor on screen, continously updating based on action
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                        settingsScreenCursorPos, true);
    // Calls the business logic for the setting screen
    Settings_screenLogic(hal_p, game, &settingsScreenCursorPos, antiCursor, cursor, numRoundsChar, numPlayersChar, &numRounds, &numPlayers);
    game->numPlayers = numPlayers; // stores local value in the game structure
    game->numRounds = numRounds; // stores local value in the game structure
}







void Settings_screenLogic(HAL* hal_p, Gamesettings* game, int *settingsScreenCursorPos, char* antiCursor, char* cursor, char* numRoundsChar, char* numPlayersChar, int *numRounds, int *numPlayers){
   // handles logic to change from rounds to players if the cursor is on rounds
    if (Button_isTapped(&hal_p->launchpadS2) && *settingsScreenCursorPos == 61)
        {// draws blank space on past cursor position
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                                *settingsScreenCursorPos, true);
            *settingsScreenCursorPos += 15;
            // draws new cursor in player position
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                                *settingsScreenCursorPos, true);
        }
    // checks if cursor is on player field this time
        else if (Button_isTapped(&hal_p->launchpadS2)
                && *settingsScreenCursorPos == 76)
        {
            // draws blank space on player cursor position
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                                *settingsScreenCursorPos, true);
            *settingsScreenCursorPos -= 15;
            // draws new cursor on rounds cursor position
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                                *settingsScreenCursorPos, true);
        }
    // Checks if joystick is clicked up to update rounds
        if (Button_isTapped(&hal_p->boosterpackJS) && *settingsScreenCursorPos == 61)
        {
            if (*numRounds == 6) // Circular increment
            {
                *numRounds = 1;
                sprintf(numRoundsChar, "%d", *numRounds);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                    105, *settingsScreenCursorPos, true);
            }
            else // If not max rounds, increase the roundNum
            {
                (*numRounds)++;
                sprintf(numRoundsChar, "%d", *numRounds);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                    105, *settingsScreenCursorPos, true);
            }

        }
        // Checks if joystick is clicked to update players
        else if (Button_isTapped(&hal_p->boosterpackJS)
                && *settingsScreenCursorPos == 76)
        {
            if (*numPlayers == 4) // if max players, circular increment back to 2
            {
                *numPlayers = 2;
                sprintf(numPlayersChar, "%d", *numPlayers);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                    -1, 105, *settingsScreenCursorPos, true);
            }
            else // increase player count
            {
                (*numPlayers)++;
                sprintf(numPlayersChar, "%d", *numPlayers);
                Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                    -1, 105, *settingsScreenCursorPos, true);
            }
        }
}








void Instructions_screen(HAL *hal_p, Gamesettings *game)
{ // init text
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
 // Like Settings screen, only display the static text once, do not reload
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
{ // init text
    char nSSLine1[] = "Name Select Screen";
    char nSSLine2[] = "Type 3 letters into ";
    char nSSLine3[] = "the UART terminal, ";
    char nSSLine4[] = "then press BB1 to";
    char nSSLine5[] = "move to the next ";
    char nSSLine6[] = "player or start the ";
    char nSSLine7[] = "game.";
// Load the text only once
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
// Here we get into some real logic, if we are on the first name execut this if statement
    if (*yNamePos == 12)
    {
        game->playerNames[0][*nameIndex] = UART_getChar(&hal_p->uart); // place the letter from uart in the i-th name index of the player
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[0], // draw letter on screen
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[0][*nameIndex]); // show the same letter on UART
        (*nameIndex)++; // increment to the next nameIndex after a letter is entered in order to take input for the next letter in name
    }
    if (*yNamePos == 20) // This is for the second player, refer to comments in player 1 above
    {
        game->playerNames[1][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[1],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[1][*nameIndex]);
        (*nameIndex)++;
    }
    if (*yNamePos == 28) // This is for third player, refer to comments in player 1 above
    {
        game->playerNames[2][*nameIndex] = UART_getChar(&hal_p->uart);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) game->playerNames[2],
                            -1, *xNamePos, *yNamePos, true);
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, game->playerNames[2][*nameIndex]);
        (*nameIndex)++;
    }
    if (*yNamePos == 36) // This is for fourth player, refer to comments in player 1 above
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
    static int i = 1; // Used to print numbered list of names
    static int yPosition = 13; // Used to place names in order on screen
    char nameField[4]; // variable to hold name
    int xNamePos = 40; // x offset of name draw string
    static int yNamePos = 12; // Used to place names in order on screen
    static int playersEntered = 0; // Holds number of players that have entered in their name so far
    static int nameIndex = 0; // Refers to each letter in each name

    NameSelect_screenDialogue(hal_p, game); // Call the text associated with this screen

    while (i <= game->numPlayers) // Execute if all players have not entered
    {
        sprintf(nameField, "%d)", i); // print "1)", "2)"....
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nameField, -1, 15,
                            yPosition, true); // print name in the respective field
        i++; // go to next player
        yPosition += 8;// increment so next player can be drawn in next field
    }

    if (nameIndex < 3) // If we are not done entering a name, execute
    {
        NameEntry(hal_p, game, &yNamePos, &xNamePos, &nameIndex);
    }

    if (Button_isTapped(&hal_p->boosterpackS1)) //Go to next player name, reset variables associated with name entry
    {
        nameIndex = 0;
        yNamePos += 8;
        playersEntered++;
        if (Button_isTapped(&hal_p->boosterpackS1) // Start the game if all players have entered
                && playersEntered == game->numPlayers)
        {
            game->startGame = true;
        }
    }
}






void Game_screen(HAL *hal_p, Gamesettings *game) // This is the game screen, calls the game logic within it
{

    char gSLine1[] = "Game Screen";
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) gSLine1, -1, 5, 5, true);
    Game_logic(hal_p, game);

}








void Round_logic(HAL *hal_p, Gamesettings *game, int *roundCount)
{
// Logic that handles player input of each round and winner/score update
    static int playersMoved = 0; // allows each player to move
    bool lastPlayer = false; // checks if round should end
    static bool MSG = true; // Allows displays MSG for player input only once
    static bool endOfRoundMSG = true; // Only display the end of Round MSG once
    char playRoundMSG [] = "Press BB1 to play the round"; // prints on UART to play round

    if (playersMoved < game->numPlayers) // Take player input if all players have not moved yet
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
        if (endOfRoundMSG){ // If all players have moved, print end of round message once
            UART_sendString(&hal_p->uart, playRoundMSG);
            endOfRoundMSG = false;
        }

        if (UART_canSend(&hal_p->uart) && Button_isTapped(&hal_p->boosterpackS1)) // If the button is tapped to end round, end it
        {
            Round_winLogic(hal_p, game, roundCount); // Computed logic for winner
            (*roundCount)++; // go to next round and reset round logic variables
                   playersMoved = 0;
                   MSG = true;
                   endOfRoundMSG = true;
                   game->player1Turn = true;
        }
    }
}


void Round_endLogic(HAL *hal_p, Gamesettings *game, int *roundCount) { // Logic to determine score alloted after round
    int r = 0; // holds num of rock inputs
    int p = 0;// holds num of paper inputs
    int s = 0;// scissors
    int i=0;// looping var

    for (i = 0; i < game->numPlayers; i++){ //Used to store all player moves in local variables
        if (game->playerMoves[i] == 'r' || game->playerMoves[i] == 'R') r++; // update if player made this move
        else if (game->playerMoves[i] == 'p' || game->playerMoves[i] == 'P') p++; // update if player made this move
        else if (game->playerMoves[i] == 's' || game->playerMoves[i] == 'S') s++; // update if player made this move
    } // Checks if all players made the same move or if all moves are the same
    if ((r > 0 && p > 0 && s > 0) || (r == game->numPlayers || p == game->numPlayers || s == game->numPlayers)){
        if(r == game->numPlayers || p == game->numPlayers || s == game->numPlayers){ // checks if all moves are the same
            for (i=0; i< game->numPlayers; i++) game->playerScores[i]++; // if so, give everyone a point
        } // nothing is done if all moves (r,p,s)  were made
    } else{ // Computer for select winners
        for (i=0; i< game->numPlayers; i++){
            if(((game->playerMoves[i] == 'r' || game->playerMoves[i] == 'R') && s > 0) || // if player moved 'r' and an 's' is present
               ((game->playerMoves[i] == 'p' || game->playerMoves[i] == 'P') && r > 0) || // or if player moved 'p' and 'r' is present
               ((game->playerMoves[i] == 's' || game->playerMoves[i] == 'S') && p > 0)){ // or if plaeyr moved 's' and 'p' is present
                game->playerScores[i]++; // give the player a point
            }
        }
    }
}

void Round_winLogic(HAL *hal_p, Gamesettings *game, int *roundCount) {

Round_endLogic(hal_p, game, roundCount); // computes score logic
            int i = 0;
            int yPos = 30;
            for (i = 0; i < game->numPlayers; i++) // iterate through all players
            {
                char displayMove[2]; // Used to hold player Move char in a string (char*)
                char displayScore[2]; // used to hold player score int in a string (char*)
                char displayRound[2]; // used to hold the round int in a string
                char wins [] = "Wins: ";
                char round [] = "Round ";
                displayMove[0] = game->playerMoves[i]; // first char of string is the player move
                displayMove[1] = '\0'; // null terminate string
                snprintf(displayScore, sizeof(displayScore), "%d", game->playerScores[i]); //put int in string
                snprintf(displayRound, sizeof(displayRound), "%d", *roundCount + 1); //put int in string

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
                yPos += 8; // increment for next character display
            }


}




void Game_logic(HAL *hal_p, Gamesettings *game)
{ // higher level function that handles the entire game logic
    static int roundsPlayed = 0; // holds round num
    char endMSG [] = "Press BB1 to end "; // if game is over, end the game

    if (roundsPlayed < game->numRounds)// call the round logic if the round is not over
    {
        Round_logic(hal_p, game, &roundsPlayed);
    }
    else
    { // end the round if the button is tapped, display the prompt
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
    int yPosPlayers = 25; //graphic positioning
    int yPosWinners = 70; //graphic positioning
    int i=0;
    char displayScore[2];// holds score int
    int winners[4]; // list of winners
    int winnerIndex; // used to iterate through list of winners
    int highestValue = 0; // used to determine highest score
    int numWinners = 0;

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Game Over", -1, 5, 10,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Winners:", -1, 5, 90,
       true);
    for (i=0; i < game->numPlayers; i++){
        if (game->playerScores[i] > highestValue){ // if player score is higher than current highest val
            highestValue = game->playerScores[i]; // new highest value is the playerscore
            winners[0] = i; // store index of player in the winners list
            numWinners = 1; // one winner
        } else if (game->playerScores[i] == highestValue){ // if two scores are tied for winner
            winners[numWinners] = i; // put the index of the player in the next winnner list spot
            numWinners++; // increase the number of winners
        }
    }

    for (i=0; i < game->numPlayers; i++){ // for all players, display score and name
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
    if (highestValue > 0){
        for (winnerIndex =0; winnerIndex < numWinners; winnerIndex++){ // displays winners from the winner list
               Graphics_drawString(&hal_p->g_sContext,
                                   (int8_t*) game->playerNames[winners[winnerIndex]], -1, 90, yPosWinners ,
                                                            true);
               yPosWinners +=8;
    }
    }
}









void getPlayerInput(HAL *hal_p, bool *currentPlayerTurn, bool *nextPlayerTurn,
                    char *name, Gamesettings *game, int playerNumber,
                    int *playersMoved, bool *MSG)
{
// display messages and array to hold player name
    char pName[32];
    char gSstr1[] = ", please enter:";
    char gSstr2[] = " R or r for Rock";
    char gSstr3[] = " P or p for Paper";
    char gSstr4[] = " S or s for Scissors";
// put player name in string
    strncpy(pName, game->playerNames[playerNumber], sizeof(pName) - 1);
    pName[sizeof(pName) - 1] = '\0';

    if (*currentPlayerTurn) // if its the players turn

    {
        strcat(pName, gSstr1); // attach the name to please enter msg
        if (UART_canSend(&hal_p->uart) && *MSG)
        { // display RPS message once
            UART_sendString(&hal_p->uart, pName);
            UART_sendString(&hal_p->uart, gSstr2);
            UART_sendString(&hal_p->uart, gSstr3);
            UART_sendString(&hal_p->uart, gSstr4);
            *(MSG) = false;
        }

        if (UART_hasChar(&hal_p->uart)) // if there is a char to receive
        {
            char playerMove = UART_getChar(&hal_p->uart); // put the char in the player move
            if (playerMove == 'R' || playerMove == 'r' || playerMove == 'P' //checks if move is valid
                    || playerMove == 'p' || playerMove == 'S'
                    || playerMove == 's')
            {
                game->playerMoves[playerNumber] = playerMove; // put the player move in the game struct
                (*playersMoved)++; // increase the number of players that have moved
                *currentPlayerTurn = false;
                *nextPlayerTurn = true; // move to next player

                if (*playersMoved < game->numPlayers)
                {
                    *(MSG) = true; // display the "please move" msg again
                }
            }
            else{
                UART_sendString(&hal_p->uart, "Please enter 'r, 'p', or 's'");
            }
        }
    }
}

