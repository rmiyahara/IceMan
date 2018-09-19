#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <cmath>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

int StudentWorld::init()
{
	player = new Iceman(this); //make new Iceman, set values below
	for(int i = 0; i < VIEW_WIDTH; i++) //Set up Ice
	{
		for (int j = 0; j < 60; j++)
		{
			if (i >= 30 && i <= 33 && j >= 4)
				field[i][j] = nullptr;
			else
				field[i][j] = new Ice(i, j);;
		}
	}
	for (int i = 0; i < VIEW_WIDTH; i++) //Set top layer to nullptr to evade undefined behavior
		for (int j = 60; j < 64; j++)
			field[i][j] = nullptr;

	m_maxprotestors = min(15, 2 + getLevel() * 1.5);
	m_protestorspawntick = max(25, 200 - getLevel());

	m_numboulders = min(getLevel() / 2 + 2, 9);
	for (int i = 0; i < m_numboulders; i++)
	{
		int x = -1;
		int y = -1;
		for (;;)
		{
			x = rand() % 61;
			y = rand() % 37 + 20;
			if (x < 26 || x > 33 && spawnCheck(x, y))
				break;
		}
		PowerUp* p = new Boulder(x, y, this);
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				delete field[p->getX() + j][p->getY() + k];
				field[p->getX() + j][p->getY() + k] = nullptr;
			}
		}
		m_items.push_back(p);
	}

	m_numbarrels = min(2 + getLevel(), 21); //Set up barrels
	for (int i = 0; i < m_numbarrels; i++)
	{
		int x, y;
		for(;;)
		{
			x = rand() % 61;
			y = rand() % 57;
			if (y == 0 && spawnCheck(x, y))
				break;
			else if ((x <= 26 || x >= 33) && spawnCheck(x, y))
				break;
		}
	PowerUp* p = new Barrel(x, y, this);
		m_items.push_back(p);
	}

	m_numgold = max(5 - getLevel() / 2, 2); //Set up gold
	for (int i = 0; i < m_numgold; i++)
	{
		int x, y;
		for (;;)
		{
			x = rand() % 61;
			y = rand() % 57;
			if (y == 0 && spawnCheck(x, y))
				break;
			else if ((x <= 26 || x >= 33) && spawnCheck(x, y))
				break;
		}
		PowerUp* p = new Nugget1(x, y, this);
		m_items.push_back(p);
	}

	goodieSpawn = getLevel() * 25 + 300;

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	player->doSomething();
	if (!player->isActive())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	for (int i = 0; i < m_items.size(); i++)
		m_items[i]->doSomething();

	for (int i = 0; i < m_items.size();)
	{
		if (!m_items[i]->isActive())
		{
			delete m_items[i];
			m_items[i] = nullptr;
			m_items.erase(m_items.begin() + i);
		}
		else
			i++;
	}
	for (int i = 0; i < m_protestors.size(); i++)
		m_protestors[i]->doSomething();

	if (m_maxprotestors > m_protestors.size() && m_protestorspawntick >= max(25, 200 - getLevel())) //Will spawn protestor
	{
		int x = rand() % 100 + 1;
		if (x < min(90, getLevel() * 10 + 30))
		{
			Protestor* p = new HardcoreProtestor(this, player);
			m_protestors.push_back(p);
		}
		else
		{
			Protestor* p = new RegularProtestor(this, player);
			m_protestors.push_back(p);
		}
		m_protestorspawntick = 0;

	}

	int goodie = rand() % goodieSpawn; //Spawn item
	if (goodie == 0)
	{
		int whatgoodie = rand() % 5;
		if (whatgoodie == 0 && !sonarHere()) //Spawn sonar
		{
			PowerUp* p = new SonarKit(this);
			m_items.push_back(p);
		}
		else //Spawn water pool
		{
			int x, y;
			int waterHere = 0;
			while (waterHere != 16)
			{
				waterHere = 0;
				x = rand() % 61;
				y = rand() % 57;
				if (spawnCheck(x, y) && field[x][y] == nullptr)
				{
					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							if (field[x + i][y + j] == nullptr)
								waterHere++;
						}
					}
				}
			}
			PowerUp* p = new WaterPool(x, y, this);
			m_items.push_back(p);
		}
	}

	for (int i = 0; i < m_protestors.size();)
	{
		if (!m_protestors[i]->isActive())
		{
			delete m_protestors[i];
			m_protestors[i] = nullptr;
			m_protestors.erase(m_protestors.begin() + i);
		}
		else
			i++;
	}

	m_protestorspawntick++;
	setGameStatText(setStatusLine());

	if (m_numbarrels <= 0)
	{
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}
	else if (player->getHealth() <= 0)
	{
		decLives();
		playSound(SOUND_PLAYER_GIVE_UP);
		return GWSTATUS_PLAYER_DIED;
	}
	else if (player->isActive())
		return GWSTATUS_CONTINUE_GAME;

	return GWSTATUS_CONTINUE_GAME; //you should never get here
}

