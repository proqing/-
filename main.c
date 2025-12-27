#define  _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<graphics.h>
#include<conio.h>
#include<windows.h>
#include<math.h>
#include "tools.h"
#include<vector>
#include<stdlib.h>
#include<string.h>
using namespace std;

#define WIN_WIDTH 1012 //窗口宽度
#define WIN_HEIGHT 396 //窗口高度
#define OBSTACLE_COUNT 10 //障碍物计数
#define WIN_SCORE  200  //达到该分数判定为胜利
#define SCORE_FILE "scores.txt" //保存分数的文件

IMAGE imgBgs[3]; //三重背景图片
int bgX[3]; //背景图片的x坐标
int bgSpeed[3] = { 1,2,4}; //三重背景的移动速度

IMAGE imgHeros[12]; //角色奔跑的12张图片
int heroX; //角色X坐标
int heroY; //角色Y坐标
int heroIndex; //角色图片帧序号
bool heroJump; //角色是否跳跃
int jumpHeightMax; //角色跳跃最大高度
int heroJumpOff; //跳跃后下降的高度
int update; //是否需要更新（根据帧数判断）

int heroBlood; //初始状态的血量
int score; //得分
HWND game_hwnd = NULL; //游戏窗口的句柄
HWND console_hwnd = NULL; //控制台窗口的句柄
int high_scores[10] = { 0 };  // 存储前10名最高分
int score_count = 0;        // 实际存储的分数数量


typedef enum {
    TORTOISE, //乌龟 0
    LION,//狮子 1
    HOOK1,
    HOOK2,
    HOOK3,
    HOOK4,
    OBSTACLE_TYPE_COUNT
}obstacle_type;
vector<vector<IMAGE>> obstacleImgs;//存放所有障碍物的图片
typedef struct obstacle {
    int type;//障碍物类型
    int imgIndex;
    int x, y;
    int speed;
    int power;
    bool exist;
    bool hited;
    bool passed;//表示是否通过
    IMAGE img[12];
}obstacle_t; //不同障碍物的信息

IMAGE imgSZ[10];  //10个数字图片

obstacle_t obstacles[OBSTACLE_COUNT]; //障碍物数组
int lastObsIndex; //上一个障碍物的下标
IMAGE imgHeroDown[2];  //两张角色下蹲照片
bool heroDown; //玩家是否处于下蹲状态

int selectedCharacter = 0;  //选择的角色（0=角色1, 1=角色2）

void gamePause() {
    if (game_hwnd == NULL) return;

    // 设置焦点
    SetForegroundWindow(game_hwnd);
    SetFocus(game_hwnd);

    // 显示提示文字
    settextcolor(BLACK);
    settextstyle(36, 0, _T("黑体"));
    outtextxy(getwidth() / 2 - 220, getheight() / 2 + 90, _T("按任意键/点击鼠标继续"));
    FlushBatchDraw();

    bool paused = true;
    MSG msg;

    // 先清除所有待处理的消息
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    while (paused) {
        // 处理窗口消息
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN) {
                // 检测到任意按键或鼠标点击
                paused = false;
                break;
            }
            if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
                closegraph();
                exit(0);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (paused) {
            Sleep(10);
        }
    }
}

// 保存分数到文件
void saveScores()
{
    FILE* file = fopen(SCORE_FILE, "wb");  // 以二进制写入模式打开
    if (file == NULL) {
        return;
    }

    // 先写入分数数量
    fwrite(&score_count, sizeof(int), 1, file);

    // 再写入分数数组
    if (score_count > 0) {
        fwrite(high_scores, sizeof(int), score_count, file);
    }

    fclose(file);
}

// 从文件加载分数
void loadScores()
{
    FILE* file = fopen(SCORE_FILE, "rb");  // 以二进制读取模式打开
    if (file == NULL) {
        score_count = 0;
        return;
    }

    // 读取分数数量
    fread(&score_count, sizeof(int), 1, file);

    // 确保不超过数组大小
    if (score_count > 10) {
        score_count = 10;
    }

    // 读取分数数组
    if (score_count > 0) {
        fread(high_scores, sizeof(int), score_count, file);
    }

    fclose(file);
}

