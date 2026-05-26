// プッシュスイッチ連動型警報ガジェット
// 詳細設計テンプレート準拠

#define PIN_BUTTON 2
#define PIN_BUZZER 3
#define PIN_LED    4

const int debounceDelay = 50;      // チャタリング許容時間[ms]
const int longPressTime = 2000;    // 長押し判定[ms]
const int alarmTimeout  = 10000;   // 自動停止[ms]
const int buzzerFreq    = 523;     // ブザー周波数[Hz]

bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long holdStartTime = 0;
unsigned long alarmStartTime = 0;
bool isLongPressed = false;
int currentState = 1; // 0:初期化, 1:待機, 2:警報発報

void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  currentState = 1;
  isLongPressed = false;
  Serial.begin(9600);
  Serial.println("[setup] 初期化完了");
  digitalWrite(PIN_LED, HIGH); // 起動確認
  delay(1000);
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  unsigned long now = millis();
  bool btn = readButton();
  bool longPress = checkLongPress();

  switch(currentState) {
    case 1: // 待機
      setBuzzer(false);
      setLED(false);
      Serial.println("[loop] 状態:待機");
      if ((btn && !isLongPressed) || longPress) {
        currentState = 2;
        alarmStartTime = now;
        Serial.println("State:2→警報");
        Serial.println("[loop] 分岐:待機→警報発報");
      }
      break;
    case 2: // 警報発報
      setBuzzer(true);
      setLED(true);
      Serial.println("[loop] 状態:警報発報");
      if (!btn) {
        currentState = 1;
        setBuzzer(false);
        setLED(false);
        Serial.println("State:1→待機");
        Serial.println("[loop] 分岐:警報発報→待機(ボタン解除)");
      } else if (now - alarmStartTime >= alarmTimeout) {
        currentState = 1;
        setBuzzer(false);
        setLED(false);
        Serial.println("警報:timeout自動停止");
        Serial.println("[loop] 分岐:警報発報→待機(自動停止)");
      }
      break;
    default:
      Serial.println("[loop] 状態:default(初期化)");
      currentState = 1;
      break;
  }
}

bool readButton() {
  bool reading = digitalRead(PIN_BUTTON);
  Serial.print("[readButton] 入力値:");
  Serial.println(reading);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    Serial.println("[readButton] 分岐:値変化→デバウンス開始");
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      Serial.print("raw:");
      Serial.print(reading);
      Serial.print(" confirm:");
      Serial.println(buttonState);
      Serial.println("[readButton] 分岐:デバウンス確定・状態変化");
      return true;
    }
    Serial.println("[readButton] 分岐:デバウンス確定・状態変化なし");
  }
  lastButtonState = reading;
  Serial.println("[readButton] 分岐:状態変化なし");
  return false;
}

void setBuzzer(bool onoff) {
  if (onoff) {
    tone(PIN_BUZZER, buzzerFreq);
    Serial.println("[setBuzzer] ブザーON");
  } else {
    noTone(PIN_BUZZER);
    Serial.println("[setBuzzer] ブザーOFF");
  }
}

void setLED(bool onoff) {
  digitalWrite(PIN_LED, onoff ? HIGH : LOW);
  if (onoff) {
    Serial.println("[setLED] LED ON");
  } else {
    Serial.println("[setLED] LED OFF");
  }
}

bool checkLongPress() {
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (holdStartTime == 0) holdStartTime = millis();
    if ((millis() - holdStartTime) >= longPressTime) {
      isLongPressed = true;
      Serial.println("longpress:true");
      Serial.println("[checkLongPress] 分岐:長押し判定true");
      return true;
    }
    Serial.println("[checkLongPress] 分岐:押下中・長押し未満");
  } else {
    holdStartTime = 0;
    isLongPressed = false;
    Serial.println("[checkLongPress] 分岐:ボタン離し・リセット");
  }
  return false;
}
