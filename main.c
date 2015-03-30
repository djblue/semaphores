#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "colors.h"

#define MAX_USERS 10

int n;

typedef struct {
  char tag[20];
  char body[141];
  int len;
  int done;
} tweet;

int completed;

tweet in_buff[MAX_USERS];
tweet out_buff[MAX_USERS];
sem_t in_stream[MAX_USERS], done[MAX_USERS], tweet_streamed, tweet_processed;

void escape(char *buff, char *str) {
  int i = 0;
  char *c;
  c = str;
  while (*c) {
    if (*c == '\n') {
      buff[i] = '\\';
      i++;
      buff[i] = 'n';
      i++;
    } else {
      buff[i] = *c;
      i++;
    }
    c++;
  }
  buff[i] = '\0';
}

void *tweeter () {

  tweet bank[1024];
  int bank_len = 0;

  char buff[1024];
  printf("[  tweeter ]: running...\n");

  while (1) {

    sem_wait(&tweet_streamed);

    escape(buff, in_buff[completed].body);
    printf("[  tweeter ]: processing tweet tag = %s, body = "__green("\"%s\"")"\n", in_buff[completed].tag, buff);

    strcpy(bank[bank_len].tag, in_buff[completed].tag);
    strcpy(bank[bank_len].body, in_buff[completed].body);
    bank_len++;

    printf("[  tweeter ]: tweet bank_len = %d\n", bank_len);

    sem_post(&tweet_processed);
  }

  printf("[  tweeter ]: stopping...\n");

  return NULL;

}

void *streamer () {

  printf("[ streamer ]: running...\n");

  for (int i = 0; i < n; i++) {
    in_buff[i].done = 0;
  }

  while (1) {
    for (int i = 0; i < n; i++) {
      sem_wait(&in_stream[i]);
      // a tweet has been completely streamed
      if (in_buff[i].done == 1) {
        printf("[ streamer ]: tweet streaming complete user = %d\n", i);
        in_buff[i].done = 0;
        sem_post(&tweet_streamed);
        completed = i;
        sem_wait(&tweet_processed);
        sem_post(&done[i]);
      }
      sem_post(&in_stream[i]);
    }
  }

  printf("[ streamer ]: stopping...\n");

}

void *user (void *i) {

  int id = *((int *) i);
  char line[141];
  char handle[20], follow[20], cmd[20], tag[20];
  printf("[  user %d  ]: going online...\n", id);


  // open command file
  char filename[256];
  sprintf(filename, "users/user%d.txt", id);
  printf("[  user %d  ]: opening command file...\n", id);
  FILE *cmds = fopen(filename, "r");

  if (cmds == NULL) {
    printf("[  user %d  ]: error opening command file. filename = %s.\n", id, filename);
    exit(1);
  }

  fscanf(cmds, "%s", &cmd);
  fscanf(cmds, "%s", &handle);
  printf("[  user %d  ]: handle = %s\n", id, handle);

  while (fscanf(cmds, "%s", &cmd)) {

    printf("[  user %d  ]: cmd = %s\n", id, cmd);

    switch (cmd[0]) {
      case 'S':
        sem_wait(&in_stream[id]);
        fscanf(cmds, "%s\n", in_buff[id].tag);
        printf("[  user %d  ]: tweeting tag = %s\n", id, in_buff[id].tag);
        in_buff[id].len = 0;
        in_buff[id].done = 0;
        sem_post(&in_stream[id]);

        // stream tweet
        while (1) {
          fgets(line, 140, cmds);
          int c = strlen(line);
          if (line[0] == 'E') {

            sem_wait(&in_stream[id]);
            printf("[  user %d  ]: streaming complete\n", id);
            in_buff[id].body[in_buff[id].len] = '\0';
            in_buff[id].done = 1;
            sem_post(&in_stream[id]);

            sem_wait(&done[id]); // wait for tweeter to read tweet
            break;

          } else {

            sem_wait(&in_stream[id]);
            printf("[  user %d  ]: streaming c = %d, line = %s", id, c, line);
            memcpy(in_buff[id].body + in_buff[id].len, line, c);
            in_buff[id].len += c;
            sem_post(&in_stream[id]);

          }
        }
        break;

      case 'F':
        fscanf(cmds, "%s", follow);
        printf("[  user %d  ]: following %s\n", id, follow);
        break;

      case 'R': {
        int n = rand() % 10;
        printf("[  user %d  ]: reading for time = %ds\n", id, n);
        for (int i = 0; i < n; i++) {
          printf("[  user %d  ]: %ds before done reading\n", id, n - i);
          sleep(1);
        }
        break;
      }

      case 'E':
        printf("[  user %d  ]: closing command file...\n", id);
        fclose(cmds);
        printf("[  user %d  ]: going offline...\n", id);
        return NULL;
    }
  }

}

void usage (char *bin) {
  printf("USAGE: %s <n>\n", bin);
  exit(1);
}

int main (int argc, char **argv) {

  printf("[   main   ]: parsing command line arguments...\n");

  int my_ids[MAX_USERS];
  pthread_t tweet, stream, tid[MAX_USERS];

  if (argc < 2) {
    printf("[   main   ]: error specify number of users n\n");
    usage(argv[0]);
  }

  if (sscanf(argv[1], "%d", &n) != 1) {
    printf("[   main   ]: error n must be an integer\n");
    usage(argv[0]);
  }

  if (n < 1 || n > MAX_USERS) {
    printf("[   main   ]: error incorrect n (0 < n <= %d).\n", MAX_USERS);
    usage(argv[0]);
  }

  printf("[   main   ]: initializing semaphores...\n");

  sem_init(&tweet_streamed, 0, 0);
  sem_init(&tweet_processed, 0, 0);

	for (int i = 0; i < n; i++) {
    sem_init(&in_stream[i], 0, 1);
    sem_init(&done[i], 0, 0);
  }

  printf("[   main   ]: starting the tweeter and streamer threads...\n");

  // launch the tweeters and streamers (1 of each)
	if (pthread_create(&tweet, NULL, tweeter, NULL) != 0) {
    printf("[   main   ]: can't create start tweeter thread.\n");
    exit(1);
  }
	if (pthread_create(&stream, NULL, streamer, NULL) != 0) {
    printf("[   main   ]: error can't create start streamer thread.\n");
    exit(1);
  }

  printf("[   main   ]: starting %d user threads...\n", n);

  // launch all the user threads
	for (int i = 0; i < n; i++) {
    my_ids[i] = i;
		if (pthread_create(&tid[i], NULL, user, &my_ids[i]) != 0) {
      printf("[   main   ]: error can't create thread = %d.\n", i);
      exit(1);
    }
  }

  // wait for all user threads to complete (like a boss)
	for (int i = 0; i < n; i++) {
		pthread_join(tid[i], NULL);

  }

  printf("[   main   ]: gracefully exiting...\n");
  return 0;
}

