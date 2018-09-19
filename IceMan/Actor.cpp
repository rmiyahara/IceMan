#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <queue>
#include <stack>
#include <iostream>

Base::Base(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth)
	:GraphObject(imageID, startX, startY, dir, size, depth)
{
	m_active = true;
	m_ticklife = 0;
	setVisible(true);
}

bool Base::isActive()
{
	return m_active;
}

void Base::donezo()
{
	m_active = false;
}

int Base::ticklife()
{
	return m_ticklife;
}

void Base::increasetick()
{
	m_ticklife++;
}

void Base::resettick()
{
	m_ticklife = 0;
}

void Base::setTickLife(int set)
{
	m_ticklife = set;
}

PowerUp::PowerUp(int id, int x, int y, Direction dir, int depth, StudentWorld* game)
	:Base(id, x, y, dir, 1.0, depth)
{
	m_game = game;
}

StudentWorld* PowerUp::getGame()
{
	return m_game;
}

Damage::Damage(int id, int x, int y, Direction dir, int depth, StudentWorld* game, int damage)
	:PowerUp(id, x, y, dir, depth, game)
{
	m_damage = damage;
}

Squirt::Squirt(int x, int y, StudentWorld* game, Iceman* player)
	:Damage(IID_WATER_SPURT, x, y, player->getDirection(), 1, game, 2)
{
	m_player = player;
}

void Squirt::doSomething()
{
	if (!isActive())
		return;

	if (getGame()->pickUpProtestor(getX(), getY(), 3.0, this))
		donezo();
	
	switch (getDirection())
	{
	case up:
		if (getGame()->itemMoveUp(getX(), getY()))
		{
			moveTo(getX(), getY() + 1);
		}
		else
			donezo();
		break;
	case down:
		if (getGame()->itemMoveDown(getX(), getY()))
			moveTo(getX(), getY() - 1);
		else
			donezo();
		break;
	case left:
		if (getGame()->itemMoveLeft(getX(), getY()))
			moveTo(getX() - 1, getY());
		else
			donezo();
		break;
	case right:
		if (getGame()->itemMoveRight(getX(), getY()))
			moveTo(getX() + 1, getY());
		else
			donezo();
		break;
	}
	if (ticklife() == 4)
		donezo();
	else
		increasetick();
	return; 
}

Boulder::Boulder(int x, int y, StudentWorld* game)
	:Damage(IID_BOULDER, x, y, right, 1, game, 100)
{
	m_state = 0;
}

int Boulder::getState()
{
	return m_state;
}

void Boulder::doSomething()
{
	if (!isActive())
		return;
	switch (getState())
	{
	case 0: //Stable
		if (getGame()->itemMoveDown(getX(), getY()))
			stateChange();
		break;
	case 1: //Waiting
		if (ticklife() == 30)
			stateChange();
		else
			increasetick();
		break;
	case 2: //Falling
		if (getGame()->itemMoveDown(getX(), getY())) //Make sure nothing is under boulder
		{
			moveTo(getX(), getY() - 1);
			if (getGame()->pickUp(getX(), getY(), 3.0))
			{
				donezo();
				getGame()->getPlayer()->takeDamage(getDamage());
			}
			if (getGame()->pickUpProtestor(getX(), getY(), 3.0, this))
			{}
		}
		else
			donezo();
	}

}

void Nugget2::doSomething()
{
	if (!isActive())
		return;

	if (getGame()->pickUpProtestor(getX() + 2, getY() + 2, 3.0, this))
	{
		donezo();
		return;
	}
	
	if (ticklife() >= 100)
	{
		donezo();
		return;
	}
	else
	{
		increasetick();
		return;
	}
}

void WaterPool::doSomething()
{
	if (!isActive())
		return;
	if (getGame()->pickUp(getX() + 2, getY() + 2, 3.0))
	{
		donezo();
		getGame()->getPlayer()->waterRefill();
	}
	if (ticklife() == max(100, 300 - 10 * getGame()->getLevel()))
		donezo();
	else
		increasetick();
}

Barrel::Barrel(int x, int y, StudentWorld* game)
	:PowerUp(IID_BARREL, x, y, right, 2, game)
{
	setVisible(false);
}

void Barrel::doSomething()
{
	if (!isActive())
		return;

	if (getGame()->pickUp(getX() + 2, getY() + 2, 3.0))
	{
		donezo();
		getGame()->playSound(SOUND_FOUND_OIL);
		getGame()->increaseScore(1000);
		getGame()->decreaseBarrels();
		return;
	}
	else if (getGame()->pickUp(getX() + 2, getY() + 2, 4.0))
	{
		setVisible(true);
		return;
	}
	return;
}

