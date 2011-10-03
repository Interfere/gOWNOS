;
; boot.s -- Kernel start location. Also defines multiboot header
; Based on Bran's kernel development tutorial file start.asm
;

MBOOT_PAGE_ALIGN	equ 1<<0	; Загрузить ядро и модули по границе страницы
MBOOT_MEM_INFO		equ 1<<1	; Запросить от загрузчика информацию о памяти
MBOOT_HEADER_MAGIC	equ 0x1BADB002	; Специальный флаг для загрузчика
; NOTE: мы не используем MBOOT_AOUT_KLUDGE. Это означает что GRUB  не сообщит 
; адрес таблицы символов
MBOOT_HEADER_FLAGS	equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM		equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]		; загрузчик сам переводит процессор в защищенный режим

[GLOBAL mboot]	; чтобы 'mboot' был доступен из кода на C
[EXTERN code]	; начало секции .text
[EXTERN bss]	; начало секции .bss
[EXTERN end]	; конец последней загружаемой секции

mboot:
	dd	MBOOT_HEADER_MAGIC	; GRUB будет искать это значение на каждой
							; 4-кБ границе в файле ядра
	dd	MBOOT_HEADER_FLAGS	; Сообщает загрузчику опции загрузки ядра
	dd	MBOOT_CHECKSUM		; Контрольная сумма первых двух полей

	dd	mboot				; адрес текущего дескриптора
	dd	code				; адрес начала секции .text
	dd	bss					; адрес конца секции .data
	dd	end					; адрес конца всех секций
	dd	start				; адрес точки входа

[GLOBAL start]				; объявляем метку точки вход глобальной
[EXTERN kmain]				; адрес функции main

start:
	push	ebx				; загрузить в стек адрес структуры, полученной от загрузчика
	
	; запускаем ядро
	cli						; запрещаем прерывания
	call	kmain			; вызываем функцию kmain
	jmp		$				; Бесконечный цикл, чтобы процессор не начал выполнять 
							; код (мусор), находящийся после кода ядра.

