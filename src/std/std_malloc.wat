;;========================================================;;
;;     BASELINE MALLOC WITH HANDWRITTEN WEBASSEMBLY       ;;
;;========================================================;;
;; 32-bit implicit-free-list first-fit baseline malloc    ;;
;;--------------------------------------------------------;;

;; IMPLICIT FREE LIST:
;; Worse utilization and throughput than explicit/segregated, but easier
;; to implement :P
;;
;; HEAP LO                                                         HEAP HI
;; +---------------------+---------------------+...+---------------------+
;; | HDR | PAYLOAD | FTR | HDR | PAYLOAD | FTR |...+ HDR | PAYLOAD | FTR |
;; +----------^----------+---------------------+...+---------------------+
;;            |_ i.e. user data
;;           
;; LAYOUT OF A BLOCK:
;; Since memory is aligned to multiple of 4 bytes, the last two bits of
;; payload_size is redundant. Therefore the last bit of header is used to
;; store the is_free flag.
;; 
;; |---- HEADER (4b)----
;; |    ,--payload size (x4)--.     ,-is free?
;; | 0b . . . . . . . . . . . . 0  0
;; |------ PAYLOAD -----
;; |
;; |  user data (N x 4b)
;; |
;; |---- FOOTER (4b)---- (duplicate of header)
;; |    ,--payload size (x4)--.     ,-is free?
;; | 0b . . . . . . . . . . . . 0  0
;; |--------------------
;;
;; FORMULAS:
;; (these formulas are used throughout the code, so they're listed here
;; instead of explained each time encountered)
;;
;; payload_size = block_size - (header_size + footer_size) = block_size - 8
;; 
;; payload_pointer = header_pointer + header_size = header_pointer + 4
;;
;; footer_pointer = header_pointer + header_size + payload_size
;;                = (header_pointer + payload_size) + 4
;;
;; next_header_pointer = footer_pointer + footer_size = footer_pointer + 4
;;
;; prev_footer_pointer = header_pointer - footer_size = header_pointer - 4

(memory $wax::mem 1)                                ;; start with 1 page (64K)
(export "mem" (memory $wax::mem))
;;// (global $wax::min_addr (mut i32) (i32.const 0))  ;; set by wax compiler depending on data section size
(global $wax::max_addr (mut i32) (i32.const 65536)) ;; initial heap size (64K)
(global $wax::heap_did_init (mut i32) (i32.const 0))     ;; init() called?

;; helpers to pack/unpack payload_size/is_free from header/footer
;; by masking out bits

;; read payload_size from header/footer given pointer to header/footer
(func $wax::hdr_get_size (param $ptr i32) (result i32)
  (i32.and (i32.load (local.get $ptr)) (i32.const 0xFFFFFFFC))
)
;; read is_free from header/footer
(func $wax::hdr_get_free (param $ptr i32) (result i32)
  (i32.and (i32.load (local.get $ptr)) (i32.const 0x00000001))
)
;; write payload_size to header/footer
(func $wax::hdr_set_size (param $ptr i32) (param $n i32) 
  (i32.store (local.get $ptr) (i32.or
    (i32.and (i32.load (local.get $ptr)) (i32.const 0x00000003))
    (local.get $n)
  ))
)
;; write is_free to header/footer
(func $wax::hdr_set_free (param $ptr i32) (param $n i32)
  (i32.store (local.get $ptr) (i32.or
    (i32.and (i32.load (local.get $ptr)) (i32.const 0xFFFFFFFE))
    (local.get $n)
  ))
)
;; align memory by 4 bytes
(func $wax::align4 (param $x i32) (result i32)
  (i32.and
    (i32.add (local.get $x) (i32.const 3))
    (i32.const -4)
  )
)

;; initialize heap
;; make the whole heap a big free block
;; - automatically invoked by first malloc() call
;; - can be manually called to nuke the whole heap
(func $wax::init_heap
  (i32.store (i32.const 0) (global.get $wax::min_addr) )
  ;; write payload_size to header and footer
  (call $wax::hdr_set_size (global.get $wax::min_addr) 
    (i32.sub (i32.sub (global.get $wax::max_addr) (global.get $wax::min_addr)) (i32.const 8))
  )
  (call $wax::hdr_set_size (i32.sub (global.get $wax::max_addr) (i32.const 4))
    (i32.sub (i32.sub (global.get $wax::max_addr) (global.get $wax::min_addr)) (i32.const 8))
  )
  ;; write is_free to header and footer
  (call $wax::hdr_set_free (global.get $wax::min_addr) (i32.const 1))
  (call $wax::hdr_set_free (i32.sub (global.get $wax::max_addr) (i32.const 4)) (i32.const 1))

  ;; set flag to tell malloc() that we've already called init()
  (global.set $wax::heap_did_init (i32.const 1)) 
)

