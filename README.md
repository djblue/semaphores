# tweeter

This project demonstrates thread synchronization using semaphores to
simulate the tweeter social network.

# Building

To build the project, do:

    make

To cleanup the project, do:

    make clean

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

# Pseudo Code

The following pseudo code represents the program:

```
# the in_buff is the buffer for users
# to stream their tweets
tweet in_buff[MAX_USERS]

# mutex to guad in_buff
sem_t in_stream[MAX_USERS] = [1..1]

# semaphores to make sure the tweet has been stored by
# tweeter before continuing, and possibly overwriting
# their previous tweet
sem_t done[MAX_USERS] = [0..0]

# semaphore for tweeter to signal streamer that a
# tweet has been completely processed
sem_t tweet_processed = 0

# mutex to guard the is_ready flag that is set when
# the streamer has recieved a tweet from a user
sem_t ready = 1

# mutexs to guad out_buff
sem_t out_stream[MAX_USERS] = [1..1]

# semaphore for signaling that a user wants to follow
# a tag
sem_t following = 1

# mutex for guarding shared following info such as
#   who is following, what tag they are following
sem_t following_info = 1

# semaphore for user to signal the streamer that they
# have finished reading the tweet
sem_t following_read = 0

def tweeter():
  while true:

    wait(ready)
    # process tweet if done, clear tweet
    if (is_ready):
      # process tweet
      signal(tweet_processed)
    signal(ready)

    wait(following)
    if(is_following):
      # stream tweet to user
      signal(following_processed)
    signal(following)


def streamer():
  while true:
    for i = 0 to n:
      wait(in_stream[i])
      if in_buff[i].done:
        wait(ready)
        is_ready = 1
        signal(ready)
        wait(tweet_processed)
      signal(in_stream[i])

def user(id):
  for each command in commands:
    switch command:
      case 'Handle':
        # read handle

      case 'Start':
        for each line in tweet:
          wait(in_buff[id])
          # stream tweet line to streamer
          signal(in_buff[id])

      case 'Follow':

        wait(following_processed)
        wait(following_info)
        # set user follow info (user, tag)
        signal(following_info)

        # recieve tweet stream
        while true:

          wait(out_stream[id])
          if out_buff[id].done:
            # access completely streamed tweet
           signal(out_stream[id])

          # check following flag to see if more tweets
          # are coming
          wait(following_info)
          f = is_following
          signal(following_info)
          if !f:
            break # all tweets have been streamed

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

# Test

To run all the test, do:

    make test
