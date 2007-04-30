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

<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> Partial Class Waiting
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
    Public WithEvents LEDTimer As System.Windows.Forms.Timer
    Public WithEvents Cancel As System.Windows.Forms.Button
    Public WithEvents LED As System.Windows.Forms.Label
    Public WithEvents Label As System.Windows.Forms.Label
    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Waiting))
        Me.components = New System.ComponentModel.Container()
        Me.ToolTip1 = New System.Windows.Forms.ToolTip(components)
        Me.LEDTimer = New System.Windows.Forms.Timer(components)
        Me.Cancel = New System.Windows.Forms.Button
        Me.LED = New System.Windows.Forms.Label
        Me.Label = New System.Windows.Forms.Label
        Me.SuspendLayout()
        Me.ToolTip1.Active = True
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.Text = "Working..."
        Me.ClientSize = New System.Drawing.Size(187, 93)
        Me.Location = New System.Drawing.Point(3, 22)
        Me.ControlBox = False
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent
        Me.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.SystemColors.Control
        Me.Enabled = True
        Me.KeyPreview = False
        Me.Cursor = System.Windows.Forms.Cursors.Default
        Me.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.ShowInTaskbar = True
        Me.HelpButton = False
        Me.WindowState = System.Windows.Forms.FormWindowState.Normal
        Me.Name = "Waiting"
        Me.LEDTimer.Interval = 300
        Me.LEDTimer.Enabled = True
        Me.Cancel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Cancel.Text = "Cancel"
        Me.Cancel.Size = New System.Drawing.Size(97, 25)
        Me.Cancel.Location = New System.Drawing.Point(144, 48)
        Me.Cancel.TabIndex = 1
        Me.Cancel.Visible = False
        Me.Cancel.Font = New System.Drawing.Font("Arial", 8.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Cancel.BackColor = System.Drawing.SystemColors.Control
        Me.Cancel.CausesValidation = True
        Me.Cancel.Enabled = True
        Me.Cancel.ForeColor = System.Drawing.SystemColors.ControlText
        Me.Cancel.Cursor = System.Windows.Forms.Cursors.Default
        Me.Cancel.RightToLeft = System.Windows.Forms.RightToLeft.No
        Me.Cancel.TabStop = True
        Me.Cancel.Name = "Cancel"
        Me.LED.BackColor = System.Drawing.Color.Black
        Me.LED.Size = New System.Drawing.Size(41, 17)
        Me.LED.Location = New System.Drawing.Point(72, 64)
        Me.LED.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.LED.Visible = True
        Me.LED.Name = "LED"
        Me.Label.TextAlign = System.Drawing.ContentAlignment.TopCenter
        Me.Label.Text = "Please wait."
        Me.Label.Size = New System.Drawing.Size(185, 49)
        Me.Label.Location = New System.Drawing.Point(1, 16)
        Me.Label.TabIndex = 0
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
        Me.Controls.Add(Cancel)
        Me.Controls.Add(LED)
        Me.Controls.Add(Label)
        Me.ResumeLayout(False)
        Me.PerformLayout()
    End Sub
#End Region
End Class