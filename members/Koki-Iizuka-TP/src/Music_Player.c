#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- ピン定義 ---
#define PIN_BUTTON1   2   // 再生/停止ボタン
#define PIN_BUTTON2   3   // 曲切替ボタン
#define PIN_LED       9   // LED（状態表示）
#define PIN_BUZZER   10   // パッシブブザー
#define I2C_ADDR   0x27   // LCM1602A用アドレス

// --- 状態管理 ---
typedef enum {
  STATE_WAIT = 0, // 待機
  STATE_PLAY = 1  // 再生中
} SystemState;

SystemState currentState = STATE_WAIT;

// --- 曲データ・管理 ---
#define MAX_TRACKS 3
const char* TrackName[MAX_TRACKS] = {"曲1", "曲2", "曲3"};
// int TrackList[MAX_TRACKS][xx]; // 曲データ（例：音階配列、詳細は略）
int TrackCount = MAX_TRACKS;
int TrackID = 0;

// --- 入出力状態変数 ---
bool led_State = false;
bool buzzer_State = false;

// --- ボタン・デバウンス ---
bool button1_State = false;
bool button2_State = false;
unsigned long Push_button1 = 0;
unsigned long Push_button2 = 0;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const int DEBOUNCE_DELAY = 100;

// --- LCD表示バッファ ---
char Display_Text[17]; // 16文字+NULL

// --- LCDインスタンス ---
LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

// --- プロトタイプ宣言 ---
void setup();
void loop();
void readButtons();        // ボタン入力＆デバウンス判定
void updateLED();          // LEDのON/OFF制御
void playMelody();         // 曲の再生制御
void updateLCD();          // LCDに状態や曲名表示
void togglePlay();         // 再生・停止の状態切替
void changeTrack();        // 曲の切替・選曲
void playNextTrack();      // 曲終端時の自動連続再生、または停止
// 必要に応じて 他のヘルパー関数

// --- 各関数の枠と日本語コメント ---

void setup() {
  // 各ピンモード設定、I2C/LCDの初期化、グローバル変数初期化、LCD曲名表示
}

void loop() {
  // ボタン状態の取得
  // 現在の状態に応じて切替や再生処理、LED表示、LCD表示などを管理
}

void readButtons() {
  // ボタン1,2のデジタル入力を読み取る
  // デバウンス処理（100ms以内のノイズを無視）
  // 有効な押下イベントをbuttonX_Stateで管理
}

void updateLED() {
  // currentStateの内容（再生中/待機中）によってLED点灯/消灯
}

void playMelody() {
  // TrackIDで指定される曲データを元に、ブザーで音階順再生を行う
  // 曲が終わればplayNextTrack()を呼ぶ
}

void updateLCD() {
  // LCD（LCM1602A）に現在の曲名や状態を表示
  // 曲変更・状態遷移時も即座に反映
}

void togglePlay() {
  // STATE_WAIT ⇔ STATE_PLAY をトグル
  // 状態遷移に合わせてLED、LCDの状態も更新
}

void changeTrack() {
  // TrackIDをインクリメント（曲選択）
  // TrackCountを超えたら0に戻す（サイクル）
  // 再生中ならそのまま新しい曲再生
  // LCDも更新
}

void playNextTrack() {
  // TrackIDを+1し、末尾ならSTATE_WAIT＆TrackIDリセット
  // 続きがあればplayMelody()（次曲自動再生）
  // LCDも更新
}

// 必要に応じて他のヘルパー関数