;; extend (grow) the heap (to accomodate more blocks)
;; parameter: number of pages (64K) to grow
;; - automatically invoked by malloc() when current heap has insufficient free space
;; - can be manually called to get more space in advance
(func $wax::extend (param $n_pages i32)
  (local $n_bytes i32)
  (local $ftr i32)
  (local $prev_ftr i32)
  (local $prev_hdr i32)
  (local $prev_size i32)

  (local.set $prev_ftr (i32.sub (global.get $wax::max_addr) (i32.const 4)) )

  ;; compute number of bytes from page count (1page = 64K = 65536bytes)
  (local.set $n_bytes (i32.mul (local.get $n_pages) (i32.const 65536)))

  ;; system call to grow memory (`drop` discards the (useless) return value of memory.grow)
  (drop (memory.grow (local.get $n_pages) ))

  ;; make the newly acquired memory a big free block
  (call $wax::hdr_set_size (global.get $wax::max_addr) (i32.sub (local.get $n_bytes) (i32.const 8)))
  (call $wax::hdr_set_free (global.get $wax::max_addr) (i32.const 1))

  (global.set $wax::max_addr (i32.add (global.get $wax::max_addr) (local.get $n_bytes) ))
  (local.set $ftr (i32.sub (global.get $wax::max_addr) (i32.const 4)))

  (call $wax::hdr_set_size (local.get $ftr)
    (i32.sub (local.get $n_bytes) (i32.const 8))
  )
  (call $wax::hdr_set_free (local.get $ftr) (i32.const 1))

  ;; see if we can join the new block with the last block of the old heap
  (if (i32.eqz (call $wax::hdr_get_free (local.get $prev_ftr)))(then)(else

    ;; the last block is free, join it.
    (local.set $prev_size (call $wax::hdr_get_size (local.get $prev_ftr)))
    (local.set $prev_hdr
      (i32.sub (i32.sub (local.get $prev_ftr) (local.get $prev_size)) (i32.const 4))
    )
    (call $wax::hdr_set_size (local.get $prev_hdr)
      (i32.add (local.get $prev_size) (local.get $n_bytes) )
    )
    (call $wax::hdr_set_size (local.get $ftr)
      (i32.add (local.get $prev_size) (local.get $n_bytes) )
    )
  ))

)

