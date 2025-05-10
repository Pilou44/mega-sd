#include "level.h"
#include "level_res.h"

#define BAR_INCREMENT 3
#define BALL_SPEED 5

static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);

static void moveP1(s16 increment);
static void moveP2(s16 increment);
static void moveWithUi();
static void moveBar(s16 increment, Sprite *bar, u16 height);

static void manageBall();
static void checkCollision(Sprite *bar, u16 width, u16 height, s16 *ballX, s16 *ballY);

bool paused;

u8 humanPlayers;

Sprite *player1;
Sprite *player2;
Sprite *ball;

u16 p1Height;
u16 p1Width;

u16 p2Height;
u16 p2Width;

u16 ballSize;
s8 ballSpeedX;
s8 ballSpeedY;

s16 minHeight;
s16 maxHeight;
s16 minWidth;
s16 maxWidth;

void showLevel(u8 players)
{
    VDP_setScreenWidth320();
    SPR_init();
    JOY_init();

    humanPlayers = players;
    paused = FALSE;
    u16 palette[64];

    minHeight = 0;
    maxHeight = screenHeight;
    minWidth = 0;
    maxWidth = screenWidth;

    p1Height = sprite_barre.h;
    p1Width = sprite_barre.w;
    p2Height = sprite_barre.h;
    p2Width = sprite_barre.w;

    s16 startX1 = 20;
    s16 startX2 = screenWidth - startX1 - p2Width;
    s16 startY1 = (screenHeight - p1Height) / 2;
    s16 startY2 = (screenHeight - p2Height) / 2;
    player1 = SPR_addSprite(&sprite_barre, startX1, startY1, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
    player2 = SPR_addSprite(&sprite_barre, startX2, startY2, TILE_ATTR(PAL1, TRUE, FALSE, TRUE));

    ballSize = sprite_ball.w;
    s16 ballX = (screenWidth - ballSize) / 2;
    s16 ballY = (screenHeight - ballSize) / 2;
    ball = SPR_addSprite(&sprite_ball, ballX, ballY, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));

    SPR_update();

    // prepare palettes (BGB image contains the 4 palettes data)
    memcpy(&palette[0], palette_all.data, 64 * 2);
    //            memcpy(&palette[16], palette_sprites.data, 16 * 2);

    JOY_setEventHandler(joyEvent);

    // fade in
    PAL_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    // Angle between 30 and 60 degrees, ie between 85 and 170
    s8 ballAngle = (random() % 86) + 85;
    ballSpeedX = F16_toInt(F16_mul(FIX16(BALL_SPEED), cosFix16(ballAngle)));
    ballSpeedY = F16_toInt(F16_mul(FIX16(BALL_SPEED), sinFix16(ballAngle)));
    KLog_f1("cos angle = ", cosFix16(ballAngle));
    KLog_f1("sin angle = ", sinFix16(ballAngle));
    KLog_f1("ball inc = ", FIX16(BALL_SPEED));
    KLog_f1("Ball speed x fix16 = ", F16_mul(F16_toInt(BALL_SPEED), cosFix16(ballAngle)));
    KLog_S1("Ball speed y fix16 = ", F16_mul(F16_toInt(BALL_SPEED), sinFix16(ballAngle)));

    KLog_S1("Ball speed x = ", ballSpeedX);
    KLog_S1("Ball speed y = ", ballSpeedY);

    // s8 xSpeedSign = (random() % 3) - 1;
    // while (xSpeedSign == 0)
    //     xSpeedSign = (random() % 3) - 1;

    // ballSpeedX = xSpeedSign * (random() % 4);
    // while (ballSpeedX == 0)
    //     ballSpeedX = xSpeedSign * (random() % 4);

    // s8 ySpeedSign = (random() % 3) - 1;
    // while (ySpeedSign == 0)
    //     ySpeedSign = (random() % 3) - 1;

    // ballSpeedY = ySpeedSign * (random() % 4);
    // while (ballSpeedY == 0)
    //     ballSpeedY = ySpeedSign * (random() % 4);

    while (1)
    {
        if (!paused)
        {
            manageBall();
            handleInput();
        }
        SPR_update();
        SYS_doVBlankProcess();
    }
}

