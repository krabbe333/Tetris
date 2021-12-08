#ifndef VIDEO_DEMO_H_
#define VIDEO_DEMO_H_

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "xil_types.h"

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

#define DEMO_PATTERN_0 0
#define DEMO_PATTERN_1 1

#define DEMO_MAX_FRAME (1920*1080*3)
#define DEMO_STRIDE (1920 * 3)

/*
 * Configure the Video capture driver to start streaming on signal
 * detection
 */
#define DEMO_START_ON_DET 1

/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */
//Game Logic
void InitializeGame();
void Run();
void PrintMenu();
void Rotate(int block);
void shift_left(int block);
void shift_right(int block);
void change_block();
void nextBlock(u8 *frame, u32 stride);
void drop(int block);
void StopGame();
void ryder();

//array
void array(int x, int y);
void array_O();
void array_I_h();
void array_I_v();
void array_L_1();
void array_L_2();
void array_L_3();
void array_L_4();
void array_J_1();
void array_J_2();
void array_J_3();
void array_J_4();
void array_S_1();
void array_S_2();
void array_Z_1();
void array_Z_2();
void array_T_1();
void array_T_2();
void array_T_3();
void array_T_4();

//Block functions
//I-block
void I_block_h(u8 *frame, u32 stride, int nextX, int nextY);
void I_block_v(u8 *frame, u32 stride);
//O-block
void O_block(u8 *frame, u32 stride, int nextX, int nextY);
//L-block
void L_block_1(u8 *frame, u32 stride, int nextX, int nextY);
void L_block_2(u8 *frame, u32 stride);
void L_block_3(u8 *frame, u32 stride);
void L_block_4(u8 *frame, u32 stride);
//J-block
void J_block_1(u8 *frame, u32 stride, int nextX, int nextY);
void J_block_2(u8 *frame, u32 stride);
void J_block_3(u8 *frame, u32 stride);
void J_block_4(u8 *frame, u32 stride);
//S-block
void S_block_1(u8 *frame, u32 stride, int nextX, int nextY);
void S_block_2(u8 *frame, u32 stride);
//Z-block
void Z_block_1(u8 *frame, u32 stride, int nextX, int nextY);
void Z_block_2(u8 *frame, u32 stride);
//J-block
void T_block_1(u8 *frame, u32 stride, int nextX, int nextY);
void T_block_2(u8 *frame, u32 stride);
void T_block_3(u8 *frame, u32 stride);
void T_block_4(u8 *frame, u32 stride);
//Initialize background
void Background(u8 *frame, u32 width, u32 height, u32 stride);
void clear(u8 *frame, u32 stride);
void update(u8 *frame, u32 stride, int i);
void levelNumber(int level);
void level();
void numbers(int score, int high);
void score();
void Menu(u8 *frame, u32 width, u32 height, u32 stride);
void highScore();
//Initialize game field
void gameField();
void Draw(u8 *frame, u32 stride, u32 fwidth, u32 fheight, u32 fxcoi, u32 fycoi, double fRed, double fGreen, double fBlue);


/* ------------------------------------------------------------ */

/************************************************************************/

#endif /* VIDEO_DEMO_H_ */
