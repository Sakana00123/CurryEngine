# 独立作品の起動とC#生成

2026-10-14時点で、作品の検証、C#ソリューション生成、Debugエディターの `--project` 起動経路を実装した。
共有資源と作品の出力先を分離し、作品DLLの初回読み込み・再読み込みを接続済み。
初期作品テンプレートと `--create-project` は未実装。以下の自動検証は描画・VS操作・エディター再生を含まないため、制作フロー全体はまだ完了扱いにしない。

## コマンド

```powershell
& 'C:\Engines\0.1.0\CurryEngine.exe' --validate-project 'D:\Projects\My Game\MyGame.curryproject' > result.json
```

- 成功: 終了コード0、標準出力にUTF-8のJSONを1行出力。
- 失敗: 終了コード1、標準エラーに `valid: false` と `error` を出力。標準エラーが利用できない場合はダイアログで通知。
- ウィンドウ・描画・.NETホスト・アセット監視を初期化する前に終了する。
- 作品・エンジンのファイルを作成・修復・更新せず、作業ディレクトリも変更しない。
- 相対的な作品パスは、コマンドを起動した作業ディレクトリを基準にする。
- エンジンの場所は、起動したexeの場所から確定する。

結果例:

```json
{"valid":true,"engineRoot":"C:/Engines/0.1.0","projectRoot":"D:/Projects/My Game","projectFile":"D:/Projects/My Game/MyGame.curryproject","name":"My Game","engineVersion":"0.1.0"}
```

`valid` は以下の作品情報・パス・バージョン検証が成功したことを表す。アセット、C#、SDK、描画環境の動作保証は含まない。
Hubから利用する際は `UseShellExecute=false` と標準出力・標準エラーのリダイレクトを指定し、終了コードを確認する。

## 最小ファイル形式

作品側の `MyGame.curryproject`:

```json
{
  "formatVersion": 1,
  "name": "My Game",
  "engineVersion": "0.1.0"
}
```

exeと同じ場所の `engine.json`:

```json
{
  "formatVersion": 1,
  "version": "0.1.0"
}
```

formatVersionは整数の1。名前とバージョンは空でない文字列。バージョンは完全一致で照合する。
engine.jsonがない場合に照合を省略したり、別バージョンを自動選択したりしない。
エンジンのディレクトリと作品のディレクトリが同一、または一方がもう一方を含む配置も拒否する。
既存Hubのモック作品ファイルはformatVersionを持たないため、この新形式には未対応。

## C#生成とエディター起動

```powershell
& 'C:\Engines\0.1.0\CurryEngine.exe' --generate-solution 'D:\Projects\My Game\MyGame.curryproject'
& 'C:\Engines\0.1.0\CurryEngine.exe' --project 'D:\Projects\My Game\MyGame.curryproject'
```

`--generate-solution` は描画・.NETホストを起動せず、作品直下に `Assembly-CSharp.csproj` と作品ファイルと同じ名前の `.sln` を生成する。成功時は終了コード0と `{"generated":true}`。Core / Runtime / APIのDLLがexeの隣に必要。既存ファイルに生成マーカーがなければ上書きを拒否する。生成ファイルは起動時にも更新されるため、手動のカスタマイズ保存先にはしない。

生成するC#プロジェクトはnet9.0 / x64で、`Assets/**/*.cs` をコンパイルする。エンジンのソースプロジェクトには依存せず、exeの隣の配布DLLを参照する。共有DLLは作品へコピーしない。ビルドには.NET 9 SDKが必要。

`--project` は現在Debugエディター用。Releaseでは描画前に拒否する。起動時に次を確認する。

- 作品側: `Assets`、`Assets/Scenes`、`ProjectSettings` の各ディレクトリ。
- エンジン側: `Resources/Data/Icon`、`Resources/Data/Fonts`、`Resources/Shader`、`Resources/Assets/Shaders` の各ディレクトリ。

