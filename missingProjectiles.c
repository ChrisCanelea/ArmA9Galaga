/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
//#define WHITE 0xFFFF
#define WHITE 0xDEFB
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
//#define BLUE 0x001F
#define BLUE 0x035C
//#define CYAN 0x07FF
#define CYAN 0x04B2
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
//#define ORANGE 0xFC00
#define ORANGE 0xDA20

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define SCREEN_W 320
#define SCREEN_H 240

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

//shape structures
typedef struct rect 
{
    int x;
    int y;
    int length;
    int height;
    int colour;
    int old_x;
    int old_y;
    int dx;
    int dy;
    int left;
    int right;
    int top;
    int bottom;
} rect;

typedef struct gameObject
{
    int length;
    int height;
    rect hitbox;
    short int sprite[8][16][16];
} gameObject;

typedef struct bullet
{
    int length;
    int height;
    rect hitbox;
    short int sprite[8][3];
} bullet;

//basic functions
void swap(int* a, int* b);
void getNumString(int a, char score[], int size);

//main loop functions
void titleScreen();
int gameLoop();
void initializeStars(int stars[][224]);

void initializeBossLine(gameObject* bossLine);
void initializeGoeiLine(gameObject* goeiLine, int lineNumber);
void initializeZakoLine(gameObject* zakoLine, int lineNumber);

//gameObject functions
gameObject createObject(int length_, int height_);
void setObjectPos(gameObject* object, int x_, int y_);
void drawObject(gameObject object, int spriteNum);
void eraseOldObject(gameObject object);
void updateObjectPos(gameObject* object);

//bullet functions
bullet createBullet(int length_, int height_);
void setBulletPos(bullet* object, int x_, int y_);
void drawBullet(bullet object);
void eraseOldBullet(bullet object);
void updateBulletPos(bullet* object);

//assets
void initializePlayer(gameObject* object);
void initializeBossGalaga(gameObject* object);
void initializeGoeiGalaga(gameObject* object);
void initializeZakoGalaga(gameObject* object);
void initializePlayerBullet(bullet* object);

//rect functions
rect createRect(int x_, int y_, int length_, int height_, short int colour_);
void setRectPos(rect* rect_, int x_, int y_);
void drawRect(rect rect_);
void eraseRect(rect rect_);
void eraseOldRect(rect rect_);
bool x_outOfBounds(rect rect_, int leftBound, int rightBound);
bool y_outOfBounds(rect rect_, int upperBound, int lowerBound);
bool contact(rect topLeft, rect bottomRight);
void updatePos(rect* _rect);
void setBottom(rect* _rect, int topBoundary);

//vga functions
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int colour);
void wait_for_vsync(volatile int status, volatile int* frontBuffAddr);
void writeText(int x, int y, char* text);
void clearText();

volatile int pixel_buffer_start; // global variable
int pixelSize = 1;
int timer = 0;
volatile int* keysBaseAddr;
volatile int* frontBuffAddr;
volatile int* backBuffAddr;
volatile int status;

// vga text globals
int intHighScore = 0;
char charHighScore[8] = "0000000\0";
int intCurrentScore = 0;
char charCurrentScore[8] = "0000000\0";
char gameTitle[7] = "GALAGA\0";
char credits1[13] = "By CHRISTIAN\0";
char credits2[14] = "and SEBASTIAN\0";
int intStageNumber = 0;
char charStageNumber[3] = "00\0";
char highScoreText[11] = "HIGH SCORE\0";
char currentScoreText[14] = "CURRENT SCORE\0";
char stageText[6] = "STAGE\0";

int main(void)
{
    //volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    keysBaseAddr = (int*) KEY_BASE;
    
    // declare other variables(not shown)
    frontBuffAddr = (int*)0xFF203020;//front buffer register address
    backBuffAddr = (int*)(frontBuffAddr + 1);
    status = (*(frontBuffAddr + 3) & 0x01);//1 if VGA not done frame, 0 if VGA done frame
    srand(time(NULL));//seed random numbers based on time

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(backBuffAddr) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    *frontBuffAddr = 1;
    wait_for_vsync(status, frontBuffAddr);

    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *frontBuffAddr;

    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */

    *(backBuffAddr) = 0xC0000000;
    pixel_buffer_start = *(backBuffAddr); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    //swap buffers once before starting main game loop
    *frontBuffAddr = 1;
    wait_for_vsync(status, frontBuffAddr);
    pixel_buffer_start = *backBuffAddr;

    while(1) { // GAME LOOP
        titleScreen();
        gameLoop();
    }
}

// code for subroutines (not shown)

//basic functions
void swap(int* a, int* b) 
{
    int temp = *b;
    *b = *a;
    *a = temp;
}

