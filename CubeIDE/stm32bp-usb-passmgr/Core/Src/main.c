/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>
#include "usbd_customhid.h"
#include "usb_hid_keys.h"
#include "OLED_SSD1306.h"
#include "GFX_BW.h"
#include "fonts/fonts.h"
#include "keystructures.h"
#include "database.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VERSION_STRING "1.05"

#define BTN_PRESSED GPIO_PIN_RESET		// Buttons are active-low
#define BTN_RELEASED GPIO_PIN_SET

#define BTN0_GPIO GPIOA
#define BTN0_PIN GPIO_PIN_0

#define BTN1_GPIO GPIOB
#define BTN1_PIN GPIO_PIN_5

#define BTN2_GPIO GPIOA
#define BTN2_PIN GPIO_PIN_7

#define LED_ON GPIO_PIN_RESET			// LED is active-low
#define LED_OFF GPIO_PIN_SET

#define LED_GPIO GPIOC
#define LED_PIN GPIO_PIN_13

#define BUTTONS 3						// Number of onboard buttons

#define BUTTON_DEBOUNCE_COUNTER 3		// Debounce conter cycles
#define BUTTON_HOLD_COUNTER 50			// Hold detect conter cycles

#define KEYPRESS_EVERY_N_CYCLES 8		// Keypress every n cycles
#define KEYPRESS_RELEASE_AFTER_CYCLES 2	// Keypress every n cycles

#define NUMBER_OF_LCD_LINES 4

#define TYPE_SEQUENCE_OFF -1			// Disable typing sequence
#define TYPE_SEQUENCE_BEGIN 0			// Begin typing sequence

#define DISPLAY_CONTEXT_MAIN 0x00		// Display context password slots
#define DISPLAY_CONTEXT_SLOTEDIT 0x01	// Display context edit password slot
#define DISPLAY_CONTEXT_TEXTEDIT 0x02	// Display context edit text line
#define DISPLAY_CONTEXT_DELCONFIRM 0x03	// Display context confirm delete
#define DISPLAY_CONTEXT_MESSAGE 0x04	// Display context display message
#define DISPLAY_CONTEXT_PINSET 0x05		// Display context display message
#define DISPLAY_CONTEXT_PINENTER 0x06	// Display context display message
#define DISPLAY_CONTEXT_HLTMSG 0x07 	// Display content halt message

#define EDIT_LOT_ROLE_NAME 0x01
#define EDIT_LOT_ROLE_USERNAME 0x02
#define EDIT_LOT_ROLE_PASSWORD 0x03

#define EDIT_OPTION_NEWPASS 0x00		// Opcja edycji - Nowe hasło
#define EDIT_OPTION_SAVE 0x01			// Opcja edycji - Zapisz
#define EDIT_OPTION_DELETE 0x02			// Opcja edycji - Zapisz
#define EDIT_OPTION_NAME 0x03			// Opcja edycji - Nazwa
#define EDIT_OPTION_USERNAME 0x04		// Opcja edycji - Użytkownik
#define EDIT_OPTION_PASSWORD 0x05		// Opcja edycji - Hasło

#define MAX_LINE_OF_TEXT_CHARS 20

#define MSG_DISPLAY_CYCLES 150
#define VERSION_DISPLAY_CYCLES 70
#define DBMSG_DISPLAY_CYCLES 20
#define SAVE_ALL_SLOTS_ON_CYCLE 10

#define LCD_DIM_COUNTER_START 6000
#define DISPLAY_ENABLED_COUNTER_START 30000

#define LCD_DIM_BRIGHT_VALUE 0x50
#define LCD_DIM_DARK_VALUE 0x00

#define LCD_BRIGHTNESS_IS_DIM 0x01
#define LCD_BRIGHTNESS_GO_DIM 0x02
#define LCD_BRIGHTNESS_GO_BRIGHT 0x04

#define PIN_ENTER_CHAR_UNKNOWN '#'
#define PIN_ENTER_CHAR_UPARROW '^'
#define PIN_ENTER_CHAR_ONE '1'
#define PIN_ENTER_CHAR_ZERO '0'

#define MAX_FAILED_LOGIN 3

#define OPT_FAILED_LOGINS 0

#define DELAYED_LOGIN_COUNTER 10

void welcomeMsg(void);
void longPress(int);
void shortPress(int);
void loadAndDisplaySlots(void);
void loadSlotsFromFlash(void);
void displayLoadedSlots(void);
void cleanAndSaveDB(void);
bool saveAllSlots(void);
char alphabetGetNextChar(char);
char alphabetGetPreviousChar(char);
void sanitizeLineOfText(char*, int);
void randomizePassword(char*, int);
void loadEncryptedDatabase(void);
void scrambleEmptyData(char*, int);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

extern USBD_HandleTypeDef hUsbDeviceFS;

struct ButtonState {
	bool isPressed;
	bool isHolded;
	bool awaitingPressAction;
	bool awaitingReleaseAction;
	bool awaitingLongAction;
	int stateCounter;
	int holdCounter;
	GPIO_TypeDef *GPIO;
	uint8_t PIN;
};

struct ButtonState buttonState[3] = {0};

char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
				   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
				   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '=', '_', '+',
				   '.', 0x00};

int letterDisplay = sizeof(alphabet)-1;

const int releaseKey = 0;
const int userBegin = 1;
const int passwordBegin = 12;

int letterToType = 0;
int keyPressed = 0;

const int longPressCycles = 35;

uint8_t capslockState = 0;

uint8_t usbConnected = 0;

char slotDisplayLines[4][26] = {{0}, {0}, {0}, {0}};
char slotEditLines[4][26] = {{0}, {0}, {0}, {0}};
bool slotsWithLabel[4] = { false, false, false, false };
static char *slotEditNewPassBtn = "[Nowe]";
static char *slotEditSaveBtn = "[Zapisz]";
static char *slotEditDeleteBtn = "[Usun]";
static char *slotEditOKBtn = "[OK]";
static char *slotEditNameLabel = "N:";
static char *slotEditUsernameLabel = "U:";
static char *slotEditPasswordLabel = "H:";
int editOption = 0;

static char *slotEditLOTLabelName = "Nazwa:";
static char *slotEditLOTLabelUsername = "Uzytkownik:";
static char *slotEditLOTLabelPassword = "Haslo:";

static char *pinEditLabel = "Ustaw PIN:";
static char *pinEnterLabel = "Podaj PIN:";

char lcdMessage[16] = {0};

char str[20] = {0};

PasswordSlot passwordSlots[NUMBER_OF_SLOTS] = {0};
PasswordSlot currentEditPasswordSlot;
volatile bool editPasswordWasChanged = false;
bool slotValid[NUMBER_OF_SLOTS + 1] = { false };
char validSignature[4] = { 'E', 'O', 'D', 'B' };
uint8_t dbSignature[4] = {0};

static char *slotInvalidName = "[ DODAJ SLOT ]";
static char *slotSetPinName = "[ USTAW PIN ]";
static char *slotLabels[NUMBER_OF_SLOTS + 1] = {"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.",
											    "A.", "B.", "C.", "D.", "E.", "F.", "G.", "H.", "I.",
											    "J.", "K.", "L.", "M.", "N.", "O.", "P.", "Q.", "R.",
											    "S.", "T.", "U.", "W.", "X.", "Y.", "Z.", ".."};

static char *slotConfirmDeleteLabel = "[YARLY!]";

