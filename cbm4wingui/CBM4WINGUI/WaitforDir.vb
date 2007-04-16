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

Option Strict Off
Option Explicit On
Friend Class Waiting
	Inherits System.Windows.Forms.Form

	
	
	Private IsOnTop As Boolean
	
	Public WriteOnly Property AlwaysOnTop() As Boolean
		Set(ByVal Value As Boolean)
			Dim lFlag As Integer
			If Value Then lFlag = HWND_TOPMOST Else lFlag = HWND_NOTOPMOST
			IsOnTop = Value
			SetWindowPos(Me.Handle.ToInt32, lFlag, 0, 0, 0, 0, (SWP_NOSIZE Or SWP_NOMOVE))
		End Set
	End Property
	
	Private Sub Waiting_Load(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles MyBase.Load
		Me.AlwaysOnTop = True
	End Sub
	
	'There's no "elegant" way to abort a running cbm4win process, short of killing the PID, so this is left for future...
	
	Private Sub Cancel_Click(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles Cancel.Click
		Me.Hide()
	End Sub
	
	Private Sub LEDTimer_Tick(ByVal eventSender As System.Object, ByVal eventArgs As System.EventArgs) Handles LEDTimer.Tick
		If System.Drawing.ColorTranslator.ToOle(LED.BackColor) = System.Drawing.ColorTranslator.ToOle(System.Drawing.Color.Red) Then
			LED.BackColor = System.Drawing.Color.Black
		Else
			LED.BackColor = System.Drawing.Color.Red
		End If
	End Sub
End Class