<img width="702" height="1133" alt="image" src="https://github.com/user-attachments/assets/afe3e344-2dc6-4a74-8800-f8b5b2ca8790" />
上面是程序结构，做的时候可以直接把函数内容收缩，看起来更简洁


#define  _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<graphics.h>
#include<conio.h>
#include<windows.h>
#include<math.h>
#include "tools.h"
#include<vector>
using namespace std;

#define WIN_WIDTH 1012
#define WIN_HEIGHT 396
#define OBSTACLE_COUNT 2

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

typedef enum {
    TORTOISE, //乌龟 0
    LION,//狮子 1
    OBSTACLE_TYPE_COUNT
}obstacle_type;
vector<vector<IMAGE>> obstacleImgs;//存放所有障碍物的图片
typedef struct obstacle {
    obstacle_type type;//障碍物类型
    int imgIndex;
    int x, y;
    int speed;
    int power;
    bool exist;
    IMAGE img[12];
}obstacle_t;

obstacle_t obstacles[OBSTACLE_COUNT];

int selectedCharacter = 0;  // 选择的角色（0=角色1, 1=角色2）
/**
 * @brief 角色选择函数（针对已有1012×396窗口）
 * @param img1 角色1图片对象
 * @param img2 角色2图片对象
 * @param char1_name 角色1名称
 * @param char2_name 角色2名称
 * @return 选择的角色索引 (0或1)，-1表示取消
 */
