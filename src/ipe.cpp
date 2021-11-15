#include "ipe.h"

#include <cstring>
#include <spdlog/fmt/bundled/core.h>

#ifdef __linux__
#include "unistd.h"
#endif

#define IPE_LOCK_NAME ".ipe_lock"
#define UINT64T_MAX "18446744073709551615"

namespace ipe {
const char *get_status(Status s) {
    switch(s) {
#define STATUS(name, msg)                                                                          \
    case Status::name:                                                                             \
        return #name;

        IPE_STATUS(STATUS)
#undef STATUS
    }

    return "";
}

const char *get_error(Errors err) {
    switch(err) {
#define ERR(name, msg)                                                                             \
    case Errors::name:                                                                             \
        return #name;
        IPE_ERRORS(ERR)
#undef ERR
    }

    return "";
}

const char *get_method(InitMethod err) {
    switch(err) {
#define METHOD(name, msg)                                                                          \
    case InitMethod::name:                                                                         \
        return #name;
        IPE_INIT_METHOD(METHOD)
#undef METHOD
    }

    return "";
}

// rename is atomic for most fle systems
// so we create unique file for each of our workers and try to rename it to the same
// file, all workers will fail except one
Errors is_master() {
    uint64_t uid = time(NULL) + rand();

    char lock_name[sizeof(UINT64T_MAX) + sizeof(IPE_LOCK_NAME) + 1] = {0};
    sprintf(lock_name, "%s_%lu", IPE_LOCK_NAME, uid);

    FILE *lock = fopen(lock_name, "w");
    if(lock == nullptr) {
        return Errors::ReadOnlyFs;
    }
    fclose(lock);

    if(rename(lock_name, IPE_LOCK_NAME) != 0) {
        remove(lock_name);
        return Errors::Locked;
    }

    return Errors::Success;
}

// wait for the lock file to disappear
Errors wait_for_lock(int timeout) {
    int passed = 0;
#ifdef __linux__
    while(access(IPE_LOCK_NAME, F_OK) == 0) {
        usleep(1);
        passed += 1;

        if(timeout > 0 && passed > timeout) {
            return Errors::TimedOut;
        }
    }
#else
    // try to open the file
    FILE *handle = nullptr;
    do {
        handle = fopen(IPE_LOCK_NAME, "r");
    } while(handle != nullptr);
#endif

    return Errors::Success;
}

Errors init_ipe(std::string const &name, std::size_t n_worker) {
    FILE *handle = fopen(name.c_str(), "w");
    if(handle == nullptr) {
        return Errors::ReadOnlyFs;
    }

    // Initialize the file
    std::vector<Entry> data(n_worker);
    fwrite((void *)data.data(), sizeof(Entry), n_worker, handle);
    fclose(handle);
    return Errors::Success;
}

// Needs to be called before the worker starts
Errors init_ffa(std::string const &name, std::size_t n_worker, int timeout) {
    FILE *h = fopen(name.c_str(), "r+");
    if(h != nullptr) {
        return Errors::Success;
    }

    // not being the main is not an error into itself
    // that just means somebody else is going to initialize our file
    // wait for it and finish normally
    if(is_master() == Errors::Locked) {
        return wait_for_lock(timeout);
    }

    // Initialize the file
    return init_ipe(name, n_worker);
}

Worker::Worker(int wid, std::string const &n, std::size_t n_w) :
    worker_id(wid), name(n), n_worker(n_w), data(std::vector<Entry>(n_w)) {

    handle = fopen(n.c_str(), "r+");

    if(is_worker()) {
        worker().status = Status::Ready;
        write();
    }
}

Worker::~Worker() {
    if(is_worker()) {
        unlock();
        worker().status = Status::Stopped;
        write();
    }
    fclose(handle);
}

void Worker::set_status(Status s) { worker().status = s; }

void Worker::set_time(uint64_t t) { worker().time = t; };

void Worker::set_name(std::string const &name) { set_name(name.c_str(), name.size()); }

void Worker::set_name(const char *name) { set_name(name, strlen(name)); }

void Worker::set_name(const char *name, std::size_t size) {
    memcpy((void *)worker().name, name, size > NAME_SIZE ? NAME_SIZE : size);
};

void Worker::set_data(const char *value, std::size_t size) {
    memcpy((void *)worker().value, value, size > VALUE_SIZE ? VALUE_SIZE : size);
};

std::vector<std::string> Worker::values() {
    std::vector<std::string> v;
    if(!handle) {
        return v;
    }

    read();
    v.reserve(n_worker);
    for(Entry const &entry : data) {
        auto r = std::string(entry.value);
        if(r == "") {
            continue;
        }
        v.push_back(r);
    }

    return v;
}

Errors Worker::acquire_lock() {
    if(is_monitor()) {
        return Errors::Success;
    }

    if(!handle) {
        return Errors::MissingFile;
    }

    // Before acquiring the lock, we set it to 1 so we prevent other workers from
    // locking while we are checking
    worker().lock = 1;
    write();
    read();

    int locks = 0;
    for(Entry const &entry : data) {
        locks += entry.lock;
    }

    if(locks > 1) {
        worker().lock = 0;
        return Errors::Locked;
    }

    return Errors::Success;
}

Errors Worker::unlock() {
    if(is_monitor()) {
        return Errors::Success;
    }

    if(worker().lock > 0) {
        worker().lock = 0;
        write();
    }
    return Errors::Success;
}

Errors Worker::read() {
    if(!handle) {
        return Errors::MissingFile;
    }
    fseek(handle, 0, SEEK_SET);
    fread((void *)data.data(), sizeof(Entry), n_worker, handle);
    return Errors::Success;
}

// we can only work on our worker
Errors Worker::write() {
    if(is_monitor()) {
        return Errors::Success;
    }

    if(!handle) {
        return Errors::MissingFile;
    }
    fseek(handle, sizeof(Entry) * worker_id, SEEK_SET);
    fwrite((void *)&worker(), sizeof(Entry), 1, handle);
    fflush(handle);
    return Errors::Success;
}

int Worker::select(std::string const &key) {
    int offset = std::hash<std::string>()(key);
    int k      = 0;
    int i      = 0;

    read();

    do {
        k = int((offset + i) % n_worker);
        i += 1;
    } while(data[k].status != Status::Ready);

    return k;
}

Errors Worker::status(std::ostream &out) {
    if(!handle) {
        out << "Could not open IPE file";
        return Errors::MissingFile;
    }

    read();

    out << fmt::format("{:>16} | {:>8} | {:>12} | {:<64} |\n", "Name", "Status", "Time", "Value");
    for(Entry const &entry : data) {
        out << fmt::format("{:>16} | {:>8} | {:>12d} | {:<64} |\n", entry.name,
                           get_status(entry.status), entry.time, entry.value);
    }

    return Errors::Success;
}

int Worker::rendezvous(int timeout) {
    if(!handle) {
        return 0;
    }

    int passed  = 0;
    int ready   = 0;
    int stopped = 0;
    do {
        if(ready != 0) {
            usleep(1);
            passed += 1;
        }

        read();

        ready   = 0;
        stopped = 0;
        for(Entry const &entry : data) {
            // There is nothing we cannot about stopped workers
            // so they must count as ready
            ready += entry.status == Status::Ready;
            stopped += entry.status == Status::Stopped;
        }

        if(timeout > 0 && passed > timeout) {
            return ready;
        }
    } while(ready + stopped != n_worker);
    return ready;
}

Worker new_worker(int worker_id, std::string const &path, std::size_t n, InitMethod m, int t) {
    switch(m) {
    case InitMethod::FFA:
        init_ffa(path, n, t);
        break;
    case InitMethod::Zero:
        init_ipe(path, n);
        break;
    case InitMethod::None:
        break;
    }

    return Worker(worker_id, path, n);
}

Monitor new_monitor(std::string const &path, std::size_t n_worker, InitMethod method, int timeout) {

    switch(method) {
    case InitMethod::FFA:
        init_ffa(path, n_worker, timeout);
        break;
    case InitMethod::Zero:
        init_ipe(path, n_worker);
        break;
    case InitMethod::None:
        break;
    }

    return Worker(-1, path, n_worker);
}

} // namespace ipe