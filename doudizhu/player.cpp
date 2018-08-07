#include "player.h"
#include <algorithm>
#include "assert.h"
#include <iostream>
#include <sstream>
#include "minimax.h"

extern vector<CardGroup> all_actions;

template< typename T, typename Pred >
typename std::vector<T>::iterator
insert_sorted(std::vector<T> & vec, T const& item, Pred pred)
{
	return vec.insert
	(
		std::upper_bound(vec.begin(), vec.end(), item, pred),
		item
	);
}

Player::Player(Env *env)
{
	this->reset();
	this->_env = env;
}

Player::Player(const Player &player) {
	this->_handcards = player._handcards;
	this->_avail_actions = player._avail_actions;
	this->_cnts = player._cnts;
}

Player::~Player()
{
}


void Player::reset() {
	this->_handcards.clear();
	this->_avail_actions.clear();
	this->_cnts.clear();
	this->_cnts.resize(15, 0);
}

void Player::add_card(Card card) {
	insert_sorted(this->_handcards, card, [](const Card &c1, const Card &c2) {
		return static_cast<int>(c1) < static_cast<int>(c2);
	});
	this->_cnts[static_cast<int>(card)]++;
	if (this->_cnts[static_cast<int>(card)] == 1)
	{
		this->_avail_actions.push_back(CardGroup({ card }, Category::SINGLE, static_cast<int>(card)));
		if ((card == Card::BLACK_JOKER || card == Card::RED_JOKER) && _cnts[13] + _cnts[14] == 2)
		{
			this->_avail_actions.push_back(CardGroup({ Card(13), Card(14) }, Category::BIGBANG, 100));
		}
	}
	else if (this->_cnts[static_cast<int>(card)] == 2)
	{
		this->_avail_actions.push_back(CardGroup({ card, card }, Category::DOUBLE, static_cast<int>(card)));
	}
	else if (this->_cnts[static_cast<int>(card)] == 3)
	{
		this->_avail_actions.push_back(CardGroup({ card, card, card }, Category::TRIPLE, static_cast<int>(card)));
	}
	else if (this->_cnts[static_cast<int>(card)] == 4)
	{
		this->_avail_actions.push_back(CardGroup({ card, card, card, card }, Category::QUADRIC, static_cast<int>(card)));
	}
}

void Player::remove_card(Card card) {
	vector<Card>::iterator it;
	for (it = _handcards.begin(); it < _handcards.end(); it++)
	{
		if (card == *it)
		{
			break;
		}
	}
	//auto pr = std::equal_range(this->_handcards.begin(), this->_handcards.end(), card);
	assert(it < this->_handcards.end());
	
	int a = this->_handcards.size();
	this->_handcards.erase(it);
	assert(a - this->_handcards.size() == 1);
	if (this->_cnts[static_cast<int>(card)] == 1)
	{
		this->_avail_actions.erase(remove(this->_avail_actions.begin(), this->_avail_actions.end(), CardGroup({ card }, Category::SINGLE, static_cast<int>(card))), this->_avail_actions.end());
		if ((card == Card::BLACK_JOKER || card == Card::RED_JOKER) && _cnts[13] + _cnts[14] == 2)
		{
			this->_avail_actions.erase(remove(this->_avail_actions.begin(), this->_avail_actions.end(), CardGroup({ Card(13), Card(14) }, Category::BIGBANG, 100)), this->_avail_actions.end());
		}
	}
	else if (this->_cnts[static_cast<int>(card)] == 2)
	{
		this->_avail_actions.erase(remove(this->_avail_actions.begin(), this->_avail_actions.end(), CardGroup({ card, card }, Category::DOUBLE, static_cast<int>(card))), this->_avail_actions.end());
	}
	else if (this->_cnts[static_cast<int>(card)] == 3)
	{
		this->_avail_actions.erase(remove(this->_avail_actions.begin(), this->_avail_actions.end(), CardGroup({ card, card, card }, Category::TRIPLE, static_cast<int>(card))), this->_avail_actions.end());
	}
	else if (this->_cnts[static_cast<int>(card)] == 4)
	{
		this->_avail_actions.erase(remove(this->_avail_actions.begin(), this->_avail_actions.end(), CardGroup({ card, card, card, card }, Category::QUADRIC, static_cast<int>(card))), this->_avail_actions.end());
	}
	this->_cnts[static_cast<int>(card)]--;
}

bool Player::over() {
	return this->_handcards.empty();
}

