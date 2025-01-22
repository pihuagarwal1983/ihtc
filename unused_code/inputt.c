#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
//#include <cjson/cJSON.h>
#include "cJSON.h"
#include "cJSON.c"

typedef enum

void parse_json(const char* json_string) {
    // Parse the JSON string
    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *days = cJSON_GetObjectItem(root, "days");
    cJSON *skill_levels = cJSON_GetObjectItem(root, "skill_levels");
    if (cJSON_IsNumber(days) && cJSON_IsNumber(skill_levels)) {
        printf("days: %d\n", days->valueint);
        printf("skill_levels: %d\n", skill_levels->valueint);
    }

    cJSON *shift_types = cJSON_GetObjectItem(root, "shift_types");
    cJSON *shift_types = cJSON_GetObjectItem(root, "age_groups");
    if (cJSON_IsArray(shift_types) && cJSON_IsArray(age_groups)) {
        printf("shift_types: ");
        cJSON *shift;
        cJSON_ArrayForEach(shift, shift_types) {
            if (cJSON_IsNumber(shift)) {
                printf("%d, ", shift->valueint);
            }
        }
        printf("age_groups: ");
        cJSON *age;
        cJSON_ArrayForEach(age, age_groups) {
            if (cJSON_IsNumber(age)) {
                printf("%d, ", age->valueint);
            }
        }
        puts("\b\b ");
    }

    cJSON *occupants = cJSON_GetObjectItem(root, "occupants");
    if (cJSON_IsObject(occupants)) {
        cJSON *id = cJSON_GetObjectItem(occupants, "id");
        cJSON *gender = cJSON_GetObjectItem(occupants, "gender");
        cJSON *age_group = cJSON_GetObjectItem(occupants, "age_group");
        cJSON *length_of_stay = cJSON_GetObjectItem(occupants, "length_of_stay");
        //cJSON *zip = cJSON_GetObjectItem(occupants, "gender");

        if (cJSON_IsString(id) && cJSON_IsString(gender) && cJSON_IsString(age_group) && cJSON_IsNumber(length_of_stay)) {
            printf("id: %s\ngender: %s\nage_group: %s\nlength_of_stay: %s\n", \
                   id->valuestring, gender->valuestring);
        }
    }

    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (cJSON_IsString(name)) {
        printf("Name: %s\n", name->valuestring);
    }

    cJSON *is_student = cJSON_GetObjectItem(root, "is_student");
    if (cJSON_IsBool(is_student)) {
        printf("Is Student: %s\n", is_student->valueint ? "true" : "false");
    }

    // Clean up
    cJSON_Delete(root);
}

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File opening failed");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    fread(content, 1, length, file);
    fclose(file);
    content[length] = '\0';  // Null-terminate the string
    puts(content);
    return content;
}

void main (void)
{   char file_name[100] = "Training dataset/i01.json";
    char *json_string = read_file(file_name);
    parse_json(json_string);
    free(json_string);
}
