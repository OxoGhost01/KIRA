ENTRY( _start )

SECTIONS{
    . = 0x80000000;

    .text :{
        *(.text)
        *(.text.*)
    }

    . = ALIGN(0x1000);
    stack_bottom = .;
    . += 0x1000;
    stack_top = .;
    end = .;
}
