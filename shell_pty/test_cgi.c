#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[])
{
    char * req_method = getenv("REQUEST_METHOD");

    printf("Content-type: textml\r\n\r\n");
    printf(req_method);

}