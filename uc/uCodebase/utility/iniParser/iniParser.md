# iniParser

This implementation of an ini parser was specifically made for microcontrollers initialisation.
Predefined variables can be directly initialized from a file.

Data must be passed to the ini-parser line-by-line.
Only static memory allocation is used. The data is parsed in place, therefore no additional memory allocation or buffers are needed.

The file format supports sections, key-value pairs, and comments.

## File Requirements

*   The file must only contain ASCII characters.
*   Only spaces, but not tabs, is considered whitespace
*   Leading whitespace in a line is ignored (thereby permitted)
*   All key-value pairs must be inside of a section.
*   Section and key names are case-sensitive.
*   Whitespace surrounding or within the section name is part of the section name (e.g. [ Sect ion ] will be " Sect ion ")
*   Key-value pairs must be contained in one line.
*   Key names must not contain whitespace or '=' characters.
*   Repeated keys will overwrite the previous value
*   Lines starting with a ';' character are considered a comment.
*   Comments must be on their own separate line.



 ## Example

```plaintext
; This is example ini-file content.

[Section1]

Integer1= 1038
Number1= 37.5
String1= this is a string


[Section2]
; Sections can be empty

[Section3]
Integer1= 84
Number1= 38.9
String1= Another string
```
