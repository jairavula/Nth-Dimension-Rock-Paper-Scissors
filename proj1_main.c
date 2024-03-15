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
void Game_logic(HAL *hal_p, Gamesettings *game);

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

    static Gamesettings game = { titleScreen, 3, 2, true, true, true, true,
                                 false, "", "", "", "", true, false, false,
                                 false,'O','O','O','O'};

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
        break;
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
    static int numRounds = 0;
    static int numPlayers = 1;
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

        game->loadSettingsScreen = false;
    }

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                        settingsScreenCursorPos, true);

    if (Button_isTapped(&hal_p->launchpadS2) && settingsScreenCursorPos == 61)
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                            settingsScreenCursorPos, true);
        settingsScreenCursorPos += 15;
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                            settingsScreenCursorPos, true);
    }
    else if (Button_isTapped(&hal_p->launchpadS2)
            && settingsScreenCursorPos == 76)
    {
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) antiCursor, -1, 115,
                            settingsScreenCursorPos, true);
        settingsScreenCursorPos -= 15;
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) cursor, -1, 115,
                            settingsScreenCursorPos, true);
    }
    if (Button_isTapped(&hal_p->boosterpackJS) && settingsScreenCursorPos == 61)
    {
        if (numRounds == 6)
        {
            numRounds = 1;
            sprintf(numRoundsChar, "%d", numRounds);
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                105, settingsScreenCursorPos, true);

        }
        else
        {
            numRounds++;
            sprintf(numRoundsChar, "%d", numRounds);
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) numRoundsChar, -1,
                                105, settingsScreenCursorPos, true);
        }

    }
    else if (Button_isTapped(&hal_p->boosterpackJS)
            && settingsScreenCursorPos == 76)
    {
        if (numPlayers == 4)
        {
            numPlayers = 2;
            sprintf(numPlayersChar, "%d", numPlayers);
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                -1, 105, settingsScreenCursorPos, true);

        }
        else
        {
            numPlayers++;
            sprintf(numPlayersChar, "%d", numPlayers);
            Graphics_drawString(&hal_p->g_sContext, (int8_t*) numPlayersChar,
                                -1, 105, settingsScreenCursorPos, true);
        }

    }
    game->numPlayers = numPlayers;
    game->numRounds = numRounds;
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

