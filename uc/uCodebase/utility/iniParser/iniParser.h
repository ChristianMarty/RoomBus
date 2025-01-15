//**********************************************************************************************************************
// FileName : iniParser.h
// FilePath : /utility/iniParser
// Author   : Christian Marty
// Date		: 12.01.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    iniParser_integer,
    iniParser_number,
    iniParser_string
} iniParser_item_type_t;

typedef struct {
    const char* name;
    const iniParser_item_type_t type;
    const union {
        int* integer;
        float* number;
        char* string;
    } data;
    const int dataSize;
} iniParser_item_t;

typedef struct {
    const char* name;
    const iniParser_item_t *items;
    const uint8_t numberOfItems;
} iniParser_section_t;

typedef struct {
    const iniParser_section_t *sections;
    const uint8_t numberOfSections;

    const iniParser_section_t *_currentSection;
} iniParser_t;

void iniParser_run(iniParser_t *iniParser, const char* line, uint8_t maxLength);

#ifdef __cplusplus
}
#endif