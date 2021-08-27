# Quickstart

A quick tour of the wax language to get started.

## Variables & types

There are only 7 types in wax. 

- `int`: integer
- `float`: floating-point number
- `string`: string
- `vec`: fixed size array
- `arr`: dynamically resizable array
- `map`: hashtables
- `struct` : user defined data structures

See [Appendix](#appendix) for how wax types map to types in the target languages.

### Variable Declaration


```scheme
(let x int)

```

Shorthand for initializing to a value.

```scheme
(let x int 42)

```

Declaring a compound type, array of floats:

```scheme
(let x (arr float))

```

An array of 3D vectors:

```scheme
(let x (arr (vec 3 float)))

```

Variables default to the zero value of their type when an initial value is not specified. `int` and `float` default to `0`. Other types default to null. Null is not a type itself in wax, but nullable objects can be nullified with expression `(null x)`. To check if a variable is NOT null, use `(?? x)` (equivalent to `x!=null` in other languages).

You can also use `local` in place of `let`, to declare variables that get automatically freed when it goes out of scope. See next section for details.

See [Appendix](#appendix) for reserved identifier names.


### Variable Assignment

```scheme
(set x 42)
```

## Math

Math in wax uses prefix notation like the rest of the language. e.g.:

```scheme
(+ 1 1)
```

When nested, `(1 + 2) *3` is:

```scheme
(* (+ 1 2) 3)
```

`+` `*` `&&` `||` can take multiple parameters, which is a shorthand that gets expanded by the compiler.

e.g., `1 + 2 + 3 + 4` is:

```scheme
(+ 1 2 3 4)
```
`a && b && c` is:

```scheme
(&& a b c)
```
which the compiler will read as:

```scheme
(&& (&& a b) c)
```


### Ternary operator

`?` is the ternary operator in wax. e.g., `y = (x==0) ? 1 : 2` is:

```scheme
(set y (? (= x 0) 1 2))
```


### List of operators

These operators work just like their namesakes in other languages.

```
<< >> = && || >= <= <>
+ - * / ^ % & | ! ~ < >
```

Note: `<>` is `!=`. `=` is `==`. `^` is xor.

Note: Wax ensures that `&&` and `||` are shortcircuited on all targets.

## Arrays and Vectors

### Initializing

To allocate a `vec` or an `arr`, use `(alloc <type>)`

```scheme
(let x (vec 3 float) (alloc (vec 3 float)))
```

To free it, use

```scheme
(free x)
```

**Important:** Freeing the container does not free the individual elements in it if the elements of the array is not a primitive type (`int`/`float`). Simple rule: if you `alloc`'ed something, you need to `free` it yourself. The container is more like a management service that helps you arrange data; it does not take ownership.


To allocate something that is automatically freed when it goes out of scope, use the `local` keyword to declare it.

```scheme
(local x (vec 3 float) (alloc (vec 3 float)))

```

The memory will be freed at the end of the block the variable belongs to, or immediately before any `return` statement. `local` variables cannot be returned or accessed out of its scope.

You can also use a literal to initialize the `vec` or `arr`, by listing the elements in `alloc` expression.


```scheme
(let x (arr float) (alloc (arr float) 1.0 2.0 3.0))

```

Think of it as

```java
float[] x = new float[] {1.0, 2.0, 3.0};
```

### Accessing

To get the `i`th element of `arr` `x`:

```scheme
(get x i)
```

To set the `i`th element of `arr` `x` to `v`;

```scheme
(set x i v)
```

`get` supports a "chaining" shorthand when you're accessing nested containers. For example, if `x` is a `(arr (arr int))` (2D array),

```scheme
(get x i j)
```
is equivalent to

```scheme
(get (get x i) j)
```

To set the same element to `v`, you'll need

```scheme
(set (get x i) j v)
```

If the array is 3D, then `get` will be:

```scheme
(get x i j k)
```
or (if you enjoy typing):

```scheme
(get (get (get x i) j) k)
```
and `set` will be:

```scheme
(set (get x i j) k v)
```

### Operations

To find out the length of an array `x`, use `#` operator:

```scheme
(# x)
```

`vec`'s length is already burnt into its type, so `#` is not needed.

To insert `v` into a an array `x` at index `i`, use

```scheme
(insert x i v)
```

So say to push to the end of the array, you might use

```scheme
(insert x (# x) v)
```

To remove `n` values starting from index `i` from array `x`, use

```scheme
(remove x i n)
```

To produce a new array that contains a range of values from an array `x`, starting from index `i` and with length `n`, use

```scheme
(set y (arr int) (slice x i n))
```

Note that if the result of `slice` operation is neither assigned to anything nor returned, it would be a memory leak since `slice` allocates a new array.

These four are the only operations with syntax level support (`#`, `insert` `remove` and `slice` are keywords). Other methods can be implemented as function calls derived from these fundamental operations.


## Maps

```scheme
(let m (map str int) (alloc (map str int)))

(set m "xyz" 123)

(insert m "abc" 456) ; exactly same as 'set'

(print (get m "xyz"))

(remove m "xyz")

(print (get m "xyz")) 
;^ if a value is not there, the "zero" value of the element type is returned
; for numbers, 0; for compound types, null.

```

Map key type can be `int` `float` or `str`. Map value type can be anything.

## Structs

```scheme
(struct point
    (let x float)
    (let y float)
)
```
Structs are declared with `struct` keyword. In it, fields are listed with `let` expressions, though initial values cannot be specified (they'll be set to zero values of respective types when the struct gets allocated).

Another example: structs used for implementing linked lists might look something like this:

```scheme
(struct list
	(let len int)
	(let head (struct node))
	(let tail (struct node))
)
(struct node
	(let prev (struct node)) 
	(let next (struct node))
	(let data int)
)
```

Structs fields of struct type are always references. Think of them as pointers in C:

```c
struct node {
    struct node * prev;
    struct node * next;
    int data;
};
```

However the notion of "pointers" is hidden in wax; From user's perspective, all non-primitives (`arr`,`vec`,`map`,`str`,`struct`) are manipulated as references.

### Instantiating

```scheme
(let p (struct point) (alloc (struct point)))
```

To free:

```scheme
(free p)
```

The `local` keyword works for structs the same way it does for arrays and vectors.

### Accessing

The `get` and `set` keywords are overloaded for structs too.

To get field `x` of a `(struct point)` instance `p`:

```scheme
(get p x)
```

To set field `x` of struct `point` to `42`:

```scheme
(set p x 42.0)
```

If the struct is an element of an array, say the `j`th point of the `i`th polyline in the `arr` of `polylines`:

```scheme
(get polylines i j x)
(set (get polylines i j) x 42.0)
```

## Strings

In wax, string is an object similar to an array.

To initialize a new string:

```scheme
(let s str (alloc str "hello"))
```

To free the string:

```scheme
(free s)
```

To append to a string, use `<<` operator.

```scheme
(<< s " world!")
```

Now `s` is modified in place, to become `"hello world!"`.

Note that a string does not always need to be allocated to be used, but it needs to be allocated if it needs to:

- be modified
- persist outside of its block

E.g. if all you want is to just print a string:

```scheme
(let s str "hello world")
(print s)
```

is OK. (And so is `(print "hello world")`)

The right-hand-side of string `<<` operator does not have to be allocated, while the left-hand-side must be.

If the function returns a string, it needs to be allocated.

To add a character to a string, `<<` can also be used:

```scheme
(<< s 'a')
```
Note that `'a'` is just an integer (ASCII of `a` is 97). It's the same as:

```scheme
(<< s 97)
```

To add the string `"97"` instead, `cast` expression can be used (see more about casting in next section):

```scheme
(<< s (cast 97 str))
```

Strings can be compared with `=` and `<>` equality tests. They actually check if the strings contain the same content, NOT just checking if they're the exact same object.

```scheme
(let s str (alloc str "hello"))
(<< s "!!")
(print (= s "hello!!"))
;; prints 1
```

To find out the length of a string:

```scheme
(# s)
```

To get a character from a string:


```scheme
(let s str "hello")
(let c int (get s 0)) ;; 'h'==104
```

To copy part of a string into a new string use `(slice s i n)` the same way as `slice` for `arr`:

```scheme
(let s str "hello")
(slice s 1 3)  ;; "ell"
```

## Casting

Ints and Floats can be cast to each other implicitly. You can also use `(cast var to_type)` to do so explicitly:

```scheme
(let x float 3.14)
(let y int (cast x int))
```

Numbers can be cast to and from string with the same `cast` keyword.

```scheme
(let x float (cast "3.14" float))
(let y str (cast x str))
```

In other words, `cast` is also responsible for doing `parseInt` `parseFloat` `toString` present in other languages.

Types other than `int` `float` `str` cannot be `cast`ed. You can define and call custom functions to do the job.

## Control Flow


### If statement

```scheme
(if (= x 42) (then
    (print "the answer is 42")
))
```

with else:

```scheme
(if (= x 42) (then
    (print "the answer is 42")
)(else
    (print "what?")
))
```

else if:

```scheme
(if (= x 42) (then
    (print "the answer is 42")
)(else (if (< x 42) (then
    (print "too small!")
)(else (if (> x 42) (then
    (print "too large!")
)(else    
    (print "impossible!")
))))))
```
with `&&` and `||` and `!`:

```scheme
(if (|| (! (&& (= x 42) (= y 666))) (< z 0)) (then
    (print "complicated logic evaluates to true")
))

```


### For loops


```scheme
(for i 0 (< i 100) 1 (do
    (print "i is:")
    (print i)
))
```

The first argument is the looper variable name, the second is starting value, the third is stopping condition, the fourth is the increment, the fifth is a `(do ...)` expression containing the body of the loop. It is equivalent to:

```java
for (int i = 0; i < 100; i+= 1){

}
```
Looping backwards (99, 98, 97, ..., 2, 1, 0), iterating over the same set of numbers as above:

```scheme
(for i 99 (>= i 0) -1 (do

))
```
Looping with a step of 3 (0, 3, 6, 9, ...):

```scheme
(for i 0 (< i 100) 3 (do

))
```

### Iterate over a Map

```scheme
(let m (map str int) (alloc (map str int)))

; populate ...

(for k v m (do
    (print "key is")
    (print k)
    (print "val is")
    (print v)
))
```

### While loops

```scheme
(while (<> x 0) (do

))
```
which is equivalent to

```java
while (x != 0){

}
```

### Break

Break works for both 

```scheme
(while (<> x 0) (do
    (if (= y 0) (then
        (break)
    ))
))
```

## Functions

A minimal function:

```scheme
(func foo 
    (return)
)
```

A simple function that adds to integers, returning an int:

```scheme
(func add_ints (param x int) (param y int) (result int)
    (return (+ x y))
)
```
A function that adds 1 to each element in an array of floats, in-place:

```scheme
(func add_one (param a (arr float))
    (for i 0 (< i (# a)) 1 (do
        (set a i (+ (get a i) 1.0))
    ))
)
```

Fibonacci:

```scheme
(func fib (param i int) (result int)
	(if (<= i 1) (then
		(return i)
	))
	(return (+
		(call fib (- i 1))
		(call fib (- i 2))
	))
)
```

### Calling

To call a function, use `call` keyword, followed by function name, and the list of arguments.

```scheme
(call foo 1 2)
```

### The main function

The main function is optional. If you're making a library, then you probably don't want to include a main function. If you do, the main function will map to the main function of the target language, (if the target language has the notion of main function, that is).

The main function has 2 signatures, similar to C. One that takes no argument, and returns an int that is the exit code. The other one takes one argument, which is an array of strings containing the commandline arguments, and returns the exit code.

```scheme
(func main (result int)
    (return 0)
)

(func main (param args (arr str)) (result int)
    (for i 0 (< i (# args)) 1 (do
        (print (get args i))
    ))
    (return 0)
)
```

### Function Signature

Functions need to be defined before they're called. Therefore for mutually recursive functions, (or for organizing code), it is useful to declare a signature first.

```scheme
(func foo (param x int) (param y int) (result int))
```

It looks just like a function without body. Therefore, to instead define a  void function that actually does nothing at all, a single `return` needs to be the body to make it not turn into a function signature.

```scheme
(func do_nothing
    (return)
)
```

### File Layout

Functions and structures can only be defined on the top level. So your source code file might looks something like this:

```scheme
;; constants
(let XYZ int 1)
(let ZYX float 2.0)

;; data structures
(struct Foo
    (let x int 3)
)

;; implementation
(func foo
    (return)
)
(func bar
    (return)
)

;; optional main function
(func main (result int)
    (call foo)
    (call bar)
)
```

## Macros

wax supports C-like preprocessor directives. In wax, macros look like other expressions, but the keywords are prefixed with `@`.


```scheme
(@define MY_CONSTANT 5)

(@if MY_CONSTANT 5
    (print "yes, it's")
    (print @MY_CONSTANT)
)
```

after it goes through the preprocessor, the above becomes:

```scheme
(print "yes, it's")
(print 5)
```

Note the `@` in `@MY_CONSTANT` when it is used outside of a macro expression.

If a macro is not defined, and is tested in an `@if` macro, the value defaults to `0`:

```scheme
(@if IM_NOT_DEFINED 1
    (print "never gets printed")
)
(@if IM_NOT_DEFINED 0
    (print "always gets printed")
)

```
The second argument to `@define` can be omitted, which makes it default to `1`:

```scheme
(@define IMPLICITLY_ONE)

(print "printing '1' now:")
(print @IMPLICITLY_ONE)
```

### Including Files

To include another source code, use:

```scheme
(@include "path/to/file.wax")
```
The content of the included file gets dumped into exactly where this `@include` line is. To make sure a file doesn't get included multiple times, use:


```scheme
(@pragma once)
```

To include a standard library, include its name without quotes:

```scheme
(@include math)
```


### Target-specific behaviors

These macros are pre-defined to be `1` when the wax compiler is asked to compile to a specific language, so the user can specify different behavior for different languages:

```c
TARGET_C
TARGET_JAVA
TARGET_TS
...
```

For example:

```scheme
(@if TARGET_C 1
    (print "hello from C")
)
(@if TARGET_JAVA 1
    (print "hello from Java")
)
```

## External Functions

To call functions written in the target language, you can describe their signature with `extern` keyword so that the compiler doesn't yell at you for referring to undefined things.

For example:

```scheme
(extern sin (param x float) (result float))
```

Then in your code, you can write:

```scheme
(call sin 3.14)
```

(This is exactly how `(@include math)` is implemented: the functions get mapped to the math library of the target language with `extern`s)

You can also have extern variables in addition to functions:

```scheme
(extern PI float)
```

## Inline "Assembly"

You can embed fragments of the target language into wax, similar to embedding assembly in C, using the `(asm "...")` expression. For example:

```java
(@if TARGET_C 1
    (asm "printf(\"hello from C\n\");")
)
(@if TARGET_JAVA 1
    (asm "System.out.println(\"hello from Java\n\");")
)
(@if TARGET_TS 1
    (asm "console.log(\"hello from TypeScript\n\");")
)
```

## Appendix

### Datatype mapping

wax tries to give the generated code an "idiomatic" look & feel by mapping wax types directly to common types in target language whenever possible, in favor of rolling out custom ones.

|   | int | float | str | vec | arr | map |
|---|-----|-------|-----|-----|-----|-----|
| C | `int` | `float` | `char*` | `T*` | `w_arr_t*` (custom impl.) | `w_map_t*` (custom impl.) |
| Java | `int` | `float` | `String` | `T[]` | `ArrayList<T>` | `HashMap<K,V>` |
| TypeScript | `number` | `number` | `string` | `Array` | `Array` | `Record<K,V>` |
| Python | `int` | `float` | `str` | `list` | `list` | `dict` |
| C# | `int` | `float` | `string` | `T[]` | `List<T>` | `Dictionary<K,V>` |
| C++ | `int` | `float` | `std::string` | `std::array` | `std::vector` | `std::map` |
| Swift | `Int` | `Float` | `String?` | `w_Arr<T>?` (wraps `[T]`) | `w_Arr<T>?` (wraps `[T]`) | `w_Map<K,V>?` (wraps `Dictionary`) |
| Lua | `number` | `number` | `string` | `table` | `table` | `table` |


### Reserved identifier names


- Identifiers beginning with `w_` are reserved. (They're for wax standard library functions)
- Identifiers ending with `_` are reserved. (They're for resolving clashes with target language reserved words)
- Identifiers containing double underscore `__` are reserved. (They're for temporary variables generated by the compiler)

---------

- Identifiers colliding with target language reserved words are automatically resolved by `waxc` by appending `_` to their ends; Nevertheless, it's better to avoid them altogether.
- Identifiers must start with a letter `[A-z]`, and can contain any symbol that is not `{}[]()` or whitespaces. Non-alphanumeric symbols are automatically fixed by the compiler, e.g. `he||@-wor|d` is a valid identifier in wax, and will be translated to `heU7c__U7c__U40__U2d__worU7c__d` in target languages.



