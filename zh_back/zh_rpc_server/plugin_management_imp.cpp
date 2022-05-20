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
        char buff = 0;
        while (read(pipfd[0], &buff, sizeof(buff)) > 0)
        {
            _stdout_string.push_back(buff);
        }
        std::string error_string;
        while (read(pipfd_err[0], &buff, sizeof(buff)) > 0)
        {
            error_string.push_back(buff);
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
        if (error_string.size() > 0 || !exit_normal)
        {
            error_string.insert(0, "出错原因");
            _stderr_string = error_string;
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
    auto user = zh_rpc_util_get_online_user(ssid, cmd_permisson_req);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
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
    auto user = zh_rpc_util_get_online_user(ssid, 0);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }

    std::string cmd = "mv '" + file_name + "' '/tmp/" + plugin_name + ".tar.gz' && [ -d /plugin ] || mkdir /plugin && tar xf '/tmp/" + plugin_name + ".tar.gz' -C /plugin/ && mkdir '/plugin/" + plugin_name + "/conf' && /plugin/" + plugin_name + "/bin/" + plugin_name + "_plugin init";
    if (0 == system(cmd.c_str()))
    {
        ret = true;
    }

    return ret;
}
void plugin_management_handler::uninstall_plugin(const std::string &ssid, const std::string &plugin_name)
{
    auto user = zh_rpc_util_get_online_user(ssid, 0);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    std::string cmd = "rm -rf '/plugin/" + plugin_name + "'";
    system(cmd.c_str());
}
void plugin_management_handler::get_installed_plugins(std::vector<std::string> &_return, const std::string &ssid)
{
    auto user = zh_rpc_util_get_online_user(ssid, 2);
    if (!user)
    {
        ZH_RETURN_NO_PRAVILIGE();
    }
    auto all_plugins = internel_get_installed_plugins();
    _return = all_plugins;
}

std::vector<std::string> plugin_management_handler::internel_get_installed_plugins()
{
    std::vector<std::string> _return;
    std::string cmd = "ls /plugin/";
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
        }
    }
    return _return;
}