//main loop functions
void titleScreen() {
    //reset globals and flags
    bool gameStart = FALSE;
    intStageNumber = 0;

    if (intCurrentScore > intHighScore) { // update high score if current score is higher than record
        intHighScore = intCurrentScore;
    }

    intCurrentScore = 0;

    clear_screen();

    //buffer stuff
    *frontBuffAddr = 1;//swap buffers
    wait_for_vsync(status, frontBuffAddr); //wait for VGA vertical sync
    pixel_buffer_start = *backBuffAddr; // new back buffer

    clear_screen();
    clearText();

    gameObject player = createObject(16, 16); // initialize title screen player (dummy not controllable)
    initializePlayer(&player);
    setObjectPos(&player, 104, 220);

    int stars[2][224];
    initializeStars(&stars);

    char startTitle[23] = "Press Any Key to Start\0"; // title screen text prompt

    getNumString(intHighScore, charHighScore, 8);
    getNumString(intStageNumber, charStageNumber, 3);
    getNumString(intCurrentScore, charCurrentScore, 8); // converts globals into a character string for writing on vga

    writeText(18, 24, startTitle);

    writeText(60, 3, gameTitle);
    writeText(66, 9, highScoreText);
    writeText(69, 11, charHighScore);
    writeText(63, 20, currentScoreText);
    writeText(69, 22, charCurrentScore);
    writeText(71, 31, stageText);
    writeText(74, 33, charStageNumber);

    writeText(60, 54, credits1);
    writeText(64, 56, credits2);

    *(keysBaseAddr + 3) = 0xF; //clear anything already in edge capture

    while (!gameStart) {
        if (*(keysBaseAddr + 3) & 0xF) { // any key will start the game
            *(keysBaseAddr + 3) = 0xF;
            gameStart = TRUE;
        }

        //stars
        int tempOldPos;

        for (int i = 0; i < 224; i++) {
            // erase old star, draw new star
            tempOldPos = (stars[0][i] - (2*stars[1][i])); // old star position

            if ((tempOldPos >= 0) && (tempOldPos < 240)) { // erase only if old star in bounds
                plot_pixel(i,tempOldPos, 0);
            }

            if (stars[0][i] > 260) { // reset position after star passes screen
                stars[0][i] = -740;
            }

            plot_pixel(i,stars[0][i],0xFFFF);

            stars[0][i] += stars[1][i]; // increment position by dy
        }

        drawObject(player, 1);

        //buffer stuff
        *frontBuffAddr = 1;//swap buffers
        wait_for_vsync(status, frontBuffAddr); //wait for VGA vertical sync
        pixel_buffer_start = *backBuffAddr; // new back buffer
    }
}

