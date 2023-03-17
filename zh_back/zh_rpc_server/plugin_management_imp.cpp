#include "plugin_management_imp.h"
#include "zh_rpc_util.h"
#include <unistd.h>
#include <sys/wait.h>

plugin_management_handler *plugin_management_handler::m_inst = nullptr;
static std::vector<std::string> split_string(const std::string &str, const std::string &pattern)
{
    using namespace std;
    vector<string> res;
    if (str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + pattern.size(), strs.size());
        pos = strs.find(pattern);
    }

    std::vector<std::string> ret;
    auto head = res.begin();
    auto tail = head;
    while (head != res.end())
    {
        if ((*head)[0] == '\'')
        {
            while (tail != res.end())
            {
                if ((*tail)[tail->length() - 1] == '\'')
                {
                    break;
                }
                tail++;
            }
            auto i = head;
            std::string single_arg;
            for (;i<= tail;i++)
            {
                single_arg += *i + " ";
            }
            if (single_arg.length() > 0)
            {
                single_arg.pop_back();
                ret.push_back(single_arg.substr(1, single_arg.length() - 2));
            }
            head = tail;
        }
        else
        {
            ret.push_back(*head);
        }
        head++;
    }

    return ret;
}
void plugin_management_handler::zh_plugin_run_plugin(const std::string &_cmd, const std::string &_plugin_name, std::string &_stdout_string, std::string &_stderr_string)
{
    int pipfd[2];
    int pipfd_err[2];
    tdf_log tmp_log("plugin_exec", "/plugin/audit.log", "/plugin/audit.log");
    tmp_log.log("execute cmd:%s from plugin:%s", _cmd.c_str(), _plugin_name.c_str());
    std::string plug_cmd = "/plugin/" + _plugin_name + "/bin/" + _plugin_name + "_plugin";
    if (access(plug_cmd.c_str(), X_OK) != 0)
    {
        tdf_log tmp("plugin_exe");
        tmp.err("cannot exec plugin");
        return;
    }
    pipe(pipfd);
    pipe(pipfd_err);
    int ipid = fork();
    if (ipid > 0)
    {
        close(pipfd[1]);
        close(pipfd_err[1]);
        char buff[4096 * 1048] = {0};
        int read_len = 0;
        fd_set set;
        bool select_should_exit = false;
        while (select_should_exit == false)
        {
            FD_ZERO(&set);
            FD_SET(pipfd[0], &set);
            FD_SET(pipfd_err[0], &set);
            int max_fd = pipfd[0];
            if (pipfd_err[0] > max_fd)
            {
                max_fd = pipfd_err[0];
            }
            if (0 < select(max_fd + 1, &set, nullptr, nullptr, nullptr))
            {
                if (FD_ISSET(pipfd[0], &set))
                {
                    if ((read_len = read(pipfd[0], buff, sizeof(buff))) > 0)
                    {
                        _stdout_string.assign(buff, read_len);
                    }
                    else
                    {
                        select_should_exit = true;
                    }
                }
                if (FD_ISSET(pipfd_err[0], &set))
                {
                    if ((read_len = read(pipfd_err[0], buff, sizeof(buff))) > 0)
                    {
                        _stderr_string.assign(buff, read_len);
                    }
                    else
                    {
                        select_should_exit = true;
                    }
                }
                else
                {
                    select_should_exit = true;
                }
            }
            else
            {
                select_should_exit = true;
            }
        }
        close(pipfd[0]);
        close(pipfd_err[0]);

        int istatus = 0;
        waitpid(ipid, &istatus, 0);
        bool exit_normal = false;
        if (WEXITSTATUS(istatus) == 0)
        {
            exit_normal = true;
        }
        if (!exit_normal)
        {
            _stderr_string.insert(0, "出错原因");
        }
        if (_stdout_string.length() > 0 && _stdout_string[_stdout_string.size() - 1] == '\n')
        {
            _stdout_string.pop_back();
        }
    }
    else
    {
        auto all_args = split_string(_cmd, " ");
        char **argv = (char **)calloc(1UL, (all_args.size() + 2) * sizeof(char *));
        int i = 1;
        argv[0] = (char *)calloc(1UL, plug_cmd.length() + 1);
        plug_cmd.copy(argv[0], plug_cmd.length(), 0);
        for (auto &itr : all_args)
        {
            argv[i] = (char *)calloc(1UL, itr.length() + 1);
            itr.copy(argv[i], itr.length(), 0);
            i++;
        }
        for (auto i = 0; i < all_args.size() + 1; ++i)
        {
            m_log.log("args is :%s", argv[i]);
        }
        close(pipfd[0]);
        close(pipfd_err[0]);
        dup2(pipfd[1], 1);
        dup2(pipfd_err[1], 2);
        execv(plug_cmd.c_str(), argv);
        _exit(0);
    }
}
void plugin_management_handler::run_plugin_cmd(std::string &_return, const std::string &ssid, const std::string &plugin_name, const std::string &cmd)
{
    int cmd_permisson_req = 1;
    if (cmd.substr(0, 3) == "get")
    {
        cmd_permisson_req = 3;
    }
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    std::string error_string;
    zh_plugin_run_plugin(cmd, plugin_name, _return, error_string);
    if (error_string.length() > 0)
    {
        ZH_RETURN_MSG(error_string);
    }
}

