# 提出用のポータブルエディターZIP

`build_editor_package.bat` をダブルクリックして作成します。Windows標準のPowerShell 5.1を使用し、PowerShell 7の追加インストールは不要です。作成側にはVisual Studio 2022 C++ツール（v143 / Windows SDK）と.NET 9 SDKが必要です。

```powershell
.\build_editor_package.bat 0.1.0-portable.1
# 出力先・並列数を指定する場合
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./Tools/Packaging/Build-EditorPackage.ps1 -Version 0.1.0-portable.1 -OutputDirectory D:/EnginePackages -BuildWorkers 2
```

同じバージョンの出力は上書きしません。既定の出力先は `BuildOutput/Engines/CurryEngine-<version>-win-x64.zip` です。隣に展開済みフォルダとZIPのSHA256ファイルを保存します。

## 提出先での使い方

1. ZIP全体を書き込み可能なフォルダへ展開します。
2. `StartEditor.bat` をダブルクリックします。
3. `StartEditor.bat` と同じフォルダ内に `CurryEngineProject` を作成し、エディターで開きます。次回は同じプロジェクトを開きます。エンジン本体は `Engine/` に入っています。

別の場所を使う場合は `StartEditor.bat "D:\MyProject"` のようにプロジェクトフォルダを指定します。既存プロジェクトはこのエンジンのバージョンと一致する必要があります。エンジンとプロジェクトのフォルダは互いの中に配置できません。

CurryHubがある場合は **Engine Versions → Add** から `Engine/CurryEngine.exe` を登録することもできます。Hubは必須ではありません。C#ファイルの編集にはWindows標準のメモ帳なども使えます。保存後のコンパイルには同梱SDKを使用します。Visual Studioでのデバッグや完成ゲームのビルドは、このポータブルパッケージの対象外です。

## 構成

- ネイティブ側はReleaseでビルドし、`CURRY_EDITOR` と `USE_IMGUI` で編集・再生機能を有効化します。従来のDebug開発構成と完成ゲーム用Release構成も維持します。
- DirectXTexもReleaseでビルドします。assimpは `x64/Release/assimp.dll` を使います。
- Visual StudioのRedistフォルダから再配布用x64 C++ランタイムを同梱します。Debugランタイム依存を検出した場合はZIP作成を失敗させます。
- `DotNet/` に作成PCの安定版.NET 9 SDKと、それに一致するhostfxr・ランタイム・参照パック、ライセンス文書をコピーします。エディターはこのSDKとランタイムを優先します。同梱フォルダが壊れている場合、インストール済み.NETへ黙って切り替えません。
- テンプレート・フォント・画像・今回生成したシェーダーを収集します。利用者の作品データ、UserScripts、開発用PDBは含めません。
- `engine.json` と `version.txt` にバージョンを、`package-files.json` にペイロードのSHA256を記録します。

対象はWindows 10/11 x64、Direct3D 11.1に対応するGPUとドライバーです。32ビットWindowsやすべてのハードウェアでの動作を保証するものではありません。通常の起動・編集・再生ではPowerShell 7、Visual Studio、.NETの別途インストールを必要としない構成です。

## 検証とログ

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./Tests/Run-EditorPackageTests.ps1
```

梱包テストは入力検証、既存ファイル保護、日本語・空白・引用符を含む引数、タイムアウト、ZIPの圧縮・展開を検証します。パッケージ作成処理では展開したエンジンで新規プロジェクト作成・検証・ソリューション生成・同梱SDKでのC#ビルドを実行し、共有エンジンの内容が変わらないことも確認します。

ビルドログは `Library/Packaging/<実行ID>`、梱包・展開検証は出力先の `.curry-package-<実行ID>` に残します。起動ログはプロジェクトの `Logs/startup.log` です。

作成コマンドの成功だけでは、開発ツール未導入PCでの描画・編集・再生の確認にはなりません。提出前にクリーンなWindows PCでもZIPを展開し、初回起動、シーン保存、C#変更後の再コンパイル、再生・停止を確認してください。
