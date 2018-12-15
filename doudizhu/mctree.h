#pragma once

#include <vector>

#include <iostream>
#include <fstream>

#include "player.h"
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <random>

using namespace std;

#define NEG_REWARD -1.f

enum class StateId {
	NORMAL = 0,
	FINISHED = 1
};

class State {
public:
	CardGroup _last_group;
	StateId _id;
	vector<Player*> _players;
	int _current_idx, _current_controller, _winner, _target_idx;

    State();
	State(const State &s);
	State(const CEnv &env);
	~State();

	vector<vector<CardGroup>::iterator> get_action_space() const;
};


class Edge;

class Node {
public:
	State *st = nullptr;
	vector<vector<CardGroup>::iterator> actions;
	Edge *src = nullptr;
	vector<Edge*> edges;
	std::mutex mu;

	Node(Edge *src, State*st, vector<float> priors = vector<float>());

	~Node();

	Edge *choose(float c = sqrtf(2.f));
};


class Edge {
public:
	vector<CardGroup>::iterator action;
	int n = 0;
	float w = 0.f;
	float q = 0.f;
	bool terminiated = false;
	std::shared_timed_mutex mu;
	float r = 0.f;
	float p = 0.f;
	Node *src = nullptr;
	Node *dest = nullptr;

	Edge(Node *src, const vector<CardGroup>::iterator &action, float prior);

	~Edge();
};


class MCTree {
public:
	Node *root = nullptr;
	int idx = -1;
	int counter = 0;
	float c = 0;
	std::mutex counter_mu;

	MCTree(State*, float c = sqrtf(2.f));

	~MCTree();

	void search(int n_threads, int n);
	void search_thread(mt19937 *generator);
	Node *explore(Node *node, float &val, mt19937 &generator);
	void backup(Node *node, float val);
	float rollout(Node *node, mt19937 &generator);
	vector<int> predict();
};

CardGroup mcsearch(vector<Card> self_cards, vector<Card> unseen_cards,
    int next_handcards_cnt,
    const CardGroup &last_cardgroup, int current_idx, int current_controller,
    int n_threads, int max_d, int max_iter);
void step_ref(State &s, const vector<CardGroup>::iterator &a);
State* step(const State& s, const vector<CardGroup>::iterator &a);