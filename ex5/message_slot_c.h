#ifndef MESSAGE_SLOT_
#define MESSAGE_SLOT_

typedef struct Message_slot Smessage;

typedef struct message_slot_info MSinfo;

// According to
// https://stackoverflow.com/a/22658358
// ioctl does not require the major number of the driver, but actually a unique
// number. While the major is unique to the driver, if hard coded, there is no
// guarantee that it is available.
// Even so I will use the MAGIC_NUM as the major for the driver.
#define MAGIC_NUM 247

/* Set the message of the device driver */
#define IOCTL_SET_CHAN _IOW(MAGIC_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FOLDER "/dev"
#define BUF_LEN 128
#define NUM_CHANNELS 4

#endif /* !MESSAGE_SLOT_ */