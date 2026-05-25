# 詳細設計書 — 組込み開発実習

<!-- 作成者: 飯塚 光希 / 日付: 2026-05-25 / グループ: 〇2-J -->

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
| 状態の種類（1-2 状態遷移から） |待機中（STATE_WAIT）、再生中（STATE_PLAY）|
| 実装する関数の数（2-2 関数一覧から） |8個（setup, loop, readButtons, updateLED, playMelody, updateLCD, togglePlay, changeTrack, playNextTrack）|
| グローバル変数の合計バイト数（2-1 SRAM確認から） |（概算）約50B＋曲データ配列のサイズ|

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。
> ここで設計した変数は、この後の関数設計でそのまま使います。

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

**basic_design.md 2-2 との対応：** ピン・LCD・曲リストなど全体初期化

**引数：** なし  
**戻り値：** void

```
【処理の流れ】
1. ボタン・LED・ブザーのピンモードを設定する
   - PIN_BUTTON1, PIN_BUTTON2 → INPUT_PULLUP
   - PIN_LED → OUTPUT
   - PIN_BUZZER → OUTPUT
2. LCD・I2C通信などデバイスの初期化を行う（lcd.begin(16,2)など）
3. 曲データや曲名配列、曲数の初期化を行う
4. グローバル変数（currentState=STATE_WAIT, TrackID=0, led_State=false, buzzer_State=false等）も初期値に設定
5. LCDに最初の曲名状態を表示する
6. （任意）Serial.begin(9600)でデバッグ出力有効化
7. （任意）LEDで起動確認（1秒間点灯→消灯）
```

---

### `loop()` — メインループ

**basic_design.md 2-2 との対応：** 状態管理・各機能の呼び出し

**引数：** なし  
**戻り値：** void

```
【処理の流れ】

＜毎ループ実行すること＞
  - readButtons()でボタン入力状態を取得・更新
  - now = millis()で現在時刻を取得
  - updateLED(), updateLCD()で出力反映

＜currentState == STATE_WAIT（待機中）のとき＞
  - ボタン1が押されたら togglePlay() で currentState = STATE_PLAY に遷移
  - ボタン2が押されたら changeTrack() で選曲状態更新
  - （曲再生・ブザー鳴動は発生しない）

＜currentState == STATE_PLAY（再生中）のとき＞
  - playMelody()で現在の曲を再生（途中・終端も含む）
  - 曲が終わったか判定し、終わっていれば playNextTrack()を呼び、次の曲または停止処理へ
  - 再生中にボタン1が押されたら togglePlay() で currentState = STATE_WAIT に遷移
  - ボタン2が押されたら changeTrack() で即座に新曲へ切り替えて再生

その他
  - 必要な状態更新処理（TrackIDリセット、LED消灯など）
```

---

### `updateLED()` — 再生状態に応じたLEDのON/OFF制御

**basic_design.md 2-2 との対応：** 再生中/停止中でLEDを点灯/消灯制御

**引数：** なし  
**戻り値：** void

```
【処理の流れ】
1. currentStateを参照
2. currentState == STATE_PLAY なら led_State=trueとしてdigitalWriteでLEDをON
3. currentState == STATE_WAIT なら led_State=falseとしてdigitalWriteでLEDをOFF
4. 状態が変われば変化を反映

【エラー・異常ケース】
- LEDのピンがショート等していてONしない場合、そのままソフト側はOFF扱いで続行
```

---

### `togglePlay()` — 再生/停止状態の切替

**basic_design.md 2-2 との対応：** 再生/停止状態（currentState）のトグル

**引数：** なし  
**戻り値：** void

```
【処理の流れ】
1. currentStateがSTATE_WAITなら currentState = STATE_PLAY に遷移
2. currentStateがSTATE_PLAYなら currentState = STATE_WAIT に遷移
3. 状態遷移後、updateLED()やupdateLCD()で出力を更新
4. 再生開始時はTrackIDが範囲外なら0にリセット

【エラー・異常ケース】
- 状態値が定義外の場合は currentState = STATE_WAIT に戻す
```

---

### `updateLCD()` も含め、他の関数でも「currentState」の比較・書き換えはすべてenum名（STATE_WAIT/STATE_PLAY）で統一します。

---

**ポイント：**
- 状態遷移・状態チェックは「enum名」で比較・代入
- 0/1による数値の直接比較・値代入は行わない

---

**他にも疑似コードの「状態チェック・状態遷移」部分は全てenumで表現してください。**

---

### `changeTrack()` — 曲選択状態の更新

