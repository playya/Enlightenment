#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Ecore_Data.h>

Ecore_Hash *ini_file_cache;

/* Function Prototypes */
char *get_t(char *icon);
char *parse_exec(char *exec);
char *parse_buffer(char *b, char *section);
void parse_desktop_file(char *path);
void parse_debian_file(char *file);
void process_file(char *file, G_Eap *eap);
void parse_ini_init(void);
Ecore_Hash *parse_ini_file(char *file);
void parse_ini_shutdown(void);

#endif
