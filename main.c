#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_USERS 10

typedef struct {
  char name[20];
  char body[140];
} tweet;

void *tweeter () {

  printf("tweeter: running...\n");

  printf("tweeter: stopping...\n");

}

void *streamer () {

  printf("streamer: running...\n");

  printf("streamer: stopping...\n");

}

void *user (void *i) {

  int id = *((int *) i);
  printf("user %d: going online...\n", id);





  printf("user %d: going offline...\n", id);

}

void usage (char *bin) {
  printf("USAGE: %s <n>\n", bin);
  exit(1);
}

int main (int argc, char **argv) {

  printf("main: parsing command line arguments...\n");

  int n, my_ids[MAX_USERS];
  pthread_t tweet, stream, tid[MAX_USERS];

  if (argc < 2) {
    printf("error: specify number of users n\n");
    usage(argv[0]);
  }

  if (sscanf(argv[1], "%d", &n) != 1) {
    printf("error: n must be an integer\n");
    usage(argv[0]);
  }

  if (n < 1 || n > MAX_USERS) {
    printf("error: incorrect n (0 < n <= %d).\n", MAX_USERS);
    usage(argv[0]);
  }

  printf("main: starting the tweeter and streamer threads...\n");

  // launch the tweeters and streamers (1 of each)
	if (pthread_create(&tweet, NULL, tweeter, NULL) != 0) {
    printf("error: can't create start tweeter thread.\n");
    exit(1);
  }
	if (pthread_create(&stream, NULL, streamer, NULL) != 0) {
    printf("error: can't create start streamer thread.\n");
    exit(1);
  }

  printf("main: starting %d user threads...\n", n);

  // launch all the user threads
	for (int i = 0; i < n; i++) {
    my_ids[i] = i;
		if (pthread_create(&tid[i], NULL, user, &my_ids[i]) != 0) {
      printf("error: can't create thread = %d.\n", i);
      exit(1);
    }
  }

  // wait for all user threads to complete (like a boss)
	for (int i = 0; i < n; i++) {
		pthread_join(tid[i], NULL);
  }

  printf("main: gracefully exiting...\n");
  return 0;
}
