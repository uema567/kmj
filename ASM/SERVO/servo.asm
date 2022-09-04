;; Servo Moter: SG90                                Tsuyoshi Uema   OKINAWA JAPAN
;; 
;; Timer0:CTC動作:0.01[ms]毎にMAINを実行する
;;
;;                    *------------------------
;; VCC-[10k] --------|RESET                 VCC|---------------- VCC
;;               ----|PB3/~OC1B        OC1A/PB2|---- SW -------- GND
;;               ----|PB4/OC1B         OC0B/PB1|------------------------ オレンジServo:SG90    
;; GND --------------|GND        OC0A/~OC1A/PB0|-    VCC --------------- 赤Servo:SG90      
;;                    -------------------------      GND --------------- 茶servo:SG90      
;;                             TINY85                
;; 
;;  -----               -----               -----               -----               -----
;; |     |             |     |             |     |             |     |             |     
;; |     |             |     |             |     |             |     |             |     
;;        -------------       -------------       -------------       -------------      
;; |  ON |     OFF     |  ON |     OFF     |  ON |     OFF     |  ON |     OFF     | ~~~~
;; |<------- 20[ms]--->|
;; |<-X->|               周期は20[ms]で  X[ms]を 0.5から2.4[ms]までの可変で
;;                       Servo Moter (SG90)の角度を0°(-90°)から180°(+90°)にすることが
;;                       できます。90°(0°)にするには、1.45[ms]
;;
;;     周期：20[ms]
;;     ON時間：0.5[ms]:   0°（-９０°）
;;     ON時間：1.45[ms]: 90°（０°）
;;     ON時間：2.4[ms]: 180°（+９０°）
;; 
;; tiny85は、16bitTimerがないので、MAINを0.01[ms]でまわし、擬似PWMを発生させています。
;;
;; PROGRAMER: USBASP
;; 8[MHz]   8Mhz/8=1[MHz]のクロックだと思っていましたが、感覚的に8[MHz]で動いている気がします。
;; AVR Programmer: USBasp がうまく動作していない気がしますので、以前使っていたHIDaspxを自作しようと思います。
;; 部品は注文中です。
;; 
.nolist
.include "../AVR_DEF/tn85def.inc"
.list
.include "../MACRO/macro.inc"

.def temp=R16
.def flag=R17
.def pwm_on_time=R18
.def pwm_cnt=R19
.def pwm_cycle_l=R24
.def pwm_cycle_h=R25
.def cnt24bit_byte1=R20
.def cnt24bit_byte2=R21
.def cnt24bit_byte3=R22


.macro inc_cnt24bit
	ldi  temp, 1
	add  cnt24bit_byte1, temp
	clr  temp
	adc  cnt24bit_byte2, temp
	adc  cnt24bit_byte3, temp
.endmacro

.macro cpi_cnt24bit
	ldi temp, BYTE1(@0)
	cp cnt24bit_byte1, temp
	ldi temp, BYTE2(@0)
	cpc cnt24bit_byte2, temp
	ldi temp, BYTE3(@0)
	cpc cnt24bit_byte3, temp
.endmacro

.macro pwm_cpi16
	ldi temp, byte1( @0 )
	cp  pwm_cycle_l, temp
	ldi temp, byte2( @0 )
	cpc pwm_cycle_h, temp
.endmacro

.listmac

;;; macro InReg, OutReg defined

.org 0
	rjmp RESET

.org OC0Aaddr
	rjmp tim0ctc

.org INT_VECTORS_SIZE

RESET:
 	OutReg DDRB, 0b00011111  ;;; PWM OutPut
 	OutReg TCCR0A, (1<<WGM01)  ;;; 高速PWM動作 ; 
 	OutReg TCCR0B, (1<<CS01)               ;;; ck/8
 	OutReg OCR0A, 9            ;;; 1/100000[s]==0.01[ms]
	OutReg TIMSK, (1<<OCIE0A)  ;;; 比較A割り込み許可
	sei
MAIN:
	tst flag
	breq MAIN   ;;; 0.01[ms]に達するまで待機
	clr flag    ;;; flag クリア
	inc_cnt24bit
	cpi_cnt24bit 100000
	brne MAIN9
	clr cnt24bit_byte1
	clr cnt24bit_byte2
	clr cnt24bit_byte3
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	inc pwm_cnt
	cpi pwm_cnt, 1
	brne MAIN_A
	ldi pwm_on_time, 50
	rjmp MAIN9
MAIN_A:
	cpi pwm_cnt, 2
	brne MAIN_B
	ldi pwm_on_time, 145
	rjmp MAIN9
MAIN_B:
	cpi pwm_cnt, 3
	brne MAIN_C
	ldi pwm_on_time, 240
	rjmp MAIN9
MAIN_C:
	cpi pwm_cnt, 4
	brne MAIN9
	clr pwm_cnt
MAIN9:
	rcall PWM
	rjmp MAIN


PWM:
	pwm_cpi16 1
	brne PWM1
	OutReg PORTB, 0b0001111
	rjmp PWM9
PWM1:
	cp pwm_cycle_l, pwm_on_time
	ldi temp, 0
	cpc pwm_cycle_h, temp
	brne PWM2
	OutReg PORTB, 0
	rjmp PWM9
PWM2:
	pwm_cpi16 2000
	brne PWM9
	clr pwm_cycle_l
	clr pwm_cycle_h
PWM9:
	adiw pwm_cycle_l, 1
	ret

tim0ctc:
	push temp
	in  temp, SREG
	ser flag
	out SREG, temp
	pop temp
	reti
