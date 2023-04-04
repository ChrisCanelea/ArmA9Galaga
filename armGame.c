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
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define SCREEN_W 320
#define SCREEN_H 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

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
    short int sprite[16][15];
} gameObject;

//basic functions
void swap(int* a, int* b);

//gameObject functions
gameObject createObject(int length_, int height_);
void setObjectPos(gameObject* object, int x_, int y_);
void drawObject(gameObject object);

//assets
void initializePlayer(gameObject* object);
void initializeBossGalaga(gameObject* object);
void initializeGoeiGalaga(gameObject* object);

//rect functions
rect createRect(int x_, int y_, int length_, int height_, short int colour_);
void setRectPos(rect* rect_, int x_, int y_);
void drawRect(rect rect_);
void eraseOldRect(rect rect_);
void eraseRect(rect rect_);
bool x_outOfBounds(rect rect_, int leftBound, int rightBound);
bool y_outOfBounds(rect rect_, int upperBound, int lowerBound);
bool contact(rect topLeft, rect bottomRight);
void updatePos(rect* _rect);
void setBottom(rect* _rect, int topBoundary);

void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int colour);
void waitForVSync(volatile int status, volatile int* frontBuffAddr);

volatile int pixel_buffer_start; // global variable
int pixelSize = 2;

int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    volatile int* keysBaseAddr = (int*) KEY_BASE;
    
    // declare other variables(not shown)
    volatile int* frontBuffAddr = (int*)0xFF203020;//front buffer register address
    volatile int* backBuffAddr = (int*)(frontBuffAddr + 1);
    volatile int status = (*(frontBuffAddr + 3) & 0x01);//1 if VGA not done frame, 0 if VGA done frame
    int numBoxes = 8;
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

    //initialize player object
    gameObject player = createObject(15, 16);
    initializePlayer(&player);

    gameObject bossGalaga = createObject(15, 16);
    initializeBossGalaga(&bossGalaga);
    setObjectPos(&bossGalaga, 20, 0);
    
    gameObject goeiGalaga = createObject(15, 16);
    initializeGoeiGalaga(&goeiGalaga);
    setObjectPos(&goeiGalaga, 40, 0);

    //initialize and draw a rect
    rect square = createRect(20, 20, 20, 20, YELLOW);
    rect ground = createRect(0, SCREEN_H - 20, SCREEN_W, 20, GREEN);
    rect platform = createRect(200, SCREEN_H - 100, 50, 20, RED);
    rect test = createRect(0, 0, 20, 20, YELLOW);

    drawRect(square);
    drawRect(ground);
    drawRect(platform);
    drawObject(player);
    drawObject(bossGalaga);
    drawObject(goeiGalaga);

    *frontBuffAddr = 1;
    wait_for_vsync(status, frontBuffAddr);
    pixel_buffer_start = *backBuffAddr;

    drawRect(ground);
    drawRect(platform);

    while (1)
    {
        //Erase any boxes and lines that were drawn in the last iteration
        eraseOldRect(square);

        if ((square.old_y + square.height) > ground.top) {
            drawRect(ground);
        }

        //set "old" values to current values
        square.old_x = square.x;
        square.old_y = square.y;

        if (contact(square, ground)) {
            square.dy = 0;
            setBottom(&square, ground.top);
            if ((*keysBaseAddr & 0x0004) == 4) {
                square.dy = -10;
            }
        } /*else if (contact(square, platform)) {
            drawRect(test);
        }*/ else {
            square.dy += 1;
        }
        
        drawObject(player);
        drawObject(bossGalaga);
        drawObject(goeiGalaga);

        // if ((square.bottom >= platform.top)/* && (square.right >= platform.left) && (square.left <= platform.right)*/) {
        //     square.dy = 0;
        // }

        /*if (contact(square, platform)){
            square.dy = 0;
            setBottom(&square, platform.top);
            if ((*keysBaseAddr & 0x0004) == 4) {
                square.dy = -10;
            }
        }*/

        if ((*keysBaseAddr & 0x0001) == 1) {
            square.dx = 3;
        } else if ((*keysBaseAddr & 0x0002) == 2) {
            square.dx = -3;
        } else {
            square.dx = 0;
        }

        if (x_outOfBounds(square, 0, 319)) {
            square.dx = square.dx * -1;
        }
        
        if (y_outOfBounds(square, 0, 239)) {
            square.dy = square.dy * -1;
        }

        updatePos(&square);//update current positions
        drawRect(square);
        //drawRect(ground);

        //buffer stuff
        *frontBuffAddr = 1;//swap buffers
        wait_for_vsync(status, frontBuffAddr); //wait for VGA vertical sync
        pixel_buffer_start = *backBuffAddr; // new back buffer
    }
}

