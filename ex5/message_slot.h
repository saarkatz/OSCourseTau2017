#ifndef MESSAGE_SLOT_
#define MESSAGE_SLOT_

typedef struct Message_slot Smessage;

typedef struct message_slot_info MSinfo;

/* The major device number. We can't rely on dynamic
* registration any more, because ioctls need to know
* it. */
#define MAGIC_NUM 247

/* Set the message of the device driver */
#define IOCTL_SET_CHAN _IOW(MAGIC_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FOLDER "/dev"
#define BUF_LEN 128
#define NUM_CHANNELS 4

#endif /* !MESSAGE_SLOT_ */