#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <future>
#include <queue>
#include <atomic>

struct Address {
    std::string addr;
    int status_code;
    double ping;
};

std::vector<int> ports = {80, 443};
std::string output_format = "[$status_code$]: $addr$ | $ping$ ms";
std::string output_file;
std::mutex cout_mutex;
std::mutex queue_mutex;
std::queue<std::string> address_queue;
std::atomic<int> active_threads(0);
int max_scans = 1;

void check_address(const std::string& addr) {
    Address result{addr, 0, 0.0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(addr.c_str(), NULL, &hints, &res) != 0) {
        result.status_code = -1;
    } else {
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == -1) {
            result.status_code = -2;
        } else {
            for (int port : ports) {
                ((struct sockaddr_in*)res->ai_addr)->sin_port = htons(port);
                if (connect(sock, res->ai_addr, res->ai_addrlen) == 0) {
                    result.status_code = 200;
                    break;
                }
            }
            close(sock);
        }
        freeaddrinfo(res);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.ping = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::string output = output_format;
    output.replace(output.find("$status_code$"), 13, std::to_string(result.status_code));
    output.replace(output.find("$addr$"), 6, result.addr);
    output.replace(output.find("$ping$"), 6, std::to_string(result.ping));
    
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << output << std::endl;
    
    if (!output_file.empty()) {
        std::ofstream outfile(output_file, std::ios_base::app);
        outfile << output << std::endl;
    }
}

void worker() {
    while (true) {
        std::string addr;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (address_queue.empty()) {
                break;
            }
            addr = address_queue.front();
            address_queue.pop();
        }
        for (int i = 0; i < max_scans; ++i) {
            check_address(addr);
        }
    }
    --active_threads;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> addresses;
    int num_threads = std::thread::hardware_concurrency();
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--ports") {
            ports.clear();
            std::istringstream iss(argv[++i]);
            int port;
            while (iss >> port) {
                ports.push_back(port);
                if (iss.peek() == ',') iss.ignore();
            }
        } else if (arg == "-f" || arg == "--format") {
            output_format = argv[++i];
        } else if (arg == "-o" || arg == "--output") {
            output_file = argv[++i];
        } else if (arg == "-t" || arg == "--threads") {
            num_threads = std::stoi(argv[++i]);
        } else if (arg == "-s" || arg == "--scans") {
            max_scans = std::stoi(argv[++i]);
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        } else {
            if (arg.find('.') != std::string::npos) {
                addresses.push_back(arg);
            } else {
                std::ifstream infile(arg);
                std::string line;
                while (std::getline(infile, line)) {
                    addresses.push_back(line);
                }
            }
        }
    }
    
    for (const auto& addr : addresses) {
        address_queue.push(addr);
    }
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
        ++active_threads;
    }
    
    while (active_threads > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