int gameLoop() {
    bool gameOver = FALSE; //reset flag
    intStageNumber += 1;

    clear_screen();

    //buffer stuff
    *frontBuffAddr = 1;//swap buffers
    wait_for_vsync(status, frontBuffAddr); //wait for VGA vertical sync
    pixel_buffer_start = *backBuffAddr; // new back buffer

    clear_screen();
    clearText();


    //initialize game objects
    gameObject player = createObject(16, 16); //player initialization (controllable)
    initializePlayer(&player);
    setObjectPos(&player, 104, 220);

    gameObject bossLine[4];
    gameObject goeiLine1[8];
    gameObject goeiLine2[8];
    gameObject zakoLine1[10];
    gameObject zakoLine2[10]; // enemy instantiation

    initializeBossLine(bossLine);
    initializeGoeiLine(goeiLine1, 1);
    initializeGoeiLine(goeiLine2, 2);
    initializeZakoLine(zakoLine1, 1);
    initializeZakoLine(zakoLine2, 2); // enemy initialization

    int stars[2][224];
    initializeStars(&stars);

    while (!gameOver)
    {   
        // DEBUG STUFF
        if (*(keysBaseAddr + 3) & 0x8) { // KEY3 ENDS GAME
            *(keysBaseAddr + 3) = 0xF;
            gameOver = TRUE;
        }
        if (*(keysBaseAddr + 3) & 0x4) { // KEY2 adds random score
            *(keysBaseAddr + 3) = 0xF;
            intCurrentScore += ((rand()%4) + 1) * 50;
        }
        
        getNumString(intStageNumber, charStageNumber, 3);
        getNumString(intCurrentScore, charCurrentScore, 8); // converts globals into a character string for writing on vga
        
        writeText(60, 3, gameTitle);
        writeText(66, 9, highScoreText);
        writeText(69, 11, charHighScore);
        writeText(63, 20, currentScoreText);
        writeText(69, 22, charCurrentScore);
        writeText(71, 31, stageText);
        writeText(74, 33, charStageNumber);

        writeText(60, 54, credits1);
        writeText(64, 56, credits2);
        
        //Erase objects from previous iteration
        eraseOldObject(player);
        player.hitbox.old_x = player.hitbox.x; //set "old" values to current values
        player.hitbox.old_y = player.hitbox.y;
        
        for (int i = 0; i < 4; i++) { // delete boss
            eraseOldObject(bossLine[i]);
            bossLine[i].hitbox.old_x = bossLine[i].hitbox.x;
            bossLine[i].hitbox.old_y = bossLine[i].hitbox.y;
        }

        for (int i = 0; i < 8; i++) { // delete goei
            eraseOldObject(goeiLine1[i]);
            goeiLine1[i].hitbox.old_x = goeiLine1[i].hitbox.x;
            goeiLine1[i].hitbox.old_y = goeiLine1[i].hitbox.y;
            eraseOldObject(goeiLine2[i]);
            goeiLine2[i].hitbox.old_x = goeiLine2[i].hitbox.x;
            goeiLine2[i].hitbox.old_y = goeiLine2[i].hitbox.y;
        }

        for (int i = 0; i < 10; i++) { // delete zako
            eraseOldObject(zakoLine1[i]);
            zakoLine1[i].hitbox.old_x = zakoLine1[i].hitbox.x;
            zakoLine1[i].hitbox.old_y = zakoLine1[i].hitbox.y;
            eraseOldObject(zakoLine2[i]);
            zakoLine2[i].hitbox.old_x = zakoLine2[i].hitbox.x;
            zakoLine2[i].hitbox.old_y = zakoLine2[i].hitbox.y;
        }

        if ((*keysBaseAddr & 0x0001) == 1) { // keys to move player left or right
            player.hitbox.dx = 3;
        } else if ((*keysBaseAddr & 0x0002) == 2) {
            player.hitbox.dx = -3;
        } else {
            player.hitbox.dx = 0;
        }

        if (x_outOfBounds(player.hitbox, 0, 319)) { // prevent player motion if going out of bounds
            player.hitbox.dx = 0;
        }
        
        if (y_outOfBounds(player.hitbox, 0, 239)) {
            player.hitbox.dy = 0;
        }

        updateObjectPos(&player); // update current positions        

        //stars
        int tempOldPos;

        for (int i = 0; i < 224; i++) {
            // erase old star, draw new star
            tempOldPos = (stars[0][i] - (2*stars[1][i]));

            if ((tempOldPos >= 0) && (tempOldPos < 240)) { // erase only if old star in bounds
                plot_pixel(i,tempOldPos, 0);
            }

            if (stars[0][i] > 260) { // reset position after star passes screen
                stars[0][i] = -740;
            }

            plot_pixel(i,stars[0][i],0xFFFF);

            stars[0][i] += stars[1][i]; // increment position by dy
        }


        drawObject(player, 1);

        for (int i = 0; i < 4; i++) { // draw boss
            if (bossLine[i].hitbox.y < 16) {
                bossLine[i].hitbox.dy = 1;
            } else {
                bossLine[i].hitbox.dy = 0;
            }

            updateObjectPos(&bossLine[i]);

            drawObject(bossLine[i], (timer/8)%2); // (timer/8)%2
        }

        for (int i = 0; i < 8; i++) { // draw goei
            if (goeiLine1[i].hitbox.y < 32) {
                goeiLine1[i].hitbox.dy = 1;
            } else {
                goeiLine1[i].hitbox.dy = 0;
            }

            if (goeiLine2[i].hitbox.y < 48) {
                goeiLine2[i].hitbox.dy = 1;
            } else {
                goeiLine2[i].hitbox.dy = 0;
            }

            updateObjectPos(&goeiLine1[i]);
            updateObjectPos(&goeiLine2[i]);

            drawObject(goeiLine1[i], (timer/8)%2);
            drawObject(goeiLine2[i], (timer/8)%2);
        }
        
        for (int i = 0; i < 10; i++) { // draw zako
            if (zakoLine1[i].hitbox.y < 64) {
                zakoLine1[i].hitbox.dy = 1;
            } else {
                zakoLine1[i].hitbox.dy = 0;
            }
            
            if (zakoLine2[i].hitbox.y < 80) {
                zakoLine2[i].hitbox.dy = 1;
            } else {
                zakoLine2[i].hitbox.dy = 0;
            }

            updateObjectPos(&zakoLine1[i]);
            updateObjectPos(&zakoLine2[i]);

            drawObject(zakoLine1[i], (timer/8)%2);
            drawObject(zakoLine2[i], (timer/8)%2);
        }
        
        timer = timer + 1;

        //buffer stuff
        *frontBuffAddr = 1;//swap buffers
        wait_for_vsync(status, frontBuffAddr); //wait for VGA vertical sync
        pixel_buffer_start = *backBuffAddr; // new back buffer
    }

}

void initializeBossLine(gameObject* bossLine) {
    for (int i = 0; i < 4; i++) {
        bossLine[i] = createObject(16,16);
        initializeBossGalaga(&bossLine[i]);
        setObjectPos(&bossLine[i], 80 + (16*i), -96);
    }
}

void initializeGoeiLine(gameObject* goeiLine, int lineNumber) {
    for (int i = 0; i < 8; i++) {
        goeiLine[i] = createObject(16, 16);
        initializeGoeiGalaga(&goeiLine[i]);
        setObjectPos(&goeiLine[i], 48 + (16*i), -88 + (16*lineNumber));
    }
}

