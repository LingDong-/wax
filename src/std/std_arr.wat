;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                  ;;
;;                                  ;;
;;               ARRAY              ;;
;;                                  ;;
;;                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; addapted from https://github.com/LingDong-/wasm-fun

;;   Continous, resizable storage for a sequence of values,
;;   similar to C++ vector<T>
;;
;;   +--------------------+
;;   |data|length|capacity|
;;   +-|------------------+
;;     |        +---------------------------
;;     `------> |elem 0|elem 1|elem 2|......
;;              +---------------------------

;; struct arr {
;;   i32/f32/(void*) data
;;   int length
;;   int capacity
;; }

(global $wax::DEFAULT_CAPACITY (mut i32) (i32.const 8))

;; (internal) getter/setters for arr struct fields

(func $wax::_arr_set_data (param $ptr i32) (param $data i32)
  (i32.store (local.get $ptr) (local.get $data))
)
(func $wax::_arr_set_length (param $ptr i32) (param $length i32)
  (i32.store (i32.add (local.get $ptr) (i32.const 4)) (local.get $length))
)
(func $wax::_arr_set_capacity (param $ptr i32) (param $capacity i32)
  (i32.store (i32.add (local.get $ptr) (i32.const 8)) (local.get $capacity))
)
(func $wax::_arr_get_data (param $ptr i32) (result i32)
  (i32.load (local.get $ptr))
)
(func $wax::_arr_get_capacity (param $ptr i32) (result i32)
  (i32.load (i32.add (local.get $ptr) (i32.const 8)))
)

;; returns length of an array given an arr pointer
(func $wax::arr_length (param $ptr i32) (result i32)
  (i32.load (i32.add (local.get $ptr) (i32.const 4)) )
)

;; initialize a new arr, returns a pointer to it
;; elem_size: size of each element, in bytes
(func $wax::arr_new (param $len i32) (result i32)
  (local $ptr i32)
  (local $cap i32)
  (local $data i32)
  (if (i32.lt_u (local.get $len) (global.get $wax::DEFAULT_CAPACITY)) (then
    (local.set $cap (global.get $wax::DEFAULT_CAPACITY))
  )(else
    (local.set $cap (local.get $len))
  ))
  (local.set $ptr (call $wax::malloc (i32.const 12)))
  (local.set $data (call $wax::calloc (i32.mul (local.get $cap) (i32.const 4))))
  (call $wax::_arr_set_data (local.get $ptr) (local.get $data))
  (call $wax::_arr_set_length (local.get $ptr) (local.get $len))
  (call $wax::_arr_set_capacity (local.get $ptr) (local.get $cap))
  (local.get $ptr)
)

;; free allocated memory given an arr pointer
(func $wax::arr_free (param $a i32)
  (call $wax::free (call $wax::_arr_get_data (local.get $a)))
  (call $wax::free (local.get $a))
)



;; get ith element of an array
(func $wax::arr_get (param $a i32) (param $i i32) (result i32)
  (local $data i32)
  (local $elem_size i32)
  (local.set $data (call $wax::_arr_get_data (local.get $a)))
  (local.set $elem_size (i32.const 4))
  (i32.load (i32.add (i32.mul (local.get $i) (local.get $elem_size)) (local.get $data)))
)

;; set ith element of an array
(func $wax::arr_set (param $a i32) (param $i i32) (param $v i32)
  (local $data i32)
  (local $elem_size i32)
  (local.set $data (call $wax::_arr_get_data (local.get $a)))
  (local.set $elem_size (i32.const 4))
  (i32.store (i32.add (i32.mul (local.get $i) (local.get $elem_size)) (local.get $data)) (local.get $v))
)

;; remove n elements from an array starting at index i
(func $wax::arr_remove (param $a i32) (param $i i32) (param $n i32)
  (local $data i32)
  (local $elem_size i32)
  (local $length i32)
  (local $offset i32)

  (local.set $length (call $wax::arr_length (local.get $a)))
  (local.set $data (call $wax::_arr_get_data (local.get $a)))
  (local.set $elem_size (i32.const 4))

  (local.set $offset 
    (i32.add (local.get $data) (i32.mul (local.get $i) (local.get $elem_size) ))
  )

  (call $wax::memmove 
    (local.get $offset)
    (i32.add (local.get $offset) (i32.mul (local.get $n) (local.get $elem_size)))
    (i32.mul (i32.sub (local.get $length) (i32.add (local.get $i) (local.get $n)) ) (local.get $elem_size))
  )
  (call $wax::_arr_set_length  (local.get $a) (i32.sub (local.get $length) (local.get $n) ))
)


;; add an element to the end of the array
;; does not write the element, instead, returns a pointer
;; to the new last element for the user to write at
(func $wax::arr_push (param $a i32) (result i32)
  (local $length i32)
  (local $capacity i32)
  (local $data i32)
  (local $elem_size i32)

  (local.set $length (call $wax::arr_length (local.get $a)))
  (local.set $capacity (call $wax::_arr_get_capacity (local.get $a)))
  (local.set $data (call $wax::_arr_get_data (local.get $a)))
  (local.set $elem_size (i32.const 4))

  (if (i32.lt_u (local.get $length) (local.get $capacity) ) (then) (else
    (local.set $capacity (i32.add
      (i32.add (local.get $capacity) (i32.const 1))
      (local.get $capacity)
    ))
    (call $wax::_arr_set_capacity (local.get $a) (local.get $capacity))

    (local.set $data 
      (call $wax::realloc (local.get $data) (i32.mul (local.get $elem_size) (local.get $capacity) ))
    )
    (call $wax::_arr_set_data (local.get $a) (local.get $data))
  ))
  (call $wax::_arr_set_length (local.get $a) (i32.add (local.get $length) (i32.const 1)))
  
  ;; (i32.store (i32.add (local.get $data) (i32.mul (local.get $length) (local.get $elem_size))) (i32.const 0))

  (i32.add (local.get $data) (i32.mul (local.get $length) (local.get $elem_size)))
)

;; insert into an array at given index
(func $wax::arr_insert (param $a i32) (param $i i32) (param $v i32)
  (local $data i32)
  (local $elem_size i32)
  (local $length i32)
  (local $offset i32)

  (local.set $length (call $wax::arr_length (local.get $a)))

  (drop (call $wax::arr_push (local.get $a)))

  (local.set $data (call $wax::_arr_get_data (local.get $a)))
  (local.set $elem_size (i32.const 4))

  (local.set $offset 
    (i32.add (local.get $data) (i32.mul (local.get $i) (local.get $elem_size) ))
  )

  (call $wax::memmove
    (i32.add (local.get $offset) (local.get $elem_size))
    (local.get $offset)
    (i32.mul 
      (i32.sub (local.get $length) (local.get $i) ) 
      (local.get $elem_size)
    )
  )

  (i32.store (local.get $offset) (local.get $v))

)

;; slice an array, producing a copy of a range of elements 
;; i = starting index (inclusive), j = stopping index (exclusive)
;; returns pointer to new array
(func $wax::arr_slice (param $a i32) (param $i i32) (param $n i32) (result i32)
  (local $a_length i32)
  (local $length i32)
  (local $elem_size i32)
  (local $ptr i32)
  (local $data i32)
  (local $j i32)

  (local.set $j (i32.add (local.get $i) (local.get $n)))

  (local.set $a_length (call $wax::arr_length (local.get $a)))

  (if (i32.lt_s (local.get $i) (i32.const 0) )(then
    (local.set $i (i32.add (local.get $a_length) (local.get $i)))
  ))
  (if (i32.lt_s (local.get $j) (i32.const 0) )(then
    (local.set $j (i32.add (local.get $a_length) (local.get $j)))
  ))

  (local.set $length (i32.sub (local.get $j) (local.get $i)))
  (local.set $elem_size (i32.const 4))

  (local.set $ptr (call $wax::malloc (i32.const 12)))
  (local.set $data (call $wax::malloc (i32.mul (local.get $length) (local.get $elem_size))))
  (call $wax::_arr_set_data (local.get $ptr) (local.get $data))
  (call $wax::_arr_set_length (local.get $ptr) (local.get $length))
  (call $wax::_arr_set_capacity (local.get $ptr) (local.get $length))

  (call $wax::memcpy (local.get $data) 
    (i32.add
      (call $wax::_arr_get_data (local.get $a))
      (i32.mul (local.get $i) (local.get $elem_size))
    )
    (i32.mul (local.get $length) (local.get $elem_size))
  )

  (local.get $ptr)
)

