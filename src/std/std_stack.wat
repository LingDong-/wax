;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                  ;;
;;                                  ;;
;;               STACK              ;;
;;                                  ;;
;;                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; poor man's stack
;; malloc a block of memory and pretend it's the stack, because of the apparent lack(?) of stack address in webassembly

(global $wax::stack_indices_ptr (mut i32) (i32.const 0))
(global $wax::stack_content_ptr (mut i32) (i32.const 0))
(global $wax::stack_count   (mut i32) (i32.const 32))
(global $wax::stack_size    (mut i32) (i32.const 128))
(global $wax::stack_index   (mut i32) (i32.const 0))
(global $wax::stack_now     (mut i32) (i32.const 0))
(global $wax::stack_did_init(mut i32) (i32.const 0))

(func $wax::init_stack
  (global.set $wax::stack_indices_ptr (call $wax::calloc (i32.mul (global.get $wax::stack_count) (i32.const 4))))
  (global.set $wax::stack_content_ptr (call $wax::malloc (global.get $wax::stack_size)))
  (global.set $wax::stack_index (i32.const 0))
  (global.set $wax::stack_did_init (i32.const 1))
)
(func $wax::stack_index_offset (result i32)
  (i32.add (global.get $wax::stack_indices_ptr) (i32.mul (global.get $wax::stack_index) (i32.const 4)))
)
(func $wax::push_stack

  (if (i32.eqz (global.get $wax::stack_did_init)) (then (call $wax::init_stack) ))

  (global.set $wax::stack_index (i32.add (global.get $wax::stack_index) (i32.const 1) ))

  (if (i32.ge_u (global.get $wax::stack_index) (global.get $wax::stack_count)) (then
    (global.set $wax::stack_count (i32.add (global.get $wax::stack_count) (i32.const 128)))
    (global.set $wax::stack_indices_ptr (call $wax::realloc 
      (global.get $wax::stack_indices_ptr) 
      (i32.mul (global.get $wax::stack_count) (i32.const 4))
    ))
  ))

  (i32.store (call $wax::stack_index_offset) (global.get $wax::stack_now) )

)
(func $wax::pop_stack

  (global.set $wax::stack_now (i32.load (call $wax::stack_index_offset) ))
  (global.set $wax::stack_index (i32.sub (global.get $wax::stack_index) (i32.const 1) ))
  
)
(func $wax::alloca (param $n_bytes i32) (result i32)
  (local $inc i32)
  (if (i32.ge_u (i32.add (global.get $wax::stack_now) (local.get $n_bytes) ) (global.get $wax::stack_size)) (then
    (local.set $inc (i32.const 512))
    (if (i32.gt_u (local.get $n_bytes) (local.get $inc)) (then
      (local.set $inc (call $wax::align4 (local.get $n_bytes)))
    ))
    (global.set $wax::stack_size (i32.add (global.get $wax::stack_size) (local.get $inc) ))
    (global.set $wax::stack_content_ptr
      (call $wax::realloc (global.get $wax::stack_content_ptr) (global.get $wax::stack_size))
    )
  ))

  ;; repurpose $inc for ret val
  (local.set $inc (i32.add (global.get $wax::stack_content_ptr ) (global.get $wax::stack_now)))

  (global.set $wax::stack_now (i32.add (global.get $wax::stack_now) (local.get $n_bytes)))

  (local.get $inc)
  return
)