// 更新排名并自动保存到文件
void updateHighScores(int new_score)
{
    // 如果排名未满，直接添加
    if (score_count < 10) {
        high_scores[score_count] = new_score;
        score_count++;
    }

    // 如果新分数比最低分高，替换最低分
    else {
       
        int min_score = high_scores[0];
        int min_index = 0;
        for (int i = 1; i < score_count; i++) {
            if (high_scores[i] < min_score) {
                min_score = high_scores[i];
                min_index = i;
            }
        } // 找到最低分

        if (new_score > min_score) {
            high_scores[min_index] = new_score;
        }// 如果新分数比最低分高，替换
        else {
            return;  
        }
    }

    // 降序排序
    for (int i = 0; i < score_count - 1; i++) {
        for (int j = i + 1; j < score_count; j++) {
            if (high_scores[i] < high_scores[j]) {
                int temp = high_scores[i];
                high_scores[i] = high_scores[j];
                high_scores[j] = temp;
            }
        }
    }

    saveScores();
}

// 显示历史记录排行榜
void showHistoryRecords()
{
    cleardevice(); 
    setbkmode(TRANSPARENT); //文字背景为透明

    // 绘制背景
    setfillcolor(0xFFD700);
    solidrectangle(0, 0, getwidth(), getheight());
    for (int y = 0; y < getheight(); y += 5) {
        COLORREF gradColor = RGB(240 + (y * 15) / getheight(),
            248 + (y * 7) / getheight(), 255);
        setfillcolor(gradColor);
        solidrectangle(0, y, getwidth(), y + 5);
    }
    int win_width = getwidth();   // 1012
    int win_height = getheight(); // 396
    int center_x = win_width / 2;

    // 显示标题
    settextcolor(RED);
    settextstyle(30, 0, _T("黑体"));
    int title_width = textwidth(_T("排行榜"));
    outtextxy(center_x - title_width / 2, 15, _T("排行榜"));

    // 显示当前得分
    settextcolor(LIGHTMAGENTA);
    settextstyle(28, 0, _T("楷体"));
    TCHAR current_score[50];
    _stprintf(current_score, _T("本次得分: %d"), score);
    int current_score_width = textwidth(current_score);
    outtextxy(center_x - current_score_width / 2, 45, current_score);

    // 显示分隔线
    settextcolor(BLACK);
    settextstyle(20, 0, _T("宋体"));
    int line_width = textwidth(_T("══════════════════════════"));
    outtextxy(center_x - line_width / 2, 65, _T("══════════════════════════"));

    // 显示历史排名
    settextstyle(24, 0, _T("宋体"));

    if (score_count == 0) {
        settextcolor(LIGHTGRAY);
        settextstyle(28, 0, _T("楷体"));
        int no_record_width = textwidth(_T("暂无历史记录"));
        outtextxy(center_x - no_record_width / 2, 170, _T("暂无历史记录"));

        int prompt_width = textwidth(_T("赶快开始游戏创造纪录吧！"));
        outtextxy(center_x - prompt_width / 2, 220, _T("赶快开始游戏创造纪录吧！"));
    }
    else {
        // 显示前8名
        int start_y = 80;
        int max_display = (score_count < 8) ? score_count : 8; 

        for (int i = 0; i < max_display; i++) {
            // 为前3名设置不同颜色
            if (i == 0) {
                settextcolor(0xFFD700);  
            }
            else if (i == 1) {
                settextcolor(0xC0C0C0);  
            }
            else if (i == 2) {
                settextcolor(0xCD7F32);  
            }
            else {
                settextcolor(BLACK);  
            }

            // 显示名次和分数
            TCHAR score_str[50];
            _stprintf(score_str, _T("%2d. %6d 分"), i + 1, high_scores[i]);
            int score_str_width = textwidth(score_str);
            outtextxy(center_x - score_str_width / 2, start_y + i * 25, score_str);
        }
    }

    // 显示当前游戏排名
    int info_y_position = win_height - 110; 
    if (score > 0) {
        // 计算当前分数的排名
        int current_rank = -1;
        for (int i = 0; i < score_count; i++) {
            if (high_scores[i] == score) {
                current_rank = i + 1;
                break;
            }
        }

        if (current_rank > 0) {
            settextcolor(BROWN);
            settextstyle(20, 0, _T("黑体"));

            TCHAR rank_str[50];
            _stprintf(rank_str, _T("您的排名: 第 %d 名"), current_rank);
            int rank_str_width = textwidth(rank_str);
            outtextxy(center_x - rank_str_width / 2, info_y_position, rank_str);

            // 如果是第一名，特别祝贺
            if (current_rank == 1) {
                settextcolor(LIGHTRED);
                settextstyle(20, 0, _T("黑体"));
                int congrats_width = textwidth(_T("恭喜打破纪录！"));
                outtextxy(center_x - congrats_width / 2, info_y_position + 30, _T("恭喜打破纪录！"));

                // 调整返回提示的位置
                info_y_position += 80;
            }
            else {
                info_y_position += 40;
            }
        }
        else {
            // 如果分数不在排行榜中，鼓励
            settextcolor(BROWN);
            settextstyle(20, 0, _T("楷体"));
            int encourage_width = textwidth(_T("继续努力，争取上榜！"));
            outtextxy(center_x - encourage_width / 2, info_y_position, _T("继续努力，争取上榜！"));
            info_y_position += 40;
        }
    }

    // 显示返回提示
    settextcolor(BROWN);
    settextstyle(20, 0, _T("楷体"));
    int return_prompt_width = textwidth(_T("按任意键返回"));
    outtextxy(center_x - return_prompt_width / 2, win_height - 40, _T("按任意键返回"));

    FlushBatchDraw();
    bool waiting = true;
    Sleep(200);

    // 清空所有按键状态
    for (int i = 0; i < 256; i++) {
        while (GetAsyncKeyState(i) & 0x8000) {
            Sleep(1);
        }
    }

    while (waiting) {
        // 检查按键
        for (int i = 0; i < 256; i++) {
            // 无效按键
            if (i == VK_SHIFT || i == VK_CONTROL || i == VK_MENU ||
                i == VK_LSHIFT || i == VK_RSHIFT ||
                i == VK_LCONTROL || i == VK_RCONTROL ||
                i == VK_LMENU || i == VK_RMENU ||
                i == VK_CAPITAL || i == VK_NUMLOCK || i == VK_SCROLL ||
                i == VK_LWIN || i == VK_RWIN ||
                i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON||i==VK_BACK) {
                continue;
            }

            if (GetAsyncKeyState(i) & 0x8000) {
                waiting = false;
                break;
            }
        }

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
                closegraph();
                exit(0);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (waiting) {
            Sleep(10);
        }
    }

    for (int i = 0; i < 256; i++) {
        while (GetAsyncKeyState(i) & 0x8000) {
            Sleep(1);
        }
    }
}

