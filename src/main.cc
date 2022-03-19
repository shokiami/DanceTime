#include "game.h"

int main() {
  cout << "Loading DanceTime." << endl;
  Game game;
  while (!game.isFinished()) {
    game.update();
    game.render();
  }
  cout << endl << "Closing DanceTime." << endl;
  return EXIT_SUCCESS;
}
