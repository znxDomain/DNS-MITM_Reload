// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

static Service g_sfdnsresSrv;

void libnx_getaddrinfo(const char *hostname, const char *port)
{
    socketInitializeDefault();

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    printf("\n\nCalling getaddrinfo hostname: \"%s\" port \"%s\":\n", hostname, port);

    if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        printf("ERR getaddrinfo: %s, error nr: %d\n", gai_strerror(status), status);
    }

    printf("IP addresses for %s:\n", hostname);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // free the linked list
    socketExit();
}


// Main program entrypoint
int main(int argc, char* argv[])
{
    // This example uses a text console, as a simple way to output text to the screen.
    // If you want to write a software-rendered graphics application,
    //   take a look at the graphics/simplegfx example, which uses the libnx Framebuffer API instead.
    // If on the other hand you want to write an OpenGL based application,
    //   take a look at the graphics/opengl set of examples, which uses EGL instead.
    consoleInit(NULL);

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    // Other initialization goes here. As a demonstration, we print hello world.
    printf("Hello World!\n");
    printf("Press + to exit.\n");
    printf("Press X to relaod hosts file.\n");

    // Main loop
    while (appletMainLoop())
    {
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        if (kDown & HidNpadButton_X){
            char hostname[] = "switchbrew.org";
            char port[] = "80";

            Result rc = 0;
            libnx_getaddrinfo(hostname, port);

            printf("\n\nStarting Reload.\n");

            rc = smGetService(&g_sfdnsresSrv, "sfdnsres");
            printf("smGetService: %d\n", rc);
            
            rc = serviceDispatch(&g_sfdnsresSrv, 65000);
            printf("serviceDispatch: %d\n", rc);
            
            serviceClose(&g_sfdnsresSrv);
            printf("serviceClosed");

            libnx_getaddrinfo(hostname, port);

        }

        // Your code goes here

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }

    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    return 0;
}
