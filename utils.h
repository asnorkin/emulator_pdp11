#ifndef UTILS_H
#define UTILS_H

#define error_exit(msg) do {                    \
                            perror(msg);        \
                            exit(EXIT_FAILURE); \
                        } while (0)

#endif // UTILS_H
