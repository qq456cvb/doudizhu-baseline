#include "player.h"
#include <algorithm>
#include "assert.h"
#include <iostream>
#include <sstream>
#include "env.h"
#include "mctree.h"

extern vector<CardGroup> all_actions;
int n_threads = 10, max_d = 50, max_iter = 250;

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

Player::Player()
{
    this->reset();
}

Player::Player(CEnv *env)
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
	_avail_actions.clear();
	fill(_cnts.begin(), _cnts.end(), 0);
	for (const auto &c : _handcards)
	{
		_cnts[static_cast<int>(c)]++;
	}
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

void MCPlayer::multisearch(vector<int> &cnts, State *root) {
	MCTree tree(root, sqrtf(2.f));
	tree.search(n_threads / 2, max_iter);
	vector<int> tree_cnts = tree.predict();
	{
		lock_guard<mutex> lock(mu);
		for (size_t i = 0; i < tree.root->edges.size(); i++)
		{
			cnts[i] += tree_cnts[i];
		}
	}
}

CardGroup MCPlayer::respond(const CardGroup &last_card) {
	State *s = new State(*this->_env);
	auto action_space = s->get_action_space();
	vector<int> total_cnts(action_space.size(), 0);

	// random shuffle other player's card, determinized UCT
	int idx1 = (_env->_current_idx + 1) % 3, idx2 = (_env->_current_idx + 2) % 3;
	vector<Card> unseen_cards = _env->_players[idx1]->_handcards;
	unseen_cards.insert(unseen_cards.end(), _env->_players[idx2]->_handcards.begin(), _env->_players[idx2]->_handcards.end());
	// if (_env->_current_idx != 0) {
	// 	for (auto c : _env->_extra_cards) {
	// 		auto it = find(unseen_cards.begin(), unseen_cards.end(), c);
	// 		assert(it != unseen_cards.end());
	// 		unseen_cards.erase(it);
	// 	}
	// }
	vector<thread> threads;
	for (size_t d = 0; d < max_d; d++)
	{
		State *ss = new State(*s);
		shuffle(unseen_cards.begin(), unseen_cards.end(), _env->_generator);
		ss->_players[idx1]->_handcards = vector<Card>(unseen_cards.begin(), unseen_cards.begin() + _env->_players[idx1]->_handcards.size());
		// if (idx1 == 0) {
		// 	ss->_players[idx1]->_handcards.insert(ss->_players[idx1]->_handcards.end(), _env->_extra_cards.begin(), _env->_extra_cards.end());
		// }
		sort(ss->_players[idx1]->_handcards.begin(), ss->_players[idx1]->_handcards.end());
		ss->_players[idx1]->calc_avail_actions();
		ss->_players[idx2]->_handcards = vector<Card>(unseen_cards.begin() + _env->_players[idx1]->_handcards.size(), unseen_cards.end());
		// if (idx2 == 0) {
		// 	ss->_players[idx2]->_handcards.insert(ss->_players[idx2]->_handcards.end(), _env->_extra_cards.begin(), _env->_extra_cards.end());
		// }
		sort(ss->_players[idx2]->_handcards.begin(), ss->_players[idx2]->_handcards.end());
		ss->_players[idx2]->calc_avail_actions();

		// parallel determinization (parallel MCT)
		/*threads.push_back(std::move(std::thread(&MCPlayer::multisearch, this, std::ref(total_cnts), ss)));
		if ((d + 1) % 2 == 0 || d == max_d - 1)
		{
			for (auto& t : threads) {
				t.join();
			}
			threads.clear();
		}*/
		
		// sequential determinization (parallel MCT)
		MCTree tree(ss, sqrtf(2.f));
		tree.search(n_threads, max_iter);
		vector<int> cnts = tree.predict();
		for (size_t i = 0; i < action_space.size(); i++)
		{
			total_cnts[i] += cnts[i];
		}
	}

	

	/*MCTree tree(s, sqrtf(2.f));
	tree.search(8, 2500);
	total_cnts = tree.predict();*/
	
	auto cand = *action_space[max_element(total_cnts.begin(), total_cnts.end()) - total_cnts.begin()];
	remove_cards(cand._cards);
	delete s;
	return cand;
}

ostream& operator <<(ostream& os, const Player& c) {
	for (auto card : c._handcards)
	{
		os << card << ", ";
	}
	return os;
}

string Player::to_str() {
	ostringstream ss;
	ss << *this;
	return ss.str();
}
