#pragma once
#include <vector>
#include "card.h"
#include "player.h"
#include <random>
#include <iostream>
using namespace std;

// #define DEBUG

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
		auto seed = random_device{}();
		cout << "seeding " << seed << endl;
		this->_generator = mt19937(seed);
		//this->_players.push_back(new MCPlayer(this));
		for (int i = 0; i < 1; i++) {
			//this->_players.push_back(new RandomPlayer(this));
			this->_players.push_back(new MCPlayer(this));
		}
		for (int i = 0; i < 2; i++) {
			this->_players.push_back(new MCPlayer(this));
		}
	}

	~Env() {
		for (auto player : _players)
		{
			delete player;
		}
	}
};

