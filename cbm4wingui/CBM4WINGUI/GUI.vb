#Region "Copyright"
'-----------------
'GUI4CBM4WIN
'
' Copyright (C) 2004-2005 Leif Bloomquist
' Copyright (C) 2006      Wolfgang Moser
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

' Version 0.06 - First general release.
'         0.07 - Added quotes around parameters in DoCommand(), to handle paths with spaces.
'         0.08 - Added removal of .prg extensions with PC->64 copy.
'                Added file size calculation and display.
'                Added "Make Dir" button.
'                Prevented a mix of files and D64s from being selected.
'         0.09 - Fixed bug with ini file save location.
'                Uses cbmcopy '-f S' when writing files that end in .seq (and strips off extension)
'                Uses ",s" at end of name when reading files that end in .seq (and handles extension)
'                Option to inhibit the automatic directory reread after most operations, so errors aren't covered up.
'                Fixed hardcoded reference to c:\ drive in two places
'                Improved handling of default values when INI file has errors.
'
'   Additions by Wolfgang Moser, based on Gui4Cbm4Win 0.0.9
'         0.40 - Renamed all "Warp"-Options into "No-Warp" ones
'                Added "auto" transfer option
'                Renamed the stdout and stderr log files to g4c4w*.log
'                Extended the "Detect" dialog in the options menu so that more than one drive is shown
'                Fixed the "status" action, the return string now needs to be taken from stdout
'                Fixed the "dir" action, the status string now needs to be taken from the last line on stdout
'                The "Morse" code action is now directed to the currently selected IEC bus device
'                Put the sources under the zlib/libpng license to clearly define usage and reusage
'                    by further developers, to make a crystal clear definition of this branch of
'                    Leif's software beeing OpenSource for now and all times without the copyleft
'                    restriction of the GNU General Public License, but still beeing compatible
'                    to the GPL and finally to allow distribution via Sourceforge. All in all I
'                    hope to preserve the will of Leif as much as possible with that license.
'                Added a "Reset bus" action button to the options dialog
'                Added a generalized check for error conditions in the commands executor
'                Format action: Added the switches "-v" and "-o" as well as the missing "-s" for printing the status
'                Fixed the format action, the status string now needs to be taken from the last line on stdout
'                Using cbmforng instead of cbmformat now
'                Initialize: Added querying the status after sending the "I0" command
'                Options dialog fix: the selected drive number is put to the main window object
'                Options dialog fix: the selected working directory patch is taken over to the main form
'                Temporary files are created within the Environ$("temp") (%TEMP%) specified directory
'                Added the quiet option "-q" to cbmcopy, so only error messages get printed
'                Added Chr$(34)/'"' quotations to all filename arguments given to DoCommand
'
'         0.41 - Fix the scratch command by appending a space character after the drive number
'                Added a "Validate" button
'TODO:
'                Improve the ASCII/PETSCII translation when reading dirs, for rename/scratch
'   
'         0.50 - Convert to VB.Net 2.0 - Payton Byrd
'                Make the user interface sizable
'
'         0.60 - Replaced calls to Shell with using the .Net Process object
'                Capturing the stdout and stderr via the process instead of reading files whenever possible.
'                Added Splitter Panel
'                Added Logger
'                Added CBM Drive selector to GUI.vb
#End Region

Option Strict Off
Option Explicit On

Imports VB = Microsoft.VisualBasic
Imports System.IO

