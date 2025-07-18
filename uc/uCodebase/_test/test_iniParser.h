//
// Created by Christian on 12.01.2025.
//

#ifndef TESTUCODEBASE_TEST_INIPARSER_H
#define TESTUCODEBASE_TEST_INIPARSER_H

#include "utility/iniParser/iniParser.h"


uint16_t line1 = 45;
float line2 = 7.5;
char line3[20];

float line21 = 9.9;
char line22[20];
float line23 = 9.9;


iniParser_item_t item1[] = {
    {"line1", iniParser_integer, {.integer=&line1}},
    {"line2", iniParser_number,  {.number =&line2}},
    {"line3", iniParser_string,  {.string ={&line3[0], sizeof(line3)}} }
};

iniParser_item_t item2[] = {
    {"line1", iniParser_number, {.number =&line21}},
    {"line2", iniParser_string, {.string ={&line22[0], sizeof(line3)}} },
    {"line3", iniParser_number, {.number =&line23}}
};

iniParser_section_t test1[] = {
    {.name = "test1", .items = &item1[0], .numberOfItems = sizeof(item1)/sizeof(iniParser_item_t)},
    {.name = "test2", .items = &item2[0], .numberOfItems = sizeof(item2)/sizeof(iniParser_item_t)}
};

iniParser_t dataset {
    .sections = &test1[0],
    .numberOfSections = sizeof(test1)/sizeof(iniParser_section_t)
};


TEST_CASE( "Ini Parser", "[iniParser]" ) {
    SECTION("Load valid file") {

        FILE *file = fopen("../test.ini", "r");

        if (file == nullptr) {
            FAIL("Test file test.ini could not be opened");
        }

        char line[100];
        while (fgets(line, sizeof(line), file)) {
            uint8_t i = 0;
            while (line[i] != '\n' && line[i] != '\r' && line[i] != 0x00) {
                i++;
            }
            line[i] = 0;
            REQUIRE(iniParser_run(&dataset, line, i));
        }
        fclose(file);

        REQUIRE(line1 == 1038);
        REQUIRE(line2 == 37.5f);
        REQUIRE(match_string(line3, "this is a string", sizeof("this is a string")));

        REQUIRE(line21 == 25.23f);
        REQUIRE(match_string(line22, "string2", sizeof("string2")));
        REQUIRE(line23 == 9.0f);
    }

    SECTION("Invalid section") {
        iniParser_reset(&dataset);
        const char line[] = "[section";
        REQUIRE(!iniParser_run(&dataset,line, sizeof(line)-1));
    }

    SECTION("Input not in section") {
        iniParser_reset(&dataset);
        const char line[] = "notInSection=1";
        REQUIRE(!iniParser_run(&dataset,line, sizeof(line)-1));
    }

    SECTION("Invalid key-value pair") {
        iniParser_reset(&dataset);
        const char section[] = "[test1]";
        REQUIRE(iniParser_run(&dataset,section, sizeof(section)-1));

        const char value1[] = "noValue";
        REQUIRE(!iniParser_run(&dataset,value1, sizeof(value1)-1));
    }
}


#endif //TESTUCODEBASE_TEST_INIPARSER_H
