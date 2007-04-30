#Region "Copyright"
'-----------------
'GUI4CBM4WIN
'
' Copyright (C) 2004-2005 Leif Bloomquist
' Copyright (C) 2006      Wolfgang 0.6.4
' Copyright (C) 2006      Spiro Trikaliotis
' Copyright (C) 2006-2007 Payton Byrd
'
' This software Is provided 'as-is', without any express or implied
' warranty. In no event will the authors be held liable for any damages
' arising from the use of this software.
'
' Permission is granted to anyone to use this software for any purpose,
' including commercial applications, and to alter it and redistribute it
' freely, subject to the following restrictions:
'
'     1. The origin of this software must not be misrepresented; you must
'        not claim that you wrote the original software. If you use this
'        software in a product, an acknowledgment in the product
'        documentation would be appreciated but is not required.
'
'     2. Altered source versions must be plainly marked as such, and must
'        not be misrepresented as being the original software.
'
'     3. This notice may not be removed or altered from any source
'        distribution.
'
#End Region

Option Strict Off
Option Explicit On
Imports System.Configuration
Imports System.Runtime.InteropServices
Imports System.Text

Module Constants_Renamed
    Public Declare Function ShellExecute Lib "shell32.dll" Alias "ShellExecuteA" (ByVal hWnd As Integer, ByVal lpOperation As String, ByVal lpFile As String, ByVal lpParameters As String, ByVal lpDirectory As String, ByVal nShowCmd As Integer) As Integer

    Public Declare Sub SetWindowPos Lib "user32" (ByVal hWnd As Integer, ByVal hWndInsertAfter As Integer, ByVal x As Integer, ByVal y As Integer, ByVal cx As Integer, ByVal cy As Integer, ByVal wFlags As Integer)
    Private IsOnTop As Boolean

    Public Const HWND_TOPMOST As Short = -&H1S
    Public Const HWND_NOTOPMOST As Short = -&H2S
    Public Const SWP_NOSIZE As Short = &H1S
    Public Const SWP_NOMOVE As Short = &H2S

    Public Const CANCELSTRING As String = "***CANCEL***"

    Public Structure ReturnStringType
        Dim Output As String
        Dim Errors As String
    End Structure

    'Misc. API Calls
    Public Declare Function OpenProcess Lib "Kernel32" (ByVal dwDesiredAccess As Integer, ByVal bInheritHandle As Integer, ByVal dwProcessId As Integer) As Integer
    Public Declare Function GetExitCodeProcess Lib "Kernel32" (ByVal hProcess As Integer, ByRef lpExitCode As Integer) As Integer
    Public Const STILL_ACTIVE As Short = &H103S
    Public Const PROCESS_QUERY_INFORMATION As Short = &H400S

    'Public Const TEMPFILE1 As String = "OutFile.log"
    'Public Const TEMPFILE2 As String = "ErrFile.log"
    Public ReadOnly OUTPUT_PATH As String = GetShortPath()

    Public Class Interop
        <DllImport("kernel32.dll", SetLastError:=True, CharSet:=CharSet.Auto)> _
        Public Shared Function GetShortPathName(ByVal longPath As String, <MarshalAs(UnmanagedType.LPTStr)> ByVal ShortPath As StringBuilder, <MarshalAs(UnmanagedType.U4)> ByVal bufferSize As Integer) As Integer
        End Function
    End Class

    Private Function GetShortPath() As String
        Dim TempPath As New System.Text.StringBuilder(1024)
        Interop.GetShortPathName(System.IO.Path.GetTempPath(), TempPath, 1024)
        Return TempPath.ToString()
    End Function

    Private g_objConfig As Configuration = _
        ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None)

    Public ReadOnly Property Config() As Configuration
        Get
            Return g_objConfig
        End Get
    End Property
End Module