//角色选择函数（角色1图片，角色二图片，角色1名字，角色2名字）
int selectCharacterInWindow(IMAGE* img1, IMAGE* img2,const char* char1_name, const char* char2_name)
{
    int windowWidth = 1012;
    int windowHeight = 396;//游戏窗口
    int img_width = 130;
    int img_height = 130;//角色图片窗口大小
    int padding = 50;
    int total_width = 2 * img_width + padding;
    int start_x = (windowWidth - total_width) / 2;

    int char1_x = start_x;
    int char2_x = start_x + img_width + padding;
    int char_y = 120; 

    int currentSelection = 0;  // 0未选择, 1角色1, 2角色2

    // 动态效果参数
    static int glowStep = 0;    // 发光动画步长
    COLORREF bgColor = 0xFFD700;
    cleardevice();
    setbkcolor(bgColor);
    settextcolor(BLACK);

    while (1) {
        BeginBatchDraw();
        
        //绘制渐变背景
        setfillcolor(bgColor);
        solidrectangle(0, 0, windowWidth, windowHeight);
        for (int y = 0; y < windowHeight; y += 5) {
            COLORREF gradColor = RGB(
                240 + (y * 15) / windowHeight,
                248 + (y * 7) / windowHeight,
                255
            );
            setfillcolor(gradColor);
            solidrectangle(0, y, windowWidth, y + 5);
        }

        // 绘制美化后的标题 
        settextstyle(32, 0, "站酷高端黑");
        settextcolor(0x5C3317); 
        int titleWidth = textwidth("请选择你的解药");
        int titleX = (windowWidth - titleWidth) / 2;
        settextcolor(0x191970); // 深蓝描边
        outtextxy(titleX - 1, 39, "请选择你的解药");
        outtextxy(titleX + 1, 39, "请选择你的解药");
        outtextxy(titleX - 1, 41, "请选择你的解药");
        outtextxy(titleX + 1, 41, "请选择你的解药");
        // 主标题
        settextcolor(0x4169E1);
        outtextxy(titleX, 40, "请选择你的解药");

        // 动态发光画框，循环实现
        glowStep = (glowStep + 1) % 100; 
        int glowWidth = 2 + (sin(glowStep * 0.06) * 1.5); // 发光宽度动态变化
        COLORREF selectGlowColor = (glowStep % 2 == 0) ? 0x32CD32 : 0x228B22;

        // 绘制角色1
        if (currentSelection == 1) {
            setlinecolor(selectGlowColor);
            setlinestyle(PS_SOLID, (int)glowWidth); 
            setfillcolor(0xE8FFE8);//浅绿色
        }
        else {
            setlinecolor(0xD3D3D3); 
            setlinestyle(PS_SOLID, 2);
            setfillcolor(0xFAFAFA);
        }
        fillroundrect(char1_x - 8, char_y - 8,
            char1_x + img_width + 8, char_y + img_height + 8, 12, 12);
        rectangle(char1_x - 8, char_y - 8,
            char1_x + img_width + 8, char_y + img_height + 8);

        setbkmode(TRANSPARENT); // 文字透明
        putimage(char1_x + 2, char_y + 2, img1); // 阴影
        putimage(char1_x, char_y, img1);

        settextcolor(0x2F4F4F); // 深青灰
        settextstyle(24, 0, "微软雅黑 Bold"); 
        int name1Width = textwidth(char1_name);
        outtextxy(char1_x + (img_width - name1Width) / 2,
            char_y + img_height + 12, char1_name);

        //角色2
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

        putimage(char2_x + 2, char_y + 2, img2); // 阴影
        putimage(char2_x, char_y, img2);

        settextcolor(0x2F4F4F);
        outtextxy(char2_x + (img_width - textwidth(char2_name)) / 2,
            char_y + img_height + 12, char2_name);

        //提示文字
        settextstyle(18, 0, "微软雅黑");
        char tipText[32];
        COLORREF tipColor;
        if (currentSelection == 0) {
            strcpy(tipText, "点击选择角色");
            tipColor = 0xFF4500;
        }
        else {
            strcpy(tipText, "请按空格开始");
            tipColor = 0x228B22;
        }

        int tipWidth = textwidth(tipText);
        int tipX = (windowWidth - tipWidth) / 2;
        setfillcolor(0xFFFFFF); // 白色
        setlinecolor(0xE0E0E0); // 浅灰边框
        fillroundrect(tipX - 10, 320, tipX + tipWidth + 10, 345, 8, 8);
        
        // 绘制提示文字
        settextcolor(tipColor);
        outtextxy(tipX, 325, tipText);

        // ESC取消提示
        const char* escText = "按ESC退出";
        int escWidth = textwidth(escText);
        int escX = (windowWidth - escWidth) / 2;
        setfillcolor(0xFFFFFF);
        setlinecolor(0xE0E0E0);
        fillroundrect(escX - 10, 350, escX + escWidth + 10, 375, 8, 8);
        settextcolor(0x4682B4); // 钢蓝色
        outtextxy(escX, 355, escText);

        // 查看历史记录
        const char* historyText = "按 H 键查看历史记录";
        int historyWidth = textwidth(historyText);
        int historyX = (windowWidth - historyWidth) / 2;
        setfillcolor(0xFFFFFF);
        setlinecolor(0xE0E0E0);
        fillroundrect(historyX - 10, 280, historyX + historyWidth + 10, 305, 8, 8);
        settextcolor(0x8B4513); // 棕色
        outtextxy(historyX, 285, historyText);


        // 输入处理
        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= char1_x && msg.x <= char1_x + img_width &&
                    msg.y >= char_y && msg.y <= char_y + img_height) {
                    currentSelection = 1;
                }
                else if (msg.x >= char2_x && msg.x <= char2_x + img_width &&
                    msg.y >= char_y && msg.y <= char_y + img_height) {
                    currentSelection = 2;
                }
            }
            else if (msg.message == WM_KEYDOWN) {
                // 空格键
                if (msg.vkcode == VK_SPACE && currentSelection > 0) {
                    EndBatchDraw();
                    return currentSelection - 1; 
                }

                // ESC键
                else if (msg.vkcode == VK_ESCAPE) {
                    EndBatchDraw();
                    return -1; 
                }

                // 数字键1或左键选择角色1
                else if (msg.vkcode == '1' || msg.vkcode == VK_LEFT) {
                    currentSelection = 1;
                }

                // 数字键2或右键选择角色2
                else if (msg.vkcode == '2' || msg.vkcode == VK_RIGHT) {
                    currentSelection = 2;
                }

                // H键查看历史记录
                else if (msg.vkcode == 'H' || msg.vkcode == 'h') {
                    EndBatchDraw(); 
                    showHistoryRecords(); 
                    cleardevice();
                   continue; 

                }
            }
        }

        EndBatchDraw();
        Sleep(15);
        glowStep++;
    }
}

