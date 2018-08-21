#include "env.h"
#include <iostream>
#include <algorithm>
#include <assert.h>

vector<Card> CEnv::_init_cards = {
	Card::THREE,
	Card::THREE,
	Card::THREE,
	Card::THREE,
	Card::FOUR,
	Card::FOUR,
	Card::FOUR,
	Card::FOUR,
	Card::FIVE,
	Card::FIVE,
	Card::FIVE,
	Card::FIVE,
	Card::SIX,
	Card::SIX,
	Card::SIX,
	Card::SIX,
	Card::SEVEN,
	Card::SEVEN,
	Card::SEVEN,
	Card::SEVEN,
	Card::EIGHT,
	Card::EIGHT,
	Card::EIGHT,
	Card::EIGHT,
	Card::NINE,
	Card::NINE,
	Card::NINE,
	Card::NINE,
	Card::TEN,
	Card::TEN,
	Card::TEN,
	Card::TEN,
	Card::JACK,
	Card::JACK,
	Card::JACK,
	Card::JACK,
	Card::QUEEN,
	Card::QUEEN,
	Card::QUEEN,
	Card::QUEEN,
	Card::KING,
	Card::KING,
	Card::KING,
	Card::KING,
	Card::ACE,
	Card::ACE,
	Card::ACE,
	Card::ACE,
	Card::TWO,
	Card::TWO,
	Card::TWO,
	Card::TWO,
	Card::BLACK_JOKER,
	Card::RED_JOKER
};

void CEnv::reset() {
	this->_cards = _init_cards;
	this->_extra_cards.clear();
	for (auto player : _players)
	{
		player->reset();
	}
	shuffle(_cards.begin(), _cards.end(), _generator);

	for (int i = 0; i < 20; i++)
	{
		_players[0]->add_card(_cards[i]);
	}
	for (int i = 17; i < 20; i++) {
		_extra_cards.push_back(_cards[i]);
	}
	for (int i = 20; i < 37; i++) {
		_players[1]->add_card(_cards[i]);
	}
	for (int i = 37; i < 54; i++) {
		_players[2]->add_card(_cards[i]);
	}
	for (int i = 0; i < 3; i++)
	{
		_players[i]->calc_avail_actions();
	}
	/*cout << *_players[0] << endl;
	for (auto cg : _players[0]->_avail_actions)
	{
		cout << cg << endl;
	}
	cout << "available actions: " << _players[0]->_avail_actions.size() << endl;*/
#ifdef DEBUG
	cout << *_players[0] << endl << *_players[1] << endl << *_players[2] << endl;
#endif
	_current_idx = 0;
	_current_controller = 0;
	_last_group = CardGroup({}, Category::EMPTY, 0);
}

tuple<int, bool, CardGroup> CEnv::step_auto() {
	CardGroup respondence = _players[_current_idx]->respond(_last_group);
	auto next_idx = (_current_idx + 1) % 3;

	if (respondence._category != Category::EMPTY)
	{
#ifdef DEBUG
		cout << "player: " << _current_idx << " gives " << respondence << endl;
#endif
		_current_controller = _current_idx;
		_last_group = respondence;
		if (_players[_current_idx]->over())
		{
			return make_tuple(_current_idx, true, respondence);
		}
		
	} else {
#ifdef DEBUG
		cout << "player: " << _current_idx << " pass" << endl;
#endif
	}
	if (next_idx == _current_controller)
	{
		_last_group = CardGroup({}, Category::EMPTY, 0);
	}
	_current_idx = next_idx;
	//cout << "stepped" << endl;
	return make_tuple(_current_idx, false, respondence);
}

tuple<int, bool, CardGroup> CEnv::step_manual(CardGroup cg) {
	_players[_current_idx]->remove_cards(cg._cards);
	auto next_idx = (_current_idx + 1) % 3;

	if (cg._category != Category::EMPTY)
	{
#ifdef DEBUG
		cout << "player: " << _current_idx << " gives " << respondence << endl;
#endif
		_current_controller = _current_idx;
		_last_group = cg;
		if (_players[_current_idx]->over())
		{
			return make_tuple(_current_idx, true, cg);
		}
		
	} else {
#ifdef DEBUG
		cout << "player: " << _current_idx << " pass" << endl;
#endif
	}
	if (next_idx == _current_controller)
	{
		_last_group = CardGroup({}, Category::EMPTY, 0);
	}
	_current_idx = next_idx;
	//cout << "stepped" << endl;
	return make_tuple(_current_idx, false, cg);
}

vector<int> to_value(const vector<Card> &cards) {
	vector<int> results(cards.size(), 0);
	for (size_t i = 0; i < cards.size(); i++) {
		results[i] = static_cast<int>(cards[i]);
	}
	return results;
}

vector<int> CEnv::get_current_handcards() {
	return to_value(_players[_current_idx]->_handcards);
}

vector<int> CEnv::get_last_outcards() {
	return to_value(_last_group._cards);
}

int CEnv::get_role_ID() {
	switch (_current_idx) {
		case 0:
			return 2;
		case 1:
			return 3;
		case 2:
			return 1;
		default:
			return -1;
	}
}

std::vector<int> toOneHot60(const std::vector<Card> &v) {
	std::vector<int> result(60, 0);
	for (auto c : v) {
		int unordered_color = static_cast<int>(c) * 4;
		while (result[unordered_color++] > 0);
		result[unordered_color - 1]++;
	}
	return result;
    }

vector<float> CEnv::get_state_prob() {
	assert(_current_idx == 0 && "get state prob must be called for lord");
	vector<Card> tmp = _players[1]->_handcards;
	tmp.insert(tmp.end(), _players[2]->_handcards.begin(), _players[2]->_handcards.end());
	vector<int> remains = toOneHot60(tmp);
	vector<float> prob1(remains.begin(), remains.end());
    vector<float> prob2(remains.begin(), remains.end());

	auto size1 = _players[1]->_handcards.size();
	auto size2 = _players[2]->_handcards.size();
	for (int i = 0; i < remains.size(); i++) {
		prob1[i] *= float(size1) / (size1 + size2);
		prob2[i] *= float(size2) / (size1 + size2);
	}
	prob1.insert(prob1.end(), prob2.begin(), prob2.end());
	return prob1;
}

