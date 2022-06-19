#include "game.h"

int main() {
  cout << "Loading DanceTime..." << endl;
  Game game;
  while (!game.finished()) {
    game.update();
    game.render();
  }
  cout << "Thanks for playing!" << endl;
  return EXIT_SUCCESS;
}