void initializeZakoLine(gameObject* zakoLine, int lineNumber) {
    for (int i = 0; i < 10; i++) {
        zakoLine[i] = createObject(16,16);
        initializeZakoGalaga(&zakoLine[i]);
        setObjectPos(&zakoLine[i], 32 + (16*i), -48 + (16*lineNumber));
    }
}

void initializeStars(int stars[][224]) {
    for (int i = 0; i < 224; i++) {
        stars[0][i] = (rand()%1001) - 760; // initial y-pos
        stars[1][i] = (rand()%3) + 1; // initial dy
    }
}

//gameObject functions
gameObject createObject(int length_, int height_)
{
    gameObject object;
    object.length = length_;
    object.height = height_;
    object.hitbox = createRect(0, 0, length_, height_, 0);
    //object.sprite = (short int**) malloc((length_ * height_) * (sizeof(short int)));
    return object;
}

void setObjectPos(gameObject* object, int x_, int y_)
{
    setRectPos(&object->hitbox, x_, y_);
}

void updateObjectPos(gameObject* object) {
    updatePos(&((*object).hitbox));
}

void drawObject(gameObject object, int spriteNum)
{
    for (int row = 0; row < object.height; row++) {
        for (int col = 0; col < object.length; col++) {
            //plot_pixel(object.hitbox.x + col, object.hitbox.y + row, object.sprite[row][col]);
            for (int i = 0; i < pixelSize; i++) {
                for (int j = 0; j < pixelSize; j++) {
                    plot_pixel(pixelSize*(object.hitbox.x + col) + i, pixelSize*(object.hitbox.y + row) + j, object.sprite[spriteNum][row][col]);
                }
            }
        }
    }
}

void eraseOldObject(gameObject object) {
    for (int i = 0; i < object.height; i++) {
        for (int j = 0; j < object.length; j++) {
            plot_pixel(object.hitbox.old_x + i, object.hitbox.old_y + j, 0);
        }
    }
}

//bullet functions
bullet createBullet(int length_, int height_) {
    bullet object;
    object.length = length_;
    object.height = height_;
    object.hitbox = createRect(0, 0, length_, height_, 0);
    return object;
}

void setBulletPos(bullet* object, int x_, int y_) {
    setRectPos(&object->hitbox, x_, y_);
}

void updateBulletPos(bullet* object) {
    updatePos(&((*object).hitbox));
}

void drawBullet(bullet object) {
    for (int row = 0; row < object.height; row++) {
        for (int col = 0; col < object.length; col++) {
            for (int i = 0; i < pixelSize; i++) {
                for (int j = 0; j < pixelSize; j++) {
                    plot_pixel(pixelSize*(object.hitbox.x + col) + i, pixelSize*(object.hitbox.y + row) + j, object.sprite[row][col]);
                }
            }
        }
    }
}

void eraseOldBullet(bullet object) {
    for (int i = 0; i < object.height; i++) {
        for (int j = 0; j < object.length; j++) {
            plot_pixel(object.hitbox.old_x + i, object.hitbox.old_y + j, 0);
        }
    }
}

//assets
void initializePlayer(gameObject* object)
{   
    //player has no 0th sprite, fill with black (complete)
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[0][row][col] = 0;
        }
    }
    
    //1st sprite (complete)
    short int array1[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[1][row][col] = array1[row][col];
        }
    }
    //2nd sprite (incomplete)
    short int array2[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[2][row][col] = array2[row][col];
        }
    }
    //3rd sprite (incomplete)
    short int array3[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[3][row][col] = array3[row][col];
        }
    }
    //4th sprite (incomplete)
    short int array4[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[4][row][col] = array4[row][col];
        }
    }
    //5th sprite (incomplete)
    short int array5[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[5][row][col] = array5[row][col];
        }
    }
    //6th sprite (incomplete)
    short int array6[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[6][row][col] = array6[row][col];
        }
    }
    //7th sprite (incomplete)
    short int array7[16][16] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0,     0},
                                {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0,     0},
                                {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED,     0},
                                {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED,     0},
                                {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE,     0},
                                {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE,     0},
                                {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE,     0},
                                {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE,     0},
                                {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[7][row][col] = array7[row][col];
        }
    }
}

