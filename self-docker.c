#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif //_GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mount.h>


int main(int argc, char** argv){
	pid_t pid = getpid();

	// Create veth pair
	system("sudo ip link add veth0 type veth peer name veth1");
	// Create bridge
	system("sudo ip link add br1 type bridge");
	// Set up veth0 and br1
	system("sudo ip link set dev veth0 up");
	system("sudo ip link set dev br1 up");
	// Create new namespace
	system("sudo ip netns add myns");
	// Move veth1 into new namespace
	system("sudo ip link set veth1 netns myns");
	// Set up veth1 in new namespace
	system("sudo ip netns exec myns ip link set dev veth1 up");
	// Assign local ip address to veth0, veth1 and br1
	system("sudo ip addr add 10.10.0.10/16 dev veth0");
	system("sudo ip addr add 10.10.0.1/16 dev br1");
	system("sudo ip netns exec myns ip addr add 10.10.0.20/16 dev veth1");
	// Assign default gateway for myns namespace to forward all ip addresses not in ip table to default gateway
	system("sudo ip netns exec myns ip route add default via 10.10.0.1");
	// Adjust firewall setting to allow fowarding of packets
	system("sudo iptables -P FORWARD ACCEPT");
	// Add a network translation table
	system("sudo iptables --table nat -A POSTROUTING -s 10.10.0.0/16 ! -o br1 -j MASQUERADE");


	// Initialise unshare flags
	int UNSHARE_FLAGS = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS; // | CLONE_NEWNET;
	unshare(UNSHARE_FLAGS);
	printf("unshare\n");
	// Set host name
	char* hostname = "version1";
	sethostname(hostname, strlen(hostname));
	printf("sethostname\n");
	// Make subtree in new mount namespace private from host namespace
	mount("", "/", NULL, MS_REC | MS_SLAVE, NULL);
    
	char * rootfs = "./bundle/rootfs";
	// mount new rootfs
	mount(rootfs, rootfs, NULL, MS_REC | MS_PRIVATE | MS_BIND, NULL);
	// chdir to new rootfs
	chdir(rootfs);
    // pivot to new root file system
	syscall(SYS_pivot_root, ".", ".");
	chdir("/");
	mount("", ".", NULL, MS_REC|MS_SLAVE, NULL);
	umount2(".", MNT_DETACH); 
	// chdir("/"); //? i thought change alr

	pid = fork();

	int status;

	if (pid == 0){
		printf("in child process\n");

		mkdir("/proc", 0755); // from mnt namespace doc
		mount("proc", "/proc", "proc", 0, NULL);
		printf("proc mounted\n");

		printf("in init process\n");
		execvp(argv[1], &argv[1]);

		while (wait(&status) != -1 || errno != ECHILD);

	}
	wait(&status);
	exit(0);
}