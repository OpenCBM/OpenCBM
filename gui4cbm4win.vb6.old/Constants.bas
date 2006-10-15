Attribute VB_Name = "Constants"
Option Explicit

Public Declare Function ShellExecute _
    Lib "shell32.dll" Alias "ShellExecuteA" ( _
    ByVal hWnd As Long, _
    ByVal lpOperation As String, _
    ByVal lpFile As String, _
    ByVal lpParameters As String, _
    ByVal lpDirectory As String, _
    ByVal nShowCmd As Long) As Long
    
Public Declare Sub SetWindowPos Lib "user32" _
    (ByVal hWnd As Long, ByVal hWndInsertAfter As Long, _
ByVal x As Long, ByVal y As Long, ByVal cx As Long, _
ByVal cy As Long, ByVal wFlags As Long)
Private IsOnTop As Boolean

Public Const HWND_TOPMOST = -&H1
Public Const HWND_NOTOPMOST = -&H2
Public Const SWP_NOSIZE = &H1
Public Const SWP_NOMOVE = &H2

Public Const CANCELSTRING = "***CANCEL***"

Public Type ReturnStringType
    Output As String
    Errors As String
End Type

'Misc. API Calls
Public Declare Function OpenProcess Lib "Kernel32" (ByVal dwDesiredAccess As Long, ByVal bInheritHandle As Long, ByVal dwProcessId As Long) As Long
Public Declare Function GetExitCodeProcess Lib "Kernel32" (ByVal hProcess As Long, lpExitCode As Long) As Long
Public Declare Sub Sleep Lib "Kernel32" (ByVal dwMilliseconds As Long)
Public Const STILL_ACTIVE = &H103
Public Const PROCESS_QUERY_INFORMATION = &H400

Public Const TEMPFILE1 = "\temp1.out"
Public Const TEMPFILE2 = "\temp2.out"
