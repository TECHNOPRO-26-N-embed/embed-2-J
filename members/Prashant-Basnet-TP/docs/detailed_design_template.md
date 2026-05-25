
# Detailed Design Document — Embedded Systems Practice

Prashant Basnet · 2026-05-22 / Group: 2-J

---

## 0. Connection to Basic Design

| Item | From basic_design.md |
|:--------------------------|:-----------------------------------------------|
| Project Title             | Electronic Dice                                |
| State Types (from 1-2)    | Startup Animation, Standby, Rolling, Result    |
| Number of Functions (2-2) | 10 (setup, loop, rollTheDice, showNumber, one, two, three, four, five, six) |
| Global Variable Size (2-1)| ~30B (estimate)                               |

---

## 1. Global Variables & Constants Design

```c
// Pin Definitions
// aPin = 7, bPin = 9, cPin = 2, dPin = 3, ePin = 4, fPin = 6, gPin = 5
// PIN_BUTTON = A0, PIN_BUZZER = 10, PIN_CHAOS = A5

// 7-segment display wiring diagram (see hardware doc)

// State Management
// No explicit currentState variable; flow is controlled by input checks in loop() and rollTheDice() calls

// Timers (for millis())
// lastDebounceTime: unsigned long = 0
// debounceDelay: const int = 200
// lastAnimationTime: unsigned long = 0
// animationInterval: const int = 100

// Input Values
// buttonPressed: int = 0

// Other Flags/Counters
// animationStep: int = 0
// BEEP_FREQUENCY: const int = 4000
```

---

## 2. Function Design Details

### setup() — Initialization

```c
// Summary: Initializes pins, random seed, and runs startup animation
// Steps:
// 1. Set all 7-segment LED pins to OUTPUT
// 2. Set button pin to INPUT_PULLUP
// 3. Set buzzer pin to OUTPUT
// 4. Initialize random seed with analogRead(PIN_CHAOS)
// 5. Run startup animation (show 1–6 in order + buzzer)
```

---

### loop() — Main Loop

```c
// Summary: Main program loop, handles button input, debounce, and dice rolling
// Steps:
// 1. Check button state (with debounce)
// 2. If pressed, call rollTheDice()
// 3. Otherwise, remain in standby
```

---

### rollTheDice() — Dice Rolling Animation

```c
// Summary: Animates dice roll, then displays result
// Steps:
// 1. Animate 7-segment display (1–6 cycling)
// 2. Play buzzer sound
// 3. After animation, generate random number (1–6)
// 4. Show result on display
```

---

### showNumber(x) — Display Number

```c
// Summary: Lights up segments to display number x (1–6)
// Input: x (int, 1–6)
// Output: Updates 7-segment display
```

---

### one(), two(), ..., six() — Segment Patterns

```c
// Summary: Set segment pins for each number
// Called by showNumber(x)
```

---

## 3. Error Handling & Edge Cases

- If the button is held down, only one roll is triggered per press (debounce logic)
- If hardware fails (e.g., button stuck), system ignores further input until released

---

## 4. Traceability Matrix (Requirement ↔ Function)

| Requirement                | Function(s)                |
|:---------------------------|:---------------------------|
| Random dice (1–6)          | rollTheDice, showNumber    |
| Debounce                   | loop, debounce logic       |
| Startup animation          | setup, showNumber, buzzer  |
| Buzzer sound               | setup, rollTheDice         |
| Standby state              | loop                       |

---

## 5. Test Items

- Confirm dice shows 1–6 randomly
- Confirm debounce prevents double input
- Confirm startup animation and buzzer
- Confirm only one roll per button press

---

## 6. Review Record

| Reviewer | Date       | Comments                |
|:---------|:-----------|:------------------------|
| Teacher  | 2026-05-25 | Formatting and comments improved |

---

```
【処理の流れ】
＜毎ループ実行すること＞
- 現在時刻を取得: now = millis()
- ボタン状態をdigitalReadで取得し、チャタリング対策（millisで判定）
- ボタンがLOWかつ buttonPressed==0 かつ (now - lastDebounceTime > debounceDelay) のとき:
  - buttonPressed = 1
  - lastDebounceTime = now
  - rollTheDice() を1回呼ぶ
- ボタンがHIGHのとき:
  - buttonPressed = 0（次の押下を受け付ける）
```

