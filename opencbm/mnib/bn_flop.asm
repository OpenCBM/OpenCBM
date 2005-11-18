; Burst Nibbler - main floppy routines
; V1.0 Assembled code matches original code exactly

; Fragen: $05f7: Formatieren eines Tracks, wird hier SYNC geloescht ?

; $c2: current track
; $c3-$c8: density statistic bins

* = $0300
_flop_main             SEI  
                       LDA  #$ee
                       STA  $1c0c
                       LDA  #$0b
                       STA  $180c
                       LDA  $1c00	;
                       AND  #$f3	; motor off, LED off
                       STA  $1c00	;
                       LDA  #$24	;
                       STA  $c2		; current halftrack = 36

					; MAIN LOOP
_main_loop             LDX  #$45	;
                       TXS  		; reset stack
                       TYA  		; return value from last call
                       JSR  _send_byte	; parallel-send data byte to C64
                       LDA  #>(_main_loop-1)
                       PHA  		; set RTS to main loop
                       LDA  #<(_main_loop-1)
                       PHA
                       JSR  _read_command
                       ASL  		; * 2 for 16 bit index
                       TAX
                       LDA  _command_table+1,X
                       PHA
                       LDA  _command_table,X
                       PHA
                       RTS  		; -> to command function

;----------------------------------------
					; read out track w/out waiting for Sync
_read_track            JSR  _send_byte	; parallel-send data byte to C64
                       LDA  #$ff	; 
                       STA  $1800	; send handshake
                       LDX  #$20	; read $2000 GCR bytes
                       STX  $c0		; (index for read loop)
                       CLV  		;
                       BNE  _read_gcr_loop	; read without waiting for Sync

;----------------------------------------
					; read out track after Sync
_read_after_sync       JSR  _send_byte	; parallel-send data byte to C64
                       LDA  #$ff
                       STA  $1800	; send handshake
                       LDX  #$20	; read $2000 GCR bytes
                       STX  $c0

_in_sync               BIT  $1c00
                       BMI  _in_sync	; wait for end of Sync
                       LDX  $1c01	; read GCR byte
                       CLV 
_wait_for_byte         BVC  _wait_for_byte

_read_gcr_loop         BVS  _read_gcr_1	; wait for next GCR byte
                       BVS  _read_gcr_1
                       BVS  _read_gcr_1
                       BVS  _read_gcr_1
                       BVS  _read_gcr_1
                       BVS  _read_gcr_1
                       BVS  _read_gcr_1
                       LDX  #$ff	; if pause too long, send 0xff (Sync?)
                       BVS  _read_gcr_1
                       EOR  #$ff	; toggle handshake value
                       BVS  _read_gcr_2	; read and transfer GCR byte
                       STX  $1801	; PA, port A (8 bit parallel data)
                       BVS  _read_gcr_2	; read and transfer GCR byte
                       STA  $1800	; send handshake (send 0xff byte)
                       INY  		;
_rtp6                  BNE  _read_gcr_loop
                       DEC  $c0		; total byte counter hb 
                       BEQ  _read_track_end
_rtp5                  BVC  _read_gcr_loop

_read_gcr_1            LDX  $1c01	; read GCR byte
                       CLV
                       EOR  #$ff	; toggle handshake flag
                       STX  $1801	; PA, port A (8 bit parallel data)
                       STA  $1800	; send handshake
                       INY
_rtp4                  BNE  _read_gcr_loop
                       DEC  $c0	
_rtp3                  BNE  _read_gcr_loop
_read_track_end        STY  $1800	; send handshake: $00
                       RTS   		; done reading

_read_gcr_2            LDX  $1c01	; read GCR byte
                       CLV  		;
                       STX  $1801	; PA, port A (8 bit parallel data)
                       STA  $1800	; send handshake
                       INY  		;
_rtp2                  BNE  _read_gcr_loop
                       DEC  $c0		; total byte counter hb
_rtp1                  BNE  _read_gcr_loop
                       STY  $1800	; send handshake: $00
                       RTS  		; done reading

;----------------------------------------
					; step motor to destination halftrack
_step_dest             JSR  _read_byte	; read byte from parallel data port
                       LDX  #$01	; step value: step up
                       CMP  $c2		; compare with current track (CARRY!!!)
                       BEQ  _step_dest_end; destination track == current -> RTS
                       PHA  		; push destination track
                       SBC  $c2		; calculate track difference
                       BPL  _step_up	; destination track > current ->

                       EOR  #$ff	; else negate track difference
                       LDX  #$ff	; step value: step down