;; find a free block that fit the request number of bytes
;; modifies the heap once a candidate is found
;; first-fit: not the best policy, but the simplest
(func $wax::find (param $n_bytes i32) (result i32)
  (local $ptr i32)
  (local $size i32)
  (local $is_free i32)
  (local $pay_ptr i32)
  (local $rest i32)

  ;; loop through all blocks
  (local.set $ptr (global.get $wax::min_addr))
  loop $search
    ;; we reached the end of heap and haven't found anything, return NULL
    (if (i32.lt_u (local.get $ptr) (global.get $wax::max_addr))(then)(else
      (i32.const 0)
      return
    ))

    ;; read info about current block
    (local.set $size    (call $wax::hdr_get_size (local.get $ptr)))
    (local.set $is_free (call $wax::hdr_get_free (local.get $ptr)))
    (local.set $pay_ptr (i32.add (local.get $ptr) (i32.const 4) ))

    ;; check if the current block is free
    (if (i32.eq (local.get $is_free) (i32.const 1))(then

      ;; it's free, but too small, move on
      (if (i32.gt_u (local.get $n_bytes) (local.get $size))(then
        (local.set $ptr (i32.add (local.get $ptr) (i32.add (local.get $size) (i32.const 8))))
        (br $search)

      ;; it's free, and large enough to be split into two blocks
      )(else(if (i32.lt_u (local.get $n_bytes) (i32.sub (local.get $size) (i32.const 8)))(then
        ;; OLD HEAP
        ;; ...+-------------------------------------------+...
        ;; ...| HDR |              FREE             | FTR |...
        ;; ...+-------------------------------------------+...
        ;; NEW HEAP
        ;; ...+---------------------+---------------------+...
        ;; ...| HDR | ALLOC   | FTR | HDR |  FREE   | FTR |...
        ;; ...+---------------------+---------------------+...

        ;; size of the remaining half
        (local.set $rest (i32.sub (i32.sub (local.get $size) (local.get $n_bytes) ) (i32.const 8)))

        ;; update headers and footers to reflect the change (see FORMULAS)

        (call $wax::hdr_set_size (local.get $ptr) (local.get $n_bytes))
        (call $wax::hdr_set_free (local.get $ptr) (i32.const 0))

        (call $wax::hdr_set_size (i32.add (i32.add (local.get $ptr) (local.get $n_bytes)) (i32.const 4))
          (local.get $n_bytes)
        )
        (call $wax::hdr_set_free (i32.add (i32.add (local.get $ptr) (local.get $n_bytes)) (i32.const 4))
          (i32.const 0)
        )
        (call $wax::hdr_set_size (i32.add (i32.add (local.get $ptr) (local.get $n_bytes)) (i32.const 8))
          (local.get $rest)
        )
        (call $wax::hdr_set_free (i32.add (i32.add (local.get $ptr) (local.get $n_bytes)) (i32.const 8))
          (i32.const 1)
        )
        (call $wax::hdr_set_size (i32.add (i32.add (local.get $ptr) (local.get $size)) (i32.const 4))
          (local.get $rest)
        )

        (local.get $pay_ptr)
        return

      )(else
        ;; the block is free, but not large enough to be split into two blocks 
        ;; we return the whole block as one
        (call $wax::hdr_set_free (local.get $ptr) (i32.const 0))
        (call $wax::hdr_set_free (i32.add (i32.add (local.get $ptr) (local.get $size)) (i32.const 4))
          (i32.const 0)
        )
        (local.get $pay_ptr)
        return
      ))))
    )(else
      ;; the block is not free, we move on to the next block
      (local.set $ptr (i32.add (local.get $ptr) (i32.add (local.get $size) (i32.const 8))))
      (br $search)
    ))
  end

  ;; theoratically we will not reach here
  ;; return NULL
  (i32.const 0)
)


;; malloc - allocate the requested number of bytes on the heap
;; returns a pointer to the block of memory allocated
;; returns NULL (0) when OOM
;; if heap is not large enough, grows it via extend()
(func $wax::malloc (param $n_bytes i32) (result i32)
  (local $ptr i32)
  (local $n_pages i32)

  ;; call init() if we haven't done so yet
  (if (i32.eqz (global.get $wax::heap_did_init)) (then
    (call $wax::init_heap)
  ))

  ;; payload size is aligned to multiple of 4
  (local.set $n_bytes (call $wax::align4 (local.get $n_bytes)))

  ;; attempt allocation
  (local.set $ptr (call $wax::find (local.get $n_bytes)) )

  ;; NULL -> OOM -> extend heap
  (if (i32.eqz (local.get $ptr))(then
    ;; compute # of pages from # of bytes, rounding up
    (local.set $n_pages
      (i32.div_u 
        (i32.add (local.get $n_bytes) (i32.const 65527) )
        (i32.const 65528)
      )
    )
    (call $wax::extend (local.get $n_pages))

    ;; try again
    (local.set $ptr (call $wax::find (local.get $n_bytes)) )
  ))
  (local.get $ptr)
)