//显示游戏说明
int showGameTips(IMAGE* imgTip) {
    int windowWidth = 1012;
    int windowHeight = 396;
    int tipX = (windowWidth - imgTip->getwidth()) / 2;
    int tipY = 15;

    // 按钮区域
    int btnStartX = windowWidth - 280;
    int btnStartY = windowHeight - 230;
    int btnStartW = 150;
    int btnStartH = 50;

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

        //绘制背景
        setfillcolor(0xFFD700);
        solidrectangle(0, 0, windowWidth, windowHeight);
        for (int y = 0; y < windowHeight; y += 5) {
            COLORREF gradColor = RGB(240 + (y * 15) / windowHeight,
                248 + (y * 7) / windowHeight, 255);
            setfillcolor(gradColor);
            solidrectangle(0, y, windowWidth, y + 5);
        }

        // 游戏说明
        putimage(tipX, tipY, imgTip);

        // 开始游戏
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

        // 返回按钮
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

        // 处理鼠标/键盘输入
        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
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
                    return 0; 
                }
                // 点击返回
                if (hoverBack) {
                    EndBatchDraw();
                    return 1; 
                }
            }
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

//初始化游戏窗口
void initGameWindow(int width, int height)
{
    static bool initialized = false;

    if (initialized) {
        // 设置焦点和捕获
        SetForegroundWindow(game_hwnd);
        SetFocus(game_hwnd);
        SetCapture(game_hwnd);
        SetActiveWindow(game_hwnd);
        return;
    }

    console_hwnd = GetConsoleWindow();
    game_hwnd = initgraph(width, height);
    if (game_hwnd == NULL) {
        MessageBox(NULL, _T("图形窗口初始化失败！"), _T("错误"), MB_OK);
        exit(0);
    }

    EnableWindow(console_hwnd, FALSE);
    ShowWindow(console_hwnd, SW_HIDE);

    SetForegroundWindow(game_hwnd);
    SetFocus(game_hwnd);
    SetCapture(game_hwnd);
    SetActiveWindow(game_hwnd);

    initialized = true;
}

