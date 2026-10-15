# CurryHub / 独立プロジェクト化の調査と実装案

調査日: 2026-10-14

## 位置付け

`HANDOFF_CurryHub.md` の次の段階として、現行ソースを読み、実装単位と検証条件を整理した。
この文書の構成・形式・分担は技術的な提案であり、以前から合意済みの要件とは区別する。
調査時点ではソース変更・ビルド・起動検証を行っていなかった。実装の進捗は末尾に追記する。既存の DLL / obj の変更は保持する。

合意済みの到達点は、ZIP生成 → Hub取り込み → 環境準備 → 独立した作品作成 → C#編集・エディター内実行。
既存作品の移行、オンラインのエンジン配信、完成ゲームの書き出しは対象外。

## 現行コードで確認できたこと

| 箇所 | 確認結果と影響 |
| --- | --- |
| `Engine/main.cpp:12` | `WinMain` の `cmd_line` を使用していない。Hubが作品パスを渡しても作品選択には反映されない。 |
| `Engine/Core/EnginePaths.h` | Assets・設定・シェーダー・アイコンがすべて相対パス。共有資源と作品の保存先が混在している。 |
| `Engine/ProjectSettings.cpp:6` | exe横の `projectSettings.json` を読み、既定値をexeの2階層上の開発ツリーから組み立てる。設定読み込み失敗時の戻り値をScriptSystem側で確認していない。 |
| `Engine/Scripting/ScriptSystem.cpp:111` | 設定を読むが、`scriptOutputPath` をロード先に渡していない。Reload時のDLL引数は空文字。 |
| `CurryEngine.Runtime/EngineRuntime.cs:20` | exeと作業ディレクトリが一致すると、exeの場所にさらに `x64/Debug` を付加する。現行Hubの起動方法と衝突し得る。 |
| `CurryEngine.Runtime/EngineRuntime.cs:145` | 初回と再読み込みで作品DLLをexe側から探す。別作品のDLLを選ぶ経路がない。 |
| `CurryEngine.Runtime/EngineBootstrap.cs` | API DLLをexe側から登録する。Core / Runtime / APIの共有アセンブリは、作品DLLと区別して保持する必要がある。 |
| `UserScripts/Assembly-CSharp.csproj` | net9.0。Core / APIのソースプロジェクトを参照し、出力を常にエンジンの `x64/Debug` へコピーする。 |
| `Engine/Automation/SolutionGenerator.cpp` | 定義とビルド対象への登録はあるが、Engine内検索では呼び出し元が見つからない。生成内容はnet8.0、Runtimeのみ参照。既存net9.0構成との整合、Compile項目の重複回避、出力場所、ソリューションのBuild設定を修正する必要がある。 |
| `Engine/Scripting/ScriptWatcher.cpp` | `dotnet build` を実行し、成功後にビルドスレッドから直接Reloadする。起動時ビルド・メインスレッドへの反映順序も確認対象。 |
| `Engine/Core/EnginePaths.h:26`, `Engine/Core/ScriptComponent.cpp:147` | VSで開くソリューションは `CurryEngine.sln`、スクリプト検索は `UserScripts` に固定。 |
| `Engine/Editor/AssetBrowser.cpp:44` | 表示側はAssetsだがAssetDatabase初期化は `TestAssets`。新規作品では同じAssetsを使うよう揃える。 |
| `Engine/Resources/Shader.cpp:118` | 実行中にCSOをShadersDataDirへ保存する。パスを単にエンジン側へ向けると共有インストールへの書き込みが発生する。 |
| `Engine/Scenes/SceneManager.cpp:539` | `Assets/Scenes` を存在確認せず列挙する。空作品の初期構成と欠損時の扱いが必要。 |
| `CurryEngine.vcxproj`, `Engine/Core/framework.cpp`, `Engine/Scenes/Scene.cpp` | エディター・ホットリロード・テキストシーン読み込み等が `_DEBUG` / `USE_IMGUI` に結び付いている。現行Releaseをそのまま制作用ZIPにはできない。 |
| `build.bat`, `pack.bat` | ゲーム名でexeをリネームし、作品用copyItemsを収集する。制作用パッケージ生成は別の入口を設ける。 |
| Hub `Services/ProjectManager.cs` | Hubの登録情報からバージョンを照合し、exeの場所を作業ディレクトリとして起動する。作品ファイルを正本として起動直前に照合する変更が必要。 |
| Hub `Services/ProjectGenerator.cs` | フォルダとJSONの生成のみ。C#プロジェクト、初期シーン、エンジン固有の初期設定は生成しない。 |

Hubの調査対象: `C:/Users/2240001/Desktop/CurryHub/CurryHub/`。Hubのファイルは変更していない。

## 提案する構成

