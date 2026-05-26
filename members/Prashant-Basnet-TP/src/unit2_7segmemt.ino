// Define pins for 7-segment display (a-g)
#define aPin 7
#define bPin 9
#define cPin 2
#define dPin 3
#define ePin 4
#define fPin 6
#define gPin 5

// Define pins for button, buzzer, and random seed
#define PIN_BUTTON A0 // ボタンのピン
#define PIN_BUZZER 10 // ブザーのピン
#define PIN_CHAOS A5  // 乱数生成用のピン

// Define buzzer frequency and ON/OFF states
#define BEEP_FREQUENCY 4000 // ブザーの周波数
#define ON 1  // ON状態
#define OFF 0 // OFF状態

// Variables for button debounce (チャタリング防止用)
unsigned long lastDebounceTime = 0; // 最後にボタンが押された時間
const unsigned long debounceDelay = 200; // チャタリング防止の遅延時間

int buttonPressed = 0; // ボタンが押されたかどうかを記録

// Variables for dice animation timing (アニメーション用)
unsigned long lastAnimationTime = 0; // 最後のアニメーション時間
const unsigned long animationInterval = 100; // アニメーション間隔

int animationStep = 0; // アニメーションのステップ数

// Function declarations (関数の宣言)
void showNumber(int x); // 数字を表示する関数
void one(void); // 数字1を表示
void two(void); // 数字2を表示
void three(void); // 数字3を表示
void four(void); // 数字4を表示
void five(void); // 数字5を表示
void six(void); // 数字6を表示
void rollTheDice(void); // サイコロを振る関数

void setup(void)
{
    Serial.begin(9600); // Start serial communication
    randomSeed(analogRead(PIN_CHAOS)); // Initialize random seed (乱数の初期化)

    // Set pin modes for 7-segment display (7セグメントディスプレイのピン設定)
    pinMode(aPin, OUTPUT);
    pinMode(bPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    pinMode(dPin, OUTPUT);
    pinMode(ePin, OUTPUT);
    pinMode(fPin, OUTPUT);
    pinMode(gPin, OUTPUT);

    // Set pin modes for button and buzzer (ボタンとブザーのピン設定)
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);

    int i;

    // Show numbers 1-6 as a demo (1〜6をデモとして表示)
    for (i = 1; i <= 6; i++)
    {
        Serial.print("Displaying number: ");
        Serial.println(i); // Print the number to Serial Monitor
        showNumber(i); // Display number (数字を表示)
        tone(PIN_BUZZER, BEEP_FREQUENCY, 100); // Short beep (短いビープ音)
        delay(1000); // Wait 1 second (1秒待つ)
    }

    tone(PIN_BUZZER, BEEP_FREQUENCY, 600); // Long beep (長いビープ音)
    delay(500); // Wait 0.5 seconds (0.5秒待つ)
}

void loop(void)
{
    unsigned long currentTime = millis(); // Get current time (現在の時間を取得)

    // Check if button is pressed (ボタンが押されたか確認)
    if (digitalRead(PIN_BUTTON) == LOW)
    {
        // If button is pressed and debounce delay passed (ボタンが押され、遅延時間が経過した場合)
        if (!buttonPressed &&
            (currentTime - lastDebounceTime > debounceDelay))
        {
            buttonPressed = 1; // Mark button as pressed (ボタンが押されたと記録)
            lastDebounceTime = currentTime; // Update last debounce time (最後の時間を更新)

            rollTheDice(); // Roll the dice (サイコロを振る)
        }
    }
    else
    {
        buttonPressed = 0; // Reset button state (ボタン状態をリセット)
    }
}

void rollTheDice(void)
{
    animationStep = 0; // Reset animation step (アニメーションステップをリセット)

    // Animate dice roll (サイコロのアニメーション)
    while (animationStep < 10)
    {
        if (millis() - lastAnimationTime >= animationInterval)
        {
            lastAnimationTime = millis(); // Update animation time (アニメーション時間を更新)

            int number = random(1, 7); // Get random number (1〜6の乱数を取得)

            Serial.print("Rolling... Number: ");
            Serial.println(number); // Print the rolling number to Serial Monitor

            showNumber(number); // Display random number (乱数を表示)

            tone(PIN_BUZZER, BEEP_FREQUENCY, 5); // Short beep (短いビープ音)

            animationStep++; // Move to next step (次のステップへ)
        }
    }

    int finalNumber = random(1, 7); // Final dice number (最終的なサイコロの数字)

    Serial.print("Final number: ");
    Serial.println(finalNumber); // Print the final number to Serial Monitor

    showNumber(finalNumber); // Display final number (最終的な数字を表示)

    tone(PIN_BUZZER, BEEP_FREQUENCY, 300); // Long beep (長いビープ音)
}

void showNumber(int x)
{
    // Display the number based on the input (入力に基づいて数字を表示)
    switch (x)
    {
        case 1:
            one(); // Show 1 (1を表示)
            break;

        case 2:
            two(); // Show 2 (2を表示)
            break;

        case 3:
            three(); // Show 3 (3を表示)
            break;

        case 4:
            four(); // Show 4 (4を表示)
            break;

        case 5:
            five(); // Show 5 (5を表示)
            break;

        case 6:
            six(); // Show 6 (6を表示)
            break;
    }
}

// Functions to display numbers on 7-segment display (7セグメントディスプレイで数字を表示する関数)
void one(void)
{
    digitalWrite(aPin, OFF);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, OFF);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, OFF);
}

void two(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, OFF);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, ON);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, ON);
}

void three(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, ON);
}

void four(void)
{
    digitalWrite(aPin, OFF);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, OFF);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}

void five(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, OFF);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}

void six(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, OFF);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, ON);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}