vector<CardGroup> Player::candidate(const CardGroup &last_card) {
	/*vector<CardGroup> results;
	results.reserve(16);*/
	if (last_card._category == Category::EMPTY)
	{
		return _avail_actions;
		/*for (int i = 0; i < 4; i++) {
			if (_cnts[i] >= 1)
			{
				results.push_back(CardGroup({ Card(i) }, Category::SINGLE, i));
			}
			if (_cnts[i] >= 2)
			{
				results.push_back(CardGroup({ Card(i), Card(i) }, Category::DOUBLE, i));
			}
			if (_cnts[i] >= 3)
			{
				results.push_back(CardGroup({ Card(i), Card(i), Card(i) }, Category::TRIPLE, i));
			}
			if (_cnts[i] >= 4)
			{
				results.push_back(CardGroup({ Card(i), Card(i), Card(i) }, Category::QUADRIC, i));
			}
		}
		if (_cnts[13] >= 1)
		{
			results.push_back(CardGroup({ Card(13) }, Category::SINGLE, 13));
			if (_cnts[14] >= 1)
			{
				results.push_back(CardGroup({ Card(14) }, Category::SINGLE, 14));
				results.push_back(CardGroup({ Card(13), Card(14) }, Category::BIGBANG, 100));
			}
		}
		else if (_cnts[14] >= 1)
		{
			results.push_back(CardGroup({ Card(14) }, Category::SINGLE, 14));
		}*/
	}
	else {
		vector<CardGroup> results;
		results.push_back(CardGroup({}, Category::EMPTY, 0));
		for (const auto &cg : _avail_actions)
		{
			if ((cg._category == last_card._category && cg._rank > last_card._rank) || cg._category == Category::BIGBANG)
			{
				results.push_back(cg);
			}
		}
		/*if (last_card._category == Category::SINGLE)
		{
			for (int i = last_card._rank + 1; i < 15; i++)
			{
				if (_cnts[i] >= 1) results.push_back(CardGroup({ Card(i) }, Category::SINGLE, i));
			}
		}
		else if (last_card._category == Category::DOUBLE)
		{
			for (int i = last_card._rank + 1; i < 15; i++)
			{
				if (_cnts[i] >= 2) results.push_back(CardGroup({ Card(i), Card(i) }, Category::DOUBLE, i));
			}
		}
		else if (last_card._category == Category::TRIPLE)
		{
			for (int i = last_card._rank + 1; i < 15; i++)
			{
				if (_cnts[i] >= 3) results.push_back(CardGroup({ Card(i), Card(i), Card(i) }, Category::TRIPLE, i));
			}
		}
		else if (last_card._category == Category::QUADRIC)
		{
			for (int i = last_card._rank + 1; i < 15; i++)
			{
				if (_cnts[i] >= 4) results.push_back(CardGroup({ Card(i), Card(i), Card(i), Card(i) }, Category::QUADRIC, i));
			}
		}*/
		return results;
	}
	
	/*for (auto &action : all_actions)
	{
		if (includes(this->_handcards.begin(), this->_handcards.end(), action._cards.begin(), action._cards.end()) &&
			action > last_card) {
			results.push_back(action);
		}
	}*/
	//return results;
}

CardGroup Player::respond(const CardGroup &last_card) {
	return CardGroup({}, Category::EMPTY, 0);
}

CardGroup RandomPlayer::respond(const CardGroup &last_card) {
	auto cands = candidate(last_card);
	//cout << cands.size() << endl;
	auto cand = cands[rand() % cands.size()];
	for (auto c : cand._cards)
	{
		this->remove_card(c);
	}
	return cand;
}

CardGroup MinimaxPlayer::respond(const CardGroup &last_card) {
	State s(*this->_env);
	auto action_space = s.get_action_space();
	int best_idx = 0;
	float alpha = -100.f, beta = 100.f;
	minimax(s, best_idx, alpha, beta);
	auto max_action = action_space[best_idx];
	/*float max_score = -100.f;
	CardGroup max_action;
	for (auto &a : action_space)
	{
		auto last_group = s._last_group;
		auto current_idx = s._current_idx;
		auto current_controller = s._current_controller;
		auto winner = s._winner;
		auto id = s._id;
		auto is_max = s._is_max;
		step_ref(s, a);
		float sc = score(s);
		if (sc > max_score)
		{
			max_action = a;
			max_score = sc;
		}
		for (auto c : a._cards)
		{
			s._players[current_idx]->add_card(c);
		}
		s._last_group = last_group;
		s._current_idx = current_idx;
		s._current_controller = current_controller;
		s._winner = winner;
		s._id = id;
		s._is_max = is_max;
	}*/
	for (auto c : max_action._cards)
	{
		this->remove_card(c);
	}
	return max_action;
}

ostream& operator <<(ostream& os, const Player& c) {
	for (auto c : c._handcards)
	{
		os << c;
	}
	return os;
}

string Player::to_str() {
	ostringstream ss;
	ss << *this;
	return ss.str();
}
