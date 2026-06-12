# doudizhu-baseline

A multithreaded C++ **Monte Carlo Tree Search** player for **Dou Di Zhu (斗地主)**, used as the search-based baseline and opponent in the related RL projects — [doudizhu-C](https://github.com/qq456cvb/doudizhu-C) (the `mcsearch` import) and the [doudizhu-tornado](https://github.com/qq456cvb/doudizhu-tornado) web game.

## How It Works

Dou Di Zhu is imperfect-information, so plain MCTS doesn't apply directly. `mcsearch` uses **determinized MCTS**:

1. **Determinization** — the unseen cards are shuffled and dealt to the two hidden opponents (`max_d` random deals).
2. **Parallel UCT** — for each deal, a tree search runs `max_iter` iterations across `n_threads` threads on a shared, mutex-protected tree (UCB with c = √2), with uniformly random rollouts to the end of the game. Rewards are team-aware: +1 if the searching seat's side (landlord vs. farmers) wins, −1 otherwise.
3. **Vote** — root visit counts are summed over all determinizations and the most-visited action is played.

Supporting code:

- `card.cpp / player.cpp` — card/category model, enumeration of all legal card groups from a hand, and the follow/beat rules.
- `env.cpp` — a self-contained 3-player game environment whose seats are all MCTS players (`step_auto` plays one turn).
- `minimax.cpp` — an experimental exact alpha-beta solver with hashed states for two-player endgames.
- `main.cpp` — a self-play benchmark: plays N games, logs each deal to `cards.bin`, and reports landlord vs. farmer win counts. CLI: `doudizhu [total_games] [n_threads] [max_d] [max_iter]`.

## Building

Two targets:

- **Python module** (the way the other repos consume this): requires [pybind11](https://github.com/pybind/pybind11); `cmake` the `doudizhu/` folder to build the `mct` extension exposing `mcsearch`, `CEnv`, `CCardGroup`, `CCard`, and `CCategory`.
- **Standalone benchmark**: open `doudizhu.sln` in Visual Studio (C++14).

```python
from mct import mcsearch, CCard, CCardGroup, CCategory

intention = mcsearch(self_cards, unseen_cards, next_handcards_cnt,
                     last_cardgroup, current_idx, current_controller,
                     n_threads=10, max_d=50, max_iter=250)
```
