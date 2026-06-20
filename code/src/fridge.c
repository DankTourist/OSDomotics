#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEF_DELAY 60
#define DEF_TEMP 5
#define TEMP_PREC 0.1

typedef struct {
  int time = 0,
      delay = DEF_DELAY,
      perc = 100;
  float temp,
        thermostat = DEF_TEMP;
} registry;

int my_id;
int is_on = 0; // 0 = off, 1 = on
char my_fifo[128];
int fifo_fd;
registry my_reg;

void cleanup_and_exit(int sig) {
  printf("\n[Fridge %d] Shutting down...\n", my_id);
  close(fifo_fd);
  unlink(my_fifo); // Remove named pipe from filesystem
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./fridge <id>\n");
    exit(EXIT_FAILURE);
  }

  my_id = atoi(argv[1]);
  sprintf(my_fifo, "%s%d.fifo", FIFO_PATH_PREFIX, my_id);

  // termination
  signal(SIGTERM, cleanup_and_exit);
  signal(SIGINT, cleanup_and_exit);

  // FIFO for this specific device
  if (mkfifo(my_fifo, 0666) == -1 && errno != EEXIST) {
    perror("mkfifo failed");
    exit(EXIT_FAILURE);
  }

  printf("[Fridge %d] Ready. Listening on %s\n", my_id, my_fifo);

  // open FIFO for reading (blocks until a writer connects)
  // using O_RDWR prevents EOF when the writer closes the pipe
  fifo_fd = open(my_fifo, O_RDWR);
  if (fifo_fd < 0) {
    perror("open fifo failed");
    exit(EXIT_FAILURE);
  }

  time_t tmp_time;
  IPC_Message msg;
  
  while (1) {
    ssize_t bytes = read(fifo_fd, &msg, sizeof(IPC_Message));
    if (bytes > 0) {
      char **tokens = tokenise(msg.command);
      printf("[Fridge %d] Received command: %s\n", my_id, msg.command);

      // simulate processing latency (1 to 3 seconds)
      sleep((rand() % 3) + 1);
      
      /*if(atoi(msg.sender_id) == -1){
        handle_external_cmd(msg);
      }*/
      if (strcmp(tokens[0], "set ") == 0) {
        if (strcmp(tokens[1], "delay") == 0) {
          errno = 0;
          int delay = strtol(tokens[2], NULL, 10);
          if (errno) {
            perror("strtol()");
          } else {
            my_reg.delay = delay;
            printf("[Fridge %d] Set delay to %d\n", delay);
          }
        }
        if (strcmp(tokens[1], "thermo") == 0 ) {
          errno = 0;
          float thermo = strtof(tokens[2], NULL);
          if (errno) {
            perror("strtof()");
          } else {
            my_reg.thermostat = thermo;
            printf("[Fridge %d] Set target temperature to %f\n", thermo);
          }
        }
        if (strcmp(tokens[1], "perc") == 0 ) {
          errno = 0;
          int perc = strtol(tokens[2], NULL, 10);
          if (errno) {
            perror("strtof()");
          }
          if (perc < 0 || perc > 100) {
            printf("Percentage value is off-range (0-100)\n");
          } else {
            my_reg.perc = perc;
            printf("[Fridge %d] Set percentage to %d\n", perc);
          }
        }
      }
      if (strncmp(msg.command, "switch power on", 15) == 0) {
        tmp_time = time(NULL);
        is_on = 1;
        printf("[Fridge %d] Status changed to ON\n", my_id);
      } else if (strncmp(msg.command, "switch power off", 16) == 0) {
        my_reg.time += (time(NULL) - tmp_time);
        is_on = 0;
        printf("[Fridge %d] Status changed to OFF\n", my_id);
      }

      // TODO: Send acknowledgment back to Controller via CONTROLLER_FIFO
      // temp.
      char *msg_str;
      sprintf(msg, "done");
      IPC_Message ack_msg = { my_id, msg.sender_id, msg_str };
      
      if (write(parent_fifo, (char*)&ack_msg, sizeof(IPC_Message)) == -1) {
        errno = ERR_PIPE_BROKEN;
        perror("write()");
      }
    }

    if (is_on && (time(NULL) - tmp_time) >= msg.delay) {
      my_reg.time += (time(NULL) - tmp_time);
      is_on = 0;
    }
    if (my_reg.temp != my_reg.thermostat) {
      my_reg.temp += (my_reg.temp < my_reg.thermostat) ? 0.1 : -0.1;
    }
  }

  return 0;
}
