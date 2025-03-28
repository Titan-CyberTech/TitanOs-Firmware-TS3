#include "games.h"
#include "display.h"
#include "utils.h"

const int cellSize = 10;
const int maxLength = 100;
int gridWidth, gridHeight;

int snakeX[maxLength], snakeY[maxLength];
int snakeLength = 3;
int direction = 1;

int foodX, foodY;
bool gameOver = false;

void initGame() {
  gridWidth = tft.width() / cellSize;
  gridHeight = tft.height() / cellSize;

  // Initialisation du serpent
  snakeX[0] = gridWidth / 2;
  snakeY[0] = gridHeight / 2;
  snakeX[1] = snakeX[0] - 1;
  snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[0] - 2;
  snakeY[2] = snakeY[0];

  // Initialisation de la nourriture
  foodX = random(0, gridWidth);
  foodY = random(0, gridHeight);

  gameOver = false;
}

void drawGame() {
  tft.fillScreen(COLOR_BLACK);
  
  // Dessiner la nourriture
  tft.fillRect(foodX * cellSize, foodY * cellSize, cellSize, cellSize, COLOR_RED);

  // Dessiner le serpent
  for (int i = 0; i < snakeLength; i++) {
    tft.fillRect(snakeX[i] * cellSize, snakeY[i] * cellSize, cellSize, cellSize, COLOR_GREEN);
  }
}

void updateDirection() {
  if (digitalRead(BUTTON_A) == LOW) {
    direction = (direction + 3) % 4; // Tourner à gauche
    delay(150);
  }
  if (digitalRead(BUTTON_B) == LOW) {
    direction = (direction + 1) % 4; // Tourner à droite
    delay(150);
  }
}

void updateSnake() {
  // Déplacer le serpent
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  switch (direction) {
    case 0: snakeY[0]--; break; // Haut
    case 1: snakeX[0]++; break; // Droite
    case 2: snakeY[0]++; break; // Bas
    case 3: snakeX[0]--; break; // Gauche
  }
}

bool checkCollisions() {
  // Vérifier les collisions avec les murs
  if (snakeX[0] < 0 || snakeX[0] >= gridWidth || snakeY[0] < 0 || snakeY[0] >= gridHeight) {
    return true;
  }

  // Vérifier les collisions avec le corps du serpent
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      return true;
    }
  }

  return false;
}

void checkFood() {
  // Si le serpent mange la nourriture
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    if (snakeLength < maxLength) {
      snakeLength++;
    }
    foodX = random(0, gridWidth);
    foodY = random(0, gridHeight);
  }
}

void displayGameOver() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.drawString("Game Over", tft.width() / 2, tft.height() / 2);
  delay(3000);
  displayMenu();
}

void displaySnakeGame() {
  initGame();
  
  while (!gameOver) {
    updateDirection();
    updateSnake();
    
    // Vérifier les collisions
    if (checkCollisions()) {
      gameOver = true;
    }

    // Vérifier la nourriture
    checkFood();

    // Mettre à jour l'affichage
    drawGame();
    
    delay(200); // Contrôle de la vitesse du jeu
  }

  displayGameOver();
}
