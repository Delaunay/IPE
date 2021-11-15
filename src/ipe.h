
#include <ostream>
#include <string>
#include <vector>

namespace ipe {

#define IPE_STATUS(STATUS)                                                                         \
    STATUS(None, "")                                                                               \
    STATUS(Ready, "")                                                                              \
    STATUS(Stopped, "")

#define IPE_ERRORS(ERR)                                                                            \
    ERR(Success, "")                                                                               \
    ERR(MissingFile, "")                                                                           \
    ERR(Locked, "")                                                                                \
    ERR(TimedOut, "")                                                                              \
    ERR(ReadOnlyFs, "")

#define IPE_INIT_METHOD(METHOD)                                                                    \
    METHOD(FFA, "")                                                                                \
    METHOD(Zero, "")                                                                               \
    METHOD(None, "")

enum class Status {
#define STATUS(name, msg) name,
    IPE_STATUS(STATUS)
#undef STATUS
};

enum class InitMethod {
#define METHOD(name, msg) name,
    IPE_INIT_METHOD(METHOD)
#undef METHOD
};

enum class Errors {
#define ERR(name, msg) name,
    IPE_ERRORS(ERR)
#undef ERR
};

const char *get_status(Status s);

const char *get_error(Errors err);

const char *get_method(InitMethod err);

#define NAME_SIZE 16
#define VALUE_SIZE 64

//! Entry represent the data of a single worker
struct Entry {
    const char name[NAME_SIZE]   = {0};
    Status     status            = Status::None;
    uint64_t   time              = 0;
    uint8_t    lock              = 0;
    const char value[VALUE_SIZE] = {0};
};

/*!
 * @param wid worker id between [0, n_worker[
 * @param n name of the IPE file
 * @param n_w number of workers in total
 * @param t timeout
 * @param ffa use free for all initialization
 *
 * Free For All initialization will make all workers try to initialize IFE but only the
 * first one do it.
 * To avoid ffa you can call init before launching the workers.
 */
struct Worker {
    public:
    Worker(int wid, std::string const &n, std::size_t n_w);

    ~Worker();

    inline bool is_worker() const { return worker_id >= 0 && worker_id < data.size(); }

    inline bool is_monitor() const { return !is_worker(); }

    //! Set the worker status, call write() to commit to file
    void set_status(Status s);
    //! Set the worker time, call write() to commit to file
    void set_time(uint64_t t);
    //! Set the worker name, call write() to commit to file
    void set_name(std::string const &name);
    //! Set the worker name, call write() to commit to file
    void set_name(const char *name);
    //! Set the worker name, call write() to commit to file
    void set_name(const char *name, std::size_t size);
    //! Set the worker data, call write() to commit to file
    void set_data(const char *value, std::size_t size);

    std::vector<std::string> values();

    // acquire lock, no other workers can acquire lock
    Errors acquire_lock();

    // unlock our workers
    Errors unlock();

    // read the state of all our workers
    Errors read();

    //! commit our worker state to file
    Errors write();

    // Display the status of all workers
    Errors status(std::ostream &out);

    // Wait for all workers to appear, ie. status != None
    int rendezvous(int timeout);

    // Select a server for a task k
    // all workers will select the same
    int select(std::string const &k);

    private:
    Entry &worker() {
        if(is_monitor()) {
            static Entry e;
            return e;
        }

        return data[worker_id];
    }

    int                worker_id;
    std::string const &name;
    std::size_t        n_worker;
    FILE *             handle = nullptr;
    std::vector<Entry> data;
};

// Initialize the IPE file, this needs to be called once and only once
// if a file already exists it will be overriden
Errors init_ipe(std::string const &name, std::size_t n_worker);

// Initialize the IPE file, the first worker that is able to book the initialization
// will create the IPE file, this can be called many times
// if a file exists, the workers will assume it is a correct IPE file
Errors init_ffa(std::string const &name, std::size_t n_worker, int timeout);

using Monitor = Worker;

Worker new_worker(int worker_id, std::string const &path, std::size_t n_worker,
                  InitMethod ffa = InitMethod::FFA, int timeout = -1);

Monitor new_monitor(std::string const &path, std::size_t n_worker,
                    InitMethod ffa = InitMethod::None, int timeout = -1);

} // namespace ipe