#ifndef PCC_CLIENT_H
#define PCC_CLIENT_H

#define ARG_FILE "-f"
#define USAGE "%s LEN [" ARG_FILE " SOURCE_FILENAME]\n\
The client reads LEN (positive integer) bytes from SOURCE_FILENAME (or\n\
from " CLIENT_DATASOURCE " if none is specified) and sends them to a\n\
server to count the number of readable characters. It then prints the\n\
number returned from the server.\n\
\n\
LEN - positive integer\n\
SOURCE_FILENAME - Valide filename\n"

// Debug messages
#define D_CLIENT_CONNETING "Client connecting...\n"

#endif // PCC_CLIENT_H
