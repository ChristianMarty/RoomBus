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
    if(item == NULL)return;

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
                item->data.string[i] = value[i];
                i++;
            }
            break;
        }
    }
}

void iniParser_run(iniParser_t *iniParser, const char* line, uint8_t maxLength)
{
    uint8_t position = 0;
    while(line[position] == ' '){
        position++;
        if(position > maxLength)return;
    }
    if(line[position] == 0x00){
        return;
    }else if(line[position] == ';'){
        return;
    }else if(line[position] == '['){
        position++;

        const char *name = &line[position];
        uint8_t nameLength = position;
        while(line[position] != ']'){
            position++;
            if(position > maxLength)return;
        }
        nameLength = position-nameLength;

        for(uint8_t i = 0; i < iniParser->numberOfSections; i++){
            if(_iniParser_stringIsEqual(iniParser->sections[i].name, name, nameLength)){
                iniParser->_currentSection = &iniParser->sections[i];
                break;
            }
        }
    }else{
        if(iniParser->_currentSection == NULL) return;

        const char *name = &line[position];
        uint8_t nameLength = position;
        while((line[position] != ' ') && (line[position] != '=')){
            position++;
            if(position > maxLength)return;
        }
        nameLength = position-nameLength;

        position++;
        while((line[position] == ' ') || (line[position] == '=')){
            position++;
            if(position > maxLength)return;
        }

        const char *value = &line[position];
        uint8_t valueLength = position;
        while(line[position] != 0x00){
            position++;
            if(position > maxLength)return;
        }
        valueLength = position-valueLength;

        _iniParser_parsePair(iniParser, name, nameLength, value, valueLength);
    }
}

#ifdef __cplusplus
}
#endif