const char* lcdMessageTyping = "PISZE...";
const char* lcdMessageSaving = "Zapisuje...";
const char* lcdMessageDeleting = "Usuwam...";
const char* lcdMessageSaveError = "BLAD!";
const char* lcdMessageSaveOK = "OK!";

const char* lcdDelConfirmMessage = "ORLY???";

const char* lcdMessageLocked = "Blokada";
const char* lcdMessageFailedPin = "Zly PIN!";

const char* lcdMessageVersion = VERSION_STRING;

typedef struct EditedLineOfText {
	char line[25];
	char name[25];
	int lineRole;
	int editedChar;
} EditedLineOfText;

static char emptyChar = '~';
static char upArrowChar = '^';

int okButtonIdx = 0;

EditedLineOfText editedLineOfText = { {0}, {0}, 0, 0 };

volatile int selectedSlot = 0;

typedef struct TypeSequence {
	int lUsername;
	int lPassword;
	int lPreviousPassword;
	bool withTerminations;
} TypeSequence;

TypeSequence typeSequence = {-1, -1};

volatile uint8_t displayContext = DISPLAY_CONTEXT_MAIN;

volatile int messageDisplayCounter = 0;

uint8_t randomSeed[MAX_LINE_OF_TEXT_CHARS * 4] = {0};
volatile uint8_t currentSeedPart = 0;

volatile int lcdDimCounter = 0;

volatile uint8_t lcdBrightnessStatus = 0x00;

volatile int displayEnabledCounter = 0;

uint8_t displaySlotsFirst = {0};
uint8_t displaySlotsLast = {0};

uint8_t xorPinKey = 0x00;

volatile uint8_t pinEntered = 0x00;
volatile uint8_t pinEnterPos = 0;

uint8_t optBuffer[4];

volatile int delayedLogin = 0x00;

volatile bool btnCancelNextAction = false;

uint8_t databaseMode = PM_DATABASE_MODE_MAIN;
volatile uint8_t saveDatabasePending = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void welcomeMsg(void) {
	displayContext = DISPLAY_CONTEXT_MESSAGE;
	if (databaseMode == PM_DATABASE_MODE_BACKUP) {
		sprintf(lcdMessage, "baza zapas.");
	} else {
		sprintf(lcdMessage, "wersja %s", lcdMessageVersion);
	}
	messageDisplayCounter = VERSION_DISPLAY_CYCLES;
}

