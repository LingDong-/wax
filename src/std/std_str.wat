;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                  ;;
;;                                  ;;
;;              STRING              ;;
;;                                  ;;
;;                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(func $wax::fmod (param $x f32) (param $y f32) (result f32)
  (f32.sub (local.get $x) (f32.mul (local.get $y) (f32.trunc (f32.div (local.get $x) (local.get $y)) )))
)

(func $wax::str_new (param $ptr i32) (result i32)
  (if (i32.eqz (local.get $ptr))(then
    (call $wax::calloc (i32.const 1))
    return
  ))
  (call $wax::str_slice (local.get $ptr) (i32.const 0) (call $wax::str_len (local.get $ptr)) )
  return
)

(func $wax::str_get (param $ptr i32) (param $i i32) (result i32)
  (i32.load8_s (i32.add (local.get $ptr) (local.get $i)))
  return
)

(func $wax::str_add (param $ptr i32) (param $c i32) (result i32)
  (local $l i32)
  (local.set $l (call $wax::str_len (local.get $ptr)))
  (local.set $ptr (call $wax::realloc (local.get $ptr) (i32.add (local.get $l) (i32.const 2) ) ) )
  (i32.store8 (i32.add (local.get $ptr) (local.get $l)) (local.get $c)  )
  (i32.store8 (i32.add (i32.add (local.get $ptr) (local.get $l)) (i32.const 1)) (i32.const 0)  )

  (local.get $ptr)
  (return)
)

(func $wax::str_cat (param $s0 i32) (param $s1 i32) (result i32)
  (local $l0 i32)
  (local $l1 i32)
  (local.set $l0 (call $wax::str_len (local.get $s0)))
  (local.set $l1 (call $wax::str_len (local.get $s1)))

  (local.set $s0 (call $wax::realloc (local.get $s0) (i32.add (i32.add (local.get $l0) (local.get $l1) ) (i32.const 1)) ))

  (call $wax::memcpy (i32.add (local.get $s0) (local.get $l0)) (local.get $s1) (local.get $l1) )
  (i32.store8 (i32.add (i32.add (local.get $s0) (local.get $l0)) (local.get $l1)) (i32.const 0)  )
  (local.get $s0)
  return
)

(func $wax::str_cmp (param $s0 i32) (param $s1 i32) (result i32)
  (local $offset i32)
  (local $x i32)
  (local $y i32)
  (local.set $offset (i32.const 0))
  loop $cmp
    (local.set $x (i32.load8_u (i32.add (local.get $s0) (local.get $offset))) )
    (local.set $y (i32.load8_u (i32.add (local.get $s1) (local.get $offset))) )

    (if (i32.eq (local.get $x) (local.get $y)) (then
      (if (i32.eqz (local.get $x)) (then
        (i32.const 1)
        return
      ))
    )(else
      (i32.const 0)
      return
    ))

    (local.set $offset (i32.add (local.get $offset) (i32.const 1)))
    (br $cmp)
  end
  (i32.const 0) ;;impossible, just shut compiler up
  return
)

(func $wax::str_len (param $ptr i32) (result i32)
  (local $offset i32)
  (local.set $offset (i32.const 0))
  loop $zero
    (if (i32.load8_u (i32.add (local.get $ptr) (local.get $offset))) (then
      (local.set $offset (i32.add (local.get $offset) (i32.const 1)))
      (br $zero)
    ))
  end
  (local.get $offset)
  return
)

(func $wax::str_slice (param $ptr i32) (param $i i32) (param $n i32) (result i32)
  (local $nptr i32)
  (local.set $nptr (call $wax::malloc (i32.add (local.get $n) (i32.const 1))))
  (call $wax::memcpy 
    (local.get $nptr) 
    (i32.add (local.get $ptr) (local.get $i)) 
    (local.get $n)
  )
  (i32.store8 (i32.add (local.get $nptr) (local.get $n) ) (i32.const 0))
  (local.get $nptr)
  return
)

