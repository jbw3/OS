/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

ENTRY(start)
SECTIONS
{
  .text 0x100000 :
  {
    code = .; _code = .; __code = .;
    *(.text)
    . = ALIGN(4096);
  }

  .data :
  {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata)
     . = ALIGN(4096);
  }

  .bss :
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  end = .; _end = .; __end = .;
}

/*
ENTRY(start)
SECTIONS {
    . = 0x00100000;          /* the code should be loaded at 1 MB *

    .text ALIGN (0x1000) :   /* align at 4 KB *
    {
        code = .;
        *(.text)             /* all text sections from all files *
    }

    .rodata ALIGN (0x1000) : /* align at 4 KB *
    {
        *(.rodata*)          /* all read-only data sections from all files *
    }

    .data ALIGN (0x1000) :   /* align at 4 KB *
    {
        *(.data)             /* all data sections from all files *
    }

    .bss ALIGN (0x1000) :    /* align at 4 KB *
    {
        bss = .;
        *(COMMON)            /* all COMMON sections from all files *
        *(.bss)              /* all bss sections from all files *
    }

    end = .;
}
*/

/*
OUTPUT_FORMAT("binary")
ENTRY(start)
phys = 0x00100000;
SECTIONS
{
  .text phys : AT(phys) {
    code = .;
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
  }
  .data : AT(phys + (data - code))
  {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }
  .bss : AT(phys + (bss - code))
  {
    bss = .;
    *(.bss)
    . = ALIGN(4096);
  }
  end = .;
}
*/

/*
ENTRY(start)
SECTIONS
{
  .text 0x100000 :
  {
    code = .; _code = .; __code = .;
    *(.text)
    . = ALIGN(4096);
  }

  .data :
  {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata*)
     . = ALIGN(4096);
  }

  .bss :
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  end = .; _end = .; __end = .;
  . = ALIGN(4096);
}
*/
