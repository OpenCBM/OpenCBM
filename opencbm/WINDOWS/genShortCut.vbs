Set wsc = WScript.CreateObject("WScript.Shell")
Set lnk = wsc.CreateShortcut(WScript.Arguments.item(0) + "\Desktop\OpenCBM.lnk")

lnk.targetpath = "%ComSpec%"
lnk.arguments = "/k ""PATH " + WScript.Arguments.item(1) + ";%PATH% && set OC=" + WScript.Arguments.item(1) + " && cd /d %USERPROFILE% && cd %HOMEPATH% && echo. && echo OpenCBM v" + WScript.Arguments.item(2) + " environment && echo. && title OpenCBM v" + WScript.Arguments.item(2) + """"
lnk.workingdirectory = "%HOMEPATH%"
lnk.WindowStyle = 4
lnk.save
