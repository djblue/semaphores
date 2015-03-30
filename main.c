#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_USERS 10

int n;

typedef struct {
  char tag[20];
  char body[141];
  int len;
  int done;
  char handle[20];
} tweet;

int completed;

sem_t in_stream[MAX_USERS], done[MAX_USERS], ready, tweet_streamed, tweet_processed;
int is_ready;
tweet in_buff[MAX_USERS];

sem_t out_stream[MAX_USERS], following, following_info, following_read;
int following_user, is_following;
char following_tag[20];
tweet out_buff[MAX_USERS];

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
  int r, f, user;
  char tag[20];

  char buff[1024];
  printf("[  tweeter ]: running...\n");

  while (1) {

    sem_wait(&ready);
    r = is_ready;
    sem_post(&ready);

    if (r == 1) {

      sem_wait(&tweet_streamed);

      escape(buff, in_buff[completed].body);
      printf("[  tweeter ]: processing tweet tag = %s, handle = %s, body = \"%s\"\n", in_buff[completed].tag, in_buff[completed].handle, buff);

      strcpy(bank[bank_len].tag, in_buff[completed].tag);
      strcpy(bank[bank_len].body, in_buff[completed].body);
      strcpy(bank[bank_len].handle, in_buff[completed].handle);
      bank_len++;

      printf("[  tweeter ]: tweet bank_len = %d\n", bank_len);

      sem_post(&tweet_processed);

      sem_wait(&ready);
      is_ready = 0;
      sem_post(&ready);
    }

    sem_wait(&following_info);
    f = is_following;
    user = following_user;
    strcpy(tag, following_tag);
    sem_post(&following_info);

    if (f == 1) {

      printf("[  tweeter ]: user i = %d is following tag = %s \n", user, tag);

      int found = 0;
      for (int i = 0; i < bank_len; i++) {
        printf("[  tweeter ]: checking tweet in bank i = %d for user = %d\n", i, user);
        if (strcmp(bank[i].tag, tag) == 0) {
          printf("[  tweeter ]: found a tweet with tag = %s\n", tag);
          sem_wait(&out_stream[user]);
          strcpy(out_buff[user].body, bank[i].body);
          strcpy(out_buff[user].handle, bank[i].handle);
          out_buff[user].done = 1;
          sem_post(&out_stream[user]);
          sem_wait(&following_read);
          found++;
        }
      }

      if (found == 0) {
        printf("[  tweeter ]: found no tweet with tag = %s\n", tag);
      }

      sem_wait(&following_info);
      is_following = 0;
      sem_post(&following_info);
      sem_post(&following);
    }


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
        sem_wait(&ready);
        is_ready = 1;
        sem_post(&ready);
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
        strcpy(in_buff[id].handle, handle);
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

      case 'F': {
        int f;
        char buff[1024];
        fscanf(cmds, "%s", follow);

        printf("[  user %d  ]: following %s\n", id, follow);

        // make sure no other users are following
        sem_wait(&following);
        sem_wait(&following_info);
        following_user = id;
        is_following = 1;
        strcpy(following_tag, follow);
        sem_post(&following_info);

        while (1) {

          sem_wait(&out_stream[id]);
          if (out_buff[id].done == 1) {
            out_buff[id].done = 0;
            escape(buff, out_buff[id].body);
            printf("[  user %d  ]: follow tweet tag = %s, handle = %s, body = \"%s\"\n", id, follow, out_buff[id].handle, buff);
            sem_post(&following_read);
          }
          sem_post(&out_stream[id]);

          sem_wait(&following_info);
          f = is_following;
          sem_post(&following_info);
          if (!f) break;
        }

        break;
      }

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
  sem_init(&following, 0, 1);
  sem_init(&following_info, 0, 1);
  sem_init(&following_read, 0, 0);
  sem_init(&ready, 0, 1);

  is_following = 0;
  is_ready = 0;

	for (int i = 0; i < n; i++) {
    sem_init(&in_stream[i], 0, 1);
    sem_init(&out_stream[i], 0, 1);
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

