;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                  ;;
;;                                  ;;
;;                MAP               ;;
;;                                  ;;
;;                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; adapted from https://github.com/LingDong-/wasm-fun

;;   Hash table (separate chaining with linked lists)
;;   similar to C++ map<T,T>.
;;
;;   Entire key is stored in map node; val is 32 bit int/float/pointer
;;
;;   Functions involving keys have two versions, *_i and *_h.
;;   _i takes an i32 as key directly (for simple small keys), 
;;   while _h versions read the key from the heap given a 
;;   pointer and a byte count (for larger keys)
;;
;;   +-----------+
;;   |num_buckets|          ,----------------------.
;;   |-----------|        +-|-------------------+  |  +---------------------+
;;   | bucket 0  |------->|next|key_size|key|val|  `->|next|key_size|key|val|
;;   |-----------|        +---------------------+     +---------------------+
;;   | bucket 1  |
;;   |-----------|        +---------------------+
;;   | bucket 2  |------->|next|key_size|key|val|
;;   |-----------|        +---------------------+
;;   | ......... |


;; struct map{
;;   int num_buckets;
;;   int length;
;;   mapnode* bucket0;
;;   mapnode* bucket1;
;;   mapnode* bucket2;
;;   ...
;; }
;; struct mapnode{
;;   mapnode* next;
;;   int key_size;
;;   key_t key;
;;   int val;
;; }

;; (internal) getters and setters for map struct

(func $wax::_map_get_num_buckets (param $m i32) (result i32)
  (i32.load (local.get $m))
)
(func $wax::_map_set_num_buckets (param $m i32) (param $num_buckets i32)
  (i32.store (local.get $m) (local.get $num_buckets))
)
(func $wax::map_length (param $m i32) (result i32)
  (i32.load (i32.add (local.get $m) (i32.const 4)))
)
(func $wax::_map_inc_length (param $m i32) (param $dx i32)
  (local $l i32)
  (local $o i32)
  (local.set $o (i32.add (local.get $m) (i32.const 4)))
  (local.set $l (i32.load (local.get $o)))
  (i32.store (local.get $o) (i32.add (local.get $l) (local.get $dx)))
)
(func $wax::_map_get_bucket (param $m i32) (param $i i32) (result i32)
  (i32.load (i32.add 
    (i32.add (local.get $m) (i32.const 8)) 
    (i32.mul (local.get $i) (i32.const 4))
  ))
)
(func $wax::_map_set_bucket (param $m i32) (param $i i32) (param $ptr i32)
  (i32.store (i32.add 
    (i32.add (local.get $m) (i32.const 8)) 
    (i32.mul (local.get $i) (i32.const 4))
  ) (local.get $ptr) )
)

;; (internal) getters and setters for map node struct

(func $wax::_mapnode_get_next (param $m i32) (result i32)
  (i32.load (local.get $m))
)
(func $wax::_mapnode_get_key_size (param $m i32) (result i32)
  (i32.load (i32.add (local.get $m) (i32.const 4)))
)
(func $wax::_mapnode_get_key_ptr (param $m i32) (result i32)
  (i32.add (local.get $m) (i32.const 8))
)
(func $wax::_mapnode_get_val_ptr (param $m i32) (result i32)
  (local $key_size i32)
  (local.set $key_size (call $wax::_mapnode_get_key_size (local.get $m)))
  (i32.add (i32.add (local.get $m) (i32.const 8)) (local.get $key_size))
)

(func $wax::_mapnode_set_next (param $m i32) (param $v i32)
  (i32.store (local.get $m) (local.get $v))
)
(func $wax::_mapnode_set_key_size (param $m i32) (param $v i32)
  (i32.store (i32.add (local.get $m) (i32.const 4)) (local.get $v))
)

(func $wax::_mapnode_set_key_h (param $m i32) (param $key_ptr i32) (param $key_size i32)
  (local $ptr i32)
  (local $i i32)
  (local.set $ptr (call $wax::_mapnode_get_key_ptr (local.get $m)))
  loop $loop_mapnode_set_key_h
    (i32.store8 
      (i32.add (local.get $ptr) (local.get $i))
      (i32.load8_u (i32.add (local.get $key_ptr) (local.get $i)))
    )
    (local.set $i (i32.add (local.get $i) (i32.const 1)))
    (br_if $loop_mapnode_set_key_h (i32.lt_u (local.get $i) (local.get $key_size) ))
  end
)
(func $wax::_mapnode_set_key_i (param $m i32) (param $key i32)
  (i32.store
    (call $wax::_mapnode_get_key_ptr (local.get $m))
    (local.get $key) 
  )
)

;; Hash functions

;; hash an integer with SHR3
(func $wax::_map_hash_i (param $num_buckets i32) (param $key i32) (result i32)
  (local.set $key (i32.xor (local.get $key) (i32.shl   (local.get $key) (i32.const 17))))
  (local.set $key (i32.xor (local.get $key) (i32.shr_u (local.get $key) (i32.const 13))))
  (local.set $key (i32.xor (local.get $key) (i32.shl   (local.get $key) (i32.const 5 ))))
  (i32.rem_u (local.get $key) (local.get $num_buckets))
)

;; hash a sequence of bytes by xor'ing them into an integer and calling _map_hash_i
(func $wax::_map_hash_h (param $num_buckets i32) (param $key_ptr i32) (param $key_size i32) (result i32)
  (local $key i32)
  (local $i i32)
  (local $byte i32)

  (local.set $i (i32.const 0))
  loop $loop_map_hash_h
    (local.set $byte (i32.load8_u (i32.add (local.get $key_ptr) (local.get $i))))
    
    (local.set $key
      (i32.xor (local.get $key) 
        (i32.shl (local.get $byte) (i32.mul (i32.const 8) (i32.rem_u (local.get $i) (i32.const 4))))
      )
    )
    (local.set $i (i32.add (local.get $i) (i32.const 1)))
    (br_if $loop_map_hash_h (i32.lt_u (local.get $i) (local.get $key_size) ))
  end

  (call $wax::_map_hash_i (local.get $num_buckets) (local.get $key))
)

;; initialize a new map, given number of buckets
;; returns a pointer to the map
(func $wax::map_new (param $num_buckets i32) (result i32)
  (local $m i32)
  (local $i i32)
  (local.set $m (call $wax::malloc (i32.add (i32.mul (local.get $num_buckets) (i32.const 4)) (i32.const 8)) ))
  (call $wax::_map_set_num_buckets (local.get $m) (local.get $num_buckets))

  (local.set $i (i32.const 0))
  loop $loop_map_new_clear
    (call $wax::_map_set_bucket (local.get $m) (local.get $i) (i32.const 0))
    (local.set $i (i32.add (local.get $i) (i32.const 1)))
    (br_if $loop_map_new_clear (i32.lt_u (local.get $i) (local.get $num_buckets) ))
  end
  (local.get $m)
)

;; compare the key stored in a node agianst a key on the heap
(func $wax::_map_cmp_key_h (param $node i32) (param $key_ptr i32) (param $key_size i32) (result i32)
  (local $key_ptr0 i32)
  (local $key_size0 i32)
  (local $i i32)
  (local.set $key_ptr0 (call $wax::_mapnode_get_key_ptr (local.get $node)))
  (local.set $key_size0 (call $wax::_mapnode_get_key_size (local.get $node)))
  (if (i32.eq (local.get $key_size0) (local.get $key_size))(then
    (local.set $i (i32.const 0))
    loop $loop_map_cmp_key_h

      (if (i32.eq 
        (i32.load8_u (i32.add (local.get $key_ptr0) (local.get $i)))
        (i32.load8_u (i32.add (local.get $key_ptr ) (local.get $i)))
      )(then)(else
        (i32.const 0)
        return
      ))
      (local.set $i (i32.add (local.get $i) (i32.const 1)))
      (br_if $loop_map_cmp_key_h (i32.lt_u (local.get $i) (local.get $key_size) ))
    end
    (i32.const 1)
    return
  ))
  (i32.const 0)
  return
)

;; compare the key stored in a node agianst a key passed directly as i32 argument
(func $wax::_map_cmp_key_i (param $node i32) (param $key i32) (result i32)
  (local $key_ptr0 i32)
  (local $key_size0 i32)
  (local.set $key_ptr0 (call $wax::_mapnode_get_key_ptr (local.get $node)))
  (local.set $key_size0 (call $wax::_mapnode_get_key_size (local.get $node)))

  (if (i32.eq (local.get $key_size0) (i32.const 4))(then
    (i32.eq (i32.load (local.get $key_ptr0))  (local.get $key) )
    return
  ))
  (i32.const 0)
  return
)

;; insert a new entry to the map, taking a key stored on the heap
;; m : the map
;; key_ptr: pointer to the key on the heap
;; key_size: size of the key in bytes
;; returns pointer to the value inserted in the map for the user to write at

(func $wax::map_set_h (param $m i32) (param $key_ptr i32) (param $val i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)
  (local $node_size i32)
  (local $prev i32)
  (local $key_size i32)
  (local.set $key_size (i32.add (call $wax::str_len (local.get $key_ptr)) (i32.const 1)))

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_h (local.get $num_buckets) (local.get $key_ptr) (local.get $key_size)))
  
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))
  (local.set $node_size (i32.add (local.get $key_size) (i32.const 12) ))


  (if (i32.eqz (local.get $it))(then
    (local.set $it (call $wax::malloc (local.get $node_size)))

    (call $wax::_mapnode_set_key_size (local.get $it) (local.get $key_size))
    (call $wax::_mapnode_set_next (local.get $it) (i32.const 0))
    (call $wax::_mapnode_set_key_h (local.get $it) (local.get $key_ptr) (local.get $key_size))

    (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $it))

    (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
    (call $wax::_map_inc_length (local.get $m) (i32.const 1))
    return
  )(else
    (local.set $prev (i32.const 0))
    loop $loop_map_set_h
      (if (i32.eqz (local.get $it))(then)(else
        (if (call $wax::_map_cmp_key_h (local.get $it) (local.get $key_ptr) (local.get $key_size) )(then
          (local.set $it (call $wax::realloc (local.get $it) (local.get $node_size)))

          (if (i32.eqz (local.get $prev)) (then
            (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $it))
          )(else
            (call $wax::_mapnode_set_next (local.get $prev) (local.get $it))
          ))
          (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
          return
        ))
        (local.set $prev (local.get $it))
        (local.set $it (call $wax::_mapnode_get_next (local.get $it)))
        (br $loop_map_set_h)
      ))
    end
    (local.set $it (call $wax::malloc (local.get $node_size)))
    (call $wax::_mapnode_set_key_size (local.get $it) (local.get $key_size))
    (call $wax::_mapnode_set_next (local.get $it) (i32.const 0))
    (call $wax::_mapnode_set_key_h (local.get $it) (local.get $key_ptr) (local.get $key_size))

    (call $wax::_mapnode_set_next (local.get $prev) (local.get $it))
    (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
    (call $wax::_map_inc_length (local.get $m) (i32.const 1))
    return
  ))

)

;; insert a new entry to the map, taking a key passed directly as i32 argument
;; m : the map
;; key: the key
;; returns pointer to the value inserted in the map for the user to write at

(func $wax::map_set_i (param $m i32) (param $key i32)  (param $val i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)
  (local $node_size i32)
  (local $prev i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_i (local.get $num_buckets) (local.get $key)))
  
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))
  (local.set $node_size (i32.const 16) )


  (if (i32.eqz (local.get $it))(then
    (local.set $it (call $wax::malloc (local.get $node_size)))

    (call $wax::_mapnode_set_key_size (local.get $it) (i32.const 4))
    (call $wax::_mapnode_set_next (local.get $it) (i32.const 0))
    (call $wax::_mapnode_set_key_i (local.get $it) (local.get $key))

    (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $it))

    (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
    (call $wax::_map_inc_length (local.get $m) (i32.const 1))
    return
  )(else
    (local.set $prev (i32.const 0))
    loop $loop_map_set_i
      (if (i32.eqz (local.get $it))(then)(else
        (if (call $wax::_map_cmp_key_i (local.get $it) (local.get $key) )(then
          (local.set $it (call $wax::realloc (local.get $it) (local.get $node_size)))

          (if (i32.eqz (local.get $prev)) (then
            (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $it))
          )(else
            (call $wax::_mapnode_set_next (local.get $prev) (local.get $it))
          ))
          (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
          return
        ))
        (local.set $prev (local.get $it))
        (local.set $it (call $wax::_mapnode_get_next (local.get $it)))
        (br $loop_map_set_i)
      ))
    end
    (local.set $it (call $wax::malloc (local.get $node_size)))
    (call $wax::_mapnode_set_key_size (local.get $it) (i32.const 4))
    (call $wax::_mapnode_set_next (local.get $it) (i32.const 0))
    (call $wax::_mapnode_set_key_i (local.get $it) (local.get $key))

    (call $wax::_mapnode_set_next (local.get $prev) (local.get $it))
    (i32.store (call $wax::_mapnode_get_val_ptr (local.get $it)) (local.get $val))
    (call $wax::_map_inc_length (local.get $m) (i32.const 1))
    return
  ))

)

;; lookup a key for its value in the map, taking a key stored on the heap
;; m : the map
;; key_ptr: pointer to the key on the heap
;; key_size: size of the key in bytes
;; returns pointer to the value in the map, NULL (0) if not found.

(func $wax::map_get_h (param $m i32) (param $key_ptr i32) (result i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)
  (local $key_size i32)
  (local.set $key_size (i32.add (call $wax::str_len (local.get $key_ptr)) (i32.const 1)))

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_h (local.get $num_buckets) (local.get $key_ptr) (local.get $key_size)))
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))

  loop $loop_map_get_h
    (if (i32.eqz (local.get $it))(then)(else
      (if (call $wax::_map_cmp_key_h (local.get $it) (local.get $key_ptr) (local.get $key_size) )(then
        (i32.load (call $wax::_mapnode_get_val_ptr (local.get $it)))
        return
      ))
      (local.set $it (call $wax::_mapnode_get_next (local.get $it)))
      (br $loop_map_get_h)
    ))
  end

  (i32.const 0)
)

;; lookup a key for its value in the map, taking a key passed directly as i32 argument
;; m : the map
;; key : the key
;; returns pointer to the value in the map, NULL (0) if not found.

(func $wax::map_get_i (param $m i32) (param $key i32) (result i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_i (local.get $num_buckets) (local.get $key)))
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))

  loop $loop_map_get_i
    (if (i32.eqz (local.get $it))(then)(else
      (if (call $wax::_map_cmp_key_i (local.get $it) (local.get $key) )(then
        (i32.load (call $wax::_mapnode_get_val_ptr (local.get $it)))
        return
      ))
      (local.set $it (call $wax::_mapnode_get_next (local.get $it)))
      (br $loop_map_get_i)
    ))
  end

  (i32.const 0)
)

;; remove a key-value pair from the map, given a key stored on the heap
;; m : the map
;; key_ptr: pointer to the key on the heap
;; key_size: size of the key in bytes

(func $wax::map_remove_h (param $m i32) (param $key_ptr i32) (param $key_size i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)
  (local $prev i32)
  (local $next i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_h (local.get $num_buckets) (local.get $key_ptr) (local.get $key_size)))
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))
  
  (local.set $prev (i32.const 0))

  loop $loop_map_remove_h
    (if (i32.eqz (local.get $it))(then)(else
      (if (call $wax::_map_cmp_key_h (local.get $it) (local.get $key_ptr) (local.get $key_size) )(then
        (local.set $next (call $wax::_mapnode_get_next (local.get $it)))

        (if (i32.eqz (local.get $prev)) (then
          (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $next))
        )(else
          (call $wax::_mapnode_set_next (local.get $prev) (local.get $next))
        ))
        (call $wax::free (local.get $it))
        (call $wax::_map_inc_length (local.get $m) (i32.const -1))
        return
      ))
      (local.set $prev (local.get $it))
      (local.set $it (local.get $next))
      (br $loop_map_remove_h)
    ))
  end
)

;; remove a key-value pair from the map, given a key passed directly as i32 argument
;; m : the map
;; key : the key
(func $wax::map_remove_i (param $m i32) (param $key i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)
  (local $prev i32)
  (local $next i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))
  (local.set $hash (call $wax::_map_hash_i (local.get $num_buckets) (local.get $key)))
  (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))
  
  (local.set $prev (i32.const 0))

  loop $loop_map_remove_i
    (if (i32.eqz (local.get $it))(then)(else
      (if (call $wax::_map_cmp_key_i (local.get $it) (local.get $key) )(then
        (local.set $next (call $wax::_mapnode_get_next (local.get $it)))

        (if (i32.eqz (local.get $prev)) (then
          (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (local.get $next))
        )(else
          (call $wax::_mapnode_set_next (local.get $prev) (local.get $next))
        ))
        (call $wax::free (local.get $it))
        (call $wax::_map_inc_length (local.get $m) (i32.const 1))
        return
      ))
      (local.set $prev (local.get $it))
      (local.set $it (local.get $next))
      (br $loop_map_remove_i)
    ))
  end
)


;; generate a new iterator for traversing map pairs
;; in effect, this returns a pointer to the first node
(func $wax::map_iter_new  (param $m i32) (result i32)
  (local $num_buckets i32)
  (local $i i32)
  (local $node i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))

  (local.set $i (i32.const 0))
  loop $loop_map_iter_new
    (local.set $node (call $wax::_map_get_bucket (local.get $m) (local.get $i)))
    (if (i32.eqz (local.get $node))(then)(else
      (local.get $node)
      return
    ))
    (local.set $i (i32.add (local.get $i) (i32.const 1)))
    (br_if $loop_map_iter_new (i32.lt_u (local.get $i) (local.get $num_buckets) ))
  end
  (i32.const 0)
  return
)

;; increment an interator for traversing map pairs
;; in effect, this finds the next node of a given node, by first looking
;; at the linked list, then re-hashing the key to look through the rest of the hash table
(func $wax::map_iter_next (param $m i32) (param $iter i32) (result i32)
  (local $next i32)
  (local $num_buckets i32)
  (local $node i32)
  (local $i i32)
  
  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))

  (local.set $next (call $wax::_mapnode_get_next (local.get $iter)))

  (if (i32.eqz (local.get $next))(then

    (local.set $i (i32.add (call $wax::_map_hash_h
      (local.get $num_buckets)
      (call $wax::_mapnode_get_key_ptr  (local.get $iter))
      (call $wax::_mapnode_get_key_size (local.get $iter))
    ) (i32.const 1)))

    
    (if (i32.eq (local.get $i) (local.get $num_buckets)) (then
      (i32.const 0)
      return
    ))
    
    loop $loop_map_iter_next
      (local.set $node (call $wax::_map_get_bucket (local.get $m) (local.get $i)))
      (if (i32.eqz (local.get $node))(then)(else
        (local.get $node)
        return
      ))
      (local.set $i (i32.add (local.get $i) (i32.const 1)))
      (br_if $loop_map_iter_next (i32.lt_u (local.get $i) (local.get $num_buckets) ))
    end

    (i32.const 0)
    return

  )(else
    (local.get $next)
    return
  ))
  (i32.const 0)
  return
)

;; given a map iterator, get a pointer to the key stored
(func $wax::map_iter_key_h (param $iter i32) (result i32)
  (call $wax::_mapnode_get_key_ptr (local.get $iter))
)
;; given a map iterator, read the key stored as an int
;; only works if your key is an i32
(func $wax::map_iter_key_i (param $iter i32) (result i32)
  (i32.load (call $wax::_mapnode_get_key_ptr (local.get $iter)))
)
;; given a map iterator, get a pointer to the value stored
(func $wax::map_iter_val (param $iter i32) (result i32)
  (i32.load (call $wax::_mapnode_get_val_ptr (local.get $iter)))
)

;; remove all key-values in the map
(func $wax::map_clear (param $m i32)
  (local $num_buckets i32)
  (local $hash i32)
  (local $it i32)

  (local $next i32)

  (local.set $num_buckets (call $wax::_map_get_num_buckets (local.get $m)))

  (local.set $hash (i32.const 0))

  loop $loop_map_clear_buckets

    (local.set $it (call $wax::_map_get_bucket (local.get $m) (local.get $hash)))

    loop $loop_map_clear_nodes
      (if (i32.eqz (local.get $it))(then)(else
        (local.set $next (call $wax::_mapnode_get_next (local.get $it)))

        (call $wax::free (local.get $it))

        (local.set $it (local.get $next))
        (br $loop_map_clear_nodes)
      ))
    end

    (call $wax::_map_set_bucket (local.get $m) (local.get $hash) (i32.const 0))

    (local.set $hash (i32.add (local.get $hash) (i32.const 1)))
    (br_if $loop_map_clear_buckets (i32.lt_u (local.get $hash) (local.get $num_buckets)))

  end  
)

;; free all allocated memory for a map
(func $wax::map_free (param $m i32)
  (call $wax::map_clear (local.get $m))
  (call $wax::free (local.get $m))
)