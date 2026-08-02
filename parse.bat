if exist "%~dp0Generated\CSharp" (
    rmdir /s /q "%~dp0Generated\CSharp"
)
if exist "%~dp0Generated\Interop" (
    rmdir /s /q "%~dp0Generated\Interop"
)
if exist "%~dp0Generated\Reflection" (
    rmdir /s /q "%~dp0Generated\Reflection"
)
if exist "Generated\Json" (
    rmdir /s /q "Generated\Json"
)
"Tools\MetaParser\x64\Debug\MetaParser.exe" "Generated" "Engine" "Game"