//初始化函数
void init() 
{
    loadScores();
    initGameWindow(WIN_WIDTH, WIN_HEIGHT);
    cleardevice();

    // 显示初始界面
    loadimage(0, "res/over.png",1012,396,true);
    setbkmode(TRANSPARENT);

    // 提示文字
    settextcolor(BROWN);
    settextstyle(20, 0, _T("楷体"));
    outtextxy(getwidth() / 2 - 150, getheight() - 55, _T("按 H 键查看历史记录"));
    outtextxy(getwidth() / 2 - 155, getheight() - 30, _T("按其他任意键开始游戏"));

    FlushBatchDraw();

    // 等待按键
    bool waiting = true;
    bool key_pressed = false;

    while (waiting) {
        for (int i = 0; i < 256; i++) {

            // 跳过系统修饰键以及鼠标按键
            if (i == VK_SHIFT || i == VK_CONTROL || i == VK_MENU ||
                i == VK_LSHIFT || i == VK_RSHIFT ||
                i == VK_LCONTROL || i == VK_RCONTROL ||
                i == VK_LMENU || i == VK_RMENU ||
                i == VK_CAPITAL || i == VK_NUMLOCK || i == VK_SCROLL ||
                i == VK_LWIN || i == VK_RWIN ||
                i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON || i == VK_BACK) {
                continue;
            }

            if (GetAsyncKeyState(i) & 0x8000) {
                key_pressed = true;
                if (i == VK_SHIFT || i == VK_CONTROL || i == VK_MENU ||
                    i == VK_LSHIFT || i == VK_RSHIFT ||
                    i == VK_LCONTROL || i == VK_RCONTROL ||
                    i == VK_LMENU || i == VK_RMENU ||
                    i == VK_CAPITAL || i == VK_NUMLOCK || i == VK_SCROLL ||
                    i == VK_LWIN || i == VK_RWIN ||
                    i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON || i == VK_BACK) {
                    continue;
                }

                // 如果是H键，显示历史记录
                if (i == 'H' || i == 'h') {
                    while (GetAsyncKeyState('H') & 0x8000 || GetAsyncKeyState('h') & 0x8000) {
                        Sleep(10);
                    }
                    showHistoryRecords();

                    // 重新显示初始界面
                    cleardevice();
                    loadimage(0, "res/over.png", 1012, 396, true);

                    setbkmode(TRANSPARENT);

                    settextcolor(BROWN);
                    settextstyle(20, 0, _T("楷体"));
                    outtextxy(getwidth() / 2 - 150, getheight() - 55, _T("按 H 键查看历史记录"));
                    outtextxy(getwidth() / 2 - 155, getheight() - 30, _T("按其他任意键开始游戏"));
                    FlushBatchDraw();

                    key_pressed = false;
                    break;
                }
                // 如果是其他键，开始游戏
                else {
                    waiting = false;
                    break;
                }
            }
        }
        
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
                closegraph();
                exit(0);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (waiting) {
            Sleep(10); 
        }
    }

    cleardevice();
    FlushBatchDraw();

    score = 0;
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

    // 角色选择和游戏说明
    while (true) {

        // 调用角色选择函数
        selectedCharacter = selectCharacterInWindow(&character1, &character2, "role1", "role2");

        // 处理取消选择（直接退出）
        if (selectedCharacter < 0)
        {
            closegraph();
            exit(0);
        }

        //  加载游戏说明图片
        IMAGE imgTip;
        loadimage(&imgTip, "res/说明.png");

        // 显示游戏说明界面，获取用户选择
        int tipResult = showGameTips(&imgTip);
        if (tipResult == 0) {
            // 开始游戏
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
            loadimage(&imgHeros[i], name,150,143,true);
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

    //初始化障碍物池子
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        obstacles[i].exist = 0;
    }
   
    //加载下蹲素材
    if (selectedCharacter == 1)
    {
        loadimage(&imgHeroDown[0], "res/d1.png");
        loadimage(&imgHeroDown[1], "res/d2.png");
        heroDown = false;
    }
    else
    {
        loadimage(&imgHeroDown[0], "res/bark/d1.png",150,81,true);
        loadimage(&imgHeroDown[1], "res/bark/d2.png", 150, 81, true);
        heroDown = false;
    }

    IMAGE imgH;
    for (int i = 0; i < 4; ++i)
    {
        vector<IMAGE> imgHookArray;
        sprintf(name, "res/h%d.png", i + 1);
        loadimage(&imgH, name, 63, 260, true);
        imgHookArray.push_back(imgH);
        obstacleImgs.push_back(imgHookArray);
    }
    
    heroBlood = 100;

    //预加载音效
    preLoadSound("res/hit.mp3");
    mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
    lastObsIndex = -1;
    
    //加载数字图片
    for (int i = 0; i < 20; ++i)
    {
        sprintf(name, "res/sz/%d.png", i);
        loadimage(&imgSZ[i], name);
    }

 }

 //障碍物属性的初始化
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
    obstacles[i].hited = false;
    obstacles[i].imgIndex = 0;
    obstacles[i].type = (obstacle_type)(rand() % 3);//优化频率

    if (lastObsIndex>=0&&
         obstacles[lastObsIndex].type >= HOOK1 &&
         obstacles[lastObsIndex].type <= HOOK4&&
         obstacles[i].type==LION&&
         obstacles[lastObsIndex].x>(WIN_WIDTH-500))
    {
        obstacles[i].type = TORTOISE;
    }
    lastObsIndex = i;//柱子和狮子同时出现，把狮子替换成乌龟

    if (obstacles[i].type == HOOK1)
    {
        obstacles[i].type+=rand() % 4;
    }//优化柱子出现的频率
    obstacles[i].x = WIN_WIDTH;
    obstacles[i].y = 345 + 5 - obstacleImgs[obstacles[i].type][0].getheight();
    
    if (obstacles[i].type == TORTOISE) {
        obstacles[i].speed = 0;
        obstacles[i].power = 2;
    }
    else if (obstacles[i].type == LION) {
        obstacles[i].speed = 4;
        obstacles[i].power = 10;
    }
    else if (obstacles[i].type >= HOOK1&& obstacles[i].type <= HOOK4) {//枚举变量
        obstacles[i].speed = 0;
        obstacles[i].power = 5;
        obstacles[i].y = 0;
    }
    obstacles[i].passed = false;
}

