# iniParser

This implementation of an ini-file parser was specifically made for microcontrollers initialisation.
Predefined variables can be directly initialized from a file.

Only static memory allocation is used. The file is parsed in place, therefore no additional memory allocation or buffers are needed.

The file format supports sections, key-value pairs, and comments.

## File Requirements

*   Section and key names are case-sensitive.
*   The file must only contain ASCII characters.
*   All key-value pairs must be inside of a section.
*   Key-value pairs must be contained to one line.
*   Key names must not contain whitespace.
*   Key names must not contain a '=' character.
*   Comments must be on a separate line.
*   Lines started with a ';' character are considered a comment.
*   Repeated values will overwrite the previous value
*   Tabs are not considered as whitespace and should not be used or they become part of the section or key name.


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