作品に共有エンジンのソースやexeを複製せず、次のように分離する。

```text
Engines/<engineVersion>/
  engine.json                     # パッケージ形式・バージョン・動作要件
  version.txt                     # 当面の既存Hub互換用。engine.jsonと同じ情報から生成
  CurryEngine.exe
  CurryEngine.Core.dll
  CurryEngine.API.dll
  CurryEngine.Runtime.dll
  CurryEngine.Runtime.deps.json
  CurryEngine.Runtime.runtimeconfig.json
  <必要なネイティブDLL>
  Resources/                      # 組み込みシェーダー、アイコン、エディター用フォント
  Templates/Empty/                 # 初期作品用の最小データ

Projects/MyGame/
  MyGame.curryproject              # formatVersion / name / engineVersion
  MyGame.sln                      # 生成可能
  Assembly-CSharp.csproj          # 生成可能。配布済みDLLを参照
  Assets/
    Scripts/
    Scenes/Main.scene
  ProjectSettings/
  Library/
    ScriptAssemblies/             # Assembly-CSharp.dll / pdb
    obj/
    Artifacts/
    ShaderCache/
  Temp/
  Logs/
```

絶対インストールパスは作品の正本に保存せず、生成ファイルの参照にのみ使う。
Hubが作品のengineVersionからインストール先を解決し、生成ファイルを更新する。
Assets・ProjectSettings・作品ファイルは保持し、Library以下やソリューションの再生成で上書きしない。
標準C#配置は `Assets/Scripts` とし、コンパイル対象は `Assets/**/*.cs` に限定する案。

## 起動と生成の分担案

Hubはインストール一覧・作品一覧・バージョン選択・環境準備を担当する。
エンジンは自分のバージョンに対応する作品形式、テンプレート、C#生成処理を持つ。
これによりHubが各エンジンの内部仕様を重複実装せずに済む。

コマンド構成（`--create-project` は未実装、その他は末尾の進捗を参照）:

```text
CurryEngine.exe --create-project "D:\Projects\MyGame" --name "MyGame"
CurryEngine.exe --generate-solution "D:\Projects\MyGame\MyGame.curryproject"
CurryEngine.exe --project "D:\Projects\MyGame\MyGame.curryproject"
```

- 作成・生成コマンドはウィンドウや描画・.NETホストを起動する前に処理し、終了コードとエラー内容を返す。
- 作品作成は既存の非空ディレクトリを上書きせず、一時ディレクトリで準備して成功後に確定する。
- 起動引数はUnicodeで解析し、作品ファイル・形式・バージョンをFrameworkの構築前に検証する。
- `engineRoot` はexeから、`projectRoot` は作品ファイルから一度だけ確定する。パスに空白・日本語がある場合も扱う。
- 初期段階では作業ディレクトリをprojectRootに固定し、既存の作品相対パスを維持する。共有資源へのアクセスはengineRootを明示する。
- 開発ツリーの引数なし起動は明示的な互換経路として残す。指定作品が不正な場合に既存作品へ黙ってフォールバックしない。
- Core / Runtime / APIはエンジンから共有し、作品DLLの絶対パスはC++からC#初期化・再読み込みへ一貫して渡す。
- 初回DLL未生成でもビルド後にロードできるようにし、ビルド失敗時は前回成功分を維持する。Reloadはメインスレッドの安全なタイミングに移す。
- 組み込み資源の読み込み先と書き込み先を分ける。組み込みシェーダーはエンジンから読み、再コンパイル結果は作品のLibraryへ保存する。
- 保存されるアセット参照にエンジンの絶対パスを埋め込まない。作品相対参照と組み込み資源の識別方法を、シリアライズ処理と一緒に整理する。

## 実装順序と完了条件

### 1. 独立した一作品を開いてC#を実行する

変更対象はmain / EnginePaths / ProjectSettings / SolutionGenerator / ScriptSystem / ScriptHost / EngineRuntime / EngineBootstrap / VS起動・アセット参照処理。
必要最小限のテンプレートを作り、Title・DemoGame0など既存作品のシーンやスクリプトを流用しない。
既定のUI画像・ゲーム用フォントなど、作品に含める初期資源と共有するエディター資源もこの段階で分ける。

完了条件:

1. エンジンの外の空ディレクトリに作品とC#ソリューションを生成できる。
2. 任意の作業ディレクトリから明示した作品を開ける。
3. VSで編集 → ビルド → エディター再生で変更が反映される。
4. 同じエンジンを使う2作品のDLL・アセット・設定・キャッシュが混ざらない。
5. 利用中にエンジンのインストール先へ書き込まない。
6. 不正な作品パス、欠損設定、C#エラー、初回DLLなしを適切に処理できる。
7. 開発ツリーの既存Debug起動も維持する。