int selectCharacterInWindow(IMAGE* img1, IMAGE* img2,
    const char* char1_name,
    const char* char2_name)
{
    // 窗口尺寸
    int windowWidth = 1012;
    int windowHeight = 396;

    // 图片显示尺寸（根据窗口调整）
    int img_width = 130;
    int img_height = 130;

    // 计算角色位置（居中布局）
    int padding = 50;
    int total_width = 2 * img_width + padding;
    int start_x = (windowWidth - total_width) / 2;

    int char1_x = start_x;
    int char2_x = start_x + img_width + padding;
    int char_y = 120;  // 垂直居中偏上

    int currentSelection = 0;  // 0: 未选择, 1: 角色1, 2: 角色2

    // 动态效果参数
    static int glowStep = 0;    // 发光动画步长
    COLORREF bgColor = 0xFFD700; // 淡蓝色背景（替代纯白）

    // 清屏并初始化背景（删除flushbatchdraw，适配旧版EasyX）
    cleardevice();
    setbkcolor(bgColor);
    settextcolor(BLACK);

    while (1) {
        BeginBatchDraw();
        
        // -------------------------- 1. 绘制渐变背景（替代纯白） --------------------------
        // 从上到下的淡蓝渐变（简化版，避免性能问题）
        setfillcolor(bgColor);
        solidrectangle(0, 0, windowWidth, windowHeight);
        // 顶部浅蓝到底部白色的渐变（用矩形条模拟，更兼容）
        for (int y = 0; y < windowHeight; y += 5) {
            COLORREF gradColor = RGB(
                240 + (y * 15) / windowHeight,
                248 + (y * 7) / windowHeight,
                255
            );
            setfillcolor(gradColor);
            solidrectangle(0, y, windowWidth, y + 5);
        }

        // -------------------------- 2. 绘制美化后的标题 --------------------------
        settextstyle(32, 0, "站酷高端黑"); // 加粗+放大
        settextcolor(0x5C3317);  // 巧克力黑
        int titleWidth = textwidth("请选择你的解药");
        int titleX = (windowWidth - titleWidth) / 2;
        // 标题描边（增加立体感，兼容所有版本）
        settextcolor(0x191970); // 深蓝描边
        outtextxy(titleX - 1, 39, "请选择你的解药");
        outtextxy(titleX + 1, 39, "请选择你的解药");
        outtextxy(titleX - 1, 41, "请选择你的解药");
        outtextxy(titleX + 1, 41, "请选择你的解药");
        // 主标题
        settextcolor(0x4169E1);
        outtextxy(titleX, 40, "请选择你的解药");

        // -------------------------- 3. 动态发光画框（核心美化） --------------------------
        glowStep = (glowStep + 1) % 100; // 循环动画
        int glowWidth = 2 + (sin(glowStep * 0.06) * 1.5); // 发光宽度动态变化
        // 兼容旧版EasyX：如果没有HSLtoRGB，替换为固定色
        COLORREF selectGlowColor = (glowStep % 2 == 0) ? 0x32CD32 : 0x228B22;

        // 绘制角色1
        if (currentSelection == 1) {
            // 选中状态：动态发光框+淡绿背景
            setlinecolor(selectGlowColor);
            setlinestyle(PS_SOLID, (int)glowWidth); // 动态宽度边框（转int兼容）
            setfillcolor(0xE8FFE8); // 更柔和的浅绿色
        }
        else {
            // 未选中状态：浅灰边框+极浅灰背景
            setlinecolor(0xD3D3D3); // 浅灰
            setlinestyle(PS_SOLID, 2);
            setfillcolor(0xFAFAFA); // 极浅灰
        }
        // 圆角矩形（增加圆角半径，更圆润）
        fillroundrect(char1_x - 8, char_y - 8,
            char1_x + img_width + 8, char_y + img_height + 8, 12, 12);
        rectangle(char1_x - 8, char_y - 8,
            char1_x + img_width + 8, char_y + img_height + 8); // 外边框

        // 绘制角色1图片（修正：用标准3参数putimage，删除错误的5参数写法）
        // 简易阴影：先画黑色偏移，再画原图（兼容所有版本）
        setbkmode(TRANSPARENT); // 文字透明，图片也适配
        putimage(char1_x + 2, char_y + 2, img1); // 阴影（偏移2像素）
        putimage(char1_x, char_y, img1); // 主图

        // 绘制角色1名称（美化）
        settextcolor(0x2F4F4F); // 深青灰（替代纯黑）
        settextstyle(24, 0, "微软雅黑 Bold"); // 加粗
        int name1Width = textwidth(char1_name);
        outtextxy(char1_x + (img_width - name1Width) / 2,
            char_y + img_height + 12, char1_name);

        // -------------------------- 4. 角色2绘制（同角色1逻辑） --------------------------
        if (currentSelection == 2) {
            setlinecolor(selectGlowColor);
            setlinestyle(PS_SOLID, (int)glowWidth);
            setfillcolor(0xE8FFE8);
        }
        else {
            setlinecolor(0xD3D3D3);
            setlinestyle(PS_SOLID, 2);
            setfillcolor(0xFAFAFA);
        }
        fillroundrect(char2_x - 8, char_y - 8,
            char2_x + img_width + 8, char_y + img_height + 8, 12, 12);
        rectangle(char2_x - 8, char_y - 8,
            char2_x + img_width + 8, char_y + img_height + 8);

        // 角色2图片（标准3参数putimage）
        putimage(char2_x + 2, char_y + 2, img2); // 阴影
        putimage(char2_x, char_y, img2); // 主图

        // 角色2名称
        settextcolor(0x2F4F4F);
        outtextxy(char2_x + (img_width - textwidth(char2_name)) / 2,
            char_y + img_height + 12, char2_name);

        // -------------------------- 5. 美化提示文字（增加半透背景） --------------------------
        settextstyle(18, 0, "微软雅黑");
        char tipText[32];
        COLORREF tipColor;
        if (currentSelection == 0) {
            strcpy(tipText, "点击选择角色");
            tipColor = 0xFF4500; // 橙红
        }
        else {
            strcpy(tipText, "请按空格开始");
            tipColor = 0x228B22; // 森林绿
        }

        // 提示文字半透背景（圆角矩形，兼容所有版本）
        int tipWidth = textwidth(tipText);
        int tipX = (windowWidth - tipWidth) / 2;
        // 半透背景：用浅白色填充（模拟半透，避免Alpha通道兼容问题）
        setfillcolor(0xFFFFFF); // 白色
        setlinecolor(0xE0E0E0); // 浅灰边框
        fillroundrect(tipX - 10, 320, tipX + tipWidth + 10, 345, 8, 8);
        // 绘制提示文字
        settextcolor(tipColor);
        outtextxy(tipX, 325, tipText);

        // ESC取消提示（同样增加半透背景）
        const char* escText = "按ESC取消";
        int escWidth = textwidth(escText);
        int escX = (windowWidth - escWidth) / 2;
        setfillcolor(0xFFFFFF);
        setlinecolor(0xE0E0E0);
        fillroundrect(escX - 10, 350, escX + escWidth + 10, 375, 8, 8);
        settextcolor(0x4682B4); // 钢蓝色保留
        outtextxy(escX, 355, escText);

        // -------------------------- 6. 输入处理（保留原有逻辑） --------------------------
        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            if (msg.message == WM_LBUTTONDOWN) {
                // 检查点击角色1
                if (msg.x >= char1_x && msg.x <= char1_x + img_width &&
                    msg.y >= char_y && msg.y <= char_y + img_height) {
                    currentSelection = 1;
                }
                // 检查点击角色2
                else if (msg.x >= char2_x && msg.x <= char2_x + img_width &&
                    msg.y >= char_y && msg.y <= char_y + img_height) {
                    currentSelection = 2;
                }
            }
            else if (msg.message == WM_KEYDOWN) {
                // 空格键确认选择
                if (msg.vkcode == VK_SPACE && currentSelection > 0) {
                    EndBatchDraw();
                    return currentSelection - 1;  // 返回0或1
                }
                // ESC键取消
                else if (msg.vkcode == VK_ESCAPE) {
                    EndBatchDraw();
                    return -1;  // 取消选择
                }
                // 数字键1或左键选择角色1
                else if (msg.vkcode == '1' || msg.vkcode == VK_LEFT) {
                    currentSelection = 1;
                }
                // 数字键2或右键选择角色2
                else if (msg.vkcode == '2' || msg.vkcode == VK_RIGHT) {
                    currentSelection = 2;
                }
            }
        }

        EndBatchDraw(); // 解决频闪
        Sleep(15);
        glowStep++; // 更新动画步长
    }
}

