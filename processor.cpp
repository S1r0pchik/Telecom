#include "processor.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace factory {

void Processor::Solve(const int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return;
    }

    freopen(argv[1], "r", stdin);

    ReadInput();
    InitMachines();

    while (!event_queue_.empty()) {
        using enum factory::EventType;
        Event current = event_queue_.top();
        event_queue_.pop();

        if (current.type == START) {
            ProcessStart(current);
        } else if (current.type == FINISH) {
            ProcessFinish(current);
        } else if (current.type == WAIT) {
            ProcessWait(current);
        } else if (current.type == READY) {
            ProcessReady(current);
        }
    }
}

void Processor::ReadInput() {
    ReadHeader();
    ReadTimeMatrix();
    ReadMachineQueues();
}

void Processor::ReadHeader() {
    const std::string line = GetNextLine();
    std::stringstream ss(line);

    if (!(ss >> type_count_ >> machines_count_) || type_count_ < 1 || type_count_ > 100 || machines_count_ < 1 ||
        machines_count_ > 100) {
        PrintErrorAndExit(line);
    }

    if (std::string trash; ss >> trash) {
        PrintErrorAndExit(line);
    }
}

void Processor::ReadTimeMatrix() {
    for (int i = 0; i < type_count_ - 1; i++) {
        std::string line = GetNextLine();
        std::stringstream ss_time(line);

        for (int j = 0; j < machines_count_; j++) {
            long long time_val;
            if (!(ss_time >> time_val) || time_val < 0 || time_val > 10000) {
                PrintErrorAndExit(line);
            }
            time_matrix_[i][j] = time_val;
        }

        if (std::string trash; ss_time >> trash) {
            PrintErrorAndExit(line);
        }
    }
}

void Processor::ReadMachineQueues() {
    machines_.resize(machines_count_);
    int product_id = 0;

    for (int machine_id = 0; machine_id < machines_count_; machine_id++) {
        std::string line = GetNextLine();
        std::stringstream ss_queue(line);

        int q_count;
        if (!(ss_queue >> q_count) || q_count < 0) {
            PrintErrorAndExit(line);
        }

        total_products_ += q_count;
        if (total_products_ > 100000) {
            PrintErrorAndExit(line);
        }

        for (int j = 0; j < q_count; j++) {
            int level;
            if (!(ss_queue >> level) || level < 0 || level > type_count_ - 2) {
                PrintErrorAndExit(line);
            }
            AddProductToMachine(machine_id, level, product_id);
            product_id++;
        }

        if (std::string trash; ss_queue >> trash) {
            PrintErrorAndExit(line);
        }
    }
}

std::string Processor::GetNextLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "Unexpected End of File\n";
        std::exit(1);
    }
    return line;
}

void Processor::PrintErrorAndExit(const std::string &line) {
    std::cout << line << std::endl;
    std::exit(1);
}

void Processor::InitMachines() {
    for (int machine_id = 0; machine_id < machines_count_; machine_id++) {
        if (machines_[machine_id].products.empty()) {
            continue;
        }

        auto [product_level, product_id] = machines_[machine_id].products.front();
        AddEvent(0, EventType::START, product_level, product_id, machine_id);
    }
}

void Processor::ProcessStart(const Event &event) {
    PrintStart(event.time, event.product_id, event.product_level, event.machine_id);

    const long long finish_time = event.time + time_matrix_[event.product_level][event.machine_id];
    AddEvent(finish_time, EventType::FINISH, event.product_level, event.product_id, event.machine_id);
}

void Processor::ProcessFinish(const Event &event) {
    PrintFinish(event.time, event.product_id, event.product_level, event.machine_id);

    RemoveProductFromMachine(event.machine_id);

    ScheduleNextOnMachine(event.machine_id, event.time);

    RouteFinishedProduct(event);
}

void Processor::ScheduleNextOnMachine(const int machine_id, const long long current_time) {
    if (machines_[machine_id].products.empty()) {
        return;
    }

    auto [next_level, next_id] = machines_[machine_id].products.front();
    AddEvent(current_time, EventType::START, next_level, next_id, machine_id);
}

void Processor::RouteFinishedProduct(const Event &event) {
    if (event.product_level == type_count_ - 2) {
        AddEvent(event.time, EventType::READY, event.product_level + 1, event.product_id, event.machine_id);
    } else {
        const int next_level = event.product_level + 1;
        const int best_machine = FindBestMachine();
        const auto current_queue_size = static_cast<int>(machines_[best_machine].products.size());

        AddProductToMachine(best_machine, next_level, event.product_id);
        AddEvent(event.time, EventType::WAIT, next_level, event.product_id, best_machine, current_queue_size);
    }
}

void Processor::ProcessWait(const Event &event) {
    PrintWait(event.time, event.product_id, event.product_level, event.machine_id, event.queue_size);

    if (event.queue_size == 0) {
        AddEvent(event.time, EventType::START, event.product_level, event.product_id, event.machine_id);
    }
}

void Processor::ProcessReady(const Event &event) {
    PrintReady(event.time, event.product_id, event.machine_id);

    finished_products_++;
    if (finished_products_ == total_products_) {
        PrintStop(event.time);
    }
}

int Processor::FindBestMachine() const {
    int best_machine = 0;
    long long min_time = LLONG_MAX;

    for (int j = 0; j < machines_count_; j++) {
        if (const long long current_wait_time = machines_[j].product_sum_time; current_wait_time < min_time) {
            min_time = current_wait_time;
            best_machine = j;
        }
    }
    return best_machine;
}

void Processor::PrintStart(const long long t, const int k, const int i, const int j) {
    std::cout << "start " << t << " " << k << " " << i << " " << j << "\n";
}

void Processor::PrintFinish(const long long t, const int k, const int i, const int j) {
    std::cout << "finish " << t << " " << k << " " << i << " " << j << "\n";
}

void Processor::PrintReady(const long long t, const int k, const int j) {
    std::cout << "ready " << t << " " << k << " " << j << "\n";
}

void Processor::PrintWait(const long long t, const int k, const int i, const int j, const int p) {
    std::cout << "wait " << t << " " << k << " " << i << " " << j << " " << p << "\n";
}

void Processor::PrintStop(const long long t) {
    std::cout << "stop " << t << "\n";
}

void Processor::AddProductToMachine(const int machine_id, const int product_level, const int product_id) {
    machines_[machine_id].products.push({product_level, product_id});
    machines_[machine_id].product_sum_time += time_matrix_[product_level][machine_id];
}

void Processor::RemoveProductFromMachine(const int machine_id) {
    if (!machines_[machine_id].products.empty()) {
        auto [product_level, product_id] = machines_[machine_id].products.front();
        machines_[machine_id].products.pop();
        machines_[machine_id].product_sum_time -= time_matrix_[product_level][machine_id];
    }
}

void Processor::AddEvent(
    const long long time, const EventType type, const int level, const int id, const int machine, const int q_size) {
    event_queue_.emplace(time, type, level, id, machine, q_size);
}

} // namespace factory