void initializeBossGalaga(gameObject* object)
{   
    //0th sprite (complete)
    short int array0[16][16] = {{    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,  CYAN,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,  CYAN,       0,      0,     0},
                                {    0,      0,      0,      0,     0,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,   CYAN,   CYAN,   CYAN,   CYAN,   CYAN,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,  CYAN,  CYAN, YELLOW, YELLOW,   CYAN, YELLOW, YELLOW,  CYAN,  CYAN,       0,      0,     0},
                                {    0,      0,      0,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,      0,     0},
                                {    0,   CYAN,   CYAN,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,   CYAN,  CYAN},
                                {    0,      0,   CYAN,   CYAN,  CYAN,     0, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,     0,  CYAN,    CYAN,   CYAN,     0},
                                {    0,      0,      0,   CYAN,  CYAN,     0,      0, ORANGE,      0, ORANGE,      0,     0,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,   CYAN,  CYAN,     0,      0, ORANGE,      0, ORANGE,      0,     0,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,   CYAN,  CYAN,  CYAN,      0,      0,      0,      0,      0,  CYAN,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,      0,  CYAN,  CYAN,      0,      0,      0,      0,      0,  CYAN,  CYAN,       0,      0,     0},
                                {    0,      0,      0,      0,     0,  CYAN,   CYAN,      0,      0,      0,   CYAN,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,   CYAN,   CYAN,      0,   CYAN,   CYAN,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[0][row][col] = array0[row][col];
        }
    }
    //1st sprite (complete)
    short int array1[16][16] = {{    0,      0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,  CYAN,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,  CYAN,       0,      0,     0},
                                {    0,      0,      0,      0,     0,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,   CYAN,   CYAN,   CYAN,   CYAN,   CYAN,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,  CYAN, YELLOW, YELLOW,   CYAN, YELLOW, YELLOW,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,      0,     0},
                                {    0,   CYAN,   CYAN,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,   CYAN,  CYAN},
                                {    0,      0,   CYAN,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,   CYAN,     0},
                                {    0,      0,   CYAN, ORANGE,  CYAN,  CYAN,      0, ORANGE,      0, ORANGE,      0,  CYAN,  CYAN,  ORANGE,   CYAN,     0},
                                {    0,   CYAN,   CYAN, ORANGE,  CYAN,     0,      0, ORANGE,      0, ORANGE,      0,     0,  CYAN,  ORANGE,   CYAN,  CYAN},
                                {    0,   CYAN, ORANGE,   CYAN,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,    CYAN, ORANGE,  CYAN},
                                {    0,   CYAN, ORANGE, ORANGE,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,  ORANGE, ORANGE,  CYAN},
                                {    0,   CYAN, ORANGE, ORANGE,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,  ORANGE, ORANGE,  CYAN},
                                {    0,   CYAN,   CYAN,   CYAN,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,    CYAN,   CYAN,  CYAN},
                                {    0,      0,   CYAN,   CYAN,     0,     0,      0,      0,      0,      0,      0,     0,     0,    CYAN,   CYAN,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[1][row][col] = array1[row][col];
        }
    }
    //2nd sprite (complete)
    short int array2[16][16] = {{    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,      0,      0,      0,   CYAN,      0,      0,   CYAN,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,      0,   CYAN,   CYAN, ORANGE, ORANGE,   CYAN,      0,     0,     0,       0,      0,     0},
                                {    0,      0,   CYAN,   CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,   CYAN,   CYAN, ORANGE,   CYAN,   CYAN,   CYAN,   CYAN,   CYAN,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,      0,      0,   CYAN, YELLOW,   CYAN, YELLOW, YELLOW,   CYAN,  CYAN,  CYAN,       0,   CYAN,     0},
                                {    0,      0,      0,   CYAN,   CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,   CYAN,  CYAN,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,   CYAN,   CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,     0,  CYAN,    CYAN,      0,     0},
                                {    0,      0,   CYAN,   CYAN,   CYAN,   CYAN, YELLOW, YELLOW, YELLOW, YELLOW,      0,     0,  CYAN,    CYAN,      0,     0},
                                {    0,   CYAN,   CYAN,   CYAN,   CYAN,      0, YELLOW, ORANGE,      0, ORANGE,      0,     0,  CYAN,    CYAN,      0,     0},
                                {    0,      0,   CYAN,   CYAN,   CYAN,      0,      0, ORANGE,      0, ORANGE,      0,  CYAN,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,   CYAN,   CYAN,      0,      0,      0,      0,      0,      0,     0,  CYAN,    CYAN,      0,     0},
                                {    0,      0,      0,   CYAN,   CYAN,   CYAN,   CYAN,      0,      0,      0,      0,  CYAN,  CYAN,       0,      0,     0},
                                {    0,      0,      0,      0,      0,   CYAN,   CYAN,   CYAN,      0,      0,      0,  CYAN,     0,       0,      0,     0},
                                {    0,      0,      0,      0,      0,      0,   CYAN,   CYAN,   CYAN,      0,   CYAN,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,      0,      0,      0,      0,   CYAN,      0,      0,     0,     0,       0,      0,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[2][row][col] = array2[row][col];
        }
    }
    //3rd sprite (complete)
    short int array3[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0xe240,0xe240,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xe240,0x04d3,0xe240,0x04d3,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x04d3,0x04d3,0xe240,0x04d3,0x04d3,0xffe0,0xffe0,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xffe0,0xffe0,0xffe0,0xffe0,0xffe0,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0xe240,0x0000,0x0000,0x04d3,0x04d3,0xe240,0x04d3,0x0000},
                                {0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xffe0,0xe240,0x0000,0xe240,0x0000,0x0000,0x04d3,0xe240,0xe240,0x04d3,0x0000},
                                {0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0xe240,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0xe240,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0xe240,0xe240,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[3][row][col] = array3[row][col];
        }
    }
    //4th sprite (complete)
    short int array4[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0xe240,0xe240,0x0000,0x0000,0x04d3,0x04d3,0x0000,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x0000,0xe240,0xe240,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xe240,0xe240,0x04d3,0x04d3,0xffe0,0xffe0,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xe240,0xe240,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x04d3,0x04d3,0x0000,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xffe0,0xe240,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0x0000,0xe240,0x0000,0x0000,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0xffe0,0xe240,0xe240,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[4][row][col] = array4[row][col];
        }
    }
    //5th sprite (complete)
    short int array5[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0xe240,0xe240,0x04d3,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0xffe0,0x04d3,0x04d3,0x04d3,0xe240,0xe240,0xe240,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0x04d3,0x04d3,0x04d3,0xe240,0x04d3,0x04d3,0x04d3},
                                {0x0000,0x0000,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xe240,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x04d3,0x04d3,0x0000,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0xffe0,0xffe0,0xe240,0xe240,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xe240,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0xe240,0xe240,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[5][row][col] = array5[row][col];
        }
    }
    //6th sprite (complete)
    short int array6[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x04d3,0x04d3,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xe240,0x04d3,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0x04d3,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x04d3,0xe240,0xe240,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0xe240,0xe240,0x0000,0x0000,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3},
                                {0x0000,0x0000,0x04d3,0x0000,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xe240,0xe240,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x04d3,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x04d3,0x0000,0x04d3,0x04d3,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
};
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[6][row][col] = array6[row][col];
        }
    }
    //7th sprite (complete)
    short int array7[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0xe240,0xe240,0xe240,0x04d3,0x04d3},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xe240,0xe240,0x04d3,0xe240,0xe240,0x04d3,0x04d3},
                                {0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x04d3,0x04d3,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x04d3,0x04d3,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xe240,0xe240,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x04d3,0x04d3,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0xe240,0xe240,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xe240,0xe240,0x04d3,0xffe0,0xffe0,0xffe0,0xffe0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0x04d3,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x04d3,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0xe240,0xe240,0x04d3,0xe240,0xe240,0x04d3,0x04d3},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0xe240,0xe240,0xe240,0x04d3,0x04d3},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x04d3,0x0000,0x0000,0x04d3,0x04d3,0x04d3,0x04d3,0x04d3,0x0000}
};

    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[7][row][col] = array7[row][col];
        }
    }
}

