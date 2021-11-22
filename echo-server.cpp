#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux
#include <iostream>
#include <thread>
#include <set>

using namespace std;

set <int> sd_set;

void usage() {
	cout << "syntax: echo-server <port> [-e [-b]]\n";
	cout << "sample: echo-server 1234 -e -b\n";
}

struct Param {
	bool echo{false};
	bool bcast{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				cout << "echo mode\n";
				continue;
			}
			if (strcmp(argv[i],"-b") == 0) {
				bcast = true;
				cout << "broadcast mode\n";
				continue;
			}
			port = stoi(argv[i]);
		}
		return port != 0;
	}
} param;

void recvThread(int sd) {
	cout << "[client " << sd << "]" << "connected\n";
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	while (true) {
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res;
			perror(" ");
			break;
		}
		buf[res] = '\0';
		cout << "[message from client " << sd << "] " << buf;
		cout.flush();
		if (param.echo) {
			cout << "[echo to " << sd << "]\n";
			res = send(sd, buf, res, 0);
			if (res == 0 || res == -1) {
				cerr << "send return " << res;
				perror(" ");
				break;
			}
		}
		if (param.bcast) {
			cout << "[broad cast]\n";
			set<int>::iterator iter;
    		for(iter = sd_set.begin(); iter != sd_set.end(); iter++){
				if(param.echo && *iter==sd){
					continue;
				}

        		res = send(*iter, buf, res, 0);
				if (res == 0 || res == -1) {
				cerr << "send return " << res;
				perror(" ");
				break;
			}
    		}
		}
	}
	cout << "disconnected\n";
	close(sd);
}

int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int res;
#ifdef __linux__
	int optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}
#endif // __linux

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}

		sd_set.insert(cli_sd);

		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
