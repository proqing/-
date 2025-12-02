# 2025c游戏项目



#define  _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<graphics.h>
#include "tools.h"

#define WIN_WIDTH 1012
#define WIN_HEIGHT 396

IMAGE imgBgs[3];
int bgX[3];//背景图片的x坐标
int bgSpeed[3] = { 1,2,4};

IMAGE imgHeros[12];
int heroX;
int heroY;
int heroIndex;//图片帧序号

void init() 
{
	initgraph(WIN_WIDTH, WIN_HEIGHT);
	char name[64];
	for (int i = 0; i < 3; ++i)
	{
		sprintf(name, "res/bg%03d.png",i+1);
		loadimage(&imgBgs[i], name);//属性，多字节字符集
		bgX[i] = 0;
	}
	//加载玩家图片奔跑素材
	for (int i = 0; i < 12; ++i)
	{
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHeros[i], name);
	}
	heroX = WIN_WIDTH * 0.5 - imgHeros[0].getwidth()*0.5;
	heroY = 345 - imgHeros[0].getheight();
	heroIndex = 0;
}

void fly()
{
	for (int i = 0; i < 3; ++i)
	{
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WIN_WIDTH) {
			bgX[i] = 0;
		}
	}
	heroIndex = (heroIndex + 1) % 12;//循环呈现这几张图片
}
//游戏背景(坐标)
void updateBg()
{
	putimagePNG2(bgX[0], 0, &imgBgs[0]);
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}

int main()
{
	init();

	while (1)
	{
		BeginBatchDraw();
		updateBg();
		putimagePNG2(heroX,heroY,&imgHeros[heroIndex]);
		EndBatchDraw();//解决频闪
		fly();
		Sleep(30);
	}
	

	system("pause");

	return 0;
}