void initializeGoeiGalaga(gameObject* object)
{   
    //0th sprite (complete)
    short int array0[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x035c,0x0000,0x035c,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0xe71c,0xf800,0xe71c,0xf800,0xe71c,0x0000,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0xe71c,0xe71c,0xe71c,0xe71c,0xe71c,0x0000,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x035c,0x035c,0x035c,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x035c,0x035c,0x035c,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x035c,0x035c,0x035c,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x035c,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[0][row][col] = array0[row][col];
        }
    }
    //1st sprite (complete)
    short int array1[16][16] = {{    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,   RED,     0,      0,   BLUE,      0,   BLUE,      0,     0,   RED,       0,      0,     0},
                                {    0,      0,    RED,    RED,   RED,     0,      0,   BLUE,      0,   BLUE,      0,     0,   RED,     RED,    RED,     0},
                                {    0,      0,    RED,    RED,   RED,     0,  WHITE,    RED,  WHITE,    RED,  WHITE,     0,   RED,     RED,    RED,     0},
                                {    0,      0,    RED,    RED,   RED,     0,  WHITE,  WHITE,  WHITE,  WHITE,  WHITE,     0,   RED,     RED,    RED,     0},
                                {    0,      0,      0,    RED,   RED,   RED,    RED,  WHITE,  WHITE,  WHITE,    RED,   RED,   RED,     RED,      0,     0},
                                {    0,      0,      0,      0,   RED,   RED,    RED,   BLUE,   BLUE,   BLUE,    RED,   RED,   RED,       0,      0,     0},
                                {    0,      0,      0,    RED,   RED,   RED,    RED,   BLUE,   BLUE,   BLUE,    RED,   RED,   RED,     RED,      0,     0},
                                {    0,      0,    RED,    RED,   RED,   RED,    RED,  WHITE,  WHITE,  WHITE,    RED,   RED,   RED,     RED,    RED,     0},
                                {    0,      0,      0,    RED,   RED,   RED,      0,   BLUE,   BLUE,   BLUE,      0,   RED,   RED,     RED,      0,     0},
                                {    0,      0,      0,      0,     0,   RED,      0,      0,      0,      0,      0,   RED,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                                {    0,      0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[1][row][col] = array1[row][col];
        }
    }
    //2nd sprite (complete)
    short int array2[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0x0000,0x0000,0x035c,0xe71c,0xf800,0xe71c,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0x0000,0xe71c,0xf800,0x035c,0x035c,0x035c,0x035c,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0x0000,0x035c,0x035c,0x035c,0x035c,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xe71c,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x035c,0x035c,0x035c,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x035c,0x035c,0x035c,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[2][row][col] = array2[row][col];
        }
    }
    //3rd sprite (complete)
    short int array3[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x035c,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0xf800,0xe71c,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0x0000,0x0000,0xf800,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0xf800,0xf800,0xf800,0x0000,0xe71c,0xe71c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000},
                                {0x0000,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xe71c,0x035c,0x035c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x035c,0x035c,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xe71c,0xe71c,0x035c,0x0000,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xe71c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[3][row][col] = array3[row][col];
        }
    }
    //4th sprite (complete)
    short int array4[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0xe71c,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0xf800,0xe71c,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xe71c,0xe71c,0xe71c,0xf800,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0x0000,0xe71c,0xe71c,0xe71c,0xe71c,0x035c,0x035c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0xe71c,0x035c,0x035c,0x035c,0xe71c,0x0000,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x035c,0x035c,0xe71c,0xe71c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0xf800,0xe71c,0xe71c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[4][row][col] = array4[row][col];
        }
    }
    //5th sprite (complete)
    short int array5[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0xe71c,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0xe71c,0xe71c,0xe71c,0x035c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0xe71c,0xe71c,0x035c,0x035c,0xe71c,0xe71c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0xf800,0xf800,0x035c,0xe71c,0xe71c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xe71c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[5][row][col] = array5[row][col];
        }
    }
    //6th sprite (complete)
    short int array6[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0x0000,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0x035c,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0x035c,0x035c,0xe71c,0xe71c,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0x035c,0x035c,0xe71c,0xe71c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0x035c,0x035c,0xe71c,0xe71c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0xf800,0xe71c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0xf800,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[6][row][col] = array6[row][col];
        }
    }
    //7th sprite (complete)
    short int array7[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xe71c,0xe71c,0x035c,0x035c,0xe71c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0xe71c,0xe71c,0x035c,0x035c,0xe71c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xe71c,0xe71c,0x035c,0x035c,0xe71c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0xe71c,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[7][row][col] = array7[row][col];
        }
    }
}

