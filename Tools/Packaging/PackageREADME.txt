CurryEngine ポータブルエディター（Windows x64）

【起動方法】
ZIPを丸ごと書き込み可能なフォルダに展開し、StartEditor.batをダブルクリックしてください。
batと同じフォルダ内にCurryEngineProjectを作成して開きます。次回からは同じプロジェクトを開きます。
別の場所に作る場合：StartEditor.bat "D:\MyProject"
エンジン本体はEngineフォルダにあります。エンジンとプロジェクトのフォルダは互いの中に配置できません。
CurryHubを使う場合は、Engine Versions → AddからEngine\CurryEngine.exeを登録できます。

【動作環境】
Windows 10/11 x64、Direct3D 11.1対応のGPUとドライバーが必要です。
.NET 9 SDK・ランタイムと再配布用C++ランタイムは同梱されています。
通常の起動・編集・再生にPowerShell 7、Visual Studio、別途インストールした.NETは不要です。
Engine\DotNetなど、同梱フォルダの内容はそのまま残してください。
C#ファイルはメモ帳などで編集でき、保存すると再コンパイルされます。
完成ゲームのビルドやVisual Studioでのデバッグには開発ツールが必要です。

【バージョン・検証・ログ】
Engine\engine.jsonとEngine\version.txtにエンジンのバージョンを記録しています。
package-files.jsonには配布ファイルのSHA256、ZIPと一緒に配布される.zip.sha256にはZIPのSHA256を記録しています。
梱包時には展開後のプロジェクト作成と同梱SDKによるC#ビルドを確認しています。
描画・編集・再生は、配布前に開発ツール未導入のWindows PCでも確認してください。
起動時の診断ログはプロジェクト内のLogs\startup.logに保存されます。
