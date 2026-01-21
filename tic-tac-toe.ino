#include <Servo.h>

// Parameters
struct BoardConfig {
    float centerX = -190.0;
    float centerY = 0.0; // Vertical
    float centerZ = 0.0;
    
    float homeX = -220.0;
    float homeY = -24.0;
    float homeZ = -140.0;

    float cellSpacing = 40.0; // Side length of cells

    float moveHeight = 20.0;
    float hoverHeight = -8.0;
    
    unsigned int dwellTime = 2000;
} boardConfig;

class SmartServo {
  private:
    Servo _servo;
    int _currentAngle;
    int _minAngle;
    int _maxAngle;
    int _pin;

  public:
    SmartServo(int minAngle = 0, int maxAngle = 180): _minAngle(minAngle), _maxAngle(maxAngle), _currentAngle(minAngle) {}

    void begin(int pin, int initialAngle = -1) {
      _pin = pin;
      _servo.attach(_pin);
      writeAngle(initialAngle < 0 ? _currentAngle : initialAngle);
    }

    void writeAngle(int angle) {
      _currentAngle = constrain(angle, _minAngle, _maxAngle);
      //_servo.write(_currentAngle);
      _servo.write(map(_currentAngle, 0, 180, 544, 2400));
    }

    void writeAngleSmooth(int targetAngle, int stepDelay = 20) {
      targetAngle = constrain(targetAngle, _minAngle, _maxAngle);
      int step = (targetAngle > _currentAngle) ? 1 : -1;

      while (_currentAngle != targetAngle) {
        _currentAngle += step;
        //_servo.write(_currentAngle);
        _servo.write(map(_currentAngle, 0, 180, 544, 2400));
        delay(stepDelay);
      }
    }

    void increment(int amount) {
      writeAngle(_currentAngle + amount);
    }

    int getLastAngle() {
      return _currentAngle;
    }
};

// Controls one joystick and one button
class controller {
  private:
    int _XPin, _YPin, _buttonPin;
  public:
    unsigned long startTime;
    bool state;
    controller(int XPin, int YPin, int buttonPin)
        : _XPin(XPin), _YPin(YPin), _buttonPin(buttonPin), startTime(0), state(false) {}

    void begin() {
      pinMode(_buttonPin, INPUT_PULLUP);
    }
    
    int getX() {
      return abs(512 - analogRead(_XPin)) < 40 ? 512 : analogRead(_XPin);
    }
    int getY() {
      return abs(512 - analogRead(_YPin)) < 40 ? 512 : analogRead(_YPin);
    }
    bool refresh() {
      state = (digitalRead(_buttonPin) == 0);
      return state;
    }
};

class TicTacToe {
private:
    char board[3][3];  // ' ' empty, 'X' Dolphin, 'O' Human
    
public:
    TicTacToe() {
        reset();
    }
    
    void reset() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                board[i][j] = ' ';
    }
    
    void printBoard() {
        Serial.println("=============");
        for (int i = 0; i < 3; i++) {
            Serial.print("| ");
            for (int j = 0; j < 3; j++) {
                Serial.print(board[i][j]);
                Serial.print(" | ");
            }
            Serial.println();
            Serial.println("=============");
        }
    }
    
    bool makeMove(int row, int col, char player) {
        if (row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != ' ') {
            return false;
        }
        board[row][col] = player;
        return true;
    }
    
    bool isFull() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (board[i][j] == ' ') return false;
        return true;
    }
    
    char checkWinner() {
        // Rows
        for (int i = 0; i < 3; i++) {
            if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
                return board[i][0];
        }
        
        // Columns
        for (int j = 0; j < 3; j++) {
            if (board[0][j] != ' ' && board[0][j] == board[1][j] && board[1][j] == board[2][j])
                return board[0][j];
        }
        
        // Diagonals
        if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
            return board[0][0];
        if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
            return board[0][2];
            
        return ' ';
    }
    
    // Computer strategy
    void getBestMove(int &row, int &col) {
        // 1. Directly win
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = 'X';
                    if (checkWinner() == 'X') {
                        board[i][j] = ' ';
                        row = i; col = j;
                        return;
                    }
                    board[i][j] = ' ';
                }
            }
        }
        
        // 2. Stop human from winning
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = 'O';
                    if (checkWinner() == 'O') {
                        board[i][j] = ' ';
                        row = i; col = j;
                        return;
                    }
                    board[i][j] = ' ';
                }
            }
        }
        
        // 3. Center
        if (board[1][1] == ' ') {
            row = 1; col = 1;
            return;
        }
        
        // 4. Corners
        int corners[4][2] = {{0,0}, {0,2}, {2,0}, {2,2}};
        for (int i = 0; i < 4; i++) {
            if (board[corners[i][0]][corners[i][1]] == ' ') {
                row = corners[i][0]; col = corners[i][1];
                return;
            }
        }
        
        // 5. Sides
        int edges[4][2] = {{0,1}, {1,0}, {1,2}, {2,1}};
        for (int i = 0; i < 4; i++) {
            if (board[edges[i][0]][edges[i][1]] == ' ') {
                row = edges[i][0]; col = edges[i][1];
                return;
            }
        }
        
        // 6. Arbitrary move
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    row = i; col = j;
                    return;
                }
            }
        }
    }
};