Friend Class MainForm
    Inherits System.Windows.Forms.Form



    Private Sub About_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles About.Click
        MsgBox( _
            "GUI4CBM4WIN by Leif Bloomquist (no support anymore) & adoption to OpenCBM 0.4.0 by Wolfgang Moser (http://d81.de/)            " & vbNewLine _
            & "This is a simple GUI front-end for cbm4win by Spiro Trikaliotis (www.trikaliotis.net/cbm4win/)" & vbNewLine _
            & "cbm4win itself is heavily based on cbm4linux written by Michael Klein (http://www.lb.shuttle.de/puffin/)" & vbNewLine _
            & "This is version " & Application.ProductVersion & " of the GUI, " _
            & "which is currently maintained by Payton Byrd (payton@paytonbyrd.com) and is " & vbNewLine _
            & "distributed under the zlib/libpng OpenSource license.", MsgBoxStyle.Information, "About")
    End Sub

    'Called when user selects a file on the CBM directory

    Private Sub CBMDirectory_SelectedIndexChanged(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMDirectory.SelectedIndexChanged

        'Prevent "blocks free" from being selected
        If CBMDirectory.GetSelected(CBMDirectory.Items.Count - 1) Then
            CBMDirectory.SetSelected(CBMDirectory.Items.Count - 1, False)
        End If
    End Sub

    'Fetch the drive status strings.
    Private Sub CBMDriveStatus_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMDriveStatus.Click


        Dim Status As ReturnStringType

        Status = DoCommand("cbmctrl", "status " & DriveNumber, "Reading drive status, please wait.")

        LastStatus.Text = UCase(Status.Output)
    End Sub

    'Format a floppy.
    Private Sub CBMFormat_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMFormat.Click


        Dim Result As Object
        Dim Status As ReturnStringType
        Dim args As String

        Result = MsgBox("This will erase ALL data on the floppy disk.  Are you sure?", _
            MsgBoxStyle.Exclamation Or MsgBoxStyle.YesNo, _
            "Format Disk")

        If Not (Result = MsgBoxResult.No) Then

            Prompt.Ask("Please Enter Diskname, ID")

            If (Prompt.LastResult = CANCELSTRING) Then Exit Sub

            args = String.Format(" -vso {0} ""{1}""", DriveNumber, Prompt.LastResult.ToUpper())

            Status = DoCommand("cbmforng", args, "Formatting floppy disk, please wait.")

            LastStatus.Text = UCase(Status.Output)

            System.Threading.Thread.Sleep(1000)

            RefreshCBMDir()
        End If
    End Sub

    Private Sub CBMInitialize_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMInitialize.Click


        DoCommand("cbmctrl", "command " & DriveNumber & " I0", "Initializing Drive")

        CBMDriveStatus_Click(eventSender, eventArgs)
    End Sub

    Private Sub CBMRefresh_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMRefresh.Click


        Dim temp As String
        Dim Results As ReturnStringType

        Try

            'Probably the most complicated action - read a directory.
            CBMDirectory.Items.Clear()

            'Run the program
            'UPGRADE_WARNING: Couldn't resolve default property of object Results. Click for more: 'ms-help://MS.VSCC.v80/dv_commoner/local/redirect.htm?keyword="6A50421D-15FE-4896-8A1B-2EC21E9037B2"'
            Results = DoCommand("cbmctrl", "dir " & DriveNumber, "Reading directory, please wait.", False)

            ' 'The drive status is always returned.
            ' LastStatus.Caption = UCase(Results.Errors)
            '
            ' It's not that simple anymore, we now need to extract it from the last line of stdout

            'Read in the complete output file -------------

            Dim reader As StringReader = New StringReader(Results.Output)
            Dim last As String = vbNullString

            'Check for empty file
            'First line is dir. name and ID
            temp = reader.ReadLine()
            CBMDiskName.Text = UCase(ExtractQuotes(temp))
            CBMDiskID.Text = UCase(VB.Right(temp, 5))

            temp = reader.ReadLine()
            While Not (temp Is Nothing)
                ' Whenever another line is available, store the current one into the panel
                CBMDirectory.Items.Add(temp.ToUpper()) 'Not only does uppercase look better, but Scratch, Rename need uppercase

                last = temp
                temp = reader.ReadLine()
            End While

            'The drive status is taken from the last line on stdout
            If Not (last Is Nothing) Then
                If Not last.Length = 0 Then
                    LastStatus.Text = last.ToUpper()
                End If
            End If

            reader.Close()

            'And delete both temp files, so we're not cluttering things up
            File.Delete(Path.Combine(OUTPUT_PATH, TEMPFILE1))
            File.Delete(Path.Combine(OUTPUT_PATH, TEMPFILE2))

        Catch exception As Exception

            If Not (Err.Number = 53) Then
                MsgBox(exception.Message, MsgBoxStyle.Information, "Refresh")
            End If

            ClearCBMDir()

        End Try
    End Sub

    Private Sub CBMRename_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMRename.Click


        Dim T As Short
        Dim args As String

        For T = 0 To CBMDirectory.Items.Count - 1

            If (CBMDirectory.GetSelected(T)) Then

                Prompt.Reply.Text = ExtractQuotes(VB6.GetItemString(CBMDirectory, T))

                Prompt.Ask("Enter new name for '" & ExtractQuotes(VB6.GetItemString(CBMDirectory, T)) & "'", False)

                If Not (Prompt.LastResult = CANCELSTRING) Then
                    args = String.Format("command {0} ""R0:{1}={2}""", _
                        DriveNumber, _
                        Prompt.LastResult.ToUpper(), _
                        ExtractQuotes(VB6.GetItemString(CBMDirectory, T)))

                    DoCommand("cbmctrl", args, "Renaming")
                Else
                    Exit Sub
                End If
            End If
        Next T

        RefreshCBMDir()
    End Sub

    Private Sub CBMReset_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMReset.Click

        DoCommand("cbmctrl", "reset", "Resetting drives, please wait.")
    End Sub

    Private Sub CBMScratch_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMScratch.Click


        Dim T As Short
        Dim args As String
        Dim file As String

        For T = 0 To CBMDirectory.Items.Count - 1

            If (CBMDirectory.GetSelected(T)) Then

                file = ExtractQuotes(VB6.GetItemString(CBMDirectory, T))

                args = String.Format("command {0} ""S0:{1}""", _
                    DriveNumber, _
                    file)

                DoCommand("cbmctrl", args, _
                    "Scratching " & file)
            End If
        Next T

        RefreshCBMDir()
    End Sub

    Private Sub CBMValidate_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles CBMValidate.Click


        Dim args As String

        args = String.Format("command {0} ""V0:""", _
            DriveNumber)

        DoCommand("cbmctrl", args, "Validating drive, please wait.")
    End Sub

    Private Sub CopyFromFloppy_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) _
      Handles CopyFromFloppy.Click

        Dim T As Short
        Dim FilesSelected As Short
        Dim Result As Object
        Dim FileName As String
        Dim FileNameOut As String
        Dim FileNameTemp As String
        Dim args As String

        FilesSelected = 0

        ChDir(PCWorkingDir.Text)

        For T = 0 To CBMDirectory.Items.Count - 1
            If (CBMDirectory.GetSelected(T)) Then
                FileName = ExtractQuotes(VB6.GetItemString(CBMDirectory, T)).ToLower()

                'Check for SEQ file
                FileNameTemp = VB6.GetItemString(CBMDirectory, T).Trim() 'Remove spaces first
                If FileNameTemp.ToUpper().EndsWith("SEQ") Then
                    FileNameOut = FileName & ".seq"
                    FileName = FileName & ",s"
                Else
                    FileNameOut = FileName
                    'FileName stays the same
                End If

                args = String.Format("--transfer={0} -r {1} ""{2}"" --output=""{3}""", _
                    TransferString, DriveNumber, FileName, FileNameOut)

                DoCommand("cbmcopy", args, "Copying '" & ExtractQuotes(VB6.GetItemString(CBMDirectory, T)) & "' from floppy disk.")
                FilesSelected = FilesSelected + 1
            End If
        Next T

        'No Files were selected, make a D64 instead.
        If (FilesSelected = 0) Then
            Result = MsgBox("No files selected.  Do you want to make a D64 image of this floppy disk?", _
                MsgBoxStyle.Question Or MsgBoxStyle.YesNo, _
                "Create D64")

            If Not (Result = MsgBoxResult.No) Then

                Prompt.Ask("Please Enter Filename:")
                If Not (Prompt.LastResult = CANCELSTRING) Then
                    args = String.Format("--transfer={0} {1} {2} ""{3}""", _
                        TransferString, NoWarpString, DriveNumber, Prompt.LastResult)

                    DoCommand("d64copy", args, "Creating D64 image, please wait.")

                    PCDirectory.Refresh()
                End If
            End If
        End If
    End Sub

    Private Sub CopyToFloppy_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) _
      Handles CopyToFloppy.Click

        Dim T As Short
        Dim FilesSelected As Short
        Dim FileName As String
        Dim FileNameOut As String
        Dim SeqType As String
        Dim args As String

        FilesSelected = 0

        For T = 0 To PCDirectory.Items.Count - 1
            If (PCDirectory.GetSelected(T)) Then
                FilesSelected = FilesSelected + 1

                If PCDirectory.Items(T).ToUpper().EndsWith(".D64") Then 'Make Disk from D64
                    WriteD64toFloppy(PCDirectory.Items(T))
                    Exit Sub 'Exit so only 1 D64 is copied!
                Else 'Copy a File
                    FileName = PCDirectory.Items(T).ToLower()

                    'Remove .prg extention
                    If FileName.EndsWith(".prg") Then
                        FileNameOut = FileName.Substring(0, FileName.Length - 4)
                    Else
                        FileNameOut = FileName
                    End If

                    'Change file type for .seq extension
                    If FileName.EndsWith(".seq") Then
                        SeqType = " --file-type S"
                        FileNameOut = FileName.Substring(0, FileName.Length - 4)
                    Else
                        SeqType = ""
                    End If
                    args = String.Format("--transfer={0} -w {1} ""{2}"" --output=""{3}""{4}", _
                        TransferString, DriveNumber, FileName, FileNameOut, SeqType)



                    DoCommand("cbmcopy", args, _
                        "Copying '" & PCDirectory.Items(T) & "' to floppy disk as '" & UCase(FileNameOut) & "'")
                End If
            End If
        Next T

        If (FilesSelected > 0) Then
            RefreshCBMDir()
        End If
    End Sub

    Private Sub WriteD64toFloppy(ByRef d64file As String)

        Dim Result As Object
        Dim args As String

        Result = MsgBox("This will overwrite ALL data on the floppy disk.  Are you sure?", _
            MsgBoxStyle.Exclamation Or MsgBoxStyle.YesNo, _
            "Write D64 to Disk")

        If Not (Result = MsgBoxResult.No) Then
            args = String.Format("--transfer={0} {1} ""{2}"" {3}", _
                TransferString, NoWarpString, d64file, DriveNumber)

            DoCommand("d64copy", args, "Creating disk from D64 image, please wait.")

            CBMRefresh_Click(CBMRefresh, New System.EventArgs())

        End If
    End Sub

    ' Program Initialization
    Private Sub MainForm_Load(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles MyBase.Load

        On Error Resume Next

        Text = String.Format("GUI4CBM4WIN {0}", Application.ProductVersion)

        If (VB.Command() = "-leifdevelopment") Then
            OptionsForm.PreviewCheck.CheckState = System.Windows.Forms.CheckState.Checked
        End If

        CheckExes()
        ExeDir = AddSlash(CurDir())

        LoadINI()
        GotoDir(OptionsForm.StartingPath.Text)
        DriveNumber = CShort(OptionsForm.DriveNum.Text)

        PCWorkingDir.Text = AddSlash(CurDir())
        PCRefresh_Click(PCRefresh, New System.EventArgs())

    End Sub

    'A slightly smarter version of Chdir, that handles drives as well.
    Private Sub GotoDir(ByRef FullPath As String)

    End Sub


    'This function must be private, because of the return type.
    Private Function DoCommand( _
       ByRef Action As String, _
       ByRef Args As String, _
       ByRef WaitMessage As String, _
       Optional ByRef DeleteOutFile As Boolean = True) _
       As ReturnStringType

        DoCommand.Output = vbNullString
        DoCommand.Errors = vbNullString

        Static InProgress As Boolean
        Dim CmdLine As String
        Dim ErrorString As String = vbNullString
        Dim FirstLog As Boolean = True

        Try
            If (InProgress) Then
                MsgBox("cbm4win command already in progress, cannot continue.", MsgBoxStyle.Critical)
                DoCommand.Errors = "cbm4win command already in progress."
                DoCommand.Output = vbNullString
                Exit Function
            End If

            'Check command - mostly for debugging.
            Dim Result As Object
            If (OptionsForm.PreviewCheck.CheckState) Then
                Result = MsgBox("Requested command:" & vbNewLine & vbNewLine & _
                    Action & " " & Args & vbNewLine & vbNewLine & _
                    "OK to continue?", MsgBoxStyle.YesNo)
                If Result = MsgBoxResult.No Then
                    Exit Function
                End If
            End If

            'Flag that the background process is starting.
            InProgress = True

            'Show in Progress Dialog
            ' Waiting.Show
            Waiting.Label.Text = WaitMessage
            VB6.ShowForm(Waiting, VB6.FormShowConstants.Modeless, Me)

            'cmd /c is needed in order to have a shell write to a file (long, complicated explanation)
            '1> redirects stdout to a file, and 2> redirects stderr to a file (Win2K/XP only)

            'All these quotes [chr$(34)] are needed to handle spaces.  So you get:
            'cmd /c ""path\command" args "files""

            'OutFile = Path.Combine(OUTPUT_PATH, TEMPFILE1)
            'ErrFile = Path.Combine(OUTPUT_PATH, TEMPFILE2)

            CmdLine = String.Format("{0} {1}", Action, Args)

            'System.Diagnostics.Debug.WriteLine(CmdLine)
            'ShellWait(CmdLine, AppWinStyle.Hide)

            Dim p As Process = New Process()

            p.StartInfo.FileName = Path.Combine(ExeDir, Action)
            p.StartInfo.Arguments = Args
            p.StartInfo.CreateNoWindow = True
            p.StartInfo.RedirectStandardError = True
            p.StartInfo.RedirectStandardOutput = True
            p.StartInfo.UseShellExecute = False


            If (p.Start()) Then

                Dim stdOut As StreamReader = p.StandardOutput
                Dim stdOutString As String
                Dim stdOutLength As Long = 0
                Dim stdErr As StreamReader = p.StandardError

                While Not p.HasExited

                    stdOutString = stdOut.ReadLine

                    If Not FirstLog Then
                        WriteLog(Nothing, stdOutString)
                    Else
                        WriteLog(CmdLine, stdOutString)
                        FirstLog = False
                    End If

                    'End If
                    Application.DoEvents()

                End While

                stdOutString = stdOut.ReadToEnd

                If Not FirstLog Then
                    WriteLog(Nothing, stdOutString)
                Else
                    WriteLog(CmdLine, stdOutString)
                    FirstLog = False
                End If

                Log.AppendText("========================================" & vbNewLine)

                DoCommand.Output += stdOutString
                DoCommand.Errors = stdErr.ReadToEnd()

            End If
        Catch exception As Exception

            DoCommand.Errors = exception.ToString()

        Finally

            Waiting.Hide()
            InProgress = False

            If (Not DoCommand.Errors = "") Then
                MsgBox(DoCommand.Errors, MsgBoxStyle.OkOnly, Action)
            End If

        End Try
    End Function

    Private Sub WriteLog(ByVal CmdLine As String, ByVal outString As String)
        If Not (CmdLine Is Nothing) Then
            If CmdLine.Length > 0 Then
                Log.AppendText(String.Format("{0:yyyy/MM/dd HH:mm:ss} {1}", DateTime.Now, CmdLine) & vbNewLine)
            End If
        End If

        Log.AppendText(outString & vbNewLine)
        Log.Select(Log.Text.Length - 1, 0)
        Log.ScrollToCaret()
        Log.Refresh()


    End Sub

    Private Sub MakeDir_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles MakeDir.Click
        Prompt.Ask("Enter Directory Name")
        If (Prompt.LastResult = CANCELSTRING) Then Exit Sub

        MkDir(AddSlash(PCWorkingDir.Text) & Prompt.LastResult)
        PCRefresh_Click(PCRefresh, New System.EventArgs())
    End Sub

    Private Sub Options_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles Options.Click
        OptionsForm.StartingPath.Text = AddSlash(PCWorkingDir.Text)

        OptionsForm.ShowDialog()

        DriveNumber = CShort(OptionsForm.DriveNum.Text)
        PCWorkingDir.Text = OptionsForm.StartingPath.Text
        GotoDir(PCWorkingDir.Text)
    End Sub

    Private Sub PCDelete_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles PCDelete.Click
        Dim T As Short
        Dim Result As Object

        Result = MsgBox("Delete the selected file(s)?", MsgBoxStyle.YesNo Or MsgBoxStyle.Question, "Delete Files")
        If (Result = MsgBoxResult.No) Then Exit Sub

        For T = 0 To PCDirectory.Items.Count - 1
            If (PCDirectory.GetSelected(T)) Then
                File.Delete(PCDirectory.Items(T))
            End If
        Next T

        PCDirectory.Refresh()
    End Sub

    Private busy As Boolean

    Private Sub PCDirectory_SelectedIndexChanged(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles PCDirectory.SelectedIndexChanged


        If Not busy Then

            Dim T As Short
            Dim Bytes As Integer
            Dim D64Selected As Short

            busy = True

            D64Selected = -1
            Bytes = 0

            'If a D64 is selected, deselect all the others
            Dim item As String

            For Each item In PCDirectory.SelectedItems
                If item.ToUpper().EndsWith(".D64") Then
                    PCDirectory.ClearSelected()
                    PCDirectory.SelectedItem = item
                    Exit For
                End If
            Next

            'Refresh the KB/Blocks display
            For Each item In PCDirectory.SelectedItems
                Dim filename As String = Path.Combine(PCWorkingDir.Text, PCDirectory.Items(T))

                ' Check if file exists first
                If Not File.Exists(filename) Then
                    ' File does not exist anymore
                    PCDirectory.ClearSelected()
                    Bytes = 0
                    PCDirectory.Refresh()
                    Exit For
                Else
                    Bytes += FileLen(filename)
                End If
            Next

            KBText.Text = VB6.Format(Bytes / 1024, "0.0") & " KB"
            BlockText.Text = CShort(Bytes / 254) & " Blocks" '254 Bytes per C= Block

            busy = False
        End If
    End Sub

    Private Sub PCRefresh_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles PCRefresh.Click
        PCWorkingDir.Text = AddSlash(CurDir())
        PCDirectory.Path = CurDir()
        PCDirectory.Refresh()
    End Sub

    Private Sub PCRename_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles PCRename.Click
        Dim T As Short

        For T = 0 To PCDirectory.Items.Count - 1
            If (PCDirectory.GetSelected(T)) Then
                Prompt.Reply.Text = PCDirectory.Items(T)
                Prompt.Ask("Enter new name for '" & PCDirectory.Items(T) & "'", False)

                If Not (Prompt.LastResult = CANCELSTRING) Then
                    File.Move(PCDirectory.Items(T), Prompt.LastResult)
                End If
            End If
        Next T

        PCDirectory.Refresh()
    End Sub

    Private Sub PCWorkingDir_KeyDown(ByVal eventSender As System.Object, ByVal eventArgs As System.Windows.Forms.KeyEventArgs)


        Dim KeyCode As Short = eventArgs.KeyCode
        Dim Shift As Short = eventArgs.KeyData \ &H10000

        On Error Resume Next

        'Enable Enter Key
        If (KeyCode = 13) Then
            ' ChDir PCWorkingDir.Text
            GotoDir(PCWorkingDir.Text)
            PCRefresh_Click(PCRefresh, New System.EventArgs())
            PCWorkingDir.Text = PCWorkingDir.Text
        End If
    End Sub

    'Thanks to vinnyd79 on Experts Exchange for this function!
    Private Function ShellWait(ByRef PathName As Object, Optional ByRef WindowStyle As AppWinStyle = AppWinStyle.NormalFocus) As Double

        Dim hProcess, RetVal As Integer

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, Shell(PathName, WindowStyle))

        Do
            GetExitCodeProcess(hProcess, RetVal)

            System.Windows.Forms.Application.DoEvents()
            System.Threading.Thread.Sleep(10) ' was 100

        Loop While RetVal = STILL_ACTIVE

    End Function

    Private Function ExtractQuotes(ByRef FullString As String) As String
        Dim Quote1 As Short
        Dim Quote2 As Short

        ExtractQuotes = FullString

        Try

            Quote1 = InStr(FullString, Chr(34))
            Quote2 = InStr(Quote1 + 1, FullString, Chr(34))
            ExtractQuotes = Mid(FullString, Quote1 + 1, Quote2 - Quote1 - 1)

        Catch exception As Exception

            MsgBox(exception.ToString())

        End Try
    End Function

    Private Sub MainForm_FormClosing(ByVal eventSender As System.Object, ByVal eventArgs As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing
        Dim Cancel As Boolean = eventArgs.Cancel
        Dim UnloadMode As System.Windows.Forms.CloseReason = eventArgs.CloseReason
        'Make sure all child forms are closed
        End
        eventArgs.Cancel = Cancel
    End Sub

    Private Sub RunFile_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles RunFile.Click
        Dim T As Short
        Dim hWnd As Object = Nothing

        For T = 0 To PCDirectory.Items.Count - 1
            If (PCDirectory.GetSelected(T)) Then
                ShellExecute(hWnd, "open", PCDirectory.Items(T), vbNullString, CurDir(), 1)

                'Stop so only the first file is executed
                Exit Sub
            End If
        Next T
    End Sub

    Public Function PubDoCommand( _
      ByRef Action As String, _
      ByRef Args As String, _
      ByRef WaitMessage As String, _
      Optional ByRef DeleteOutFile As Boolean = True) As String

        Dim Returns As ReturnStringType

        Returns = DoCommand(Action, Args, WaitMessage, DeleteOutFile)

        PubDoCommand = Returns.Output
    End Function

    'Check that the program is in the correct place, berate user if not :-)
    Private Sub CheckExes()
        Dim FilesOK As Boolean

        FilesOK = Not (Dir("cbmctrl.exe") = "")

        If Not (FilesOK) Then

            MsgBox("Can't find cbmctrl.exe " & vbNewLine & vbNewLine & _
                "gui4cbm4win must be run from within the same directory as the cbm4win executable files." & vbNewLine & vbNewLine & _
                "Current Directory: " & CurDir(), _
                MsgBoxStyle.Critical, "Error")

            End

        End If
    End Sub

    'Refresh the directory - only if automatic refresh hasn't been turned off
    Private Sub RefreshCBMDir()
        If (OptionsForm.AutoRefreshDir.CheckState) Then
            CBMRefresh_Click(CBMRefresh, New System.EventArgs())
        Else
            ClearCBMDir()
        End If
    End Sub

    'Clear the directory listing, because contents have changed
    Private Sub ClearCBMDir()
        CBMDirectory.Items.Clear()
        CBMDiskName.Text = ""
        CBMDiskID.Text = ""
        LastStatus.Text = ""
    End Sub

    Private Sub Drive_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Drive.SelectedIndexChanged
        DriveNumber = CShort(Drive.SelectedItem)
    End Sub

    Private Sub cmdBrowse_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles cmdBrowse.Click
        folderBrowser.SelectedPath = PCWorkingDir.Text
        If folderBrowser.ShowDialog() = Windows.Forms.DialogResult.OK Then
            PCWorkingDir.Text = folderBrowser.SelectedPath
            PCDirectory.Path = folderBrowser.SelectedPath
        End If
    End Sub
End Class