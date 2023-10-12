#include "../tdf_timer.h"
int main(int argc, char const *argv[])
{
    timer_wheel_init();
    bool need_exit = false;
    bool run_config = true;
    while (run_config)
    {
        std::cout << "enter func no" << std::endl;
        std::cout << "1:add timer func" << std::endl;
        std::cout << "2:add one time func" << std::endl;
        std::cout << "3:add timer exit" << std::endl;
        std::cout << "4:run" << std::endl;
        int func_no = 0;
        std::string tmp;
        std::getline(std::cin, tmp);
        func_no = atoi(tmp.c_str());
        switch (func_no)
        {
        case 1:
        {
            std::cout << "enter sec:";
            int sec = 0;
            std::string tmp;
            std::getline(std::cin, tmp);
            sec = atoi(tmp.c_str());
            std::cout << "enter func:";
            std::string func_cont;
            std::getline(std::cin, func_cont);
            timer_wheel_add_node(
                sec, [=](void *)
                { system(func_cont.c_str()); });
            break;
        }
        case 2:
        {
            std::cout << "enter sec:";
            int sec = 0;
            std::string tmp;
            std::getline(std::cin, tmp);
            sec = atoi(tmp.c_str());
            std::cout << "enter func:";
            std::string func_cont;
            std::getline(std::cin, func_cont);
            timer_wheel_add_node(
                sec, [=](void *)
                { system(func_cont.c_str()); },
                true);
            break;
        }
        case 3:
        {
            std::cout << "enter sec:";
            int sec = 0;
            std::string tmp;
            std::getline(std::cin, tmp);
            sec = atoi(tmp.c_str());
            timer_wheel_add_node(
                sec, [&](void *)
                { need_exit = true; });
            break;
        }
        case 4:
        {
            run_config = false;
        }
        default:
            break;
        }
    }
    struct abc
    {
        int *m_cont;
        abc() : m_cont(new int())
        {
        }
        ~abc()
        {
            delete m_cont;
            m_cont = nullptr;
        }
    };
    auto p_p = new abc();
    auto t3 = timer_wheel_add_node(
        1, [](void *_pp)
        {
            usleep(1500000);
        std::cout << "========1=======" << std::endl;
        *(((abc *)_pp)->m_cont) = 10; },
        false, p_p);

    int i = 3;
    while (!need_exit)
    {
        timer_wheel_schc();
        if (0 == i--)
        {
            timer_wheel_del_node(t3);
            p_p->m_cont = nullptr;
        }
    }
    delete p_p;
    timer_wheel_fini();
    return 0;
}