;; free - free an allocated block given a pointer to it
(func $wax::free (param $ptr i32)
  (local $hdr i32)
  (local $ftr i32)
  (local $size i32)
  (local $prev_hdr i32)
  (local $prev_ftr i32)
  (local $prev_size i32)
  (local $prev_free i32)
  (local $next_hdr i32)
  (local $next_ftr i32)
  (local $next_size i32)
  (local $next_free i32)
  
  ;; step I: mark the block as free

  (local.set $hdr (i32.sub (local.get $ptr) (i32.const 4)))
  (local.set $size (call $wax::hdr_get_size (local.get $hdr)))
  (local.set $ftr (i32.add (i32.add (local.get $hdr) (local.get $size)) (i32.const 4)))

  (call $wax::hdr_set_free (local.get $hdr) (i32.const 1))
  (call $wax::hdr_set_free (local.get $ftr) (i32.const 1))

  ;; step II: try coalasce

  ;; coalasce with previous block

  ;; check that we're not already the first block
  (if (i32.eq (local.get $hdr) (global.get $wax::min_addr)) (then)(else

    ;; read info about previous block
    (local.set $prev_ftr (i32.sub (local.get $hdr) (i32.const 4)))
    (local.set $prev_size (call $wax::hdr_get_size (local.get $prev_ftr)))
    (local.set $prev_hdr 
      (i32.sub (i32.sub (local.get $prev_ftr) (local.get $prev_size)) (i32.const 4))
    )

    ;; check if previous block is free -> merge them
    (if (i32.eqz (call $wax::hdr_get_free (local.get $prev_ftr))) (then) (else
      (local.set $size (i32.add (i32.add (local.get $size) (local.get $prev_size)) (i32.const 8)))
      (call $wax::hdr_set_size (local.get $prev_hdr) (local.get $size))
      (call $wax::hdr_set_size (local.get $ftr) (local.get $size))

      ;; set current header pointer to previous header
      (local.set $hdr (local.get $prev_hdr))
    ))
  ))

  ;; coalasce with next block

  (local.set $next_hdr (i32.add (local.get $ftr) (i32.const 4)))

  ;; check that we're not already the last block
  (if (i32.eq (local.get $next_hdr) (global.get $wax::max_addr)) (then)(else
    
    ;; read info about next block
    (local.set $next_size (call $wax::hdr_get_size (local.get $next_hdr)))
    (local.set $next_ftr 
      (i32.add (i32.add (local.get $next_hdr) (local.get $next_size)) (i32.const 4))
    )

    ;; check if next block is free -> merge them
    (if (i32.eqz (call $wax::hdr_get_free (local.get $next_hdr))) (then) (else
      (local.set $size (i32.add (i32.add (local.get $size) (local.get $next_size)) (i32.const 8)))
      (call $wax::hdr_set_size (local.get $hdr) (local.get $size))
      (call $wax::hdr_set_size (local.get $next_ftr) (local.get $size))
    ))

  ))

)
;; copy a block of memory over, from src pointer to dst pointer
;; WebAssembly seems to be planning to support memory.copy
;; until then, this function uses a loop and i32.store8/load8
(func $wax::memcpy (param $dst i32) (param $src i32) (param $n_bytes i32)
  (local $ptr i32)
  (local $offset i32)
  (local $data i32)

  (if (i32.eqz (local.get $n_bytes))(then
    return
  ))

  (local.set $offset (i32.const 0))

  loop $cpy
    (local.set $data (i32.load8_u (i32.add (local.get $src) (local.get $offset))))
    (i32.store8 (i32.add (local.get $dst) (local.get $offset)) (local.get $data))

    (local.set $offset (i32.add (local.get $offset) (i32.const 1)))
    (br_if $cpy (i32.lt_u (local.get $offset) (local.get $n_bytes)))
  end
)

