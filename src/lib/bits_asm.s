.global count_leading_zeroes

count_leading_zeroes:
    clz r0, r0
    bx lr
@    bl    pi_abort   @ included as a sanity check -- otherwise jumping here
                     @ before it's implemented will result in running
                     @ some random code.
    b count_leading_zeroes
