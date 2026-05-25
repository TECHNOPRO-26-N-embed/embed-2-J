# 詳細設計書 — 組込み開発実習

<!-- 作成者: 飯塚 光希 / 日付: 2026-05-25 / グループ: 2-J -->

> **このドキュメントの目的**
> 基本設計書（basic_design.md）で「**どのような構造で作るか**」を決めました。
> この詳細設計書では「**各処理を具体的にどう実装するか**」を決めます。
> 書き終わったとき、**コードの骨格がほぼ完成している**状態を目指してください。

> [!NOTE]
> **V字モデルにおける位置づけ**
> 詳細設計書 ←→ **単体テスト**（関数・部品ごとのテスト）が対応します。
> 「この関数が正しく動くか」の確認は Section 5 の単体テスト仕様書で計画します。
> ※ 必須機能全体が動くかの「結合テスト」は基本設計書（Section 6）に記載します。

---

## 0. 基本設計書との接続確認

| 項目 | basic_design.md から転記 |
|:--|:--|
| 作品タイトル |音楽が流れるボタン|
| LCDディスプレイ型番 |LCM1602A（I2Cバックパック付）|
| 状態の種類（1-2 状態遷移から） |待機中（STATE_WAIT）、再生中（STATE_PLAY）|
| 実装する関数の数（2-2 関数一覧から） |8個（setup, loop, readButtons, updateLED, playMelody, updateLCD, togglePlay, changeTrack, playNextTrack）|
| グローバル変数の合計バイト数（2-1 SRAM確認から） |（概算）約50B＋曲データ配列のサイズ|

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。

```
【ピン定義】
  PIN_BUTTON1   = 2     // タクトスイッチ①（再生/停止）
  PIN_BUTTON2   = 3     // タクトスイッチ②（曲切替）
  PIN_LED       = 9     // LED
  PIN_BUZZER    = 10    // パッシブブザー
  PIN_LCD_SDA   = A4    // LCD通信(SDA)
  PIN_LCD_SCL   = A5    // LCD通信(SCL)

【状態管理】
  typedef enum {
      STATE_WAIT = 0,  // 待機中
      STATE_PLAY = 1   // 再生中
  } SystemState;

  currentState : SystemState = STATE_WAIT; // 初期状態は待機

【曲データ管理】
  TrackList[MAX_TRACKS][]         // 曲データ配列（int配列や構造体など）
  TrackName[MAX_TRACKS][文字数]   // 曲名配列（任意機能対応の場合）
  TrackCount     : int = 3       // 総曲数（例）
  TrackID        : int = 0       // 現在選択中の曲番号（0 start）

【LED・ブザー出力状態】
  led_State      : bool = false  // LED ON/OFF
  buzzer_State   : bool = false  // ブザーON/OFF

【ボタン状態・デバウンス判定】
  button1_State     : bool = false      // ボタン1押下状態
  button2_State     : bool = false      // ボタン2押下状態
  Push_button1      : unsigned long = 0 // ボタン1前回確定押下時刻
  Push_button2      : unsigned long = 0 // ボタン2前回確定押下時刻
  lastDebounceTime1 : unsigned long = 0 // ボタン1デバウンス用
  lastDebounceTime2 : unsigned long = 0 // ボタン2デバウンス用
  DEBOUNCE_DELAY    : const int = 100   // チャタリング判定100ms

【LCD表示関連】
  Display_Text[文字数]    // LCD表示テキストバッファ
```

---

## 2. 各関数の詳細設計

---

### `setup()` — 初期化処理

- ボタン・LED・ブザーのピンモード設定（INPUT_PULLUP, OUTPUT）
- LCD（LCM1602A/I2C）およびI2C通信の初期化
- 曲データや曲名配列・曲数の初期化
- 主要なグローバル変数の値をリセット
- LCDに初期メッセージ（例：一番目の曲名、状態）を表示
- （任意）Serial.begin(9600)
- （任意）LEDで起動確認

---

### `loop()` — メインループ