void initializeZakoGalaga(gameObject* object)
{   
    //0th sprite (complete)
    short int array0[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0xfe00,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0xfe00,0xf800,0xfe00,0xf800,0xfe00,0x0000,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0xf800,0xfe00,0xf800,0xf800,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0xfe00,0xfe00,0xfe00,0xfe00,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0xf800,0xf800,0xf800,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xfe00,0xfe00,0xfe00,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0xf800,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[0][row][col] = array0[row][col];
        }
    }
    //1st sprite (complete)
    short int array1[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0xfe00,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0xfe00,0xf800,0xfe00,0xf800,0xfe00,0x0000,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xf800,0xf800,0xfe00,0xf800,0xf800,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0xfe00,0xfe00,0xfe00,0xfe00,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0xf800,0xf800,0xf800,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0xfe00,0xfe00,0xfe00,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0xf800,0xf800,0xf800,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[1][row][col] = array1[row][col];
        }
    }
    //2nd sprite (complete)
    short int array2[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xfe00,0xf800,0xfe00,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0xfe00,0xf800,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xfe00,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xfe00,0xfe00,0xfe00,0x0000,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[2][row][col] = array2[row][col];
        }
    }
    //3rd sprite (complete)
    short int array3[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0x0000,0xfe00,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xf800,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xfe00,0xfe00,0xfe00,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x035c,0x035c,0x0000,0xf800,0xf800,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0xfe00,0xfe00,0xfe00,0xf800,0x035c,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xf800,0xf800,0xf800,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0xf800,0xfe00,0xfe00,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0xfe00,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[3][row][col] = array3[row][col];
        }
    }
    //4th sprite (complete)
    short int array4[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0x0000,0xf800,0xfe00,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xf800,0xfe00,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xfe00,0xfe00,0xfe00,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x035c,0x0000,0xfe00,0xf800,0xfe00,0xfe00,0xfe00,0xf800,0xfe00,0x035c,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x035c,0xfe00,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0x035c,0x035c,0x035c,0x035c,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0xf800,0xf800,0xfe00,0xfe00,0xf800,0x0000,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0xfe00,0xfe00,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0xf800,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[4][row][col] = array4[row][col];
        }
    }
    //5th sprite (complete)
    short int array5[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0x0000,0x035c,0x035c,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0x035c,0x035c,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xf800,0xfe00,0xfe00,0xfe00,0xf800,0x035c,0x0000,0x035c,0x0000,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0xfe00,0xf800,0xfe00,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x035c,0x035c,0x0000,0x035c,0x035c,0x035c,0xf800,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[5][row][col] = array5[row][col];
        }
    }
    //6th sprite (complete)
    short int array6[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0xfe00,0x035c,0x0000,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xfe00,0xfe00,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xfe00,0xfe00,0xfe00,0xfe00,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x035c,0x035c,0x0000,0xf800,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x035c,0x035c,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[6][row][col] = array6[row][col];
        }
    }
    //7th sprite (complete)
    short int array7[16][16] = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0xfe00,0xfe00,0xfe00,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xf800,0xf800,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xfe00,0xfe00,0xf800,0xf800,0xfe00,0xf800,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0xfe00,0xf800,0xfe00,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x035c,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x035c,0x035c,0x035c,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
                                {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[7][row][col] = array7[row][col];
        }
    }
}

