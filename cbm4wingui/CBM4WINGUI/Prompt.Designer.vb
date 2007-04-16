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

<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> Partial Class Prompt
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
    Public WithEvents Reply As System.Windows.Forms.TextBox
    Public WithEvents OK As System.Windows.Forms.Button
    Public WithEvents Cancel As System.Windows.Forms.Button
    Public WithEvents Label As System.Windows.Forms.Label
    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Prompt))
        Me.components = New System.ComponentModel.Container()
        Me.ToolTip1 = New System.Windows.Forms.ToolTip(components)
        Me.Reply = New System.Windows.Forms.TextBox
        Me.OK = New System.Windows.Forms.Button
        Me.Cancel = New System.Windows.Forms.Button
        Me.Label = New System.Windows.Forms.Label
        Me.SuspendLayout()
        Me.ToolTip1.Active = True
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.ClientSize = New System.Drawing.Size(268, 145)
        Me.Location = New System.Drawing.Point(3, 22)
        Me.Icon = CType(resources.GetObject("Prompt.Icon"), System.Drawing.Icon)
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
        Me.Name = "Prompt"
        Me.Reply.AutoSize = False
        Me.Reply.Size = New System.Drawing.Size(209, 19)
        Me.Reply.Location = New System.Drawing.Point(24, 72)
        Me.Reply.TabIndex = 0
        Me.Reply.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Reply.AcceptsReturn = True
        Me.Reply.TextAlign = System.Windows.Forms.HorizontalAlignment.Left
        Me.Reply.BackColor = System.Drawing.SystemColors.Window
        Me.Reply.CausesValidation = True
        Me.Reply.Enabled = True
        Me.Reply.ForeColor = System.Drawing.SystemColors.WindowText
        Me.Reply.HideSelection = True
        Me.Reply.ReadOnly = False
        Me.Reply.MaxLength = 0
        Me.Reply.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.Reply.Multiline = False
        Me.Reply.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Reply.ScrollBars = System.Windows.Forms.ScrollBars.None
        Me.Reply.TabStop = True
        Me.Reply.Visible = True
        Me.Reply.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.Reply.Name = "Reply"
        Me.OK.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.OK.Text = "OK"
        Me.OK.Size = New System.Drawing.Size(89, 25)
        Me.OK.Location = New System.Drawing.Point(24, 104)
        Me.OK.TabIndex = 3
        Me.OK.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.OK.BackColor = System.Drawing.SystemColors.Control
        Me.OK.CausesValidation = True
        Me.OK.Enabled = True
        Me.OK.ForeColor = System.Drawing.SystemColors.ControlText
        Me.OK.Cursor = System.Windows.Forms.Cursors.Default
        Me.OK.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.OK.TabStop = True
        Me.OK.Name = "OK"
        Me.Cancel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Cancel.Text = "Cancel"
        Me.Cancel.Size = New System.Drawing.Size(89, 25)
        Me.Cancel.Location = New System.Drawing.Point(144, 104)
        Me.Cancel.TabIndex = 1
        Me.Cancel.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Cancel.BackColor = System.Drawing.SystemColors.Control
        Me.Cancel.CausesValidation = True
        Me.Cancel.Enabled = True
        Me.Cancel.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Cancel.Cursor = System.Windows.Forms.Cursors.Default
        Me.Cancel.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Cancel.TabStop = True
        Me.Cancel.Name = "Cancel"
        Me.Label.TextAlign = System.Drawing.ContentAlignment.TopCenter
        Me.Label.Text = "Question"
        Me.Label.Size = New System.Drawing.Size(265, 33)
        Me.Label.Location = New System.Drawing.Point(0, 8)
        Me.Label.TabIndex = 2
        Me.Label.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label.BackColor = System.Drawing.Color.Transparent
        Me.Label.Enabled = True
        Me.Label.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Label.Cursor = System.Windows.Forms.Cursors.Default
        Me.Label.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Label.UseMnemonic = True
        Me.Label.Visible = True
        Me.Label.AutoSize = False
        Me.Label.BorderStyle = System.Windows.Forms.BorderStyle.None
        Me.Label.Name = "Label"
        Me.Controls.Add(Reply)
        Me.Controls.Add(OK)
        Me.Controls.Add(Cancel)
        Me.Controls.Add(Label)
        Me.ResumeLayout(False)
        Me.PerformLayout()
    End Sub
#End Region
End Class