#include "minimax.h"
#include <stdlib.h>
#include "env.h"
#include <unordered_map>

static unordered_map<State, float> vmap;

State::State(const State &s) {
	_last_group = s._last_group;
	_current_idx = s._current_idx;
	_current_controller = s._current_controller;
	_target_idx = s._target_idx;
	_is_max = s._is_max;
	_winner = s._winner;
	_id = s._id;
	//_cards = s._cards;
	for (int i = 0; i < s._players.size(); i++)
	{
		_players.push_back(new Player(*s._players[i]));
		//_players.push_back(new Player(*s._players[i]));
	}
}

State::State(const Env &env) {
	_last_group = env._last_group;
	_current_idx = env._current_idx;
	_current_controller = env._current_controller;
	_winner = -1;
	_id = StateId::NORMAL;
	_target_idx = _current_idx;
	_is_max = true;
	for (int i = 0; i < env._players.size(); i++)
	{
		_players.push_back(new Player(*env._players[i]));
	}
}

State::~State() {
	for (auto player : _players)
	{
		delete player;
	}
}

vector<vector<CardGroup>::iterator> State::get_action_space() const {
	return _players[_current_idx]->candidate(_last_group);
}

State *step(const State &s, const CardGroup &cg) {
	State *sprime = new State(s);
	CardGroup	respondence = cg;
	sprime->_players[sprime->_current_idx]->remove_cards(cg._cards);

	auto next_idx = (sprime->_current_idx + 1) % 2;
	if (respondence._category != Category::EMPTY)
	{
		sprime->_current_controller = sprime->_current_idx;
		sprime->_last_group = respondence;
		if (sprime->_players[sprime->_current_idx]->over())
		{
			sprime->_winner = sprime->_current_idx;
			sprime->_id = StateId::FINISHED;
			return sprime;
		}
	}
	if (next_idx == sprime->_current_controller)
	{
		sprime->_last_group = CardGroup({}, Category::EMPTY, 0);
	}
	sprime->_current_idx = next_idx;
	//cout << "stepped" << endl;
	return sprime;
}

void step_ref(State &s, const CardGroup &cg) {
	CardGroup	respondence = cg;
	s._players[s._current_idx]->remove_cards(cg._cards);

	s._is_max = !s._is_max;
	auto next_idx = (s._current_idx + 1) % 2;
	if (respondence._category != Category::EMPTY)
	{
		s._current_controller = s._current_idx;
		s._last_group = respondence;
		if (s._players[s._current_idx]->over())
		{
			s._winner = s._current_idx;
			s._id = StateId::FINISHED;
			return;
		}
	}
	if (next_idx == s._current_controller)
	{
		s._last_group = CardGroup({}, Category::EMPTY, 0);
	}
	s._current_idx = next_idx;
	//cout << "stepped" << endl;
	return;
}


float minimax(State &s, int &best_idx, float alpha, float beta) {
	if (vmap.find(s) != vmap.end())
	{
		return vmap[s];
	}
	if (s._id == StateId::FINISHED) {
		if (s._winner == s._target_idx)
		{
			return 1.f;
		} else {
			return 0.f;
		}
	}
	auto action_space = s.get_action_space();
	float result = -100.f;
	if (!s._is_max)
	{
		result = 100.f;
	}
	int tmp = 0;
	for (int i = 0; i < action_space.size(); i++)
	{
		auto &a = *action_space[i];
		auto last_group = s._last_group;
		auto current_idx = s._current_idx;
		auto current_controller = s._current_controller;
		auto winner = s._winner;
		auto id = s._id;
		auto is_max = s._is_max;
		step_ref(s, a);
		
		float sc = minimax(s, best_idx, alpha, beta);
		if (is_max)
		{
			if (sc > result)
			{
				result = sc;
				tmp = i;
			}
			if (sc >= beta)
			{
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
				//vmap.insert({ s, sc });
				return sc;
			}
			alpha = max(alpha, sc);
		} else {
			if (sc < result)
			{
				result = sc;
				tmp = i;
			}
			if (sc <= alpha)
			{
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
				//vmap.insert({ s, sc });
				return sc;
			}
			beta = min(beta, sc);
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
	}
	best_idx = tmp;
	vmap.insert({ s, result });
	return result;
}