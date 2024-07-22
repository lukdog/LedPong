# LedPong: A Classic Pong Game on Arduino UNO R4 WiFi

LedPong is a fun and interactive implementation of the classic Pong game designed for the Arduino UNO R4 WiFi. The game utilizes the LED Matrix from the Arduino Plug&Make kit to display the ball and paddles, providing a nostalgic and engaging experience. The ball is represented by a moving LED, creating a visually appealing game environment. Additional features like a secondary Modulino encoder and optional secondary Modulino Pixels enhance the gameplay with background colors and effects.

## Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Game Controls](#game-controls)
- [Customization](#customization)
- [Contributing](#contributing)
- [License](#license)

## Features
- Classic Pong game on an LED Matrix.
- Interactive game with two paddles controlled by rotary encoders.
- Sound effects using a buzzer.
- Visual effects with background LEDs.
- Configurable game settings including ball speed and winning conditions.

## Requirements
- Arduino UNO R4 WiFi
- Arduino Plug&Make kit
- Secondary Modulino encoder (required)
- Secondary Modulino Pixels (optional for background effects)
- Arduino IDE

## Installation
1. **Clone the Repository:**
   ```sh
   git clone https://github.com/yourusername/ledpong.git
   cd ledpong
   ```

2. **Install Required Libraries:**
   - `Modulino`
   - `Scheduler`
   - `Arduino_LED_Matrix`
   
   You can install these libraries via the Arduino Library Manager.

3. **Upload the Code:**
   - Open the `ledpong.ino` file in the Arduino IDE.
   - Connect your Arduino UNO R4 WiFi to your computer.
   - Select the correct board and port from the Tools menu.
   - Upload the code to your Arduino.

## Usage
1. **Setup:**
   - Assemble your hardware setup as described in the requirements.
   - Ensure all connections are secure and components are powered.

2. **Start the Game:**
   - Power on your Arduino UNO R4 WiFi.
   - The game will initialize and start with an animation.

3. **Playing the Game:**
   - Use the rotary encoders to control the paddles.
   - The game will keep score and display the ball's movement on the LED Matrix.

## Game Controls
- **Left Paddle:** Controlled by the primary Modulino encoder.
- **Right Paddle:** Controlled by the secondary Modulino encoder.
- **Mute Sound:** Press the button connected to pin 12 to mute/unmute the sound.

## Customization
You can customize various aspects of the game by modifying the following defines in the code:

- **Ball Speed:** Adjust the ball's speed by changing `BALL_SPEED`.
- **Winning Conditions:** Set the number of points required to win by modifying `MATCH_TO_WIN`.
- **LED Brightness:** Change the brightness of the LEDs using `LED_BRI`.

## Contributing
We welcome contributions to enhance the game. To contribute:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes.
4. Push to the branch.
5. Open a pull request.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

Enjoy playing LedPong on your Arduino UNO R4 WiFi! If you encounter any issues or have suggestions for improvements, please open an issue on the GitHub repository.