_step_up               TAY  		; # of tracks to step
_step_loop             TXA  		; step value
                       CLC  		;
                       ADC  $1c00	; 
                       AND  #$03	;
                       STA  $c0		; temp store
                       LDA  $1c00	;
                       AND  #$fc	; mask off stepper bits
                       ORA  $c0		;
                       STA  $1c00	; perform half step
                       LDA  #$04	;
                       STA  $c1		;

                       LDA  #$00	; busy wait $0400 times
                       STA  $c0		;
_stepL1                DEC  $c0		;
                       BNE  _stepL1	;
                       DEC  $c1		;
                       BNE  _stepL1	;

                       DEY		;
                       BNE  _step_loop	; repeat for # of halftracks
                       PLA  		; pull destination track
                       STA  $c2		; current track = destination
_step_dest_end         RTS  

;----------------------------------------
					; adjust routines to density value
_adjust_density        JSR  _read_byte	; read byte from parallel data port
                       STA  _rtp1+1
                       CLC  		;
                       ADC  #$04	;
                       STA  _rtp2+1	; adjust read routines to the
                       ADC  #$11	;  density (timing) value read
                       STA  _rtp3+1	;  from computer
                       ADC  #$04	;
                       STA  _rtp4+1	;
                       ADC  #$13	;
                       STA  _rtp5+1	;
                       ADC  #$06	;
                       STA  _rtp6+1	;

;----------------------------------------
				 	; set $1c00 bits (head/motor)
_set_1c00              JSR  _read_byte	; read byte from parallel data port
                       STA  $c0		; $1c00 mask
                       JSR  _read_byte	; read byte from parallel data port
                       STA  $c1		; new bit value for $1c00
                       LDA  $1c00	;
                       AND  $c0		; mask off $1c00 bits
                       ORA  $c1		; set new $1c00 bits
                       STA  $1c00	;
                       RTS  		;

;----------------------------------------
					; detect 'killer tracks'
_detect_killer         LDX  #$80	;
                       STY  $c0		; $c0 = 0
_dkL1                  LDA  $1c00	; wait for SYNC
                       BPL  _dk_sync	; if SYNC found, check for 'killer track'
                       DEY  		;
                       BNE  _dkL1	; wait max. $8000 times for at least one SYNC
                       DEX  		;
                       BNE  _dkL1	;
                       LDY  #$40	; track doesn't contain SYNC
                       RTS  		; -> $40 = track has no SYNC

					; try to read 256 bytes
					;     within 0x10000 cycles
_dk_sync               LDX  #$00	;
                       LDA  $1c01	; read GCR byte
                       CLV  		;
_dkL2                  DEY  		;
                       BNE  _dkWait	; wait max $10000 times
                       DEX  		;
                       BEQ  _dk_killer	; timeout, not enough bytes found ->
_dkWait                BVC  _dkL2	;
                       CLV  		;
                       DEC  $c0		; check for at least 256 bytes in track
                       BNE  _dkWait	;

                       LDY  #$00	; track contains at least 256 bytes
                       RTS  		; -> $00 = track OK

_dk_killer             LDY  #$80	; track doesn't contain 256 bytes
                       RTS  		; -> $80 = killer track (too many syncs)

;----------------------------------------

;----------------------------------------
;--- Density Scan for current track   ---
;----------------------------------------
_scan_density          LDX  #$05	;
_scL1                  STY  $c3,X	; reset bit-rate statistic
                       DEX  		;
                       BPL  _scL1	;

_sc_retry              CLV  		;
_scW1                  BVC  _scW1	; wait for GCR byte
                       CLV  		;
                       LDA  $1c01	; read GCR byte
                       PHA  		;
                       PLA  		; (busy wait timing)
                       PHA  		;
                       PLA  		;

_scL2                  NOP  		;
                       BVS  _scJ1	; 
                       BVS  _scJ2	;
                       BVS  _scJ3	; measure bit-rate between bytes
                       BVS  _scJ4	;
                       BVS  _scJ5	;
                       BVS  _scJ6	;
                       BNE  _sc_retry	;-> time too long, retry with next pair

_scJ1                  LDX  #$00	; bit-rate = 0
                       BEQ  _scJ7	;
_scJ2                  LDX  #$01	; bit-rate = 1
                       BNE  _scJ7	;
_scJ3                  LDX  #$02	; bit-rate = 2
                       BNE  _scJ7	;
_scJ4                  LDX  #$03	; bit-rate = 3
                       BNE  _scJ7	;
