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
#End Region

<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> Partial Class OptionsForm
#Region "Windows Form Designer generated code "
    <System.Diagnostics.DebuggerNonUserCode()> Public Sub New()
        MyBase.New()
        'This call is required by the Windows Form Designer.
        InitializeComponent()
    End Sub
    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> Protected Overloads Overrides Sub Dispose(ByVal Disposing As Boolean)
        If Disposing Then
            If Not components Is Nothing Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(Disposing)
    End Sub
    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer
    Public ToolTip1 As System.Windows.Forms.ToolTip
    Public WithEvents ApplyChanges As System.Windows.Forms.Button
    Public WithEvents Cancel As System.Windows.Forms.Button
    Public WithEvents _Transfer_4 As System.Windows.Forms.RadioButton
    Public WithEvents AutoRefreshDir As System.Windows.Forms.CheckBox
    Public WithEvents _Transfer_0 As System.Windows.Forms.RadioButton
    Public WithEvents PreviewCheck As System.Windows.Forms.CheckBox
    Public WithEvents ResetBus As System.Windows.Forms.Button
    Public WithEvents CBMDetect As System.Windows.Forms.Button
    Public WithEvents Morse As System.Windows.Forms.Button
    Public WithEvents _Frame_1 As System.Windows.Forms.GroupBox
    Public WithEvents _Transfer_3 As System.Windows.Forms.RadioButton
    Public WithEvents _Transfer_2 As System.Windows.Forms.RadioButton
    Public WithEvents _Transfer_1 As System.Windows.Forms.RadioButton
    Public WithEvents CheckNoWarpMode As System.Windows.Forms.CheckBox
    Public WithEvents DriveNum As System.Windows.Forms.ComboBox
    Public WithEvents StartingPath As System.Windows.Forms.TextBox
    Public WithEvents _Label_1 As System.Windows.Forms.Label
    Public WithEvents _Label_2 As System.Windows.Forms.Label
    Public WithEvents _Label_0 As System.Windows.Forms.Label
    Public WithEvents _Frame_0 As System.Windows.Forms.GroupBox
    Public WithEvents Frame As Microsoft.VisualBasic.Compatibility.VB6.GroupBoxArray
    Public WithEvents Label As Microsoft.VisualBasic.Compatibility.VB6.LabelArray
    Public WithEvents Transfer As Microsoft.VisualBasic.Compatibility.VB6.RadioButtonArray
    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(OptionsForm))
        Me.components = New System.ComponentModel.Container()
        Me.ToolTip1 = New System.Windows.Forms.ToolTip(components)
        Me.ApplyChanges = New System.Windows.Forms.Button
        Me.Cancel = New System.Windows.Forms.Button
        Me._Frame_0 = New System.Windows.Forms.GroupBox
        Me._Transfer_4 = New System.Windows.Forms.RadioButton
        Me.AutoRefreshDir = New System.Windows.Forms.CheckBox
        Me._Transfer_0 = New System.Windows.Forms.RadioButton
        Me.PreviewCheck = New System.Windows.Forms.CheckBox
        Me._Frame_1 = New System.Windows.Forms.GroupBox
        Me.ResetBus = New System.Windows.Forms.Button
        Me.CBMDetect = New System.Windows.Forms.Button
        Me.Morse = New System.Windows.Forms.Button
        Me._Transfer_3 = New System.Windows.Forms.RadioButton
        Me._Transfer_2 = New System.Windows.Forms.RadioButton
        Me._Transfer_1 = New System.Windows.Forms.RadioButton
        Me.CheckNoWarpMode = New System.Windows.Forms.CheckBox
        Me.DriveNum = New System.Windows.Forms.ComboBox
        Me.StartingPath = New System.Windows.Forms.TextBox
        Me._Label_1 = New System.Windows.Forms.Label
        Me._Label_2 = New System.Windows.Forms.Label
        Me._Label_0 = New System.Windows.Forms.Label
        Me.Frame = New Microsoft.VisualBasic.Compatibility.VB6.GroupBoxArray(components)
        Me.Label = New Microsoft.VisualBasic.Compatibility.VB6.LabelArray(components)
        Me.Transfer = New Microsoft.VisualBasic.Compatibility.VB6.RadioButtonArray(components)
        Me._Frame_0.SuspendLayout()
        Me._Frame_1.SuspendLayout()
        Me.SuspendLayout()
        Me.ToolTip1.Active = True
        CType(Me.Frame, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Label, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Transfer, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.Text = "Options"
        Me.ClientSize = New System.Drawing.Size(420, 374)
        Me.Location = New System.Drawing.Point(3, 22)
        Me.Icon = CType(resources.GetObject("OptionsForm.Icon"), System.Drawing.Icon)
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.StartPosition = System.Windows.Forms.FormStartPosition.WindowsDefaultLocation
        Me.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.SystemColors.Control
        Me.ControlBox = True
        Me.Enabled = True
        Me.KeyPreview = False
        Me.Cursor = System.Windows.Forms.Cursors.Default
        Me.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ShowInTaskbar = True
        Me.HelpButton = False
        Me.WindowState = System.Windows.Forms.FormWindowState.Normal
        Me.Name = "OptionsForm"
        Me.ApplyChanges.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.ApplyChanges.Text = "Save+Apply Changes"
        Me.ApplyChanges.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ApplyChanges.Size = New System.Drawing.Size(209, 33)
        Me.ApplyChanges.Location = New System.Drawing.Point(24, 328)
        Me.ApplyChanges.TabIndex = 10
        Me.ApplyChanges.BackColor = System.Drawing.SystemColors.Control
        Me.ApplyChanges.CausesValidation = True
        Me.ApplyChanges.Enabled = True
        Me.ApplyChanges.ForeColor = System.Drawing.SystemColors.ControlText
        Me.ApplyChanges.Cursor = System.Windows.Forms.Cursors.Default
        Me.ApplyChanges.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ApplyChanges.TabStop = True
        Me.ApplyChanges.Name = "ApplyChanges"
        Me.Cancel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Cancel.Text = "Cancel"
        Me.Cancel.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Cancel.Size = New System.Drawing.Size(113, 33)
        Me.Cancel.Location = New System.Drawing.Point(273, 328)
        Me.Cancel.TabIndex = 9
        Me.Cancel.BackColor = System.Drawing.SystemColors.Control
        Me.Cancel.CausesValidation = True
        Me.Cancel.Enabled = True
        Me.Cancel.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Cancel.Cursor = System.Windows.Forms.Cursors.Default
        Me.Cancel.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Cancel.TabStop = True
        Me.Cancel.Name = "Cancel"
        Me._Frame_0.Text = "Preferences"
        Me._Frame_0.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Frame_0.ForeColor = System.Drawing.Color.Black
        Me._Frame_0.Size = New System.Drawing.Size(401, 309)
        Me._Frame_0.Location = New System.Drawing.Point(8, 8)
        Me._Frame_0.TabIndex = 0
        Me._Frame_0.BackColor = System.Drawing.SystemColors.Control
        Me._Frame_0.Enabled = True
        Me._Frame_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Frame_0.Visible = True
        Me._Frame_0.Name = "_Frame_0"
        Me._Transfer_4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_4.Text = "Auto (Recommended)"
        Me._Transfer_4.Size = New System.Drawing.Size(201, 17)
        Me._Transfer_4.Location = New System.Drawing.Point(8, 227)
        Me._Transfer_4.TabIndex = 18
        Me.ToolTip1.SetToolTip(Me._Transfer_4, "Let OpenCBM select the most efficient transfer mode")
        Me._Transfer_4.Checked = True
        Me._Transfer_4.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_4.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_4.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_4.CausesValidation = True
        Me._Transfer_4.Enabled = True
        Me._Transfer_4.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Transfer_4.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_4.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_4.Appearance = System.Windows.Forms.Appearance.Normal
        Me._Transfer_4.TabStop = True
        Me._Transfer_4.Visible = True
        Me._Transfer_4.Name = "_Transfer_4"
        Me.AutoRefreshDir.Text = "Automatically refresh directory after write to floppy"
        Me.AutoRefreshDir.Size = New System.Drawing.Size(345, 25)
        Me.AutoRefreshDir.Location = New System.Drawing.Point(8, 273)
        Me.AutoRefreshDir.TabIndex = 17
        Me.AutoRefreshDir.CheckState = System.Windows.Forms.CheckState.Checked
        Me.AutoRefreshDir.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.AutoRefreshDir.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.AutoRefreshDir.FlatStyle = System.Windows.Forms.FlatStyle.Standard
        Me.AutoRefreshDir.BackColor = System.Drawing.SystemColors.Control
        Me.AutoRefreshDir.CausesValidation = True
        Me.AutoRefreshDir.Enabled = True
        Me.AutoRefreshDir.ForeColor = System.Drawing.SystemColors.ControlText
        Me.AutoRefreshDir.Cursor = System.Windows.Forms.Cursors.Default
        Me.AutoRefreshDir.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.AutoRefreshDir.Appearance = System.Windows.Forms.Appearance.Normal
        Me.AutoRefreshDir.TabStop = True
        Me.AutoRefreshDir.Visible = True
        Me.AutoRefreshDir.Name = "AutoRefreshDir"
        Me._Transfer_0.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_0.Text = "Original (Very Slow!)"
        Me._Transfer_0.Size = New System.Drawing.Size(201, 17)
        Me._Transfer_0.Location = New System.Drawing.Point(8, 160)
        Me._Transfer_0.TabIndex = 15
        Me._Transfer_0.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_0.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_0.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_0.CausesValidation = True
        Me._Transfer_0.Enabled = True
        Me._Transfer_0.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Transfer_0.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_0.Appearance = System.Windows.Forms.Appearance.Normal
        Me._Transfer_0.TabStop = True
        Me._Transfer_0.Checked = False
        Me._Transfer_0.Visible = True
        Me._Transfer_0.Name = "_Transfer_0"
        Me.PreviewCheck.Text = "Preview CBM4WIN commands"
        Me.PreviewCheck.Size = New System.Drawing.Size(345, 25)
        Me.PreviewCheck.Location = New System.Drawing.Point(8, 250)
        Me.PreviewCheck.TabIndex = 14
        Me.PreviewCheck.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PreviewCheck.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.PreviewCheck.FlatStyle = System.Windows.Forms.FlatStyle.Standard
        Me.PreviewCheck.BackColor = System.Drawing.SystemColors.Control
        Me.PreviewCheck.CausesValidation = True
        Me.PreviewCheck.Enabled = True
        Me.PreviewCheck.ForeColor = System.Drawing.SystemColors.ControlText
        Me.PreviewCheck.Cursor = System.Windows.Forms.Cursors.Default
        Me.PreviewCheck.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PreviewCheck.Appearance = System.Windows.Forms.Appearance.Normal
        Me.PreviewCheck.TabStop = True
        Me.PreviewCheck.CheckState = System.Windows.Forms.CheckState.Unchecked
        Me.PreviewCheck.Visible = True
        Me.PreviewCheck.Name = "PreviewCheck"
        Me._Frame_1.Text = "System Tools"
        Me._Frame_1.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Frame_1.ForeColor = System.Drawing.Color.Black
        Me._Frame_1.Size = New System.Drawing.Size(121, 132)
        Me._Frame_1.Location = New System.Drawing.Point(264, 64)
        Me._Frame_1.TabIndex = 11
        Me._Frame_1.BackColor = System.Drawing.SystemColors.Control
        Me._Frame_1.Enabled = True
        Me._Frame_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Frame_1.Visible = True
        Me._Frame_1.Name = "_Frame_1"
        Me.ResetBus.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.ResetBus.Text = "Reset Bus"
        Me.ResetBus.Size = New System.Drawing.Size(89, 25)
        Me.ResetBus.Location = New System.Drawing.Point(16, 88)
        Me.ResetBus.TabIndex = 19
        Me.ToolTip1.SetToolTip(Me.ResetBus, "Reset all device on the IEC bus")
        Me.ResetBus.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ResetBus.BackColor = System.Drawing.SystemColors.Control
        Me.ResetBus.CausesValidation = True
        Me.ResetBus.Enabled = True
        Me.ResetBus.ForeColor = System.Drawing.SystemColors.ControlText
        Me.ResetBus.Cursor = System.Windows.Forms.Cursors.Default
        Me.ResetBus.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ResetBus.TabStop = True
        Me.ResetBus.Name = "ResetBus"
        Me.CBMDetect.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.CBMDetect.Text = "Detect Drive"
        Me.CBMDetect.Size = New System.Drawing.Size(89, 25)
        Me.CBMDetect.Location = New System.Drawing.Point(16, 24)
        Me.CBMDetect.TabIndex = 13
        Me.ToolTip1.SetToolTip(Me.CBMDetect, "Detect all currently active device on the IEC bus")
        Me.CBMDetect.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDetect.BackColor = System.Drawing.SystemColors.Control
        Me.CBMDetect.CausesValidation = True
        Me.CBMDetect.Enabled = True
        Me.CBMDetect.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMDetect.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDetect.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDetect.TabStop = True
        Me.CBMDetect.Name = "CBMDetect"
        Me.Morse.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Morse.Text = "Morse Code"
        Me.Morse.Size = New System.Drawing.Size(89, 25)
        Me.Morse.Location = New System.Drawing.Point(16, 56)
        Me.Morse.TabIndex = 12
        Me.ToolTip1.SetToolTip(Me.Morse, "Send a SOS morse code to the LED of the currently selected device")
        Me.Morse.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Morse.BackColor = System.Drawing.SystemColors.Control
        Me.Morse.CausesValidation = True
        Me.Morse.Enabled = True
        Me.Morse.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Morse.Cursor = System.Windows.Forms.Cursors.Default
        Me.Morse.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Morse.TabStop = True
        Me.Morse.Name = "Morse"
        Me._Transfer_3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_3.Text = "Parallel"
        Me._Transfer_3.Size = New System.Drawing.Size(201, 17)
        Me._Transfer_3.Location = New System.Drawing.Point(8, 210)
        Me._Transfer_3.TabIndex = 8
        Me.ToolTip1.SetToolTip(Me._Transfer_3, "Requires a XP1541/XP1571 cable")
        Me._Transfer_3.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_3.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_3.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_3.CausesValidation = True
        Me._Transfer_3.Enabled = True
        Me._Transfer_3.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Transfer_3.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_3.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_3.Appearance = System.Windows.Forms.Appearance.Normal
        Me._Transfer_3.TabStop = True
        Me._Transfer_3.Checked = False
        Me._Transfer_3.Visible = True
        Me._Transfer_3.Name = "_Transfer_3"
        Me._Transfer_2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_2.Text = "Serial 2"
        Me._Transfer_2.Size = New System.Drawing.Size(201, 17)
        Me._Transfer_2.Location = New System.Drawing.Point(8, 194)
        Me._Transfer_2.TabIndex = 7
        Me.ToolTip1.SetToolTip(Me._Transfer_2, "This only works with one serial device connected.")
        Me._Transfer_2.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_2.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_2.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_2.CausesValidation = True
        Me._Transfer_2.Enabled = True
        Me._Transfer_2.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Transfer_2.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_2.Appearance = System.Windows.Forms.Appearance.Normal
        Me._Transfer_2.TabStop = True
        Me._Transfer_2.Checked = False
        Me._Transfer_2.Visible = True
        Me._Transfer_2.Name = "_Transfer_2"
        Me._Transfer_1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_1.Text = "Serial 1 (Slow!)"
        Me._Transfer_1.Size = New System.Drawing.Size(201, 17)
        Me._Transfer_1.Location = New System.Drawing.Point(8, 177)
        Me._Transfer_1.TabIndex = 6
        Me._Transfer_1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_1.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me._Transfer_1.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_1.CausesValidation = True
        Me._Transfer_1.Enabled = True
        Me._Transfer_1.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Transfer_1.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_1.Appearance = System.Windows.Forms.Appearance.Normal
        Me._Transfer_1.TabStop = True
        Me._Transfer_1.Checked = False
        Me._Transfer_1.Visible = True
        Me._Transfer_1.Name = "_Transfer_1"
        Me.CheckNoWarpMode.Text = "Disable Warp Mode for D64 Transfer"
        Me.CheckNoWarpMode.Size = New System.Drawing.Size(193, 25)
        Me.CheckNoWarpMode.Location = New System.Drawing.Point(8, 112)
        Me.CheckNoWarpMode.TabIndex = 5
        Me.CheckNoWarpMode.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CheckNoWarpMode.CheckAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.CheckNoWarpMode.FlatStyle = System.Windows.Forms.FlatStyle.Standard
        Me.CheckNoWarpMode.BackColor = System.Drawing.SystemColors.Control
        Me.CheckNoWarpMode.CausesValidation = True
        Me.CheckNoWarpMode.Enabled = True
        Me.CheckNoWarpMode.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CheckNoWarpMode.Cursor = System.Windows.Forms.Cursors.Default
        Me.CheckNoWarpMode.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CheckNoWarpMode.Appearance = System.Windows.Forms.Appearance.Normal
        Me.CheckNoWarpMode.TabStop = True
        Me.CheckNoWarpMode.CheckState = System.Windows.Forms.CheckState.Unchecked
        Me.CheckNoWarpMode.Visible = True
        Me.CheckNoWarpMode.Name = "CheckNoWarpMode"
        Me.DriveNum.Size = New System.Drawing.Size(58, 19)
        Me.DriveNum.Location = New System.Drawing.Point(8, 78)
        Me.DriveNum.Items.AddRange(New Object() {"8", "9", "10", "11"})
        Me.DriveNum.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.DriveNum.TabIndex = 3
        Me.DriveNum.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.DriveNum.BackColor = System.Drawing.SystemColors.Window
        Me.DriveNum.CausesValidation = True
        Me.DriveNum.Enabled = True
        Me.DriveNum.ForeColor = System.Drawing.SystemColors.WindowText
        Me.DriveNum.IntegralHeight = True
        Me.DriveNum.Cursor = System.Windows.Forms.Cursors.Default
        Me.DriveNum.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.DriveNum.Sorted = False
        Me.DriveNum.TabStop = True
        Me.DriveNum.Visible = True
        Me.DriveNum.Name = "DriveNum"
        Me.StartingPath.AutoSize = False
        Me.StartingPath.Size = New System.Drawing.Size(361, 19)
        Me.StartingPath.Location = New System.Drawing.Point(8, 32)
        Me.StartingPath.TabIndex = 1
        Me.StartingPath.Text = "c:\"
        Me.StartingPath.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.StartingPath.AcceptsReturn = True
        Me.StartingPath.TextAlign = System.Windows.Forms.HorizontalAlignment.Left
        Me.StartingPath.BackColor = System.Drawing.SystemColors.Window
        Me.StartingPath.CausesValidation = True
        Me.StartingPath.Enabled = True
        Me.StartingPath.ForeColor = System.Drawing.SystemColors.WindowText
        Me.StartingPath.HideSelection = True
        Me.StartingPath.ReadOnly = False
        Me.StartingPath.MaxLength = 0
        Me.StartingPath.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.StartingPath.Multiline = False
        Me.StartingPath.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.StartingPath.ScrollBars = System.Windows.Forms.ScrollBars.None
        Me.StartingPath.TabStop = True
        Me.StartingPath.Visible = True
        Me.StartingPath.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.StartingPath.Name = "StartingPath"
        Me._Label_1.Text = "Transfer Mode:"
        Me._Label_1.Size = New System.Drawing.Size(161, 17)
        Me._Label_1.Location = New System.Drawing.Point(8, 144)
        Me._Label_1.TabIndex = 16
        Me._Label_1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_1.TextAlign = System.Drawing.ContentAlignment.TopLeft
        Me._Label_1.BackColor = System.Drawing.Color.Transparent
        Me._Label_1.Enabled = True
        Me._Label_1.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_1.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_1.UseMnemonic = True
        Me._Label_1.Visible = True
        Me._Label_1.AutoSize = False
        Me._Label_1.BorderStyle = System.Windows.Forms.BorderStyle.None
        Me._Label_1.Name = "_Label_1"
        Me._Label_2.Text = "Drive Number:"
        Me._Label_2.Size = New System.Drawing.Size(161, 17)
        Me._Label_2.Location = New System.Drawing.Point(8, 64)
        Me._Label_2.TabIndex = 4
        Me._Label_2.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_2.TextAlign = System.Drawing.ContentAlignment.TopLeft
        Me._Label_2.BackColor = System.Drawing.Color.Transparent
        Me._Label_2.Enabled = True
        Me._Label_2.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_2.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_2.UseMnemonic = True
        Me._Label_2.Visible = True
        Me._Label_2.AutoSize = False
        Me._Label_2.BorderStyle = System.Windows.Forms.BorderStyle.None
        Me._Label_2.Name = "_Label_2"
        Me._Label_0.Text = "Default location of your .d64, .prg, and .seq files:"
        Me._Label_0.Size = New System.Drawing.Size(361, 17)
        Me._Label_0.Location = New System.Drawing.Point(8, 16)
        Me._Label_0.TabIndex = 2
        Me._Label_0.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_0.TextAlign = System.Drawing.ContentAlignment.TopLeft
        Me._Label_0.BackColor = System.Drawing.Color.Transparent
        Me._Label_0.Enabled = True
        Me._Label_0.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_0.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_0.UseMnemonic = True
        Me._Label_0.Visible = True
        Me._Label_0.AutoSize = False
        Me._Label_0.BorderStyle = System.Windows.Forms.BorderStyle.None
        Me._Label_0.Name = "_Label_0"
        Me.Controls.Add(ApplyChanges)
        Me.Controls.Add(Cancel)
        Me.Controls.Add(_Frame_0)
        Me._Frame_0.Controls.Add(_Transfer_4)
        Me._Frame_0.Controls.Add(AutoRefreshDir)
        Me._Frame_0.Controls.Add(_Transfer_0)
        Me._Frame_0.Controls.Add(PreviewCheck)
        Me._Frame_0.Controls.Add(_Frame_1)
        Me._Frame_0.Controls.Add(_Transfer_3)
        Me._Frame_0.Controls.Add(_Transfer_2)
        Me._Frame_0.Controls.Add(_Transfer_1)
        Me._Frame_0.Controls.Add(CheckNoWarpMode)
        Me._Frame_0.Controls.Add(DriveNum)
        Me._Frame_0.Controls.Add(StartingPath)
        Me._Frame_0.Controls.Add(_Label_1)
        Me._Frame_0.Controls.Add(_Label_2)
        Me._Frame_0.Controls.Add(_Label_0)
        Me._Frame_1.Controls.Add(ResetBus)
        Me._Frame_1.Controls.Add(CBMDetect)
        Me._Frame_1.Controls.Add(Morse)
        Me.Frame.SetIndex(_Frame_1, CType(1, Short))
        Me.Frame.SetIndex(_Frame_0, CType(0, Short))
        Me.Label.SetIndex(_Label_1, CType(1, Short))
        Me.Label.SetIndex(_Label_2, CType(2, Short))
        Me.Label.SetIndex(_Label_0, CType(0, Short))
        Me.Transfer.SetIndex(_Transfer_4, CType(4, Short))
        Me.Transfer.SetIndex(_Transfer_0, CType(0, Short))
        Me.Transfer.SetIndex(_Transfer_3, CType(3, Short))
        Me.Transfer.SetIndex(_Transfer_2, CType(2, Short))
        Me.Transfer.SetIndex(_Transfer_1, CType(1, Short))
        CType(Me.Transfer, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Label, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Frame, System.ComponentModel.ISupportInitialize).EndInit()
        Me._Frame_0.ResumeLayout(False)
        Me._Frame_1.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()
    End Sub
#End Region
End Class