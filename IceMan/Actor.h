#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <stack>

struct Position
{
	Position(int posx, int posy)
	{
		x = posx;
		y = posy;
	}
	int x;
	int y;
};

class StudentWorld;

class Base : public GraphObject
{
public:
	Base(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth);
	virtual void doSomething() = 0;
	bool isActive();
	void donezo();
	int ticklife();
	void increasetick();
	void resettick();
	void setTickLife(int set);
private:
	bool m_active;
	int m_ticklife;
};

class Ice : public Base
{
public:
	Ice(int x, int y) : Base(IID_ICE, x, y, right, 0.25, 3) {}
	virtual ~Ice() {}
	virtual void doSomething() {}
private:
};

class Actor : public Base
{
public:
	Actor(int id, int x, int y, Direction dir, int depth, StudentWorld* game, int hp);
	virtual ~Actor() {}
	virtual void doSomething() = 0;
	virtual int getHealth() { return m_hp; }
	StudentWorld* getGame() { return m_game; }
	void takeDamage(int dmg);
private:
	int m_hp;
	StudentWorld* m_game;
};

class Iceman : public Actor
{
public:
	Iceman(StudentWorld* game);
	virtual ~Iceman() {}
	virtual void doSomething();
	void waterRefill();
	void sonarRefill();
	void goldRefill();
	int getWater() { return m_squirts; }
	int getSonar() { return m_sonar; }
	int getGold() { return m_gold; }
private:
	int m_squirts;
	int m_sonar;
	int m_gold;
};

class Protestor : public Actor
{
public:
	Protestor(int id, StudentWorld* game, Iceman* player, int hp);
	virtual ~Protestor() {}
	virtual void doSomething() = 0;
	virtual void pickUpNugget() = 0;
	int getState() { return m_state; }
	int getRestTicks() { return ticksToWaitBetweenMoves; }
	void increaseRestTicks(int inc) { ticksToWaitBetweenMoves += inc; }
	void resetRestTicks();
	int getYellTicks() { return m_yellticks; }
	void increaseYellTicks() { m_yellticks++; }
	void resetYellTicks() { m_yellticks = 0; }
	int getTurnTicks() { return m_turnticks; }
	void increaseTurnTicks() { m_turnticks++; }
	void resetTurnTicks() { m_turnticks = 0; }
	void changeState(); //0: moving, 1: leave oil field
	int getSteps() { return numSquaresToMoveInCurrentDirection; }
	void decSteps() { numSquaresToMoveInCurrentDirection--; }
	void resetSteps() { numSquaresToMoveInCurrentDirection = 0; }
	Iceman* getPlayer() { return player; }
	void decideSteps();
	void yell(); //Reduces player hp by 2
	void getNewDirection();
	virtual void squirtKill() = 0;
	void oneStepTowardsExit();
private:
	Iceman* player;
	std::stack<Position*> leaving;
	int m_state;
	int m_yellticks;
	int m_turnticks;
	int ticksToWaitBetweenMoves;
	int numSquaresToMoveInCurrentDirection;
};

class RegularProtestor : public Protestor
{
public:
	RegularProtestor(StudentWorld* game, Iceman* player) : Protestor(IID_PROTESTER, game, player, 5) {}
	virtual ~RegularProtestor() {}
	virtual void doSomething();
	virtual void pickUpNugget();
	virtual void squirtKill();
};

class HardcoreProtestor : public Protestor
{
public:
	HardcoreProtestor(StudentWorld* game, Iceman* player);
	virtual ~HardcoreProtestor() {}
	virtual void doSomething();
	virtual void pickUpNugget();
	virtual void squirtKill();
private:
	int m_pursuitSteps;
	std::stack<Position*> pursuit;
};

class PowerUp : public Base
{
public:
	PowerUp(int id, int x, int y, Direction dir, int depth, StudentWorld* game);
	~PowerUp() {}
	virtual void doSomething() = 0;
	StudentWorld* getGame();
	virtual bool boulder() { return false; } //To make boulders tangible
	virtual bool isSonar() { return false; } //For sonar spawns
	virtual bool isNugget() { return false; } //To trigger protestor picking up nugget2
private:
	StudentWorld* m_game;
};

class Damage : public PowerUp
{
public:
	Damage(int id, int x, int y, Direction dir, int depth, StudentWorld* game, int damage);
	virtual ~Damage() {}
	virtual void doSomething() = 0;
	int getDamage() { return m_damage; }
	virtual bool boulder() { return false; }
	virtual bool isNugget() { return false; } //To trigger protestor picking up nugget2
	virtual bool isSquirt() { return false; } //To trigger stun

private:
	int m_damage;
};

class Squirt : public Damage
{
public:
	Squirt(int x, int y, StudentWorld* game, Iceman* player);
	virtual ~Squirt() {}
	virtual void doSomething();
	virtual bool isSquirt() { return true; }
private:
	Iceman* m_player;
};

class Boulder : public Damage
{
public:
	Boulder(int x, int y, StudentWorld* game);
	virtual ~Boulder() {}
	virtual void doSomething();
	int getState(); //0 is stable, 1 is waiting, 2 is stable
	void stateChange() { m_state++; }
	virtual bool boulder() { return true; }
private:
	int m_state;
};

class Nugget2 : public Damage
{
public:
	Nugget2(int x, int y, StudentWorld* game) : Damage(IID_GOLD, x, y, right, 2, game, 5) {}
	virtual ~Nugget2() {}
	virtual void doSomething();
	virtual bool isNugget() { return true; } //To trigger protestor picking up nugget2
};

class WaterPool : public PowerUp
{
public:
	WaterPool(int x, int y, StudentWorld* game) : PowerUp(IID_WATER_POOL, x, y, right, 2, game) {}
	virtual ~WaterPool() {}
	virtual void doSomething();
private:
};

class Barrel : public PowerUp
{
public:
	Barrel(int x, int y, StudentWorld* game);
	virtual ~Barrel() {}
	virtual void doSomething();
};

class SonarKit : public PowerUp
{
public:
	SonarKit(StudentWorld* game) : PowerUp(IID_SONAR, 0, 60, right, 2, game) {}
	virtual ~SonarKit() {}
	virtual void doSomething();
	virtual bool isSonar() { return true; }
};

class Nugget1 : public PowerUp
{
public:
	Nugget1(int x, int y, StudentWorld* game) : PowerUp(IID_GOLD, x, y, right, 2, game) { setVisible(false); }
	virtual ~Nugget1() {}
	virtual void doSomething();
};

#endif // ACTOR_H_
