cd %~dp0

mkdir .\Output\x64-Debug

del .\Output\x64-Debug\application.json

mklink /h    .\Output\x64-Debug\application.json .\application.json

mkdir .\Output\x64-Release

del .\Output\x64-Release\application.json

mklink /h    .\Output\x64-Release\application.json .\application.json
