#include "../utils/local_utils.h"

static neb::CJsonObject get_core_config()
{
    neb::CJsonObject ret;
    std::string cmd = "cat /conf/core_config.yaml | yaml2json";
    auto filep = popen(cmd.c_str(), "r");
    if (filep)
    {
        char buff[1024] = {0};
        long read_len = -1;
        std::string json_string;
        while (0<(read_len = fread(buff, 1, sizeof(buff), filep)))
        {
            json_string.append(buff, read_len);
        }
        pclose(filep);
        ret.Parse(json_string);
    }

    return ret;
}
static epoll_sch_center g_epoll_sch;

struct process_meta_t;
static std::map<std::string, process_meta_t> g_proc_map;
static std::map<int, process_meta_t *> g_proc_pid_map;
class pipfd_epoll_node : public epoll_node_t
{
    int m_fd = -1;
    process_meta_t *m_meta = nullptr;
    bool is_error = false;

public:
    pipfd_epoll_node(int _fd, process_meta_t *_meta, bool _is_err) : m_fd(_fd), m_meta(_meta), is_error(_is_err)
    {
    }
    bool proc_in();
    int get_fd() const
    {
        return m_fd;
    }
};

class pidfd_epoll_node : public epoll_node_t
{
    int m_fd = -1;
    int pid = -1;
    process_meta_t *m_process = nullptr;

public:
    pidfd_epoll_node(int _fd, int _pid, process_meta_t *_process) : m_fd(_fd), pid(_pid), m_process(_process) {}
    bool proc_in();
    int get_fd() const
    {
        return m_fd;
    }
};
struct process_meta_t
{
    std::string program_name;
    std::vector<std::string> cmd_args;
    int m_pid = -1;
    std::shared_ptr<tdf_log> log = nullptr;
    int std_out_fd = -1;
    int std_err_fd = -1;
    int pid_fd = -1;
    std::shared_ptr<pipfd_epoll_node> pip_out_node;
    std::shared_ptr<pipfd_epoll_node> pip_err_node;
    std::shared_ptr<pidfd_epoll_node> pid_node;

    void start_process();
    void end_process();
};

bool pidfd_epoll_node::proc_in()
{
    int status = 0;
    m_process->end_process();
    m_process->start_process();
    return true;
}
bool pipfd_epoll_node::proc_in()
{
    char buff[2048] = {0};
    int readlen = -1;
    readlen = read(m_fd, buff, sizeof(buff));
    if (readlen > 0)
    {
        if (is_error)
        {
            m_meta->log->err(std::string(buff, readlen));
        }
        else
        {
            m_meta->log->log(std::string(buff, readlen));
        }
    }

    return true;
}
void process_meta_t::start_process()
{
    int pipfd[2] = {-1};
    int pipfd_err[2] = {-1};
    log.reset(new tdf_log( program_name));
    if (0 == pipe(pipfd) && 0 == pipe(pipfd_err))
    {
        auto pid = fork();
        if (pid == 0)
        {
            char *buff[1024] = {0};
            int i = 0;
            for (auto &arg_itr : cmd_args)
            {
                char *tmp = (char *)calloc(1UL, arg_itr.size() + 1);
                strcpy(tmp, arg_itr.c_str());
                buff[i++] = tmp;
            }
            close(pipfd[0]);
            close(pipfd_err[0]);
            dup2(pipfd[1], 1);
            dup2(pipfd_err[1], 2);

            prctl(PR_SET_PDEATHSIG, SIGKILL);
            execv(program_name.c_str(), buff);
            exit(-1);
        }
        else
        {
            close(pipfd[1]);
            close(pipfd_err[1]);
            std_err_fd = pipfd_err[0];
            std_out_fd = pipfd[0];
            pid_fd = syscall(SYS_pidfd_open, pid, 0);
            m_pid = pid;
            pip_out_node = std::make_shared<pipfd_epoll_node>(std_out_fd, this, false);
            pip_err_node = std::make_shared<pipfd_epoll_node>(std_err_fd, this, true);
            pid_node = std::make_shared<pidfd_epoll_node>(pid_fd, pid, this);
            auto add_res = g_epoll_sch.add_node(*pip_out_node);
            add_res += g_epoll_sch.add_node(*pip_err_node);
            add_res += g_epoll_sch.add_node(*pid_node);
            log.reset(new tdf_log(program_name + "]<" + std::to_string(pid) +  ">[" + cmd_args[0]));
            log->log("begin to run, add epoll ret:%d", add_res);
        }
    }
    else
    {
        log->err("failed to open pipe");
    }
}
void process_meta_t::end_process()
{
    log->log("%s is shutdown, pid:%d, name:%s", program_name.c_str(), m_pid, cmd_args[0].c_str());
    int status = 0;
    waitpid(m_pid, &status, 0);
    g_epoll_sch.del_node(*pip_out_node);
    g_epoll_sch.del_node(*pip_err_node);
    g_epoll_sch.del_node(*pid_node);
    close(pid_fd);
    close(std_err_fd);
    close(std_out_fd);
}

static void make_global_proc_map()
{
    auto core_config = get_core_config();
    for (auto i = 0; i < core_config["components"].GetArraySize(); i++)
    {
        auto process_meta = core_config["components"][i];
        auto program_name = process_meta("program_name");
        auto device_name = process_meta("name");
        std::vector<std::string> real_cmd_args;
        real_cmd_args.push_back(device_name);
        if (process_meta.KeyExist("msg_center"))
        {
            real_cmd_args.push_back(process_meta("msg_center"));
        }
        for (auto j = 0; j < process_meta["cmd_args"].GetArraySize(); j++)
        {
            auto tmp_arg = process_meta(process_meta["cmd_args"](j));
            if (tmp_arg.length() <= 0)
            {
                break;
            }
            real_cmd_args.push_back(tmp_arg);
        }
        g_proc_map[device_name].cmd_args = real_cmd_args;
        g_proc_map[device_name].program_name = program_name;
    }
}

static void start_process()
{
    auto itr = g_proc_map.begin();
    for (; itr != g_proc_map.end(); ++itr)
    {
        std::string mq_name = "/" + itr->first;
        auto fd = mq_open(mq_name.c_str(), O_RDWR | O_CREAT, 0666, nullptr);
        if (fd)
        {
            mq_close(fd);
        }
        itr->second.start_process();
    }
}

int main(int argc, char const *argv[])
{
    make_global_proc_map();
    start_process();
    g_epoll_sch.run();

    return 0;
}
