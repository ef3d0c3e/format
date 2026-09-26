![Tests badge](https://github.com/ef3d0c3e/format/actions/workflows/tests.yml/badge.svg)

# Format

Format is a modern C formatting library.
It provides features similar to [libfmt](https://fmt.dev) or Python's formatting facility.

This library requires a Gnu99 compiler (gcc or clang). Mind you that the recommended standard version is Gnu23.
If you use this library in a Gnu99 project, you will get warnings when including the `include/format.h` header.
These warnings can be ignored, but you might want so silence them globally.

# Usage

## Setup

To use `format` as a library to your project, I recommend the following setup:
 * Fetch the library from git; either as a submodule, or via a make/cmake rule
 * Compile the static `libformat.a`: `make -C libs/format`
 * Link your binary with `libformat.a` and add `-I./libs/format/include` to access the library header `format.h`

```
# Add as git submodule, you might want to pin to a release version
git submodule add https://github.com/ef3d0c3e/format libs/format
```
Add this to your makefile:
```
# Static libformat.a location
LIBFORMAT_A := ./libs/format/libformat.a
# Add to IFLAGS and LFLAGS
IFLAGS += -I./libs/format/include/
LFLAGS += $(LIBFORMAT_A)

# Rule for building libformat.a
$(LIBFORMAT_A):
	@echo "Building libformat..."
	$(MAKE) -C $(dir $(LIBFORMAT_A))

# Add this all your targets that depend on libformat:
my-target: $(LIBFORMAT_A)
```

The default make target for format builds `libformat.a`, which is what you should be using in your projects.

Example program:
```
#include <format.h>

int main()
{
    // You can use __attribute__((cleanup(format_output_destroy))) to avoid the call to destroy at the end
    struct format_output out = format_output_file(stdout);

    format(&out, "Hello, {}!\n", "World");

    format_output_destroy(&out);
}
```

## Outputs

Format support 4 kinds of outputs, which is where `format` will write its output:
 * `format_output_file(FILE*)` this will output to a stdio `FILE*`, it's useful if you want to use `stdout`/`stderr` and let stdio handle flushing as it already does for `printf`.
 * `format_output_fd(int fd)` this will output to a file descriptor. By default, the output to file descriptor is buffered.
 * `format_output_buf()` this will output to an internal buffer, that will grow dynamically to hold the formatted strings. You can make multiple calls to `format`, which will grow the buffer to contain the concatenation of all messages.
 * `format_output_none()` this will not output, but keep track of size. This can be used to determine how many bytes it would take to format a given format string with its arguments.

At any given time, if you wish to flush the formatted content, call `format_output_flush(struct format_output*)`:
 * If the outputs is a `FILE*`, this will simply call `fflush()` on the contained `FILE*`.
 * If the output is a file descriptor, this will write the internal buffer.
 * Otherwise, this has no effects

You can also set the flushing strategy for file descriptors. By calling `format_output_set_flush(struct format_output* output, enum format_output_flush_mode mode)`.
Here are the available `format_output_flush_mode`:
 - `kFormatFlushNewline` flush on newlines (similar to stdio's default strategy for `stdout`/`stderr`)
 - `kFormatFlushNone` flush when the internal buffer is full
 - `kFormatFlushAlways` always flush. Use this if you don't want to have a buffer

**WARNING:** If you want to set the flushing mode, you must call `format_output_set_flush` BEFORE writing to the output. If you've called `format` on the output, the result is undefined.

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
 - Time formatting
