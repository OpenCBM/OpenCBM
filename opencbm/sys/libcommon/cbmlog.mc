MessageIdTypedef=NTSTATUS

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0
               Opencbm=0x45:FACILITY_OPENCBM
              )

LanguageNames=(English = 0x0409:msg00001
               German  = 0x0407:msg00002
              ) 

MessageId=0x0001 Facility=Opencbm Severity=Informational SymbolicName=CBM_STARTED
Language=English
%1 successfully started.
.
Language = German
%1 erfolgreich gestarted.
.

MessageId=0x0002 Facility=Opencbm Severity=Error SymbolicName=CBM_START_FAILED
Language=English
Starting of %1 failed at %2.
.
Language = German
Der Start von %1 ist fehlgeschlagen bei %2.
.

MessageId=0x0003 Facility=Opencbm Severity=Error SymbolicName=CBM_INSUFFICIENT_RESOURCES
Language=English
Memory allocation failed for the device %1.
.
Language = German
Gerät %1: Speicheranforderung fehlgeschlagen.
.

MessageId=0x0004 Facility=Opencbm Severity=Warning SymbolicName=CBM_NO_ISR
Language=English
Could not initialize the interrupt for %1. Will try without interrupt,
but this is not recommended.
.
Language = German
Der Interrupt für %1 konnte nicht angefordert werden. Es wird versucht,
ohne Interrupt zu arbeiten. Dies ist aber nicht empfohlen.
.

MessageId=0x0005 Facility=Opencbm Severity=Informational SymbolicName=CBM_IEC_INIT
Language=English
IEC bus initialized. Using %2%3 cable.
.
Language = German
Der IEC bus wurde initialisiert. Ein %2%3 Kabel wird verwendet.
.
