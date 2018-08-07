#pragma once

#include "player.h"
#include "card.h"
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <xhash>

enum class StateId {
	NORMAL,
	FINISHED
};

class Env;

template<typename T>
void hash_combine(size_t &seed, T const &v) {
	seed ^= hash_value(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

class State {
public:
	CardGroup _last_group;
	StateId _id;
	bool _is_max;
	//vector<Card> _cards;
	vector<Player*> _players;
	int _current_idx, _current_controller, _winner, _target_idx;

	State(const State &s);
	State(const Env &env);
	~State();

	vector<vector<CardGroup>::iterator> get_action_space() const;

	bool operator==(const State &other) const {
		return _id == other._id && _last_group == other._last_group
			&& _is_max == other._is_max && _current_idx == other._current_idx
			&& _current_controller == other._current_controller
			&& _winner == other._winner && _target_idx == other._target_idx
			&& _players[0]->_cnts == other._players[0]->_cnts
			&& _players[1]->_cnts == other._players[1]->_cnts;
	}
};

template <>
struct hash<State>
{
	std::size_t operator()(const State& s) const
	{
		using std::size_t;
		using std::hash;
		using std::string;

		// Compute individual hash values for first,
		// second and third and combine them using XOR
		// and bit shifting:
		/*ostringstream ss;
		ss << static_cast<int>(s._id) << static_cast<int>(s._last_group._category) << s._last_group._rank
			<< (int)s._is_max << s._current_idx << s._current_controller << s._winner << s._target_idx;

		size_t h = hash<string>()(ss.str());*/
		size_t h = 0;
		hash_combine(h, static_cast<int>(s._id));
		hash_combine(h, static_cast<int>(s._last_group._category));
		hash_combine(h, s._last_group._rank);
		hash_combine(h, s._is_max);
		hash_combine(h, s._current_idx);
		hash_combine(h, s._current_controller);
		hash_combine(h, s._winner);
		hash_combine(h, s._target_idx);
		for (auto &c : s._players[0]->_cnts)
		{
			hash_combine(h, c);
		}
		for (auto &c : s._players[1]->_cnts)
		{
			hash_combine(h, c);
		}
		return h;
	}
};

State *step(const State &s, const CardGroup &cg);
float minimax(State &s, int &best_idx, float, float);
void step_ref(State &s, const CardGroup &cg);
