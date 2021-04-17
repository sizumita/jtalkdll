@echo off
openfiles > NUL 2>&1
if not %ERRORLEVEL% == 0 (
    echo �Ǘ��Ҍ����Ŏ��s���Ă�������
    pause
    goto :eof
)
cd /d %~dp0
set dll=JTalkCOMx64.dll
if not "%PROCESSOR_ARCHITECTURE%" == "x86" (
    if exist "%dll%" (
        echo �������s:%dll%
        rem C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regasm.exe %dll% /unregister /nologo /tlb
        C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regasm.exe %dll% /unregister /nologo
        echo �I���R�[�h�F%errorlevel%
    )
    if exist "%dll%" (
        echo �o�^���s:%dll%
        rem C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regasm.exe %dll% /codebase /nologo /tlb
        C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regasm.exe %dll% /codebase /nologo
        echo �I���R�[�h�F%errorlevel%
    )
)
set dll=JTalkCOMx86.dll
if exist "%dll%" (
    echo �������s:%dll%
    rem C:\Windows\Microsoft.NET\Framework\v4.0.30319\regasm.exe %dll% /unregister /nologo /tlb
    C:\Windows\Microsoft.NET\Framework\v4.0.30319\regasm.exe %dll% /unregister /nologo
    echo �I���R�[�h�F%errorlevel%
)
if exist "%dll%" (
    echo �o�^���s:%dll%
    rem C:\Windows\Microsoft.NET\Framework\v4.0.30319\regasm.exe %dll% /codebase /nologo /tlb
    C:\Windows\Microsoft.NET\Framework\v4.0.30319\regasm.exe %dll% /codebase /nologo
    echo �I���R�[�h�F%errorlevel%
)

set /p=�L�[�������ƏI�����܂�<NUL
pause >NUL
echo.