void SonarKit::doSomething()
{
	if (!isActive())
		return;
	if (getGame()->pickUp(getX() + 2, getY(), 3.0))
	{
		donezo();
		getGame()->getPlayer()->sonarRefill();
	}
	return;
}

void Nugget1::doSomething()
{
	if (!isActive())
		return;

	if (getGame()->pickUp(getX() + 2, getY() + 2, 3.0))
	{
		donezo();
		getGame()->getPlayer()->goldRefill();
		return;
	}
	else if (getGame()->pickUp(getX() + 2, getY() + 2, 4.0))
	{
		setVisible(true);
		return;
	}
	return;
}

Actor::Actor(int id, int x, int y, Direction dir, int depth, StudentWorld* game, int hp)
	:Base(id, x, y, dir, 1.0, depth)
{
	m_game = game;
	m_hp = hp;
}

void Actor::takeDamage(int dmg)
{
	m_hp -= dmg;
}

Protestor::Protestor(int id, StudentWorld* game, Iceman* player, int hp)
	:Actor(id, 60, 60, left, 0, game, hp)
{
	this->player = player;
	m_state = 0;
	m_yellticks = 15;
	m_turnticks = 200;
	resetRestTicks();
	decideSteps();
}

void Protestor::changeState()
{
	m_state = 1;
	leaving = getGame()->protestorLeaving(this, 60, 60);
	return;
}

void Protestor::decideSteps()
{
	numSquaresToMoveInCurrentDirection = rand() % 53 + 8;
}

void Protestor::resetRestTicks()
{
	ticksToWaitBetweenMoves = max(0, 3 - getGame()->getLevel() / 4);
}

void Protestor::yell()
{
	if (getYellTicks() == 0 || getYellTicks() >= 15)
	{
		getGame()->playSound(SOUND_PROTESTER_YELL);
		getPlayer()->takeDamage(2);
		resetYellTicks();
	}
	return;
}

void Protestor::getNewDirection()
{
	bool moving = false;
	while (moving == false)
	{
		int direction = rand() % 4;
		switch (direction)
		{
		case 0:
			if (getGame()->itemMoveUp(getX(), getY()))
			{
				setDirection(up);
				moving = true;
			}
			break;
		case 1:
			if (getGame()->itemMoveDown(getX(), getY()))
			{
				setDirection(down);
				moving = true;
			}
			break;
		case 2:
			if (getGame()->itemMoveLeft(getX(), getY()))
			{
				setDirection(left);
				moving = true;	
			}
			break;
		case 3:
			if (getGame()->itemMoveRight(getX(), getY()))
			{
				setDirection(right);
				moving = true;
			}
			break;
		}
		decideSteps();
	}
}

void Protestor::oneStepTowardsExit()
{
	Position* move = leaving.top();
	leaving.pop();
	if (move->y > this->getY())
		setDirection(up);
	else if (move->y < this->getY())
		setDirection(down);
	else if (move->x > this->getX())
		setDirection(right);
	else if (move->x < this->getX())
		setDirection(left);
	moveTo(move->x, move->y);
	delete move;
}