void initializePlayerBullet(bullet* object) {
    short int array[8][3] = {{0x0000, 0x035c, 0x0000},
                             {0x0000, 0x035c, 0x0000},
                             {0x035c, 0x035c, 0x035c},
                             {0x035c,  WHITE, 0x035c},
                             {0x0000, 0xF800, 0x0000},
                             {0x0000, 0xF800, 0x0000},
                             {0x0000, 0xF800, 0x0000},
                             {0x0000, 0xF800, 0x0000}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[row][col] = array[row][col];
        }
    }
}

//rect functions
rect createRect(int x_, int y_, int length_, int height_, short int colour_) 
{
    rect box = {x_, y_, length_, height_, colour_, 0, 0, 0, 0, x_, x_ + length_, y_, y_ + height_};
    return box;
}

void setRectPos(rect* rect_, int x_, int y_) 
{
    rect_->old_x = rect_->x;
    rect_->old_y = rect_->y;
    rect_->x = x_;
    rect_->y = y_;
    rect_->left = x_;
    rect_->right = x_ + rect_->length - 1;
    rect_->top = y_;
    rect_->bottom = y_ + rect_->height - 1;
}

void drawRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.x, rect_.y + i, rect_.x + rect_.length - 1, rect_.y + i, rect_.colour);
    }
}

void eraseRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.x, rect_.y + i, rect_.x + rect_.length - 1, rect_.y + i, 0);
    }
}

void eraseOldRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.old_x, rect_.old_y + i, rect_.old_x + rect_.length - 1, rect_.old_y + i, 0);
    }
}

bool x_outOfBounds(rect rect_, int leftBound, int rightBound) 
{
    if (((rect_.x + rect_.dx) > (rightBound - rect_.length)) || ((rect_.x + rect_.dx) < leftBound)) {
        return true;
    } else {
        return false;
    }
}

bool y_outOfBounds(rect rect_, int upperBound, int lowerBound) 
{
    if (((rect_.y + rect_.dy) > (lowerBound - rect_.height)) || ((rect_.y + rect_.dy) < upperBound)) {
        return true;
    } else {
        return false;
    }
}

bool contact(rect topLeft, rect bottomRight) 
{
    if ((topLeft.right >= bottomRight.left) && (topLeft.bottom >= bottomRight.top)) {
        return true;
    } else {
        return false;
    }
}

void updatePos(rect* _rect)
{
    _rect->x = _rect->x + _rect->dx;
    _rect->y = _rect->y + _rect->dy;
    _rect->top = _rect->y;
    _rect->bottom = _rect->y + _rect->height - 1;
    _rect->left = _rect->x;
    _rect->right = _rect->x + _rect->length - 1;
}

void setBottom(rect* _rect, int topBoundary)
{
    _rect->y = topBoundary - _rect->height;
    _rect->top = _rect->y;
    _rect->bottom = topBoundary;
}

//vga functions
void clear_screen() 
{
    for (int i = 0; i < SCREEN_W; i++) {
        for (int j = 0; j < SCREEN_H; j++) {
            plot_pixel(i, j, 0);//plot black pixels
        }
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    if (!(x < 0) && !(x > 319) && !(y < 0) && !(y > 239)) {
        *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int colour) 
{
    int y_step;
    bool is_steep = (abs(y1 - y0) > abs(x1 - x0));

    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }

    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = -(dx / 2);
    int y = y0;

    if (y0 < y1) {
        y_step = 1;
    } else {
        y_step = -1;
    }

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, colour);
        } else {
            plot_pixel(x, y, colour);
        }
        error = error + dy;
        if (error > 0) {
            y = y + y_step;
            error = error - dx;
        }
    }
}

void wait_for_vsync(volatile int status, volatile int* frontBuffAddr) 
{

    while((status = (*(frontBuffAddr + 3) & 0x01)) != 0) {
        continue;
    }
    
}

void writeText(int x, int y, char* text) {
    volatile char * character_buffer = (char *)FPGA_CHAR_BASE;
    int offset = (y << 7) + x;

    while (*(text)) {
        *(character_buffer + offset) = *(text); // write to the character buffer
        ++text;
        ++offset;
    }
}

void clearText() 
{
    volatile char * character_buffer = (char *)FPGA_CHAR_BASE;
    char text = '\0';

    for (int x = 0; x < 80; x++) 
    {
        for (int y = 0; y < 60; y++) 
        {
            int offset = (y << 7) + x;
            *(character_buffer + offset) = text;
        }
    }       
}

void getNumString(int a, char score[], int size) 
{
    int temp;
    int j = 10;

    for (int i = (size-2); i >= 0; i--) 
    {
        temp = a % j;
        temp = temp / (j / 10);
        
        score[i] = (temp + '0');
        j = j * 10;
    }
}