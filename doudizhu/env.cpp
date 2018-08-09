#include "env.h"
#include <iostream>
#include <algorithm>

vector<Card> Env::_init_cards = {
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

void Env::reset() {
	this->_cards = _init_cards;
	for (auto player : _players)
	{
		player->reset();
	}
	shuffle(_cards.begin(), _cards.end(), _generator);

	for (int i = 0; i < 20; i++)
	{
		_players[0]->add_card(_cards[i]);
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

bool Env::step(int &winner) {
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
			winner = _current_idx;
			return true;
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
	return false;
}