void RegularProtestor::doSomething()
{
	if (!isActive())
		return;

	if (ticklife() < getRestTicks()) //Rest
	{
		increasetick();
		return;
	}

	GraphObject::Direction whichway;
	switch (getState())
	{
	case 0: //Moving
		if (distance(getX() + 2, getY() + 2, getPlayer()->getX() + 2, getPlayer()->getY() + 2) <= 4) //In range to yell
		{
			switch (getDirection())
			{
			case up:
				if (getPlayer()->getY() > getY())
					yell();
				break;
			case down:
				if (getPlayer()->getY() < getY())
					yell();
				break;
			case left:
				if (getPlayer()->getX() < getX())
					yell();
				break;
			case right:
				if (getPlayer()->getX() > getX())
					yell();
				break;
			}
			resetSteps();
			resettick();
			increaseYellTicks();
			increaseTurnTicks();
			resetRestTicks();
			return;
		}
		else if (distance(getX() + 2, getY() + 2, getPlayer()->getX() + 2, getPlayer()->getY() + 2) > 4 && getGame()->lineOfSight(whichway, this)) //Has line of sight
		{
			setDirection(whichway);
			switch (whichway)
			{
			case up:
				moveTo(getX(), getY() + 1);
				break;
			case down:
				moveTo(getX(), getY() - 1);
				break;
			case left:
				moveTo(getX() - 1, getY());
				break;
			case right:
				moveTo(getX() + 1, getY());
				break;
			}
			resetSteps();
			resettick();
			increaseYellTicks();
			increaseTurnTicks();
			resetRestTicks();
			return;
		}
		else if (getSteps() <= 0) //6) Steps is at zero
		{
			getNewDirection();
		}
		else if (getGame()->atJunction(getX(), getY()) && getTurnTicks() >= 200) //7) At a junction
		{
			getNewDirection();
			resetTurnTicks();
		}
		switch (getDirection())
		{
		case up:
			if (getGame()->itemMoveUp(getX(), getY()))
			{
				moveTo(getX(), getY() + 1);
				decSteps();
			}
			else
				resetSteps();
			break;
		case down:
			if (getGame()->itemMoveDown(getX(), getY()))
			{
				moveTo(getX(), getY() - 1);
				decSteps();
			}
			else
				resetSteps();
			break;
		case left:
			if (getGame()->itemMoveLeft(getX(), getY()))
			{
				moveTo(getX() - 1, getY());
				decSteps();
			}
			else
				resetSteps();
			break;
		case right:
			if (getGame()->itemMoveRight(getX(), getY()))
			{
				moveTo(getX() + 1, getY());
				decSteps();
			}
			else
				resetSteps();
			break;
		}
		break;
	case 1: //Leaving the oil field
		if (getX() == 60 && getY() == 60)
			donezo();
		else
			oneStepTowardsExit();
		break;
	}
	resettick();
	increaseYellTicks();
	increaseTurnTicks();
	resetRestTicks();
	return;
}

void RegularProtestor::pickUpNugget()
{
	getGame()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getGame()->increaseScore(25);
	setTickLife(getRestTicks());
	changeState();
}

void RegularProtestor::squirtKill()
{
	getGame()->increaseScore(100);
	resetRestTicks();
}

HardcoreProtestor::HardcoreProtestor(StudentWorld* game, Iceman* player)
	:Protestor(IID_HARD_CORE_PROTESTER, game, player, 20)
{
	m_pursuitSteps = 16 + getGame()->getLevel() * 2;
}

void HardcoreProtestor::doSomething()
{
	if (!isActive()) //1)
		return;

	if (ticklife() < getRestTicks()) //2) Rest
	{
		increasetick();
		return;
	}

	GraphObject::Direction whichway;
	if (!getGame()->protestorLeaving(this, getPlayer()->getX(), getPlayer()->getY()).empty())
		pursuit = getGame()->protestorLeaving(this, getPlayer()->getX(), getPlayer()->getY());
	switch (getState())
	{
	case 0: //Moving
		if (distance(getX() + 2, getY() + 2, getPlayer()->getX() + 2, getPlayer()->getY() + 2) <= 4) //4) In range to yell
		{
			switch (getDirection())
			{
			case up:
				if (getPlayer()->getY() > getY())
					yell();
				break;
			case down:
				if (getPlayer()->getY() < getY())
					yell();
				break;
			case left:
				if (getPlayer()->getX() < getX())
					yell();
				break;
			case right:
				if (getPlayer()->getX() > getX())
					yell();
				break;
			}
			resetSteps();
			resettick();
			increaseYellTicks();
			increaseTurnTicks();
			resetRestTicks();
			return;
		}
		else if (pursuit.size() <= m_pursuitSteps) //5) In pursuit
		{
			pursuit.pop();
			Position* move = pursuit.top();
			pursuit.pop();
			if (move->y > this->getY())
				setDirection(up);
			else if (move->y < this->getY())
				setDirection(down);
			else if (move->x > this->getX())
				setDirection(right);
			else if (move->x < this->getX())
				setDirection(left);
			moveTo(move->x, move->y);
			delete move;
			move = nullptr;
			while (!pursuit.empty())
			{
				move = pursuit.top();
				delete move;
				move = nullptr;
				pursuit.pop();
			}
			resettick();
			increaseYellTicks();
			increaseTurnTicks();
			resetRestTicks();
			return;
			
		}
		else if (distance(getX() + 2, getY() + 2, getPlayer()->getX() + 2, getPlayer()->getY() + 2) > 4.0 && getGame()->lineOfSight(whichway, this)) //6) Has line of sight
		{
			setDirection(whichway);
			switch (whichway)
			{
			case up:
				moveTo(getX(), getY() + 1);
				break;
			case down:
				moveTo(getX(), getY() - 1);
				break;
			case left:
				moveTo(getX() - 1, getY());
				break;
			case right:
				moveTo(getX() + 1, getY());
				break;
			}
			resetSteps();
			resettick();
			increaseYellTicks();
			increaseTurnTicks();
			resetRestTicks();
			return;
		}
		else if (getSteps() <= 0) //7) Steps is at zero
		{
			getNewDirection();
		}
		else if (getGame()->atJunction(getX(), getY()) && getTurnTicks() >= 200) //8) At a junction
		{
			getNewDirection();
			resetTurnTicks();
		}
		switch (getDirection()) //9) Take step
		{
		case up:
			if (getGame()->itemMoveUp(getX(), getY()))
			{
				moveTo(getX(), getY() + 1);
				decSteps();
			}
			else
				resetSteps(); //10) Blocked
			break;
		case down:
			if (getGame()->itemMoveDown(getX(), getY()))
			{
				moveTo(getX(), getY() - 1);
				decSteps();
			}
			else
				resetSteps();
			break;
		case left:
			if (getGame()->itemMoveLeft(getX(), getY()))
			{
				moveTo(getX() - 1, getY());
				decSteps();
			}
			else
				resetSteps();
			break;
		case right:
			if (getGame()->itemMoveRight(getX(), getY()))
			{
				moveTo(getX() + 1, getY());
				decSteps();
			}
			else
				resetSteps();
			break;
		}
		break;
	case 1: //Leaving the oil field
		if (getX() == 60 && getY() == 60)
			donezo();
		else
			oneStepTowardsExit();
		break;
	}
	resettick();
	increaseYellTicks();
	increaseTurnTicks();
	resetRestTicks();
	return;
}