(func $wax::print (param $ptr i32)
  (local $len i32)
  (local.set $len (call $wax::str_len (local.get $ptr)))
  (call $wax::js::console.log (local.get $ptr) (local.get $len))
)

(func $wax::int2str (param $x i32) (result i32)
  (local $ptr i32)
  (local $rem i32)
  (local $isneg i32)
  (local $str i32)

  (local.set $str (call $wax::alloca (i32.const 16)))
  (local.set $ptr (i32.add (local.get $str) (i32.const 15)))
  (i32.store8 (local.get $ptr) (i32.const 0))
  
  (local.set $isneg (i32.const 0))
  (if (i32.lt_s (local.get $x) (i32.const 0)) (then
    (local.set $isneg (i32.const 1))
    (local.set $x (i32.sub (i32.const 0) (local.get $x) ))
  ))

  loop $digits
    (local.set $ptr (i32.sub (local.get $ptr) (i32.const 1)))

    (local.set $rem (i32.rem_u (local.get $x) (i32.const 10)))
    (i32.store8 (local.get $ptr) (i32.add (local.get $rem) (i32.const 48)))

    (local.set $x (i32.div_u (local.get $x) (i32.const 10)))

    (if (i32.eqz (i32.eqz (local.get $x))) (then
      (br $digits)
    ))
  end

  (if (local.get $isneg) (then
    (local.set $ptr (i32.sub (local.get $ptr) (i32.const 1)))
    (i32.store8 (local.get $ptr) (i32.const 45)) ;; '-'
  ))

  (local.get $ptr)
  return
)

(func $wax::fint2str (param $x f32) (result i32)
  (local $ptr i32)
  (local $rem i32)
  (local $isneg i32)
  (local $str i32)
  (local.set $x (f32.trunc (local.get $x)))

  (local.set $str (call $wax::alloca (i32.const 48)))
  (local.set $ptr (i32.add (local.get $str) (i32.const 47)))
  (i32.store8 (local.get $ptr) (i32.const 0))
  
  (if (f32.lt (local.get $x) (f32.const 0)) (then
    (local.set $isneg (i32.const 1))
    (local.set $x (f32.sub (f32.const 0) (local.get $x) ))
  ))

  loop $digits
    (local.set $ptr (i32.sub (local.get $ptr) (i32.const 1)))

    (local.set $rem (i32.trunc_f32_s (call $wax::fmod (local.get $x) (f32.const 10.0))))
    (i32.store8 (local.get $ptr) (i32.add (local.get $rem) (i32.const 48)))

    (local.set $x (f32.div (local.get $x) (f32.const 10.0)))

    (if (f32.gt (local.get $x) (f32.const 0.99999994) ) (then ;;nextafterf(1.00000001,0.0);
      (br $digits)
    ))
  end

  (if (local.get $isneg) (then
    (local.set $ptr (i32.sub (local.get $ptr) (i32.const 1)))
    (i32.store8 (local.get $ptr) (i32.const 45)) ;; '-'
  ))

  (local.get $ptr)
  return
)




(func $wax::flt2str (param $x f32) (result i32)
  (local $ptr0 i32)
  (local $ptr i32)
  (local $rem i32)
  (local $isneg i32)
  (local $str i32)

  (local.set $ptr0 (call $wax::fint2str (local.get $x)))

  (if (f32.lt (local.get $x) (f32.const 0)) (then
    (local.set $x (f32.sub (f32.const 0.0) (local.get $x)))
  ))
  (local.set $x (f32.sub (local.get $x) (f32.trunc (local.get $x))))
  
  (local.set $str (call $wax::alloca (i32.const 16)))
  (i32.store8 (i32.sub (local.get $str) (i32.const 1)) (i32.const 46)  )
  (local.set $ptr (local.get $str))
  
  loop $digits
    
    (local.set $rem (i32.trunc_f32_s (f32.mul (local.get $x) (f32.const 10.0)) ))
    (i32.store8 (local.get $ptr) (i32.add (local.get $rem) (i32.const 48)))

    (local.set $x (f32.sub (f32.mul (local.get $x) (f32.const 10.0)) (f32.convert_i32_s (local.get $rem))) )

    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))

    (if (i32.and
      (f32.gt (local.get $x) (f32.const 1.1920928955078126e-7) ) ;; floating-point epsilon
      (i32.lt_s (i32.sub (local.get $ptr) (local.get $str)) (i32.const 15))
    )(then
      (br $digits)
    ))
  end
  (i32.store8 (local.get $ptr) (i32.const 0))

  (local.get $ptr0)
  return

)

