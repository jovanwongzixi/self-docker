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
	system("ip link add veth0 type veth peer name veth1");
	// Create bridge
	system("ip link add br1 type bridge");
	// Link veth0 to bridge
	system("ip link set dev veth0 master br1");
	// Create new namespace
	system("ip netns add selfdocker");
	// Move veth1 into new namespace
	system("ip link set veth1 netns selfdocker");
	// Set ip address of veth1
	system("ip netns exec selfdocker ip addr add 10.10.0.2/16 dev veth1");
	// Set up veth0 and br1
	system("ip link set dev veth0 up");
	system("ip link set dev br1 up");
	// Set up veth1 in new namespace
	system("ip netns exec selfdocker ip link set dev veth1 up");
	// Set ip address of bridge
	system("ip addr add 10.10.0.1/16 dev br1");
	// Assign default gateway for myns namespace to forward all ip addresses not in ip table to default gateway
	system("ip netns exec selfdocker ip route add default via 10.10.0.1");
	// Adjust firewall setting to allow fowarding of packets
	system("iptables -P FORWARD ACCEPT");
	// Add a network translation table
	system("iptables --table nat -A POSTROUTING -s 10.10.0.0/16 ! -o br1 -j MASQUERADE");

	// Initialise unshare flags
	int UNSHARE_FLAGS = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS; // | CLONE_NEWNET;
	unshare(UNSHARE_FLAGS);

	// search for myns_ns namespace fd to set our network namespace in myns_ns
	int selfdocker_ns = open("/var/run/netns/selfdocker", O_RDONLY);

    if (selfdocker_ns == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (setns(selfdocker_ns, CLONE_NEWNET) == -1) {
        perror("setns");
        close(selfdocker_ns);
        exit(EXIT_FAILURE);
    }

    close(selfdocker_ns);

    // Initialise unshare flags
	int UNSHARE_FLAGS = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS; // | CLONE_NEWNET;
	unshare(UNSHARE_FLAGS);

	pid_t pid = fork();

	int status;

	if (pid == 0){
		// Make subtree in new mount namespace private from host namespace
		mount("", "/", NULL, MS_REC | MS_SLAVE, NULL); 
		// Mount a proc directory for our processes in private namespace
		mkdir("/proc", 0755);
		mount("proc", "/proc", "proc", 0, NULL);

		char * rootfs = "./bundle/rootfs";
		// mount new rootfs
		mount(rootfs, rootfs, NULL, MS_REC | MS_PRIVATE | MS_BIND, NULL);
		// chdir to new rootfs
		chdir(rootfs);
		// set current directory as the root directory
		syscall(SYS_pivot_root, ".", ".");
		// unmount old root directory
		umount2(".", MNT_DETACH);
		chdir("/");
		// Set hostname
		char* hostname = "jovan";
		sethostname(hostname, strlen(hostname));

		pid = fork();

		// Execute the command provided
		if(pid == 0) execvp(argv[1], &argv[1]);
		while (wait(&status) != -1 || errno != ECHILD);
	}	
	wait(&status);
	exit(0);
}