#include <bits/stdc++.h>
#include <windows.h>
#include <ctime>
#include <thread>
#include <atomic>
namespace raylib
{
#include "raylib.h"
}
#pragma comment(lib, "ws2_32.lib")
#define MapSize 3025
#define AreaSize 55
#define PlayerMax 10
#define AreaFoodNum 2
#define Port 11451
#define EnemyLessTime 5000
#define MaxEnergy 11220
#define ScreenWidth 1600
#define ScreenHeight 900
#define FoodMax 3000
using namespace std;
mt19937 rd(random_device{}());
default_random_engine eng(random_device{}());
uniform_real_distribution<> colorhue(0, 360);
uniform_real_distribution<> colorsat(0.3, 1);
uniform_real_distribution<> colorl(0.78125, 1);
atomic_int PlayerNumber(0);
int foodtot;
inline void MyWait(const int &time)
{
  int st = clock();
  while (clock() - st < time)
    ;
}
vector<pair<int, int>> zeroarea; // areas without food
class area
{
public:
  int x, y; // top-left point
  int foodnum;
  pair<int, int> foodpos[AreaFoodNum];
  raylib::Color col[AreaFoodNum];
  void update()
  {
    foodnum = rd() % (AreaFoodNum + AreaFoodNum);
    if (foodnum > AreaFoodNum)
      foodnum = 0; // add the rate of being empty, you can change the latter adder to change the rate higher or lower.
    if (foodtot >= FoodMax)
      foodnum = 0;
    foodtot += foodnum;
    for (int i = 0; i < foodnum; ++i)
    {
      foodpos[i] = make_pair(x + rd() % AreaSize, y + rd() % AreaSize);
      col[i] = raylib::ColorFromHSV(colorhue(eng), colorsat(eng), colorl(eng));
    }
    if (!foodnum)
    {
      zeroarea.push_back(make_pair(x / AreaSize, y / AreaSize));
    }
  }
} gamemap[AreaSize][AreaSize];
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
  Player(double x = 0, double y = 0) : x(x), y(y)
  {
    waitedenergy = energy = 0;
    col = raylib::ColorFromHSV(colorhue(eng), colorsat(eng), colorl(eng));
    alive = 1;
    move.first = move.second = 0;
    isfast = 0;
    birthtime = clock();
  }
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
};
double dis(const Player &x, const Player &y)
{
  return sqrt(pow(y.x - x.x, 2) + pow(y.y - x.y, 2));
}
double dis(const area &x, const Player &y)
{
  double ans = 0;
  if (y.x < x.x)
    ans += (x.x - y.x) * (x.x - y.x);
  else if (y.x >= x.x + AreaSize)
    ans += (y.x - x.x - AreaSize + 1) * (y.x - x.x - AreaSize + 1);
  if (y.y < x.y)
    ans += (x.y - y.y) * (x.y - y.y);
  else if (y.y >= x.y + AreaSize)
    ans += (y.y - x.y - AreaSize + 1) * (y.y - x.y - AreaSize + 1);
  return sqrt(ans);
}
double dis(const pair<int, int> &x, const Player &y)
{
  return sqrt(pow(y.x - x.first, 2) + pow(y.y - x.second, 2));
}
list<Player> PlayerList;
pair<char[128], double> ranklist[PlayerMax];
bool iscovered[AreaSize][AreaSize]; // if areas are covered by players (not completely covered are included as well)
// the areas covered by players can't generate food
map<string, bool> namemap;
Player NewPlayer()
{
  Player ans;
  for (int i = 0; i < 10; ++i)
  {
    ans.x = rd() % (MapSize - 46) + 18;
    ans.y = rd() % (MapSize - 46) + 18;
    for (auto &j : PlayerList)
    {
      if (dis(j, ans) <= j.getradio())
      {
        goto nxt;
      }
    }
    return ans;
  nxt:;
  }
  ans.x = rd() % (MapSize - 46) + 18;
  ans.y = rd() % (MapSize - 46) + 18;
  return ans;
}
// net part
SOCKET ListenSocket;
DWORD WINAPI PlayerThread(LPVOID lpParameter)
{
  auto ClientPlayer = prev(PlayerList.end());
  SOCKET *ClientSocket = (SOCKET *)lpParameter;
  int status;
  status = recv(*ClientSocket, ClientPlayer->name, sizeof(ClientPlayer->name), 0);
  if (status == SOCKET_ERROR || status == 0)
  {
    --PlayerNumber;
    PlayerList.erase(ClientPlayer);
    closesocket(*ClientSocket);
    free(ClientSocket);
    return 0;
  }
  if (namemap.find((string)ClientPlayer->name) != namemap.end() && namemap[(string)ClientPlayer->name])
    goto done;
  cout << ClientPlayer->name << " join the game!" << endl;
  namemap[(string)ClientPlayer->name] = 1;
  while (ClientPlayer->energy >= 0 && ClientPlayer->alive || clock() - ClientPlayer->birthtime <= EnemyLessTime)
  {
    if (clock() - ClientPlayer->birthtime > EnemyLessTime)
    {
      for (auto it = PlayerList.begin(); it != PlayerList.end(); ++it)
      {
        if (it == ClientPlayer)
          continue;
        if (it->getradio() > ClientPlayer->getradio() && dis(*it, *ClientPlayer) < it->getradio())
        {
          it->waitedenergy += ClientPlayer->energy;
          goto finish;
        }
      }
    }
    /*
    send:
    ClientPlayer
    areasize
    area
    SeenPlayerNumber
    SeenPlayer
    PlayerNumber
    Ranklist
    */
    if (send(*ClientSocket, (char *)&(*ClientPlayer), sizeof(*ClientPlayer), 0) <= 0)
      goto finish;
    int tlx, tly, brx, bry; // top-left (x,y), bottom-right(x,y) horizon
    tlx = ((int)floor((ClientPlayer->x - ClientPlayer->gethorizon() * ScreenWidth / ScreenHeight - 1))) / AreaSize;
    tly = ((int)floor((ClientPlayer->y - ClientPlayer->gethorizon() - 1))) / AreaSize;
    brx = ((int)ceil((ClientPlayer->x + ClientPlayer->gethorizon() * ScreenWidth / ScreenHeight + 1))) / AreaSize;
    bry = ((int)ceil((ClientPlayer->y + ClientPlayer->gethorizon() + 1))) / AreaSize;
    tlx = max(tlx, 0);
    tly = max(tly, 0);
    brx = min(brx, AreaSize - 1);
    bry = min(bry, AreaSize - 1);
    int areasize = (brx - tlx + 1) * (bry - tly + 1);
    if (send(*ClientSocket, (char *)&areasize, sizeof(areasize), 0) <= 0)
      goto finish;
    for (int i = tlx; i <= brx; ++i)
    {
      for (int j = tly; j <= bry; ++j)
      {
        if (send(*ClientSocket, (char *)&gamemap[i][j], sizeof(gamemap[i][j]), 0) <= 0)
          goto finish;
      }
    }
    int SeenPlayerNum = 0;
    for (auto it = PlayerList.begin(); it != PlayerList.end(); ++it)
    {
      if (it == ClientPlayer)
        continue;
      if (dis(*it, *ClientPlayer) - it->getradio() <= ClientPlayer->gethorizon() * sqrt(2))
        ++SeenPlayerNum;
    }
    if (send(*ClientSocket, (char *)&SeenPlayerNum, sizeof(SeenPlayerNum), 0) <= 0)
      goto finish;
    for (auto it = PlayerList.begin(); it != PlayerList.end(); ++it)
    {
      if (it == ClientPlayer)
        continue;
      if (dis(*it, *ClientPlayer) - it->getradio() <= ClientPlayer->gethorizon() * sqrt(2))
      {
        if (send(*ClientSocket, (char *)&(*it), sizeof(*it), 0) <= 0)
          goto finish;
      }
    }

    int totPlayerNum = PlayerNumber.load();
    if (send(*ClientSocket, (char *)&totPlayerNum, sizeof(totPlayerNum), 0) <= 0)
      goto finish;
    send(*ClientSocket, (char *)ranklist, sizeof(pair<char[128], double>[totPlayerNum]), 0);

    if (recv(*ClientSocket, (char *)&(ClientPlayer->move), sizeof(ClientPlayer->move), 0) <= 0)
      goto finish;
    if (recv(*ClientSocket, (char *)&(ClientPlayer->isfast), sizeof(ClientPlayer->isfast), 0) <= 0)
      goto finish;
  }
finish:
  namemap[(string)ClientPlayer->name] = 0;
  cout << ClientPlayer->name << " leave the game!" << endl;
done:
  --PlayerNumber;
  PlayerList.erase(ClientPlayer);
  closesocket(*ClientSocket);
  free(ClientSocket);
  return 0;
}
void PlayerAddListen()
{
  cout << "Start successfully!\n";
  while (true)
  {
    SOCKET *ClientSocket = new SOCKET;
    ClientSocket = (SOCKET *)malloc(sizeof(SOCKET));
    int SockAddrlen = sizeof(sockaddr);
    *ClientSocket = accept(ListenSocket, 0, 0);
    if (PlayerNumber == PlayerMax)
    { // to avoid the player number exceed the limit
      closesocket(*ClientSocket);
      free(ClientSocket);
      continue;
    }
    cout << "A client has connected to the server, and the socket is: " << *ClientSocket << endl;
    ++PlayerNumber;
    PlayerList.push_back(NewPlayer());
    CreateThread(NULL, 0, &PlayerThread, ClientSocket, 0, NULL);
  }
}
void netinit()
{
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    cout << "Fail to load winsock.dll" << endl;
    exit(-1);
  }
  ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
  SOCKADDR_IN ListenAddr;
  ListenAddr.sin_family = AF_INET;
  ListenAddr.sin_addr.S_un.S_addr = INADDR_ANY;
  ListenAddr.sin_port = htons(Port);
  int status;
  status = bind(ListenSocket, (LPSOCKADDR)&ListenAddr, sizeof(ListenAddr));
  if (status == SOCKET_ERROR)
  {
    cout << "Fail to bind the port" << endl;
    exit(-2);
  }
  status = listen(ListenSocket, 10);
  thread thListenNewPlayerAdd(PlayerAddListen);
  thListenNewPlayerAdd.detach();
}
void run()
{
  for (int i = 0; i < AreaSize; ++i)
  {
    for (int j = 0; j < AreaSize; ++j)
    {
      gamemap[i][j].x = i * AreaSize;
      gamemap[i][j].y = j * AreaSize;
      gamemap[i][j].update();
    }
  }
  int st, lst = clock();
  while (true)
  {
    memset(iscovered, 0, sizeof(iscovered));
    st = clock();
    // player's move
    for (auto &i : PlayerList)
    {
      double dis = sqrt(i.move.first * i.move.first + i.move.second * i.move.second);
      if (!i.isfast)
      {
        i.x += i.getv() * i.move.first / (dis + 20) * (st - lst) / 1000.0;
        i.y += i.getv() * i.move.second / (dis + 20) * (st - lst) / 1000.0;
      }
      else
      {
        i.x += i.getfastv() * i.move.first / (dis + 10) * (st - lst) / 1000.0;
        i.y += i.getfastv() * i.move.second / (dis + 10) * (st - lst) / 1000.0;
        if (clock() - i.birthtime > EnemyLessTime)
          i.energy -= (0.4 * i.energy + 1) * (st - lst) / 1000.0;
      }
      if (i.x > (double)MapSize - i.getradio())
        i.x = (double)MapSize - i.getradio();
      else if (i.x < i.getradio())
        i.x = i.getradio();
      if (i.y > (double)MapSize - i.getradio())
        i.y = (double)MapSize - i.getradio();
      else if (i.y < i.getradio())
        i.y = i.getradio();
      i.move = {0, 0};
    }
    // eat food
    for (auto &i : PlayerList)
    {
      int tlx, tly, brx, bry; // top-left (x,y), bottom-right(x,y)
      tlx = ((int)floor((i.x - i.getradio()))) / AreaSize;
      tly = ((int)floor((i.y - i.getradio()))) / AreaSize;
      brx = ((int)ceil((i.x + i.getradio()))) / AreaSize;
      bry = ((int)ceil((i.y + i.getradio()))) / AreaSize;
      for (int j = tlx; j <= brx; ++j)
      {
        for (int k = tly; k <= bry; ++k)
        {
          if (j < 0 || j >= AreaSize || k < 0 || k >= AreaSize)
            continue;
          if (dis(gamemap[j][k], i) > i.getradio())
            continue;
          iscovered[j][k] = true;
          if (!gamemap[j][k].foodnum)
            continue;
          int l, r;
          for (l = 0, r = 0; r < gamemap[j][k].foodnum; ++r)
          { // eat food
            if (dis(gamemap[j][k].foodpos[r], i) > i.getradio())
            {
              gamemap[j][k].col[l] = gamemap[j][k].col[r];
              gamemap[j][k].foodpos[l++] = gamemap[j][k].foodpos[r];
            }
            else
            {
              --foodtot;
              i.waitedenergy += 1;
            }
          }
          gamemap[j][k].foodnum = l;
          if (!l)
            zeroarea.push_back(make_pair(j, k));
        }
      }
    }
    // digest food
    for (auto &i : PlayerList)
    {
      if (i.waitedenergy > 0)
      {
        ++i.energy;
        --i.waitedenergy;
        if (i.energy > MaxEnergy)
          i.energy = MaxEnergy;
      }
      if (clock() - i.birthtime > EnemyLessTime)
        i.energy -= 1 * (st - lst) / 1000.0;
    }
    int pointer = 0;
    for (auto &i : PlayerList)
    {
      strcpy(ranklist[pointer].first, i.name);
      ranklist[pointer++].second = i.energy;
    }
    {
      int j = 0, k = zeroarea.size();
      random_shuffle(zeroarea.begin(), zeroarea.end());
      for (auto i = zeroarea.begin(); i != zeroarea.end() && j < k; ++j)
      {
        if (!iscovered[i->first][i->second] && rd() % 3 == 0 && foodtot < FoodMax)
        { // You can change the possibility as well
          gamemap[i->first][i->second].update();
          i = zeroarea.erase(i);
        }
        else
        {
          ++i;
        }
        if (foodtot >= FoodMax)
          break;
      }
    }
    lst = st;
    MyWait(33 - clock() + st);
  }
}
int main()
{
  SetConsoleOutputCP(65001); // use utf-8
  netinit();
  run();
  closesocket(ListenSocket);
  WSACleanup();
  return 0;
}