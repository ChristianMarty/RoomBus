//**********************************************************************************************************************
// FileName : iniParser.c
// FilePath : /utility/iniParser
// Author   : Christian Marty
// Date		: 12.01.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "iniParser.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#include "../string.h"

bool _iniParser_stringIsEqual(const char *string1, const char *string2, uint8_t string2Length)
{
    uint8_t j = 0;
    while((string1[j] != 0) && (j<string2Length)){
        if(string1[j] != string2[j]) return false;
        j++;
    }
    return true;
}

void _iniParser_parsePair(iniParser_t *iniParser, const char* key, uint8_t keyLength, const char* value, uint8_t valueLength)
{
    const iniParser_item_t *item = NULL;

    for(uint8_t i = 0; i < iniParser->_currentSection->numberOfItems; i++){
        if(_iniParser_stringIsEqual(iniParser->_currentSection->items[i].name, key, keyLength)){
            item = &iniParser->_currentSection->items[i];
            break;
        }
    }
    if(item == NULL) return;

    switch(item->type){
        case iniParser_integer :
            *item->data.integer = string_parseInteger(value, valueLength);
            break;

        case iniParser_number :
            *item->data.number = string_parseNumber(value, valueLength);
            break;

        case iniParser_string : {
            uint8_t i = 0;
            while(value[i]){
                item->data.string.data[i] = value[i];
                i++;
            }
            break;
        }
    }
}

void iniParser_reset(iniParser_t *iniParser)
{
    iniParser->_currentSection = NULL;
}

bool iniParser_run(iniParser_t *iniParser, const char* line, uint8_t lineLength)
{
    if(!lineLength){
        return true;
    }

    uint8_t position = 0;
    // ignore leading whitespace
    while(line[position] == ' '){
        position++;
        if(position >= lineLength){
            return true;
        }
    }

    if(line[position] == '['){

        position++;
        const char *name = &line[position];
        uint8_t nameLength = position;

        while(line[position] != ']'){
            position++;
            if(position >= lineLength){
                return false;
            }
        }
        nameLength = position-nameLength;

        iniParser->_currentSection = NULL;
        for(uint8_t i = 0; i < iniParser->numberOfSections; i++){
            if(_iniParser_stringIsEqual(iniParser->sections[i].name, name, nameLength)){
                iniParser->_currentSection = &iniParser->sections[i];
                break;
            }
        }

    }else if(line[position] != ';'){
        if(iniParser->_currentSection == NULL){
            // line is not within a section
            return false;
        }

        const char *name = &line[position];
        uint8_t nameLength = position;
        while((line[position] != ' ') && (line[position] != '=')){
            position++;
            if(position >= lineLength){
                return false;
            }
        }
        nameLength = position-nameLength;

        while(line[position] == ' '){
            position++;
            if(position >= lineLength){
                return false;
            }
        }

        if(line[position] != '='){
            return false;
        }

        position++;
        while(line[position] == ' '){
            position++;
            if(position >= lineLength){
                return false;
            }
        }

        const char *value = &line[position];
        uint8_t valueLength = lineLength -position;

        _iniParser_parsePair(iniParser, name, nameLength, value, valueLength);
    }

    return true;
}

#ifdef __cplusplus
}
#endif