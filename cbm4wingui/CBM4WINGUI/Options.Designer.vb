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
        Me.components = New System.ComponentModel.Container
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(OptionsForm))
        Me.ToolTip1 = New System.Windows.Forms.ToolTip(Me.components)
        Me._Transfer_4 = New System.Windows.Forms.RadioButton
        Me.ResetBus = New System.Windows.Forms.Button
        Me.CBMDetect = New System.Windows.Forms.Button
        Me.Morse = New System.Windows.Forms.Button
        Me._Transfer_3 = New System.Windows.Forms.RadioButton
        Me._Transfer_2 = New System.Windows.Forms.RadioButton
        Me.ApplyChanges = New System.Windows.Forms.Button
        Me.Cancel = New System.Windows.Forms.Button
        Me._Frame_0 = New System.Windows.Forms.GroupBox
        Me.cmdBrowse = New System.Windows.Forms.Button
        Me.AutoRefreshDir = New System.Windows.Forms.CheckBox
        Me._Transfer_0 = New System.Windows.Forms.RadioButton
        Me.PreviewCheck = New System.Windows.Forms.CheckBox
        Me._Frame_1 = New System.Windows.Forms.GroupBox
        Me._Transfer_1 = New System.Windows.Forms.RadioButton
        Me.CheckNoWarpMode = New System.Windows.Forms.CheckBox
        Me.DriveNum = New System.Windows.Forms.ComboBox
        Me.StartingPath = New System.Windows.Forms.TextBox
        Me._Label_1 = New System.Windows.Forms.Label
        Me._Label_2 = New System.Windows.Forms.Label
        Me._Label_0 = New System.Windows.Forms.Label
        Me.Frame = New Microsoft.VisualBasic.Compatibility.VB6.GroupBoxArray(Me.components)
        Me.Label = New Microsoft.VisualBasic.Compatibility.VB6.LabelArray(Me.components)
        Me.Transfer = New Microsoft.VisualBasic.Compatibility.VB6.RadioButtonArray(Me.components)
        Me._BrowseFolders = New System.Windows.Forms.FolderBrowserDialog
        Me._Frame_0.SuspendLayout()
        Me._Frame_1.SuspendLayout()
        CType(Me.Frame, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Label, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Transfer, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        '_Transfer_4
        '
        Me._Transfer_4.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_4.Checked = True
        Me._Transfer_4.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_4.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_4.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Transfer.SetIndex(Me._Transfer_4, CType(4, Short))
        Me._Transfer_4.Location = New System.Drawing.Point(8, 249)
        Me._Transfer_4.Name = "_Transfer_4"
        Me._Transfer_4.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_4.Size = New System.Drawing.Size(201, 24)
        Me._Transfer_4.TabIndex = 18
        Me._Transfer_4.TabStop = True
        Me._Transfer_4.Text = "Auto (Recommended)"
        Me.ToolTip1.SetToolTip(Me._Transfer_4, "Let OpenCBM select the most efficient transfer mode")
        Me._Transfer_4.UseVisualStyleBackColor = False
        '
        'ResetBus
        '
        Me.ResetBus.BackColor = System.Drawing.SystemColors.Control
        Me.ResetBus.Cursor = System.Windows.Forms.Cursors.Default
        Me.ResetBus.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ResetBus.ForeColor = System.Drawing.SystemColors.ControlText
        Me.ResetBus.Location = New System.Drawing.Point(16, 88)
        Me.ResetBus.Name = "ResetBus"
        Me.ResetBus.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ResetBus.Size = New System.Drawing.Size(89, 25)
        Me.ResetBus.TabIndex = 19
        Me.ResetBus.Text = "Reset Bus"
        Me.ToolTip1.SetToolTip(Me.ResetBus, "Reset all device on the IEC bus")
        Me.ResetBus.UseVisualStyleBackColor = False
        '
        'CBMDetect
        '
        Me.CBMDetect.BackColor = System.Drawing.SystemColors.Control
        Me.CBMDetect.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDetect.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDetect.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMDetect.Location = New System.Drawing.Point(16, 24)
        Me.CBMDetect.Name = "CBMDetect"
        Me.CBMDetect.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDetect.Size = New System.Drawing.Size(89, 25)
        Me.CBMDetect.TabIndex = 13
        Me.CBMDetect.Text = "Detect Drive"
        Me.ToolTip1.SetToolTip(Me.CBMDetect, "Detect all currently active device on the IEC bus")
        Me.CBMDetect.UseVisualStyleBackColor = False
        '
        'Morse
        '
        Me.Morse.BackColor = System.Drawing.SystemColors.Control
        Me.Morse.Cursor = System.Windows.Forms.Cursors.Default
        Me.Morse.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Morse.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Morse.Location = New System.Drawing.Point(16, 56)
        Me.Morse.Name = "Morse"
        Me.Morse.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Morse.Size = New System.Drawing.Size(89, 25)
        Me.Morse.TabIndex = 12
        Me.Morse.Text = "Morse Code"
        Me.ToolTip1.SetToolTip(Me.Morse, "Send a SOS morse code to the LED of the currently selected device")
        Me.Morse.UseVisualStyleBackColor = False
        '
        '_Transfer_3
        '
        Me._Transfer_3.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_3.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_3.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_3.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Transfer.SetIndex(Me._Transfer_3, CType(3, Short))
        Me._Transfer_3.Location = New System.Drawing.Point(8, 225)
        Me._Transfer_3.Name = "_Transfer_3"
        Me._Transfer_3.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_3.Size = New System.Drawing.Size(201, 24)
        Me._Transfer_3.TabIndex = 8
        Me._Transfer_3.TabStop = True
        Me._Transfer_3.Text = "Parallel"
        Me.ToolTip1.SetToolTip(Me._Transfer_3, "Requires a XP1541/XP1571 cable")
        Me._Transfer_3.UseVisualStyleBackColor = False
        '
        '_Transfer_2
        '
        Me._Transfer_2.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_2.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_2.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_2.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Transfer.SetIndex(Me._Transfer_2, CType(2, Short))
        Me._Transfer_2.Location = New System.Drawing.Point(8, 201)
        Me._Transfer_2.Name = "_Transfer_2"
        Me._Transfer_2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_2.Size = New System.Drawing.Size(201, 24)
        Me._Transfer_2.TabIndex = 7
        Me._Transfer_2.TabStop = True
        Me._Transfer_2.Text = "Serial 2"
        Me.ToolTip1.SetToolTip(Me._Transfer_2, "This only works with one serial device connected.")
        Me._Transfer_2.UseVisualStyleBackColor = False
        '
        'ApplyChanges
        '
        Me.ApplyChanges.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.ApplyChanges.BackColor = System.Drawing.SystemColors.Control
        Me.ApplyChanges.Cursor = System.Windows.Forms.Cursors.Default
        Me.ApplyChanges.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ApplyChanges.ForeColor = System.Drawing.SystemColors.ControlText
        Me.ApplyChanges.Location = New System.Drawing.Point(24, 347)
        Me.ApplyChanges.Name = "ApplyChanges"
        Me.ApplyChanges.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ApplyChanges.Size = New System.Drawing.Size(209, 33)
        Me.ApplyChanges.TabIndex = 10
        Me.ApplyChanges.Text = "Save+Apply Changes"
        Me.ApplyChanges.UseVisualStyleBackColor = False
        '
        'Cancel
        '
        Me.Cancel.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Cancel.BackColor = System.Drawing.SystemColors.Control
        Me.Cancel.Cursor = System.Windows.Forms.Cursors.Default
        Me.Cancel.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Cancel.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Cancel.Location = New System.Drawing.Point(273, 347)
        Me.Cancel.Name = "Cancel"
        Me.Cancel.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Cancel.Size = New System.Drawing.Size(113, 33)
        Me.Cancel.TabIndex = 9
        Me.Cancel.Text = "Cancel"
        Me.Cancel.UseVisualStyleBackColor = False
        '
        '_Frame_0
        '
        Me._Frame_0.BackColor = System.Drawing.SystemColors.Control
        Me._Frame_0.Controls.Add(Me.cmdBrowse)
        Me._Frame_0.Controls.Add(Me._Transfer_4)
        Me._Frame_0.Controls.Add(Me.AutoRefreshDir)
        Me._Frame_0.Controls.Add(Me._Transfer_0)
        Me._Frame_0.Controls.Add(Me.PreviewCheck)
        Me._Frame_0.Controls.Add(Me._Frame_1)
        Me._Frame_0.Controls.Add(Me._Transfer_3)
        Me._Frame_0.Controls.Add(Me._Transfer_2)
        Me._Frame_0.Controls.Add(Me._Transfer_1)
        Me._Frame_0.Controls.Add(Me.CheckNoWarpMode)
        Me._Frame_0.Controls.Add(Me.DriveNum)
        Me._Frame_0.Controls.Add(Me.StartingPath)
        Me._Frame_0.Controls.Add(Me._Label_1)
        Me._Frame_0.Controls.Add(Me._Label_2)
        Me._Frame_0.Controls.Add(Me._Label_0)
        Me._Frame_0.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Frame_0.ForeColor = System.Drawing.Color.Black
        Me.Frame.SetIndex(Me._Frame_0, CType(0, Short))
        Me._Frame_0.Location = New System.Drawing.Point(8, 8)
        Me._Frame_0.Name = "_Frame_0"
        Me._Frame_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Frame_0.Size = New System.Drawing.Size(401, 333)
        Me._Frame_0.TabIndex = 0
        Me._Frame_0.TabStop = False
        Me._Frame_0.Text = "Preferences"
        '
        'cmdBrowse
        '
        Me.cmdBrowse.Location = New System.Drawing.Point(329, 32)
        Me.cmdBrowse.Name = "cmdBrowse"
        Me.cmdBrowse.Size = New System.Drawing.Size(49, 26)
        Me.cmdBrowse.TabIndex = 19
        Me.cmdBrowse.Text = "..."
        Me.cmdBrowse.UseVisualStyleBackColor = True
        '
        'AutoRefreshDir
        '
        Me.AutoRefreshDir.BackColor = System.Drawing.SystemColors.Control
        Me.AutoRefreshDir.Checked = True
        Me.AutoRefreshDir.CheckState = System.Windows.Forms.CheckState.Checked
        Me.AutoRefreshDir.Cursor = System.Windows.Forms.Cursors.Default
        Me.AutoRefreshDir.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.AutoRefreshDir.ForeColor = System.Drawing.SystemColors.ControlText
        Me.AutoRefreshDir.Location = New System.Drawing.Point(4, 302)
        Me.AutoRefreshDir.Name = "AutoRefreshDir"
        Me.AutoRefreshDir.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.AutoRefreshDir.Size = New System.Drawing.Size(345, 25)
        Me.AutoRefreshDir.TabIndex = 17
        Me.AutoRefreshDir.Text = "Automatically refresh directory after write to floppy"
        Me.AutoRefreshDir.UseVisualStyleBackColor = False
        '
        '_Transfer_0
        '
        Me._Transfer_0.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_0.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_0.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_0.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Transfer.SetIndex(Me._Transfer_0, CType(0, Short))
        Me._Transfer_0.Location = New System.Drawing.Point(8, 153)
        Me._Transfer_0.Name = "_Transfer_0"
        Me._Transfer_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_0.Size = New System.Drawing.Size(201, 24)
        Me._Transfer_0.TabIndex = 15
        Me._Transfer_0.TabStop = True
        Me._Transfer_0.Text = "Original (Very Slow!)"
        Me._Transfer_0.UseVisualStyleBackColor = False
        '
        'PreviewCheck
        '
        Me.PreviewCheck.BackColor = System.Drawing.SystemColors.Control
        Me.PreviewCheck.Cursor = System.Windows.Forms.Cursors.Default
        Me.PreviewCheck.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PreviewCheck.ForeColor = System.Drawing.SystemColors.ControlText
        Me.PreviewCheck.Location = New System.Drawing.Point(4, 279)
        Me.PreviewCheck.Name = "PreviewCheck"
        Me.PreviewCheck.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PreviewCheck.Size = New System.Drawing.Size(345, 25)
        Me.PreviewCheck.TabIndex = 14
        Me.PreviewCheck.Text = "Preview CBM4WIN commands"
        Me.PreviewCheck.UseVisualStyleBackColor = False
        '
        '_Frame_1
        '
        Me._Frame_1.BackColor = System.Drawing.SystemColors.Control
        Me._Frame_1.Controls.Add(Me.ResetBus)
        Me._Frame_1.Controls.Add(Me.CBMDetect)
        Me._Frame_1.Controls.Add(Me.Morse)
        Me._Frame_1.Font = New System.Drawing.Font("Arial", 7.62!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Frame_1.ForeColor = System.Drawing.Color.Black
        Me.Frame.SetIndex(Me._Frame_1, CType(1, Short))
        Me._Frame_1.Location = New System.Drawing.Point(264, 64)
        Me._Frame_1.Name = "_Frame_1"
        Me._Frame_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Frame_1.Size = New System.Drawing.Size(121, 132)
        Me._Frame_1.TabIndex = 11
        Me._Frame_1.TabStop = False
        Me._Frame_1.Text = "System Tools"
        '
        '_Transfer_1
        '
        Me._Transfer_1.BackColor = System.Drawing.SystemColors.Control
        Me._Transfer_1.Cursor = System.Windows.Forms.Cursors.Default
        Me._Transfer_1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Transfer_1.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Transfer.SetIndex(Me._Transfer_1, CType(1, Short))
        Me._Transfer_1.Location = New System.Drawing.Point(8, 177)
        Me._Transfer_1.Name = "_Transfer_1"
        Me._Transfer_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Transfer_1.Size = New System.Drawing.Size(201, 24)
        Me._Transfer_1.TabIndex = 6
        Me._Transfer_1.TabStop = True
        Me._Transfer_1.Text = "Serial 1 (Slow!)"
        Me._Transfer_1.UseVisualStyleBackColor = False
        '
        'CheckNoWarpMode
        '
        Me.CheckNoWarpMode.BackColor = System.Drawing.SystemColors.Control
        Me.CheckNoWarpMode.Cursor = System.Windows.Forms.Cursors.Default
        Me.CheckNoWarpMode.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CheckNoWarpMode.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CheckNoWarpMode.Location = New System.Drawing.Point(8, 112)
        Me.CheckNoWarpMode.Name = "CheckNoWarpMode"
        Me.CheckNoWarpMode.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CheckNoWarpMode.Size = New System.Drawing.Size(193, 25)
        Me.CheckNoWarpMode.TabIndex = 5
        Me.CheckNoWarpMode.Text = "Disable Warp Mode for D64 Transfer"
        Me.CheckNoWarpMode.UseVisualStyleBackColor = False
        '
        'DriveNum
        '
        Me.DriveNum.BackColor = System.Drawing.SystemColors.Window
        Me.DriveNum.Cursor = System.Windows.Forms.Cursors.Default
        Me.DriveNum.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.DriveNum.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.DriveNum.ForeColor = System.Drawing.SystemColors.WindowText
        Me.DriveNum.Items.AddRange(New Object() {"8", "9", "10", "11"})
        Me.DriveNum.Location = New System.Drawing.Point(8, 84)
        Me.DriveNum.Name = "DriveNum"
        Me.DriveNum.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.DriveNum.Size = New System.Drawing.Size(58, 24)
        Me.DriveNum.TabIndex = 3
        '
        'StartingPath
        '
        Me.StartingPath.AcceptsReturn = True
        Me.StartingPath.BackColor = System.Drawing.SystemColors.Window
        Me.StartingPath.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.StartingPath.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.StartingPath.ForeColor = System.Drawing.SystemColors.WindowText
        Me.StartingPath.Location = New System.Drawing.Point(8, 32)
        Me.StartingPath.MaxLength = 0
        Me.StartingPath.Name = "StartingPath"
        Me.StartingPath.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.StartingPath.Size = New System.Drawing.Size(314, 23)
        Me.StartingPath.TabIndex = 1
        Me.StartingPath.Text = "c:\"
        '
        '_Label_1
        '
        Me._Label_1.BackColor = System.Drawing.Color.Transparent
        Me._Label_1.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_1.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Label.SetIndex(Me._Label_1, CType(1, Short))
        Me._Label_1.Location = New System.Drawing.Point(1, 140)
        Me._Label_1.Name = "_Label_1"
        Me._Label_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_1.Size = New System.Drawing.Size(161, 17)
        Me._Label_1.TabIndex = 16
        Me._Label_1.Text = "Transfer Mode:"
        '
        '_Label_2
        '
        Me._Label_2.BackColor = System.Drawing.Color.Transparent
        Me._Label_2.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_2.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_2.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Label.SetIndex(Me._Label_2, CType(2, Short))
        Me._Label_2.Location = New System.Drawing.Point(1, 64)
        Me._Label_2.Name = "_Label_2"
        Me._Label_2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_2.Size = New System.Drawing.Size(161, 17)
        Me._Label_2.TabIndex = 4
        Me._Label_2.Text = "Drive Number:"
        '
        '_Label_0
        '
        Me._Label_0.BackColor = System.Drawing.Color.Transparent
        Me._Label_0.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_0.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_0.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Label.SetIndex(Me._Label_0, CType(0, Short))
        Me._Label_0.Location = New System.Drawing.Point(1, 18)
        Me._Label_0.Name = "_Label_0"
        Me._Label_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_0.Size = New System.Drawing.Size(361, 17)
        Me._Label_0.TabIndex = 2
        Me._Label_0.Text = "Default location of your .d64, .prg, and .seq files:"
        '
        'Transfer
        '
        '
        'OptionsForm
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(7.0!, 16.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.SystemColors.Control
        Me.ClientSize = New System.Drawing.Size(420, 392)
        Me.Controls.Add(Me.ApplyChanges)
        Me.Controls.Add(Me.Cancel)
        Me.Controls.Add(Me._Frame_0)
        Me.Cursor = System.Windows.Forms.Cursors.Default
        Me.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Location = New System.Drawing.Point(3, 22)
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "OptionsForm"
        Me.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Text = "Options"
        Me._Frame_0.ResumeLayout(False)
        Me._Frame_0.PerformLayout()
        Me._Frame_1.ResumeLayout(False)
        CType(Me.Frame, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Label, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Transfer, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents cmdBrowse As System.Windows.Forms.Button
    Friend WithEvents _BrowseFolders As System.Windows.Forms.FolderBrowserDialog
#End Region
End Class