void StudentWorld::cleanUp()
{
	for (int i = 0; i < VIEW_WIDTH; i++) //delete all Ice
	{
		for (int j = 0; j < 60; j++)
		{
			if (field[i][j] != nullptr)
			{
				delete field[i][j];
				field[i][j] = nullptr;
			}
		}
	}
	delete player; //delete player
	for (int i = 0; i < m_items.size();) //delete all items
	{
		delete m_items[i];
		m_items[i] = nullptr;
		m_items.erase(m_items.begin() + i);
	}
	for (int i = 0; i < m_protestors.size();) //delete all protestors
	{
		delete m_protestors[i];
		m_protestors[i] = nullptr;
		m_protestors.erase(m_protestors.begin() + i);
	}
}

string StudentWorld::setStatusLine()
{
	string lvl = to_string(getLevel());
	while (lvl.size() < 2)
		lvl.insert(0, " ");
	string lives = to_string(getLives());
	string health = to_string(player->getHealth() * 10);
	while (health.size() < 3)
		health.insert(0, " ");
	string water = to_string(player->getWater());
	while (water.size() < 2)
		water.insert(0, " ");
	string gold = to_string(player->getGold());
	while (gold.size() < 2)
		gold.insert(0, " ");
	string oil = to_string(m_numbarrels);
	while (oil.size() < 2)
		oil.insert(0, " ");
	string sonar = to_string(player->getSonar());
	while (sonar.size() < 2)
		sonar.insert(0, " ");
	string score = to_string(getScore());
	while (score.size() < 6)
		score.insert(0, "0");

	return ("Lvl: " + lvl + "  Lives: " + lives + "  Hlth: " + health + "%  Wtr: " + water + "  Gld: " + gold + "  Oil Left: " + oil + "  Sonar: " + sonar + "  Scr: " + score);
}

bool StudentWorld::isBoulder(int x, int y, double radius)
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (m_items[i]->boulder()) //if the item is a boulder
		{
			if (distance(m_items[i]->getX() + 2, m_items[i]->getY() + 2, x + 2, y + 2) <= radius && distance(m_items[i]->getX() + 2, m_items[i]->getY() + 2, x + 2, y + 2) != 1)
				return true;
		}
	}
	return false;
}

bool StudentWorld::spawnCheck(int x, int y)
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (distance(m_items[i]->getX() + 2, m_items[i]->getY() + 2, x + 2, y + 2) <= 6.0)
			return false;
	}
	return true;
}
void StudentWorld::dig(Iceman* player)
{
	bool dug = false;
	for (int i = player->getX(); i <= player->getX() + 3; i++)
	{
		for (int j = player->getY(); j <= player->getY() + 3; j++)
		{
			if (field[i][j] != nullptr && j < 60)
			{
				delete field[i][j];
				field[i][j] = nullptr;
				dug = true;
			}
		}
	}
	if (dug)
		playSound(SOUND_DIG);
}

void StudentWorld::shoot(Iceman* player)
{
	PowerUp* p;
	switch (player->getDirection())
	{
	case up:
		if (player->getY() <= 60)
		{
			bool canSpawn = true;
			for (int i = 0; i < 4; i++)
			{
				if (!itemMoveUp(player->getX(), player->getY() + i))
				{
					canSpawn = false;
					break;
				}
			}
			if (canSpawn == true)
			{
				p = new Squirt(player->getX(), player->getY() + 4, this, player);
				m_items.push_back(p);
			}
		}
		break;
	case down:
		if (player->getY() >= 4)
		{
			bool canSpawn = true;
			for (int i = 0; i < 4; i++)
			{
				if (!itemMoveDown(player->getX(), player->getY() - i))
				{
					canSpawn = false;
					break;
				}
			}
			if (canSpawn == true)
			{
				p = new Squirt(player->getX(), player->getY() - 4, this, player);
				m_items.push_back(p);
			}
		}
		break;
	case Direction::left:
		if (player->getX() >= 4)
		{
			bool canSpawn = true;
			for (int i = 0; i < 4; i++)
			{
				if (!itemMoveLeft(player->getX() - i, player->getY()))
				{
					canSpawn = false;
					break;
				}
			}
			if (canSpawn == true)
			{
				p = new Squirt(player->getX() - 4, player->getY(), this, player);
				m_items.push_back(p);
			}
		}
		break;
	case Direction::right:
	{
		if (player->getX() < 57)
		{
			bool canSpawn = true;
			for (int i = 0; i < 4; i++)
			{
				if (!itemMoveRight(player->getX() + i, player->getY()))
				{
					canSpawn = false;
					break;
				}
			}
			if (canSpawn == true)
			{
				p = new Squirt(player->getX() + 4, player->getY(), this, player);
				m_items.push_back(p);
			}
		}
	}
		break;
	}
	playSound(SOUND_PLAYER_SQUIRT);
}