//碰撞检测
void checkHit()
{
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        if (obstacles[i].exist&&obstacles[i].hited==false)
        {
            int a1x, a1y, a2x, a2y;
            int off = 30;

            //获取玩家的坐标
            if (!heroDown)
            {
                a1x = heroX + off;
                a1y = heroY + off;
                a2x = heroX + imgHeros[heroIndex].getwidth() - off;
                a2y = heroY + imgHeros[heroIndex].getheight();
            }
            else
            {
                a1x = heroX + off;
                a1y = 345 - imgHeroDown[heroIndex].getheight();
                a2x = heroX + imgHeroDown[heroIndex].getwidth()-off;
                a2y =345;
            }
            
            //获取障碍物的坐标
            IMAGE img = obstacleImgs[obstacles[i].type][obstacles[i].imgIndex];
            int b1x = obstacles[i].x + off;
            int b1y = obstacles[i].y + off;
            int b2x = obstacles[i].x + img.getwidth() - off;
            int b2y = obstacles[i].y+ img.getheight() - 10;

            if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y))
            {
                heroBlood -= obstacles[i].power;//省略了打印血量的步骤
                playSound("res/hit.mp3");
                obstacles[i].hited = true;
            }
        }
    }
}

//更新背景、玩家、障碍物的状态
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
    //下蹲
	else if(heroDown){
        static int count = 0;
        int delays[2] = { 4,30 };
        count++;
        if (count >= delays[heroIndex])
        {
            count = 0;
            heroIndex++;
            if (heroIndex >= 2)
            {
                heroIndex = 0;
                heroDown = false;
            }

        }
        
	}
    //奔跑
    else
    {
        heroIndex = (heroIndex + 1) % 12;//循环呈现这几张图片
    }

    //障碍物出现的频率
	static int frameCount = 0; 
	static int enemyFre = 80;
	frameCount++;
	if (frameCount > enemyFre) {
		frameCount = 0;
        enemyFre = rand() % 80 + 80;
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
    }//障碍物移除游戏窗口

    checkHit();
}

