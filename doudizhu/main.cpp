#include "env.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;
extern vector<CardGroup> all_actions;
extern int n_threads, max_d, max_iter;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(double percentage)
{
	int val = (int)(percentage * 100);
	int lpad = (int)(percentage * PBWIDTH);
	int rpad = PBWIDTH - lpad;
	printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	//cout << all_actions.size() << endl;
	CEnv env;
	int cnt[3] = { 0, 0 };
	fstream fs;
	fs.open("cards.bin", fstream::out | fstream::trunc);
	if (!fs.is_open())
	{
		cout << "wrong" << endl;
	}
	int total = argc > 1 ? atoi(argv[1]) : 1000;
	n_threads = argc > 2 ? atoi(argv[2]) : 8;
	max_d = argc > 3 ? atoi(argv[3]) : 50;
	max_iter = argc > 4 ? atoi(argv[4]) : 250;
	for (int i = 0; i < total; i++)
	{
		env.reset();
		for (auto c : env._cards)
		{
			fs << c;
		}
		bool done = false;
		int winner = 0;
		while (!done) {
			auto res = env.step_auto();
			done = get<1>(res);
		}
		cnt[winner]++;
		printProgress((i + 1) / double(total));
		cout << "  " << winner << "\r";
	}
	fs.close();
	cout << cnt[0] << ", " << cnt[1] + cnt[2] << endl;
	system("pause");
}