/**
 * @brief 游戏说明界面（展示说明图+开始/返回按键）
 * @param imgTip 游戏说明图片对象
 * @return 0=开始游戏，1=返回角色选择
 */
int showGameTips(IMAGE* imgTip) {
    int windowWidth = 1012;
    int windowHeight = 396;

    // 说明图显示位置（居中）
    int tipX = (windowWidth - imgTip->getwidth()) / 2;
    int tipY = 15;

    // 按钮区域定义
    // 开始游戏按钮（右下）
    int btnStartX = windowWidth - 280;
    int btnStartY = windowHeight - 230;
    int btnStartW = 150;
    int btnStartH = 50;
    // 返回按钮（左下）
    int btnBackX = 130;
    int btnBackY = windowHeight - 230;
    int btnBackW = 150;
    int btnBackH = 50;

    // 按钮高亮标记
    bool hoverStart = false;
    bool hoverBack = false;

    while (1) {
        BeginBatchDraw();
        cleardevice();

        // 1. 绘制背景（淡蓝色渐变，和角色选择界面统一）
        setfillcolor(0xFFD700);
        solidrectangle(0, 0, windowWidth, windowHeight);
        for (int y = 0; y < windowHeight; y += 5) {
            COLORREF gradColor = RGB(240 + (y * 15) / windowHeight,
                248 + (y * 7) / windowHeight, 255);
            setfillcolor(gradColor);
            solidrectangle(0, y, windowWidth, y + 5);
        }

        // 2. 绘制游戏说明图片
        putimage(tipX, tipY, imgTip);

        // 3. 绘制开始游戏按钮（带hover高亮）
        if (hoverStart) {
            setfillcolor(0xE8FFE8); // 浅绿高亮
            setlinecolor(0x32CD32); // 绿色边框
        }
        else {
            setfillcolor(0xFAFAFA); // 浅灰
            setlinecolor(0xD3D3D3); // 灰色边框
        }
        fillroundrect(btnStartX - 5, btnStartY - 5,
            btnStartX + btnStartW + 5, btnStartY + btnStartH + 5, 20, 20);
        // 按钮文字
        settextcolor(0x2F4F4F);
        settextstyle(30, 0, "SimSun Bold");
        int startTextW = textwidth("开始游戏");
        outtextxy(btnStartX + (btnStartW - startTextW) / 2,
            btnStartY + 10, "开始游戏");

        // 4. 绘制返回按钮（带hover高亮）
        if (hoverBack) {
            setfillcolor(0xFFE8E8); // 浅红高亮
            setlinecolor(0xCD3232); // 红色边框
        }
        else {
            setfillcolor(0xFAFAFA); // 浅灰
            setlinecolor(0xD3D3D3); // 灰色边框
        }
        fillroundrect(btnBackX - 5, btnBackY - 5,
            btnBackX + btnBackW + 5, btnBackY + btnBackH + 5, 20, 20);
        // 按钮文字
        int backTextW = textwidth("返回选择");
        outtextxy(btnBackX + (btnBackW - backTextW) / 2,
            btnBackY + 10, "返回选择");

        // 5. 处理鼠标/键盘输入
        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            // 鼠标交互
            if (msg.message == WM_MOUSEMOVE) {
                // 检测鼠标是否悬浮在按钮上
                hoverStart = (msg.x >= btnStartX && msg.x <= btnStartX + btnStartW &&
                    msg.y >= btnStartY && msg.y <= btnStartY + btnStartH);
                hoverBack = (msg.x >= btnBackX && msg.x <= btnBackX + btnBackW &&
                    msg.y >= btnBackY && msg.y <= btnBackY + btnBackH);
            }
            else if (msg.message == WM_LBUTTONDOWN) {
                // 点击开始游戏
                if (hoverStart) {
                    EndBatchDraw();
                    return 0; // 开始游戏
                }
                // 点击返回
                if (hoverBack) {
                    EndBatchDraw();
                    return 1; // 返回角色选择
                }
            }
            // 键盘交互
            else if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == VK_SPACE || msg.vkcode == VK_RETURN) {
                    EndBatchDraw();
                    return 0; // 空格/回车开始游戏
                }
                else if (msg.vkcode == VK_ESCAPE) {
                    EndBatchDraw();
                    return 1; // ESC返回
                }
            }
        }

        EndBatchDraw();
        Sleep(10);
    }
}


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

    IMAGE character1, character2;
    loadimage(&character1, "character1.png", 130, 130);
    loadimage(&character2, "character2.png", 130, 130);
    // 循环处理角色选择+游戏说明（支持返回重选）
    while (true) {
        // 1. 调用角色选择函数
        selectedCharacter = selectCharacterInWindow(&character1, &character2, "role1", "role2");

        // 处理取消选择（直接退出）
        if (selectedCharacter < 0) {
            closegraph();
            exit(0);
        }

        // 2. 加载游戏说明图片（替换为你的说明图路径）
        IMAGE imgTip;
        loadimage(&imgTip, "res/说明.png"); // 游戏说明图（建议尺寸：900x250左右）

        // 3. 显示游戏说明界面，获取用户选择
        int tipResult = showGameTips(&imgTip);

        // 4. 处理说明界面返回结果
        if (tipResult == 0) {
            // 开始游戏：跳出循环，继续初始化
            break;
        }
        else {
            // 返回选择：重新进入角色选择界面
            continue;
        }
    }
	//加载玩家图片奔跑素材
    if (selectedCharacter == 1)
    {
        for (int i = 0; i < 12; ++i)
        {
            sprintf(name, "res/hero%d.png", i + 1);
            loadimage(&imgHeros[i], name);
        }
    }
    else
    {
        for (int i = 0; i < 12; ++i)
        {
            sprintf(name, "res/bark/hero%d.png", i + 1);
            loadimage(&imgHeros[i], name);
        }
    }
    
    heroX = WIN_WIDTH * 0.5 - imgHeros[0].getwidth() * 0.5; 
    heroY = 345 - imgHeros[0].getheight(); 
    heroIndex = 0; 
    heroJump = false;

	jumpHeightMax = 345 - imgHeros[0].getheight() - 120;
	heroJumpOff = -4;
	update = true;

    IMAGE imgTort;
    loadimage(&imgTort, "res/t1.png");
    vector<IMAGE> imgTortArray;
    imgTortArray.push_back(imgTort);
    obstacleImgs.push_back(imgTortArray);

    IMAGE imgLion;
    vector<IMAGE> imgLionArray;
    for (int i = 0; i < 6; ++i)
    {
        sprintf(name, "res/p%d.png", i + 1);
        loadimage(&imgLion, name);
        imgLionArray.push_back(imgLion);
    }
    obstacleImgs.push_back(imgLionArray);
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        obstacles[i].exist = 0;
    }
}

