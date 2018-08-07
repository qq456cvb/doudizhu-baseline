#include "player.h"
#include <algorithm>
#include "assert.h"
#include <iostream>
#include <sstream>
#include "mctree.h"

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
		auto cards = action._cards;
		sort(cards.begin(), cards.end());
		if (includes(_handcards.begin(), _handcards.end(), cards.begin(), cards.end()))
		{
			_avail_actions.push_back(action);
		}
	}
}

bool Player::over() {
	return this->_handcards.empty();
}

vector<vector<CardGroup>::iterator> Player::candidate(const CardGroup &last_card) {
	vector<vector<CardGroup>::iterator> its;
	if (last_card._category == Category::EMPTY)
	{
		its.reserve(_avail_actions.size() - 1);
		for (auto it = _avail_actions.begin() + 1; it != _avail_actions.end(); it++) {
			its.push_back(it);
		}
		return its;
	}
	else {
		for (auto it = _avail_actions.begin(); it != _avail_actions.end(); it++) {
			if (*it > last_card)
			{
				its.push_back(it);
			}
		}
		return its;
	}
}

const vector<CardGroup> &Player::get_avail_actions() const {
	return _avail_actions;
}

CardGroup Player::respond(const CardGroup &last_card) {
	return CardGroup({}, Category::EMPTY, 0);
}

CardGroup RandomPlayer::respond(const CardGroup &last_card) {
	auto cands = candidate(last_card);
	//cout << cands.size() << endl;
	auto cand = cands[rand() % cands.size()];
	remove_cards(cand->_cards);
	return *cand;
}

CardGroup MinimaxPlayer::respond(const CardGroup &last_card) {
	/*State s(*this->_env);
	auto action_space = s.get_action_space();
	int best_idx = 0;
	float alpha = -100.f, beta = 100.f;
	minimax(s, best_idx, alpha, beta);
	auto max_action = action_space[best_idx];
	remove_cards(max_action->_cards);
	return *max_action;*/
	auto cands = candidate(last_card);
	//cout << cands.size() << endl;
	auto cand = cands[rand() % cands.size()];
	remove_cards(cand->_cards);
	return *cand;
}

CardGroup MCPlayer::respond(const CardGroup &last_card) {
	State *s = new State(*this->_env);
	auto action_space = s->get_action_space();
	MCTree tree(s, sqrtf(2.f));
	tree.search(8, 100);
	vector<int> cnts = tree.predict();
	auto cand = *action_space[max_element(cnts.begin(), cnts.end()) - cnts.begin()];
	remove_cards(cand._cards);
	return cand;
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
