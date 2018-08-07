#include "card.h"
#include <ostream>

// sort all group cards
vector<CardGroup> get_all_actions() {
	vector<CardGroup> actions;
	actions.push_back(CardGroup({}, Category::EMPTY, 0));
	for (int i = 0; i < 4; i++)
	{
		actions.push_back(CardGroup({ Card(i) }, Category::SINGLE, i));
	}
	for (int i = 13; i < 15; i++)
	{
		actions.push_back(CardGroup({ Card(i) }, Category::SINGLE, i));
	}
	for (int i = 0; i < 4; i++)
	{
		actions.push_back(CardGroup({ Card(i), Card(i) }, Category::DOUBLE, i));
	}
	for (int i = 0; i < 4; i++)
	{
		actions.push_back(CardGroup({ Card(i), Card(i), Card(i) }, Category::TRIPLE, i));
	}
	for (int i = 0; i < 4; i++)
	{
		actions.push_back(CardGroup({ Card(i), Card(i), Card(i), Card(i) }, Category::QUADRIC, i));
	}
	actions.push_back(CardGroup({ Card(13), Card(14) }, Category::BIGBANG, 100));
	return actions;
}

auto all_actions = get_all_actions();

ostream& operator<<(ostream& os, const Card& c) {
	if (c == Card::THREE)
	{
		os << "3";
	} 
	else if (c == Card::FOUR)
	{
		os << "4";
	}
	else if (c == Card::FIVE)
	{
		os << "5";
	}
	else if (c == Card::SIX)
	{
		os << "6";
	}
	else if (c == Card::BLACK_JOKER)
	{
		os << "*";
	}
	else if (c == Card::RED_JOKER)
	{
		os << "$";
	}
	return os;
}

ostream& operator<<(ostream& os, const CardGroup& cg) {
	for (auto c : cg._cards)
	{
		os << c << ", ";
	}
	return os;
}