_scJ5                  LDX  #$04	; bit-rate = 4
                       BNE  _scJ7	;
_scJ6                  LDX  #$05	; bit-rate = 5
                       BNE  _scJ7	;

_scJ7                  CLV  		;
                       ; INC  $00c3,X	; adjust statistic for bit-rate X
                       .byte $fe,$c3,$00; INC  $00c3,X (not supported by C64asm)
                       INY  		;
                       BPL  _scL2	;

                       LDY  #$00	;
_scL3                  LDA  $00c4,Y	; transfer density statistic 1-4 to C64
                       JSR  _send_byte	; parallel-send data byte to C64
                       INY  		;
                       CPY  #$04	;
                       BNE  _scL3	;

                       LDY  #$00	;
                       RTS  		;

;----------------------------------------
					; write track after variable Sync length
_write_aftr_sync       JSR  _read_byte	; read byte from parallel data port
                       STA  _write_sync+1; initial Sync length 
                       JSR  _read_byte	; read byte from parallel data port
                       STA  _wasB2+1	;  branch value after last written byte
                       JSR  _read_byte	; read byte from parallel data port
                       STA  _wasB1+1	; branch value if in Sync
_wasL1                 BIT  $1c00	;
_wasB1                 BMI  _wasL1	; -> branch to parameter #3

                       LDA  #$ce
                       STA  $1c0c
                       TYA  		;
                       STA  $1800	; send handshake
                       LDX  #$ff	;
                       STX  $1c03	; CA data direction head (0->$ff: write)
                       LDX  #$ff	;
                       STX  $1c01	; write the initial 0xff sync bytes
_write_sync            LDX  #$1e	; Initial Sync length = parameter #1
                       CLV  		;
_wasL2                 BVC  _wasL2	;
                       CLV  		;
                       INY  		; Sync length = parameter #1 * 0x100
                       BNE  _wasL2	;
                       DEX  		;
                       BPL  _wasL2	;

                       LDY  #$ee