---

### rollTheDice() — サイコロ動作処理

```
【処理の流れ】
1. animationStep=0にリセット
2. while(animationStep < 10) でアニメーション区間を実行（実行中は処理がブロッキングされる）
3. millis()で100msごとにランダムな数字を表示（10回）
4. 各回で短いブザー音を鳴らす
5. 最後に本当のサイコロ結果を表示し、長めのブザー音
6. 関数終了後、次のボタン入力待ちに戻る
```

---

### showNumber(x) — 数字を7セグLEDに表示

```
【処理の流れ】
1. xの値に応じてone()〜six()を呼ぶ
```

---

### one()〜six() — 各数字の7セグLED制御

```
【処理の流れ】
1. 各セグメントピンをON/OFFで制御し、数字を表示
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

```
【処理の流れ】
1. ボタンが押されたら、lastDebounceTimeと現在時刻を比較
2. 一定時間（debounceDelay=200ms）経過していれば有効な押下とする
3. ボタンが離されるまで次の入力を受け付けない
```

---

### 3-2. millis() を使ったタイマー管理

```
【処理の流れ】
1. ボタン判定とアニメーション間隔は millis() で管理
2. now - lastAnimationTime >= animationInterval なら次のアニメーションへ
3. 起動演出のみ delay(1000), delay(500) を使用
```

---

## 4. デバッグ出力計画（任意）

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | ボタン押下イベントの検出 | loop() | Serial.println("button pressed"); |
| 2 | デバウンス判定 | loop() | Serial.println(now - lastDebounceTime); |
| 3 | アニメーション進行 | rollTheDice() | Serial.println(animationStep); |

---

## 5. 単体テスト仕様書

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | loop() | ボタンを1回押す | サイコロが1回だけ振られる | | [ ] |
| 2 | loop() | ボタンを連打 | 1回ごとに1回だけ振られる | | [ ] |
| 3 | loop() | 前回押下から199msで再押下 | 再実行されない（デバウンスで無効） | | [ ] |
| 4 | loop() | 前回押下から200ms以上で再押下 | 再実行される | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | showNumber() | 1〜6を表示 | 7セグLEDが正しく点灯 | | [ ] |
| 2 | rollTheDice() | サイコロ動作を実行 | ブザー音が鳴動する | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | アニメーション中のボタン反応 | アニメーション中にボタンを押す | 追加実行されない | | [ ] |
| 2 | millis()タイマー精度 | アニメーション間隔を測定 | 設計通りの間隔 | | [ ] |

---

## 6. AIレビュー記録

### Q1: 実装上の問題確認

**AIの回答（要約）：**
- ピン定義および変数型に重大な問題はありません。
- millis() によるタイミング管理により応答性は確保されています。
- チャタリング対策は妥当です。

**対応した内容：**
- 変数名・型・ピン番号を再確認
- millis()で全体のタイミングを管理

---

### Q2: 単体テスト仕様の確認

**AIの回答（要約）：**
- 各関数に対するテスト観点は概ね網羅されています。
- 境界値（1,6）および連打時挙動の確認項目が含まれています。

**対応した内容：**
- テスト項目を追加し、網羅性を再確認

---

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | 状態遷移記述が実装コードと不一致（currentState未使用） | グループメンバー | currentState記述を削除し、入力判定フローへ修正 |
| 2 | 定数値の不一致（ブザー周波数） | グループメンバー | BEEP_FREQUENCYを4000に統一 |
| 3 | ボタン状態型の記述が曖昧 | グループメンバー | buttonPressedをintとして明記 |

### 7-2. レビューを受けて変更した点

- currentStateベース記述を削除し、実装コードの入力判定フローに合わせて更新
- 定数・型（BEEP_FREQUENCY=4000、buttonPressed=int）を実装コード準拠に修正

---

*初版: 2026-05-22 / AIレビュー: 2026-05-22 / グループレビュー後更新: 2026-05-22*