void createObstacle()
{
    int i;
    for (i = 0; i < OBSTACLE_COUNT; ++i)
    {
        if (obstacles[i].exist == false)
        {
            break;
        }
    }
    if (i >= OBSTACLE_COUNT) {
        return;
    }
    obstacles[i].exist = true;
    obstacles[i].imgIndex = 0;
    obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_COUNT);
    obstacles[i].x = WIN_WIDTH;
    obstacles[i].y = 345 + 5 - obstacleImgs[obstacles[i].type][0].getheight();
    
    if (obstacles[i].type == TORTOISE) {
        obstacles[i].speed = 0;
        obstacles[i].power = 5;
    }
    else if (obstacles[i].type == LION) {
        obstacles[i].speed = 4;
        obstacles[i].power = 20;
    }
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
	static int enemyFre = 50;
	frameCount++;
	if (frameCount > enemyFre) {
		frameCount = 0;
        enemyFre = rand() % 50 + 50;
        createObstacle();
	}
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        if (obstacles[i].exist) {
            obstacles[i].x -= obstacles[i].speed + bgSpeed[2];
            if (obstacles[i].x < -obstacleImgs[obstacles[i].type][0].getwidth() * 2)
                obstacles[i].exist = false;
        }
        int len = obstacleImgs[obstacles[i].type].size();
        obstacles[i].imgIndex = (obstacles[i].imgIndex + 1) % len;
    }
}
//游戏背景(坐标)
void updateBg()
{
	putimagePNG2(bgX[0], 0, &imgBgs[0]);
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}

void jump()
{
	heroJump = true;
	update = true;
}

//按键输入(先判断有没有按键输入)
void keyEvent()
{
    ExMessage msg1;
    peekmessage(&msg1, EX_KEY);
    if (msg1.message == WM_KEYDOWN) {
        if (msg1.vkcode == VK_SPACE) {
            jump();
        }
    }
}

void updateEnemy()
{
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        if (obstacles[i].exist) {
            putimagePNG2(obstacles[i].x, obstacles[i].y, WIN_WIDTH, 
                &obstacleImgs[obstacles[i].type][obstacles[i].imgIndex]);
        }
    }
}

int main()
{
	init();
	int timer = 0;
	while (1)
	{
		keyEvent();
       timer += getDelay();
	   if (timer > 30)
	   {
		   timer = 0;
		   update = true;
	   }

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
}
