# tweeter

This project demonstrates thread synchronization using semaphores.

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

1. Indicate the purpose of each semaphore you introduce.
2. To what must each semaphore be initialized and why?

The arrays of in, out semaphores indicates if a specific user can stream
their tweet to the tweeter or tweeter can stream their tweets to users.
This helps keep the integrity of tweets before they are sent to the
tweeter thread or read from the users. They must all be initialized to 1
so they can act as mutexes.

3. Discuss how you attempt to maximize concurrency in your design.

To maximize concurrency, I only blocked on points where different threads
accessed shared global data. All other points in the programs are
completely concurrent.
