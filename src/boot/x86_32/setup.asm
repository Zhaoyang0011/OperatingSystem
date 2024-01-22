    mov ax,0xb800                 ;指向文本模式的显示缓冲区
    mov es,ax
    mov byte [es:0x00],'H'
    mov byte [es:0x01],0x07

    jmp $

    times 512 * 4 - ($ - $$) db 0