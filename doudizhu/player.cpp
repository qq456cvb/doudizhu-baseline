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

	this->_cnts[static_cast<int>(card)]--;
	vector<CardGroup>::iterator it2 = _avail_actions.begin();
	while (it2 != _avail_actions.end())
	{
		// TODO: test if there is enough memory to put cnt into CardGroup directly
		int cnt[15] = { 0 };
		for (const auto &c : it2->_cards) {
			cnt[static_cast<int>(c)]++;
		}
		if (_cnts[static_cast<int>(card)] < cnt[static_cast<int>(card)])
		{
			it2 = _avail_actions.erase(it2);
		} else {
			it2++;
		}
	}
}

void Player::remove_cards(vector<Card> cards) {
	vector<Card>::iterator it;
	int a = this->_handcards.size();
	for (auto &card : cards) {
		for (it = _handcards.begin(); it < _handcards.end(); it++)
		{
			if (card == *it)
			{
				this->_cnts[static_cast<int>(card)]--;
				_handcards.erase(it);
				break;
			}
		}
	}
	//auto pr = std::equal_range(this->_handcards.begin(), this->_handcards.end(), card);
	assert(a - this->_handcards.size() == cards.size());
	
	vector<CardGroup>::iterator it2 = _avail_actions.begin();
	while (it2 != _avail_actions.end())
	{
		// TODO: test if there is enough memory to put cnt into CardGroup directly
		int cnt[15] = { 0 };
		for (const auto &c : it2->_cards) {
			cnt[static_cast<int>(c)]++;
		}
		bool found = false;
		for (auto &card : cards) {
			if (_cnts[static_cast<int>(card)] < cnt[static_cast<int>(card)])
			{
				it2 = _avail_actions.erase(it2);
				found = true;
				break;
			}
		}
		if (!found)
		{
			it2++;
		}
	}
}

void Player::calc_avail_actions() {
	for (const auto &action : all_actions) {
		if (includes(_handcards.begin(), _handcards.end(), action._cards.begin(), action._cards.end()))
		{
			_avail_actions.push_back(action);
		}
	}
}

bool Player::over() {
	return this->_handcards.empty();
}

vector<CardGroup> Player::candidate(const CardGroup &last_card) {
	if (last_card._category == Category::EMPTY)
	{
		return _avail_actions;
	}
	else {
		vector<CardGroup> results;
		results.push_back(CardGroup({}, Category::EMPTY, 0));
		for (const auto &cg : _avail_actions)
		{
			if (cg > last_card)
			{
				results.push_back(cg);
			}
		}
		return results;
	}
}

CardGroup Player::respond(const CardGroup &last_card) {
	return CardGroup({}, Category::EMPTY, 0);
}

CardGroup RandomPlayer::respond(const CardGroup &last_card) {
	auto cands = candidate(last_card);
	//cout << cands.size() << endl;
	auto cand = cands[rand() % cands.size()];
	remove_cards(cand._cards);
	return cand;
}

CardGroup MinimaxPlayer::respond(const CardGroup &last_card) {
	State s(*this->_env);
	auto action_space = s.get_action_space();
	int best_idx = 0;
	float alpha = -100.f, beta = 100.f;
	minimax(s, best_idx, alpha, beta);
	auto max_action = action_space[best_idx];
	for (auto c : max_action._cards)
	{
		this->remove_card(c);
	}
	return max_action;
}

ostream& operator <<(ostream& os, const Player& c) {
	for (auto c : c._handcards)
	{
		os << c << ", ";
	}
	return os;
}

string Player::to_str() {
	ostringstream ss;
	ss << *this;
	return ss.str();
}
