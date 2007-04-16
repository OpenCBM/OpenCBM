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
<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> Partial Class MainForm
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
    Public WithEvents Options As System.Windows.Forms.Button
    Public WithEvents About As System.Windows.Forms.Button
    Public WithEvents CopyFromFloppy As System.Windows.Forms.Button
    Public WithEvents CopyToFloppy As System.Windows.Forms.Button
    Public WithEvents Label As Microsoft.VisualBasic.Compatibility.VB6.LabelArray
    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(MainForm))
        Me.ToolTip1 = New System.Windows.Forms.ToolTip(Me.components)
        Me.CopyFromFloppy = New System.Windows.Forms.Button
        Me.Options = New System.Windows.Forms.Button
        Me.About = New System.Windows.Forms.Button
        Me.CopyToFloppy = New System.Windows.Forms.Button
        Me.Label = New Microsoft.VisualBasic.Compatibility.VB6.LabelArray(Me.components)
        Me.SplitContainer1 = New System.Windows.Forms.SplitContainer
        Me.CBMDrive = New System.Windows.Forms.GroupBox
        Me.Drive = New System.Windows.Forms.ComboBox
        Me.CBMValidate = New System.Windows.Forms.Button
        Me.CBMInitialize = New System.Windows.Forms.Button
        Me.CBMFormat = New System.Windows.Forms.Button
        Me.CBMDirectory = New System.Windows.Forms.ListBox
        Me.CBMReset = New System.Windows.Forms.Button
        Me.CBMDriveStatus = New System.Windows.Forms.Button
        Me.CBMRefresh = New System.Windows.Forms.Button
        Me.CBMRename = New System.Windows.Forms.Button
        Me.CBMScratch = New System.Windows.Forms.Button
        Me._Label_0 = New System.Windows.Forms.Label
        Me.LastStatus = New System.Windows.Forms.Label
        Me._Label_5 = New System.Windows.Forms.Label
        Me.CBMDiskName = New System.Windows.Forms.Label
        Me.CBMDiskID = New System.Windows.Forms.Label
        Me._Label_3 = New System.Windows.Forms.Label
        Me._Label_4 = New System.Windows.Forms.Label
        Me.Frame2 = New System.Windows.Forms.GroupBox
        Me.BlockText = New System.Windows.Forms.TextBox
        Me.KBText = New System.Windows.Forms.TextBox
        Me.Drive1 = New Microsoft.VisualBasic.Compatibility.VB6.DriveListBox
        Me.MakeDir = New System.Windows.Forms.Button
        Me.Dir1 = New Microsoft.VisualBasic.Compatibility.VB6.DirListBox
        Me.PCDirectory = New Microsoft.VisualBasic.Compatibility.VB6.FileListBox
        Me.PCRefresh = New System.Windows.Forms.Button
        Me.RunFile = New System.Windows.Forms.Button
        Me.PCRename = New System.Windows.Forms.Button
        Me.PCWorkingDir = New System.Windows.Forms.TextBox
        Me.PCDelete = New System.Windows.Forms.Button
        Me._Label_7 = New System.Windows.Forms.Label
        Me._Label_6 = New System.Windows.Forms.Label
        Me._Label_1 = New System.Windows.Forms.Label
        Me._Label_2 = New System.Windows.Forms.Label
        Me.LogGroup = New System.Windows.Forms.GroupBox
        Me.Log = New System.Windows.Forms.RichTextBox
        CType(Me.Label, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SplitContainer1.Panel1.SuspendLayout()
        Me.SplitContainer1.Panel2.SuspendLayout()
        Me.SplitContainer1.SuspendLayout()
        Me.CBMDrive.SuspendLayout()
        Me.Frame2.SuspendLayout()
        Me.LogGroup.SuspendLayout()
        Me.SuspendLayout()
        '
        'CopyFromFloppy
        '
        Me.CopyFromFloppy.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.CopyFromFloppy.BackColor = System.Drawing.SystemColors.Control
        Me.CopyFromFloppy.Cursor = System.Windows.Forms.Cursors.Default
        Me.CopyFromFloppy.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CopyFromFloppy.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CopyFromFloppy.Location = New System.Drawing.Point(319, 172)
        Me.CopyFromFloppy.Name = "CopyFromFloppy"
        Me.CopyFromFloppy.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CopyFromFloppy.Size = New System.Drawing.Size(52, 33)
        Me.CopyFromFloppy.TabIndex = 2
        Me.CopyFromFloppy.Text = "<--"
        Me.ToolTip1.SetToolTip(Me.CopyFromFloppy, "If no files are selected, this button will create a D64 image of the entire disk." & _
                "")
        Me.CopyFromFloppy.UseVisualStyleBackColor = False
        '
        'Options
        '
        Me.Options.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Options.BackColor = System.Drawing.SystemColors.Control
        Me.Options.Cursor = System.Windows.Forms.Cursors.Default
        Me.Options.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Options.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Options.Location = New System.Drawing.Point(319, 426)
        Me.Options.Name = "Options"
        Me.Options.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Options.Size = New System.Drawing.Size(52, 33)
        Me.Options.TabIndex = 24
        Me.Options.Text = "Options"
        Me.Options.UseVisualStyleBackColor = False
        '
        'About
        '
        Me.About.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.About.BackColor = System.Drawing.SystemColors.Control
        Me.About.Cursor = System.Windows.Forms.Cursors.Default
        Me.About.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.About.ForeColor = System.Drawing.SystemColors.ControlText
        Me.About.Location = New System.Drawing.Point(319, 461)
        Me.About.Name = "About"
        Me.About.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.About.Size = New System.Drawing.Size(52, 33)
        Me.About.TabIndex = 23
        Me.About.Text = "About"
        Me.About.UseVisualStyleBackColor = False
        '
        'CopyToFloppy
        '
        Me.CopyToFloppy.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.CopyToFloppy.BackColor = System.Drawing.SystemColors.Control
        Me.CopyToFloppy.Cursor = System.Windows.Forms.Cursors.Default
        Me.CopyToFloppy.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CopyToFloppy.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CopyToFloppy.Location = New System.Drawing.Point(319, 132)
        Me.CopyToFloppy.Name = "CopyToFloppy"
        Me.CopyToFloppy.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CopyToFloppy.Size = New System.Drawing.Size(52, 33)
        Me.CopyToFloppy.TabIndex = 0
        Me.CopyToFloppy.Text = "-->"
        Me.CopyToFloppy.UseVisualStyleBackColor = False
        '
        'SplitContainer1
        '
        Me.SplitContainer1.Dock = System.Windows.Forms.DockStyle.Fill
        Me.SplitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2
        Me.SplitContainer1.Location = New System.Drawing.Point(0, 0)
        Me.SplitContainer1.Name = "SplitContainer1"
        '
        'SplitContainer1.Panel1
        '
        Me.SplitContainer1.Panel1.Controls.Add(Me.CopyToFloppy)
        Me.SplitContainer1.Panel1.Controls.Add(Me.Options)
        Me.SplitContainer1.Panel1.Controls.Add(Me.CopyFromFloppy)
        Me.SplitContainer1.Panel1.Controls.Add(Me.About)
        Me.SplitContainer1.Panel1.Controls.Add(Me.CBMDrive)
        Me.SplitContainer1.Panel1.Controls.Add(Me.Frame2)
        '
        'SplitContainer1.Panel2
        '
        Me.SplitContainer1.Panel2.Controls.Add(Me.LogGroup)
        Me.SplitContainer1.Size = New System.Drawing.Size(992, 497)
        Me.SplitContainer1.SplitterDistance = 675
        Me.SplitContainer1.TabIndex = 27
        '
        'CBMDrive
        '
        Me.CBMDrive.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.CBMDrive.BackColor = System.Drawing.SystemColors.Control
        Me.CBMDrive.Controls.Add(Me.Drive)
        Me.CBMDrive.Controls.Add(Me.CBMValidate)
        Me.CBMDrive.Controls.Add(Me.CBMInitialize)
        Me.CBMDrive.Controls.Add(Me.CBMFormat)
        Me.CBMDrive.Controls.Add(Me.CBMDirectory)
        Me.CBMDrive.Controls.Add(Me.CBMReset)
        Me.CBMDrive.Controls.Add(Me.CBMDriveStatus)
        Me.CBMDrive.Controls.Add(Me.CBMRefresh)
        Me.CBMDrive.Controls.Add(Me.CBMRename)
        Me.CBMDrive.Controls.Add(Me.CBMScratch)
        Me.CBMDrive.Controls.Add(Me._Label_0)
        Me.CBMDrive.Controls.Add(Me.LastStatus)
        Me.CBMDrive.Controls.Add(Me._Label_5)
        Me.CBMDrive.Controls.Add(Me.CBMDiskName)
        Me.CBMDrive.Controls.Add(Me.CBMDiskID)
        Me.CBMDrive.Controls.Add(Me._Label_3)
        Me.CBMDrive.Controls.Add(Me._Label_4)
        Me.CBMDrive.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDrive.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMDrive.Location = New System.Drawing.Point(375, 0)
        Me.CBMDrive.Name = "CBMDrive"
        Me.CBMDrive.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDrive.Size = New System.Drawing.Size(297, 491)
        Me.CBMDrive.TabIndex = 25
        Me.CBMDrive.TabStop = False
        Me.CBMDrive.Text = "Commodore Drive"
        '
        'Drive
        '
        Me.Drive.FormattingEnabled = True
        Me.Drive.Items.AddRange(New Object() {"8", "9", "10", "11"})
        Me.Drive.Location = New System.Drawing.Point(226, 14)
        Me.Drive.Name = "Drive"
        Me.Drive.Size = New System.Drawing.Size(62, 22)
        Me.Drive.TabIndex = 37
        '
        'CBMValidate
        '
        Me.CBMValidate.BackColor = System.Drawing.SystemColors.Control
        Me.CBMValidate.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMValidate.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMValidate.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMValidate.Location = New System.Drawing.Point(226, 156)
        Me.CBMValidate.Name = "CBMValidate"
        Me.CBMValidate.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMValidate.Size = New System.Drawing.Size(65, 25)
        Me.CBMValidate.TabIndex = 36
        Me.CBMValidate.Text = "Validate"
        Me.CBMValidate.UseVisualStyleBackColor = False
        '
        'CBMInitialize
        '
        Me.CBMInitialize.BackColor = System.Drawing.SystemColors.Control
        Me.CBMInitialize.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMInitialize.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMInitialize.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMInitialize.Location = New System.Drawing.Point(226, 124)
        Me.CBMInitialize.Name = "CBMInitialize"
        Me.CBMInitialize.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMInitialize.Size = New System.Drawing.Size(65, 25)
        Me.CBMInitialize.TabIndex = 25
        Me.CBMInitialize.Text = "Initialize"
        Me.CBMInitialize.UseVisualStyleBackColor = False
        '
        'CBMFormat
        '
        Me.CBMFormat.BackColor = System.Drawing.SystemColors.Control
        Me.CBMFormat.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMFormat.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMFormat.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMFormat.Location = New System.Drawing.Point(226, 92)
        Me.CBMFormat.Name = "CBMFormat"
        Me.CBMFormat.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMFormat.Size = New System.Drawing.Size(65, 25)
        Me.CBMFormat.TabIndex = 10
        Me.CBMFormat.Text = "Format"
        Me.CBMFormat.UseVisualStyleBackColor = False
        '
        'CBMDirectory
        '
        Me.CBMDirectory.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.CBMDirectory.BackColor = System.Drawing.Color.FromArgb(CType(CType(0, Byte), Integer), CType(CType(0, Byte), Integer), CType(CType(192, Byte), Integer))
        Me.CBMDirectory.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDirectory.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDirectory.ForeColor = System.Drawing.Color.FromArgb(CType(CType(131, Byte), Integer), CType(CType(131, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.CBMDirectory.ItemHeight = 14
        Me.CBMDirectory.Location = New System.Drawing.Point(8, 40)
        Me.CBMDirectory.Name = "CBMDirectory"
        Me.CBMDirectory.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDirectory.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended
        Me.CBMDirectory.Size = New System.Drawing.Size(217, 410)
        Me.CBMDirectory.TabIndex = 9
        '
        'CBMReset
        '
        Me.CBMReset.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.CBMReset.BackColor = System.Drawing.SystemColors.Control
        Me.CBMReset.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMReset.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMReset.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMReset.Location = New System.Drawing.Point(226, 407)
        Me.CBMReset.Name = "CBMReset"
        Me.CBMReset.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMReset.Size = New System.Drawing.Size(65, 25)
        Me.CBMReset.TabIndex = 8
        Me.CBMReset.Text = "Reset"
        Me.CBMReset.UseVisualStyleBackColor = False
        '
        'CBMDriveStatus
        '
        Me.CBMDriveStatus.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.CBMDriveStatus.BackColor = System.Drawing.SystemColors.Control
        Me.CBMDriveStatus.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDriveStatus.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDriveStatus.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMDriveStatus.Location = New System.Drawing.Point(226, 439)
        Me.CBMDriveStatus.Name = "CBMDriveStatus"
        Me.CBMDriveStatus.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDriveStatus.Size = New System.Drawing.Size(65, 25)
        Me.CBMDriveStatus.TabIndex = 7
        Me.CBMDriveStatus.Text = "Status"
        Me.CBMDriveStatus.UseVisualStyleBackColor = False
        '
        'CBMRefresh
        '
        Me.CBMRefresh.BackColor = System.Drawing.SystemColors.Control
        Me.CBMRefresh.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMRefresh.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMRefresh.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMRefresh.Location = New System.Drawing.Point(226, 60)
        Me.CBMRefresh.Name = "CBMRefresh"
        Me.CBMRefresh.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMRefresh.Size = New System.Drawing.Size(65, 25)
        Me.CBMRefresh.TabIndex = 6
        Me.CBMRefresh.Text = "Directory"
        Me.CBMRefresh.UseVisualStyleBackColor = False
        '
        'CBMRename
        '
        Me.CBMRename.BackColor = System.Drawing.SystemColors.Control
        Me.CBMRename.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMRename.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMRename.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMRename.Location = New System.Drawing.Point(226, 220)
        Me.CBMRename.Name = "CBMRename"
        Me.CBMRename.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMRename.Size = New System.Drawing.Size(65, 25)
        Me.CBMRename.TabIndex = 5
        Me.CBMRename.Text = "Rename"
        Me.CBMRename.UseVisualStyleBackColor = False
        '
        'CBMScratch
        '
        Me.CBMScratch.BackColor = System.Drawing.SystemColors.Control
        Me.CBMScratch.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMScratch.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMScratch.ForeColor = System.Drawing.SystemColors.ControlText
        Me.CBMScratch.Location = New System.Drawing.Point(226, 252)
        Me.CBMScratch.Name = "CBMScratch"
        Me.CBMScratch.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMScratch.Size = New System.Drawing.Size(65, 25)
        Me.CBMScratch.TabIndex = 4
        Me.CBMScratch.Text = "Scratch"
        Me.CBMScratch.UseVisualStyleBackColor = False
        '
        '_Label_0
        '
        Me._Label_0.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me._Label_0.BackColor = System.Drawing.Color.Transparent
        Me._Label_0.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_0.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_0.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_0.Location = New System.Drawing.Point(8, 451)
        Me._Label_0.Name = "_Label_0"
        Me._Label_0.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_0.Size = New System.Drawing.Size(145, 17)
        Me._Label_0.TabIndex = 34
        Me._Label_0.Text = "Last Drive Status:"
        '
        'LastStatus
        '
        Me.LastStatus.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.LastStatus.BackColor = System.Drawing.Color.FromArgb(CType(CType(0, Byte), Integer), CType(CType(0, Byte), Integer), CType(CType(192, Byte), Integer))
        Me.LastStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LastStatus.Cursor = System.Windows.Forms.Cursors.Default
        Me.LastStatus.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LastStatus.ForeColor = System.Drawing.Color.FromArgb(CType(CType(131, Byte), Integer), CType(CType(131, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.LastStatus.Location = New System.Drawing.Point(8, 467)
        Me.LastStatus.Name = "LastStatus"
        Me.LastStatus.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.LastStatus.Size = New System.Drawing.Size(283, 19)
        Me.LastStatus.TabIndex = 33
        '
        '_Label_5
        '
        Me._Label_5.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me._Label_5.BackColor = System.Drawing.Color.Transparent
        Me._Label_5.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_5.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_5.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_5.Location = New System.Drawing.Point(226, 391)
        Me._Label_5.Name = "_Label_5"
        Me._Label_5.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_5.Size = New System.Drawing.Size(64, 17)
        Me._Label_5.TabIndex = 28
        Me._Label_5.Text = "Drive:"
        '
        'CBMDiskName
        '
        Me.CBMDiskName.BackColor = System.Drawing.Color.FromArgb(CType(CType(131, Byte), Integer), CType(CType(131, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.CBMDiskName.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.CBMDiskName.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDiskName.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDiskName.ForeColor = System.Drawing.Color.FromArgb(CType(CType(0, Byte), Integer), CType(CType(0, Byte), Integer), CType(CType(192, Byte), Integer))
        Me.CBMDiskName.Location = New System.Drawing.Point(8, 16)
        Me.CBMDiskName.Name = "CBMDiskName"
        Me.CBMDiskName.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDiskName.Size = New System.Drawing.Size(154, 19)
        Me.CBMDiskName.TabIndex = 14
        '
        'CBMDiskID
        '
        Me.CBMDiskID.BackColor = System.Drawing.Color.FromArgb(CType(CType(131, Byte), Integer), CType(CType(131, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.CBMDiskID.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.CBMDiskID.Cursor = System.Windows.Forms.Cursors.Default
        Me.CBMDiskID.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.CBMDiskID.ForeColor = System.Drawing.Color.FromArgb(CType(CType(0, Byte), Integer), CType(CType(0, Byte), Integer), CType(CType(192, Byte), Integer))
        Me.CBMDiskID.Location = New System.Drawing.Point(168, 16)
        Me.CBMDiskID.Name = "CBMDiskID"
        Me.CBMDiskID.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.CBMDiskID.Size = New System.Drawing.Size(57, 19)
        Me.CBMDiskID.TabIndex = 13
        Me.CBMDiskID.TextAlign = System.Drawing.ContentAlignment.TopCenter
        '
        '_Label_3
        '
        Me._Label_3.BackColor = System.Drawing.Color.Transparent
        Me._Label_3.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_3.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_3.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_3.Location = New System.Drawing.Point(223, 40)
        Me._Label_3.Name = "_Label_3"
        Me._Label_3.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_3.Size = New System.Drawing.Size(68, 17)
        Me._Label_3.TabIndex = 12
        Me._Label_3.Text = "Disk:"
        '
        '_Label_4
        '
        Me._Label_4.BackColor = System.Drawing.Color.Transparent
        Me._Label_4.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_4.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_4.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_4.Location = New System.Drawing.Point(226, 204)
        Me._Label_4.Name = "_Label_4"
        Me._Label_4.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_4.Size = New System.Drawing.Size(73, 17)
        Me._Label_4.TabIndex = 11
        Me._Label_4.Text = "File(s):"
        '
        'Frame2
        '
        Me.Frame2.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Frame2.BackColor = System.Drawing.SystemColors.Control
        Me.Frame2.Controls.Add(Me.BlockText)
        Me.Frame2.Controls.Add(Me.KBText)
        Me.Frame2.Controls.Add(Me.Drive1)
        Me.Frame2.Controls.Add(Me.MakeDir)
        Me.Frame2.Controls.Add(Me.Dir1)
        Me.Frame2.Controls.Add(Me.PCDirectory)
        Me.Frame2.Controls.Add(Me.PCRefresh)
        Me.Frame2.Controls.Add(Me.RunFile)
        Me.Frame2.Controls.Add(Me.PCRename)
        Me.Frame2.Controls.Add(Me.PCWorkingDir)
        Me.Frame2.Controls.Add(Me.PCDelete)
        Me.Frame2.Controls.Add(Me._Label_7)
        Me.Frame2.Controls.Add(Me._Label_6)
        Me.Frame2.Controls.Add(Me._Label_1)
        Me.Frame2.Controls.Add(Me._Label_2)
        Me.Frame2.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Frame2.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Frame2.Location = New System.Drawing.Point(3, 0)
        Me.Frame2.Name = "Frame2"
        Me.Frame2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Frame2.Size = New System.Drawing.Size(310, 494)
        Me.Frame2.TabIndex = 4
        Me.Frame2.TabStop = False
        Me.Frame2.Text = "Local Drive"
        '
        'BlockText
        '
        Me.BlockText.AcceptsReturn = True
        Me.BlockText.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.BlockText.BackColor = System.Drawing.SystemColors.Window
        Me.BlockText.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.BlockText.Enabled = False
        Me.BlockText.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.BlockText.ForeColor = System.Drawing.SystemColors.WindowText
        Me.BlockText.Location = New System.Drawing.Point(228, 430)
        Me.BlockText.MaxLength = 0
        Me.BlockText.Name = "BlockText"
        Me.BlockText.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.BlockText.Size = New System.Drawing.Size(73, 20)
        Me.BlockText.TabIndex = 32
        Me.BlockText.Text = "0 Blocks"
        '
        'KBText
        '
        Me.KBText.AcceptsReturn = True
        Me.KBText.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.KBText.BackColor = System.Drawing.SystemColors.Window
        Me.KBText.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.KBText.Enabled = False
        Me.KBText.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.KBText.ForeColor = System.Drawing.SystemColors.WindowText
        Me.KBText.Location = New System.Drawing.Point(228, 406)
        Me.KBText.MaxLength = 0
        Me.KBText.Name = "KBText"
        Me.KBText.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.KBText.Size = New System.Drawing.Size(73, 20)
        Me.KBText.TabIndex = 31
        Me.KBText.Text = "0 KB"
        '
        'Drive1
        '
        Me.Drive1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Drive1.BackColor = System.Drawing.SystemColors.Window
        Me.Drive1.Cursor = System.Windows.Forms.Cursors.Default
        Me.Drive1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Drive1.ForeColor = System.Drawing.SystemColors.WindowText
        Me.Drive1.FormattingEnabled = True
        Me.Drive1.Location = New System.Drawing.Point(8, 13)
        Me.Drive1.Name = "Drive1"
        Me.Drive1.Size = New System.Drawing.Size(213, 21)
        Me.Drive1.TabIndex = 26
        '
        'MakeDir
        '
        Me.MakeDir.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.MakeDir.BackColor = System.Drawing.SystemColors.Control
        Me.MakeDir.Cursor = System.Windows.Forms.Cursors.Default
        Me.MakeDir.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.MakeDir.ForeColor = System.Drawing.SystemColors.ControlText
        Me.MakeDir.Location = New System.Drawing.Point(228, 88)
        Me.MakeDir.Name = "MakeDir"
        Me.MakeDir.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.MakeDir.Size = New System.Drawing.Size(73, 25)
        Me.MakeDir.TabIndex = 29
        Me.MakeDir.Text = "Make Dir"
        Me.MakeDir.UseVisualStyleBackColor = False
        '
        'Dir1
        '
        Me.Dir1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Dir1.BackColor = System.Drawing.SystemColors.Window
        Me.Dir1.Cursor = System.Windows.Forms.Cursors.Default
        Me.Dir1.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Dir1.ForeColor = System.Drawing.SystemColors.WindowText
        Me.Dir1.FormattingEnabled = True
        Me.Dir1.IntegralHeight = False
        Me.Dir1.Location = New System.Drawing.Point(8, 40)
        Me.Dir1.Name = "Dir1"
        Me.Dir1.Size = New System.Drawing.Size(213, 111)
        Me.Dir1.TabIndex = 27
        '
        'PCDirectory
        '
        Me.PCDirectory.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PCDirectory.BackColor = System.Drawing.SystemColors.Window
        Me.PCDirectory.Cursor = System.Windows.Forms.Cursors.Default
        Me.PCDirectory.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PCDirectory.ForeColor = System.Drawing.SystemColors.WindowText
        Me.PCDirectory.FormattingEnabled = True
        Me.PCDirectory.Location = New System.Drawing.Point(8, 152)
        Me.PCDirectory.Name = "PCDirectory"
        Me.PCDirectory.Pattern = "*.*"
        Me.PCDirectory.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended
        Me.PCDirectory.Size = New System.Drawing.Size(213, 298)
        Me.PCDirectory.TabIndex = 20
        '
        'PCRefresh
        '
        Me.PCRefresh.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PCRefresh.BackColor = System.Drawing.SystemColors.Control
        Me.PCRefresh.Cursor = System.Windows.Forms.Cursors.Default
        Me.PCRefresh.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PCRefresh.ForeColor = System.Drawing.SystemColors.ControlText
        Me.PCRefresh.Location = New System.Drawing.Point(228, 56)
        Me.PCRefresh.Name = "PCRefresh"
        Me.PCRefresh.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PCRefresh.Size = New System.Drawing.Size(73, 25)
        Me.PCRefresh.TabIndex = 19
        Me.PCRefresh.Text = "Refresh"
        Me.PCRefresh.UseVisualStyleBackColor = False
        '
        'RunFile
        '
        Me.RunFile.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.RunFile.BackColor = System.Drawing.SystemColors.Control
        Me.RunFile.Cursor = System.Windows.Forms.Cursors.Default
        Me.RunFile.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.RunFile.ForeColor = System.Drawing.SystemColors.ControlText
        Me.RunFile.Location = New System.Drawing.Point(228, 240)
        Me.RunFile.Name = "RunFile"
        Me.RunFile.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.RunFile.Size = New System.Drawing.Size(73, 25)
        Me.RunFile.TabIndex = 18
        Me.RunFile.Text = "Run / View"
        Me.RunFile.UseVisualStyleBackColor = False
        '
        'PCRename
        '
        Me.PCRename.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PCRename.BackColor = System.Drawing.SystemColors.Control
        Me.PCRename.Cursor = System.Windows.Forms.Cursors.Default
        Me.PCRename.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PCRename.ForeColor = System.Drawing.SystemColors.ControlText
        Me.PCRename.Location = New System.Drawing.Point(228, 176)
        Me.PCRename.Name = "PCRename"
        Me.PCRename.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PCRename.Size = New System.Drawing.Size(73, 25)
        Me.PCRename.TabIndex = 17
        Me.PCRename.Text = "Rename"
        Me.PCRename.UseVisualStyleBackColor = False
        '
        'PCWorkingDir
        '
        Me.PCWorkingDir.AcceptsReturn = True
        Me.PCWorkingDir.Anchor = CType(((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PCWorkingDir.BackColor = System.Drawing.SystemColors.Window
        Me.PCWorkingDir.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.PCWorkingDir.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PCWorkingDir.ForeColor = System.Drawing.SystemColors.WindowText
        Me.PCWorkingDir.Location = New System.Drawing.Point(8, 470)
        Me.PCWorkingDir.MaxLength = 0
        Me.PCWorkingDir.Name = "PCWorkingDir"
        Me.PCWorkingDir.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PCWorkingDir.Size = New System.Drawing.Size(293, 20)
        Me.PCWorkingDir.TabIndex = 16
        Me.PCWorkingDir.Text = "c:\"
        '
        'PCDelete
        '
        Me.PCDelete.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PCDelete.BackColor = System.Drawing.SystemColors.Control
        Me.PCDelete.Cursor = System.Windows.Forms.Cursors.Default
        Me.PCDelete.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.PCDelete.ForeColor = System.Drawing.SystemColors.ControlText
        Me.PCDelete.Location = New System.Drawing.Point(228, 208)
        Me.PCDelete.Name = "PCDelete"
        Me.PCDelete.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.PCDelete.Size = New System.Drawing.Size(73, 25)
        Me.PCDelete.TabIndex = 15
        Me.PCDelete.Text = "Delete"
        Me.PCDelete.UseVisualStyleBackColor = False
        '
        '_Label_7
        '
        Me._Label_7.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me._Label_7.BackColor = System.Drawing.Color.Transparent
        Me._Label_7.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_7.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_7.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_7.Location = New System.Drawing.Point(8, 454)
        Me._Label_7.Name = "_Label_7"
        Me._Label_7.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_7.Size = New System.Drawing.Size(145, 17)
        Me._Label_7.TabIndex = 35
        Me._Label_7.Text = "Current Directory:"
        '
        '_Label_6
        '
        Me._Label_6.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me._Label_6.BackColor = System.Drawing.Color.Transparent
        Me._Label_6.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_6.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_6.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_6.Location = New System.Drawing.Point(228, 390)
        Me._Label_6.Name = "_Label_6"
        Me._Label_6.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_6.Size = New System.Drawing.Size(73, 17)
        Me._Label_6.TabIndex = 30
        Me._Label_6.Text = "Selected:"
        '
        '_Label_1
        '
        Me._Label_1.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me._Label_1.BackColor = System.Drawing.Color.Transparent
        Me._Label_1.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_1.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_1.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_1.Location = New System.Drawing.Point(228, 40)
        Me._Label_1.Name = "_Label_1"
        Me._Label_1.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_1.Size = New System.Drawing.Size(73, 17)
        Me._Label_1.TabIndex = 22
        Me._Label_1.Text = "Disk:"
        '
        '_Label_2
        '
        Me._Label_2.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me._Label_2.BackColor = System.Drawing.Color.Transparent
        Me._Label_2.Cursor = System.Windows.Forms.Cursors.Default
        Me._Label_2.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me._Label_2.ForeColor = System.Drawing.SystemColors.ControlText
        Me._Label_2.Location = New System.Drawing.Point(228, 160)
        Me._Label_2.Name = "_Label_2"
        Me._Label_2.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me._Label_2.Size = New System.Drawing.Size(73, 17)
        Me._Label_2.TabIndex = 21
        Me._Label_2.Text = "File(s):"
        '
        'LogGroup
        '
        Me.LogGroup.Controls.Add(Me.Log)
        Me.LogGroup.Dock = System.Windows.Forms.DockStyle.Fill
        Me.LogGroup.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LogGroup.ForeColor = System.Drawing.SystemColors.ControlText
        Me.LogGroup.Location = New System.Drawing.Point(0, 0)
        Me.LogGroup.Name = "LogGroup"
        Me.LogGroup.Size = New System.Drawing.Size(313, 497)
        Me.LogGroup.TabIndex = 1
        Me.LogGroup.TabStop = False
        Me.LogGroup.Text = "Log"
        '
        'Log
        '
        Me.Log.Dock = System.Windows.Forms.DockStyle.Fill
        Me.Log.Font = New System.Drawing.Font("Courier New", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Log.Location = New System.Drawing.Point(3, 16)
        Me.Log.Name = "Log"
        Me.Log.ReadOnly = True
        Me.Log.Size = New System.Drawing.Size(307, 478)
        Me.Log.TabIndex = 1
        Me.Log.Text = ""
        '
        'MainForm
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 14.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.SystemColors.Control
        Me.ClientSize = New System.Drawing.Size(992, 497)
        Me.Controls.Add(Me.SplitContainer1)
        Me.Cursor = System.Windows.Forms.Cursors.Default
        Me.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Location = New System.Drawing.Point(4, 23)
        Me.Name = "MainForm"
        Me.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Text = "GUI4CBM4WIN"
        CType(Me.Label, System.ComponentModel.ISupportInitialize).EndInit()
        Me.SplitContainer1.Panel1.ResumeLayout(False)
        Me.SplitContainer1.Panel2.ResumeLayout(False)
        Me.SplitContainer1.ResumeLayout(False)
        Me.CBMDrive.ResumeLayout(False)
        Me.Frame2.ResumeLayout(False)
        Me.Frame2.PerformLayout()
        Me.LogGroup.ResumeLayout(False)
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents SplitContainer1 As System.Windows.Forms.SplitContainer
    Public WithEvents CBMDrive As System.Windows.Forms.GroupBox
    Public WithEvents CBMValidate As System.Windows.Forms.Button
    Public WithEvents CBMInitialize As System.Windows.Forms.Button
    Public WithEvents CBMFormat As System.Windows.Forms.Button
    Public WithEvents CBMDirectory As System.Windows.Forms.ListBox
    Public WithEvents CBMReset As System.Windows.Forms.Button
    Public WithEvents CBMDriveStatus As System.Windows.Forms.Button
    Public WithEvents CBMRefresh As System.Windows.Forms.Button
    Public WithEvents CBMRename As System.Windows.Forms.Button
    Public WithEvents CBMScratch As System.Windows.Forms.Button
    Public WithEvents _Label_0 As System.Windows.Forms.Label
    Public WithEvents LastStatus As System.Windows.Forms.Label
    Public WithEvents _Label_5 As System.Windows.Forms.Label
    Public WithEvents CBMDiskName As System.Windows.Forms.Label
    Public WithEvents CBMDiskID As System.Windows.Forms.Label
    Public WithEvents _Label_3 As System.Windows.Forms.Label
    Public WithEvents _Label_4 As System.Windows.Forms.Label
    Public WithEvents Frame2 As System.Windows.Forms.GroupBox
    Public WithEvents BlockText As System.Windows.Forms.TextBox
    Public WithEvents KBText As System.Windows.Forms.TextBox
    Public WithEvents Drive1 As Microsoft.VisualBasic.Compatibility.VB6.DriveListBox
    Public WithEvents MakeDir As System.Windows.Forms.Button
    Public WithEvents Dir1 As Microsoft.VisualBasic.Compatibility.VB6.DirListBox
    Public WithEvents PCDirectory As Microsoft.VisualBasic.Compatibility.VB6.FileListBox
    Public WithEvents PCRefresh As System.Windows.Forms.Button
    Public WithEvents RunFile As System.Windows.Forms.Button
    Public WithEvents PCRename As System.Windows.Forms.Button
    Public WithEvents PCWorkingDir As System.Windows.Forms.TextBox
    Public WithEvents PCDelete As System.Windows.Forms.Button
    Public WithEvents _Label_7 As System.Windows.Forms.Label
    Public WithEvents _Label_6 As System.Windows.Forms.Label
    Public WithEvents _Label_1 As System.Windows.Forms.Label
    Public WithEvents _Label_2 As System.Windows.Forms.Label
    Friend WithEvents Drive As System.Windows.Forms.ComboBox
    Friend WithEvents LogGroup As System.Windows.Forms.GroupBox
    Friend WithEvents Log As System.Windows.Forms.RichTextBox
#End Region
End Class