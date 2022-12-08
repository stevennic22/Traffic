#define RETURN_NEG -1
#define TURN_ON 1
#define TURN_OFF 2
#define CHECK_STATE 3

// 5d039600-00fc-3981-9985-653653ce831f
#define MY_UUID(val) ("5D0396-" val "-43981-9985-653653CE831F")

enum processID {
  OFF,
  RLGL,
  TRAFFIC,
  RANDOM,
  FLASHR,
  FLASHY,
  FLASHG,
  FLASHA,
  LAST
};

struct light {
  int lPin;
  char descriptor[4];
  char UUID[37];
  unsigned long onTime;
};

struct connectionTracker {
  String msg;
  char cmd[8];
  unsigned long connStart;
};
