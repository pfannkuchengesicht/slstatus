#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config_reader.h"


Config_t *open_config(char *filename)
{
  FILE *cfg = fopen(filename, "r");
  if(cfg == NULL)
  {
    return NULL;
  }

  Config_t *tmp = malloc(sizeof(Config_t));
  if(tmp == NULL)
  {
    return NULL;
  }

  tmp->cfg_file = cfg;

  return tmp;
}

KeyValue_t *read_next(Config_t *config)
{
  static char key_buffer[MAX_KEY_LENGTH] = {0};
  static char value_buffer[MAX_VALUE_LENGTH] = {0};

  static KeyValue_t kv;
  kv.key = key_buffer;
  kv.value = value_buffer;

  char buffer[MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1] = {0}; 
 
  if(config == NULL || config->cfg_file == NULL)
  {
    return NULL;
  }

  if(fgets(buffer, MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1, config->cfg_file) == NULL)
  {
    return NULL;
  }

  char *eq = strchr(buffer, '=');
  size_t offset;
  if(eq == NULL || eq == buffer)
  {
    return NULL;
  }

  offset = eq - buffer;

  strncpy(key_buffer, buffer, (offset > MAX_KEY_LENGTH)?MAX_KEY_LENGTH:offset);
  key_buffer[(offset > MAX_KEY_LENGTH)?MAX_KEY_LENGTH:offset] = 0;
  strncpy(value_buffer, buffer+offset+1, MAX_VALUE_LENGTH);
  value_buffer[strlen(buffer+offset+1)] = 0;
  if(value_buffer[strlen(value_buffer) - 1] == '\n') value_buffer[strlen(value_buffer) - 1] = 0;
  return &kv;
}

void close_config(Config_t *config)
{
  if(config == NULL || config->cfg_file == NULL)
    return;

  fclose(config->cfg_file);
  free(config);
}