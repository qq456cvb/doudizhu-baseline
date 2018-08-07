#include "env.h"
#include <iostream>
using namespace std;
extern vector<CardGroup> all_actions;

int main() {
	//cout << all_actions.size() << endl;
	Env env;
	int cnt[3] = { 0, 0 };
	for (int i = 0; i < 100; i++)
	{
		env.reset();
		bool done = false;
		int winner = 0;
		while (!done) {
			done = env.step(winner);
		}
		cnt[winner]++;
		cout << winner << endl;
	}
	cout << cnt[0] << ", " << cnt[1] + cnt[2] << endl;
	system("pause");
}