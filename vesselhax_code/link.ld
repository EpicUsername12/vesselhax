OUTPUT(vesselhax_code.elf);

ENTRY(_start);

SECTIONS {
	. = 0x01800000;
	.text : {
		*(.init)		
		*(.text*);
		KEEP(*(.kernel_code*));
	}
	.rodata : {
		*(.rodata*);
	}

	. = 0x20000000;

	.data : {
		*(.data*);

		__sdata_start = .;
		*(.sdata*);
		__sdata_end = .;

		__sdata2_start = .;
		*(.sdata2*);
		__sdata2_end = .;
	}
	.bss : {
		__bss_start = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		__bss_end = .;
	}
	__CODE_END = .;

	/DISCARD/ : {
		*(*);
	}
}