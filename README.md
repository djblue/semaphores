# tweeter

This project demonstrates thread synchronization using semaphores to
simulate the tweeter social network.

# Commands

The following 6 commands are supported:

## Handle <handle>

The `Handle` command simply stores a users handle locally to the user
function. This is useful for signing tweets later on.

## Start <tag>

The `Start` command will allow a user to start streaming a tweet to the
tweeter thread. Each line is sent separately and the tweet is associated
with the provided `<tag>` in the tweeter bank of tweets.

## End <tag>

The `End` command will finish a started tweet.

## Follow <tag>

The `Follow` command will allow a user to receive all tweets currently
stored in the tweeter bank that have the associated `<tag>`.

NOTE: the search on the tweeter bank is a basic linear search that uses
`strcmp` to compare each tweet's tag with the provided search tag.

## Read

The `Read` command will randomly sleep a given user thread.

NOTE: this wont effect the interaction of the stream and tweeter threads
with other users.

## Exit

The `Exit` command will logout a user from their current session.

# Output

The output is of the following form:

```
[thread identifier]: state/execution information
```

To see an example run, see the `out` directory. The output files are
labeled `<n>.txt`, where `<n>` is the number of users used to generate
that output file. Ex: `./tweeter 2` will be stored in `out/2.txt`.

# Data Structures

The main data structure in the program is the tweet:

```
typedef struct {
  char tag[20];     // tag assocaite with tweet
  char body[141];   // body of the tweet
  int len;          // the length of the body
  int done;         // is the tweet complete or partial
  char handle[20];  // handle of users that made the tweet
} tweet;
```

All other variables are either arrays or primitive types.

# Issues

The main issues I ran into with writing the application was running into
deadlocks. In order to deal with these issues, I would trace the output
and see which thread was blocking and how far they made it through their
execution path. Thus, the output was the best source of debugging
deadlocks.

# Design and Analysis

The program is essentially the following:

```
MAX_USERS 10

struct tweet {
  char body[141]   # body of the tweet
  char tag[20]     # tag assocaite with tweet
  bool done        # is the tweet complete or partial
}

sem_t in[MAX_USERS] = 0
tweet in_buff[MAX_USERS] = [1..1]

sem_t out[MAX_USERS] = [1..1]
tweet out_buff[MAX_USERS]

semt_t to_tweeter_mutex = 1
tweet to_tweeter = NULL

sem_t handle_mutex = 1
handle h = NULL

def tweeter():
  while true:

    wait(to_tweeter_mutex)
    # process tweet if done, clear tweet
    signalto_tweeter_mutex)

    wait(following)
    # process handle, stream tweets to user
    signal(following)


def streamer():
  while true:
    for i = 0 to n:
      wait(in[i])
      if in_buff[i].done:
        wait(tweeted)
        to_tweeter = in_buffer[i]
        signal(tweeted)
      signal(in[i])

def user(id):
  for each command in commands:
    switch command:
      case 'Handle':
        # read handle

      case 'Start':
        for each line in tweet:
          wait(in[id])
          # stream tweet line to streamer
          signal(in[id])

      case 'Follow':
        while true:
          wait(out[id])
          # access to tweet
          signal(out[id])
          if done break

      case 'Read':
        wait(rand())

      case 'Exit':
        return null
      

def main(n):
  create_thread(tweeter)
  create_thread(streamer)
  for i = 0 to n:
    create_thread(user, i)
```

