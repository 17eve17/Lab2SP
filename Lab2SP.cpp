#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <tuple>

struct Proc {
    int id;
    int arrival;
    int burst;
    int remaining;
    int priority;
    int start;
    int finish;
    int wait;

    Proc(int pid, int a, int b, int prio)
        : id(pid), arrival(a), burst(b), remaining(b),
        priority(prio), start(-1), finish(0), wait(0) {
    }
};

std::vector<Proc> create_procs(int n) {
    std::vector<Proc> list;
    for (int i = 0; i < n; i++) {
        int a = rand() % 11;
        int b = rand() % 10 + 1;
        int pr = rand() % 5 + 1;
        list.push_back(Proc(i + 1, a, b, pr));
    }
    return list;
}

std::vector<std::tuple<int, int>> rr_schedule(std::vector<Proc>& list, int quantum) {
    std::vector<std::tuple<int, int>> log;
    std::queue<int> q;
    int time = 0;
    int idx = 0;
    std::sort(list.begin(), list.end(), [](Proc& x, Proc& y) { return x.arrival < y.arrival; });

    while (idx < list.size() || !q.empty()) {
        while (idx < list.size() && list[idx].arrival <= time) {
            q.push(idx);
            idx++;
        }
        if (!q.empty()) {
            Proc& p = list[q.front()]; q.pop();
            if (p.start == -1) p.start = time;
            int t = std::min(quantum, p.remaining);
            p.remaining -= t;
            time += t;
            log.push_back(std::make_tuple(p.id, p.remaining));
            while (idx < list.size() && list[idx].arrival <= time) {
                q.push(idx);
                idx++;
            }
            if (p.remaining > 0) q.push(idx - 1); else p.finish = time;
        }
        else time++;
    }

    for (auto& p : list) p.wait = p.finish - p.arrival - p.burst;
    return log;
}

std::vector<std::tuple<int, int, int, int>> fcfs_schedule(std::vector<Proc>& list) {
    std::vector<std::tuple<int, int, int, int>> log;
    std::sort(list.begin(), list.end(), [](Proc& x, Proc& y) { return x.arrival < y.arrival; });
    int time = 0;
    for (auto& p : list) {
        if (time < p.arrival) time = p.arrival;
        p.start = time;
        p.finish = time + p.burst;
        p.wait = p.start - p.arrival;
        time += p.burst;
        log.push_back(std::make_tuple(p.id, p.start, p.finish, p.wait));
    }
    return log;
}

std::vector<std::tuple<int, int, int>> priority_schedule(std::vector<Proc>& list) {
    std::vector<std::tuple<int, int, int>> log;
    std::sort(list.begin(), list.end(), [](Proc& x, Proc& y) {
        if (x.priority != y.priority) return x.priority < y.priority;
        return x.arrival < y.arrival;
        });
    int time = 0;
    for (auto& p : list) {
        if (time < p.arrival) time = p.arrival;
        p.start = time;
        p.finish = time + p.burst;
        p.wait = p.start - p.arrival;
        time += p.burst;
        log.push_back(std::make_tuple(p.id, p.burst, p.priority));
    }
    return log;
}

std::vector<std::tuple<int, int, int>> dynamic_rr(std::vector<Proc>& list, int quantum) {
    std::vector<std::tuple<int, int, int>> log;
    std::queue<int> q;
    int time = 0;
    int idx = 0;
    std::sort(list.begin(), list.end(), [](Proc& x, Proc& y) { return x.arrival < y.arrival; });

    while (idx < list.size() || !q.empty()) {
        while (idx < list.size() && list[idx].arrival <= time) {
            q.push(idx);
            idx++;
        }
        if (!q.empty()) {
            Proc& p = list[q.front()]; q.pop();
            if (p.start == -1) p.start = time;
            int t = std::min(quantum, p.remaining);
            p.remaining -= t;
            time += t;
            if (p.priority > 1) p.priority--;
            log.push_back(std::make_tuple(p.id, p.remaining, p.priority));
            while (idx < list.size() && list[idx].arrival <= time) {
                q.push(idx);
                idx++;
            }
            if (p.remaining > 0) q.push(idx - 1); else p.finish = time;
        }
        else time++;
    }

    for (auto& p : list) p.wait = p.finish - p.arrival - p.burst;
    return log;
}

std::pair<double, double> compute_stats(std::vector<Proc>& list) {
    double w = 0, ta = 0;
    for (auto& p : list) {
        w += p.wait;
        ta += p.finish - p.arrival;
    }
    return { w / list.size(), ta / list.size() };
}

int main() {
    srand(time(0));
    std::vector<Proc> base = create_procs(5);
    int quantum = 3;

    std::vector<Proc> rr_list = base;
    auto rr_log = rr_schedule(rr_list, quantum);
    auto rr_stats = compute_stats(rr_list);

    std::vector<Proc> fcfs_list = base;
    auto fcfs_log = fcfs_schedule(fcfs_list);
    auto fcfs_stats = compute_stats(fcfs_list);

    std::vector<Proc> prio_list = base;
    auto prio_log = priority_schedule(prio_list);
    auto prio_stats = compute_stats(prio_list);

    std::vector<Proc> dyn_list = base;
    auto dyn_log = dynamic_rr(dyn_list, quantum);
    auto dyn_stats = compute_stats(dyn_list);

    std::cout << "RR Log:\n";
    for (auto& e : rr_log) std::cout << "PID " << std::get<0>(e) << ", rem " << std::get<1>(e) << "\n";

    std::cout << "\nFCFS Log:\n";
    for (auto& e : fcfs_log) std::cout << "PID " << std::get<0>(e) << ", start " << std::get<1>(e)
        << ", finish " << std::get<2>(e) << ", wait " << std::get<3>(e) << "\n";

    std::cout << "\nPriority Log:\n";
    for (auto& e : prio_log) std::cout << "PID " << std::get<0>(e) << ", burst " << std::get<1>(e)
        << ", prio " << std::get<2>(e) << "\n";

    std::cout << "\nDynamic RR Log:\n";
    for (auto& e : dyn_log) std::cout << "PID " << std::get<0>(e) << ", rem " << std::get<1>(e)
        << ", prio " << std::get<2>(e) << "\n";

    std::cout << "\nAverage Times:\n";
    std::cout << "RR: Wait=" << rr_stats.first << ", Turn=" << rr_stats.second << "\n";
    std::cout << "FCFS: Wait=" << fcfs_stats.first << ", Turn=" << fcfs_stats.second << "\n";
    std::cout << "Priority: Wait=" << prio_stats.first << ", Turn=" << prio_stats.second << "\n";
    std::cout << "Dynamic RR: Wait=" << dyn_stats.first << ", Turn=" << dyn_stats.second << "\n";

    return 0;
}
