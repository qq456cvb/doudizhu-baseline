#include "mctree.h"
#include <math.h>
#include <assert.h>
#include "env.h"
#include <unordered_set>


vector<mt19937> generators;
extern unordered_set<Category> kicker_set;

State::State(const State &s) {
	_last_group = s._last_group;
	_cache_cg = s._cache_cg;
	_current_idx = s._current_idx;
	_current_controller = s._current_controller;
	_target_idx = s._target_idx;
	_winner = s._winner;
	_id = s._id;
	_remain_len = s._remain_len;
	_single = s._single;
	for (size_t i = 0; i < s._players.size(); i++)
	{
		_players.push_back(new Player(*s._players[i]));
	}
}

State::State(const Env &env) {
	_last_group = env._last_group;
	_current_idx = env._current_idx;
	_current_controller = env._current_controller;
	_winner = -1;
	_id = StateId::NORMAL;
	_target_idx = _current_idx;
	_remain_len = 0;
	_single = false;
	for (size_t i = 0; i < env._players.size(); i++)
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

// TODO: different kicker orders lead to different nodes
vector<vector<CardGroup>::iterator> State::get_action_space(bool all) const {
	if (_remain_len > 0)
	{
		if (_single)
		{
			auto its = _players[_current_idx]->get_singles(_cache_cg);
			/*for (auto it = its.begin(); it < its.end(); it++)
			{
				if (find(_cache_cg._cards.begin(),_cache_cg._cards.end(), (*it)->_cards[0]) != _cache_cg._cards.end())
				{
					it = its.erase(it);
				}
			}*/
			return its;
		} else {
			auto its = _players[_current_idx]->get_doubles(_cache_cg);
			/*for (auto it = its.begin(); it < its.end(); it++)
			{
				if (find(_cache_cg._cards.begin(), _cache_cg._cards.end(), (*it)->_cards[0]) != _cache_cg._cards.end())
				{
					it = its.erase(it);
				}
			}*/
			return its;
		}
	}
	return _players[_current_idx]->candidate(_last_group, all);
}



Node::Node(Edge *src, State *st, vector<float> priors) {
	this->st = st;
	this->actions = st->get_action_space();
	/*if (this->actions.size() > 300)
	{
		cout << "large branching factor" << endl;
	}*/
	if (priors.empty() && !this->actions.empty()) {
		priors = vector<float>(this->actions.size(), 1.f / this->actions.size());
	}
	this->src = src;
	for (size_t i = 0; i < this->actions.size(); i++) {
		this->edges.push_back(new Edge(this, this->actions[i], priors[i]));
	}
}

Node::~Node() {
	if (this->st) {
		delete this->st;
	}
	for (auto e : this->edges) {
		if (e) {
			delete e;
		}
	}
}

Edge::Edge(Node *src, const vector<CardGroup>::iterator &action, float prior) {
	this->action = action;
	this->p = prior;
	this->src = src;
}


Edge::~Edge() {
	if (this->dest) {
		delete this->dest;
	}
}


Edge* Node::choose(float c) {
	float sum = 0.f;
	size_t e_size = edges.size();
	for (size_t i = 0; i < edges.size(); i++) {
		sum += edges[i]->n;
	}

	float nsum_sqrt = sqrtf(sum);
	int best_idx = -1;
	float best = -100.f;
	for (size_t i = 0; i < e_size; i++) {
		float cand = edges[i]->q + (edges[i]->n == 0 ? 1000.f : c * nsum_sqrt / (edges[i]->n));
		if (cand > best) {
			best_idx = i;
			best = cand;
		}
	}
	return edges[best_idx];
}


MCTree::MCTree(State* st, float c) {
	this->root = new Node(nullptr, st);
	//cout << root->actions.size() << endl;
	this->counter = 0;
	this->c = c;
}


MCTree::~MCTree() {
	if (root) {
		delete root;
	}
}

void MCTree::search(int n_threads, int n) {
	if (generators.empty()) {
		for (int i = 0; i < n_threads; i++) {
			generators.push_back(mt19937(random_device{}()));
		}
	}
	counter = n;
	vector<thread> threads;
	for (int i = 0; i < n_threads; i++) {

		threads.push_back(std::move(std::thread(&MCTree::search_thread, this, &generators[i])));
	}
	for (auto& t : threads) {
		t.join();
	}
}

void MCTree::search_thread(mt19937 *generator) {
	while (true) {
		{
			std::lock_guard<std::mutex> lock(counter_mu);
			if (counter == 0) {
				break;
			}
			else {
				counter--;
			}
		}
		float val = 0.f;
		// cout << "explore" << endl;
		
		Node* leaf = explore(root, val, *generator);
		// cout << val << endl;
		backup(leaf, val);
	}
}


// TODO: change node lock to per 
Node* MCTree::explore(Node* node, float& val, mt19937 &generator) {
	std::unique_lock<std::mutex> lock(node->mu);
	auto edge = node->choose(this->c);
	if (edge->dest) {
		if (edge->terminiated) {
			val = edge->r;
			return edge->dest;
		}
		else {
			return explore(edge->dest, val, generator);
		}
	}
	else {
		// cout << node->st->idx << ": " << static_cast<int>(node->st->id) << ", ";
		// cout << node->st->get_action_space().size() << endl;
		
		State* sprime = step(*node->st, edge->action);
		edge->dest = new Node(edge, sprime);

		if (sprime->_id == StateId::FINISHED) {
			if (sprime->_winner == sprime->_target_idx)
			{
				edge->r = 1.f;
			} else {
				if (sprime->_target_idx + sprime->_winner == 3)
				{
					edge->r = 1.f;
				} else {
					edge->r = 0.f;
				}
			}
			edge->terminiated = true;
			val = edge->r;
			return edge->dest;
		}
		// force step intermediate level
		if (sprime->_remain_len > 0)
		{
			return explore(edge->dest, val, generator);
		}
		auto dest = edge->dest;
		lock.unlock();
		val = rollout(dest, generator);
		return dest;
	}
}

void MCTree::backup(Node* node, float val) {
	while (node->src) {
		auto edge = node->src;
		{
			std::lock_guard<std::mutex> lock(edge->src->mu);
			edge->n++;
			edge->w += val;
			edge->q = edge->w / edge->n;
		}
		node = edge->src;
	}
}

float MCTree::rollout(Node* node, mt19937 &generator) {
	auto st = node->st;
	auto s = State(*st);
	while (s._id != StateId::FINISHED) {
		auto actions = s.get_action_space();
		step_ref(s, actions[generator() % actions.size()]);
	}
	float r = 0;
	// cout << st->winner << endl;
	if (s._winner == s._target_idx)
	{
		r = 1.f;
	}
	else {
		if (s._target_idx + s._winner == 3)
		{
			r = 1.f;
		}
		else {
			r = 0.f;
		}
	}
	return r;
}


void predict_helper(HashNode *node, Node *s, int remain_len) {
	if (!s) return;

	if (remain_len == 0)
	{
		return;
	}
	for (size_t i = 0; i < s->edges.size(); i++) {
		HashNode *next = nullptr;
		auto it = find_if(node->_cnt_map.begin(), node->_cnt_map.end(), [&](const pair<const HashNode*, int> &p) {
			return s->edges[i]->dest && p.first->_cg == s->edges[i]->dest->st->_cache_cg;
		});
		if (it != node->_cnt_map.end())
		{
			next = it->first;
		}
		else {
			if (!s->edges[i]->dest)
			{
				continue;
			}
			next = new HashNode();
			next->_cg = s->edges[i]->dest->st->_cache_cg;
			node->_cnt_map[next] += s->edges[i]->n;
		}
		predict_helper(next, s->edges[i]->dest, remain_len - 1);
	}
}

void MCTree::predict(HashNode *node) {
	int cnt = 0;
	for (size_t i = 0; i < root->edges.size(); i++) {
		// cout << root->edges[i]->q << ", ";
		cnt += root->edges[i]->n;
		HashNode *next = nullptr;
		auto it = find_if(node->_cnt_map.begin(), node->_cnt_map.end(), [&](const pair<const HashNode*, int> &p) {
			return p.first->_cg == *root->edges[i]->action;
		});
		if (it != node->_cnt_map.end())
		{
			next = it->first;
		} else {
			next = new HashNode();
			next->_cg = *root->edges[i]->action;
			node->_cnt_map[next] += root->edges[i]->n;
		}
		
		if (kicker_set.find(root->edges[i]->action->_category) != kicker_set.end())
		{
			auto dest = root->edges[i]->dest;
			if (dest)
			{
				predict_helper(next, dest, dest->st->_remain_len);
			}
		}
	}
	// sanity check
	assert(cnt == 250);
}


// TODO: whether to force step intermediete state?
void step_ref(State &s, const vector<CardGroup>::iterator &a) {
	auto cg = *a;
	if (kicker_set.find(cg._category) != kicker_set.end() && s._remain_len == 0)
	{
		s._remain_len = cg._len;
		if (cg._category == Category::THREE_ONE || cg._category == Category::THREE_ONE_LINE ||
			cg._category == Category::FOUR_TAKE_ONE)
		{
			s._single = true;
		}
		else {
			s._single = false;
		}
		s._cache_cg = cg;
		for (int i = 0; i < cg._len * (s._single ? 1 : 2); i++)
		{
			s._cache_cg._cards.pop_back();
		}
		return;
	}
	else if (s._remain_len > 0) {

		if (cg._category == Category::SINGLE) {
			s._cache_cg._cards.push_back(cg._cards[0]);
		}
		else if (cg._category == Category::DOUBLE) {
			s._cache_cg._cards.push_back(cg._cards[0]);
			s._cache_cg._cards.push_back(cg._cards[1]);
		}
		else {
			assert(false);
		}
		--s._remain_len;
		if (s._remain_len == 0)
		{
			cg = s._cache_cg;
		} else {
			return;
		}
	}

	s._players[s._current_idx]->remove_cards(cg._cards);

	auto next_idx = (s._current_idx + 1) % 3;
	if (cg._category != Category::EMPTY)
	{
		s._current_controller = s._current_idx;
		s._last_group = cg;
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
	return;
}

State* step(const State& s, const vector<CardGroup>::iterator &a) {
	auto cg = *a;
	State *sprime = new State(s);
	if (kicker_set.find(cg._category) != kicker_set.end() && sprime->_remain_len == 0)
	{
		sprime->_remain_len = cg._len;
		if (cg._category == Category::THREE_ONE || cg._category == Category::THREE_ONE_LINE ||
			cg._category == Category::FOUR_TAKE_ONE)
		{
			sprime->_single = true;
		} else {
			sprime->_single = false;
		}
		sprime->_cache_cg = cg;
		for (int i = 0; i < cg._len * (sprime->_single ? 1 : 2); i++)
		{
			sprime->_cache_cg._cards.pop_back();
		}
		return sprime;
	} else if (sprime->_remain_len > 0) {
		
		if (cg._category == Category::SINGLE) {
			sprime->_cache_cg._cards.push_back(cg._cards[0]);
		} else if (cg._category == Category::DOUBLE) {
			sprime->_cache_cg._cards.push_back(cg._cards[0]);
			sprime->_cache_cg._cards.push_back(cg._cards[1]);
		} else {
			assert(false);
		}
		--sprime->_remain_len;
		if (sprime->_remain_len == 0)
		{
			cg = sprime->_cache_cg;
		} else {
			return sprime;
		}
	}
	
	sprime->_players[sprime->_current_idx]->remove_cards(cg._cards);

	auto next_idx = (sprime->_current_idx + 1) % 3;
	if (cg._category != Category::EMPTY)
	{
		sprime->_current_controller = sprime->_current_idx;
		sprime->_last_group = cg;
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