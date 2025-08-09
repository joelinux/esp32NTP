#include <stdio.h>

char *secTostr(long total_seconds, char *result) {

    // Calculate days
    int days = total_seconds / (24 * 3600);
    total_seconds %= (24 * 3600);

    // Calculate hours
    int hours = total_seconds / 3600;
    total_seconds %= 3600;

    // Calculate minutes
    int minutes = total_seconds / 60;

    // Calculate remaining seconds
    int seconds = total_seconds % 60;

    sprintf(result, "%d:d %02d:%02d:%02d", 
           days, hours, minutes, seconds);

    return (result);
}
