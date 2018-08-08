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
	void remove_cards(vector<Card> cards);
	void calc_avail_actions();
	bool over();
	vector<vector<CardGroup>::iterator> candidate(const CardGroup &last_card);
	const vector<CardGroup> &get_avail_actions() const;
	virtual CardGroup respond(const CardGroup &last_card);
	string to_str();

	friend ostream& operator <<(ostream& os, const Player& c);

	vector<int> _cnts;
	vector<CardGroup> _avail_actions;
	vector<Card> _handcards;
private:
	
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

class MCPlayer : public Player
{
public:
	MCPlayer(Env *env) : Player(env) {};
	CardGroup respond(const CardGroup &last_card) override;

private:

};