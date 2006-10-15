VERSION 5.00
Begin VB.Form Prompt 
   BorderStyle     =   1  'Fixed Single
   ClientHeight    =   2160
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4005
   Icon            =   "Prompt.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2160
   ScaleWidth      =   4005
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Reply 
      Height          =   285
      Left            =   360
      TabIndex        =   0
      Top             =   1080
      Width           =   3135
   End
   Begin VB.CommandButton OK 
      Caption         =   "OK"
      Height          =   375
      Left            =   360
      TabIndex        =   3
      Top             =   1560
      Width           =   1335
   End
   Begin VB.CommandButton Cancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   2160
      TabIndex        =   1
      Top             =   1560
      Width           =   1335
   End
   Begin VB.Label Label 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Question"
      Height          =   495
      Left            =   0
      TabIndex        =   2
      Top             =   120
      Width           =   3975
   End
End
Attribute VB_Name = "Prompt"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Public LastResult As String

Public Sub Ask(Q As String, Optional ClearLast = True)
    LastResult = ""
    Label.Caption = Q
    
    If (ClearLast) Then Reply.Text = ""
    
    Me.Show vbModal
End Sub


Private Sub Cancel_Click()
    LastResult = CANCELSTRING
    Me.Hide
End Sub

Private Sub OK_Click()
    LastResult = Reply.Text
    Me.Hide
End Sub

Private Sub Reply_KeyDown(KeyCode As Integer, Shift As Integer)
    'Enable Enter Key
    If (KeyCode = 13) Then OK_Click
End Sub