void NameSelect_screen(HAL *hal_p, Gamesettings *game)
{
    static int i = 1;
    static int yPosition = 13;
    char nameField[4];
    int xNamePos = 40;
    static int yNamePos = 12;
    static int playersEntered = 0;
    static int nameIndex = 0;
    game->player1Name[3] = '\0';
    game->player2Name[3] = '\0';
    game->player3Name[3] = '\0';
    game->player4Name[3] = '\0';

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

    while (i <= game->numPlayers)
    {
        sprintf(nameField, "%d)", i);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) nameField, -1, 15,
                            yPosition, true);
        i++;
        yPosition += 8;
    }

    if (playersEntered < game->numPlayers)
    {
        if (nameIndex < 3)
        {
            if (yNamePos == 12)
            {
                game->player1Name[nameIndex] = UART_getChar(&hal_p->uart);
                Graphics_drawString(&hal_p->g_sContext,
                                    (int8_t*) game->player1Name, -1, xNamePos,
                                    yNamePos, true);
                if (UART_canSend(&hal_p->uart))
                    UART_sendChar(&hal_p->uart, game->player1Name[nameIndex]);
                nameIndex++;
            }
            if (yNamePos == 20)
            {
                game->player2Name[nameIndex] = UART_getChar(&hal_p->uart);
                Graphics_drawString(&hal_p->g_sContext,
                                    (int8_t*) game->player2Name, -1, xNamePos,
                                    yNamePos, true);
                if (UART_canSend(&hal_p->uart))
                    UART_sendChar(&hal_p->uart, game->player2Name[nameIndex]);
                nameIndex++;
            }
            if (yNamePos == 28)
            {
                game->player3Name[nameIndex] = UART_getChar(&hal_p->uart);
                Graphics_drawString(&hal_p->g_sContext,
                                    (int8_t*) game->player3Name, -1, xNamePos,
                                    yNamePos, true);
                if (UART_canSend(&hal_p->uart))
                    UART_sendChar(&hal_p->uart, game->player3Name[nameIndex]);
                nameIndex++;
            }
            if (yNamePos == 36)
            {
                game->player4Name[nameIndex] = UART_getChar(&hal_p->uart);
                Graphics_drawString(&hal_p->g_sContext,
                                    (int8_t*) game->player4Name, -1, xNamePos,
                                    yNamePos, true);
                if (UART_canSend(&hal_p->uart))
                    UART_sendChar(&hal_p->uart, game->player4Name[nameIndex]);
                nameIndex++;
            }
        }
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

void Game_logic(HAL *hal_p, Gamesettings *game)
{

    int playersMoved = 0;

    char p1Name[32];
    strncpy(p1Name, game->player1Name, sizeof(p1Name) - 1);
    p1Name[sizeof(p1Name) - 1] = '\0';
    char p2Name[32];
    strncpy(p2Name, game->player2Name, sizeof(p2Name) - 1);
    p2Name[sizeof(p2Name) - 1] = '\0';
    char p3Name[32];
    strncpy(p3Name, game->player3Name, sizeof(p3Name) - 1);
    p3Name[sizeof(p3Name) - 1] = '\0';
    char p4Name[32];
    strncpy(p4Name, game->player4Name, sizeof(p4Name) - 1);
    p4Name[sizeof(p4Name) - 1] = '\0';

    char gSstr1[] = ", please enter:";
    char gSstr2[] = " R or r for Rock";
    char gSstr3[] = " P or p for Paper";
    char gSstr4[] = " S or s for Scissors";


if (playersMoved <= game->numPlayers) {
    if (game->player1Turn)
       {
           static bool loadMSGp1 = true;
           strcat(p1Name, gSstr1);
           if (UART_canSend(&hal_p->uart) && loadMSGp1)
           {
               UART_sendString(&hal_p->uart, p1Name);
               UART_sendString(&hal_p->uart, gSstr2);
               UART_sendString(&hal_p->uart, gSstr3);
               UART_sendString(&hal_p->uart, gSstr4);
               loadMSGp1 = false;
           }

           if (UART_hasChar(&hal_p->uart)){
               char player1Move = UART_getChar(&hal_p-> uart);
               if (player1Move == 'R' || player1Move == 'r' || player1Move == 'P' || player1Move == 'p' || player1Move == 'S' || player1Move == 's'){
                   game->player1Move = player1Move;
                   playersMoved ++;
                   game->player1Turn = false;
                   game->player2Turn = true;
               }
           }
       }
       if (game->player2Turn)
          {
              static bool loadMSGp2 = true;
              strcat(p2Name, gSstr1);
              if (UART_canSend(&hal_p->uart) && loadMSGp2)
              {
                  UART_sendString(&hal_p->uart, p2Name);
                  UART_sendString(&hal_p->uart, gSstr2);
                  UART_sendString(&hal_p->uart, gSstr3);
                  UART_sendString(&hal_p->uart, gSstr4);
                  loadMSGp2 = false;
              }

              if (UART_hasChar(&hal_p->uart)){
                  char player2Move = UART_getChar(&hal_p-> uart);
                  if (player2Move == 'R' || player2Move == 'r' || player2Move == 'P' || player2Move == 'p' || player2Move == 'S' || player2Move == 's'){
                      game->player2Move = player2Move;
                      playersMoved ++;
                      game->player2Turn = false;
                      game->player3Turn = true;
                  }
              }
          }
       if (game->player3Turn)
           {
               static bool loadMSGp3 = true;
               strcat(p3Name, gSstr1);
               if (UART_canSend(&hal_p->uart) && loadMSGp3)
               {
                   UART_sendString(&hal_p->uart, p3Name);
                   UART_sendString(&hal_p->uart, gSstr2);
                   UART_sendString(&hal_p->uart, gSstr3);
                   UART_sendString(&hal_p->uart, gSstr4);
                   loadMSGp3 = false;
               }

               if (UART_hasChar(&hal_p->uart)){
                   char player3Move = UART_getChar(&hal_p-> uart);
                   if (player3Move == 'R' || player3Move == 'r' || player3Move == 'P' || player3Move == 'p' || player3Move == 'S' || player3Move == 's'){
                       game->player3Move = player3Move;
                       playersMoved ++;
                       game->player3Turn = false;
                       game->player4Turn = true;
                   }
               }
           }
       if (game->player4Turn)
             {
                 static bool loadMSGp4 = true;
                 strcat(p4Name, gSstr1);
                 if (UART_canSend(&hal_p->uart) && loadMSGp4)
                 {
                     UART_sendString(&hal_p->uart, p4Name);
                     UART_sendString(&hal_p->uart, gSstr2);
                     UART_sendString(&hal_p->uart, gSstr3);
                     UART_sendString(&hal_p->uart, gSstr4);
                     loadMSGp4 = false;
                 }

                 if (UART_hasChar(&hal_p->uart)){
                     char player4Move = UART_getChar(&hal_p-> uart);
                     if (player4Move == 'R' || player4Move == 'r' || player4Move == 'P' || player4Move == 'p' || player4Move == 'S' || player4Move == 's'){
                         game->player4Move = player4Move;
                         playersMoved ++;
                         game->player4Turn = false;
                     }
                 }
             }

} else {
    if (UART_canSend(&hal_p->uart))
                     {
                         UART_sendString(&hal_p->uart, game->player1Name);
                         UART_sendString(&hal_p->uart, gSstr2);
                         UART_sendString(&hal_p->uart, gSstr3);
                         UART_sendString(&hal_p->uart, gSstr4);
}

}
}