_wasL3                 STX  $1c01	; write data byte to track (#$00)
                       CLV  		;
                       EOR  #$ff	; toggle handshake value
                       LDX  $1801	; PA, port A (8 bit parallel data)
                       STA  $1800	; handshake
_wasL4                 BVC  _wasL4	;
                       BNE  _wasL3	; write until 0x00 byte is read

                       LDA  #$55	; write a final 0x55 byte
                       STA  $1c01	; write 0x55 byte
_wasB2                 BNE  _wasJ1	; (branch value = parameter #2)
                       NOP  
                       NOP  
                       NOP  
_wasJ1                 NOP  
                       NOP  
                       STY  $1c0c
                       STX  $1c03	; CA data direction head ($ff->0: read)
                       STX  $1800	; handshake
                       LDY  #$00	; done writing
                       RTS  

;----------------------------------------
					; write a track on destination
_write_track           JSR  _read_byte	; read byte from parallel data port
                       STA  _wtB1+1	; can change Sync Branch value
_wtL1                  BIT  $1c00	; wait for end of Sync, if writing
_wtB1                  BMI  _wtL1	;  halftracks, and 'adjust target'

                       LDA  #$ce	;  selected, else BMI $0503
                       STA  $1c0c
                       TYA  
                       DEC  $1c03	; CA data direction head (0->$ff: write)
                       STA  $1800	; send handshake
                       LDX  #$55	; write 256x $55 bytes after Sync
                       STX  $1c01	;
_wtL2                  CLV  		;
_wtL3                  BVC  _wtL3	;
                       INY  		;
                       BNE  _wtL2	;

_wtL4                  STX  $1c01	; write GCR byte to disk
                       CLV  		;
                       EOR  #$ff	; toggle handshake value
                       LDX  $1801	; PA, port A (8 bit parallel data)
                       STA  $1800	; send handshake
_wtL5                  BVC  _wtL5	;
                       BNE  _wtL4	; write GCR bytes until $00 byte

                       CLV  		;
_wtL6                  BVC  _wtl6	;
                       LDA  #$ee
                       STA  $1c0c
                       STX  $1c03	; CA data direction head ($ff->0: read)
                       STX  $1800	; send handshake
                       LDY  #$00
                       RTS  

;----------------------------------------
					; read $1c00 motor/head status
_read_1c00             LDY  $1c00	;
                       RTS  		;

; ----------------------------------------

_send_byte             JMP  _send_byte_1; parallel-send data byte to C64

;----------------------------------------

                       LDX  #$00
                       STX  $b80c
                       DEX  
                       STX  $b808
                       LDX  #$04
                       STX  $b80c
                       BNE  _sbJ1

;----------------------------------------

_send_byte_1           LDX  #$ff	;
                       STX  $1803	; data direction port A = output
_sbJ1                  LDX  #$10	;
_sbL1                  BIT  $1800	; wait for ATN IN = 1
                       BPL  _sbL1	;
                       STA  $1801	; PA, port A (8 bit parallel data)
                       STX  $1800	; handshake: ATN OUT = 1
                       DEX  		;
_sbL2                  BIT  $1800	;
                       BMI  _sbL2	; wait for ATN IN = 0
                       STX  $1800	; ATN OUT = 0
                       RTS  		;
;----------------------------------------
					; read 1 byte with 4 byte command header
_read_command          LDY  #$04	; read 4 byte command header
_rcL1                  JSR  _read_byte	; read byte from parallel data port
                       CMP  _command_header-1,Y	; check with command header:
                       BNE  _read_command	;  $00,$55,$aa,$ff
                       DEY  		;
                       BNE  _rcL1	;
_read_byte             JMP  _read_byte_1; read byte from parallel data port

;----------------------------------------

                       LDX  #$00
                       STX  $b80c
                       STX  $b808
                       LDX  #$04
                       STX  $b80c
                       BNE  _rbJ1

_read_byte_1           LDX  #$00	;
                       STX  $1803	; data direction port A = input
_rbJ1                  LDX  #$10	;
_rbL1                  BIT  $1800	; wait for ATN IN = 1
                       BPL  _rbL1	;
                       STX  $1800	; handshake: ATN OUT = 1
                       DEX  		;
_rbL2                  BIT  $1800	;
                       BMI  _rbL2	; wait for ATN IN = 0
                       LDA  $1801	; PA, port A (8 bit parallel data)
                       STX  $1800	; ATN OUT = 0
                       RTS  		;
;----------------------------------------
					; send 0,1,2,...,$ff bytes to C64
_send_count            TYA  		;
                       JSR  _send_byte	; parallel-send data byte to C64
                       INY  		;  (send 0,1,2,...,$ff)
                       BNE  _send_count	;
                       RTS  		;

;----------------------------------------

_perform_ui            LDA  #$12	;
                       STA  $22		; current track = 18
                       JMP  $eb22	; UI command (?)

;----------------------------------------
					; measure destination track length
_measure_trk_len       LDX  #$20
                       LDA  #$ce
                       STA  $1c0c
                       DEC  $1c03	; CA data direction head (0->$ff: write)

                       LDA  #$55	;
                       STA  $1c01	; write $55 byte
_mtL1                  BVC  _mtL1	;
                       CLV  		;
                       INY  		; write $2000 times
                       BNE  _mtL1	;
                       DEX  		;
                       BNE  _mtL1	;

                       LDA  #$ff	;
                       STA  $1c01	; write $ff byte (Sync mark)
_mtL2                  BVC  _mtL2	;
                       CLV  		;
                       INX  		; write 5 times (short Sync)
                       CPX  #$05	;
                       BNE  _mtL2	;

                       LDA  #$ee
                       STA  $1c0c
                       STY  $1c03	; CA data direction head ($ff->0: read)
_mtJ1                  LDA  $1c00	;
                       BPL  _mt_end	; 1st time: Sync, 2nd time: no Sync ->

_mtL3                  BVC  _mtJ1	; if no more bytes available ->
                       CLV  		;
                       INX  		; X/Y = counter: GCR bytes in one spin
                       BNE  _mtL3	;
                       INY  		;
                       BNE  _mtL3	;
_mt_end                TXA  		; (0) : Track 'too long'
                       JMP  _send_byte	; parallel-send data byte to C64

;----------------------------------------
					; initialise write track
_format_track          LDX  #$02	;
                       LDA  #$ce
                       STA  $1c0c
                       DEC  $1c03	; CA data direction head (0->$ff: write)

                       LDA  #$55	;
                       STA  $1c01	; write $55 byte
_ftL1                  BVC  _ftL1	;
                       CLV  		;
                       INY  		; write $0200 times
                       BNE  _ftL1	;  makes a clean start of track
                       DEX  		;
                       BNE  _ftL1	;

                       LDA  #$ff	;
                       STA  $1c01	; write $ff byte (Sync mark)
_ftL2                  BVC  _ftL2	;
                       CLV  		;
                       INX  		;
                       CPX  #$0a	; write 10 times (long Sync mark)
                       BNE  _ftL2	;

                       LDA  #$55	;
                       STA  $1c01	; write $55 bytes
                       LDX  #$1d	;
_ftL3                  BVC  _ftL3	;
                       CLV  		; write $1d00 GCR times
                       INY  		;
                       BNE  _ftL3	;
                       DEX  		;
                       BNE  _ftL3	;

                       LDA  #$ee
                       STA  $1c0c
                       STY  $1c03	; CA data direction head ($ff->0: read)
                       RTS  

;----------------------------------------
					; find last GCR byte before 'hole'
					; or next Sync
_find_last_gcr         BIT  $1c00	;
                       BMI  _find_last_gcr	; wait for end of Sync
                       LDA  $1c01	; skip GCR byte (last sync byte)
                       CLV  		;

_flgL1                 BVC  _flgL1	; wait for first GCR byte after Sync
                       LDA  $1c01	; read GCR byte

_flgL2                 LDX  #$0a	; minimum 'hole' length = 10
                       TAY  		; return with last read byte
_flgL3                 DEX  		;
                       BEQ  _flg_end	; -> return with Y
                       BVC  _flgL3	; wait for next GCR byte
                       CLV  		;
                       LDA  $1c01	;
                       CMP  #$ff	; if next GCR byte == Sync, return
                       BNE  _flgL2	; else, continue searching
_flg_end               RTS  		;

;----------------------------------------
					; read out track from MARKER BYTE
_read_from_mark        JSR  _read_byte	; read byte from parallel data port
                       STA  _marker+1	; MARKER BYTE
                       JSR  _send_byte	; parallel-send data byte to C64
                       LDA  #$ff	;
                       STA  $1800	; send handshake
                       LDX  #$20	;
                       STX  $c0		; read $2000 GCR bytes

_rfm_L1                BIT  $1c00	; wait for end of Sync
                       BMI  _rfm_L1	;
                       LDX  $1c01	; read GCR byte ($ff)
                       CLV  		;
_rfm_L2                BVC  _rfm_L2	;
                       LDX  $1c01	; read GCR byte
_marker                CPX  #$37	; check for MARKER BYTE
                       BNE  _rfm_L1	; wrong header mark, repeat
                       JMP  _in_sync	; -> read out track

;----------------------------------------

_verify_code           LDY  #$00	;
                       STY  $c0
                       LDA  #$03
                       STA  $c1
_verify_L1             LDA  ($c0),Y
                       JSR  _send_byte	; parallel-send data byte to C64
                       INY 
                       BNE  _verify_L1	;
                       INC  $c1
                       LDA  $c1
                       CMP  #$08
                       BNE  _verify_L1
                       RTS  		;

;----------------------------------------
;--- Command Jump table               ---; return value: Y
;----------------------------------------
_command_table         .byte <(_step_dest-1), >(_step_dest-1)
					; step motor to destination halftrack
                       .byte <(_set_1c00-1), >(_set_1c00-1)
					; set $1c00 bits (head/motor)
                       .byte <(_perform_ui-1), >(_perform_ui-1)
					; track $22 = 17, UI command: $eb22
                       .byte <(_read_after_sync-1), >(_read_after_sync-1)
					; read out track after Sync
                       .byte <(_write_aftr_sync-1), >(_write_aftr_sync-1)
					; write track after variable Sync length
                       .byte <(_adjust_density-1), >(_adjust_density-1)
					; adjust read routines to density value
                       .byte <(_detect_killer-1), >(_detect_killer-1)
					; detect 'killer tracks'
                       .byte <(_scan_density-1), >(_scan_density-1)
					; perform Density Scan
                       .byte <(_read_track-1), >(_read_track-1)
					; read out track w/out waiting for Sync
                       .byte <(_read_1c00-1), >(_read_1c00-1)
					; read $1c00 motor/head status
                       .byte <(_send_count-1), >(_send_count-1)
					; send 0,1,2,...,$ff bytes to C64
                       .byte <(_write_track-1), >(_write_track-1)
					; write a track on destination
                       .byte <(_measure_trk_len-1), >(_measure_trk_len-1)
					; measure destination track length
                       .byte <(_format_track-1), >(_format_track-1)
					; initialise write track (long  Sync)
                       .byte <(_find_last_gcr-1), >(_find_last_gcr-1)
					; find GCR byte before 'hole' or 'Sync'
                       .byte <(_read_from_mark-1), >(_read_from_mark-1)
					; read out track from MARKER BYTE
                       .byte <(_verify_code-1), >(_verify_code-1)
					; send floppy side code back to PC

_command_header        .byte $ff,$aa,$55,$00	; command header code