- readButtons()でボタン入力状態を読取、必要なフラグ更新
- updateLED(), updateLCD()で出力を反映
- 状態による分岐
  - STATE_WAIT時
    - ボタン1押下でtogglePlay()→STATE_PLAY遷移、演奏開始
    - ボタン2押下でchangeTrack()、選曲番号を更新
  - STATE_PLAY時
    - playMelody()で現在曲を再生（進行状況管理）
    - 曲終端ならplayNextTrack()
    - 再生中にボタン1押下→togglePlay()で停止
    - ボタン2押下でchangeTrack()即新曲へ

---

### `updateLED()`

- 状態currentState==STATE_PLAYでLED点灯、それ以外で消灯
- 状態変化時はdigitalWriteでON/OFF指令
- LED異常時はスキップ

---

### `updateLCD()`

- TrackID・currentStateの内容をもとに曲名や再生/待機表示を作成
- LCM1602A用APIで表示反映
- 曲変更や状態遷移では最新表示へ自動反映
- LCDの書き込みエラーは再描画/スキップ

---

### `readButtons()`

- digitalReadでボタン状態取得
- millis利用でデバウンス（100ms未満は無視）
- 押下/リリースイベントは明示（buttonX_Stateで管理）

---

### `togglePlay()`

- STATE_WAIT ⇔ STATE_PLAYトグル遷移
- 状態遷移後、LED/LCD表示更新
- 再生開始時にTrackID範囲外なら0へ戻す
- 異常値は待機（STATE_WAIT）へ

---

### `changeTrack()`

- TrackIDを+1（次曲）し、TrackCount以上なら0に戻すサイクル
- STATE_PLAY中は現曲停止し次曲再生
- LCD表示も同期
- 曲なし、TrackCount=0では何もしない

---

### `playMelody()`

- TrackIDで指定された曲データを曲進行に従いブザーで再生
- 曲進行管理用カウンタ・タイマー使用
- 再生完了でplayNextTrack()呼ぶ
- 再生中もボタンイベントによる割込み可能

---

### `playNextTrack()`

- TrackIDを+1
- TrackCount未満なら次曲playMelody()へ
- 最後の曲後はSTATE_WAITへ遷移しTrackID=0へリセット、各表示更新
- 曲データなし（異常時）はSTATE_WAIT強制復帰

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

- ボタン押下時は100ms未満の連続入力（ノイズ/チャタリング）を排除
- millisで前回確定時刻と比較、経過時間で有効化
- 各ボタン独立デバウンス管理

---

### 3-2. millis()によるタイミング管理

- チャタリング処理・曲時間制御はすべてmillis照合
- ブロッキングdelayの多用を避ける（曲ごとにブロック時のみ利用可）

---

### 3-3. サイクル曲・割込み処理

- changeTrack()はSTATE_WAIT/STATE_PLAYどちらも呼べる（動的に対応）
- 曲終端時のみplayNextTrack()を呼ぶ設計でTrackID=0復帰も明示
- 再生/停止/曲変更すべてでLED・LCDも正しく同期

---

## 4. デバッグ出力計画（任意）

| No | 確認したい内容 | 挿入関数 | Serial.println例 |
|:---|:---|:---|:---|
| 1 | TrackIDやcurrentStateの変化 | changeTrack(),togglePlay(),playNextTrack() | Serial.println(TrackID); Serial.println(currentState); |
| 2 | 曲データ・配列参照 | playMelody() | Serial.println(音階番号); |
| 3 | デバウンスの動作 | readButtons() | Serial.println("BTN1 valid"); |
| 4 | LCD表示更新 | updateLCD() | Serial.println(Display_Text); |

---

## 5. 単体テスト仕様書

（主要テスト項目は前バージョンと同じ、LCD型番=LCM1602Aと明示）

---

## 6. AIレビュー記録　※LCD型番・I2C仕様・割込み応答の再確認を必ず実施

---

## 7. グループレビュー記録

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 |  |  |  |
| 2 |  |  |  |
| 3 |  |  |  |

---

*初版: 2026-05-25 / AIレビュー: 2026-05-25 / グループレビュー後更新: 2026-05-25*