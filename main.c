#define _CRT_SECURE_NO_WARNINGS 
#include<stdio.h> 
#include<graphics.h> 
#include<conio.h> 
#include "tools.h"
#include <vector>
using namespace std;
#define WIN_WIDTH 1012 
#define WIN_HEIGHT 396
IMAGE imgBgs[3]; 
int bgX[3];//背景图片的x坐标 
int bgSpeed[3] = { 1,2,4};
IMAGE imgHeros[12]; 
int heroX;
int heroY; 
int heroIndex;//图片帧序号 
bool heroJump;
int jumpHeightMax;
int heroJumpOff;
int update;

//IMAGE imgTortoise;
//int torToiseX;
//int torToiseY;
//bool torToiseExist = false;

typedef enum {
	TORTOISE, //乌龟 0
	LION,//狮子 1
	OBSTACLE_TYPE_COUNT // 2
}obstacle_type;
vector<vector<IMAGE>> obstacleImges;//存放所有障碍物的图片
typedef struct obstacle {
	obstacle_type type;//障碍物类型
	int imgIndex;
	int x, y;
	int speed;
	int power;
	bool exit;
	IMAGE img[12];
}obstacle_t;
void init() {
	initgraph(WIN_WIDTH, WIN_HEIGHT); char name[64]; for (int i = 0; i < 3; ++i) {
		sprintf(name, "res/bg%03d.png", i + 1); loadimage(&imgBgs[i], name);//属性，多字节字符集 bgX[i] = 0; } //加载玩家图片奔跑素材 for (int i = 0; i < 12; ++i) { sprintf(name, "res/hero%d.png", i + 1); loadimage(&imgHeros[i], name); } heroX = WIN_WIDTH * 0.5 - imgHeros[0].getwidth()*0.5; heroY = 345 - imgHeros[0].getheight(); heroIndex = 0; heroJump = false;
		jumpHeightMax = 345 - imgHeros[0].getheight() - 120;
		heroJumpOff = -4;
		update = true;

		loadimage(&imgTortoise, "res/t1.png");
		torToiseExist = false;
		torToiseY = 345 - imgTortoise.getheight() + 5;
		Copy
	}
	void fly() {
		for (int i = 0; i < 3; ++i) { bgX[i] -= bgSpeed[i]; if (bgX[i] < -WIN_WIDTH) { bgX[i] = 0; } }
		//实现跳跃
		if (heroJump) {
			if (heroY < jumpHeightMax) {
				heroJumpOff = 4;
			}

			heroY += heroJumpOff;

			if (heroY > 345 - imgHeros[0].getheight()) {
				heroJump = false;
				heroJumpOff = -4;
			}
		}
		else {
			heroIndex = (heroIndex + 1) % 12;//循环呈现这几张图片
		}

		static int frameCount = 0;
		static int torToiseFre = 100;
		frameCount++;
		if (frameCount > torToiseFre) {
			frameCount = 0;
			if (!torToiseExist) {
				torToiseExist = true;
				torToiseX = WIN_WIDTH;
				torToiseFre = rand() % 300 + 200;
			}
		}

		if (torToiseExist)
		{
			torToiseX -= bgSpeed[2];
			if (torToiseX < -imgTortoise.getwidth())
			{
				torToiseExist = false;
			}
		}
		Copy
	} //游戏背景(坐标) 
	void updateBg() { putimagePNG2(bgX[0], 0, &imgBgs[0]); putimagePNG2(bgX[1], 119, &imgBgs[1]); putimagePNG2(bgX[2], 330, &imgBgs[2]); }
	void jump() { heroJump = true; update = true; }
	//按键输入(先判断有没有按键输入) 
	void keyEvent() { char ch; if (kbhit()) {//有按键输入返回true
		ch=getch(); if (ch ==' ') { jump(); } } }
	void updateEnemy() { if (torToiseExist) { putimagePNG2(torToiseX, torToiseY, WIN_WIDTH, &imgTortoise); } }
	int main() {
		init(); int timer = 0; while (1) {
			keyEvent(); timer += getDelay(); if (timer > 30) { timer = 0; update = true; }
			if (update)
			{
				update = false;
				BeginBatchDraw();
				updateBg();
				putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
				updateEnemy();
				EndBatchDraw();//解决频闪
				fly();
			}
		}


		system("pause");

		return 0;
		Copy
	}
