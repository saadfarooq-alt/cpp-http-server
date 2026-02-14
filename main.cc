import <iostream>;
import <sys/socket.h>;
import <netinet/in.h>;
import <unistd.h>;

using namespace std;

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "failed to create such socket"
        return 1;
    }

    cout << "Socket was created successfully"
    close(server_fd);
}