(func $wax::str2int (param $s i32) (result i32)
  (local $x i32)
  (local $ptr i32)
  (local $d i32)
  (local $sign i32)
  (local.set $x (i32.const 0))

  (local.set $ptr (local.get $s))

  (local.set $sign (i32.const 1))
  (if (i32.eq (i32.load8_s (local.get $ptr) ) (i32.const 45) ) (then ;;'-'
    (local.set $sign (i32.const -1))
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))
  )(else(if (i32.eq (i32.load8_s (local.get $ptr) ) (i32.const 43) ) (then ;;'+'
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1))) ;;redundant
  ))))

  loop $digits
    (local.set $d (i32.load8_s (local.get $ptr) ))
    (if (i32.or
      (i32.lt_s (local.get $d) (i32.const 48))
      (i32.gt_s (local.get $d) (i32.const 57))
    )(then
      (i32.mul (local.get $sign) (local.get $x))
      return
    ))
    
    (local.set $x (i32.mul (local.get $x) (i32.const 10)))
    (local.set $x (i32.add (local.get $x) (i32.sub (local.get $d) (i32.const 48))))
    
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))
    (br $digits)
  end

  (i32.mul (local.get $sign) (local.get $x))
  return
)

(func $wax::str2flt (param $s i32) (result f32)
  (local $x f32)
  (local $ptr i32)
  (local $d i32)
  (local $sign f32)
  (local $mlt f32)

  (local.set $x (f32.const 0.0))

  (local.set $ptr (local.get $s))


  (local.set $sign (f32.const 1.0))
  (if (i32.eq (i32.load8_s (local.get $ptr) ) (i32.const 45) ) (then ;;'-'
    (local.set $sign (f32.const -1))
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))
  )(else(if (i32.eq (i32.load8_s (local.get $ptr) ) (i32.const 43) ) (then ;;'+'
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1))) ;;redundant
  ))))

  block $out
  loop $digits
    (local.set $d (i32.load8_s (local.get $ptr) ))
    (if (i32.eq (local.get $d) (i32.const 46)) (then ;; '.'
      (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))
      (br $out)
    ))
    (if (i32.or
      (i32.lt_s (local.get $d) (i32.const 48))
      (i32.gt_s (local.get $d) (i32.const 57))
    )(then
      (f32.mul (local.get $sign) (local.get $x))
      return
    ))
    
    (local.set $x (f32.mul (local.get $x) (f32.const 10.0)))
    (local.set $x (f32.add (local.get $x) (f32.convert_i32_s (i32.sub (local.get $d) (i32.const 48)))))
    
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))
    (br $digits)
  end
  end

  (local.set $mlt (f32.const 0.1))
  loop $fracs
    (local.set $d (i32.load8_s (local.get $ptr) ))

    (if (i32.or
      (i32.lt_s (local.get $d) (i32.const 48))
      (i32.gt_s (local.get $d) (i32.const 57))
    )(then
      (f32.mul (local.get $sign) (local.get $x))
      return
    ))
    (local.set $x (f32.add
      (local.get $x)
      (f32.mul (f32.convert_i32_s (i32.sub (local.get $d) (i32.const 48))) (local.get $mlt))
    ))
    (local.set $mlt (f32.mul (local.get $mlt) (f32.const 0.1)))
    
    (local.set $ptr (i32.add (local.get $ptr) (i32.const 1)))

    (br $fracs)
  end

  (f32.mul (local.get $sign) (local.get $x))
  return
)