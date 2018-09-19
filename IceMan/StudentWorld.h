#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "Actor.h"
#include <string>
#include <vector>

enum Direction { none, up, down, left, right };
class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir) {}
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	Iceman* getPlayer() { return player; } //returns the player
	std::string setStatusLine(); //Used to format the status line
	bool isBoulder(int x, int y, double radius); //Returns true if a boulder is at the point passed to it
	bool spawnCheck(int x, int y); //Returns true if no distributed items are within 6.0 radius
	void dig(Iceman* player); //Deletes Ice as the player passes through it
	void shoot(Iceman* player); //Makes a new squirt in front of the player
	bool itemMoveUp(int x, int y); //Makes sure the next place for a moving squirt is valid
	bool itemMoveDown(int x, int y);
	bool itemMoveLeft(int x, int y);
	bool itemMoveRight(int x, int y);
	bool pickUp(int x, int y, double radius); //Returns true if within radius
	bool pickUpProtestor(int x, int y, double radius, Damage* item);
	void decreaseBarrels(); //Decreases barrel count by 1
	void sonar(Iceman* player); //When a sonar charge is used
	bool sonarHere(); //Returns true if a sonar is already spawned
	void dropGold(Iceman* player); //When player drops a nugget
	bool lineOfSight(GraphObject::Direction& dir, Protestor* protestor); //Returns true if the protestor has a clear line of sight and changes dir to that direction.
	bool atJunction(int x, int y); //Returns true if there are more than one path at a certain spot
	void damageProtestor(Protestor* protestor, int damage);
	std::stack<Position*> protestorLeaving(Protestor* user, int goalX, int goalY); //Gets the Protestor out

private:
	std::vector<PowerUp*> m_items;
	std::vector<Protestor*> m_protestors;
	Iceman* player;
	Ice* field[64][64];
	int m_maxprotestors;
	int m_protestorspawntick;
	int m_numbarrels;
	int m_numboulders;
	int m_numgold;
	int goodieSpawn;
};

	int max(int a, int b);
	int min(int a, int b);
	double distance(int x1, int y1, int x2, int y2); //Calculates radius

#endif // STUDENTWORLD_H_
