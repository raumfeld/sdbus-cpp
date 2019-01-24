#include <systemd/sd-bus.h>
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <map>

int method_get_problem(sd_bus_message *m, void */*userdata*/, sd_bus_error */*ret_error*/)
{
    int r{};

    int clientID;
    r = sd_bus_message_read_basic(m, SD_BUS_TYPE_INT32, &clientID);
    if (r < 0)
        throw std::runtime_error(strerror(-r));

    // REAL WORK/LOGIC HERE >>
    std::cout << "Got call of client ID " << clientID << std::endl;
    std::map<std::string, std::string> problem {{std::to_string(clientID), std::to_string(clientID+1)}, {std::to_string(clientID+2), std::to_string(clientID+3)}};
    //problem = generateProblemSomehow(clientID);
    // <<

    sd_bus_message *reply{};
    r = sd_bus_message_new_method_return(m, &reply);
    if (r < 0)
        throw std::runtime_error(strerror(-r));

    r = sd_bus_message_open_container(reply, SD_BUS_TYPE_ARRAY, "{ss}");
    if (r < 0)
        throw std::runtime_error(strerror(-r));

    for (const auto& item : problem)
    {
        r = sd_bus_message_open_container(reply, SD_BUS_TYPE_DICT_ENTRY, "ss");
        if (r < 0)
            throw std::runtime_error(strerror(-r));

        r = sd_bus_message_append_basic(reply, SD_BUS_TYPE_STRING, item.first.c_str());
        if (r < 0)
            throw std::runtime_error(strerror(-r));

        r = sd_bus_message_append_basic(reply, SD_BUS_TYPE_STRING, item.second.c_str());
        if (r < 0)
            throw std::runtime_error(strerror(-r));

        r = sd_bus_message_close_container(reply);
        if (r < 0)
            throw std::runtime_error(strerror(-r));
    }

    r = sd_bus_message_close_container(reply);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
    
    r = sd_bus_send(nullptr, reply, nullptr);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
    
    sd_bus_message_unref(reply);

    return r;
}

const sd_bus_vtable problems_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("getProblem", "i", "a{ss}", method_get_problem, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END
};

int main()
{
    int r{};

    /* Open system bus */
    sd_bus *bus;
    r = sd_bus_new(&bus);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    int fds[2] = {0, 0};
    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

/*
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error(strerror(-fd));

    struct sockaddr_un namesock;
    namesock.sun_family = AF_UNIX;
    strncpy(namesock.sun_path, (char *)"/run/sdbustest-socket", sizeof(namesock.sun_path));
    r = bind(fd, (struct sockaddr *) &namesock, sizeof(struct sockaddr_un));
    if (r < 0)
        throw std::runtime_error(strerror(-r));

    printf("Here\n");

    r = sd_bus_set_fd(bus, fd, fd);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
*/

    r = sd_bus_set_fd(bus, fds[0], fds[0]);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    sd_id128_t id;
    r = sd_id128_randomize(&id);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    r = sd_bus_set_server(bus, 1, id);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    r = sd_bus_start(bus);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    /* Register callbacks for com.kistler.dbusapp.problems interface */
    sd_bus_slot *slot1;
    r = sd_bus_add_object_vtable(bus,
                                 &slot1,
                                 "/com/kistler/dbusapp",  /* object path */
                                 "com.kistler.dbusapp.problems",   /* interface name */
                                 problems_vtable,
                                 nullptr);
    if (r < 0)
        throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    /* Request service name */
    //r = sd_bus_request_name(bus, "com.kistler.testsdbuscpp", 0);
    //if (r < 0)
    //    throw std::runtime_error(std::to_string(__LINE__) + strerror(-r));

    bool run = true;
    while (run)
    {
        /* Process requests */
        auto r = sd_bus_process(bus, nullptr);
        if (r < 0)
            throw std::runtime_error(std::string("Failed to process bus: ") + strerror(-r));
        if (r > 0) /* we processed a request, try to process another one, right-away */
            continue;

        /* Wait for the next request to process */
        r = sd_bus_wait(bus, 1000000);
        if (r < 0)
            throw std::runtime_error(std::string("Failed to wait on bus: ") + strerror(-r));
    }

    /* Cleanup */
    sd_bus_slot_unref(slot1);
    sd_bus_unref(bus);
}