bool StudentWorld::itemMoveUp(int x, int y)
{
	bool canMove = false;
	for (int i = x; i < x + 4; i++)
	{
		if (y < 60)
		{
			if (field[i][y + 4] == nullptr)
				canMove = true;
			else
				return false;
		}
		else
			return false;

		if (isBoulder(x, y + 1, 3.0))
			return false;
	}
	return canMove;
}

bool StudentWorld::itemMoveDown(int x, int y)
{
	bool canMove = false;
	for (int i = x; i < x + 4; i++)
	{
		if (y > 0)
		{
			if (field[i][y - 1] == nullptr)
				canMove = true;
			else
				return false;

			if (isBoulder(x, y - 1, 3.0))
				return false;
		}
		else
			return false;
	}
	return canMove;
}

bool StudentWorld::itemMoveLeft(int x, int y)
{
	bool canMove = false;
	for (int j = y; j < y + 4; j++)
	{
		if (x > 0)
		{
			if (field[x - 1][j] == nullptr)
				canMove = true;
			else
				return false;
		}
		else
			return false;

		if (isBoulder(x - 1, y, 3.0))
			return false;

	}
	return canMove;
}

bool StudentWorld::itemMoveRight(int x, int y)
{
	bool canMove = false;
	for (int j = y; j < y + 4; j++)
	{
		if (x < 60)
		{
			if (field[x + 4][j] == nullptr)
				canMove = true;
			else
				return false;
		}
		else
			return false;

		if (isBoulder(x + 1, y, 3.0))
			return false;
	}
	return canMove;
}

bool StudentWorld::pickUp(int x, int y, double radius)
{
	if (distance(x, y, player->getX() + 2, player->getY() + 2) <= radius)
		return true;
	else
		return false;
}

bool StudentWorld::pickUpProtestor(int x, int y, double radius, Damage* item)
{
	for (int i = 0; i < m_protestors.size(); i++)
	{
		if (distance(x, y, m_protestors[i]->getX() + 2, m_protestors[i]->getY() + 2) <= radius && m_protestors[i]->getState() == 0)
		{
			if (item->isNugget())
			{
				m_protestors[i]->pickUpNugget();
				return true;
			}
			else if (item->isSquirt())
			{
				m_protestors[i]->increaseRestTicks(max(50, 100 - getLevel() * 10));
				damageProtestor(m_protestors[i], item->getDamage());
				if (m_protestors[i]->getHealth() <= 0)
					m_protestors[i]->squirtKill();
			}
			else
			{
				damageProtestor(m_protestors[i], item->getDamage());
				increaseScore(500);
			}
			if (m_protestors[i]->getHealth() > 0)
				playSound(SOUND_PROTESTER_ANNOYED);
			else
			{
				m_protestors[i]->changeState();
				m_protestors[i]->setTickLife(max(0, 3 - getLevel() / 4));
				m_protestors[i]->resetRestTicks();
				playSound(SOUND_PROTESTER_GIVE_UP);
			}
			return true;
		}
	}
	return false;
}



void StudentWorld::decreaseBarrels()
{
	m_numbarrels--;
}

void StudentWorld::sonar(Iceman* player)
{
	playSound(SOUND_SONAR);
	for (int i = 0; i < m_items.size(); i++)
	{
		if (!m_items[i]->isVisible())
		{
			if(distance(m_items[i]->getX() + 2, m_items[i]->getY() + 2, player->getX() + 2, player->getY() + 2) <= 12.0)
				m_items[i]->setVisible(true);
		}
	}
}

bool StudentWorld::sonarHere()
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (m_items[i]->isSonar())
			return true;
	}
	return false;
}

void StudentWorld::dropGold(Iceman* player)
{
	PowerUp* p = new Nugget2(player->getX(), player->getY(), this);
	m_items.push_back(p);
}

