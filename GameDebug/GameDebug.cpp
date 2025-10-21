
#include <windows.h>
#include <cmath>
#include <vector>
#pragma comment(lib, "Msimg32.lib")

const float PI = 3.14;

struct Window {
    HWND hwnd;
    HDC DC, bufferDC;
    HBITMAP bufferHBM;
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
};

struct Sprite {
    float x, y, width, height, V, Vx, Vy, angle;
    HBITMAP hbm;
    bool back = FALSE;
    bool alpha = FALSE;
    bool status = TRUE;
};
struct Point {
    float x, y, dx, dy, distance;
    int brickID, direction;

};

Window window;
Sprite background;
Sprite player;
Sprite ball;
Sprite PointBall;
Sprite brick[41];
bool StartGame = FALSE;
int Score = 0;
bool TrajectoryFlag = FALSE;
Point NextPoint;
Point NextNextPoint;
Point NextNextNextPoint;
Point OldPoint;
float deltaTime = 0.0f;
LARGE_INTEGER frequency;
float time2Collision = 1.0f;
float surplus = 0.0f;

void CheckPlatform();
void CheckBorders();


void InitGame() {
    background.hbm = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    background.back = TRUE;
    background.x = 0;
    background.y = 0;

    player.hbm = (HBITMAP)LoadImageA(NULL, "player.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player.alpha = TRUE;
    player.x = window.width / 2;
    player.y = window.height * 0.9;
    player.width = 400;
    player.height = 50;
    player.Vx = 600.0f;

    ball.hbm = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ball.alpha = TRUE;
    ball.width = 50;
    ball.height = 50;
    ball.x = window.width / 2;
    ball.y = player.y - ball.height;
    ball.V = 6660.0f;

    HBITMAP BrickHbm = (HBITMAP)LoadImageA(NULL, "brick.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);


    for (int i = 0; i < 4; i++) {

        for (int k = 0; k < 10; k++) {

            brick[k + 10 * i].hbm = BrickHbm;
            brick[k + 10 * i].width = 150;
            brick[k + 10 * i].height = 70;
            brick[k + 10 * i].x = 100 + k * 150;
            brick[k + 10 * i].y = window.height * 0.2 + i * 70;

        }

    }

    brick[40].x = window.width / 2;
    brick[40].width = window.width;
    brick[40].y = window.height / 2;
    brick[40].height = window.height;

    NextPoint.x = window.width / 2;
    NextPoint.y = 0;
    NextPoint.brickID = 40;
}

Point PointsCompare(std::vector<Point> points) {

    if (points.size() > 0) {
        int Select = 0;
        for (int i = 0; i < points.size(); i++) {

            points[i].distance = points[i].dx / cosf(ball.angle);

            if (points[i].distance <= points[Select].distance) {
                Select = i;
            }

        }

        return points[Select];
    }
    else {

        return NextPoint;

    }
}

Point PointSlover(std::vector<Point> points, float k) {

    Point point;

    for (int i = points.size() - 1; i >= 0; i--) {

        if (points[i].dx == 0.0f) {

            points[i].dx = points[i].dy / tanf(ball.angle);
            points[i].x = ball.x + points[i].dx;
            points[i].direction = 0; //horizontal
            if (!(points[i].x >= brick[points[i].brickID].x - ((brick[points[i].brickID].width + ball.width) / 2) && points[i].x <= brick[points[i].brickID].x + ((brick[points[i].brickID].width + ball.width) / 2))) {

                points.erase(points.begin() + i);

            }
            else {

                if ((points[i].x < ball.x && ball.Vx > 0) || (points[i].x > ball.x && ball.Vx < 0)) {

                    points.erase(points.begin() + i);

                }
            }
        }
        else {

            points[i].dy = points[i].dx * tanf(ball.angle);
            points[i].y = ball.y - points[i].dy;
            points[i].direction = 1; //vertical
            if (!(points[i].y >= brick[points[i].brickID].y - ((brick[points[i].brickID].height + ball.height) / 2) && points[i].y <= brick[points[i].brickID].y + ((brick[points[i].brickID].height + ball.height) / 2))) {

                points.erase(points.begin() + i);

            }
            else {
                if ((points[i].y > ball.y && ball.Vy > 0) || (points[i].y < ball.y && ball.Vy < 0)) {

                    points.erase(points.begin() + i);

                }
            }
        }

    }

    point = PointsCompare(points);
    points.clear();

    return point;

}

Point Brick2Point(std::vector<int> bricks, float k) {
    Point point;

    std::vector<Point> points;

    for (int i = 0; i < bricks.size(); i++) {

        // Правая грань
        Point p_right;
        p_right.x = brick[bricks[i]].x + ((ball.width + brick[bricks[i]].width) / 2);
        p_right.dx = p_right.x - ball.x;
        p_right.dy = 0.0f;
        p_right.brickID = bricks[i];
        points.push_back(p_right);

        // Левая грань
        Point p_left;
        p_left.x = brick[bricks[i]].x - ((ball.width + brick[bricks[i]].width) / 2);
        p_left.dx = p_left.x - ball.x;
        p_left.dy = 0.0f;
        p_left.brickID = bricks[i];
        points.push_back(p_left);

        // Нижняя грань
        Point p_bottom;
        p_bottom.y = brick[bricks[i]].y + ((ball.height + brick[bricks[i]].height) / 2);
        p_bottom.dy = -(p_bottom.y - ball.y);
        p_bottom.dx = 0.0f;
        p_bottom.brickID = bricks[i];
        points.push_back(p_bottom);

        // Верхняя грань
        Point p_top;
        p_top.y = brick[bricks[i]].y - ((ball.height + brick[bricks[i]].height) / 2);
        p_top.dy = -(p_top.y - ball.y);
        p_top.dx = 0.0f;
        p_top.brickID = bricks[i];
        points.push_back(p_top);
    }

    if (ball.Vy > 0) {
        Point p_top_camera;
        p_top_camera.y = ball.height / 2;
        p_top_camera.dy = -(p_top_camera.y - ball.y);
        p_top_camera.dx = 0.0f;
        p_top_camera.brickID = 40;
        points.push_back(p_top_camera);
    }
    else
    {
        Point p_bottom_camera;
        p_bottom_camera.y = player.y - ball.height;
        p_bottom_camera.dy = -(p_bottom_camera.y - ball.y);
        p_bottom_camera.dx = 0.0f;
        p_bottom_camera.brickID = 40;
        points.push_back(p_bottom_camera);
    }

    if (ball.Vx > 0) {
        Point p_right_camera;
        p_right_camera.x = window.width - ball.width / 2;
        p_right_camera.dx = p_right_camera.x - ball.x;
        p_right_camera.dy = 0.0f;
        p_right_camera.brickID = 40;
        points.push_back(p_right_camera);
    }
    else
    {
        Point p_left_camera;
        p_left_camera.x = ball.width / 2;
        p_left_camera.dx = p_left_camera.x - ball.x;
        p_left_camera.dy = 0.0f;
        p_left_camera.brickID = 40;
        points.push_back(p_left_camera);
    }



    point = PointSlover(points, k);

    return point;

}

void Trajectory() {

    float k = -ball.Vy / ball.Vx;
    std::vector<int> bricks;

    for (int i = 0; i < 40; i++) {

        if (brick[i].status) {

            bool inDirection = true;

            if (ball.Vx > 0 && brick[i].x + brick[i].width / 2 < ball.x) inDirection = false;
            if (ball.Vx < 0 && brick[i].x - brick[i].width / 2 > ball.x) inDirection = false;

            if (ball.Vy > 0 && brick[i].y - brick[i].height / 2 > ball.y) inDirection = false;
            if (ball.Vy < 0 && brick[i].y + brick[i].height / 2 < ball.y) inDirection = false;


            if (inDirection) {
                bricks.push_back(i);
            }


        }

    }

    //bricks.push_back(40);

    OldPoint = NextPoint;

    NextPoint = Brick2Point(bricks, k);

    time2Collision = NextPoint.distance / ball.V;

    bricks.clear();

    TrajectoryFlag = TRUE;

}

void BallTrajectory(bool AngleUpdate, bool PointBall) {

    if (AngleUpdate) {

        ball.angle = atan2(ball.Vy, ball.Vx);

    }
    else {
        ball.Vx = ball.V * cos(ball.angle);
        ball.Vy = ball.V * sin(ball.angle);
    }

    TrajectoryFlag = FALSE;

}

void CollisionEffect() {

    if (NextPoint.direction == 1) {
        ball.Vx = -ball.Vx;
        BallTrajectory(true, false);
        Trajectory();

    }
    else {
        ball.Vy = -ball.Vy;
        BallTrajectory(true, false);
        Trajectory();
    }

}

void CheckBricks(bool repeat) {

    if (NextPoint.brickID != 40) {

       /* if ((ball.Vx >= 0 && ball.x >= NextPoint.x) || (ball.Vx <= 0 && ball.x <= NextPoint.x)) {

            if ((ball.Vy >= 0 && ball.y <= NextPoint.y) || (ball.Vy <= 0 && ball.y >= NextPoint.y)) {

                if (OldPoint.x != NextPoint.x && OldPoint.y != NextPoint.y) {

                    brick[NextPoint.brickID].status = FALSE;

                }

                ball.x = NextPoint.x;
                ball.y = NextPoint.y;

                if (NextPoint.direction == 1) {
                    ball.Vx = -ball.Vx;
                    BallTrajectory(true, false);

                }
                else {
                    ball.Vy = -ball.Vy;
                    BallTrajectory(true, false);
                }


            }

        }*/

        if (time2Collision <= 0) {


            if (OldPoint.x != NextPoint.x && OldPoint.y != NextPoint.y) {

                brick[NextPoint.brickID].status = FALSE;

            }

            if (!repeat) {
                surplus = abs(time2Collision);
            }

            CollisionEffect();

            if (NextPoint.distance > surplus) {
                StartGame = false;
                ball.x = OldPoint.x + ball.Vx * surplus;
                ball.y = OldPoint.y + ball.Vy * surplus;

            }
            else 
            {
                StartGame = false;
                 ball.x = NextPoint.x;
                 ball.y = NextPoint.y;
                 surplus = surplus - NextPoint.distance;
                 CheckBricks(TRUE);

            }

        }

    }
    else {
        CheckPlatform();
        CheckBorders();
    }

    /* for (int i = 0; i < 40; i++) {
         if (brick[i].status) {

             if ((ball.x + ball.width / 2 >= brick[i].x - brick[i].width / 2 && ball.x - ball.width / 2 <= brick[i].x + brick[i].width / 2) && (ball.y - ball.height / 2 <= brick[i].y + brick[i].height / 2 && ball.y + ball.height / 2 >= brick[i].y - brick[i].height / 2)) {


                 float dx = (abs(brick[i].x - ball.x) - ball.width / 2) / brick[i].width;
                 float dy = (abs(brick[i].y - ball.y) - ball.height / 2) / brick[i].height;

                 if (dx > dy) {
                     ball.Vx = -ball.Vx;
                     BallTrajectory(true);

                 }
                 else if (dy > dx) {
                     ball.Vy = -ball.Vy;
                     BallTrajectory(true);
                 }
                 else {

                     ball.Vx = -ball.Vx;
                     ball.Vy = -ball.Vy;
                     BallTrajectory(true);

                 }

                 brick[i].status = 0;

                 break;
             }


         }
     }*/

}

void PlrCheckBorders() {
    if (player.x - player.width / 2 <= 0) {
        player.x = player.width / 2;
    }
    if (player.x + player.width / 2 >= window.width) {

        player.x = window.width - player.width / 2;

    }
}
void CheckBorders() {

    if (ball.y - ball.height / 2 <= 0) {
        ball.Vy = -ball.Vy;
        BallTrajectory(true, false);

    }
    if (ball.x - ball.width / 2 <= 0 || ball.x + ball.width / 2 >= window.width) {

        ball.Vx = -ball.Vx;
        BallTrajectory(true, false);

    }
    if (ball.y + ball.height / 2 >= window.height) {

        StartGame = false;
        ball.y -= ball.V;
        float time2Collision = 1.0f;
        float surplus = 0.0f;

        BallTrajectory(true, false);

        InitGame();

    }

}

void CheckPlatform() {

    if ((ball.y + ball.height) >= player.y && (ball.x - ball.width / 2 <= player.x + player.width / 2 && ball.x + ball.width / 2 >= player.x - player.width / 2)) {

        float dx = (abs(player.x - ball.x) - ball.width / 2) / player.width;
        float dy = (abs(player.y - ball.y) - ball.height / 2) / player.height;


        if (abs(dy) >= abs(dx)) {

            float difX = (ball.x - (player.x - player.width / 2.0)) / (player.width);
            float res = difX * (-2.0 / 3.0);
            ball.angle = (2.0 / 3.0 - 1.0 / 3.0 * difX) * PI;
            ball.y = player.y - ball.height;
            BallTrajectory(false, false);

        }
        else {
            //ball.x = player.x - player.width*(dx/abs(dx));
            ball.Vx = -ball.Vx;
            BallTrajectory(true, false);

        }
    }

}

void Update(float deltaTime) {

    // CheckPlatform();
    // CheckBorders();
    time2Collision = time2Collision - deltaTime;

    CheckBricks(FALSE);
    if (!TrajectoryFlag) {

        Trajectory();

    }



    ball.y -= ball.Vy * deltaTime;;
    ball.x += ball.Vx * deltaTime;;

    StartGame = false;
}

void RenderTraj() {

    HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(window.bufferDC, hPen);

    MoveToEx(window.bufferDC, ball.x, ball.y, NULL);
    LineTo(window.bufferDC, NextPoint.x, NextPoint.y);

    SelectObject(window.bufferDC, hOldPen);
    DeleteObject(hPen);

}


void RenderObj(Sprite sprite) {
    HBITMAP oldBMP;
    HDC tempDC;
    tempDC = CreateCompatibleDC(window.bufferDC);
    oldBMP = (HBITMAP)SelectObject(tempDC, sprite.hbm);
    if (oldBMP) {

        if (!sprite.alpha) {
            if (!sprite.back) {
                BitBlt(window.bufferDC, sprite.x - (sprite.width / 2), sprite.y - (sprite.height / 2), sprite.width, sprite.height, tempDC, 0, 0, SRCCOPY);
            }
            else {
                StretchBlt(window.bufferDC, sprite.x, sprite.y, window.width, window.height, tempDC, 0, 0, 1920, 1080, SRCCOPY);
            }
        }
        else {

            TransparentBlt(window.bufferDC, sprite.x - (sprite.width / 2), sprite.y - (sprite.height / 2), sprite.width, sprite.height, tempDC, 0, 0, sprite.width, sprite.height, RGB(0, 0, 0));
        }

        SelectObject(tempDC, oldBMP);
    }
    DeleteDC(tempDC);
}

void Render() {


    RenderObj(background);
    RenderObj(player);
    RenderObj(ball);
    for (int i = 0; i < 40; i++) {
        if (brick[i].status) {
            RenderObj(brick[i]);
        }
    }

    RenderTraj();

    SetTextColor(window.bufferDC, RGB(255, 255, 255));
    SetBkMode(window.bufferDC, TRANSPARENT);

    wchar_t text[256];
    int angelW = (int)(ball.angle * 180 / PI);
    wsprintf(text, L"Angel: %d", angelW);
    TextOut(window.bufferDC, 10, 30, text, lstrlen(text));
    int time = (int)(time2Collision * 100);
    wsprintf(text, L"Time: %d", time);
    TextOut(window.bufferDC, 10, 50, text, lstrlen(text));




    BitBlt(window.DC, 0, 0, window.width, window.height, window.bufferDC, 0, 0, SRCCOPY);
}

void Input() {

    if (GetAsyncKeyState(VK_ESCAPE)) {
        PostMessage(window.hwnd, WM_CLOSE, 0, 0);
    }

    if (GetAsyncKeyState(VK_SPACE)) {
        StartGame = true;
    }
    if (GetAsyncKeyState('Q')) {
        StartGame = false;
    }

    if (GetAsyncKeyState('A')) {
        player.x -= player.Vx * deltaTime;
    }
    if (GetAsyncKeyState('D')) {
        player.x += player.Vx * deltaTime;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Render();
        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_KEYDOWN:
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND InitWindow(HINSTANCE hInstance) {

    WNDCLASSEX WinClass = { 0 };
    WinClass.cbSize = sizeof(WNDCLASSEX);
    WinClass.style = CS_HREDRAW | CS_VREDRAW;
    WinClass.lpfnWndProc = WndProc;
    WinClass.hInstance = hInstance;
    WinClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WinClass.hCursor = NULL;
    WinClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WinClass.lpszMenuName = NULL;
    WinClass.lpszClassName = L"MyWindow";
    WinClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&WinClass)) {
        return NULL;
    }

    HWND hwnd = CreateWindowEx(
        0,
        L"MyWindow",
        L"Arkanoid",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window.width,
        window.height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    return hwnd;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    window.hwnd = InitWindow(hInstance);
    InitGame();
    ShowWindow(window.hwnd, nCmdShow);
    UpdateWindow(window.hwnd);
    ShowCursor(FALSE);

    window.DC = GetDC(window.hwnd);
    window.bufferDC = CreateCompatibleDC(window.DC);
    window.bufferHBM = CreateCompatibleBitmap(window.DC, window.width, window.height);
    SelectObject(window.bufferDC, window.bufferHBM);

    MSG msg;

    LARGE_INTEGER lastTime;
    QueryPerformanceCounter(&lastTime); // Инициализируем время старта
    QueryPerformanceFrequency(&frequency);

    while (true) {

        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;

        
        Render();
        Input();
        if (StartGame) {
            Update(deltaTime);
        }
        PlrCheckBorders();
        Sleep(10);

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            // Если получили WM_QUIT - выходим
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }

    ReleaseDC(window.hwnd, window.DC);
    DeleteDC(window.bufferDC);
    return (int)msg.wParam;

}