// Procedures executed on buttons long press
void longPress(int button) {

	if (btnCancelNextAction) {
		btnCancelNextAction = false;
		return;
	}

	switch (displayContext)
	{
		case DISPLAY_CONTEXT_MAIN:
			switch (button) {
				case 0:	// BUTTON 0 LONG PRESS
					if ((typeSequence.lUsername == TYPE_SEQUENCE_OFF) && (typeSequence.lPassword == TYPE_SEQUENCE_OFF) && (typeSequence.lPreviousPassword == TYPE_SEQUENCE_OFF) && slotValid[selectedSlot]) {
						typeSequence.lUsername = TYPE_SEQUENCE_OFF;
						typeSequence.lPassword = TYPE_SEQUENCE_BEGIN;
						typeSequence.lPreviousPassword = TYPE_SEQUENCE_OFF;
						typeSequence.withTerminations = false;
						keyPressed = 0;
						//Display typing message
						strcpy(lcdMessage, lcdMessageTyping);
						displayContext = DISPLAY_CONTEXT_MESSAGE;
					}
					break;
				case 1: // BUTTON 1 LONG PRESS
					if (selectedSlot > displaySlotsLast) {
						pinEntered = 0x00;
						pinEnterPos = 0x00;
						displayContext = DISPLAY_CONTEXT_PINSET;
					}
					else
					{
						if (slotValid[selectedSlot]) {
							strncpy(currentEditPasswordSlot.name, passwordSlots[selectedSlot].name, 25);
							strncpy(currentEditPasswordSlot.username, passwordSlots[selectedSlot].username, 25);
							strncpy(currentEditPasswordSlot.password, passwordSlots[selectedSlot].password, 25);
							strncpy(currentEditPasswordSlot.previouspassword, passwordSlots[selectedSlot].previouspassword, 25);
							sanitizeLineOfText(currentEditPasswordSlot.name, sizeof(currentEditPasswordSlot.name));
							sanitizeLineOfText(currentEditPasswordSlot.username, sizeof(currentEditPasswordSlot.username));
							sanitizeLineOfText(currentEditPasswordSlot.password, sizeof(currentEditPasswordSlot.password));
							sanitizeLineOfText(currentEditPasswordSlot.previouspassword, sizeof(currentEditPasswordSlot.previouspassword));
						} else {
							strcpy(currentEditPasswordSlot.name, "");
							strcpy(currentEditPasswordSlot.username, "");
							strcpy(currentEditPasswordSlot.password, "");
							strcpy(currentEditPasswordSlot.previouspassword, "");
						}
						displayContext = DISPLAY_CONTEXT_SLOTEDIT;
						editOption = EDIT_OPTION_NEWPASS;
						editPasswordWasChanged = false;
					}
					break;
				case 2: // BUTTON 2 LONG PRESS
					if ((typeSequence.lUsername == TYPE_SEQUENCE_OFF) && (typeSequence.lPassword == TYPE_SEQUENCE_OFF) && (typeSequence.lPreviousPassword == TYPE_SEQUENCE_OFF) && slotValid[selectedSlot]) {
						typeSequence.lUsername = TYPE_SEQUENCE_OFF;
						typeSequence.lPassword = TYPE_SEQUENCE_OFF;
						typeSequence.lPreviousPassword = TYPE_SEQUENCE_BEGIN;
						typeSequence.withTerminations = false;
						keyPressed = 0;
						//Display typing message
						strcpy(lcdMessage, lcdMessageTyping);
						displayContext = DISPLAY_CONTEXT_MESSAGE;
					}
					break;
			}
			break;

		case DISPLAY_CONTEXT_SLOTEDIT:
			switch (button) {
				case 0:	// BUTTON 0 LONG PRESS
					if ((editOption == EDIT_OPTION_SAVE) || (editOption == EDIT_OPTION_NEWPASS) || (editOption == EDIT_OPTION_DELETE)) {
						sanitizeLineOfText(currentEditPasswordSlot.name, sizeof(currentEditPasswordSlot.name));
						sanitizeLineOfText(currentEditPasswordSlot.username, sizeof(currentEditPasswordSlot.username));
						sanitizeLineOfText(currentEditPasswordSlot.password, sizeof(currentEditPasswordSlot.password));
						sanitizeLineOfText(currentEditPasswordSlot.previouspassword, sizeof(currentEditPasswordSlot.previouspassword));

						switch (editOption)
						{
							case EDIT_OPTION_SAVE:
								strncpy(passwordSlots[selectedSlot].name, currentEditPasswordSlot.name, sizeof(currentEditPasswordSlot.name));
								strncpy(passwordSlots[selectedSlot].username, currentEditPasswordSlot.username,  sizeof(currentEditPasswordSlot.username));
								if (editPasswordWasChanged)
								{
									strncpy(passwordSlots[selectedSlot].previouspassword, passwordSlots[selectedSlot].password,  sizeof(passwordSlots[selectedSlot].password));
									sanitizeLineOfText(passwordSlots[selectedSlot].previouspassword, sizeof(passwordSlots[selectedSlot].previouspassword));
								}
								strncpy(passwordSlots[selectedSlot].password, currentEditPasswordSlot.password,  sizeof(currentEditPasswordSlot.password));
								slotValid[selectedSlot] = true;
								saveDatabasePending = true;
								strcpy(lcdMessage, lcdMessageSaving);
								displayContext = DISPLAY_CONTEXT_MESSAGE;
								messageDisplayCounter = DBMSG_DISPLAY_CYCLES;
								editPasswordWasChanged = false;
								break;

							case EDIT_OPTION_NEWPASS:
								randomizePassword(currentEditPasswordSlot.password, MAX_LINE_OF_TEXT_CHARS);
								sanitizeLineOfText(currentEditPasswordSlot.password, sizeof(currentEditPasswordSlot.password));
								editPasswordWasChanged = true;
								break;

							case EDIT_OPTION_DELETE:
								if (slotValid[selectedSlot]) {
									strcpy(lcdMessage, lcdDelConfirmMessage);
									displayContext = DISPLAY_CONTEXT_DELCONFIRM;
								}
								break;
						}
					}
					break;
				case 1: // BUTTON 1 LONG PRESS
					if (editOption == EDIT_OPTION_NAME || editOption == EDIT_OPTION_USERNAME || editOption == EDIT_OPTION_PASSWORD) {
						switch (editOption)
						{
							case EDIT_OPTION_NAME:
								strcpy(editedLineOfText.name, slotEditLOTLabelName);
								editedLineOfText.lineRole = EDIT_LOT_ROLE_NAME;
								strcpy(editedLineOfText.line, currentEditPasswordSlot.name);
								break;
							case EDIT_OPTION_USERNAME:
								strcpy(editedLineOfText.name, slotEditLOTLabelUsername);
								editedLineOfText.lineRole = EDIT_LOT_ROLE_USERNAME;
								strcpy(editedLineOfText.line, currentEditPasswordSlot.username);
								break;
							case EDIT_OPTION_PASSWORD:
								strcpy(editedLineOfText.name, slotEditLOTLabelPassword);
								editedLineOfText.lineRole = EDIT_LOT_ROLE_PASSWORD;
								strcpy(editedLineOfText.line, currentEditPasswordSlot.password);
								break;

						}
						sanitizeLineOfText(editedLineOfText.line, sizeof(editedLineOfText.line));
						editedLineOfText.editedChar = 0;
						displayContext = DISPLAY_CONTEXT_TEXTEDIT;

					}
					break;
				case 2: // BUTTON 2 LONG PRESS
					displayContext = DISPLAY_CONTEXT_MAIN;
					editPasswordWasChanged = false;
					break;
			}
			break;

		case DISPLAY_CONTEXT_TEXTEDIT:
			switch (button) {
				case 0:	// BUTTON 0 LONG PRESS
					okButtonIdx = (strlen(editedLineOfText.line)+1 < MAX_LINE_OF_TEXT_CHARS)?strlen(editedLineOfText.line)+1:MAX_LINE_OF_TEXT_CHARS;

					if (editedLineOfText.editedChar < okButtonIdx)
					{
						if (--editedLineOfText.editedChar < 0) editedLineOfText.editedChar = okButtonIdx;
					} else {
						sanitizeLineOfText(editedLineOfText.line, sizeof(editedLineOfText.line));
						switch (editedLineOfText.lineRole)
						{
							case EDIT_LOT_ROLE_NAME:
								strcpy(currentEditPasswordSlot.name, editedLineOfText.line);
								break;
							case EDIT_LOT_ROLE_USERNAME:
								strcpy(currentEditPasswordSlot.username, editedLineOfText.line);
								break;
							case EDIT_LOT_ROLE_PASSWORD:
								strcpy(currentEditPasswordSlot.password, editedLineOfText.line);
								editPasswordWasChanged = true;
								break;
						}
						displayContext = DISPLAY_CONTEXT_SLOTEDIT;
						//editOption = EDIT_OPTION_SAVE;
					}
					break;
				case 1:	// BUTTON 1 LONG PRESS
					okButtonIdx = (strlen(editedLineOfText.line)+1 < MAX_LINE_OF_TEXT_CHARS)?strlen(editedLineOfText.line)+1:MAX_LINE_OF_TEXT_CHARS;

					editedLineOfText.editedChar = okButtonIdx;
					break;
				case 2:	// BUTTON 2 LONG PRESS
					displayContext = DISPLAY_CONTEXT_SLOTEDIT;
					break;
			}
			break;

		case DISPLAY_CONTEXT_DELCONFIRM:
			switch (button) {

				case 0:	// BUTTON 0 LONG PRESS
					memset(&(currentEditPasswordSlot.name), 0x00, sizeof(currentEditPasswordSlot.name));
					memset(&(currentEditPasswordSlot.username), 0x00, sizeof(currentEditPasswordSlot.username));
					memset(&(currentEditPasswordSlot.password), 0x00, sizeof(currentEditPasswordSlot.password));
					memset(&(currentEditPasswordSlot.previouspassword), 0x00, sizeof(currentEditPasswordSlot.previouspassword));
					memset(&(currentEditPasswordSlot.checksum), 0x00, sizeof(currentEditPasswordSlot.checksum));
					slotValid[selectedSlot] = false;
					selectedSlot = 0;
					displaySlotsFirst = 0;
					saveDatabasePending = true;
					strcpy(lcdMessage, lcdMessageDeleting);
					displayContext = DISPLAY_CONTEXT_MESSAGE;
					messageDisplayCounter = DBMSG_DISPLAY_CYCLES;
					editPasswordWasChanged = false;
					break;

				case 1:
				case 2:
					displayContext = DISPLAY_CONTEXT_SLOTEDIT;
					break;

			}
			break;

		case DISPLAY_CONTEXT_PINSET:

			switch (button) {
				case 0:
					if (pinEnterPos > 7) {
						xorPinKey = pinEntered;
						pinEntered = 0x00;
						pinEnterPos = 0x00;
						saveDatabasePending = true;
						strcpy(lcdMessage, lcdMessageSaving);
						displayContext = DISPLAY_CONTEXT_MESSAGE;
						messageDisplayCounter = DBMSG_DISPLAY_CYCLES;
					}
					break;
				case 2:
					pinEntered = 0x00;
					pinEnterPos = 0x00;
					displayContext = DISPLAY_CONTEXT_MAIN;
					break;

			}
		break;

		case DISPLAY_CONTEXT_PINENTER:

			switch (button) {

				case 0:
					if (delayedLogin == 0) {
						if (pinEnterPos > 0) pinEnterPos--;
					}
				break;

				case 1:
					if (delayedLogin == 0) {
						pinEntered |= (0x01 << (7 - pinEnterPos));
						if ((++pinEnterPos) > 7)
						{
							xorPinKey = pinEntered;
							delayedLogin = DELAYED_LOGIN_COUNTER;
						}
					}
				break;

				case 2:
					if (delayedLogin == 0) {
						pinEntered &= ~(0x01 << (7 - pinEnterPos));
						if ((++pinEnterPos) > 7)
						{
							xorPinKey = pinEntered;
							delayedLogin = DELAYED_LOGIN_COUNTER;
						}
					}
				break;

			}
		break;

	}
}

