#include "dat.h"
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void su(const char *user) {
    errno = 0;
    struct passwd *pwent = getpwnam(user);
    if (errno) {
        twarn("getpwnam(\"%s\")", user);
        exit(32);
    }
    if (!pwent) {
        twarnx("getpwnam(\"%s\"): no such user", user);
        exit(33);
    }

    int r = setgid(pwent->pw_gid);
    if (r == -1) {
        twarn("setgid(%d \"%s\")", pwent->pw_gid, user);
        exit(34);
    }

    r = setuid(pwent->pw_uid);
    if (r == -1) {
        twarn("setuid(%d \"%s\")", pwent->pw_uid, user);
        exit(34);
    }
}

static void handle_sigterm_pid1(int _unused) {
    exit(143);
}

static void set_sig_handlers() {
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    int r = sigemptyset(&sa.sa_mask);
    if (r == -1) {
        twarn("sigemptyset()");
        exit(111);
    }

    r = sigaction(SIGPIPE, &sa, 0);
    if (r == -1) {
        twarn("sigaction(SIGPIPE)");
        exit(111);
    }

    sa.sa_handler = enter_drain_mode;
    r = sigaction(SIGUSR1, &sa, 0);
    if (r == -1) {
        twarn("sigaction(SIGUSR1)");
        exit(111);
    }

    // Workaround for running the server with pid=1 in Docker.
    // Handle SIGTERM so the server is killed immediately and
    // not after 10 seconds timeout. See issue #527.
    if (getpid() == 1) {
        sa.sa_handler = handle_sigterm_pid1;
        r = sigaction(SIGTERM, &sa, 0);
        if (r == -1) {
            twarn("sigaction(SIGTERM)");
            exit(111);
        }
    }
}

static const char *get_port_from_env() {
    const char *port = getenv("BEANSTALKD_PORT");
    if (!port) {
        port = "11300"; // default port
    } else if (strlen(port) > 5) {
        twarnx("BEANSTALKD_PORT is too long, maximum length is %d", 5);
        exit(4);
    }
    return port;
}

static const char *get_password_from_env() {
    const char *password = getenv("BEANSTALKD_PASSWORD");
    if (password && strlen(password) > MAX_PASSWORD_LEN - 1) {
        twarnx("BEANSTALKD_PASSWORD is too long, maximum length is %d", MAX_PASSWORD_LEN - 1);
        exit(5);
    }
    return password;
}

int main(int argc, char **argv) {
    // load the server configuration
    srv.port = (char *)get_port_from_env();
    srv.password = (char *)get_password_from_env();

    printf("version: %s\n", version);
    if (srv.port) {
        printf("port: %s\n", srv.port);
    } else {
        printf("no port set, using default %s\n", srv.port);
    }
    if (srv.password != NULL && srv.password[0] != '\0') {
        printf("using password\n");
    } else {
        printf("password is empty\n");
    }

    UNUSED_PARAMETER(argc);

    progname = argv[0];
    setlinebuf(stdout);
    optparse(&srv, argv + 1);

    if (verbose) {
        printf("pid %d\n", getpid());
    }

    int r = make_server_socket(srv.addr, srv.port);
    if (r == -1) {
        twarnx("make_server_socket()");
        exit(111);
    }

    srv.sock.fd = r;

    prot_init();

    if (srv.user)
        su(srv.user);
    set_sig_handlers();

    srv_acquire_wal(&srv);
    srvserve(&srv);
    exit(0);
}