static void manageBall()
{
    s16 ballX = SPR_getPositionX(ball);
    s16 ballY = SPR_getPositionY(ball);
    ballX += ballSpeedX;
    ballY += ballSpeedY;
    if (ballY <= minHeight)
    {
        ballY = minHeight;
        ballSpeedY = -ballSpeedY;
    }
    else if (ballY + ballSize > maxHeight)
    {
        ballY = maxHeight - ballSize;
        ballSpeedY = -ballSpeedY;
    }

    // ToDo Temp for test
    if (ballX <= minWidth)
    {
        ballX = minWidth;
        ballSpeedX = -ballSpeedX;
    }
    else if (ballX + ballSize > maxWidth)
    {
        ballX = maxWidth - ballSize;
        ballSpeedX = -ballSpeedX;
    }

    checkCollision(player1, p1Width, p1Height, &ballX, &ballY);
    checkCollision(player2, p2Width, p2Height, &ballX, &ballY);

    SPR_setPosition(ball, ballX, ballY);
}

static void checkCollision(Sprite *bar, u16 width, u16 height, s16 *ballX, s16 *ballY)
{
    s16 barLeft = SPR_getPositionX(bar);
    s16 barRight = barLeft + width;
    s16 barTop = SPR_getPositionY(bar);
    s16 barBottom = barTop + height;

    s16 ballLeft = SPR_getPositionX(ball);
    s16 ballRight = ballLeft + ballSize;
    s16 ballTop = SPR_getPositionY(ball);
    s16 ballBottom = ballTop + ballSize;

    bool isGoodY = ballBottom >= barTop && ballTop <= barBottom;
    if (isGoodY)
    {
        bool contactOnRight = ballLeft <= barRight && ballLeft - ballSpeedX > barRight && ballSpeedX < 0;
        if (contactOnRight)
        {
            *ballX = barRight;
            ballSpeedX = -ballSpeedX;
        }
        else
        {
            bool contactOnLeft = ballRight >= barLeft && ballRight - ballSpeedX < barLeft && ballSpeedX > 0;
            if (contactOnLeft)
            {
                *ballX = barLeft - ballSize;
                ballSpeedX = -ballSpeedX;
            }
        }
    }

    bool isGoodX = ballRight >= barLeft && ballLeft <= barRight;
    if (isGoodX)
    {
        bool contactOnBottom = ballTop <= barBottom && ballTop - ballSpeedY > barBottom && ballSpeedY < 0;
        if (contactOnBottom)
        {
            *ballY = barBottom;
            ballSpeedY = -ballSpeedY;
        }
        else
        {
            bool contactOnTop = ballBottom >= barTop && ballBottom - ballSpeedY < barTop && ballSpeedY > 0;
            if (contactOnTop)
            {
                *ballY = barTop - ballSize;
                ballSpeedY = -ballSpeedY;
            }
        }
    }
}

static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);
    if (value & BUTTON_UP)
        moveP1(-1);
    else if (value & BUTTON_DOWN)
        moveP1(+1);

    if (humanPlayers == 2)
    {
        u16 value = JOY_readJoypad(JOY_2);
        if (value & BUTTON_UP)
            moveP2(-1);
        else if (value & BUTTON_DOWN)
            moveP2(+1);
    }
    else
    {
        moveWithUi();
    }
}

static void moveWithUi()
{
    s16 p2Y = SPR_getPositionY(player2) + p2Height / 2;
    if (ballSpeedX < 0) {
        u16 centerY = screenHeight / 2;
        if (p2Y < centerY - BAR_INCREMENT)
        {
            moveP2(+1);
        }
        else if (p2Y > centerY + BAR_INCREMENT)
        {
            moveP2(-1);
        }
    }
    else
    {
        s16 ballY = SPR_getPositionY(ball) + ballSize / 2;
        if (p2Y < ballY - BAR_INCREMENT)
        {
            moveP2(+1);
        }
        else if (p2Y > ballY + BAR_INCREMENT)
        {
            moveP2(-1);
        }
    }
}

static void moveP1(s16 increment)
{
    moveBar(increment, player1, p1Height);
}

static void moveP2(s16 increment)
{
    moveBar(increment, player2, p2Height);
}

static void moveBar(s16 increment, Sprite *bar, u16 height)
{
    s16 x = SPR_getPositionX(bar);
    s16 y = SPR_getPositionY(bar) + BAR_INCREMENT * increment;
    if (y < minHeight)
        y = minHeight;
    if (y + height > maxHeight)
        y = maxHeight - height;
    SPR_setPosition(bar, x, y);
}

static void joyEvent(u16 joy, u16 changed, u16 state)
{
    // START button state changed --> pause / unpause
    if (changed & state & BUTTON_START)
    {
        paused = !paused;
        char *text = "PAUSE";
        u16 textLength = strlen(text);
        u16 y = screenHeight / 8 / 2 - 1;
        u16 x = ((screenWidth - textLength) / 8 - textLength) / 2;
        if (paused)
        {
            VDP_drawText(text, x, y);
        }
        else
        {
            VDP_clearText(x, y, textLength);
        }
    }
}