// Procedures executed on buttons short press
void shortPress(int button) {

	if (btnCancelNextAction) {
		btnCancelNextAction = false;
		return;
	}

	switch (displayContext)
	{
		case DISPLAY_CONTEXT_MAIN:
			switch (button) {
				case 0: // BUTTON 0 SHORT PRESS
					if ((typeSequence.lUsername == TYPE_SEQUENCE_OFF) && (typeSequence.lPassword == TYPE_SEQUENCE_OFF) && (typeSequence.lPreviousPassword == TYPE_SEQUENCE_OFF) && slotValid[selectedSlot]) {
						typeSequence.lUsername = TYPE_SEQUENCE_BEGIN;
						typeSequence.lPassword = TYPE_SEQUENCE_BEGIN;
						typeSequence.lPreviousPassword = TYPE_SEQUENCE_OFF;
						typeSequence.withTerminations = true;
						keyPressed = 0;
						//Display typing message
						strcpy(lcdMessage, lcdMessageTyping);
						displayContext = DISPLAY_CONTEXT_MESSAGE;
					}
					break;
				case 1: // BUTTON 1 SHORT PRESS
					selectedSlot++;
					if (selectedSlot > displaySlotsLast + 1) selectedSlot = 0;
					if (selectedSlot > displaySlotsFirst + NUMBER_OF_LCD_LINES - 1)
					{
						displaySlotsFirst = selectedSlot - NUMBER_OF_LCD_LINES + 1;
						displayLoadedSlots();
					}
					else if (selectedSlot < displaySlotsFirst)
					{
						displaySlotsFirst = selectedSlot;
						displayLoadedSlots();
					}
					break;
				case 2: // BUTTON 2 SHORT PRESS
					selectedSlot--;
					if (selectedSlot < 0) selectedSlot = displaySlotsLast + 1;
					if (selectedSlot > displaySlotsFirst + NUMBER_OF_LCD_LINES - 1)
					{
						displaySlotsFirst = selectedSlot - NUMBER_OF_LCD_LINES + 1;
						displayLoadedSlots();
					}
					else if (selectedSlot < displaySlotsFirst)
					{
						displaySlotsFirst = selectedSlot;
						displayLoadedSlots();
					}
					break;
			}
			break;

		case DISPLAY_CONTEXT_SLOTEDIT:
			switch (button) {
				case 0:	// BUTTON 0 SHORT PRESS
					break;
				case 1: // BUTTON 1 SHORT PRESS
					editOption++;
					if (editOption > 5) editOption = 0;
					break;
				case 2: // BUTTON 2 SHORT PRESS
					editOption--;
					if (editOption < 0) editOption = 5;
					break;
				}
				break;
			break;

		case DISPLAY_CONTEXT_TEXTEDIT:
			switch (button) {
				case 0:	// BUTTON 0 SHORT PRESS
					okButtonIdx = (strlen(editedLineOfText.line) + 1 < MAX_LINE_OF_TEXT_CHARS)?strlen(editedLineOfText.line) + 1:MAX_LINE_OF_TEXT_CHARS;
					if (++editedLineOfText.editedChar > okButtonIdx) editedLineOfText.editedChar = 0;
					break;
				case 1:	// BUTTON 0 SHORT PRESS
					if (editedLineOfText.editedChar < MAX_LINE_OF_TEXT_CHARS)
					{
						editedLineOfText.line[editedLineOfText.editedChar] = alphabetGetNextChar(editedLineOfText.line[editedLineOfText.editedChar]);
						sanitizeLineOfText(editedLineOfText.line, sizeof(editedLineOfText.line));
					}
					break;
				case 2:	// BUTTON 0 SHORT PRESS
					if (editedLineOfText.editedChar < MAX_LINE_OF_TEXT_CHARS)
					{
						editedLineOfText.line[editedLineOfText.editedChar] = alphabetGetPreviousChar(editedLineOfText.line[editedLineOfText.editedChar]);
						sanitizeLineOfText(editedLineOfText.line, sizeof(editedLineOfText.line));
					}
					break;
			}
		break;

		case DISPLAY_CONTEXT_DELCONFIRM:

			displayContext = DISPLAY_CONTEXT_SLOTEDIT;

			break;

		case DISPLAY_CONTEXT_PINSET:

			switch (button) {

				case 0:
					if (pinEnterPos > 0) pinEnterPos--;
					break;

				case 1:
					if (pinEnterPos < 8)
					{
						pinEntered |= (0x01 << (7 - pinEnterPos));
						pinEnterPos++;
					}
					break;

				case 2:
					if (pinEnterPos < 8)
					{
						pinEntered &= ~(0x01 << (7 - pinEnterPos));
						pinEnterPos++;
					}
					break;

				}
			break;

		case DISPLAY_CONTEXT_PINENTER:

			switch (button) {

				case 0:
					if (delayedLogin == 0) {
						if (pinEnterPos > 0) pinEnterPos--;
					}
					break;

				case 1:
					if (delayedLogin == 0) {
						pinEntered |= (0x01 << (7 - pinEnterPos));
						if ((++pinEnterPos) > 7)
						{
							xorPinKey = pinEntered;
							delayedLogin = DELAYED_LOGIN_COUNTER;
						}
					}
					break;

				case 2:
					if (delayedLogin == 0) {
						pinEntered &= ~(0x01 << (7 - pinEnterPos));
						if ((++pinEnterPos) > 7)
						{
							xorPinKey = pinEntered;
							delayedLogin = DELAYED_LOGIN_COUNTER;
						}
					}
					break;

				}
			break;

	}
}

void loadEncryptedDatabase(void) {
	GetDBSignature(dbSignature, NUMBER_OF_SLOTS, xorPinKey, databaseMode);

	uint32_t *tmpSignature = (uint32_t*)dbSignature;
	if (*tmpSignature == *((uint32_t*)validSignature))
	{
		loadAndDisplaySlots();
		if (optBuffer[OPT_FAILED_LOGINS] > 0)
		{
			optBuffer[OPT_FAILED_LOGINS] = 0;
			SaveOptions(optBuffer);
		}
		//displayContext = DISPLAY_CONTEXT_MAIN;
		welcomeMsg();
		selectedSlot = 0;
	}
	else
    {
		optBuffer[OPT_FAILED_LOGINS]++;
		SaveOptions(optBuffer);
		strcpy(lcdMessage, lcdMessageFailedPin);
		displayContext = DISPLAY_CONTEXT_HLTMSG;
	}
}

void loadSlotsFromFlash(void) {
	displaySlotsLast = 0;
	for (int i = 0; i < NUMBER_OF_SLOTS; i++) {
		slotValid[i] = GetPasswordSlot(&passwordSlots[i], i, xorPinKey, databaseMode);
		if ((slotValid[i]) && (i < NUMBER_OF_SLOTS - 1)) {
			displaySlotsLast = i + 1;
		}
		if (!slotValid[i])
		{
			memset(passwordSlots[i].name, 0x00, sizeof(passwordSlots[i].name));
			memset(passwordSlots[i].username, 0x00, sizeof(passwordSlots[i].username));
			memset(passwordSlots[i].password, 0x00, sizeof(passwordSlots[i].password));
			memset(passwordSlots[i].previouspassword, 0x00, sizeof(passwordSlots[i].previouspassword));
		}
		sanitizeLineOfText(passwordSlots[i].name, sizeof(passwordSlots[i].name));
		sanitizeLineOfText(passwordSlots[i].username, sizeof(passwordSlots[i].username));
		sanitizeLineOfText(passwordSlots[i].password, sizeof(passwordSlots[i].password));
		sanitizeLineOfText(passwordSlots[i].previouspassword, sizeof(passwordSlots[i].previouspassword));
	}
}

