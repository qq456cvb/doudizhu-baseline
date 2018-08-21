#pragma once
#include <vector>
#include "card.h"
#include "player.h"
#include <random>
#include <iostream>
using namespace std;

// #define DEBUG

class CEnv {
public:
	static vector<Card> _init_cards;
	vector<Card> _cards;
	vector<Card> _extra_cards;
	vector<Player*> _players;
	int _current_idx, _current_controller;
	CardGroup _last_group;
	std::mt19937 _generator;


	void reset();
	tuple<int, bool, CardGroup> step_auto();
	tuple<int, bool, CardGroup> step_manual(CardGroup cg);
	vector<int> get_current_handcards();
	vector<int> get_last_outcards();
	int get_role_ID();
	vector<float> get_state_prob();

	CEnv() : _last_group({}, Category::EMPTY, 0) {
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

	~CEnv() {
		for (auto player : _players)
		{
			delete player;
		}
	}
};

