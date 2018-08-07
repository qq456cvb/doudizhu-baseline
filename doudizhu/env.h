#pragma once
#include <vector>
#include "card.h"
#include "player.h"
#include <random>
using namespace std;

//#define DEBUG

class Env {
public:
	static vector<Card> _init_cards;
	vector<Card> _cards;
	vector<Player*> _players;
	int _current_idx, _current_controller;
	CardGroup _last_group;
	std::mt19937 _generator;


	void reset();
	bool step(int &winner);

	Env() : _last_group({}, Category::EMPTY, 0) {
		this->_generator = mt19937(random_device{}());
		for (int i = 0; i < 1; i++) {
			this->_players.push_back(new MinimaxPlayer(this));
		}
		for (int i = 0; i < 1; i++) {
			this->_players.push_back(new MinimaxPlayer(this));
		}
	}

	~Env() {
		for (auto player : _players)
		{
			delete player;
		}
	}
};

