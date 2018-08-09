#pragma once
#include <vector>
#include <xhash>
using namespace std;

enum class Category {
	EMPTY = 0,
	SINGLE = 1,
	DOUBLE = 2,
	TRIPLE = 3,
	QUADRIC = 4,
	THREE_ONE = 5,
	THREE_TWO = 6,
	SINGLE_LINE = 7,
	DOUBLE_LINE = 8,
	TRIPLE_LINE = 9,
	THREE_ONE_LINE = 10,
	THREE_TWO_LINE = 11,
	BIGBANG = 12,
	FOUR_TAKE_ONE = 13,
	FOUR_TAKE_TWO = 14
};

template<typename T>
void hash_combine(size_t &seed, T const &v) {
	seed ^= hash_value(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

enum class Card {
	THREE = 0,
	FOUR = 1,
	FIVE = 2,
	SIX = 3,
	SEVEN = 4,
	EIGHT = 5,
	NINE = 6,
	TEN = 7,
	JACK = 8,
	QUEEN = 9,
	KING = 10,
	ACE = 11,
	TWO = 12,
	BLACK_JOKER = 13,
	RED_JOKER = 14

	
};

ostream& operator<<(ostream& os, const Card& c);

class CardGroup {
public:
	CardGroup() {};
	CardGroup(const vector<Card> &cards, Category category, int rank, int len = 1)
		: _cards(cards), _category(category), _rank(rank), _len(len)
	{
	};

	vector<Card> _cards;
	Category _category;
	int _rank, _len;

	bool operator==(const CardGroup &other) const {
		return _cards == other._cards;
	}
	bool operator>(const CardGroup &other) const {
		if (this->_category == Category::EMPTY) return other._category != Category::EMPTY;
		if (other._category == Category::EMPTY) return true;
		if (this->_category == Category::BIGBANG)
		{
			return true;
		}
		if (other._category == Category::BIGBANG)
		{
			return false;
		}
		if (this->_category != other._category)
		{
			if (this->_category == Category::QUADRIC)
			{
				return true;
			}
			if (other._category == Category::QUADRIC)
			{
				return false;
			}
			return false;
		}
		else {
			return this->_rank > other._rank && this->_len == other._len;
		}
	}
	bool operator<(const CardGroup &other) const {
		return other > *this;
	}
	friend ostream& operator<<(ostream& os, const CardGroup& cg);
};

template <>
struct hash<CardGroup>
{
	std::size_t operator()(const CardGroup &cg) const
	{
		using std::size_t;
		using std::hash;

		size_t h = 0;
		for (auto &c : cg._cards)
		{
			hash_combine(h, static_cast<int>(c));
		}
		return h;
	}
};