;; reallocate memory to new size
;; currently does not support contraction
;; nothing will happen if n_bytes is smaller than current payload size
(func $wax::realloc (param $ptr i32) (param $n_bytes i32) (result i32)
  (local $hdr i32)
  (local $next_hdr i32)
  (local $next_ftr i32)
  (local $next_size i32)
  (local $ftr i32)
  (local $size i32)
  (local $rest_hdr i32)
  (local $rest_size i32)
  (local $new_ptr i32)

  (local.set $hdr (i32.sub (local.get $ptr) (i32.const 4)))
  (local.set $size (call $wax::hdr_get_size (local.get $hdr)))

  (if (i32.gt_u (local.get $n_bytes) (local.get $size)) (then) (else
    (local.get $ptr)
    return
  ))

  ;; payload size is aligned to multiple of 4
  (local.set $n_bytes (call $wax::align4 (local.get $n_bytes)))

  (local.set $next_hdr (i32.add (i32.add (local.get $hdr) (local.get $size)) (i32.const 8)))

  ;; Method I: try to expand the current block

  ;; check that we're not already the last block
  (if (i32.lt_u (local.get $next_hdr) (global.get $wax::max_addr) )(then
    (if (call $wax::hdr_get_free (local.get $next_hdr)) (then

      (local.set $next_size (call $wax::hdr_get_size (local.get $next_hdr)))
      (local.set $rest_size (i32.sub 
        (local.get $next_size)
        (i32.sub (local.get $n_bytes) (local.get $size))
      ))
      (local.set $next_ftr (i32.add (i32.add (local.get $next_hdr) (local.get $next_size)) (i32.const 4)))

      ;; next block is big enough to be split into two
      (if (i32.gt_s (local.get $rest_size) (i32.const 0) ) (then
        
        (call $wax::hdr_set_size (local.get $hdr) (local.get $n_bytes))
        
        (local.set $ftr (i32.add (i32.add (local.get $hdr) (local.get $n_bytes) ) (i32.const 4)))
        (call $wax::hdr_set_size (local.get $ftr) (local.get $n_bytes))
        (call $wax::hdr_set_free (local.get $ftr) (i32.const 0))

        (local.set $rest_hdr (i32.add (local.get $ftr) (i32.const 4) ))
        (call $wax::hdr_set_size (local.get $rest_hdr) (local.get $rest_size))
        (call $wax::hdr_set_free (local.get $rest_hdr) (i32.const 1))

        (call $wax::hdr_set_size (local.get $next_ftr) (local.get $rest_size))
        (call $wax::hdr_set_free (local.get $next_ftr) (i32.const 1))

        (local.get $ptr)
        return

      ;; next block is not big enough to be split, but is
      ;; big enough to merge with the current one into one
      )(else (if (i32.gt_s (local.get $rest_size) (i32.const -9) ) (then
      
        (local.set $size (i32.add (i32.add (local.get $size) (i32.const 8) ) (local.get $next_size)))
        (call $wax::hdr_set_size (local.get $hdr) (local.get $size))
        (call $wax::hdr_set_size (local.get $next_ftr) (local.get $size))
        (call $wax::hdr_set_free (local.get $next_ftr) (i32.const 0))

        (local.get $ptr)
        return
      ))))

    ))
  ))

  ;; Method II: allocate a new block and copy over

  (local.set $new_ptr (call $wax::malloc (local.get $n_bytes)))
  (call $wax::memcpy (local.get $new_ptr) (local.get $ptr) (local.get $size))
  (call $wax::free (local.get $ptr))
  (local.get $new_ptr)

)

(func $wax::calloc (param $n_bytes i32) (result i32)
  (local $ptr i32)
  (local $offset i32)
  (local.set $ptr (call $wax::malloc (local.get $n_bytes)))
  (local.set $offset (i32.const 0))
  loop $zero
    (if (i32.lt_u (local.get $offset) (local.get $n_bytes)) (then
      (i32.store8 (i32.add (local.get $offset) (local.get $ptr) ) (i32.const 0))
      (local.set $offset (i32.add (local.get $offset) (i32.const 1)))
      (br $zero)
    ))
  end
  (local.get $ptr)
  return
)

(func $wax::memmove (param $dst i32) (param $src i32) (param $n_bytes i32)
  (local $ptr i32)
  (local $offset i32)
  (local $data i32)

  (if (i32.eqz (local.get $n_bytes))(then
    return
  ))
  
  (if (i32.gt_u (local.get $dst) (local.get $src)) (then
    (local.set $offset (i32.sub (local.get $n_bytes) (i32.const 1)))
    loop $cpy_rev
      (local.set $data (i32.load8_u (i32.add (local.get $src) (local.get $offset))))
      (i32.store8 (i32.add (local.get $dst) (local.get $offset)) (local.get $data))

      (local.set $offset (i32.sub (local.get $offset) (i32.const 1)))
      (br_if $cpy_rev (i32.gt_s (local.get $offset) (i32.const -1)))
    end
  
  )(else
    (local.set $offset (i32.const 0))
    loop $cpy
      (local.set $data (i32.load8_u (i32.add (local.get $src) (local.get $offset))))
      (i32.store8 (i32.add (local.get $dst) (local.get $offset)) (local.get $data))

      (local.set $offset (i32.add (local.get $offset) (i32.const 1)))
      (br_if $cpy (i32.lt_u (local.get $offset) (local.get $n_bytes)))
    end
  ))
)
