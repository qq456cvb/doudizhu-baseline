#pragma once
#include <vector>
#include "card.h"
using namespace std;

class Env;
class Player
{
public:
	Player(Env *env);
	Player(const Player &);
	~Player();

	void reset();
	void add_card(Card card);
	void remove_card(Card card);
	bool over();
	vector<CardGroup> candidate(const CardGroup &last_card);
	virtual CardGroup respond(const CardGroup &last_card);
	string to_str();

	friend ostream& operator <<(ostream& os, const Player& c);

	vector<int> _cnts;
private:
	vector<Card> _handcards;
	vector<CardGroup> _avail_actions;
protected:
	Env *_env;
	
};

class RandomPlayer : public Player
{
public:
	RandomPlayer(Env *env) : Player(env) {};
	CardGroup respond(const CardGroup &last_card) override;
protected:
private:
};

class MinimaxPlayer : public Player
{
public:
	MinimaxPlayer(Env *env) : Player(env) {};
	CardGroup respond(const CardGroup &last_card) override;
};