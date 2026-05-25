detailed_design_template# 詳細設計書 — 組込み開発実習

<!-- 作成者: あなたの名前 / 日付: YYYY-MM-DD / グループ: 〇-〇 -->

---

## 0. 基本設計書との接続確認

| 項目 | basic_design.md から転記 |
|:--|:--|
|簡易アラーム装置 | |
| 待機状態 / ブザー動作中 | |
| 実装する関数の数　 | 　7個 |
| グローバル変数の合計バイト数 | 約11B |

---

## 1. グローバル変数・定数の設計

>
```
【ピン定義】（basic_design.md 3-1 から転記）
PIN_BUTTON    = 2    // タクトスイッチ（INPUT_PULLUP）
PIN_BUZZER = 8      // アクティブブザー

【状態管理】
currentState : int = 0
// 0:待機状態
// 1:ブザー動作中

【タイマー（millis()用）】
lastButtonTime : unsigned long = 0
DEBOUNCE_DELAY : const int = 50



【センサー・入力値】（basic_design.md 2-1 から転記）
lastSerialTime : unsigned long = 0
SERIAL_INTERVAL : const int = 100

【その他のフラグ・カウンター】
  （自分のものを追加）
```
lastBuzzerTime : unsigned long = 0
buzzerInterval : int = 500
buzzerToggle : bool = false
---

## 2. 各関数の詳細設計

---

### `setup()` — 初期化処理

```
【処理の流れ】
1. ボタンピンを INPUT_PULLUP に設定する
- pinMode(PIN_BUTTON, INPUT_PULLUP)

22. ブザーピンを OUTPUT に設定する
   - pinMode(PIN_BUZZER, OUTPUT)

3. ブザーを停止状態にする
   - digitalWrite(PIN_BUZZER, LOW)

4. シリアル通信を開始する
   - Serial.begin(9600)

5. currentState を待機状態（0）に設定する
```

```
【処理の流れ】
1. ボタンピンを INPUT_PULLUP に設定する
- pinMode(PIN_BUTTON, INPUT_PULLUP)

2. ブザーピンを OUTPUT に設定する
   - pinMode(PIN_BUZZER, OUTPUT)

3. ブザーを停止状態にする
   - digitalWrite(PIN_BUZZER, LOW)

4. シリアル通信を開始する
   - Serial.begin(9600)

5. currentState を待機状態（0）に設定する
```

---

### `loop()` — メインループ

```
【処理の流れ】

＜毎ループ実行すること＞
- readButton() を呼び出してボタン状態を取得
- serialMonitor() を呼び出す

＜currentState が 0（待機状態）のとき＞
- ボタンが押されたか確認する
- 押された場合
  → currentState = 1
  → doAlarm() を実行

＜currentState が 1（ブザー動作中）のとき＞
- controlBuzzer() を実行
- ボタンが離された場合
  → ブザー停止
  → currentState = 0

＜追加機能を使う場合＞
- changePattern() を実行してブザー間隔を変更する
```

```

【処理の流れ】

＜毎ループ実行すること＞
- readButton() を呼び出してボタン状態を取得
- serialMonitor() を呼び出す

＜currentState が 0（待機状態）のとき＞
- ボタンが押されたか確認する
- 押された場合
  → currentState = 1
  → doAlarm() を実行

＜currentState が 1（ブザー動作中）のとき＞
- controlBuzzer() を実行
- ボタンが離された場合
  → ブザー停止
  → currentState = 0

＜追加機能を使う場合＞
- changePattern() を実行してブザー間隔を変更する
```

---

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。

---

### `関数名()` 

**basic_design.md 2-2 との対応：** （基本設計書の関数一覧の説明を転記）