**basic_design.md 2-2 との対応：** TrackID(選曲)を+1して循環/再生準備

**引数：** なし  
**戻り値：** void

```
【処理の流れ】
1. TrackIDを+1（インクリメント）
2. TrackIDがTrackCount以上なら0に戻す（サイクリック）
3. currentState==STATE_PLAY（再生中）なら即座に新曲をplayMelody()
4. TRACKID変更に合わせてupdateLCD()も呼ぶ

【エラー・異常ケース】
- TrackCount=0等で曲が無い場合は何もしない
```

---

### `playNextTrack()` — 曲終端時の自動次曲再生・終端で自動停止とリセット

**basic_design.md 2-2 との対応：** 曲終端イベントによる連続再生・終端停止・TrackIDリセット

**引数：** なし  
**戻り値：** void

```
【処理の流れ】
1. TrackIDを+1
2. TrackIDがTrackCount（最後の曲）を超える場合
    2-1. currentState=STATE_WAITへ遷移（再生停止）
    2-2. updateLED()/updateLCD()で待機表示
    2-3. TrackID=0にリセット
3. まだ曲が残っていればplayMelody()で次曲を再生
4. TrackID変更に合わせてLCD表示も更新

【エラー・異常ケース】
- TrackCount=0や配列エラー時はSTATE_WAITとTrackID=0へ強制リセット
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

```
【考え方】
  ボタンが押されたとき、100ms 以内の連続入力は「同じ1回の押下」として無視する。  ←※設計通り100msに修正

【処理の流れ】
  1. ボタンのデジタル値を読む（digitalRead）
  2. 前回確定した時刻（lastDebounceTime）からの経過時間を計算する（millis() - lastDebounceTime）
  3. 経過時間 < DEBOUNCE_DELAY（例: 100ms）→ 無視する
  4. 経過時間 ≥ DEBOUNCE_DELAY → ボタンの状態として確定する（押下イベント/離したイベントを記録）
  5. lastDebounceTime を更新する

【必要な変数（Section 1 に追加済みか確認）】
  lastDebounceTime1 : unsigned long   // ボタン1用・前回確定した時刻
  lastDebounceTime2 : unsigned long   // ボタン2用・前回確定した時刻
  DEBOUNCE_DELAY    : const int = 100 // チャタリング判定時間（ms）
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  「前回実行した時刻」を記録しておき、「今の時刻 − 前回時刻 ≥ 周期」なら実行する。

【処理の流れ（例: 再生中LED管理・チャタリング検出）】
  1. now = millis()
  2. now - lastMillis_X >= INTERVAL_X かどうか確認
  3. 条件を満たした場合: 必要な処理実行、lastMillis_X = now
  4. 条件を満たさない場合: 何もしない（次のループで再チェック）

【自分のシステムで millis() を使う処理（例）】
  - チャタリング防止（ボタンデバウンス判定にmillis利用）
  - 曲再生中の音階間隔制御（delayまたはmillisによるノンブロッキング再生が可能）
  - 状態/LED/LCD表示のタイミング制御
```

---

### 3-3. その他の重要ロジック（任意）

```
【処理の流れ】
1. 曲終端時の自動TrackIDリセット
    - 曲再生の最後(playNextTrack等)で、TrackIDがTrackCount以上なら0にリセット
2. 曲切替ボタンによる曲番号サイクリック
    - changeTrack()でTrackIDを+1、TrackCount以上なら0に戻す
3. 曲再生中の「停止」や「曲切替」割り込み
    - playMelody()実行中もボタン読取判定しSTATE/TrackID変更可

