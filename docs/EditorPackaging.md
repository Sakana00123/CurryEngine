# 制作用エンジンの自動パッケージ作成

CurryEngine直下の `build_editor_package.bat` をダブルクリックすると、バージョン `0.1.0-dev.1` で作成します。終了時は結果を確認できるようキー入力を待ちます。PowerShell 7（`pwsh.exe`）がPATHに必要です。

バージョンを指定する場合は、CurryEngine直下で次を実行します。

```powershell
.\build_editor_package.bat 0.1.0-dev.2
```

PowerShellスクリプトを直接実行することもできます。実行場所はCurryEngineルートです。

```powershell
pwsh -File ./Tools/Packaging/Build-EditorPackage.ps1 -Version 0.1.0-dev.1
```

バージョンを変更して繰り返し実行できます。同じ出力先に同じバージョンのフォルダ・ZIP・チェックサムがある場合は、上書きせず終了します。

```powershell
pwsh -File ./Tools/Packaging/Build-EditorPackage.ps1 -Version 0.1.0-dev.2 -OutputDirectory 'D:/EnginePackages' -BuildWorkers 4
```

## 自動化する処理

1. MetaParserをビルドし、その時点のEngineソースからC++／C#の連携コードを新規生成。
2. Debug x64エディターとシェーダー、ReleaseのCore／Runtime／APIをビルド。
3. exe、マネージドDLL、RuntimeのJSON、assimp／nethost、共有シェーダー・画像・フォント、Templatesフォルダ全体を収集。Templates内のファイルとサブフォルダは個別指定せず、そのままコピーします。
4. ネイティブの依存DLLを調べ、パッケージ・OS・明示したDebugランタイム以外の不足依存を検出。
5. 指定したバージョンから `engine.json` と `version.txt` を生成し、ファイル一覧とSHA256を `package-files.json` に記録。
6. ZIPを作成して別の日本語・空白を含む場所へ展開し、内容のハッシュを照合。
7. 展開したエンジンで新規プロジェクト作成、検証、ソリューション生成、サンプルC#のビルドを実行。共有エンジンが変更されていないことも照合。
8. 検証が成功した場合に完成品を出力。

```text
BuildOutput/Engines/
  CurryEngine-0.1.0-dev.1-win-x64/
    CurryEngine.exe
    engine.json
    version.txt
    package-files.json
    README.txt
    CurryEngine.Core.dll
    CurryEngine.Runtime.dll
    CurryEngine.API.dll
    CurryEngine.Runtime.deps.json
    CurryEngine.Runtime.runtimeconfig.json
    assimp.dll
    nethost.dll
    Resources/
    Templates/Empty/
  CurryEngine-0.1.0-dev.1-win-x64.zip
  CurryEngine-0.1.0-dev.1-win-x64.zip.sha256
```

作品データ、UserScripts、Assembly-CSharp.dll、開発用PDBは含めません。シェーダーは今回のビルド結果から収集し、開発フォルダの古いCSOを混ぜません。従来の `build.bat`／`pack.bat` は完成ゲーム用として残しています。

## 必要な環境と現時点の制約

作成側にはPowerShell 7、Visual Studio 2022のC++ツール（v143、Windows SDK）、.NET 9 SDK、通常のCurryEngineビルドに必要な外部ライブラリが必要です。assimpのDebug DLLは現在の開発構成と同じ `x64/Debug/assimp.dll`、nethostは `External/DotNetHost/9.0.10/runtimes/win-x64/native/nethost.dll` を使用します。不足時は処理を失敗させます。

**現在のパッケージは開発環境向けのDebugエディターです。** エディター機能が `_DEBUG` に依存しており、Releaseは完成ゲーム用です。manifestには `packageKind: development-editor`、`configuration: Debug` と必要環境を記録します。利用側にもVisual StudioのC++ Debugランタイムを含む開発環境と.NET 9 SDKが必要です。DebugランタイムDLLはZIPへ同梱しません。

ZIPを展開後、CurryHubの **Engine Versions → Add** から `CurryEngine.exe` を登録して使用します。HubによるZIPの自動取り込みは別機能です。

パッケージ作成時の検証は描画やエディター内の再生、開発ツール未導入PCでの動作を含みません。Debugランタイムに依存しない配布用Editor構成と、未導入環境のセットアップは今後の作業です。

## ログと失敗時の扱い

ビルド・コード生成は `Library/Packaging/<実行ID>`、梱包・展開検証は出力先の `.curry-package-<実行ID>` に保存します。普段の `Generated` やエンジンexe/DLLは上書きしません。失敗した作業ファイルも原因調査のために残します。完成品の再利用や古いバイナリへのフォールバックは行いません。

エディター起動時には、プロジェクト内の `Logs/startup.log` に初期化の進行を追記します。メイン処理で捕捉したC++例外はログとエラー画面に表示します。強制終了や別スレッドの異常など、すべての障害を捕捉するものではありません。

Emptyテンプレートには `Assets/Models` がありません。モデルキャッシュの削除はこのフォルダが存在する場合だけ行い、削除中のファイル操作エラーでも起動を続けます。この修正と起動ログはソースコードの変更なので、既存の配布用exeには反映されません。利用するには配布用エンジンの再生成が必要です。

## 梱包処理のテスト

```powershell
pwsh -File ./Tests/Run-EditorPackageTests.ps1
```

バージョンの検証、収集対象、欠損ファイル、古いシェーダーや作品データの除外、既存ZIPの保護、子プロセスの日本語引数・大量出力・失敗・タイムアウトを検証します。実際のエンジンとの統合検証はパッケージ作成コマンドに組み込まれています。

実装時の確認状況: 梱包処理の34項目のテストとPowerShell構文確認は通過しています。MetaParserのビルド・生成とシェーダーコンパイルまで確認しました。エディター全体のビルドは途中で停止しており、完成ZIPの生成と展開後の実エンジン統合検証は未完了です。