**引数：** :なし
**戻り値：** bool
```
【処理の流れ】

1. digitalRead(PIN_BUTTON) を実行する

2. 前回入力時刻との差を確認する
   - millis() - lastButtonTime を計算

3. 50ms未満なら入力を無視する

4. 50ms以上なら入力を確定する

5. ボタン状態を返す

【エラー・異常ケース】
- ボタン未接続時:
  HIGH固定状態として扱う

----------------------------------------------------------------

controlBuzzer() — ブザー制御

basic_design.md 2-2 との対応：
ブザーをON/OFFする

引数： bool buttonState

戻り値： void

【処理の流れ】

1. buttonState が true か確認する

2. true の場合
   - digitalWrite(PIN_BUZZER, HIGH)

3. false の場合
   - digitalWrite(PIN_BUZZER, LOW)

【エラー・異常ケース】
- 異常入力時:
  ブザーを停止する

----------------------------------------------------------------

doAlarm() — ブザーを鳴らす

basic_design.md 2-2 との対応：
ボタン押下時にブザーを鳴らす

引数： なし

戻り値： void

【処理の流れ】

1. ブザー出力を HIGH にする

2. ブザー動作状態へ移行する

【エラー・異常ケース】
- 状態異常時:
  currentState を待機状態へ戻す

  serialMonitor() — シリアル表示

---------------------------------------------------------------

basic_design.md 2-2 との対応：
ボタン状態を表示する

引数： なし

戻り値： void

【処理の流れ】

1. millis() を取得する

2. 前回表示から100ms以上経過したか確認する

3. 経過している場合
   - ボタン状態を Serial.println() で表示する
   - currentState も表示する

4. 表示時刻を更新する

【エラー・異常ケース】
- シリアル未接続時:
  処理継続

----------------------------------------------------------------

changePattern() — ブザー間隔変更

basic_design.md 2-2 との対応：
ブザー間隔を変更する

引数： なし

戻り値： void

【処理の流れ】

1. millis() を取得する

2. 現在時刻 - lastBuzzerTime >= buzzerInterval を確認する

3. 条件成立時
   - buzzerToggle を反転する
   - ブザーON/OFFを切り替える

4. lastBuzzerTime を更新する

【エラー・異常ケース】
- 間隔値が0以下:
  初期値500msへ戻す

```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
ボタン入力後50ms以内の連続入力は無効とする。

【処理の流れ】

1. ボタン入力を取得する

2. 前回入力時刻との差を計算する

3. 50ms未満の場合
   - 入力を無視する

4. 50ms以上の場合
   - 入力を確定する

5. lastButtonTime を更新する
【必要な変数（Section 1 に追加済みか確認）】
  lastDebounceTime : unsigned long   // 前回確定した時刻
  DEBOUNCE_DELAY   : const int = 50  // チャタリング判定時間（ms）
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
delay() を使わずに一定周期処理を行う。

【処理の流れ】

1. now = millis() を取得

2. now - lastSerialTime >= SERIAL_INTERVAL を確認

3. 条件成立時
   - シリアル表示を実行

4. lastSerialTime = now に更新

【自分のシステムで millis() を使う処理】

- シリアルモニタ更新
- ブザー点滅制御
- チャタリング対策
【自分のシステムで millis() を使う処理】
  （basic_design.md 2-3 のタイミング設計から転記して具体化する）
```

---

### 3-3. その他の重要ロジック（任意）


```
【処理の流れ】
1.
2.
3.

【入力値と出力値の関係】

```

---

## 4. デバッグ出力計画（任意）


| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | ボタン入力取得確認 | `readButton()` | `button pushed;` |
| 2 | 状態遷移確認 | `loop()` | `currentState;` |
| 3 | ブザー制御確認 | `controlBuzzer()` | `buzzer ON";` |
| 4 | チャタリング確認 | `readButton()` | `debounce OK` |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readButton() | ボタンを1回押す | true を返す | [] |
| 2 | readButton() | 素早く連打する | 誤入力しない | [] |
| 3 | readSensor() | ボタンを離す | false を返す | [] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | controlBuzzer() | buttonState=true | ブザーON | [] |
| 2 | controlBuzzer() | buttonState=false | ブザーOFF | [] |
| 3 | changePattern() | 長押し状態 | 一定間隔でON/OFF | [] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | delay停止確認 | ボタン連打 | 応答が止まらない |  | [] |
| 2 | millis精度確認 | 500ms周期確認 | 正常周期動作 | [] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
millis() を使用しているため応答性は良好
INPUT_PULLUP使用時は「押された=LOW」になるため条件式に注意
currentState を enum 化すると可読性向上

**対応した内容：**

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**
ボタン長押し時のテストを追加するとよい
ブザー停止確認テストを追加するとより安全

**対応した内容：**

---

## 7. グループレビュー記録

AIに細かく解説してもらったりして、理解しようという姿勢が見えた。

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | ところどころ500msと表記があるが、コードが分からないので、しっかり確認したほうがいい | 飯塚さん |  |
| 2 |  |  |  |
| 3 |  |  |  |

### 7-2. レビューを受けて変更した点

-　
-

---

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
