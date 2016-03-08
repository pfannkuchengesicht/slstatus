#ifndef __CONFIG_READER_H__
#define __CONFIG_READER_H__

#define MAX_KEY_LENGTH 128
#define MAX_VALUE_LENGTH 128

typedef struct
{
  FILE *cfg_file;
} Config_t;

typedef struct
{
  char *key;
  char *value;
} KeyValue_t;

Config_t *open_config(char *filename);
KeyValue_t *read_next(Config_t *config);
void close_config(Config_t *config);

#endif
