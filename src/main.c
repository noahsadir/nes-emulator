/**
 * @file main.c
 * 
 * Copyright (c) 2022 Noah Sadir
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "main.h"

bool main_loadROM(char* path, FileBinary* binary) {
  FILE* fp;
  fp = fopen(path, "rb");

  if (fp == NULL) return false;

  uint8_t buffer[16];
  uint32_t n;

  // setup data struct
  fseek(fp, 0L, SEEK_END);
  binary->bytes = ftell(fp);
  binary->data = malloc(sizeof(uint8_t) * binary->bytes);
  rewind(fp);

  // populate binary object with file data
  uint32_t count = 0;
  while ((n = fread(buffer, 1, 16, fp)) > 0) {
    for (int i = 0; i < 16; i++) {
      binary->data[count] = buffer[i];
      count += 1;
    }
  }

  return true;
}

void main_compareCPUTraces() {
  #if (LOGGING)
  FILE* gfp;
  FILE* cfp;
  char* genLine = NULL;
  char* corLine = NULL;
  gfp = fopen("./debug/output.log", "r");
  cfp = fopen("./debug/nestest.log", "r");
  ssize_t gread;
  ssize_t cread;
  size_t glen = 0;
  size_t clen = 0;

  if (gfp == NULL || cfp == NULL) return;

  char cline1[81] = "";
  char cline2[81] = "";
  char cline3[81] = "";

  char gline1[81] = "";
  char gline2[81] = "";
  char gline3[81] = "";

  int line = 1;
  while ((gread = getline(&genLine, &glen, gfp)) != -1 && (cread = getline(&corLine, &clen, cfp)) != -1) {
    strncpy(cline1, cline2, 73);
    strncpy(cline2, cline3, 73);
    strncpy(cline3, corLine, 73);
    strncpy(gline1, gline2, 73);
    strncpy(gline2, gline3, 73);
    strncpy(gline3, genLine, 73);

    if (strcmp(gline3, cline3) != 0) {
      printf("(%04d) EXPECTED: `%s`\n", line - 2, cline1);
      printf("(%04d) ACTUAL:   `%s`\n\n", line - 2, gline1);
      printf("(%04d) EXPECTED: `%s`\n", line - 1, cline2);
      printf("(%04d) ACTUAL:   `%s`\n\n", line - 1, gline2);
      printf("(%04d) EXPECTED: `%s`\n", line, cline3);
      printf("(%04d) ACTUAL:   `%s`\n\n", line, gline3);
      printf("Mismatch at line %d\n", line);
      exit(1);
    }

    line += 1;
  }

  fclose(gfp);
  fclose(cfp);
  #endif
}

int main(int argc, char* argv[]) {
  char* filePath = (argc >= 2) ? argv[1] : "./rom.nes";

  FileBinary binary;

  if (!main_loadROM(filePath, &binary)) {
    bus_init(NULL);
  }

  logging_init();
  bus_init(&binary);
  free(binary.data);
  logging_kill();

  #if (CPU_DEBUG)
  main_compareCPUTraces();
  printf("** DEBUG ALERT ** Trace match successful!\n");
  #endif

  return 0;
}