### 2. 配布用エディターとZIP生成

`CURRY_EDITOR` 等の用途フラグと最適化・診断フラグを分け、配布用Editor構成を追加する案。
`_DEBUG` の一括置換はせず、シーン形式・再生操作・監視・HLSL処理・描画診断を個別に分類する。
外部ライブラリの構成名も分ける。現在は `$(Configuration)` 参照が多く、新しい構成名だけ追加してもリンク先が成立しない。
DirectXTexのライブラリパスは両構成でDebug固定なので、配布用の依存解決時に確認する。

配布候補はexe、Core/API/Runtime、RuntimeのJSON、nethost、assimp、実際の依存解析で必要となったDLL、組み込み資源、テンプレート。
これは確定済みの配布一覧ではない。生成物の依存解析とクリーン環境での起動で確定する。
開発ツリー全体のAssetsやDataを一括コピーせず、必要ファイルの明示リストで収集する。
既存のゲーム向けbuild/packとは別の生成スクリプトを設け、必須ファイル欠損は失敗にする。
engine.jsonとversion.txtを同じバージョン指定から生成し、作成途中のZIPを完成品として扱わない。

完了条件: 新しい出力先に1コマンドでZIPを作れ、解凍したエンジンで段階1の作品を開いて編集・再生できる。

### 3. HubのZIP取り込み・作品作成・起動

ZIPを一時展開し、展開先外へのパスや不正な内容を拒否、必須ファイルとmanifestを検証してから登録する。
同一バージョンの無断上書きを避け、異なるバージョンは共存させる。
Hubのモック生成処理を上記エンジンコマンド呼び出しへ変更する。
作品のバージョン変更は明示操作とし、新エンジンの取り込みだけでは作品ファイルを更新しない。

完了条件: ZIP取り込み → 新規作成 → 指定版で起動が通り、旧版の作品が変更されない。

### 4. 初回セットアップ

現行コードの対象はエンジン側net9.0、Hub側net8.0-windows。
これは現状記録であり、配布用に採用するSDK/VSのバージョンが決定済みという意味ではない。
global.jsonのSDK指定と実際に採用するSDKを合わせ、SDK・ランタイム・VS対応状況は実装時のMicrosoft公式情報で確認する。
VS検索は現在 `[17,18)` 固定なので、環境チェックとエディター側の起動対象を一致させる。
Hub自体を開発ツール未導入PCで起動できる配布方式も決める必要がある。self-contained配布を候補とする。

Hubで不足要件を表示 → 公式配布元のインストーラー起動を案内 → インストール後に再確認、という流れを実装する。
完全無人インストールは前提にしない。C#だけで制作する利用者の必要要件と、エンジン自体をビルドする開発者の要件を分ける。

完了条件: 開発ツールのない検証用Windows環境で、Hubの案内に従って段階3とC#編集・実行まで到達できる。

## 次に着手する単位

段階1のパス確定、共有資源の分離、C#生成・読み込みの接続まで実装済み。
次は最小テンプレートと作品生成を繋ぎ、実エディターで独立作品の一連の動作を検証する。
ZIPやHubだけを先に完成扱いせず、この一連の動作を最初の成立条件とする。

## 実装進捗（2026-10-14）

- 作品形式・エンジンmanifest・バージョン一致・独立した保存場所を検証するProjectContextを追加。
- `--validate-project` をmainの最初に接続。描画や.NETを起動せず、終了コードとUTF-8 JSONを返す。
- 引数なしの開発用起動は従来の作業ディレクトリを維持し、engineRoot/projectRootを保持する。
- `--project` をDebugエディターへ接続し、`--generate-solution` を追加。既存Hubの裸のパス1個は従来どおり無視する互換経路を維持する。この経路は作品選択・バージョン照合を行わない。
- 共有資源はexe側Resourcesから読み、作品DLL・中間生成物・シェーダーキャッシュは作品側Libraryへ保存。組み込みシェーダーの参照にインストール絶対パスを保存しない。
- C#生成はnet9.0、Assets配下の明示コンパイル、配布DLL参照、共有DLLコピーなし。作品slnをVSで開く導線も接続。
- 初回DLL未生成時の待機、ビルド成功後のメインスレッドReload、失敗時の前回成功分の維持、他作品DLL拒否を実装。初期化とReloadのABI変更に伴いexeとマネージドDLLの同時再ビルドが必要。
- 自動検証はProjectContext、起動プローブ、変更したC++実装のコンパイル、実C#ランタイムによる作品ビルドと再読み込み。描画・VS操作・エディター内再生・全体リンクは未確認。
- 検証仕様・使い方は `docs/ProjectStartup.md`。段階1全体の完了条件はまだ未達。