class ArmController {
private:
    SmartServo &left, &right, &horizontal, &claw;
    
public:
    ArmController(SmartServo &l, SmartServo &r, SmartServo &h, SmartServo &c) 
        : left(l), right(r), horizontal(h), claw(c) {}
    
    // Kinematics (From Feishu document)
    float angle_horizontal;
    float angle_left;
    float angle_right;
    
    float square(float n) {
        return n * n;
    }
    
    float sgn(float num) {
        if (num > 0) return 1.0;
        else if (num < 0) return -1.0;
        else return 0.0;
    }
    
    void inverseOperation(float x, float y, float z) {
        angle_horizontal = -degrees(atan(z / x)) * (72.0 / 28.0) + 90.0;
        
        float temp1 = degrees(atan((y - 65.5) / (-sgn(x) * sqrt(x * x + z * z) - 7.0 - 60.0)));
        float temp2 = degrees(acos((135.0 * 135.0 + square(-sgn(x) * sqrt(x * x + z * z) - 7.0 - 60.0) + square(y - 65.5) - 145.0 * 145.0) / (2.0 * 135.0 * sqrt(square(-sgn(x) * sqrt(x * x + z * z) - 7.0 - 60.0) + square(y - 65.5)))));
        angle_left = 180.0 - 69.0 - temp2 - temp1;
        
        float temp3 = degrees(acos((145.0 * 145.0 + 135.0 * 135.0 - square(-sgn(x) * sqrt(x * x + z * z) - 67.0) - square(y - 65.5)) / (2.0 * 145.0 * 135.0)));
        angle_right = 180.0 - (83.5 + (180.0 - 69.0 - temp2 - temp1) - temp3);
    }
    
    // Move to designated coordinates
    void moveTo(float x, float y, float z, int stepDelay = 20) {
        inverseOperation(x, y, z);
        
        left.writeAngleSmooth(constrain(angle_left, 0, 170), stepDelay);
        delay(500);
        right.writeAngleSmooth(constrain(angle_right, 0, 95), stepDelay);
        delay(500);
        horizontal.writeAngleSmooth(constrain(angle_horizontal, 0, 180), stepDelay);
    }
    
