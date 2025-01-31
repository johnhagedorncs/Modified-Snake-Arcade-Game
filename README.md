# Modified Snake Arcade Game

A **real-time arcade game** built for the Arduino Uno platform, featuring a modified version of the classic Snake game with additional gameplay elements like golden apples, fake apples, stars, and poop obstacles. The game is displayed on a TFT screen and controlled using a joystick.

## Features
- **Classic Snake Gameplay**: Move the snake around the screen to eat apples and grow longer.
- **Golden Apples**: Temporarily remove all poop obstacles from the screen.
- **Fake Apples**: Confuse the snake's controls for a short time.
- **Stars**: Grant temporary immunity to collisions and allow the snake to pass through walls.
- **Poop Obstacles**: Randomly appear as the snake grows, adding difficulty to the game.
- **Dynamic Gameplay**: Real-time updates and responsive controls using a joystick.
- **Sound Effects**: Play sound effects using an MP3 player module.

## Technologies Used
- **Microcontroller**: Arduino Uno
- **Display**: MCUFRIEND TFT LCD Screen
- **Input**: Joystick for controlling the snake
- **Sound**: Serial MP3 Player Module
- **Programming Language**: C++ (Arduino IDE)

## How It Works
1. **Snake Movement**: The snake moves in four directions (up, down, left, right) based on joystick input.
2. **Apple Collection**: Eating apples increases the snake's length and score.
3. **Golden Apples**: Eating a golden apple removes all poop obstacles from the screen.
4. **Fake Apples**: Eating a fake apple confuses the snake's controls for a short duration.
5. **Stars**: Collecting a star grants temporary immunity, allowing the snake to pass through walls and obstacles.
6. **Poop Obstacles**: Poop appears as the snake grows, and colliding with it ends the game (unless immunity is active).
7. **Game Over**: The game ends if the snake collides with itself, walls, or poop (without immunity).

## Code Structure
- **`setup()`**: Initializes the TFT screen, joystick, MP3 player, and game variables.
- **`loop()`**: Handles game logic, including snake movement, collision detection, and rendering.
- **Helper Functions**:
  - `randomPosition()`: Generates random positions for apples, stars, and poop.
  - `inSnake()`, `inPoop()`, `inApple()`, etc.: Check if a position is occupied by the snake, poop, or apples.
  - `drawApple()`, `drawStar()`, `drawPoop()`: Render game elements on the TFT screen.

John Hagedorn - Computer Engineering © UCSB