// code for subroutines (not shown)

//gameObject functions
gameObject createObject(int length_, int height_) {
    gameObject object;
    object.length = length_;
    object.height = height_;
    object.hitbox = createRect(0, 0, length_, height_, 0);
    //object.sprite = (short int**) malloc((length_ * height_) * (sizeof(short int)));
    return object;
}

void setObjectPos(gameObject* object, int x_, int y_) {
    setRectPos(&object->hitbox, x_, y_);
}

void drawObject(gameObject object) {
    for (int row = 0; row < object.height; row++) {
        for (int col = 0; col < object.length; col++) {
            //plot_pixel(object.hitbox.x + col, object.hitbox.y + row, object.sprite[row][col]);
            for (int i = 0; i < pixelSize; i++) {
                for (int j = 0; j < pixelSize; j++) {
                    plot_pixel(pixelSize*(object.hitbox.x + col) + i, pixelSize*(object.hitbox.y + row) + j, object.sprite[row][col]);
                }
            }
        }
    }
}

void initializePlayer(gameObject* object) {
    short int array[16][15] = {{    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0},
                               {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0},
                               {    0,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,     0,       0,     0,     0},
                               {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0},
                               {    0,     0,     0,     0,     0,     0, WHITE, WHITE, WHITE,     0,     0,     0,       0,     0,     0},
                               {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0},
                               {    0,     0,     0,   RED,     0,     0, WHITE, WHITE, WHITE,     0,     0,   RED,       0,     0,     0},
                               {    0,     0,     0, WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE,     0, WHITE,       0,     0,     0},
                               {  RED,     0,     0, WHITE,  BLUE, WHITE, WHITE,   RED, WHITE, WHITE,  BLUE, WHITE,       0,     0,   RED},
                               {  RED,     0,     0,  BLUE, WHITE, WHITE,   RED,   RED,   RED, WHITE, WHITE,  BLUE,       0,     0,   RED},
                               {WHITE,     0,     0, WHITE, WHITE, WHITE,   RED, WHITE,   RED, WHITE, WHITE,  WHITE,      0,     0, WHITE},
                               {WHITE,     0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,  WHITE,  WHITE,     0, WHITE},
                               {WHITE, WHITE, WHITE, WHITE, WHITE,   RED, WHITE, WHITE, WHITE,   RED, WHITE,  WHITE,  WHITE, WHITE, WHITE},
                               {WHITE, WHITE, WHITE,     0,   RED,   RED, WHITE, WHITE, WHITE,   RED,   RED,      0,  WHITE, WHITE, WHITE},
                               {WHITE, WHITE,     0,     0,   RED,   RED,     0, WHITE,     0,   RED,   RED,      0,      0, WHITE, WHITE},
                               {WHITE,     0,     0,     0,     0,     0,     0, WHITE,     0,     0,     0,      0,      0,     0, WHITE}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[row][col] = array[row][col];
        }
    }

}

void initializeBossGalaga(gameObject* object) {
    short int array[16][15] = {{    0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,   CYAN,      0,   CYAN,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,  CYAN,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,  CYAN,       0,      0,     0},
                               {    0,      0,      0,     0,  CYAN, ORANGE, ORANGE,   CYAN, ORANGE, ORANGE,  CYAN,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,   CYAN,   CYAN,   CYAN,   CYAN,   CYAN,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,  CYAN, YELLOW, YELLOW,   CYAN, YELLOW, YELLOW,  CYAN,     0,       0,      0,     0},
                               {    0,      0,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,      0,     0},
                               { CYAN,   CYAN,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,   CYAN,  CYAN},
                               {    0,   CYAN,   CYAN,  CYAN,  CYAN, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,  CYAN,  CYAN,    CYAN,   CYAN,     0},
                               {    0,   CYAN, ORANGE,  CYAN,  CYAN,      0, ORANGE,      0, ORANGE,      0,  CYAN,  CYAN,  ORANGE,   CYAN,     0},
                               { CYAN,   CYAN, ORANGE,  CYAN,     0,      0, ORANGE,      0, ORANGE,      0,     0,  CYAN,  ORANGE,   CYAN,  CYAN},
                               { CYAN, ORANGE,   CYAN,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,    CYAN, ORANGE,  CYAN},
                               { CYAN, ORANGE, ORANGE,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,  ORANGE, ORANGE,  CYAN},
                               { CYAN, ORANGE, ORANGE,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,  ORANGE, ORANGE,  CYAN},
                               { CYAN,   CYAN,   CYAN,  CYAN,     0,      0,      0,      0,      0,      0,     0,  CYAN,    CYAN,   CYAN,  CYAN},
                               {    0,   CYAN,   CYAN,     0,     0,      0,      0,      0,      0,      0,     0,     0,    CYAN,   CYAN,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[row][col] = array[row][col];
        }
    }

}