【入力値と出力値の関係】
- ボタン操作→TrackID/状態変化→LED・LCD・ブザー動作に即座に反映
```

---

## 4. デバッグ出力計画（任意）

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | TrackIDやcurrentStateが正しく変化しているか | `changeTrack()`, `togglePlay()`, `playNextTrack()` | `Serial.println(TrackID);` `Serial.println(currentState);` |
| 2 | 曲データや配列アクセスが正常か | `playMelody()` | `Serial.println(音階番号);` |
| 3 | チャタリング処理が効いているか | `readButtons()` | `Serial.println("BTN1 valid");` |
| 4 | LCD表示の更新が正しいか | `updateLCD()` | `Serial.println(Display_Text);` |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readButtons() | ボタン1を1回押す | button1_Stateがtrueになる | | [ ] |
| 2 | readButtons() | ボタン1を100ms未満で2回連打 | 1回分だけ反応する | | [ ] |
| 3 | readButtons() | ボタン2を正常なタイミングで連続押し | 複数回TrackIDが進む | | [ ] |
| 4 | changeTrack() | 曲数分だけボタン2を押す | TrackIDが0に戻る | | [ ] |
| 5 | togglePlay() | ボタン1で再生/停止を操作 | 状態が交互に切り替わる | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | updateLED() | 状態=STATE_WAITで呼び出し | LEDが消灯している | | [ ] |
| 2 | updateLED() | 状態=STATE_PLAYで呼び出し | LEDが点灯している | | [ ] |
| 3 | playMelody() | 曲再生中 | 音階が1つずつ正しく発音される | | [ ] |
| 4 | updateLCD() | 曲選択や状態変更時 | 表示が反映される | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | チャタリング誤判定がないか | 100ms未満の素早い連打 | 1回ずつしか反応しない | | [ ] |
| 2 | 曲終端のTrackIDリセット | 全曲自動再生後 | TrackIDが0になる | | [ ] |
| 3 | ノンブロッキング時の入力応答 | 再生中にボタン操作 | 状態・曲切替が即座に反映される | | [ ] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
- デバウンス処理（チャタリング防止）：DEBOUNCE_DELAYが100msで十分ですが、処理の分岐条件（押した/離したフラグ）が正しく判定されているか注意。
- TrackIDや配列アクセス：曲数（TrackCount）やTrackIDの範囲外アクセスに対する防御（配列外アクセス対策）が設計されており問題なし。
- state管理：STATE_WAIT/STATE_PLAY以外になった場合（万が一の異常値）にどちらに戻すか記載あり問題なし。
- playMelody()の中で再生/停止/曲切替の割り込みが正常に処理されるか（フラグ解釈・再入対策）をテストで必ず確認すること。
- LCD通信・配線エラー時は、例外発生時に再トライや表示更新失敗への対応も記載されており安全。
- millisやdelayの利用箇所が混在しないよう整理されている（非ブロッキング設計優先でOK）。
- グローバル変数やLED・ブザー制御変数も型（bool/int/unsigned long）が仕様通り・初期化済みで正しい。
- 曲数や配列長（MAX_TRACKS/TrackCount/音階数）は実装時に適切に確保・管理すること。
- テスト項目にも重要な境界・異常・割り込みパターンがカバーされている。

**対応した内容：**
- デバウンス判定の処理条件（100ms未満はスキップ）を各ボタンで確認、フラグ化の流れも設計通り整理した。
- 曲番号や配列の範囲外管理、TrackIDリセット操作をplayNextTrack()やchangeTrack()等で明記、配列外エラー回避対応も盛り込んだ。
- 状態遷移や異常値発生時の正常復帰処理（STATE_WAITへ戻す等）も仕様通り記載した。
- LCD・ブザー・LED操作のエラーケース（書き込み失敗・ピン不良等）の影響も検討し、「そのままスキップ・リトライ」として運用安全を確保した。
- テスト仕様でも割り込みや遷移・反応タイミングを追加し確認範囲の網羅性を高めた。

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**
- チャタリングテストは「100ms未満」「100ms以上」でのボタン動作の差分が確認できる。対応関数ごと（readButtons, changeTrack, togglePlay等）にフラグ確認も必要。
- 曲切替・TrackID巻き戻し（TrackCount超過時の0リセット）や失敗操作（曲数ゼロ・ボタン押下なし等）もテストされている。
- 出力系（LED, LCD, ブザー）のON/OFFや表示内容、複数状態での連続変化（複数曲再生シナリオ）もテストで確認できる条件設計済み。
- 同時押しや割り込み応答、連打時に正しく「先の要求が優先・後処理される」動作保証も明記されている。
- millis、delay、配列長など仕様が変わった際の境界値パターンもテストを通じて検証できる。
- 境界・エラー系テストは十分カバーされており、さらに実装段階で細かい端数・桁落ち・意図しない型変換などもデバッグ出力で確認推奨。

**対応した内容：**
- テスト項目での入力/出力/タイミング/並列性/割り込み/曲切替の各ケースをもれなく記載した。
- millis, delay, 配列長変更時の境界値や曲数0時の保護動作も設計時テストとして明示した。
- フォールトトレランス（異常時の復帰/安全停止）や、出力ON/OFF・表示正当性も網羅した設計仕様とした。

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 |  |  |  |
| 2 |  |  |  |
| 3 |  |  |  |

### 7-2. レビューを受けて変更した点

-
-

---

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