    // Move to designated cell
    void moveToCell(int row, int col, float height) {
        float x = boardConfig.centerX + (1 - row) * boardConfig.cellSpacing * 1.1;
        float y = boardConfig.centerY - height;
        float z = boardConfig.centerZ - (1 - col) * boardConfig.cellSpacing;
        
        Serial.print("Move to cell[");
        Serial.print(row);
        Serial.print("][");
        Serial.print(col);
        Serial.print("] coordinate (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(y);
        Serial.print(", ");
        Serial.print(z);
        Serial.println(")");
        
        moveTo(x, y, z);
    }
    
    void performMove(int row, int col) {
        // 在起点夹取棋子
        claw.writeAngle(25);
        delay(500);
        moveTo(boardConfig.homeX, boardConfig.homeY, boardConfig.homeZ, 30);
        claw.writeAngleSmooth(0);
        delay(500);

        // 移动到悬停位置
        moveToCell(row, col, boardConfig.hoverHeight);
        delay(500);
        
        // 在棋盘高度落子
        // moveToCell(row, col, boardConfig.moveHeight);
        // delay(500);
        claw.writeAngleSmooth(25);
        delay(500);
        
        // 抬起
        moveToCell(row, col, boardConfig.hoverHeight - 15);
        moveBack(boardConfig.homeX, boardConfig.homeY + 30, boardConfig.homeZ, 30);
    }
    
    void moveBack(float x, float y, float z, int stepDelay = 20) {
        inverseOperation(x, y, z);
        
        horizontal.writeAngleSmooth(constrain(angle_horizontal, 0, 180), stepDelay);
        delay(500);
        right.writeAngleSmooth(constrain(angle_right, 0, 95), stepDelay);
        delay(500);
        left.writeAngleSmooth(constrain(angle_left, 0, 170), stepDelay);
    }
};

SmartServo left(0, 170), right(0, 95), horizontal(0, 180), claw(0, 25);
controller joystick1(A0, A1, 2), joystick2(A2, A3, 3);
ArmController arm(left, right, horizontal, claw);
TicTacToe game;

enum GameState {
    IDLE,
    GAME_RUNNING,
    GAME_OVER
};
GameState currentState = IDLE;

void setup() {
    Serial.begin(9600);
    
    left.begin(8, 73);
    right.begin(9, 111);
    horizontal.begin(10, 6);
    claw.begin(11, 0);
    
    joystick1.begin();
    joystick2.begin();
    
    arm.moveBack(boardConfig.homeX, boardConfig.homeY + 30, boardConfig.homeZ, 30);;
    
    Serial.println("========================================");
    Serial.println("        Dolphin Tic-Tac-Toe Game        ");
    Serial.println("========================================");
    Serial.println(" Long press 2 buttons together to start ");
    Serial.println(" Long press 2 buttons to force end game ");
    Serial.println(" Human player inputs via serial monitor ");
    Serial.println("Format: Row index(0-2) Column index(0-2)");
    Serial.println("      e.g.  1 1 is the center cell      ");
    Serial.println("========================================");
}

void loop() {
    static const int gestureDuration = 1500;
    static const int syncWindow = 200;
    static bool gestureRecognized = false;
    static unsigned long now = 0;

    now = millis();

    // if (currentState != GAME_RUNNING) {
    //     left.increment(map(joystick1.getX(), 0, 1023, -4, 4));
    //     right.increment(map(joystick1.getY(), 0, 1023, -4, 4));
    //     horizontal.increment(map(joystick2.getX(), 0, 1023, -4, 4));
    //     claw.increment(map(joystick2.getY(), 0, 1023, -8, 8));
    // }

    if (joystick1.refresh() && joystick1.startTime <= 0) {
        joystick1.startTime = now;
    } else if (!joystick1.state) {
        joystick1.startTime = 0;
    }
    if (joystick2.refresh() && joystick2.startTime <= 0) {
        joystick2.startTime = now;
    } else if (!joystick2.state) {
        joystick2.startTime = 0;
    }

    if (!joystick1.state && !joystick2.state) {
        gestureRecognized = false;
    }

    if (!gestureRecognized) {
        now = millis();
        if (joystick1.state && joystick2.state && 
            (now - joystick1.startTime >= gestureDuration) && 
            (now - joystick2.startTime >= gestureDuration)) {
            gestureRecognized = true;
            
            if (currentState != GAME_RUNNING) {
                Serial.println("\n========================================");
                Serial.println("              Game starts!              ");
                Serial.println("========================================");
                startGame();
            } /*else if (currentState == GAME_RUNNING) {
                Serial.println("\n========================================");
                Serial.println("           Game force ended !           ");
                Serial.println("========================================");
                endGame();
            }*/
        }
    }
    
    // if (currentState == GAME_RUNNING) {
    //     processHumanInput();
    // }
    
    delay(100);
}

void startGame() {
    currentState = GAME_RUNNING;
    game.reset();
    
    Serial.println("\nNew game! Dolphin goes first (X), human (O)");
    game.printBoard();
    
    // Dolphin's first move (in the center)
    delay(1000);
    makeArmMove();
}

void endGame() {
    currentState = GAME_OVER;
    arm.moveBack(boardConfig.homeX, boardConfig.homeY + 30, boardConfig.homeZ, 30);;
    Serial.println("\nGame ended!");
    delay(2000);
    Serial.println("\nLong press two buttons to start a new game!");
}

void makeArmMove() {
    int row, col;
    game.getBestMove(row, col);
    
    Serial.println("\nDolphin thinking...");
    delay(1000); // Pretend to be thinking
    
    Serial.print("Dolphin chooses: [");
    Serial.print(row);
    Serial.print("][");
    Serial.print(col);
    Serial.println("]");
    
    arm.performMove(row, col);
    
    // Refreshes board
    game.makeMove(row, col, 'X');
    game.printBoard();
    
    // Check if there is winner
    char winner = game.checkWinner();
    if (winner == 'X') {
        Serial.println("\n========================================");
        Serial.println("Dolphin won!");
        Serial.println("========================================");
        currentState = GAME_OVER;
        delay(2000);
        Serial.println("\nLong press two buttons to start a new game!");
    } else if (game.isFull()) {
        Serial.println("\n========================================");
        Serial.println("Draw!");
        Serial.println("========================================");
        currentState = GAME_OVER;
        delay(2000);
        Serial.println("\nLong press two buttons to start a new game!");
    } else {
        while (Serial.available()) {
            Serial.read();
        }
        Serial.println("\nHuman Plyer's turn! Input position (row column)");
        while (!processHumanInput()) {
            delay(200);
        }
    }
}

bool processHumanInput() {
    static int pressCount = 0;
    static bool button2Pressed = false, lastButton1State = false;
    int row = -1, col = -1;
    bool receivedInput = false;

    if (Serial.available() > 0) { // Input from Serial
        receivedInput = true;
        String input = Serial.readStringUntil('\n');
        if (input.startsWith("exit")) {
            endGame();
            pressCount = 0;
            return true;
        }
        input.trim();

        if (sscanf(input.c_str(), "%d %d", &row, &col) != 2) {
            Serial.println("Invalid format! Input row-num col-num");
            Serial.println("");
            return false;
        } else if (row < 0 || row > 2 || col < 0 || col > 2)  {
            Serial.println("Invalid input! Row/Column must be 0-2");
            return false;
        }
    } else if (joystick2.refresh()) { // Input by button
        receivedInput = true;
        if (pressCount > 8) {
            Serial.println("Invalid input! Press 0~8 times");
            pressCount = 0;
            return false;
        }
        row = pressCount / 3;
        col = pressCount % 3;
        pressCount = 0;
    } else {
        bool currentButton1State = joystick1.refresh();
        if (currentButton1State && !lastButton1State) {
            pressCount++;
            Serial.print("Count: ");
            Serial.print(pressCount);
            delay(50);
        }
        lastButton1State = currentButton1State;
    }

    if (receivedInput && game.makeMove(row, col, 'O')) {
        Serial.print("Human player makes the move: [");
        Serial.print(row);
        Serial.print("][");
        Serial.print(col);
        Serial.println("]");
        game.printBoard();
        pressCount = 0;
                    
        // Check if there is winner
        if (game.checkWinner() == 'O') {
            Serial.println("\n========================================");
            Serial.println("Human won!");
            Serial.println("========================================");
            currentState = GAME_OVER;
            delay(2000);
            Serial.println("\nLong press two buttons to start a new game!");
        } else if (game.isFull()) {
            Serial.println("\n========================================");
            Serial.println("Draw!");
            Serial.println("========================================");
            currentState = GAME_OVER;
            delay(2000);
            Serial.println("\nLong press two buttons to start a new game!");
        } else {
            // Dolphin's turn
            delay(1000);
            makeArmMove();
        }
        return true;
    } else if (receivedInput) {
        Serial.println("Occupied! Choose another position");
        return false;
    }
}