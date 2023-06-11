#include <bits/stdc++.h>
#include <windows.h>
#include <ctime>
#define RAYGUI_IMPLEMENTATION
#define GUI_STARTSCENE_IMPLEMENTATION
#define GUI_GAMEOVERWINDOW_IMPLEMENTATION
#define ScreenWidth 1600
#define ScreenHeight 900
namespace raylib
{
#include "raylib.h"
#include "raygui.h"
#include "startscene.h"
#include "GameOverWindow.h"
#include "style.h"
#include "rdrawing.h"
}
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") // hide the console(only vs works)
#pragma comment(lib, "ws2_32.lib")
#define MapSize 3025
#define AreaSize 55
#define PlayerMax 10
#define AreaFoodNum 2
#define bgcolor raylib::WHITE
#define FoodRadio 10
#define MaxRankListNum 10
#define RankBoardColor \
  raylib::Color { 0, 0, 0, 128 }
using namespace std;
// net part
SOCKET SocketClient;
char ip[256];
int port;
char name[128];
pair<double, double> PlayerMove;
bool isfast;
raylib::Camera2D camera = {{ScreenWidth / 2, ScreenHeight / 2}, {0, 0}, 0, 1.0f};
int countnum;
pair<char[128], double> ranklist[PlayerMax];
pair<double, string> rankliststring[PlayerMax];
auto scenedata = raylib::InitGuiStartscene();
inline bool cmp(pair<double, string> x, pair<double, string> y)
{
  return x.first == y.first ? x.second < y.second : x.first > y.first;
}
void HideWindow() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd) {
		ShowWindow(hwnd, SW_HIDE);
	}
}
int initnet()
{
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  SocketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  SOCKADDR_IN ClientAddr;

  ClientAddr.sin_family = AF_INET;
  ClientAddr.sin_addr.S_un.S_addr = inet_addr(ip);
  ClientAddr.sin_port = htons(port);

  int status = 0;
  status = connect(SocketClient, (struct sockaddr *)&ClientAddr, sizeof(ClientAddr));
  if (status == SOCKET_ERROR)
  {
    return -1; // connect failed
  }
  cout << "INFO: OK connected" << endl;
  return 0;
}
// classes
class area
{
public:
  int x, y; // top-left point
  int foodnum;
  pair<int, int> foodpos[AreaFoodNum];
  raylib::Color col[AreaFoodNum];
} areablock, gamemap[AreaSize][AreaSize];
class Player
{
public:
  bool alive;
  double x, y;
  double energy, waitedenergy;
  raylib::Color col;
  char name[128];
  pair<double, double> move;
  bool isfast;
  int birthtime;
  double getradio()
  {
    return sqrt(max(energy, 0.0)) * 14.0 + 17.0;
  }
  double gethorizon()
  {
    return sqrt(max(energy, 0.0)) * 26.0 + 170.0;
  }
  double getv()
  {
    return -sqrt(getradio()) + 200.0;
  }
  double getfastv()
  {
    return sqrt(max(energy, 0.0)) + 300.0;
  }
  bool operator<(const Player &cmper) const
  {
    return energy > cmper.energy;
  }
} self, others;
// scenes
int startscene()
{
  // raylib::SetConfigFlags(raylib::FLAG_WINDOW_UNDECORATED);
  raylib::SetTraceLogLevel(raylib::LOG_NONE);
  raylib::InitWindow(336, 288, "球球大作战");
  raylib::SetTargetFPS(30);
  raylib::GuiLoadStyleStyle();
  raylib::isButtonDown = false;
  bool dragWindow = false;
  raylib::Vector2 mousePosition = {0};
  raylib::Vector2 windowPosition = raylib::GetWindowPosition();
  raylib::Vector2 panOffset = mousePosition;
  raylib::SetWindowPosition(windowPosition.x, windowPosition.y);
  while (scenedata.WindowBox000Active && !raylib::WindowShouldClose())
  { // Detect window close button or ESC key
    // Update
    mousePosition = raylib::GetMousePosition();
    if (raylib::IsMouseButtonPressed(raylib::MOUSE_LEFT_BUTTON))
    {
      if (raylib::CheckCollisionPointRec(mousePosition, (raylib::Rectangle){0, 0, 336, 20}))
      {
        dragWindow = true;
        panOffset = mousePosition;
      }
    }
    if (dragWindow)
    {
      windowPosition.x += (mousePosition.x - panOffset.x);
      windowPosition.y += (mousePosition.y - panOffset.y);
      if (raylib::IsMouseButtonReleased(raylib::MOUSE_LEFT_BUTTON))
        dragWindow = false;
      raylib::SetWindowPosition(windowPosition.x, windowPosition.y);
    }
    raylib::BeginDrawing();
    raylib::ClearBackground(raylib::WHITE);
    raylib::GuiStartscene(&scenedata);
    raylib::EndDrawing();
    if (raylib::isButtonDown)
    {
      strcpy(ip, scenedata.IPboxText);
      strcpy(name, scenedata.NameboxText);
      port = scenedata.PortboxValue;
      raylib::CloseWindow();
      cout << "INFO: Push the start button" << endl;
      return 0;
    }
  }
  raylib::CloseWindow();
  return -1;
}
int run()
{
  raylib::SetConfigFlags(0);
  raylib::SetTraceLogLevel(raylib::LOG_NONE);
  raylib::InitWindow(ScreenWidth, ScreenHeight, "球球大作战");
  raylib::SetTargetFPS(30);
  raylib::GuiLoadStyleStyle();
  raylib::Image background = raylib::GenImageColor(MapSize, MapSize, bgcolor);
  raylib::Image PlayerLayer = raylib::GenImageColor(MapSize, MapSize, raylib::BLANK);
  raylib::Image rankboard = raylib::GenImageColor(ScreenWidth, ScreenHeight, raylib::BLANK);
  raylib::Texture bgtext, playertext, ranktext;
  auto gameoverwindow = raylib::InitGuiGameOverWindow();

  int status;
  status = send(SocketClient, name, sizeof(name), 0);
  if (status <= 0)
    return -1;
  cout << "INFO: Send Name" << endl;
  cout << "Info: Game Begin" << endl;
  while (!raylib::WindowShouldClose())
  {
    raylib::ImageFillRectangleEx(&PlayerLayer, 0, 0, MapSize, MapSize, raylib::BLANK);
    if (recv(SocketClient, (char *)&self, sizeof(self), 0) <= 0)
      goto fail;
    raylib::ImageFillCircleEx(&PlayerLayer, self.x, self.y, self.getradio(), self.col);
    if (recv(SocketClient, (char *)&countnum, sizeof(countnum), 0) <= 0)
      goto fail;
    int tlx, tly, brx, bry;
    for (int i = 0; i < countnum; ++i)
    {
      if (recv(SocketClient, (char *)&areablock, sizeof(areablock), 0) <= 0)
        goto fail;
      raylib::ImageFillRectangleEx(&background, areablock.x, areablock.y, AreaSize, AreaSize, bgcolor); // clear
      gamemap[areablock.x / AreaSize][areablock.y / AreaSize] = areablock;
      if (!i)
        tlx = areablock.x / AreaSize, tly = areablock.y / AreaSize;
      else if (i + 1 == countnum)
        brx = areablock.x / AreaSize, bry = areablock.y / AreaSize;
    }
    for (int i = tlx; i <= brx; ++i)
    {
      for (int j = tly; j <= bry; ++j)
      {
        for (int k = 0; k < gamemap[i][j].foodnum; ++k)
        {
          raylib::ImageFillCircleEx(&background, gamemap[i][j].foodpos[k].first, gamemap[i][j].foodpos[k].second, FoodRadio, gamemap[i][j].col[k]); // draw food
        }
      }
    }

    if (recv(SocketClient, (char *)&countnum, sizeof(countnum), 0) <= 0)
      goto fail;
    for (int i = 0; i < countnum; ++i)
    {
      if (recv(SocketClient, (char *)&others, sizeof(others), 0) <= 0)
        goto fail;
      if (strlen(others.name) > 10)
      {
        others.name[7] = others.name[8] = others.name[9] = '.';
        others.name[10] = '\0';
      }
      raylib::ImageFillCircleEx(&PlayerLayer, others.x, others.y, others.getradio(), others.col);
      raylib::ImageDrawText(&PlayerLayer, others.name, others.x - self.gethorizon() / 6, others.y, self.gethorizon() / 15, raylib::BLACK);
    }

    if (recv(SocketClient, (char *)&countnum, sizeof(countnum), 0) <= 0)
      goto fail;
    if (recv(SocketClient, (char *)ranklist, sizeof(pair<char[128], double>[countnum]), 0) <= 0)
      goto fail; // TODO,I am too lazy to show it out
    for (int i = 0; i < countnum; ++i)
      rankliststring[i] = {ranklist[i].second, (string)ranklist[i].first};
    sort(rankliststring, rankliststring + countnum, cmp);
    countnum = min(countnum, MaxRankListNum);
    raylib::ImageFillRectangleEx(&rankboard, ScreenWidth - 300, 20, 270, MaxRankListNum * 30 + 51, raylib::BLANK);
    raylib::ImageFillRectangleEx(&rankboard, ScreenWidth - 300, 20, 270, countnum * 30 + 51, RankBoardColor);
    raylib::ImageDrawText(&rankboard, "Rank Board", ScreenWidth - 250, 35, 25, raylib::WHITE);
    for (int i = 0; i < countnum; ++i)
    {
      raylib::ImageDrawText(&rankboard, raylib::TextFormat("%d.%s", i + 1, rankliststring[i].second.c_str()), ScreenWidth - 280, i * 30 + 70, 20, raylib::WHITE);
    }

    PlayerMove = {ScreenWidth / 2, ScreenHeight / 2};
    isfast = 0;
    if (raylib::IsWindowFocused())
    {
      auto tmp = raylib::GetMousePosition();
      PlayerMove = {tmp.x, tmp.y};
      cout << "INFO: Player position:" << self.x << ' ' << self.y << endl;
      cout << "INFO: Camera position:" << camera.target.x << ' ' << camera.target.y << endl;
      isfast = raylib::IsKeyDown(raylib::KEY_SPACE);
    }
    PlayerMove.first -= ScreenWidth / 2;
    PlayerMove.second -= ScreenHeight / 2;
    if (send(SocketClient, (char *)&PlayerMove, sizeof(PlayerMove), 0) <= 0)
      goto fail;
    if (send(SocketClient, (char *)&isfast, sizeof(isfast), 0) <= 0)
      goto fail;

    bgtext = raylib::LoadTextureFromImage(background);
    playertext = raylib::LoadTextureFromImage(PlayerLayer);
    ranktext = raylib::LoadTextureFromImage(rankboard);
    raylib::BeginDrawing();
    raylib::BeginMode2D(camera);
    raylib::ClearBackground(raylib::LIGHTCYAN);
    DrawTexture(bgtext, 0, 0, raylib::WHITE);
    DrawTexture(playertext, 0, 0, raylib::WHITE);
    camera.target = {(float)self.x, (float)self.y};
    double tmpdis = sqrt(PlayerMove.first * PlayerMove.first + PlayerMove.second * PlayerMove.second);
    if (tmpdis > 20)
    {
      PlayerMove.first *= AreaSize / tmpdis;
      PlayerMove.second *= AreaSize / tmpdis;
    }
    camera.target.x -= PlayerMove.first / 2;
    camera.target.y -= PlayerMove.second / 2;
    camera.zoom = ScreenHeight / self.gethorizon() / 2.0;
    raylib::EndMode2D();
    DrawTexture(ranktext, 0, 0, raylib::WHITE);
    raylib::EndDrawing();
    raylib::UnloadTexture(bgtext);
    raylib::UnloadTexture(playertext);
    raylib::UnloadTexture(ranktext);
  }
  raylib::UnloadImage(background);
  raylib::UnloadImage(PlayerLayer);
  raylib::UnloadImage(rankboard);
  raylib::CloseWindow();
  return 0;

fail:
  raylib::UnloadImage(background);
  raylib::UnloadImage(PlayerLayer);
  int tmp = 0;
  while (!tmp && !raylib::WindowShouldClose())
  {
    raylib::BeginDrawing();
    tmp = raylib::GuiGameOverWindow(&gameoverwindow);
    raylib::EndDrawing();
  }
  raylib::CloseWindow();
  return tmp;
}
int main()
{
  HideWindow();
  SetConsoleOutputCP(65001); // use utf-8
  // init window
  int status = 0;
home:
  if (startscene())
    goto end;
restart:
  if (initnet())
    goto home;
  cout << "Info: Connect succesfully!" << endl;
  status = run();
  if (status < 0)
    goto home; // home
  else if (status == 0)
    goto end; // window close
  else
    goto restart; // restart
end:
  closesocket(SocketClient);
  WSACleanup();
  return 0;
}