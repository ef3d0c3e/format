![Tests badge](https://github.com/ef3d0c3e/format/actions/workflows/tests.yml/badge.svg)

# Format

Format is a modern C formatting library.
It provides features similar to [libfmt](https://fmt.dev) or Python's formatting facility.

# Grammar

## Basis

Below are the basic building blocks of the grammar:

### Number

<p><b>number</b> <b> := </b> <b> integer </b> <em>parse literally from the format string</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <b>| '{' integer '}'</b> <em>parse integer and retrieve the corresponding argument in the argument list</em></p>

**Examples**
 * `format(out, "{:.5}", "Hello, World")`
 * `format(out, "{:15}", "Hello, World")`
 * `format(out, "{:.{1}}", "Hello, World", 4)`
 * `format(out, "{:[{1}]}", FORMAT_ARRAY(array), sizeof(array) / sizeof(array[0]))`

### Size

Size are parsed exactly like <a href="#number">Number</a>, except they are limited to `16384`.
When referring to *width*, they commonly refer to the number of UTF-8 code points, and not the number of bytes.

### Specifier

<p><b>spec</b> <b> := </b> <b> codepoint </b> <em>parse literally from the format string</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <b>| '{' integer '}'</b> <em>parse integer and retrieve the corresponding argument (as a string) in the argument list</em></p>

**Examples**

 * `format(out, "{:#[]}", "Hello") -> "[Hello]"`
 * `format(out, "{:#''}", "Hello") -> "'Hello'"`
 * `format(out, "{:{1}<10}", 15, ". ") -> "15. . . . "`

### Alignment

<p><b>alignment</b> <b> := </b> <a href="#specifier">spec?</a> <em> fill string</em> <b>'&lt;'</b> <em>left-aligned</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <b>|</b> <a href="#specifier">spec?</a> <em> fill string</em> <b>'&gt;'</b> <em>right-aligned</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <b>|</b> <a href="#specifier">spec?</a> <em> fill string</em> <b>'~'</b> <em>center-aligned</em></p>

Define the alignment mode and set the fill string for a formatted value.
When no alignment is specified, the default is to left-align.

Here are the possible alignment:
 * `<` (default) left-align
 * `>` right-align
 * `^` center-align

**Examples**

 * `format(out, "{:>10}", "Hello") -> "     Hello"`
 * `format(out, "{:<10}", "Hello") -> "Hello     "`
 * `format(out, "{:^10}", "Hello") -> "   Hello  "`
 * `format(out, "{:-^10}", "Hello") -> "---Hello--"`
 * `format(out, "{:{1}>10}", "Hello", "-_") -> "-_-_-Hello"`

## Type Formatting

### String

<p><b>format_string</b> := <a href="#alignment">alignment?</a> <em> alignment</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <a href="#size">size?</a> <em> width of the formatted string</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'#'</b> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#specifier">spec</a> <em> right quote</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#specifier">spec</a> <em> left quote</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; )? <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'.'</b> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#number">number</a> <em> precision, maximum number of BYTES to display</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; )? <br>
 &nbsp;&nbsp;&nbsp;&nbsp; ( <b>'s'</b> | <b>'?'</b> | <b>'x'</b> )? <em>display type</em></p>

**Examples**

 * `format(out, "{}", "Hello, World!") -> "Hello, World!"`
 * `format(out, "{:.5}", "Hello, World!") -> "Hello"`
 * `format(out, "{:#''}", "Hello") -> "'Hello'"`
 * `format(out, "{:#[]}", "Hello") -> "[Hello]"`
 * `format(out, "{:5}", "a") -> "a    "`
 * `format(out, "{:->5}", "a") -> "----a"`
 * `format(out, "{:5.2}", "Hello") -> "He   "`
 * `format(out, "{:?}", "\nT\x87") -> "\nT\x87"`
 * `format(out, "{:x}", "\nT\x87") -> "0x0AT0x87"`

### Char

<p><b>format_string</b> := <a href="#alignment">alignment?</a> <em> alignment</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <a href="#size">size?</a> <em> width of the formatted string</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'#'</b> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#specifier">spec</a> <em> right quote</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#specifier">spec</a> <em> left quote</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; )? <br>
 &nbsp;&nbsp;&nbsp;&nbsp; ( <b>'c'</b> | <b>'?'</b> | <b>'x'</b> )? <em>display type</em></p>

### Signed Integers

Most common signed integer types can be formatted: `signed char, short, int, long, long long`

<p><b>format_signed</b> := <a href="#alignment">alignment?</a> <em> alignment</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'-'</b> | <b>'+'</b> | <b>' '</b>)? <em>sign</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <b>'#'</b>? <em>alternate mode</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <b>'0'</b>? <em>align with <span class="tt">0</span>'s</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <a href="#size">size?</a> <em> width of the formatted number</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'.'</b> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#number">number</a> <em> precision</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; )? <br>
 &nbsp;&nbsp;&nbsp;&nbsp; ( <b>'x'</b> | <b>'X'</b> | <b>'b'</b> | <b>'B'</b> )? <em>display type</em></p>

**Sign**

The sign mode operates like printf's:
 * `-` (default) shows the sign for negative numbers only
 * `+` shows the sign for all numbers
 * ` ` shows the sign for negatives numbers, and displays a placeholder white space for other numbers

**Alternate mode**

Alternate mode operates like printf's:
when present, numbers formatted in hexadecimal, or binary will show the `0x`/`0X` or `0b`/`0B` prefix.

**Precision**

Precision operates like printf's, the value corresponds to the minimum width of the number to display. If the number requires less digits than the precision, then it's *prepended* with 0's.

For instance, `format(out, "{:.5}", -1)` will display `-00001`, while `format(out, "{:.5}", 123456)` will display `123456`.

**Display type**

The display type argument defines how the number is formatted:
 * `x`/`X` formats the number as hexadecimal, the uppercase `X` displays the hexadecimal as uppercase and the alternate mode prefix is displayed as `0X`
 * `b`/`B` formats the number as binary, the uppercase `B` displays the alternate mode's `0b` prefix as `0B`

By default, the number is formatted in base 10, like printf's `%d` or `%i`.

**Examples**

 * `format(out, "{}", 123) -> "123"`
 * `format(out, "{:x}", 123) -> "7b"`
 * `format(out, "{:#x}", 123) -> "0x7b"`
 * `format(out, "{:#X}", 123) -> "0x7B"`
 * `format(out, "{:05}", 1) -> "00001"`
 * `format(out, "{:05}", -1) -> "-0001"`
 * `format(out, "{:+}", 1) -> "+1"`
 * `format(out, "{: }", 1) -> " 1"`
 * `format(out, "{:.5}", 1) -> "-00001"`

### Unsigned Integers

Most common unsigned integer types can be formatted: `unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long`

<p><b>format_signed</b> := <a href="#alignment">alignment?</a> <em> alignment</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <b>'#'</b>? <em>alternate mode</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <b>'0'</b>? <em>align with <span class="tt">0</span>'s</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; <a href="#size">size?</a> <em> width of the formatted number</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; (<b>'.'</b> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <a href="#number">number</a> <em> precision</em> <br>
 &nbsp;&nbsp;&nbsp;&nbsp; )? <br>
 &nbsp;&nbsp;&nbsp;&nbsp; ( <b>'x'</b> | <b>'X'</b> | <b>'b'</b> | <b>'B'</b> )? <em>display type</em></p>

Unsigned integers are formatted similarly to Signed Integers, except they don't have the *sign* specifier.
See [Signed Integers](#signed_integers) for reference.

# Planned Features
 - Float formatting
 - Proper object formatting
