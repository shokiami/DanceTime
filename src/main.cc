#include "game.h"

int main() {
  cout << "loading DanceTime..." << endl;
  Game game = Game("feel_this_moment");
  while (!game.finished()) {
    game.update();
    game.render();
  }
  pair<double, double> result = game.results();
  if (result.first > result.second) {
    cout << "player 1 wins!" << endl;
  } else if (result.first < result.second) {
    cout << "player 2 wins!" << endl;
  } else {
    cout << "it's a tie!" << endl;
  }
  cout << std::setprecision(2) << std::fixed;
  cout << "player 1: " << result.first << "%" << endl;
  cout << "player 2: " << result.second << "%" << endl;
  cout << "thanks for playing! :)" << endl;
  return EXIT_SUCCESS;
}