void initializeGoeiGalaga(gameObject* object) {
    short int array[16][15] = {{    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,   RED,     0,      0,   BLUE,      0,   BLUE,      0,     0,   RED,       0,      0,     0},
                               {    0,    RED,    RED,   RED,     0,      0,   BLUE,      0,   BLUE,      0,     0,   RED,     RED,    RED,     0},
                               {    0,    RED,    RED,   RED,     0,  WHITE,    RED,  WHITE,    RED,  WHITE,     0,   RED,     RED,    RED,     0},
                               {    0,    RED,    RED,   RED,     0,  WHITE,  WHITE,  WHITE,  WHITE,  WHITE,     0,   RED,     RED,    RED,     0},
                               {    0,      0,    RED,   RED,   RED,    RED,  WHITE,  WHITE,  WHITE,    RED,   RED,   RED,     RED,      0,     0},
                               {    0,      0,      0,   RED,   RED,    RED,   BLUE,   BLUE,   BLUE,    RED,   RED,   RED,       0,      0,     0},
                               {    0,      0,    RED,   RED,   RED,    RED,   BLUE,   BLUE,   BLUE,    RED,   RED,   RED,     RED,      0,     0},
                               {    0,    RED,    RED,   RED,   RED,    RED,  WHITE,  WHITE,  WHITE,    RED,   RED,   RED,     RED,    RED,     0},
                               {    0,      0,    RED,   RED,   RED,      0,   BLUE,   BLUE,   BLUE,      0,   RED,   RED,     RED,      0,     0},
                               {    0,      0,      0,     0,   RED,      0,      0,      0,      0,      0,   RED,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0},
                               {    0,      0,      0,     0,     0,      0,      0,      0,      0,      0,     0,     0,       0,      0,     0}
    };
    for (int row = 0; row < object->height; row++) {
        for (int col = 0; col < object->length; col++) {
            object->sprite[row][col] = array[row][col];
        }
    }

}


void setBottom(rect* _rect, int topBoundary) {
    _rect->y = topBoundary - _rect->height;
    _rect->top = _rect->y;
    _rect->bottom = topBoundary;
}

void updatePos(rect* _rect) {
    _rect->x += _rect->dx;
    _rect->y += _rect->dy;
    _rect->top += _rect->dy;
    _rect->bottom += _rect->dy;
    _rect->left += _rect->dx;
    _rect->right += _rect->x;
}

bool contact(rect topLeft, rect bottomRight) 
{
    if ((topLeft.right >= bottomRight.left) && (topLeft.bottom >= bottomRight.top)) {
        return true;
    } else {
        return false;
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

rect createRect(int x_, int y_, int length_, int height_, short int colour_) 
{
    rect box = {x_, y_, length_, height_, colour_, 0, 0, 0, 0, x_, x_ + length_, y_, y_ + height_};
    return box;
}

void setRectPos(rect* rect_, int x_, int y_) {
    rect_->old_x = rect_->x;
    rect_->old_y = rect_->y;
    rect_->x = x_;
    rect_->y = y_;
    rect_->left = x_;
    rect_->right = x_ + rect_->length;
    rect_->top = y_;
    rect_->bottom = y_ + rect_->height;
}

void drawRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.x, rect_.y + i, rect_.x + rect_.length - 1, rect_.y + i, rect_.colour);
    }
}

void eraseOldRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.old_x, rect_.old_y + i, rect_.old_x + rect_.length - 1, rect_.old_y + i, 0);
    }
}

void eraseRect(rect rect_) 
{
    for (int i = 0; i < rect_.height; i++) {
        draw_line(rect_.x, rect_.y + i, rect_.x + rect_.length - 1, rect_.y + i, 0);
    }
}

void wait_for_vsync(volatile int status, volatile int* frontBuffAddr) 
{

    while((status = (*(frontBuffAddr + 3) & 0x01)) != 0) {
        continue;
    }
    
}

void clear_screen() 
{
    for (int i = 0; i < SCREEN_W; i++) {
        for (int j = 0; j < SCREEN_H; j++) {
            plot_pixel(i, j, 0);//plot black pixels
        }
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

void swap(int* a, int* b) 
{
    int temp = *b;
    *b = *a;
    *a = temp;
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}
