#include "game.h"

int main() {
  cout << "loading DanceTime..." << endl;
  Game game;
  while (!game.finished()) {
    game.update();
    game.render();
  }
  cout << "thanks for playing! :)" << endl;
  return EXIT_SUCCESS;
}