bool StudentWorld::lineOfSight(GraphObject::Direction& dir, Protestor* protestor)
{
	if (player->getY() == protestor->getY())
	{
		if (player->getX() > protestor->getX()) //Player is to the right of the protestor
		{
			for (int i = protestor->getX(); i < player->getX(); i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (field[i][protestor->getY() + j] != nullptr)
						return false;
				}
				if (isBoulder(i, protestor->getY(), 3.0))
					return false;
			}
			dir = GraphObject::Direction::right;
			return true;
		}
		else if (player->getX() < protestor->getX()) //Player is to the left
		{
			for (int i = protestor->getX(); i > player->getX(); i--)
			{
				for (int j = 0; j < 4; j++)
				{
					if (field[i][protestor->getY() + j] != nullptr)
						return false;
				}
				if (isBoulder(i, protestor->getY(), 3.0))
					return false;
			}
			dir = GraphObject::Direction::left;
			return true;
		}
		else //You should never get here
			return false;
	}
	else if (player->getX() == protestor->getX())
	{
		if (player->getY() > protestor->getY()) //Player is to the above of the protestor
		{
			for (int i = protestor->getY(); i < player->getY(); i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (field[protestor->getX() + j][i] != nullptr)
						return false;
				}
				if (isBoulder(protestor->getX(), i, 3.0))
					return false;
			}
			dir = GraphObject::Direction::up;
			return true;
		}
		else if (player->getY() < protestor->getY()) //Player is below
		{
			for (int i = protestor->getY(); i > player->getY(); i--)
			{
				for (int j = 0; j < 4; j++)
				{
					if (field[protestor->getX() + j][i] != nullptr)
						return false;
				}
				if (isBoulder(protestor->getX(), i, 3.0))
					return false;
			}
			dir = GraphObject::Direction::down;
			return true;
		}
		else //You should never get here
			return false;
	}
	else
		return false;
}

bool StudentWorld::atJunction(int x, int y)
{
	int numberWays = 0;
	if (itemMoveUp(x, y) || itemMoveDown(x, y))
		numberWays++;
	if (itemMoveRight(x, y) || itemMoveLeft(x, y))
		numberWays++;
	if (numberWays > 1)
		return true;
	else return false;
}

void StudentWorld::damageProtestor(Protestor* protestor, int damage)
{
	protestor->takeDamage(damage);
}

stack<Position*> StudentWorld::protestorLeaving(Protestor* user, int goalX, int goalY)
{
	int graph[64][64];
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			graph[i][j] = -1;
 	queue<Position*> q;
	Position* origin = new Position(user->getX(), user->getY());
	graph[origin->x][origin->y] = 0;
	q.push(origin);
	int i = 1;
	for (;;)
	{
		Position* temp = q.front();
		q.pop();
		if (temp->x == goalX && temp->y == goalY) //Solution found
		{
			delete temp;
			break;
		}

		if (itemMoveUp(temp->x, temp->y) && graph[temp->x][temp->y + 1] == -1)
		{
			Position* up = new Position(temp->x, temp->y + 1);
			q.push(up);
			graph[up->x][up->y] = graph[temp->x][temp->y] + 1;
		}
		if (itemMoveDown(temp->x, temp->y) && graph[temp->x][temp->y - 1] == -1)
		{
			Position* down = new Position(temp->x, temp->y - 1);
			q.push(down);
			graph[down->x][down->y] = graph[temp->x][temp->y] + 1;
		}
		if (itemMoveLeft(temp->x, temp->y) && graph[temp->x - 1][temp->y] == -1)
		{
			Position* left = new Position(temp->x - 1, temp->y);
			q.push(left);
			graph[left->x][left->y] = graph[temp->x][temp->y] + 1;
		}
		if (itemMoveRight(temp->x, temp->y) && graph[temp->x + 1][temp->y] == -1)
		{
			Position* right = new Position(temp->x + 1, temp->y);
			q.push(right);
			graph[right->x][right->y] = graph[temp->x][temp->y] + 1;
		}
		delete temp;

		if (q.empty()) //Should never get here
		{
			stack<Position*> dummy;
			return dummy;
		}
	}


	while(!q.empty())
	{
		Position* hold = q.front();
		q.pop();
		delete hold;
		hold = nullptr;
	}

	stack<Position*> getOut;
	Position* end = new Position(goalX, goalY);
	getOut.push(end);
	for (;;) //Load up the move stack
	{
		Position* temp = getOut.top();
		if (graph[temp->x][temp->y + 1] == graph[temp->x][temp->y] - 1)
		{
			Position* up = new Position(temp->x, temp->y + 1);
			getOut.push(up);
		}
		else if (temp->y > 0 && graph[temp->x][temp->y - 1] == graph[temp->x][temp->y] - 1)
		{
			Position* down = new Position(temp->x, temp->y - 1);
			getOut.push(down);
		}
		else if (temp->x > 0 && graph[temp->x - 1][temp->y] == graph[temp->x][temp->y] - 1)
		{
			Position* left = new Position(temp->x - 1, temp->y);
			getOut.push(left);
		}
		else if (graph[temp->x + 1][temp->y] == graph[temp->x][temp->y] - 1)
		{
			Position* right = new Position(temp->x + 1, temp->y);
			getOut.push(right);
		}

		Position* check = getOut.top();
		if (graph[check->x][check->y] == 0)
			break;
	}
	return getOut;
}
int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

int min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

double distance(int x1, int y1, int x2, int y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}