void HardcoreProtestor::pickUpNugget()
{
	getGame()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getGame()->increaseScore(50);
	increaseRestTicks(max(50, 100 - getGame()->getLevel() * 10));
}

void HardcoreProtestor::squirtKill()
{
	getGame()->increaseScore(250);
	resetRestTicks();
}

Iceman::Iceman(StudentWorld* game)
	:Actor(IID_PLAYER, 30, 60, right, 0, game, 10)
{
	m_squirts = 5;
	m_sonar = 1;
	m_gold = 0;
}

void Iceman::doSomething()
{
	if(!isActive())
		return;
	if(getY() < 60)
		getGame()->dig(this); //Checks for Ice, removes if overlapping
	int ch;
	if(getGame()->getKey(ch)) //Add controls here
	{
		switch (ch)
		{
		case KEY_PRESS_UP:
			if (getDirection() == up && getY() == 60) //if at the top
				moveTo(getX(), getY());
			else if (getDirection() == up && getGame()->isBoulder(getX(), getY() + 1, 3.0))
				this->setDirection(up);
			else if (getDirection() == up) //facing up, not at the top
				moveTo(getX(), getY() + 1);
			else
				this->setDirection(up); //turn to face up
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() == down && getY() == 0)
				moveTo(getX(), getY());
			else if (getDirection() == down && getGame()->isBoulder(getX(), getY() - 1, 3.0))
				this->setDirection(down);
			else if(getDirection() == down)
				moveTo(getX(), getY() - 1);
			else
				this->setDirection(down);
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() == left && getX() == 0)
				moveTo(getX(), getY());
			else if (getDirection() == left && getGame()->isBoulder(getX() - 1, getY(), 3.0))
				this->setDirection(left);
			else if(getDirection() == left)
				moveTo(getX() - 1, getY());
			else
				this->setDirection(left);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() == right && getX() == 60)
				moveTo(getX(), getY());
			else if (getDirection() == right && getGame()->isBoulder(getX() + 1, getY(), 3.0))
				this->setDirection(right);
			else if(getDirection() == right)
				moveTo(getX() + 1, getY());
			else
				this->setDirection(right);
			break;
		case KEY_PRESS_SPACE:
			if (m_squirts > 0)
			{
				getGame()->shoot(this);
				m_squirts--;
			}
			break;
		case KEY_PRESS_TAB:
			if (m_gold > 0)
			{
				getGame()->dropGold(this);
				m_gold--;
			}
			break;
		case KEY_PRESS_ESCAPE:
			donezo();
			break;
		case 'Z':
		case 'z':
			if (m_sonar > 0)
			{
				getGame()->sonar(this);
				m_sonar--;
			}
			break;
		}
	}
}

void Iceman::waterRefill()
{
	getGame()->playSound(SOUND_GOT_GOODIE);
	m_squirts += 5;
	getGame()->increaseScore(100);
}

void Iceman::sonarRefill()
{
	getGame()->playSound(SOUND_GOT_GOODIE);
	m_sonar++;
	getGame()->increaseScore(75);
}

void Iceman::goldRefill()
{
	getGame()->playSound(SOUND_GOT_GOODIE);
	m_gold++;
	getGame()->increaseScore(10);
}