//游戏背景
void updateBg()
{
	putimagePNG2(bgX[0], 0, &imgBgs[0]);
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}

//玩家跳跃
void jump()
{
	heroJump = true;
	update = true;
}

//玩家下蹲
void down()
{
    update = true;
    heroDown = true;
    heroIndex = 0;
}

//按键输入
void keyEvent()
{
    ExMessage msg1;
    peekmessage(&msg1, EX_KEY);
    if (msg1.message == WM_KEYDOWN) {
        if (msg1.vkcode == VK_SPACE) {
            jump();
        }
        else if (msg1.vkcode == 's' || msg1.vkcode == 'S')
        {
            down();
        }
    }
}

//显示障碍物
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

//显示玩家
void  updateHero()
{
    if (!heroDown)
    {
        putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
    }
    else
    {
        int y = 345 - imgHeroDown[heroIndex].getheight();
        putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
    }
}

//显示血条
void updateBloodBar()
{
    drawBloodBar(10,10,200,20,2,BLUE,DARKGRAY,RED,heroBlood/100.0);
}

//判断游戏是否结束
void checkOver()
{
    if (heroBlood <= 0)
    {
        FlushBatchDraw();
        mciSendString("stop res/bg.mp3", 0, 0, 0);
        loadimage(0, "res/over.png");

        // 更新分数排名
        updateHighScores(score);
        FlushBatchDraw();

        // 显示历史记录
        showHistoryRecords();

        init();
    }
}

