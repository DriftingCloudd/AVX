    .align 4
    .section .data
    .global initcode
    .global initcodesize
initcode:
      .incbin "./xv6-user/initcode"
    #.incbin "./xv6-user/init-for-test"
codeend:

initcodesize:
    .long codeend-initcode