void displayLoadedSlots(void) {
	for (int i = 0; i < NUMBER_OF_LCD_LINES; i++) {
		memset(slotDisplayLines[i], 0x00, sizeof(slotDisplayLines[i]));
		slotsWithLabel[i] = false;
	}
	for (int i = 0; i < NUMBER_OF_LCD_LINES; i++) {
		if ((i+displaySlotsFirst) > displaySlotsLast)
		{
			slotsWithLabel[i] = false;
			strcpy(slotDisplayLines[i], slotSetPinName);
			break;
		}
		else
		{
			if (slotValid[i+displaySlotsFirst]) {
				strncpy(slotDisplayLines[i], passwordSlots[i+displaySlotsFirst].name, 23);	// IF checksum valid display name
				slotsWithLabel[i] = true;
			}
			else {
				strcpy(slotDisplayLines[i], slotInvalidName);
				slotsWithLabel[i] = false;
			}
		}
	}
}

void loadAndDisplaySlots(void) {
	loadSlotsFromFlash();
	displayLoadedSlots();
}

void scrambleEmptyData(char *dataBuffer, int length) {

	bool validData = true;

	for (int i = 0; i < length; i++) {
		uint32_t *currentSeed = (uint32_t*)&randomSeed[(i % sizeof(randomSeed))];
		srand(*currentSeed);
		if (!validData) {
			dataBuffer[i] = rand() % 0xFF;
		} else if (dataBuffer[i] == 0x00) validData = false;
	}

}

bool saveAllSlots(void) {
	uint8_t passwordSlotsToWrite[sizeof(PasswordSlot) * NUMBER_OF_SLOTS + 4];

	int validSlots = 0;
	for (int i = 0; i < NUMBER_OF_SLOTS; i++) {
		if (slotValid[i])
		{
			sanitizeLineOfText(passwordSlots[i].name, sizeof(passwordSlots[i].name));
			sanitizeLineOfText(passwordSlots[i].username, sizeof(passwordSlots[i].username));
			sanitizeLineOfText(passwordSlots[i].password, sizeof(passwordSlots[i].password));
			sanitizeLineOfText(passwordSlots[i].previouspassword, sizeof(passwordSlots[i].previouspassword));
			PasswordSlot *tmpSlot = (PasswordSlot*)&passwordSlotsToWrite[sizeof(PasswordSlot) * validSlots];

			strncpy(tmpSlot->name, passwordSlots[i].name, sizeof(tmpSlot->name));
			strncpy(tmpSlot->username, passwordSlots[i].username, sizeof(tmpSlot->username));
			strncpy(tmpSlot->password, passwordSlots[i].password, sizeof(tmpSlot->password));
			strncpy(tmpSlot->previouspassword, passwordSlots[i].previouspassword, sizeof(tmpSlot->previouspassword));

			scrambleEmptyData(tmpSlot->name, sizeof(tmpSlot->name));
			scrambleEmptyData(tmpSlot->username, sizeof(tmpSlot->username));
			scrambleEmptyData(tmpSlot->password, sizeof(tmpSlot->password));
			scrambleEmptyData(tmpSlot->previouspassword, sizeof(tmpSlot->previouspassword));

			SlotAddChecksum(tmpSlot);
			validSlots++;
		}
	}
	for (int i = validSlots; i < NUMBER_OF_SLOTS; i++) {
		PasswordSlot *tmpSlot = (PasswordSlot*)&passwordSlotsToWrite[sizeof(PasswordSlot) * i];

		memset(tmpSlot->name, 0x00, sizeof(tmpSlot->name));
		memset(tmpSlot->username, 0x00, sizeof(tmpSlot->username));
		memset(tmpSlot->password, 0x00, sizeof(tmpSlot->password));
		memset(tmpSlot->previouspassword, 0x00, sizeof(tmpSlot->previouspassword));
		memset(&(tmpSlot->checksum), 0x00, sizeof(tmpSlot->checksum));

		scrambleEmptyData(tmpSlot->name, sizeof(tmpSlot->name));
		scrambleEmptyData(tmpSlot->username, sizeof(tmpSlot->username));
		scrambleEmptyData(tmpSlot->password, sizeof(tmpSlot->password));
		scrambleEmptyData(tmpSlot->previouspassword, sizeof(tmpSlot->previouspassword));
	}

	for (int i = 0; i < 4; i++)
	{
		passwordSlotsToWrite[sizeof(PasswordSlot) * NUMBER_OF_SLOTS + i] = validSignature[i];
	}

	// ENCRYPTION
	for (int i = 0; i < sizeof(passwordSlotsToWrite); i++) passwordSlotsToWrite[i] ^= xorPinKey;

	return WritePasswordSlots((uint8_t*)&passwordSlotsToWrite, NUMBER_OF_SLOTS);
}

void cleanAndSaveDB(void) {
	xorPinKey = 0x00;

	uint8_t passwordSlotsToWrite[sizeof(PasswordSlot) * NUMBER_OF_SLOTS + 4];

	for (int i = 0; i < NUMBER_OF_SLOTS; i++) {
		PasswordSlot *tmpSlot = (PasswordSlot*)&passwordSlotsToWrite[sizeof(PasswordSlot) * i];

		memset(tmpSlot->name, 0x00, sizeof(tmpSlot->name));
		memset(tmpSlot->username, 0x00, sizeof(tmpSlot->username));
		memset(tmpSlot->password, 0x00, sizeof(tmpSlot->password));
		memset(tmpSlot->previouspassword, 0x00, sizeof(tmpSlot->previouspassword));

		memset(&(tmpSlot->checksum), 0x00, sizeof(tmpSlot->checksum));
	}

	for (int i = 0; i < 4; i++)
	{
		passwordSlotsToWrite[sizeof(PasswordSlot) * NUMBER_OF_SLOTS + i] = validSignature[i];
	}

	for (int i = 0; i < sizeof(passwordSlotsToWrite); i++) passwordSlotsToWrite[i] ^= xorPinKey;

	WritePasswordSlots((uint8_t*)&passwordSlotsToWrite, NUMBER_OF_SLOTS);
}

char alphabetGetNextChar(char currentChar) {
	int nextCharIdx;
	for (int i = 0; i < sizeof(alphabet); i++)
	{
		if (currentChar == alphabet[i]) {
			nextCharIdx = ((i+1)>=sizeof(alphabet))?0:(i+1);
			return alphabet[nextCharIdx];
		}
	}
	return alphabet[0];
}

char alphabetGetPreviousChar(char currentChar) {
	int prevCharIdx;
	for (int i = 0; i < sizeof(alphabet); i++)
	{
		if (currentChar == alphabet[i]) {
			prevCharIdx = ((i-1)<0)?(sizeof(alphabet)-1):(i-1);
			return alphabet[prevCharIdx];
		}
	}
	return alphabet[0];
}

void sanitizeLineOfText(char *textToSanitize, int length) {
	bool validChars = true;
	for (int i = 0; i < length; i++) {
		if ((textToSanitize[i] < 0x20) || (textToSanitize[i] > 0x7E)) validChars = false;
		if (!validChars) textToSanitize[i] = 0x00;
	}
}