この存在確認はディレクトリ単位であり、個々の設定・シーン・資源ファイルの完全性はまだ保証しない。実際の起動には通常のネイティブ依存DLL、Runtimeのruntimeconfig/deps.json、実物のアイコン・フォント・HLSL・CSOも必要。空のディレクトリだけを用意してもエディター動作の確認にはならない。

検証・ソリューション生成後に作業ディレクトリを作品ルートへ変更し、通常のFrameworkを起動する。アセット表示・監視は作品の `Assets`、VS起動は作品の `.sln` を使う。初回DLLがなくてもC#ホストは待機し、起動時ビルド成功後に読み込む。監視スレッドは再読み込みを予約し、シーン遷移中を避けてメインスレッドで適用する。ビルド失敗やDLL読み込み失敗時は最後に成功した登録を維持する。

| 内容 | 場所 |
| --- | --- |
| 共有Core / Runtime / API | exeの隣 |
| 作品DLL / PDB | `Library/ScriptAssemblies` |
| C#中間生成物 | `Library/obj` |
| シェーダーのコンパイル結果 | `Library/ShaderCache` |
| アセットの中間生成物 | `Library/Artifacts` |
| 作品設定・シーン | `ProjectSettings` / `Assets/Scenes` |

組み込みシェーダーの保存用参照は `EngineResources/Shader/...` / `EngineResources/Shaders/...`。実ファイルの読み取り時だけ `Resources/Shader` / `Resources/Assets/Shaders` へ解決する。アイコン等の `Data/...` も共有Resourcesへ解決する。共有シェーダーのEditボタンは無効にし、作品側のシェーダーは `Assets` で編集する。既存コードの診断ログには作品ルートへ直接出るものがあり、すべてが `Logs` 配下に統一されたわけではない。

ネイティブとマネージドの初期化・Reload ABIを更新しているため、exeとCore / Runtime / APIは同じソース版で再ビルドする。既存バイナリにRuntimeだけ差し替えて使わない。

## 既存起動と次の実装

引数なし起動は、従来の作業ディレクトリ・アセット・スクリプトの扱いを維持する。
既存Hubが渡す裸のパス1個は、従来と同様に無視して既存の起動経路を維持する。この互換起動は作品選択・バージョン照合を行わない。
Hubを新しい起動経路へ接続するときに明示的な `--project` 引数へ切り替え、この互換経路を整理する。

次は既存ゲームに依存しない最小テンプレート・作品生成を実装し、実エディターで新規作品 → C#編集 → 再生の一連の動作を確認する。

## 検証

Visual StudioのC++ツールがある環境で実行:

```powershell
./Tests/Run-ProjectContextTests.ps1
./Tests/Compile-ProjectRuntime.ps1
./Tests/Run-ProjectRuntimeTests.ps1
```

エンジンの既存exe・DLLを上書きせず、`Library/Tests/ProjectContext` 内に独立した検証プログラムとテストデータを生成する。
テスト対象は、形式エラー・バージョン不一致・不足ファイル・ディレクトリの重なり・相対パス・日本語/空白/記号を含むパス・非破壊性・実際の起動引数とJSON出力・引数なし互換経路。
`Compile-ProjectRuntime.ps1` は変更した15個のC++実装を隔離した中間出力先でコンパイルする。プリビルドのコード生成やエンジン全体のリンクは実行しない。

`Run-ProjectRuntimeTests.ps1` は.NET 9 SDKも使用する。日本語・空白・`&` を含むエンジン/作品配置先で、生成したslnのビルド、初回DLLなし、型の共有、DLLロックなしの再ビルドと再読み込み、意図的なC#エラー時の維持、他作品のDLL拒否、壊れたDLL時の維持、クラス名変更後の古い登録削除を確認する。手書きcsprojの上書き拒否と、処理前後の共有エンジンのファイルハッシュ一致も確認する。

このテストは実際のC# Runtimeを使うが、ネイティブ側のログ・名前登録はスタブに置き換える。描画、ネイティブホスト経由の実行、ファイル監視のタイミング、VSのGUI操作、エディターのコンポーネント再生は別途検証が必要。
