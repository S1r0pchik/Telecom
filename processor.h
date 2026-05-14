#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <array>
#include <queue>
#include <string>
#include <vector>

namespace factory {

enum class EventType { FINISH = 0, START = 1, WAIT = 2, READY = 3 };

struct Event {
    long long time;
    EventType type;
    int product_level;
    int product_id;
    int machine_id;
    int queue_size;

    bool operator<(const Event &other) const {
        if (time != other.time) return time > other.time;
        if (type != other.type) return type > other.type; // FINISH > START > WAIT > READY
        if (product_id != other.product_id) return product_id > other.product_id;
        return machine_id > other.machine_id;
    }
};

struct Product {
    int level;
    int id;
};

struct Machine {
    std::queue<Product> products;
    long long product_sum_time = 0;
};

class Processor {
public:
    void Solve(int argc, char *argv[]);

private:
private:
    void ReadInput();
    void ReadHeader();
    void ReadTimeMatrix();
    void ReadMachineQueues();

    static std::string GetNextLine();
    [[noreturn]] static void PrintErrorAndExit(const std::string &line);

    void InitMachines();

    void ProcessStart(const Event &event);
    void ProcessFinish(const Event &event);
    void ProcessReady(const Event &event);
    void ProcessWait(const Event &event);

    [[nodiscard]] int FindBestMachine() const;

    static void PrintStart(long long t, int k, int i, int j);
    static void PrintFinish(long long t, int k, int i, int j);
    static void PrintReady(long long t, int k, int j);
    static void PrintWait(long long t, int k, int i, int j, int p);
    static void PrintStop(long long t);

    void AddProductToMachine(int machine_id, int product_level, int product_id);
    void RemoveProductFromMachine(int machine_id);
    void ScheduleNextOnMachine(int machine_id, long long current_time);
    void RouteFinishedProduct(const Event &event);
    void AddEvent(long long time, EventType type, int level, int id, int machine, int q_size = 0);

    int type_count_ = 0;     // M
    int machines_count_ = 0; // N
    int total_products_ = 0; // S
    int finished_products_ = 0;

    std::array<std::array<long long, 105>, 105> time_matrix_{};
    std::priority_queue<Event> event_queue_;
    std::vector<Machine> machines_;
};

} // namespace factory

#endif // PROCESSOR_H