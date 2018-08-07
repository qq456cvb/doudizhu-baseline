#include "env.h"
#include <iostream>

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

	for (int i = 0; i < 11; i++)
	{
		_players[0]->add_card(_cards[i]);
	}
	for (int i = 11; i < 18; i++) {
		_players[1]->add_card(_cards[i]);
	}
#ifdef DEBUG
	cout << *_players[0] << endl << *_players[1] << endl;
#endif
	_current_idx = 0;
	_current_controller = 0;
	_last_group = CardGroup({}, Category::EMPTY, 0);
}

bool Env::step(int &winner) {
	CardGroup respondence = _players[_current_idx]->respond(_last_group);
	auto next_idx = (_current_idx + 1) % 2;

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