static inline uint32_t mix32(uint32_t x) {
    // tani mixer (wariant splitmix / avalanching)
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static inline uint8_t rand_unbiased(uint8_t n) {
    // zwraca równomiernie 0..n-1
    // RAND_MAX zwykle 32767, ale nie zakładamy nic poza >= 255
    uint32_t r;
    uint32_t limit = (RAND_MAX / n) * n;
    do { r = (uint32_t)rand(); } while (r >= limit);
    return (uint8_t)(r % n);
}

void randomizePassword(char *passwordBuffer, int length) {

    // Zbierz trochę entropii: tick + kilka odczytów ADC + Twoje randomSeed
    uint32_t seed = HAL_GetTick();

    for (int k = 0; k < 8; k++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        uint16_t rawADC = HAL_ADC_GetValue(&hadc1);
        seed ^= ((uint32_t)rawADC << ((k & 3) * 8));
        seed = mix32(seed);
    }

    // wmieszaj fragment Twojego randomSeed (który już mieszasz tickiem i przyciskami)
    for (int i = 0; i < 16; i++) {
        seed ^= ((uint32_t)randomSeed[(currentSeedPart + i) % (MAX_LINE_OF_TEXT_CHARS * 4)] << ((i & 3) * 8));
        seed = mix32(seed);
    }

    srand(seed);

    uint8_t alpha_len = (uint8_t)(sizeof(alphabet) - 1); // bez 0x00
    for (int i = 0; i < length; i++) {
        passwordBuffer[i] = alphabet[rand_unbiased(alpha_len)];
    }

    if (length < 25) passwordBuffer[length] = 0x00;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  //__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM10_Init();
  MX_USB_DEVICE_Init();
  MX_I2C1_Init();
  MX_CRC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  // Initialize main timer - tim10
  HAL_TIM_Base_Start_IT(&htim10);

  // Disable onboard LED
  HAL_GPIO_WritePin(LED_GPIO, LED_PIN, LED_OFF);

  // Btn GND to LOW
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

  // OLED display INIT

  SSD1306_I2cInit(&hi2c1);
  SSD1306_SetContrast(LCD_DIM_BRIGHT_VALUE);
  lcdDimCounter = LCD_DIM_COUNTER_START;
  displayEnabledCounter = DISPLAY_ENABLED_COUNTER_START;
  btnCancelNextAction = false;

  SSD1306_RotateDisplay(0);
  //SSD1306_RotateDisplay(1);
  SSD1306_ZoomIn(1);
  GFX_SetFont(font_8x5);

  // Configure buttons structures
  buttonState[0].GPIO = BTN0_GPIO;
  buttonState[0].PIN = BTN0_PIN;
  buttonState[1].GPIO = BTN1_GPIO;
  buttonState[1].PIN = BTN1_PIN;
  buttonState[2].GPIO = BTN2_GPIO;
  buttonState[2].PIN = BTN2_PIN;

  HAL_Delay(50);

  GPIO_PinState initbtnstate1 = HAL_GPIO_ReadPin(buttonState[1].GPIO, buttonState[1].PIN);
  GPIO_PinState initbtnstate2 = HAL_GPIO_ReadPin(buttonState[2].GPIO, buttonState[2].PIN);

  if ((initbtnstate1 == BTN_PRESSED) && (initbtnstate2 == BTN_PRESSED)) {
	  databaseMode = PM_DATABASE_MODE_BACKUP;
  }

  selectedSlot = 0;

  GetOptions(optBuffer);

  if (*((uint32_t*)optBuffer) == 0xFFFFFFFF)
  {
	  for (int i = 0; i < sizeof(optBuffer); i++) optBuffer[i] = 0x00;
	  SaveOptions(optBuffer);
  }

  if (optBuffer[OPT_FAILED_LOGINS] < MAX_FAILED_LOGIN) {

	  // Load DB signature
	  GetDBSignature(dbSignature, NUMBER_OF_SLOTS, xorPinKey, databaseMode);

	  uint32_t *tmpSignature = (uint32_t*)dbSignature;
	  if (*tmpSignature == 0xFFFFFFFF )
	  {
		  // DATABSE UNINTIALIZED
		  cleanAndSaveDB();
		  loadAndDisplaySlots();
		  welcomeMsg();
	  }
	  else if (*tmpSignature == *((uint32_t*)validSignature))
	  {
		  // DATABSE OK NO PASSWORD
		  loadAndDisplaySlots();
		  welcomeMsg();
	  }
	  else
	  {
		  displayContext = DISPLAY_CONTEXT_PINENTER;
		  pinEntered = 0x00;
		  pinEnterPos = 0x00;
	  }
  }
  else
  {
	  strcpy(lcdMessage, lcdMessageLocked);
	  displayContext = DISPLAY_CONTEXT_HLTMSG;
  }

  saveDatabasePending = false;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

		#ifdef SSD1306_I2C_DMA_ENABLE
		if(hi2c1.hdmatx->State == HAL_DMA_STATE_READY) {
		#endif

			if (lcdBrightnessStatus & LCD_BRIGHTNESS_GO_DIM) {
				SSD1306_SetContrast(LCD_DIM_DARK_VALUE);
				lcdBrightnessStatus &= ~LCD_BRIGHTNESS_GO_DIM;
				lcdBrightnessStatus |= LCD_BRIGHTNESS_IS_DIM;
			}

			if (lcdBrightnessStatus & LCD_BRIGHTNESS_GO_BRIGHT) {
				SSD1306_SetContrast(LCD_DIM_BRIGHT_VALUE);
				lcdBrightnessStatus &= ~LCD_BRIGHTNESS_GO_BRIGHT;
				lcdBrightnessStatus &= ~LCD_BRIGHTNESS_IS_DIM;
			}

			SSD1306_Clear(BLACK);

		if (displayEnabledCounter > 0) {

			switch (displayContext)
			{

				case DISPLAY_CONTEXT_MAIN:
					GFX_SetFontSize(1);

					for (int i = 0; i < NUMBER_OF_LCD_LINES; i++) {
						if (strlen(slotDisplayLines[i]) > 0) {
							if (slotsWithLabel[i]) GFX_DrawString(0, 8 * i, slotLabels[i+displaySlotsFirst], (selectedSlot == i+displaySlotsFirst)?BLACK:WHITE, (selectedSlot == i+displaySlotsFirst)?WHITE:BLACK);
							else GFX_DrawString(0, 8 * i, slotLabels[NUMBER_OF_SLOTS], (selectedSlot == i+displaySlotsFirst)?BLACK:WHITE, (selectedSlot == i+displaySlotsFirst)?WHITE:BLACK);
							GFX_DrawString(16, 8 * i, slotDisplayLines[i], WHITE, BLACK);
						}
					}

					break;

				case DISPLAY_CONTEXT_SLOTEDIT:
					GFX_SetFontSize(1);

					GFX_DrawString(0, 0, slotEditNameLabel, (editOption == EDIT_OPTION_NAME)?BLACK:WHITE, (editOption == EDIT_OPTION_NAME)?WHITE:BLACK);
					GFX_DrawString(0, 8, slotEditUsernameLabel, (editOption == EDIT_OPTION_USERNAME)?BLACK:WHITE, (editOption == EDIT_OPTION_USERNAME)?WHITE:BLACK);
					GFX_DrawString(0, 16, slotEditPasswordLabel, (editOption == EDIT_OPTION_PASSWORD)?BLACK:WHITE, (editOption == EDIT_OPTION_PASSWORD)?WHITE:BLACK);

					GFX_DrawString(0, 24, slotEditNewPassBtn, (editOption == EDIT_OPTION_NEWPASS)?BLACK:WHITE, (editOption == EDIT_OPTION_NEWPASS)?WHITE:BLACK);
					GFX_DrawString(39, 24, slotEditSaveBtn, (editOption == EDIT_OPTION_SAVE)?BLACK:WHITE, (editOption == EDIT_OPTION_SAVE)?WHITE:BLACK);
					GFX_DrawString(90, 24, slotEditDeleteBtn, (editOption == EDIT_OPTION_DELETE)?BLACK:WHITE, (editOption == EDIT_OPTION_DELETE)?WHITE:BLACK);

					GFX_DrawString(12, 0, currentEditPasswordSlot.name, WHITE, BLACK);
					GFX_DrawString(12, 8, currentEditPasswordSlot.username, WHITE, BLACK);
					GFX_DrawString(12, 16, currentEditPasswordSlot.password, WHITE, BLACK);

					break;

				case DISPLAY_CONTEXT_TEXTEDIT:
					GFX_SetFontSize(1);

					okButtonIdx = (strlen(editedLineOfText.line) + 1 < MAX_LINE_OF_TEXT_CHARS)?strlen(editedLineOfText.line) + 1:MAX_LINE_OF_TEXT_CHARS;

					GFX_DrawString(0, 0, editedLineOfText.name, WHITE, BLACK);
					GFX_DrawString(100, 0, slotEditOKBtn, (editedLineOfText.editedChar == okButtonIdx)?BLACK:WHITE, (editedLineOfText.editedChar == okButtonIdx)?WHITE:BLACK);

					bool charsValid = true;

					for (int i = 0; i< okButtonIdx; i++) {
						if (editedLineOfText.line[i] == 0x00) charsValid = false;
						if (charsValid) GFX_DrawChar(i * 6, 16, editedLineOfText.line[i], WHITE, BLACK);
						else GFX_DrawChar(i * 6, 16, emptyChar, WHITE, BLACK);
						if (editedLineOfText.editedChar == i) GFX_DrawChar(i * 6, 26, upArrowChar, WHITE, BLACK);
					}

					break;

				case DISPLAY_CONTEXT_DELCONFIRM:

					GFX_SetFontSize(2);
					GFX_DrawString(0,8, lcdMessage, WHITE, BLACK);

					GFX_SetFontSize(1);
					GFX_DrawString(80,24, slotConfirmDeleteLabel, BLACK, WHITE);

					break;

				case DISPLAY_CONTEXT_MESSAGE:

					GFX_SetFontSize(2);
					GFX_DrawString(0,8, lcdMessage, WHITE, BLACK);

					break;

				case DISPLAY_CONTEXT_PINSET:

					GFX_SetFontSize(1);
					GFX_DrawString(0, 0, pinEditLabel, WHITE, BLACK);

					GFX_DrawString(100, 0, slotEditOKBtn, (pinEnterPos > 7)?BLACK:WHITE, (pinEnterPos > 7)?WHITE:BLACK);

					for (int i = 0; i < 8; i++)
					{
						GFX_SetFontSize(2);
						if (i<pinEnterPos)
						{
							GFX_DrawChar(17 + i * 11, 12, (pinEntered >> (7 - i) & 0x01)?PIN_ENTER_CHAR_ONE:PIN_ENTER_CHAR_ZERO , WHITE, BLACK);
						}
						else
						{
							GFX_DrawChar(17 + i * 11, 12, PIN_ENTER_CHAR_UNKNOWN, WHITE, BLACK);
						}
						if (pinEnterPos == i)
						{
							GFX_SetFontSize(1);
							GFX_DrawChar(19 + i * 11, 28, PIN_ENTER_CHAR_UPARROW, WHITE, BLACK);
						}
					}
					break;

				case DISPLAY_CONTEXT_PINENTER:

					GFX_SetFontSize(1);
					GFX_DrawString(0, 0, pinEnterLabel, WHITE, BLACK);
					sprintf(str, "(%d/%d)", optBuffer[0]+1, MAX_FAILED_LOGIN);
					GFX_DrawString(90, 0, str, WHITE, BLACK);

					for (int i = 0; i < 8; i++)
					{
						GFX_SetFontSize(2);
						if (i<pinEnterPos)
						{
							GFX_DrawChar(17 + i * 11, 12, (pinEntered >> (7 - i) & 0x01)?PIN_ENTER_CHAR_ONE:PIN_ENTER_CHAR_ZERO , WHITE, BLACK);
						}
						else
						{
							GFX_DrawChar(17 + i * 11, 12, PIN_ENTER_CHAR_UNKNOWN, WHITE, BLACK);
						}
						if (pinEnterPos == i)
						{
							GFX_SetFontSize(1);
							GFX_DrawChar(19 + i * 11, 28, PIN_ENTER_CHAR_UPARROW, WHITE, BLACK);
						}
					}
					break;

				case DISPLAY_CONTEXT_HLTMSG:

					GFX_SetFontSize(2);
					GFX_DrawString(0,8, lcdMessage, WHITE, BLACK);

					break;

				}

			}

			SSD1306_Display();

		#if  defined(SSD1306_I2C_DMA_ENABLE) || defined(SSD1306_SPI_DMA_ENABLE)
		}
		#endif

	HAL_Delay(50);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// Caps lock internal status procedure used to remember if caps-lock disable code should be issued before typing username/pasword
void setCapslockState(uint8_t state) {
	capslockState = state;
	if (capslockState) {
		HAL_GPIO_WritePin(LED_GPIO, LED_PIN, LED_ON);
	} else {
		HAL_GPIO_WritePin(LED_GPIO, LED_PIN, LED_OFF);
	}
}

void resetCapslockState(void) {
	uint8_t *rep = GetReportByChar(SEQ_CAPSLOCK);
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	// Check if IRQ is from timer 10
	if(htim->Instance == TIM10){
		// Check is key is in "pressed" state
		if (keyPressed) {
			keyPressed--;
			if (keyPressed == KEYPRESS_RELEASE_AFTER_CYCLES) {
				uint8_t *rep = GetReportByChar(SEQ_RELEASE); // SEND RELEASE in half of typing cycle
				USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
			}
		}
		else
		{
			// Username typing sequence
			// First check caps lock state
			if (typeSequence.lUsername > -1) { // Check if there if we should type something
				if (capslockState) {
					resetCapslockState();
					keyPressed = KEYPRESS_EVERY_N_CYCLES;
				}
				else
				{
					// Check if there is no more string to type or we have some error typesequence larger than possible length
					if ((passwordSlots[selectedSlot].username[typeSequence.lUsername] == 0x00) || (typeSequence.lUsername >= sizeof(passwordSlots[selectedSlot].username)))
					{
						// Type TAB code on the end of username sequence
						if (typeSequence.withTerminations) {
							uint8_t *rep = GetReportByChar(SEQ_TAB);
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
							keyPressed = KEYPRESS_EVERY_N_CYCLES;
						}
						typeSequence.lUsername=TYPE_SEQUENCE_OFF;
					}
					else
					{
						// Type next username letter
						uint8_t *rep = GetReportByChar(passwordSlots[selectedSlot].username[typeSequence.lUsername]);
						USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
						typeSequence.lUsername++;
						keyPressed = KEYPRESS_EVERY_N_CYCLES;
					}
				}
				// Key pressed for N cycles
			}
			// If there is no more username to type, now type password
			else if (typeSequence.lPassword > -1)
			{
				// First check caps lock state
				if (capslockState) {
					resetCapslockState();
					keyPressed = KEYPRESS_EVERY_N_CYCLES;
				}
				else
				{
					// Check if there is no more string to type or we have some error typesequence larger than possible length
					if ((passwordSlots[selectedSlot].password[typeSequence.lPassword] == 0x00) || (typeSequence.lPassword >= sizeof(passwordSlots[selectedSlot].password)))
					{
						if (typeSequence.withTerminations) {
							uint8_t *rep = GetReportByChar(SEQ_ENTER); // Type ENTER to end password sequence
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
							keyPressed = KEYPRESS_EVERY_N_CYCLES;
						}
						typeSequence.lPassword=TYPE_SEQUENCE_OFF;
						//Stop displaying typing message
						displayContext = DISPLAY_CONTEXT_MAIN;
					}
					else
					{
						// Type next password letter
						uint8_t *rep = GetReportByChar(passwordSlots[selectedSlot].password[typeSequence.lPassword]);
						USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
						typeSequence.lPassword++;
						keyPressed = KEYPRESS_EVERY_N_CYCLES;
					}
				}
			}
			// Let's type previous password if needed
			else if (typeSequence.lPreviousPassword > -1)
				{
					// First check caps lock state
					if (capslockState) {
						resetCapslockState();
						keyPressed = KEYPRESS_EVERY_N_CYCLES;
					}
					else
					{
						// Check if there is no more string to type or we have some error typesequence larger than possible length
						if ((passwordSlots[selectedSlot].previouspassword[typeSequence.lPreviousPassword] == 0x00) || (typeSequence.lPreviousPassword >= sizeof(passwordSlots[selectedSlot].previouspassword)))
						{
							if (typeSequence.withTerminations) {
								uint8_t *rep = GetReportByChar(SEQ_ENTER); // Type ENTER to end password sequence
								USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
								keyPressed = KEYPRESS_EVERY_N_CYCLES;
							}
							typeSequence.lPreviousPassword=TYPE_SEQUENCE_OFF;
							//Stop displaying typing message
							displayContext = DISPLAY_CONTEXT_MAIN;
						}
						else
						{
							// Type next password letter
							uint8_t *rep = GetReportByChar(passwordSlots[selectedSlot].previouspassword[typeSequence.lPreviousPassword]);
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, rep, sizeof(KbdReport));
							typeSequence.lPreviousPassword++;
							keyPressed = KEYPRESS_EVERY_N_CYCLES;
						}
					}
				}
		}

		for (int i = 0; i < BUTTONS; i++) {
			// Buttons debounce procedures
			if (buttonState[i].stateCounter > 0) {
				if (--buttonState[i].stateCounter == 0)
				{
					if (buttonState[i].isPressed) buttonState[i].awaitingPressAction = true;
					else (buttonState[i].awaitingReleaseAction) = true;
				}
			}
			// Execute button press/release procedures
			// Check if long action should be issued (holdCounter down to zero)
			if ((buttonState[i].isHolded) && (buttonState[i].holdCounter>0))
			{
				if (--buttonState[i].holdCounter == 0) buttonState[i].awaitingLongAction = true;
			}
			// Check if there is awaiting press action (button is holded short)
			if (buttonState[i].awaitingPressAction)
			{
				buttonState[i].awaitingPressAction = false;
				buttonState[i].isHolded = true;
				buttonState[i].awaitingLongAction = false;
				buttonState[i].holdCounter = BUTTON_HOLD_COUNTER;
			}
			// Check if there is awaiting release action to execute after short or long press
			if ((buttonState[i].awaitingReleaseAction) || (buttonState[i].awaitingLongAction))
			{
				if (buttonState[i].awaitingLongAction) {
					longPress(i);
				}
				else
				{
					shortPress(i);
				}
				buttonState[i].awaitingReleaseAction = false;
				buttonState[i].isHolded = false;
				buttonState[i].awaitingLongAction = false;
			}
		}

		if (delayedLogin > 0) {
			if (--delayedLogin == 0) {
				pinEnterPos = 0x00;
				pinEntered = 0x00;
				loadEncryptedDatabase();
			}
		}

		if (messageDisplayCounter > 0)
		{
			if ((saveDatabasePending == true) && (messageDisplayCounter == SAVE_ALL_SLOTS_ON_CYCLE)) {
				saveDatabasePending = false;
				bool saveret = saveAllSlots();
				if (saveret) {
					strcpy(lcdMessage, lcdMessageSaveOK);
				} else {
					strcpy(lcdMessage, lcdMessageSaveError);
				}
				displayContext = DISPLAY_CONTEXT_MESSAGE;
				messageDisplayCounter = MSG_DISPLAY_CYCLES;
				loadAndDisplaySlots();
			}
			if (--messageDisplayCounter == 0) displayContext = DISPLAY_CONTEXT_MAIN;
		}

		uint32_t lastTick = HAL_GetTick();
		randomSeed[currentSeedPart] ^= (uint8_t)(lastTick & 0xFF);
		if (++currentSeedPart>=(MAX_LINE_OF_TEXT_CHARS * 4)) currentSeedPart=0;

		if (lcdDimCounter > 0) {
			if ((--lcdDimCounter == 0) && !(lcdBrightnessStatus & LCD_BRIGHTNESS_IS_DIM)) lcdBrightnessStatus |= LCD_BRIGHTNESS_GO_DIM;
		}

		if (displayEnabledCounter > 0) displayEnabledCounter--;
	}
}

// BUTTONs EXIT IRQ Callback procedure

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//HAL_NVIC_DisableIRQ(EXTI1_IRQn);
	if (++currentSeedPart>=(MAX_LINE_OF_TEXT_CHARS * 4)) currentSeedPart=0;

	lcdDimCounter = LCD_DIM_COUNTER_START;

	if (lcdBrightnessStatus & LCD_BRIGHTNESS_IS_DIM) {
		lcdBrightnessStatus |= LCD_BRIGHTNESS_GO_BRIGHT;
	}

	lcdBrightnessStatus &= ~LCD_BRIGHTNESS_GO_DIM;

	if (displayEnabledCounter == 0) btnCancelNextAction = true;

	displayEnabledCounter = DISPLAY_ENABLED_COUNTER_START;

	// New button debouce procedures
	// Iterate throughout buttons
	for (int i = 0; i < BUTTONS; i++)
	{
		// Check which button is changed
		if (GPIO_Pin & buttonState[i].PIN)
		{
			// Check current button state
			GPIO_PinState state = HAL_GPIO_ReadPin(buttonState[i].GPIO, buttonState[i].PIN);
			if (state == BTN_PRESSED) {
				// If pressed
				buttonState[i].isPressed = true;
				buttonState[i].stateCounter = BUTTON_DEBOUNCE_COUNTER;
				buttonState[i].awaitingPressAction = false;
			} else {
				if (buttonState[i].isHolded)
				{
					// If released while holded
					buttonState[i].isPressed = false;
					buttonState[i].stateCounter = BUTTON_DEBOUNCE_COUNTER;
					buttonState[i].awaitingReleaseAction = false;
				}
				else
				{
					// If released while short press
					buttonState[i].isPressed = false;
					buttonState[i].stateCounter = 0;
					buttonState[i].awaitingPressAction = false;
					buttonState[i].awaitingReleaseAction = false;
				}
			}
		}
	}

	//HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
