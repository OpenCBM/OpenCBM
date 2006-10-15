VERSION 5.00
Begin VB.Form Waiting 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Working..."
   ClientHeight    =   1380
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   2790
   ControlBox      =   0   'False
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1380
   ScaleWidth      =   2790
   StartUpPosition =   1  'CenterOwner
   Begin VB.Timer LEDTimer 
      Interval        =   300
      Left            =   600
      Top             =   840
   End
   Begin VB.CommandButton Cancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   2160
      TabIndex        =   1
      Top             =   720
      Visible         =   0   'False
      Width           =   1455
   End
   Begin VB.Shape LED 
      BackColor       =   &H00000000&
      BackStyle       =   1  'Opaque
      BorderWidth     =   2
      Height          =   255
      Left            =   1080
      Top             =   960
      Width           =   615
   End
   Begin VB.Label Label 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Please wait."
      Height          =   735
      Left            =   8
      TabIndex        =   0
      Top             =   240
      Width           =   2775
   End
End
Attribute VB_Name = "Waiting"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private IsOnTop As Boolean

Public Property Let AlwaysOnTop(ByVal bState As Boolean)
  Dim lFlag As Long
  If bState Then lFlag = HWND_TOPMOST Else lFlag = HWND_NOTOPMOST
  IsOnTop = bState
  SetWindowPos Me.hWnd, lFlag, 0&, 0&, 0&, 0&, (SWP_NOSIZE Or SWP_NOMOVE)
End Property

Private Sub Form_Load()
    Me.AlwaysOnTop = True
End Sub

'There's no "elegant" way to abort a running cbm4win process, short of killing the PID, so this is left for future...

Private Sub Cancel_Click()
    Me.Hide
End Sub

Private Sub LEDTimer_Timer()
    If LED.BackColor = vbRed Then
        LED.BackColor = vbBlack
    Else
        LED.BackColor = vbRed
    End If
End Sub