//加分
void checkScore()
{
    for (int i = 0; i < OBSTACLE_COUNT; ++i)
    {
        if (obstacles[i].exist &&
            obstacles[i].passed == false &&
            obstacles[i].hited==false&&
            obstacles[i].x + obstacleImgs[obstacles[i].type][0].getwidth() < heroX)
        {
            if (obstacles[i].type == LION) {
                score += 5;
            }
            else if (obstacles[i].type == TORTOISE) {
                score += 1;
            }
            else {
                score += 2;
            }
            obstacles[i].passed = TRUE;
        }
    }
}

//分数显示
void updateScore()
{
    char str[8];
    sprintf(str, "%d", score);
    int x = 20;
    int y = 35;
    for (int i = 0; str[i]; ++i)
    {
        int sz = str[i] - '0';
        putimagePNG(x, y, &imgSZ[sz]);
        x += imgSZ[sz].getwidth() + 5;
    }
}

//判断游戏是否胜利
void checkWin()
{
    if (score >= WIN_SCORE)
    {
        FlushBatchDraw();
        mciSendString("play res/win.mp3", 0, 0, 0);
        Sleep(2000);
        loadimage(0, "res/win.png");

        // 更新分数排名
        updateHighScores(score);
       
        FlushBatchDraw();
        mciSendString("stop res/bg.mp3", 0, 0, 0);

        // 显示历史记录
        showHistoryRecords();

        init();
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
	   }//大于30帧，更新画面

	   if (update)
	   {
		   update = false;

		   BeginBatchDraw();

		   updateBg();
           updateHero();
		   updateEnemy();
           updateBloodBar();
           updateScore();
           checkWin();

		   EndBatchDraw();
           
           checkOver();
           checkScore();
           
     	    fly();
	   }
	}
	system("pause");
	return 0;
}