bool plugin_management_handler::install_plugin(const std::string &ssid, const std::string &plugin_name, const std::string &file_name)
{
    bool ret = false;
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }

    std::string cmd = "mv '" + file_name + "' '/tmp/" + plugin_name + ".tar.gz' && [ -d /plugin ] || mkdir /plugin && tar xf '/tmp/" + plugin_name + ".tar.gz' -C /plugin/ && mkdir '/plugin/" + plugin_name + "/conf' && /plugin/" + plugin_name + "/bin/" + plugin_name + "_plugin init";
    if (0 == system(cmd.c_str()))
    {
        if (subscribe_event(plugin_name))
        {
            ret = true;
        }
        else
        {
            uninstall_plugin(ssid, plugin_name);
        }
    }

    return ret;
}
void plugin_management_handler::uninstall_plugin(const std::string &ssid, const std::string &plugin_name)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, false);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    std::string cmd = "rm -rf '/plugin/" + plugin_name + "'";
    system(cmd.c_str());
    unsubscribe_event(plugin_name);
}
void plugin_management_handler::get_installed_plugins(std::vector<std::string> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true);
    if (!user)
    {
        ZH_RETURN_NEED_PRAVILIGE(ZH_PERMISSON_TARGET_USER);
    }
    auto all_plugins = internel_get_installed_plugins();
    _return = all_plugins;
}

std::vector<std::string> plugin_management_handler::internel_get_installed_plugins()
{
    std::vector<std::string> _return;
    std::string cmd = "ls /plugin/ -l | grep '^d' | awk '{print $NF}'";
    auto read_fd = popen(cmd.c_str(), "r");
    if (read_fd)
    {
        std::string plugins;
        long buff;
        int read_len = 0;
        while ((read_len = fread(&buff, 1, sizeof(buff), read_fd)) > 0)
        {
            plugins.append((char *)&buff, read_len);
        }
        pclose(read_fd);
        _return = split_string(plugins, "\n");
        if (_return.size() > 0)
        {
            _return.pop_back();
            for (auto itr = _return.begin(); itr != _return.end(); itr++)
            {
                if (*itr == "audit.log")
                {
                    _return.erase(itr);
                    break;
                }
            }
        }
    }
    return _return;
}

void plugin_management_handler::deliver_event(const plugin_event_info &_event)
{
    pthread_mutex_lock(&m_que_lock);
    for (auto itr = event_deliver_map.begin(); itr != event_deliver_map.end(); itr++)
    {
        std::string std_out;
        std::string std_err;
        auto tmp_event = _event;
        zh_plugin_run_plugin("get -k prio_key", itr->first, std_out, std_err);
        if (std_out.length() > 0)
        {
            tmp_event.prio_key = atol(std_out.c_str());
        }
        zh_plugin_run_plugin("proc_event -c " + _event.get_cmd() + " xxx -t", itr->first, std_out, std_err);
        if (std_err.empty())
        {
            itr->second.push_back(tmp_event);
        }
    }
    pthread_cond_signal(&que_cond);
    pthread_mutex_unlock(&m_que_lock);
}

bool plugin_management_handler::subscribe_event(const std::string &_plugin_name)
{
    bool ret = false;

    pthread_mutex_lock(&m_que_lock);
    if (event_deliver_map.find(_plugin_name) == event_deliver_map.end())
    {
        std::list<plugin_event_info> tmp;
        event_deliver_map[_plugin_name] = tmp;
        ret = true;
    }
    pthread_mutex_unlock(&m_que_lock);

    return ret;
}
void plugin_management_handler::unsubscribe_event(const std::string &_plugin_name)
{
    pthread_mutex_lock(&m_que_lock);
    event_deliver_map.erase(_plugin_name);
    pthread_mutex_unlock(&m_que_lock);
}

void plugin_management_handler::finish_event(const std::string &_plugin_name, bool runing_only)
{
    pthread_mutex_lock(&m_que_lock);

    if (event_deliver_map.find(_plugin_name) != event_deliver_map.end())
    {
        auto &que = event_deliver_map[_plugin_name];
        if (que.size() > 0)
        {
            auto &tmp = que.front();
            bool need_pop = false;
            if (!runing_only)
            {
                need_pop = true;
            }
            else if (_plugin_name == tmp.plugin_name)
            {
                need_pop = true;
            }
            if (need_pop)
            {
                que.pop_front();
            }
        }
    }

    pthread_mutex_unlock(&m_que_lock);
}

void plugin_management_handler::get_que_by_name(std::vector<std::string> &_return, const std::string &ssid, const std::string &plugin_name)
{
    if (zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true))
    {
        pthread_mutex_lock(&m_que_lock);
        if (event_deliver_map.find(plugin_name) != event_deliver_map.end())
        {
            auto &tmp = event_deliver_map[plugin_name];
            for (auto &itr : tmp)
            {
                _return.push_back(itr.order_number + itr.get_cmd());
            }
        }
        pthread_mutex_unlock(&m_que_lock);
    }
}

void plugin_management_handler::pop_event_from_que(const std::string &ssid, const std::string &plugin_name)
{
    if (zh_rpc_util_get_online_user(ssid, ZH_PERMISSON_TARGET_USER, true))
    {
        pthread_mutex_lock(&m_que_lock);
        if (event_deliver_map.find(plugin_name) != event_deliver_map.end())
        {
            auto &tmp = event_deliver_map[plugin_name];
            if (tmp.size() > 0)
            {
                tmp.pop_front();
            }
        }
        pthread_mutex_unlock(&m_que_lock);
    }
}

void plugin_management_handler::ext_deliver_event(const std::string &order_number, const int64_t ev_type)
{
    plugin_event_info tmp;
    tmp.order_number = order_number;
    tmp.type = (plugin_event_info::event_type)ev_type;
    deliver_event(tmp);
}